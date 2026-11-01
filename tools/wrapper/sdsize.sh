#!/bin/sh
#
# sdsize.sh - Uso de memoria de un programa enlazado con SDCC (mcs51), leido
# del archivo .mem. Imprime dos lineas que lee el IDE:
#
#   FLASH <bytes>   ROM/EPROM/FLASH
#   RAM <bytes>     EXTERNAL RAM (xdata) + RAM interna usada
#
# Parte de stc8duino. LGPL 2.1.

if ! command -v awk > /dev/null 2>&1; then
	cd "${0%/wrapper/*}"
	PATH=$(pwd)/win:$PATH
fi

awk '
/ROM\/EPROM\/FLASH/ { flash = $(NF-2) + 0 }
/EXTERNAL RAM/      { xram = $(NF-2) + 0 }
END {
	print "FLASH " flash
	print "RAM " xram
}' "$1"
