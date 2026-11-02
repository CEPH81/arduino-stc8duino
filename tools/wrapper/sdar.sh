#!/bin/sh
#
# sdar.sh - Agrega un objeto a la libreria del nucleo.
#
# Uso: sdar.sh SDAR LIBRERIA.a OBJETO.o MARCA [opciones de sdar...]
#
# En lugar de sdar se usa stcar (tools/stcar), que escribe exactamente el mismo
# formato pero siempre en binario: el sdar de Windows rompia a veces la libreria
# ("file format not recognized", o todas las funciones del nucleo sin definir).
# La libreria se copia tambien como .lib, que es el nombre que espera el
# enlazador de SDCC.
#
# Parte de stc8duino (tomado de sduino). LGPL 2.1.

T="${0%/wrapper/*}"
LIB=$2
OBJ=${3%.o}.rel

case "$(uname -s 2>/dev/null)" in
	Linux*) STCAR="$T/linux/stcar" ;;
	*)      STCAR="$T/win/stcar.exe" ;;
esac

"$STCAR" "$LIB" "$OBJ"
ERR=$?
if [ $ERR -eq 0 ]; then
	if ! command -v cp > /dev/null 2>&1; then
		cd "$T"
		PATH=$(pwd)/win:$PATH
	fi
	cp -f "$LIB" "${LIB%.a}.lib"
fi
exit $ERR
