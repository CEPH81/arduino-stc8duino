#!/bin/sh
#
# sdcc.sh - Compila un archivo con SDCC con una interfaz parecida a gcc.
#
# Uso: sdcc.sh SDCC FUENTE OBJETO MARCA [opciones de sdcc...]
#
# - Los .cpp (solo el sketch .ino convertido por el IDE) se copian a .c y se
#   compilan como C. Se antepone STC_SKETCH y se cambia "Serial.metodo("
#   por "Serial__metodo(" (EEPROM, Wire, SPI): la sintaxis estilo Arduino
#   son macros que llaman directamente a las funciones (ver stc_obj.h).
# - SDCC genera .rel: se copia tambien como .o para que el IDE no recompile.
# - En Windows se ejecuta con busybox (tools/win).
#
# Parte de stc8duino (tomado de sduino). LGPL 2.1.

if ! command -v cp > /dev/null 2>&1; then
	# Windows: busybox no tiene cp/rm en el PATH; estan en tools/win
	cd "${0%/wrapper/*}"
	PATH=$(pwd)/win:$PATH
fi

SDCC=$1
SRC=$2
OBJ=$3
MODO=$4
shift 4

# SDCC con -o: en el preprocesado el IDE 2.3+ pide la salida en X.libsdetect.d
# y SDCC no acepta esa extension (error 119): se escribe por la salida estandar
sdcc_o() {
	if [ "$MODO" = "pre" ]; then
		case "$OBJ" in
			*.d) "$SDCC" "$@" > "$OBJ"; return $? ;;
		esac
	fi
	"$SDCC" "$@" -o "$OBJ"
}
REL=${OBJ%.o}.rel

case "$SRC" in
	*.cpp|*.merged)
		# el sketch: X.ino.cpp, o X.ino.cpp.merged (preprocesado en IDE 2.3+)
		case "$SRC" in
			*.cpp) CSRC="${SRC%pp}" ;;
			*)     CSRC="${SRC}.c" ;;
		esac
		{
			echo "#define STC_SKETCH 1"
			# referencia a main() para que se enlace main.c del core
			echo "void main(void); void (*stc_main_ref)(void) = main;"
			# En C 'x' es un int: .print('x') imprimiria el codigo numerico.
			# Se convierte en .print("x") para que se comporte como en Arduino.
			# "Serial.metodo(" -> "Serial__metodo(" (igual con Wire, EEPROM y SPI):
			# macros que llaman directamente a las funciones (stc_obj.h).
			sed -E \
				-e "s/(\.print|\.println)\([[:space:]]*'\"'[[:space:]]*\)/\1(\"\\\\\"\")/g" \
				-e "s/(\.print|\.println)\([[:space:]]*'([^'\\\\]|\\\\.)'[[:space:]]*\)/\1(\"\2\")/g" \
				-e ":objeto" \
				-e "s/(^|[^A-Za-z0-9_.])(Serial3|Serial|Wire|EEPROM|SPI)[[:space:]]*\.[[:space:]]*([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*\(/\1\2__\3(/" \
				-e "t objeto" \
				"$SRC"
		} > "$CSRC"
		sdcc_o "$@" "$CSRC"
		ERR=$?
		rm -f "$CSRC"
		;;
	*)
		sdcc_o "$@" "$SRC"
		ERR=$?
		;;
esac

# el IDE espera un .o y sdar/sdld un .rel: SDCC 4 (mcs51) escribe el .o con
# el nombre pedido y SDCC 3.9 el .rel; se deja una copia con el otro nombre.
# (en el preprocesado, OBJ es el archivo de salida y no hay .rel)
if [ $ERR -eq 0 ] && [ "$REL" != "$OBJ" ] && [ "${OBJ%.o}" != "$OBJ" ]; then
	if [ -e "$OBJ" ]; then
		cp -f "$OBJ" "$REL"
	elif [ -e "$REL" ]; then
		cp -f "$REL" "$OBJ"
	fi
fi

exit $ERR
