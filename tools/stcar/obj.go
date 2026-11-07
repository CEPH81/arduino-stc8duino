// obj.go - sintaxis estilo Arduino (Serial.print...) para Keil C251, que solo
// compila C90 (sin _Generic ni macros con argumentos variables).
//
// Uso: stcar -obj ENTRADA SALIDA
//
// Cambia en el sketch   Objeto.metodo(a, b)   por   Objeto__metodo_N(a, b)
// donde N es el numero de argumentos (0, 1, 2...). En print, println y write,
// si el primer argumento es un texto la terminacion es _S, si es una variable
// char es _C, y si es una llamada a funcion es _L (entero), _U (millis(),
// micros()) o _F (decimales), para que la funcion se llame una sola vez
// (_L2/_U2/_F2 con un segundo argumento).
//   Serial.print("hola")   -> Serial__print_S("hola")
//   Serial.print('A')      -> Serial__print_S("A")
//   Serial.println(x, HEX) -> Serial__println_2(x, HEX)
//   Serial.println()       -> Serial__println_0()
// Los textos y los comentarios no se tocan. Las rutas de #line pasan a "/".
//
// Parte de stc8duino. LGPL 2.1.
package main

import (
	"os"
	"regexp"
	"strconv"
	"strings"
)

var objetos = map[string]bool{"Serial": true, "Wire": true, "EEPROM": true, "SPI": true}
var metodosPrint = map[string]bool{"print": true, "println": true, "write": true}

type reescritor struct {
	textos     map[string]bool // char x[], char *x, __DATE__...
	caracteres map[string]bool // char x
}

var (
	reCharPtr  = regexp.MustCompile(`char[ \t]*\*[ \t]*([A-Za-z_][A-Za-z0-9_]*)`)
	reChar     = regexp.MustCompile(`(^|[^A-Za-z0-9_])((?:un)?signed[ \t]+)?char[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*(\[?)`)
	reLlamada  = regexp.MustCompile(`[A-Za-z_][A-Za-z0-9_]*[ \t]*\(`)
	reDecimal  = regexp.MustCompile(`float|[0-9]\.[0-9]|[0-9]\.[^0-9]`)
	reSinSigno = regexp.MustCompile(`(^|[^A-Za-z0-9_])(millis|micros)[ \t]*\(`)
	reIndice   = regexp.MustCompile(`^([A-Za-z_][A-Za-z0-9_]*)[ \t]*\[[^\]]*\]$`)
	reTextoF   = regexp.MustCompile(`^F[ \t]*\([ \t]*"`)
	reLineaDir = regexp.MustCompile(`(?m)^#line.*$`)
)

func nuevoReescritor(src string) *reescritor {
	r := &reescritor{
		textos:     map[string]bool{"__DATE__": true, "__TIME__": true, "__FILE__": true},
		caracteres: map[string]bool{},
	}
	for _, m := range reCharPtr.FindAllStringSubmatch(src, -1) {
		r.textos[m[1]] = true
	}
	for _, m := range reChar.FindAllStringSubmatch(src, -1) {
		nombre := m[3]
		if nombre == "const" {
			continue
		}
		if m[4] == "[" {
			r.textos[nombre] = true
		} else if m[2] == "" {
			r.caracteres[nombre] = true
		}
	}
	return r
}

func esID(c byte) bool {
	return c == '_' || c >= '0' && c <= '9' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z'
}

// finLiteral: posicion del caracter que cierra el literal que empieza en i (" o ')
func finLiteral(s string, i int) int {
	q := s[i]
	for j := i + 1; j < len(s); j++ {
		if s[j] == '\\' {
			j++
			continue
		}
		if s[j] == q {
			return j
		}
	}
	return len(s) - 1
}

// cierre: posicion del ")" que cierra el "(" en i, o -1
func cierre(s string, i int) int {
	n := 0
	for j := i; j < len(s); j++ {
		switch s[j] {
		case '"', '\'':
			j = finLiteral(s, j)
		case '(':
			n++
		case ')':
			n--
			if n == 0 {
				return j
			}
		}
	}
	return -1
}

// argumentos del nivel superior
func argumentos(s string) []string {
	if strings.TrimSpace(s) == "" {
		return nil
	}
	var args []string
	nivel, ini := 0, 0
	for j := 0; j < len(s); j++ {
		switch s[j] {
		case '"', '\'':
			j = finLiteral(s, j)
		case '(', '[', '{':
			nivel++
		case ')', ']', '}':
			nivel--
		case ',':
			if nivel == 0 {
				args = append(args, s[ini:j])
				ini = j + 1
			}
		}
	}
	return append(args, s[ini:])
}

