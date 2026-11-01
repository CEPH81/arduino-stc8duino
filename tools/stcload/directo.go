// directo.go - carga sin ajustar el oscilador interno.
//
// Es la secuencia que usa la herramienta oficial STC Web ISP con los cargadores
// anteriores a la version 7.5: tras el paquete de estado se envia directamente la
// orden 0x01 (velocidad de la carga) con el ajuste que el chip ya tiene, y luego
// 0x05, 0x03, los bloques de datos y 0x82. El chip conserva su frecuencia y sus
// opciones. Implementacion propia de stc8duino a partir de la descripcion del
// protocolo.
package main

import (
	"errors"
	"fmt"
	"math"
	"time"
)

// necesitaAjuste decide, en modo auto, si hay que calibrar el oscilador.
func (s *stc) necesitaAjuste() bool {
	switch s.modo {
	case "ajuste":
		return true
	case "directo":
		return false
	}
	if s.fam == "stc15" && s.externo {
		return true
	}
	if s.relojMCU <= 0 {
		return true
	}
	return math.Abs(s.relojMCU-float64(s.trimFreq))/float64(s.trimFreq) > 0.01
}

// esperarPaquete lee paquetes hasta recibir uno que empiece por cmd (los demas se descartan).
func (s *stc) esperarPaquete(cmd byte, plazo time.Duration) ([]byte, error) {
	fin := time.Now().Add(plazo)
	for {
		resto := time.Until(fin)
		if resto <= 0 {
			return nil, errors.New("tiempo de espera agotado")
		}
		r, err := s.leerPaquete(resto)
		if err != nil {
			if err == errTrama {
				continue
			}
			return nil, err
		}
		if len(r) > 0 && r[0] == cmd {
			return r, nil
		}
		depurar("(paquete %02X descartado, se esperaba %02X)\n", firstByte(r), cmd)
	}
}

func (s *stc) grabarDirecto(datos []byte) error {
	f := s.relojMCU
	if f <= 0 {
		f = 24e6
	}
	baud := s.baud
	if s.fam == "stc15" && baud > 38400 {
		baud = 38400
	}
	if d := math.Abs(f-float64(s.trimFreq)) / float64(s.trimFreq); d > 0.01 {
		info("Aviso: el chip funciona a %.3f MHz y la placa esta configurada a %.3f MHz (elija el reloj correcto o el modo de carga \"ajustar\")\n", f/1e6, float64(s.trimFreq)/1e6)
	}
	s.trimFreq = int(f)

	// 0x01: velocidad de la carga con el ajuste actual del chip
	info("Cambiando a %d baudios (oscilador sin cambios, %.3f MHz): ", baud, f/1e6)
	var pk []byte
	if s.fam == "stc15" {
		rl := 65536 - int(math.Round((f+float64(baud)/2)/float64(baud)))
		rl2 := 65536 - int(math.Round((f+float64(baud)/2)/float64(baud)*1.5))
		pk = []byte{0x01, s.status[4], 0x40, byte(rl >> 8), byte(rl), byte(rl2 >> 8), byte(rl2), 0x81}
	} else {
		rl := 65536 - int(math.Floor((f/float64(baud)+2)/4))
		pk = []byte{0x01, s.status[4], 0x40, byte(rl >> 8), byte(rl), 0x00, 0x00, 0x97}
	}
	ok := false
	for i := 0; i < 4 && !ok; i++ {
		if i > 0 {
			depurar("0x01 sin respuesta, reintento %d\n", i)
			s.p.escribir(relleno7F(16))
			s.p.vaciarSalida()
			time.Sleep(20 * time.Millisecond)
			s.p.vaciarEntrada()
		}
		s.escribirPaquete(pk, 0)
		if _, err := s.esperarPaquete(0x01, 500*time.Millisecond); err == nil {
			ok = true
		}
	}
	if !ok {
		return errors.New("el chip no acepto la velocidad de carga (0x01)")
	}
	if err := s.p.velocidad(baud); err != nil {
		return err
	}
	info("listo\n")

	// 0x05: preparar
	pk = []byte{0x05}
	if s.bsl >= 0x72 {
		pk = []byte{0x05, 0x00, 0x00, 0x5a, 0xa5}
	}
	ok = false
	for i := 0; i < 3 && !ok; i++ {
		s.escribirPaquete(pk, 0)
		if _, err := s.esperarPaquete(0x05, 300*time.Millisecond); err == nil {
			ok = true
		} else {
			time.Sleep(40 * time.Millisecond)
		}
	}
	if !ok {
		return errors.New("el chip no respondio a la orden de preparar (0x05)")
	}

	// 0x03: borrar
	info("Borrando: ")
	pk = []byte{0x03, 0x00}
	if s.bsl >= 0x72 {
		pk = []byte{0x03, 0x00, 0x00, 0x5a, 0xa5}
	}
	s.escribirPaquete(pk, 0)
	r, err := s.esperarPaquete(0x03, 5*time.Second)
	if err != nil {
		return fmt.Errorf("al borrar: %v", err)
	}
	if len(r) >= 8 {
		s.uid = r[1:8]
	}
	info("listo\n")

	// datos
	bloque := 128
	if s.fam == "stc15" {
		bloque = 64
	}
	for len(datos)%bloque != 0 {
		datos = append(datos, 0xff)
	}
	info("Grabando %d bytes: ", len(datos))
	for i, n := 0, 0; i < len(datos); i += bloque {
		b := datos[i : i+bloque]
		vacio := true
		for _, c := range b {
			if c != 0xff {
				vacio = false
				break
			}
		}
		if vacio {
			continue // el borrado ya los deja en FF
		}
		c := byte(0x02)
		if n == 0 {
			c = 0x22
		}
		n++
		pk = append([]byte{c, byte(i >> 8), byte(i), 0x5a, 0xa5}, b...)
		ok = false
		for intento := 0; intento < 6 && !ok; intento++ {
			if intento > 0 {
				depurar("reenvio del bloque %04X\n", i)
			}
			s.escribirPaquete(pk, 0)
			r, err := s.esperarPaquete(0x02, 3*time.Second)
			if err == nil && len(r) >= 2 && r[1] == 0x54 {
				ok = true
			}
		}
		if !ok {
			return fmt.Errorf("error al grabar el bloque %04X", i)
		}
		if n%8 == 0 {
			info(".")
		}
	}
	info(" listo\n")
	return nil
}

func relleno7F(n int) []byte {
	b := make([]byte, n)
	for i := range b {
		b[i] = 0x7f
	}
	return b
}
