/*
 * sd_fat.c - FAT16 / FAT32 sobre la tarjeta SD (ver MicroSD.h).
 *
 * Un solo bufer de 512 bytes (la "ventana") sirve para el directorio, la
 * tabla FAT y los datos: antes de cambiar de sector se guarda si se
 * modifico. La tabla FAT se escribe en todas sus copias.
 * Los numeros de la FAT son little-endian: se arman byte a byte (el STC32G
 * es big-endian).
 *
 * C portable: SDCC (stc8duino) y Keil C251 (stc32duino).
 * Parte de stc8duino. LGPL 2.1.
 */
#include <Arduino.h>
#include "MicroSD.h"
#include "sd_interno.h"

#define NADA 0xFFFFFFFFUL

/* ventana */
static uint8_t win[512];
static unsigned long win_sec = NADA;
static uint8_t win_sucio;

/* volumen */
static uint8_t tipo_fat, spc, nfats, fsinfo_tocado;
static unsigned long fat_ini, fat_tam, raiz_ini, raiz_secs, datos_ini, n_clus;
static unsigned long raiz_clus, fsinfo_sec, pista = 2, clus_bytes;

/* recorrido de la carpeta raiz */
static unsigned long dir_clus, dir_sec, dir_n;
static uint8_t dir_idx, dir_fin;

/* archivo abierto */
static uint8_t f_abierto, f_modo, f_cambio;
static unsigned long f_clus0, f_tam, f_pos, f_clus, f_ent_sec;
static unsigned int f_ent_off;

/* fecha de los archivos: 2026-01-01 00:00 */
#define FECHA ((unsigned int)(((2026 - 1980) << 9) | (1 << 5) | 1))

/* sin <string.h>: Keil C251 y el nucleo definen size_t por su cuenta */
static void copiar(uint8_t *d, const uint8_t *o, unsigned int n) { while (n--) *d++ = *o++; }
static void llenar(uint8_t *d, uint8_t v, unsigned int n)        { while (n--) *d++ = v; }
static uint8_t igual(const uint8_t *a, const uint8_t *b, uint8_t n)
{
	while (n--) if (*a++ != *b++) return 0;
	return 1;
}

