#!/bin/sh
#
# pre.sh - Preprocesa con SDCC para el IDE (deteccion de librerias y
# prototipos de funciones). C251 no tiene -E ni -M.
#
# Uso: pre.sh SDCC FUENTE SALIDA [opciones de sdcc...]
#
# Parte de stc8duino. LGPL 2.1.

if ! command -v cp > /dev/null 2>&1; then
	cd "${0%/wrapper/*}"
	PATH=$(pwd)/win:$PATH
fi

SDCC=$1
SRC=$2
OUT=$3
shift 3

case "$SRC" in
	*.cpp)    CSRC="${SRC%pp}"; cp -f "$SRC" "$CSRC" ;;
	*.merged) CSRC="${SRC}.c";  cp -f "$SRC" "$CSRC" ;;
	*)        CSRC="$SRC" ;;
esac
# el IDE 2.3+ pide la salida en X.libsdetect.d y SDCC no acepta esa extension
case "$OUT" in
	*.d) "$SDCC" "$@" "$CSRC" > "$OUT" ;;
	*)   "$SDCC" "$@" "$CSRC" -o "$OUT" ;;
esac
ERR=$?
[ "$CSRC" != "$SRC" ] && rm -f "$CSRC"
exit $ERR
