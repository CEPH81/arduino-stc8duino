#!/bin/sh
#
# sdar.sh - Agrega un objeto a una libreria con sdar.
#
# Uso: sdar.sh SDAR LIBRERIA.a OBJETO.o MARCA [opciones de sdar...]
#
# sdar trabaja con .rel; la libreria se copia tambien como .lib, que es
# el nombre que espera el enlazador de SDCC.
#
# El sdar de Windows a veces no sabe volver a leer la libreria que el mismo
# escribio ("file format not recognized") al agregarle objetos uno a uno. Por
# eso la libreria no se modifica: se guardan los objetos (con nombres cortos,
# hasta 15 caracteres) en una carpeta junto a ella y se crea de nuevo con todos
# en cada llamada, sin que sdar tenga que leer la anterior.
#
# Parte de stc8duino (tomado de sduino). LGPL 2.1.

if ! command -v cp > /dev/null 2>&1; then
	cd "${0%/wrapper/*}"
	PATH=$(pwd)/win:$PATH
fi

SDAR=$1
LIB=$2
OBJ=${3%.o}.rel
shift 4

# nombre corto: "wiring_analog.c.rel" -> "wiring_anal.rel"
BASE=${OBJ##*/}
BASE=${BASE##*\\}
BASE=${BASE%.rel}
BASE=${BASE%.c}
BASE=${BASE%.cpp}
CORTO=$(printf '%.11s' "$BASE").rel
DIR=${LIB%.a}_rel
mkdir -p "$DIR"
cp -f "$OBJ" "$DIR/$CORTO"

rm -f "$LIB"
# desde la carpeta de los objetos: el patron *.rel no depende de / o \ en la ruta
(cd "$DIR" && "$SDAR" "$@" "$LIB" *.rel)
ERR=$?
cp -f "$LIB" "${LIB%.a}.lib"
exit $ERR
