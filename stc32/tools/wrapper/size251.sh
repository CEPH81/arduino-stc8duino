#!/bin/sh
#
# size251.sh - Uso de memoria, de la linea de L251:
#   Program Size: data=8.0 edata+hdata=257 xdata=0 const=0 code=135
# Imprime "FLASH <bytes>" (code + const) y "RAM <bytes>" (edata+hdata + xdata).
#
# Parte de stc8duino. LGPL 2.1.

if ! command -v awk > /dev/null 2>&1; then
	cd "${0%/wrapper/*}"
	PATH=$(pwd)/win:$PATH
fi

awk '
{
	for (i = 1; i <= NF; i++) {
		split($i, kv, "=")
		if (kv[1] == "code" || kv[1] == "const") flash += kv[2] + 0
		if (kv[1] == "edata+hdata" || kv[1] == "xdata") ram += kv[2] + 0
	}
}
END {
	print "FLASH " flash + 0
	print "RAM " ram + 0
}' "$1"
