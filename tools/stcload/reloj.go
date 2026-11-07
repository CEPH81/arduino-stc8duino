// reloj.go - escribe en el programa la frecuencia real conseguida al ajustar el
// oscilador. El nucleo stc8duino guarda en la memoria de programa un bloque
// "STCG" / "STCF" (wiring_private.h) con la frecuencia y los valores que dependen de
// ella; init() los usa para el temporizador 0 (millis/micros) y para Serial.
package main

import "bytes"

func parchearReloj(datos []byte, freq int) bool {
	// "STCG": 15 bytes (desde 0.1.36); "STCF": 13 bytes (programas anteriores)
	largo := 15
	i := bytes.Index(datos, []byte("STCG"))
	if i < 0 {
		largo = 13
		i = bytes.Index(datos, []byte("STCF"))
	}
	if i < 0 || i+largo > len(datos) || bytes.Index(datos[i+4:], datos[i:i+4]) >= 0 || freq < 1000000 {
		return false // programa sin el bloque (otro nucleo) o ambiguo
	}
	f := uint32(freq)
	ticks := f / 1000
	if ticks == 0 || ticks > 65535 {
		return false
	}
	tpu := (f + 500000) / 1000000
	k := (65536000 + ticks/2) / ticks
	b := datos[i+4:]
	b[0], b[1], b[2], b[3] = byte(f), byte(f>>8), byte(f>>16), byte(f>>24)
	b[4], b[5] = byte(ticks), byte(ticks>>8)
	b[6] = byte(tpu)
	b[7], b[8] = byte(k), byte(k>>8)
	if largo == 15 {
		b[9] = byte(ticks / 1000)                 // delayMicroseconds: cuentas por us, parte entera
		b[10] = byte((ticks % 1000) * 256 / 1000) // y fraccion/256
	}
	return true
}
