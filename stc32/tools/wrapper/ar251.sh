#!/bin/sh
#
# ar251.sh - Agrega un objeto de C251 a la libreria del nucleo (core.a).
# Usa stcar (formato ar); l251.sh extrae los objetos al enlazar.
#
# Uso: ar251.sh LIBRERIA.a OBJETO.o
#
# Parte de stc8duino. LGPL 2.1.

T="${0%/wrapper/*}"
case "$(uname -s 2>/dev/null)" in
	Linux*) STCAR="$T/linux/stcar" ;;
	*)      STCAR="$T/win/stcar.exe" ;;
esac
exec "$STCAR" "$1" "$2"