// 'x' -> "x"
func caracterATexto(a string) string {
	d := a[1 : len(a)-1]
	switch d {
	case `"`:
		d = `\"`
	case `\'`:
		d = `'`
	}
	return `"` + d + `"`
}

func (r *reescritor) tipo(a string) string {
	switch {
	case strings.HasPrefix(a, `"`) || reTextoF.MatchString(a):
		return "S"
	case strings.HasPrefix(a, "'"):
		return "Q"
	case r.textos[a]:
		return "S"
	case r.caracteres[a]:
		return "C"
	}
	if m := reIndice.FindStringSubmatch(a); m != nil && r.textos[m[1]] {
		return "C" // texto[i]: un caracter
	}
	if reLlamada.MatchString(a) {
		if reDecimal.MatchString(a) {
			return "F"
		}
		if reSinSigno.MatchString(a) {
			return "U" // millis(), micros(): unsigned long
		}
		return "L"
	}
	return ""
}

func (r *reescritor) proc(s string) string {
	var out strings.Builder
	i := 0
	for i < len(s) {
		c := s[i]
		switch {
		case c == '/' && i+1 < len(s) && s[i+1] == '/':
			j := strings.IndexByte(s[i:], '\n')
			if j < 0 {
				j = len(s) - i
			}
			out.WriteString(s[i : i+j])
			i += j
			continue
		case c == '/' && i+1 < len(s) && s[i+1] == '*':
			j := strings.Index(s[i+2:], "*/")
			fin := len(s)
			if j >= 0 {
				fin = i + 2 + j + 2
			}
			out.WriteString(s[i:fin])
			i = fin
			continue
		case c == '"' || c == '\'':
			j := finLiteral(s, i)
			out.WriteString(s[i : j+1])
			i = j + 1
			continue
		case esID(c):
			j := i
			for j < len(s) && esID(s[j]) {
				j++
			}
			w := s[i:j]
			if objetos[w] && (i == 0 || (s[i-1] != '.' && s[i-1] != '>')) {
				if res, fin, ok := r.llamada(s, w, j); ok {
					out.WriteString(res)
					i = fin
					continue
				}
			}
			out.WriteString(w)
			i = j
			continue
		}
		out.WriteByte(c)
		i++
	}
	return out.String()
}

// llamada: Objeto . metodo ( args ) a partir de k (despues del nombre)
func (r *reescritor) llamada(s, obj string, k int) (string, int, bool) {
	saltar := func(p int) int {
		for p < len(s) && (s[p] == ' ' || s[p] == '\t') {
			p++
		}
		return p
	}
	k = saltar(k)
	if k >= len(s) || s[k] != '.' {
		return "", 0, false
	}
	k = saltar(k + 1)
	p := k
	for p < len(s) && esID(s[p]) {
		p++
	}
	met := s[k:p]
	p = saltar(p)
	if met == "" || p >= len(s) || s[p] != '(' {
		return "", 0, false
	}
	q := cierre(s, p)
	if q < 0 {
		return "", 0, false
	}
	args := argumentos(r.proc(s[p+1 : q]))
	t := strconv.Itoa(len(args))
	if metodosPrint[met] && len(args) >= 1 {
		a := strings.TrimSpace(args[0])
		switch r.tipo(a) {
		case "Q":
			args[0] = caracterATexto(a)
			t = "S"
		case "S", "C":
			t = r.tipo(a)
		case "L", "F", "U":
			t = r.tipo(a)
			if len(args) == 2 {
				t += "2"
			}
		}
	}
	return obj + "__" + met + "_" + t + "(" + strings.Join(args, ",") + ")", q + 1, true
}

// reescribirSketch: #line con "/" y llamadas a objetos
func reescribirSketch(src string) string {
	src = reLineaDir.ReplaceAllStringFunc(src, func(l string) string {
		return strings.ReplaceAll(l, `\\`, "/")
	})
	return nuevoReescritor(src).proc(src)
}

func modoObj(entrada, salida string) {
	d, err := os.ReadFile(entrada)
	if err != nil {
		fallo("%v", err)
	}
	if err := os.WriteFile(salida, []byte(reescribirSketch(string(d))), 0644); err != nil {
		fallo("%v", err)
	}
}
