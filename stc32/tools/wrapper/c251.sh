#!/bin/sh
#
# c251.sh - Compila un archivo con el Keil C251 con una interfaz parecida a gcc.
#
# Uso: c251.sh KEIL_C251 FUENTE OBJETO MARCA [opciones C251] [-Dx=y] [-Iruta]
#
# - Las -D pasan a DEFINE(...) y las -I a INCDIR(...).
# - El sketch (X.ino.cpp) se copia a X.ino.c: C251 compila C.
# - NOPRINT: sin listado (.lst), para no escribir en la carpeta del nucleo.
# - En Windows se ejecuta con busybox (tools/win).
#
# Parte de stc8duino. LGPL 2.1.

if ! command -v cp > /dev/null 2>&1; then
	cd "${0%/wrapper/*}"
	PATH=$(pwd)/win:$PATH
fi

KEIL=$1
SRC=$2
OBJ=$3
MARCA=$4
shift 4

if [ "$MARCA" = "asm" ]; then
	echo "c251.sh: los archivos en ensamblador aun no estan soportados: $SRC" >&2
	exit 1
fi

DEF=""
INC=""
OPC=""
for a; do
	case "$a" in
		-D*) DEF="$DEF${DEF:+,}${a#-D}" ;;
		-I*) INC="$INC${INC:+;}${a#-I}" ;;
		*)   OPC="$OPC $a" ;;
	esac
done

case "$SRC" in
	*.cpp)    CSRC="${SRC%pp}" ;;
	*.merged) CSRC="${SRC}.c" ;;
	*)        CSRC="" ;;
esac
if [ -n "$CSRC" ]; then
	# rutas de #line con "/" en lugar de "\\" (como las escribe el IDE en Windows)
	sed -e '/^#line/ s,\\\\,/,g' "$SRC" > "$CSRC"
else
	CSRC="$SRC"
fi

C251INC="$KEIL/INC"
C251LIB="$KEIL/LIB"
export C251INC C251LIB

if [ ! -e "$KEIL/BIN/C251.EXE" ] && [ ! -e "$KEIL/BIN/C251.exe" ]; then
	echo "No se encuentra el Keil C251 en $KEIL (BIN\\C251.EXE)." >&2
	echo "Si esta en otra carpeta, cree platform.local.txt en la carpeta de la plataforma con: keil.c251.path=D:/ruta/C251" >&2
	exit 1
fi

SAL="${OBJ}.txt"
# $OPC sin comillas: son las palabras de control (XSMALL INTR2 ...)
"$KEIL/BIN/C251.EXE" "$CSRC" $OPC ${DEF:+"DEFINE($DEF)"} ${INC:+"INCDIR($INC)"} "OBJECT($OBJ)" NOPRINT > "$SAL" 2>&1
ERR=$?
# C251: 0 bien, 1 avisos, 2 errores, 3 error fatal
if [ $ERR -ge 2 ] || grep -q "\*\*\* WARNING\|\*\*\* ERROR\|FATAL" "$SAL"; then
	grep -v "^C251 COMPILER\|^COPYRIGHT\|^$" "$SAL" >&2
fi
[ "$CSRC" != "$SRC" ] && rm -f "$CSRC"
if [ $ERR -ge 2 ]; then
	exit 1
fi
exit 0
