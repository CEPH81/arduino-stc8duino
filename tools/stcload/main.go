// stcload - carga programas en los STC8H, STC8G, STC8A y STC15 por el puerto serie.
//
// Implementa el protocolo del cargador de la ROM de STC tal como lo hace stcgal
// (https://github.com/grigorig/stcgal, licencia MIT, de Grigori Goronzy y
// colaboradores): calibra el oscilador interno a la frecuencia pedida, borra,
// graba y escribe las opciones (conservando las que ya tenia el chip).
//
// Ademas puede reiniciar el chip en el modo de carga sin apagarlo: si el programa
// que tiene usa Serial (nucleo stc8duino), se le envia "@STCISP#" a su velocidad.
//
// Uso: stcload -port COM3 -chip STC8H8K64U -freq 24000000 -resetbaud 115200 -hex prog.hex
//
// Parte de stc8duino. LGPL 2.1.
package main

import (
	"flag"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

var detalle bool

const version = "0.1.5"

func fail(f string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, "Error: "+f+"\n", a...)
	os.Exit(1)
}

func info(f string, a ...interface{}) { fmt.Printf(f, a...) }

func depurar(f string, a ...interface{}) {
	if detalle {
		fmt.Fprintf(os.Stderr, f, a...)
	}
}

type modelo struct {
	nombre  string
	familia string // stc15, stc8d, stc8g
	magias  []uint16
	codigo  int
}

var modelos = []modelo{
	{"STC8H8K64U", "stc8d", []uint16{0xf784}, 65024},
	{"STC8A8K64D4", "stc8d", []uint16{0xf7f4}, 65024},
	{"STC8G1K08", "stc8g", []uint16{0xf754, 0xf774, 0xf794, 0xf7a4}, 8192},
	{"STC15W4K56S4", "stc15", []uint16{0xf528}, 57344},
	{"STC15F2K60S2", "stc15", []uint16{0xf408}, 61440},
}

func main() {
	port := flag.String("port", "", "puerto serie (COM3, /dev/ttyUSB0)")
	chip := flag.String("chip", "", "modelo: "+nombres())
	freqS := flag.String("freq", "24000000", "frecuencia del oscilador interno, Hz")
	resetBaud := flag.Int("resetbaud", 0, "velocidad del Serial del programa actual para reiniciarlo en modo de carga (0 = no)")
	hexf := flag.String("hex", "", "programa (.hex)")
	baud := flag.Int("baud", 115200, "velocidad de la carga")
	espera := flag.Int("espera", 60, "segundos de espera al chip")
	corte := flag.String("corte", "", "cortar la alimentacion de la placa con la linea rts o dtr del adaptador (circuito de 2 transistores, como stcgal -a)")
	prog := flag.String("prog", "serie", "programador: serie (adaptador USB-serie), link1d (STC-USB Link1D, puerto CDC) o aicube (AiCube-ISP oficial con el Link1D, solo Windows)")
	modo := flag.String("modo", "auto", "oscilador: auto (ajusta solo si el chip no esta a la frecuencia pedida), directo (no lo toca, como STC Web ISP) o ajuste (lo calibra siempre, como stcgal)")
	aicube := flag.String("aicube", "", "ruta de AiCube-ISP*.exe (por defecto se busca)")
	flag.BoolVar(&detalle, "v", false, "mostrar los paquetes")
	soloInfo := flag.Bool("info", false, "solo probar la conexion: identifica el chip y no graba nada")
	flag.Parse()

	depurar("stcload %s\n", version)
	if *soloInfo {
		probar(*port, *chip, *resetBaud, *corte, time.Duration(*espera)*time.Second)
		return
	}
	if *prog == "aicube" {
		cargarAiCube(*aicube, *chip, *hexf)
		return
	}
	if *modo != "auto" && *modo != "directo" && *modo != "ajuste" {
		fail("-modo debe ser auto, directo o ajuste")
	}
	if *port == "" || *chip == "" || *hexf == "" {
		fail("faltan -port, -chip o -hex (modelos: %s)", nombres())
	}
	var m *modelo
	for i := range modelos {
		if strings.EqualFold(modelos[i].nombre, *chip) {
			m = &modelos[i]
		}
	}
	if m == nil {
		fail("modelo desconocido %q (modelos: %s)", *chip, nombres())
	}
	freq, err := strconv.ParseFloat(strings.TrimRight(*freqS, "lLuU"), 64)
	if err != nil || freq < 1e6 || freq > 50e6 {
		fail("frecuencia no valida: %s", *freqS)
	}

	datos, err := leerHex(*hexf)
	if err != nil {
		fail("%v", err)
	}
	if len(datos) > m.codigo {
		fail("el programa ocupa %d bytes y en el %s caben %d", len(datos), m.nombre, m.codigo)
	}
	if len(datos)%512 != 0 {
		datos = append(datos, relleno(512-len(datos)%512)...)
	}

	s := &stc{fam: m.familia, hs: 2400, baud: *baud, trimFreq: int(freq), modelo: m, modo: *modo}
	if *corte != "" && *corte != "rts" && *corte != "dtr" {
		fail("-corte debe ser rts o dtr")
	}
	s.corte = *corte
	if *prog == "link1d" {
		info("Programador STC-USB Link1D: use su puerto CDC (el que usa STC Web ISP).\n")
	}
	if err := s.conectar(*port, *resetBaud, time.Duration(*espera)*time.Second); err != nil {
		fail("%v", err)
	}
	defer s.p.cerrar()
	if err := s.grabar(datos); err != nil {
		s.desconectar()
		fail("%v", err)
	}
	s.desconectar()
	info("stcload %s - Listo: %d bytes grabados, oscilador a %.3f MHz.\n", version, len(datos), float64(s.trimFreq)/1e6)
}