static unsigned int ld16(const uint8_t *p) { return (unsigned int)p[0] | ((unsigned int)p[1] << 8); }
static unsigned long ld32(const uint8_t *p)
{
	return (unsigned long)ld16(p) | ((unsigned long)ld16(p + 2) << 16);
}
static void st16(uint8_t *p, unsigned int v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static void st32(uint8_t *p, unsigned long v) { st16(p, (unsigned int)v); st16(p + 2, (unsigned int)(v >> 16)); }

/* ------------------------------------------------------------ ventana */
static uint8_t guardar(void)
{
	if (win_sucio) {
		if (sd_writeSector(win_sec, win)) return SD_ERROR_ES;
		win_sucio = 0;
	}
	return SD_OK;
}

static uint8_t mover(unsigned long sec)
{
	if (sec == win_sec) return SD_OK;
	if (guardar()) return SD_ERROR_ES;
	if (sd_readSector(sec, win)) {
		win_sec = NADA;
		return SD_ERROR_ES;
	}
	win_sec = sec;
	return SD_OK;
}

/* para escribir un sector entero: no hace falta leerlo */
static uint8_t mover_nuevo(unsigned long sec)
{
	if (sec == win_sec) return SD_OK;
	if (guardar()) return SD_ERROR_ES;
	win_sec = sec;
	return SD_OK;
}

/* ---------------------------------------------------------- tabla FAT */
static unsigned long fat_leer(unsigned long c)
{
	if (tipo_fat == 16) {
		if (mover(fat_ini + (c >> 8))) return NADA;
		return ld16(win + ((unsigned int)(c & 0xFF) << 1));
	}
	if (mover(fat_ini + (c >> 7))) return NADA;
	return ld32(win + ((unsigned int)(c & 0x7F) << 2)) & 0x0FFFFFFFUL;
}

static uint8_t fat_poner(unsigned long c, unsigned long v) SD_PILA
{
	uint8_t i;
	uint8_t *p;
	for (i = 0; i < nfats; i++) {
		if (tipo_fat == 16) {
			if (mover(fat_ini + i * fat_tam + (c >> 8))) return SD_ERROR_ES;
			st16(win + ((unsigned int)(c & 0xFF) << 1), (unsigned int)v);
		} else {
			if (mover(fat_ini + i * fat_tam + (c >> 7))) return SD_ERROR_ES;
			p = win + ((unsigned int)(c & 0x7F) << 2);
			st32(p, (ld32(p) & 0xF0000000UL) | (v & 0x0FFFFFFFUL));
		}
		win_sucio = 1;
	}
	return SD_OK;
}

static uint8_t es_fin(unsigned long c)
{
	if (c < 2 || c >= n_clus + 2) return 1;          /* fin, libre o fuera de rango */
	return tipo_fat == 16 ? c >= 0xFFF8UL : c >= 0x0FFFFFF8UL;
}

static unsigned long clus_sec(unsigned long c) { return datos_ini + (c - 2) * spc; }

/* FAT32: el contador de espacio libre del FSInfo queda viejo: "desconocido" */
static void fsinfo_invalidar(void)
{
	if (tipo_fat != 32 || fsinfo_tocado || fsinfo_sec == 0) return;
	fsinfo_tocado = 1;
	if (mover(fsinfo_sec)) return;
	if (ld32(win) != 0x41615252UL) return;           /* firma "RRaA" */
	st32(win + 488, NADA);
	st32(win + 492, NADA);
	win_sucio = 1;
}

/* busca un cluster libre, lo marca como fin y lo engancha despues de prev */
static unsigned long asignar(unsigned long prev) SD_PILA
{
	unsigned long c = pista, i;
	for (i = 0; i < n_clus; i++, c++) {
		if (c >= n_clus + 2) c = 2;
		if (fat_leer(c) == 0) {
			if (fat_poner(c, NADA)) return 0;
			if (prev && fat_poner(prev, c)) return 0;
			pista = c + 1;
			fsinfo_invalidar();
			return c;
		}
	}
	return 0;
}

static void liberar(unsigned long c)
{
	unsigned long sig;
	while (!es_fin(c)) {
		sig = fat_leer(c);
		if (fat_poner(c, 0)) return;
		if (c < pista) pista = c;
		c = sig;
	}
	fsinfo_invalidar();
}

/* ------------------------------------------------------------- montaje */
static uint8_t es_bpb(const uint8_t *b)
{
	return (b[0] == 0xEB || b[0] == 0xE9) && ld16(b + 11) == 512 && b[13] != 0 && b[16] != 0;
}

uint8_t sd_begin(uint8_t cs) SD_PILA
{
	unsigned long vol = 0, tot, rsv, ent;
	uint8_t r;
	f_abierto = 0;
	win_sec = NADA;
	win_sucio = 0;
	tipo_fat = 0;
	fsinfo_tocado = 0;
	r = sd_cardInit(cs);
	if (r) return r;
	if (mover(0)) return SD_ERROR_ES;
	if (win[510] != 0x55 || win[511] != 0xAA) return SD_SIN_FAT;
	if (!es_bpb(win)) {                              /* tabla de particiones: la primera */
		vol = ld32(win + 0x1BE + 8);
		if (win[0x1BE + 4] == 0 || mover(vol)) return SD_SIN_FAT;
		if (win[510] != 0x55 || win[511] != 0xAA || !es_bpb(win)) return SD_SIN_FAT;
	}
	spc = win[13];
	rsv = ld16(win + 14);
	nfats = win[16];
	ent = ld16(win + 17);
	tot = ld16(win + 19);
	if (tot == 0) tot = ld32(win + 32);
	fat_tam = ld16(win + 22);
	if (fat_tam == 0) fat_tam = ld32(win + 36);
	raiz_secs = (ent * 32 + 511) / 512;
	fat_ini = vol + rsv;
	raiz_ini = fat_ini + nfats * fat_tam;
	datos_ini = raiz_ini + raiz_secs;
	n_clus = (tot - (rsv + nfats * fat_tam + raiz_secs)) / spc;
	clus_bytes = (unsigned long)spc * 512;
	if (n_clus < 4085) return SD_SIN_FAT;            /* FAT12: no */
	if (n_clus < 65525UL) {
		tipo_fat = 16;
		fsinfo_sec = 0;
	} else {
		tipo_fat = 32;
		raiz_clus = ld32(win + 44);
		fsinfo_sec = ld16(win + 48);
		if (fsinfo_sec) fsinfo_sec += vol;
	}
	pista = 2;
	return SD_OK;
}

uint8_t sd_fatType(void) { return tipo_fat; }

unsigned long sd_volumeSize(void) { return n_clus * spc / 2048; }

/* --------------------------------------------------- carpeta raiz */
static void dir_inicio(void)
{
	dir_fin = 0;
	dir_idx = 0;
	dir_n = 0;
	if (tipo_fat == 16) {
		dir_clus = 0;
		dir_sec = raiz_ini;
	} else {
		dir_clus = raiz_clus;
		dir_sec = clus_sec(raiz_clus);
	}
}

/* entrada actual (en la ventana) o 0 al terminar la carpeta */
static uint8_t *dir_entrada(void)
{
	unsigned long c;
	if (dir_fin) return 0;
	if (dir_idx == 16) {
		dir_idx = 0;
		dir_n++;
		if (tipo_fat == 16) {
			if (dir_n >= raiz_secs) return 0;
			dir_sec++;
		} else if (dir_n % spc == 0) {
			c = fat_leer(dir_clus);
			if (es_fin(c)) return 0;
			dir_clus = c;
			dir_sec = clus_sec(c);
		} else {
			dir_sec++;
		}
	}
	if (tipo_fat == 16 && dir_n >= raiz_secs) return 0;
	if (mover(dir_sec)) return 0;
	return win + ((unsigned int)dir_idx << 5);
}

/* "datos.txt" -> "DATOS   TXT" */
static uint8_t a_83(const char *s, uint8_t *n)
{
	uint8_t i = 0, lim = 8;
	char c;
	llenar(n, ' ', 11);
	if (*s == 0 || *s == '.') return 0;
	while ((c = *s++) != 0) {
		if (c == '.') {
			if (lim == 11) return 0;                 /* dos puntos */
			i = 8;
			lim = 11;
			continue;
		}
		if (c <= ' ' || c == '"' || c == '*' || c == '+' || c == ',' || c == '/' ||
		    (c >= ':' && c <= '?') || c == '[' || c == '\\' || c == ']' || c == '|') return 0;
		if (c >= 'a' && c <= 'z') c = (char)(c - 32);
		if (i >= lim) return 0;
		n[i++] = (uint8_t)c;
	}
	return 1;
}

static uint8_t usable(const uint8_t *e)
{
	return e[0] != 0xE5 && e[11] != 0x0F && !(e[11] & 0x08) && e[0] != '.';
}

void sd_dirStart(void) { dir_inicio(); }

uint8_t sd_dirNext(char *nombre, unsigned long *tam) SD_PILA
{
	uint8_t *e;
	uint8_t i, k;
	if (!tipo_fat) return 0;
	while ((e = dir_entrada()) != 0) {
		dir_idx++;
		if (e[0] == 0) {                             /* fin de la carpeta */
			dir_fin = 1;
			return 0;
		}
		if (!usable(e)) continue;
		k = 0;
		for (i = 0; i < 8 && e[i] != ' '; i++) nombre[k++] = (char)e[i];
		if (e[8] != ' ') {
			nombre[k++] = '.';
			for (i = 8; i < 11 && e[i] != ' '; i++) nombre[k++] = (char)e[i];
		} else if (e[11] & 0x10) {
			nombre[k++] = '/';                       /* carpeta */
		}
		nombre[k] = 0;
		if (tam) *tam = (e[11] & 0x10) ? 0 : ld32(e + 28);
		return 1;
	}
	return 0;
}

/* busca n en la raiz. 1 = encontrado (f_ent_*); recuerda un lugar libre en lib_* */
static unsigned long lib_sec;
static unsigned int lib_off;
static uint8_t buscar(const uint8_t *n)
{
	uint8_t *e;
	lib_sec = NADA;
	dir_inicio();
	while ((e = dir_entrada()) != 0) {
		if (e[0] == 0 || e[0] == 0xE5) {
			if (lib_sec == NADA) {
				lib_sec = dir_sec;
				lib_off = (unsigned int)dir_idx << 5;
			}
			if (e[0] == 0) return 0;
		} else if (usable(e) && igual(e, n, 11)) {
			f_ent_sec = dir_sec;
			f_ent_off = (unsigned int)dir_idx << 5;
			return 1;
		}
		dir_idx++;
	}
	return 0;
}

uint8_t sd_exists(const char *nombre)
{
	uint8_t n[11];
	if (!tipo_fat || !a_83(nombre, n)) return 0;
	return buscar(n);
}

/* ---------------------------------------------------------- archivo */
uint8_t sd_flush(void) SD_PILA
{
	uint8_t *e;
	if (!f_abierto) return SD_OK;
	if (f_cambio) {
		if (mover(f_ent_sec)) return SD_ERROR_ES;
		e = win + f_ent_off;
		st16(e + 26, (unsigned int)f_clus0);
		st16(e + 20, tipo_fat == 32 ? (unsigned int)(f_clus0 >> 16) : 0);
		st32(e + 28, f_tam);
		e[11] |= 0x20;                               /* archivo modificado */
		st16(e + 22, 0);
		st16(e + 24, FECHA);
		st16(e + 18, FECHA);
		win_sucio = 1;
		f_cambio = 0;
	}
	return guardar();
}

uint8_t sd_close(void)
{
	uint8_t r = sd_flush();
	f_abierto = 0;
	return r;
}

/* carpeta raiz llena: en FAT32 se le agrega un cluster (en FAT16 su tamano es fijo) */
static uint8_t raiz_crecer(void) SD_PILA
{
	unsigned long c;
	uint8_t i;
	if (tipo_fat != 32) return SD_LLENA;
	c = asignar(dir_clus);                          /* dir_clus: el ultimo de la raiz */
	if (!c) return SD_LLENA;
	for (i = 0; i < spc; i++) {
		if (mover_nuevo(clus_sec(c) + i)) return SD_ERROR_ES;
		llenar(win, 0, 512);
		win_sucio = 1;
	}
	lib_sec = clus_sec(c);
	lib_off = 0;
	return SD_OK;
}

uint8_t sd_open(const char *nombre, uint8_t modo) SD_PILA
{
	uint8_t n[11];
	uint8_t *e;
	unsigned long k;
	if (f_abierto) sd_close();
	if (!tipo_fat) return SD_SIN_FAT;
	if (!a_83(nombre, n)) return SD_NOMBRE_MALO;
	if (!buscar(n)) {
		if (modo == SD_LEER) return SD_NO_EXISTE;
		if (lib_sec == NADA && raiz_crecer()) return SD_LLENA;
		if (mover(lib_sec)) return SD_ERROR_ES;
		e = win + lib_off;
		llenar(e, 0, 32);
		copiar(e, n, 11);
		e[11] = 0x20;
		st16(e + 16, FECHA);
		st16(e + 18, FECHA);
		st16(e + 24, FECHA);
		win_sucio = 1;
		f_ent_sec = lib_sec;
		f_ent_off = lib_off;
	}
	if (mover(f_ent_sec)) return SD_ERROR_ES;
	e = win + f_ent_off;
	if (e[11] & 0x10) return SD_NO_EXISTE;           /* es una carpeta */
	f_clus0 = ld16(e + 26);
	if (tipo_fat == 32) f_clus0 |= (unsigned long)ld16(e + 20) << 16;
	f_tam = ld32(e + 28);
	f_pos = 0;
	f_cambio = 0;
	if (modo == SD_ESCRIBIR && (f_clus0 || f_tam)) { /* vaciar */
		liberar(f_clus0);
		f_clus0 = 0;
		f_tam = 0;
		f_cambio = 1;
	}
	f_clus = f_clus0;
	if (modo == SD_AGREGAR && f_tam) {               /* ir al ultimo cluster */
		for (k = (f_tam - 1) / clus_bytes; k && !es_fin(f_clus); k--)
			f_clus = fat_leer(f_clus);
		f_pos = f_tam;
	}
	f_modo = modo;
	f_abierto = 1;
	return SD_OK;
}

unsigned long sd_size(void)     { return f_abierto ? f_tam : 0; }
unsigned long sd_position(void) { return f_abierto ? f_pos : 0; }

/* al empezar un cluster nuevo (posicion multiplo de su tamano) pasa al siguiente */
static uint8_t avanzar(uint8_t crear)
{
	unsigned long c;
	if (f_pos == 0 || (f_pos % clus_bytes) != 0) return SD_OK;
	c = fat_leer(f_clus);
	if (es_fin(c)) {
		if (!crear) return SD_ERROR_ES;
		c = asignar(f_clus);
		if (!c) return SD_LLENA;
	}
	f_clus = c;
	return SD_OK;
}

unsigned int sd_readBuf(void *p, unsigned int n) SD_PILA
{
	uint8_t *d = (uint8_t *)p;
	unsigned int off, k, hecho = 0;
	if (!f_abierto || f_modo != SD_LEER) return 0;
	while (n && f_pos < f_tam) {
		if (avanzar(0)) break;
		off = (unsigned int)(f_pos & 511);
		if (mover(clus_sec(f_clus) + (uint8_t)((f_pos >> 9) % spc))) break;
		k = 512 - off;
		if (k > n) k = n;
		if ((unsigned long)k > f_tam - f_pos) k = (unsigned int)(f_tam - f_pos);
		copiar(d, win + off, k);
		d += k;
		n -= k;
		hecho += k;
		f_pos += k;
	}
	return hecho;
}

int sd_read(void)
{
	uint8_t c;
	return sd_readBuf(&c, 1) == 1 ? c : -1;
}

unsigned int sd_write(const void *p, unsigned int n) SD_PILA
{
	const uint8_t *s = (const uint8_t *)p;
	unsigned int off, k, hecho = 0;
	unsigned long sec;
	if (!f_abierto || f_modo == SD_LEER) return 0;
	while (n) {
		if (f_clus0 == 0) {                          /* archivo vacio: primer cluster */
			f_clus0 = asignar(0);
			if (!f_clus0) break;
			f_clus = f_clus0;
			f_cambio = 1;
		} else if (avanzar(1)) {
			break;
		}
		off = (unsigned int)(f_pos & 511);
		k = 512 - off;
		if (k > n) k = n;
		sec = clus_sec(f_clus) + (uint8_t)((f_pos >> 9) % spc);
		/* sector entero nuevo (mas alla del final): no hace falta leerlo */
		if ((off == 0 && k == 512) || (off == 0 && f_pos >= f_tam)) {
			if (mover_nuevo(sec)) break;
			if (k < 512) llenar(win + k, 0, 512 - k);
		} else if (mover(sec)) {
			break;
		}
		copiar(win + off, s, k);
		win_sucio = 1;
		s += k;
		n -= k;
		hecho += k;
		f_pos += k;
		if (f_pos > f_tam) f_tam = f_pos;
		f_cambio = 1;
	}
	return hecho;
}

unsigned int sd_print(const char *s)
{
	unsigned int n = 0;
	while (s[n]) n++;
	return sd_write(s, n);
}

void sd_printNumber(long v) SD_PILA
{
	char b[12];
	uint8_t i = 11;
	unsigned long u = v < 0 ? (unsigned long)(-v) : (unsigned long)v;
	b[11] = 0;
	do {
		b[--i] = (char)('0' + u % 10);
		u /= 10;
	} while (u);
	if (v < 0) b[--i] = '-';
	sd_print(b + i);
}

uint8_t sd_remove(const char *nombre)
{
	uint8_t n[11];
	uint8_t *e;
	unsigned long c;
	if (!tipo_fat) return SD_SIN_FAT;
	if (!a_83(nombre, n)) return SD_NOMBRE_MALO;
	if (f_abierto) sd_close();
	if (!buscar(n)) return SD_NO_EXISTE;
	e = win + f_ent_off;
	if (e[11] & 0x10) return SD_NO_EXISTE;
	c = ld16(e + 26);
	if (tipo_fat == 32) c |= (unsigned long)ld16(e + 20) << 16;
	e[0] = 0xE5;                                     /* borrada */
	win_sucio = 1;
	liberar(c);
	return guardar();
}
