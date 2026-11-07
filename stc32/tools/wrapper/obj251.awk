# obj251.awk - sintaxis estilo Arduino (Serial.print...) para C251, que solo
# compila C90 (sin _Generic ni macros variables).
#
# Cambia en el sketch   Objeto.metodo(a, b)   por   Objeto__metodo_N(a, b)
# donde N es el numero de argumentos (0, 1, 2...). Para print, println y
# write, si el primer argumento es un texto la terminacion es _S, si es una
# variable char es _C, y si es una llamada a funcion es _L (entero) o _F
# (con decimales) para que la funcion se llame una sola vez. Las macros de
# cada objeto (HardwareSerial.h...) eligen la funcion.
#   Serial.print("hola")   -> Serial__print_S("hola")
#   Serial.print('A')      -> Serial__print_S("A")
#   Serial.println(x, HEX) -> Serial__println_2(x, HEX)
#   Serial.println()       -> Serial__println_0()
# Los textos y comentarios no se tocan.
#
# Parte de stc8duino. LGPL 2.1.

BEGIN {
	OBJ["Serial"] = 1; OBJ["Wire"] = 1; OBJ["EEPROM"] = 1; OBJ["SPI"] = 1
	PRN["print"] = 1; PRN["println"] = 1; PRN["write"] = 1
	TXT["__DATE__"] = 1; TXT["__TIME__"] = 1; TXT["__FILE__"] = 1
	src = ""
}

{ src = src $0 "\n" }

function esid(c) { return c ~ /[A-Za-z0-9_]/ }

# declaraciones de char: texto (char x[], char *x) o caracter (char x)
function declaraciones(s,    l, n, i, lineas, r, nombre, antes) {
	n = split(s, lineas, "\n")
	for (i = 1; i <= n; i++) {
		l = lineas[i]
		while (match(l, /char[ \t]*\*[ \t]*[A-Za-z_][A-Za-z0-9_]*/)) {
			r = substr(l, RSTART, RLENGTH); sub(/char[ \t]*\*[ \t]*/, "", r)
			TXT[r] = 1
			l = substr(l, RSTART + RLENGTH)
		}
		l = lineas[i]
		while (match(l, /char[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*/)) {
			r = substr(l, RSTART, RLENGTH)
			antes = substr(l, 1, RSTART - 1)
			l = substr(l, RSTART + RLENGTH)
			nombre = r; sub(/char[ \t]+/, "", nombre); sub(/[ \t]*$/, "", nombre)
			if (substr(l, 1, 1) == "[") TXT[nombre] = 1
			else if (antes !~ /(unsigned|signed)[ \t]*$/ && nombre != "const") CHR[nombre] = 1
		}
	}
}

# posicion del caracter que cierra el literal que empieza en i (" o ')
function fin_literal(s, i,    q, j, c) {
	q = substr(s, i, 1)
	for (j = i + 1; j <= length(s); j++) {
		c = substr(s, j, 1)
		if (c == "\\") { j++; continue }
		if (c == q) return j
	}
	return length(s)
}

# posicion del ")" que cierra el "(" en i
function cierre(s, i,    n, j, c) {
	n = 0
	for (j = i; j <= length(s); j++) {
		c = substr(s, j, 1)
		if (c == "\"" || c == "'") { j = fin_literal(s, j); continue }
		if (c == "(") n++
		else if (c == ")") { n--; if (n == 0) return j }
	}
	return 0
}

# separa los argumentos del nivel superior en ARG[1..n]; devuelve n
function argumentos(s,    n, j, c, nivel, ini) {
	if (s ~ /^[ \t\n]*$/) return 0
	n = 0; nivel = 0; ini = 1
	for (j = 1; j <= length(s); j++) {
		c = substr(s, j, 1)
		if (c == "\"" || c == "'") { j = fin_literal(s, j); continue }
		if (c == "(" || c == "[" || c == "{") nivel++
		else if (c == ")" || c == "]" || c == "}") nivel--
		else if (c == "," && nivel == 0) { ARG[++n] = substr(s, ini, j - ini); ini = j + 1 }
	}
	ARG[++n] = substr(s, ini)
	return n
}

function recortar(s) { sub(/^[ \t\n]+/, "", s); sub(/[ \t\n]+$/, "", s); return s }

# 'x' -> "x"
function caracter_a_texto(a,    d) {
	d = substr(a, 2, length(a) - 2)
	if (d == "\"") d = "\\\""
	else if (d == "\\'") d = "'"
	return "\"" d "\""
}

function tipo(a,    b) {
	if (a ~ /^"/ || a ~ /^F[ \t]*\([ \t]*"/) return "S"
	if (a ~ /^'/) return "Q"
	if (a in TXT) return "S"
	b = a; sub(/[ \t]*\[.*$/, "", b)
	if (a ~ /^[A-Za-z_][A-Za-z0-9_]*[ \t]*\[[^]]*\]$/ && (b in TXT)) return "C"   # texto[i]: un caracter
	if (a in CHR) return "C"
	if (a ~ /[A-Za-z_][A-Za-z0-9_]*[ \t]*\(/) {
		if (a ~ /float|[0-9]\.[0-9]|[0-9]\.[^0-9]/) return "F"
		return "L"
	}
	return ""
}

function proc(s,    out, i, c, j, w, k, met, p, q, n, a, args, t, todo, m) {
	out = ""
	i = 1
	while (i <= length(s)) {
		c = substr(s, i, 1)
		if (c == "/" && substr(s, i + 1, 1) == "/") {
			j = index(substr(s, i), "\n"); if (j == 0) j = length(s) - i + 1
			out = out substr(s, i, j); i += j; continue
		}
		if (c == "/" && substr(s, i + 1, 1) == "*") {
			j = index(substr(s, i + 2), "*/"); if (j == 0) j = length(s) - i - 1
			out = out substr(s, i, j + 3); i += j + 3; continue
		}
		if (c == "\"" || c == "'") {
			j = fin_literal(s, i); out = out substr(s, i, j - i + 1); i = j + 1; continue
		}
		if (esid(c)) {
			j = i; while (j <= length(s) && esid(substr(s, j, 1))) j++
			w = substr(s, i, j - i)
			if ((w in OBJ) && (i == 1 || substr(s, i - 1, 1) !~ /[.>]/)) {
				k = j; while (substr(s, k, 1) ~ /[ \t]/) k++
				if (substr(s, k, 1) == ".") {
					k++; while (substr(s, k, 1) ~ /[ \t]/) k++
					p = k; while (esid(substr(s, p, 1))) p++
					met = substr(s, k, p - k)
					while (substr(s, p, 1) ~ /[ \t]/) p++
					if (met != "" && substr(s, p, 1) == "(") {
						q = cierre(s, p)
						if (q > 0) {
							todo = proc(substr(s, p + 1, q - p - 1))
							n = argumentos(todo)
							for (m = 1; m <= n; m++) A2[m] = ARG[m]
							t = n ""
							if ((met in PRN) && n >= 1) {
								a = recortar(A2[1])
								t = tipo(a)
								if (t == "Q") { A2[1] = caracter_a_texto(a); t = "S" }
								if (t == "") t = n ""
								else if (n == 2 && (t == "L" || t == "F")) t = t "2"
							}
							args = ""
							for (m = 1; m <= n; m++) args = args (m > 1 ? "," : "") A2[m]
							out = out w "__" met "_" t "(" args ")"
							i = q + 1
							continue
						}
					}
				}
			}
			out = out w; i = j; continue
		}
		out = out c; i++
	}
	return out
}

END {
	declaraciones(src)
	printf "%s", proc(src)
}