// probar: conecta con el cargador de la ROM, muestra el chip y lo deja arrancar su programa.
func probar(port, chip string, resetBaud int, corte string, espera time.Duration) {
	if port == "" {
		fail("falta -port")
	}
	s := &stc{hs: 2400, baud: 115200, corte: corte}
	for i := range modelos {
		if strings.EqualFold(modelos[i].nombre, chip) {
			s.modelo, s.fam = &modelos[i], modelos[i].familia
		}
	}
	if err := s.conectar(port, resetBaud, espera); err != nil {
		fail("%v", err)
	}
	info("Codigo del modelo: %04X. Conexion correcta: el adaptador, TX/RX y GND estan bien.\n", s.magia)
	s.desconectar()
	s.p.cerrar()
}

func nombres() string {
	var n []string
	for _, m := range modelos {
		n = append(n, m.nombre)
	}
	return strings.Join(n, ", ")
}

func relleno(n int) []byte {
	b := make([]byte, n)
	for i := range b {
		b[i] = 0xff
	}
	return b
}

// leerHex lee un Intel HEX y devuelve la imagen desde la direccion 0.
func leerHex(nombre string) ([]byte, error) {
	d, err := os.ReadFile(nombre)
	if err != nil {
		return nil, err
	}
	img := []byte{}
	base := 0
	for n, l := range strings.Split(string(d), "\n") {
		l = strings.TrimSpace(l)
		if l == "" {
			continue
		}
		if l[0] != ':' || len(l) < 11 || len(l)%2 == 0 {
			return nil, fmt.Errorf("%s, linea %d: formato HEX no valido", nombre, n+1)
		}
		b := make([]byte, (len(l)-1)/2)
		var suma byte
		for i := range b {
			v, err := strconv.ParseUint(l[1+2*i:3+2*i], 16, 8)
			if err != nil {
				return nil, fmt.Errorf("%s, linea %d: %v", nombre, n+1, err)
			}
			b[i] = byte(v)
			suma += b[i]
		}
		if suma != 0 || int(b[0])+5 != len(b) {
			return nil, fmt.Errorf("%s, linea %d: suma de comprobacion", nombre, n+1)
		}
		dir := base + int(b[1])<<8 + int(b[2])
		switch b[3] {
		case 0:
			fin := dir + int(b[0])
			for len(img) < fin {
				img = append(img, 0xff)
			}
			copy(img[dir:], b[4:4+int(b[0])])
		case 1:
			return img, nil
		case 2:
			base = (int(b[4])<<8 | int(b[5])) << 4
		case 4:
			base = (int(b[4])<<8 | int(b[5])) << 16
		}
	}
	return img, nil
}
