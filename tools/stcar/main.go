// stcar - crea y actualiza la libreria del nucleo (core.a) para el enlazador de SDCC.
//
// Hace lo mismo que "sdar rcs LIB OBJ" (formato ar de GNU con el indice de simbolos
// que lee sdld), pero leyendo y escribiendo siempre en binario. El sdar de Windows
// rompia a veces la libreria ("file format not recognized" o el indice ilegible y
// todas las funciones del nucleo sin definir).
//
// Uso: stcar LIBRERIA.a OBJETO.rel [OBJETO.rel...]
//      stcar -x LIBRERIA.a CARPETA    (extrae los objetos; lo usa el nucleo STC32)
//      stcar -obj ENTRADA SALIDA      (Serial.print... para C251, ver obj.go)
//
// Parte de stc8duino. LGPL 2.1.
package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type miembro struct {
	nombre string
	datos  []byte
}

func fallo(f string, a ...interface{}) {
	fmt.Fprintf(os.Stderr, "stcar: "+f+"\n", a...)
	os.Exit(1)
}

// leer devuelve los objetos de una libreria existente (sin el indice ni la tabla de nombres).
func leer(ruta string) ([]miembro, error) {
	d, err := os.ReadFile(ruta)
	if errors.Is(err, os.ErrNotExist) {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	if !bytes.HasPrefix(d, []byte("!<arch>\n")) {
		return nil, nil // libreria rota o de otro formato: se crea de nuevo
	}
	var largos []byte
	var ms []miembro
	p := 8
	for p+60 <= len(d) {
		h := d[p : p+60]
		tam, err := strconv.Atoi(strings.TrimSpace(string(h[48:58])))
		if err != nil || p+60+tam > len(d) {
			return nil, nil
		}
		cuerpo := d[p+60 : p+60+tam]
		n := strings.TrimRight(string(h[:16]), " ")
		switch {
		case n == "/":
		case n == "//":
			largos = cuerpo
		case strings.HasPrefix(n, "/"):
			off, err := strconv.Atoi(n[1:])
			if err != nil || off >= len(largos) {
				return nil, nil
			}
			fin := bytes.Index(largos[off:], []byte("/\n"))
			if fin < 0 {
				return nil, nil
			}
			ms = append(ms, miembro{string(largos[off : off+fin]), append([]byte{}, cuerpo...)})
		default:
			ms = append(ms, miembro{strings.TrimSuffix(n, "/"), append([]byte{}, cuerpo...)})
		}
		p += 60 + tam + tam&1
	}
	return ms, nil
}

// simbolos: las definiciones globales del objeto ("S nombre Def...") en su orden.
func simbolos(rel []byte) []string {
	var s []string
	sc := bufio.NewScanner(bytes.NewReader(rel))
	sc.Buffer(make([]byte, 64*1024), 1024*1024)
	for sc.Scan() {
		c := strings.Fields(sc.Text())
		if len(c) >= 3 && c[0] == "S" && strings.HasPrefix(c[2], "Def") {
			s = append(s, c[1])
		}
	}
	return s
}

func cabecera(b *bytes.Buffer, nombre string, modo string, tam int) {
	fmt.Fprintf(b, "%-16s%-12s%-6s%-6s%-8s%-10d`\n", nombre, "0", "0", "0", modo, tam)
}

func relleno(b *bytes.Buffer, tam int) {
	if tam&1 == 1 {
		b.WriteByte('\n')
	}
}

func escribir(ruta string, ms []miembro) error {
	// tabla de nombres largos (mas de 15 caracteres con la barra final)
	var largos bytes.Buffer
	nombres := make([]string, len(ms))
	for i, m := range ms {
		if len(m.nombre)+1 > 16 {
			nombres[i] = "/" + strconv.Itoa(largos.Len())
			largos.WriteString(m.nombre + "/\n")
		} else {
			nombres[i] = m.nombre + "/"
		}
	}
	// indice: numero de simbolos, posicion de la cabecera de cada objeto, nombres
	var syms [][]string
	nsym, tamNombres := 0, 0
	for _, m := range ms {
		s := simbolos(m.datos)
		syms = append(syms, s)
		nsym += len(s)
		for _, x := range s {
			tamNombres += len(x) + 1
		}
	}
	tamIndice := 4 + 4*nsym + tamNombres
	pos := 8 + 60 + tamIndice + tamIndice&1
	if largos.Len() > 0 {
		pos += 60 + largos.Len() + largos.Len()&1
	}
	var offs []uint32
	for _, m := range ms {
		offs = append(offs, uint32(pos))
		pos += 60 + len(m.datos) + len(m.datos)&1
	}

	var b bytes.Buffer
	b.WriteString("!<arch>\n")
	cabecera(&b, "/", "0", tamIndice)
	binary.Write(&b, binary.BigEndian, uint32(nsym))
	for i, s := range syms {
		for range s {
			binary.Write(&b, binary.BigEndian, offs[i])
		}
	}
	for _, s := range syms {
		for _, x := range s {
			b.WriteString(x)
			b.WriteByte(0)
		}
	}
	relleno(&b, tamIndice)
	if largos.Len() > 0 {
		cabecera(&b, "//", "", largos.Len())
		b.Write(largos.Bytes())
		relleno(&b, largos.Len())
	}
	for i, m := range ms {
		cabecera(&b, nombres[i], "100644", len(m.datos))
		b.Write(m.datos)
		relleno(&b, len(m.datos))
	}
	tmp := ruta + ".tmp"
	if err := os.WriteFile(tmp, b.Bytes(), 0644); err != nil {
		return err
	}
	os.Remove(ruta)
	return os.Rename(tmp, ruta)
}

func main() {
	if len(os.Args) < 3 {
		fallo("uso: stcar LIBRERIA.a OBJETO.rel [OBJETO.rel...]")
	}
	if os.Args[1] == "-obj" {
		if len(os.Args) != 4 {
			fallo("uso: stcar -obj ENTRADA SALIDA")
		}
		modoObj(os.Args[2], os.Args[3])
		return
	}
	if os.Args[1] == "-x" {
		if len(os.Args) != 4 {
			fallo("uso: stcar -x LIBRERIA.a CARPETA")
		}
		ms, err := leer(os.Args[2])
		if err != nil {
			fallo("%v", err)
		}
		if err := os.MkdirAll(os.Args[3], 0755); err != nil {
			fallo("%v", err)
		}
		for _, m := range ms {
			ruta := filepath.Join(os.Args[3], m.nombre)
			if err := os.WriteFile(ruta, m.datos, 0644); err != nil {
				fallo("%v", err)
			}
			fmt.Println(ruta)
		}
		return
	}
	lib := os.Args[1]
	ms, err := leer(lib)
	if err != nil {
		fallo("%v", err)
	}
	for _, obj := range os.Args[2:] {
		d, err := os.ReadFile(obj)
		if err != nil {
			fallo("%v", err)
		}
		n := filepath.Base(strings.ReplaceAll(obj, "\\", "/"))
		hecho := false
		for i := range ms {
			if ms[i].nombre == n {
				ms[i].datos = d
				hecho = true
			}
		}
		if !hecho {
			ms = append(ms, miembro{n, d})
		}
	}
	if err := escribir(lib, ms); err != nil {
		fallo("%v", err)
	}
}
