#!/bin/sh
#
# l251.sh - Enlaza con L251 y genera el .hex con OH251.
#
# Uso: l251.sh KEIL_C251 BASE OBJETO... LIBRERIA.a
#
# - Los objetos de core.a se extraen (stcar -x) y se enlazan todos;
#   REMOVEUNUSED quita lo que no se usa.
# - Memoria como en los proyectos de STC para el STC32G: codigo desde FF0000,
#   EDATA/HDATA en 0-FFF. NOOVERLAY: las variables locales no se comparten,
#   porque attachInterrupt y timer1Begin llaman funciones por puntero desde
#   las interrupciones (el enlazador no puede ver esas llamadas).
# - Escribe BASE.hex, BASE.map y BASE.size (lo lee size251.sh).
#
# Parte de stc8duino. LGPL 2.1.

T="${0%/wrapper/*}"
if ! command -v cp > /dev/null 2>&1; then
	PATH=$T/win:$PATH
fi
case "$(uname -s 2>/dev/null)" in
	Linux*) STCAR="$T/linux/stcar" ;;
	*)      STCAR="$T/win/stcar.exe" ;;
esac

KEIL=$1
BASE=$2
shift 2

C251INC="$KEIL/INC"
C251LIB="$KEIL/LIB"
export C251INC C251LIB

DIR="${BASE}_nucleo"
rm -rf "$DIR"
LNP="${BASE}.lnp"
: > "$LNP"
PRIMERO=1
agregar() {
	if [ $PRIMERO -eq 1 ]; then
		printf '"%s"' "$1" >> "$LNP"
		PRIMERO=0
	else
		printf ',\n"%s"' "$1" >> "$LNP"
	fi
}
for a; do
	case "$a" in
		*.a)
			for o in $("$STCAR" -x "$a" "$DIR" | tr -d '\r'); do
				agregar "$o"
			done
			;;
		*) agregar "$a" ;;
	esac
done
ABS="${BASE}.abs"
{
	echo ""
	echo "TO \"$ABS\""
	echo "PRINT(\"${BASE}.map\") CASE"
	echo "REMOVEUNUSED NOOVERLAY"
	echo "CLASSES (EDATA (0x0-0xFFF),"
	echo "HDATA (0x0-0xFFF))"
} >> "$LNP"

SAL="${BASE}.l251.txt"
"$KEIL/BIN/L251.EXE" "@$LNP" > "$SAL" 2>&1
ERR=$?
grep -v "^L251 LINKER\|^COPYRIGHT\|^$" "$SAL"
# L251: 0 bien, 1 avisos, 2 errores. Un simbolo sin definir es solo aviso,
# pero el programa no funcionaria: se trata como error.
if [ $ERR -ge 2 ] || grep -q "UNRESOLVED EXTERNAL" "$SAL"; then
	echo "Error al enlazar (L251 termino con codigo $ERR)" >&2
	exit 1
fi
grep "Program Size" "$SAL" > "${BASE}.size"

"$KEIL/BIN/OH251.EXE" "$ABS" "HEXFILE(${BASE}.hex)" > "${BASE}.oh251.txt" 2>&1
ERR=$?
if [ $ERR -ge 2 ] || [ ! -e "${BASE}.hex" ]; then
	cat "${BASE}.oh251.txt" >&2
	echo "Error al crear el .hex (OH251 termino con codigo $ERR)" >&2
	exit 1
fi
exit 0
