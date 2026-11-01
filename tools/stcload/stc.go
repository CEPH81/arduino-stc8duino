// stc.go - protocolo del cargador de la ROM de los STC15 y STC8 (portado de
// stcgal/protocols.py: Stc15Protocol, Stc8dProtocol y Stc8gProtocol).
package main

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"time"
)

var (
	inicioPaquete = []byte{0x46, 0xb9}
	desdeMCU      = byte(0x68)
	desdeHost     = byte(0x6a)
	finPaquete    = byte(0x16)
	errTrama      = errors.New("trama no valida")
)

type stc struct {
	p      *puerto
	modelo *modelo
	fam    string // stc15, stc8d, stc8g
	hs     int    // velocidad del saludo (2400)
	baud   int    // velocidad de la carga

	status      []byte
	magia       uint16
	relojMCU    float64
	bsl         byte
	externo     bool
	freqCount24 byte
	msr         []byte
	uid         []byte

	trimFreq    int     // frecuencia pedida y luego la conseguida, Hz
	trimValue   [2]byte // ajuste fino, rango
	trimDivider byte
	corte       string // "", "rts" o "dtr": linea que corta la alimentacion
}

// ---------------------------------------------------------------- paquetes

func (s *stc) leerN(n int, plazo time.Duration) ([]byte, error) {
	buf := make([]byte, 0, n)
	tmp := make([]byte, n)
	fin := time.Now().Add(plazo)
	for len(buf) < n {
		resto := time.Until(fin)
		if resto <= 0 {
			return buf, errors.New("tiempo de espera agotado")
		}
		k := s.p.leer(tmp[:n-len(buf)], resto)
		buf = append(buf, tmp[:k]...)
	}
	return buf, nil
}

func (s *stc) leerPaquete(plazo time.Duration) ([]byte, error) {
	b, err := s.leerN(1, plazo)
	if err != nil {
		return nil, err
	}
	pk := []byte{}
	if b[0] == desdeMCU {
		// algunos cargadores no envian el inicio de trama con el estado
		pk = append(pk, inicioPaquete[0], inicioPaquete[1], desdeMCU)
	} else {
		if b[0] != inicioPaquete[0] {
			return nil, errTrama
		}
		r, err := s.leerN(2, plazo)
		if err != nil {
			return nil, err
		}
		if r[0] != inicioPaquete[1] || r[1] != desdeMCU {
			return nil, errTrama
		}
		pk = append(pk, b[0], r[0], r[1])
	}
	l, err := s.leerN(2, plazo)
	if err != nil {
		return nil, err
	}
	pk = append(pk, l...)
	n := int(binary.BigEndian.Uint16(l))
	if n < 6 || n > 1024 {
		return nil, errTrama
	}
	resto, err := s.leerN(n-3, plazo)
	if err != nil {
		return nil, err
	}
	pk = append(pk, resto...)
	depurar("<- % X\n", pk)
	if pk[len(pk)-1] != finPaquete {
		return nil, errTrama
	}
	var suma uint16
	for _, c := range pk[2 : len(pk)-3] {
		suma += uint16(c)
	}
	if binary.BigEndian.Uint16(pk[len(pk)-3:len(pk)-1]) != suma {
		return nil, errors.New("suma de comprobacion del paquete")
	}
	return pk[5 : len(pk)-3], nil
}

func (s *stc) escribirPaquete(datos []byte, epilogo int) {
	pk := append([]byte{}, inicioPaquete...)
	pk = append(pk, desdeHost)
	pk = binary.BigEndian.AppendUint16(pk, uint16(len(datos)+6))
	pk = append(pk, datos...)
	var suma uint16
	for _, c := range pk[2:] {
		suma += uint16(c)
	}
	pk = binary.BigEndian.AppendUint16(pk, suma)
	pk = append(pk, finPaquete)
	for i := 0; i < epilogo; i++ {
		pk = append(pk, 0x66)
	}
	depurar("-> % X\n", pk)
	s.p.escribir(pk)
	s.p.vaciarSalida()
}

// pulso envia c cada 30 ms hasta que el chip responde.
func (s *stc) pulso(c byte, plazo time.Duration) error {
	fin := time.Now().Add(plazo)
	for {
		s.p.escribir([]byte{c})
		s.p.vaciarSalida()
		time.Sleep(30 * time.Millisecond)
		if s.p.pendientes() > 0 {
			return nil
		}
		if plazo > 0 && time.Now().After(fin) {
			return errors.New("el chip no responde")
		}
	}
}

func (s *stc) pedir(datos []byte, epilogo int, magia byte) ([]byte, error) {
	s.escribirPaquete(datos, epilogo)
	r, err := s.leerPaquete(15 * time.Second)
	if err != nil {
		return nil, err
	}
	if len(r) < 1 || r[0] != magia {
		if len(r) == 1 && r[0] == 0x0f {
			return nil, errors.New("el chip esta bloqueado")
		}
		return nil, fmt.Errorf("respuesta inesperada del chip (%02X, se esperaba %02X)", firstByte(r), magia)
	}
	return r, nil
}

func firstByte(b []byte) byte {
	if len(b) == 0 {
		return 0
	}
	return b[0]
}

// ---------------------------------------------------------------- conexion

func (s *stc) conectar(nombre string, resetBaud int, espera time.Duration) error {
	ini := s.hs
	if resetBaud > 0 {
		ini = resetBaud
	}
	p, err := abrirPuerto(nombre, ini, false)
	if err != nil {
		return fmt.Errorf("no se puede abrir %s: %v", nombre, err)
	}
	s.p = p
	p.lineas(false, false)

	reiniciar := func() {
		if resetBaud <= 0 {
			return
		}
		p.velocidad(resetBaud)
		p.setParidad(false)
		p.escribir([]byte("@STCISP#"))
		p.vaciarSalida()
		p.setParidad(true)
		p.velocidad(s.hs)
	}
	reiniciar()
	p.setParidad(true)
	p.velocidad(s.hs)
	p.vaciarEntrada()

	if s.corte != "" {
		// como stcgal -a: linea activa = alimentacion cortada
		info("Apagando y encendiendo la placa (%s): ", s.corte)
		p.lineas(s.corte == "dtr", s.corte == "rts")
		time.Sleep(500 * time.Millisecond)
		p.lineas(false, false)
		time.Sleep(30 * time.Millisecond)
		info("listo\n")
	}

	if s.corte != "" {
		info("Esperando al chip: ")
	} else if resetBaud > 0 {
		info("Esperando al chip (reinicio automatico a %d baudios; si no responde, apague y encienda la placa): ", resetBaud)
	} else {
		info("Esperando al chip: apague y encienda la placa... ")
	}
	inicio := time.Now()
	ultimo := inicio
	for {
		if time.Since(inicio) > espera {
			info("\n")
			return errors.New("el chip no respondio: compruebe el puerto, las conexiones TX/RX y GND, y apague y encienda la placa")
		}
		if resetBaud > 0 && time.Since(ultimo) > 2*time.Second {
			ultimo = time.Now()
			reiniciar()
		}
		if s.pulso(0x7f, 500*time.Millisecond) != nil {
			continue
		}
		st, err := s.leerEstado()
		if err == nil && len(st) >= 23 {
			s.status = st
			break
		}
	}
	info("conectado\n")
	return s.iniciar()
}

func (s *stc) leerEstado() ([]byte, error) {
	st, err := s.leerPaquete(500 * time.Millisecond)
	if err != nil {
		return nil, err
	}
	if st[0] == 0x80 {
		s.escribirPaquete([]byte{0x80}, 0)
		s.pulso(0x7f, time.Second)
		if st, err = s.leerPaquete(500 * time.Millisecond); err != nil {
			return nil, err
		}
	}
	if st[0] != 0x50 {
		return nil, errTrama
	}
	return st, nil
}

func be32(b []byte) uint32 { return binary.BigEndian.Uint32(b) }
func be16(b []byte) uint16 { return binary.BigEndian.Uint16(b) }

func (s *stc) iniciar() error {
	st := s.status
	s.magia = be16(st[20:22])
	ok := false
	for _, m := range s.modelo.magias {
		if m == s.magia {
			ok = true
		}
	}
	if !ok {
		return fmt.Errorf("el chip conectado (codigo %04X) no es un %s: elija la placa correcta", s.magia, s.modelo.nombre)
	}
	s.bsl = st[17]
	if s.fam == "stc15" {
		if len(st) < 38 {
			return errors.New("paquete de estado corto")
		}
		s.externo = st[7]&0x01 == 0
		if s.externo {
			s.relojMCU = float64(s.hs) * float64(be16(st[13:15]))
		} else if v := be32(st[8:12]); v != 0xffffffff {
			s.relojMCU = float64(v)
		}
		s.freqCount24 = st[4]
		s.msr = append(append(append([]byte{}, st[5:8]...), st[12]), st[37])
	} else {
		if len(st) < 40 {
			return errors.New("paquete de estado corto")
		}
		if v := be32(st[1:5]); v != 0xffffffff {
			s.relojMCU = float64(v)
		}
		s.msr = append(append([]byte{}, st[9:12]...), st[15:17]...)
	}
	info("Chip: %s, cargador %d.%d%c, reloj actual %.3f MHz\n", s.modelo.nombre, s.bsl>>4, s.bsl&0x0f, st[18], s.relojMCU/1e6)
	return nil
}

// ---------------------------------------------------------------- calibracion

func iapDelay(f float64) byte {
	switch {
	case f < 1e6:
		return 0x87
	case f < 2e6:
		return 0x86
	case f < 3e6:
		return 0x85
	case f < 6e6:
		return 0x84
	case f < 12e6:
		return 0x83
	case f < 20e6:
		return 0x82
	case f < 24e6:
		return 0x81
	}
	return 0x80
}

func pyRound(x float64) int { return int(math.RoundToEven(x)) }

type ajuste struct {
	trim, rango int
}

// rango15/ajuste15: stcgal Stc15Protocol.choose_range / choose_trim
func rango15(pk, r []byte, objetivo int) *ajuste {
	cal, reto, n := r[2:], pk[2:], int(r[1])
	if len(cal) < 2*n {
		return nil
	}
	for i := 0; i < n-1; i++ {
		ca, cb := int(be16(cal[2*i:])), int(be16(cal[2*i+2:]))
		ta, tb, tr := int(reto[2*i]), int(reto[2*i+2]), int(reto[2*i+3])
		if (ca <= objetivo && cb >= objetivo) || (cb <= objetivo && ca >= objetivo) {
			if cb == ca {
				continue
			}
			m := float64(tb-ta) / float64(cb-ca)
			b := float64(ta) - m*float64(ca)
			t := pyRound(m*float64(objetivo) + b)
			if t > 65536 || t < 0 {
				return nil
			}
			return &ajuste{t, tr}
		}
	}
	return nil
}

func mejorAjuste(pk, r []byte, objetivo int) (*ajuste, int) {
	cal, reto, n := r[2:], pk[2:], int(r[1])
	if len(cal) < 2*n {
		return nil, 0
	}
	var mejor *ajuste
	cuenta, dif := 0, math.MaxInt32
	for i := 0; i < n; i++ {
		c := int(be16(cal[2*i:]))
		d := c - objetivo
		if d < 0 {
			d = -d
		}
		if d < dif {
			dif = d
			mejor = &ajuste{int(reto[2*i]), int(reto[2*i+1])}
			cuenta = c
		}
	}
	return mejor, cuenta
}

// rango8d: stcgal Stc8dProtocol.choose_range
func rango8d(pk, r []byte, objetivo int) (*ajuste, error) {
	cal, reto, n := r[2:], pk[2:], int(r[1])
	if len(cal) < 2*n {
		return nil, errors.New("faltan datos de calibracion")
	}
	for i := 0; i < n>>1; i++ {
		ca, cb := int(be16(cal[4*i:])), int(be16(cal[4*i+2:]))
		ta, tb, tr := int(reto[4*i]), int(reto[4*i+2]), int(reto[4*i+3])
		if ca <= objetivo && cb >= objetivo {
			if cb == ca {
				continue
			}
			t := pyRound(float64(objetivo-ca)*float64(tb-ta)/float64(cb-ca) + float64(ta))
			if t < 6 || t > 255-5 {
				return nil, errors.New("no se pudo ajustar la frecuencia")
			}
			return &ajuste{t, tr}, nil
		}
	}
	return nil, nil
}

func (s *stc) calibrar15() error {
	if s.relojMCU == 0 && s.trimFreq <= 0 {
		return errors.New("chip sin calibrar: indique la frecuencia")
	}
	usuario := float64(s.trimFreq)
	programa := 22118400.0
	objU := pyRound(usuario / (float64(s.hs) / 2))
	objP := pyRound(programa / (float64(s.hs) / 2))

	info("Ajustando el oscilador: ")
	pk := []byte{0x00, 12,
		0x00, 0xc0, 0x80, 0xc0, 0xff, 0xc0,
		0x00, 0x80, 0x80, 0x80, 0xff, 0x80,
		0x00, 0x40, 0x80, 0x40, 0xff, 0x40,
		0x00, 0x00, 0x80, 0x00, 0xc0, 0x00}
	r, err := s.calibracion(pk, 0)
	if err != nil {
		return err
	}
	u := rango15(pk, r, objU)
	pr := rango15(pk, r, objP)
	if u == nil || pr == nil {
		return errors.New("no se pudo ajustar la frecuencia")
	}
	pk = []byte{0x00, 12}
	for i := u.trim - 3; i < u.trim+3; i++ {
		pk = append(pk, byte(i), byte(u.rango))
	}
	for i := pr.trim - 3; i < pr.trim+3; i++ {
		pk = append(pk, byte(i), byte(pr.rango))
	}
	if r, err = s.calibracion(pk, 0); err != nil {
		return err
	}
	ua, uc := mejorAjuste(pk, r, objU)
	pa, _ := mejorAjuste(pk, r, objP)
	if ua == nil || pa == nil {
		return errors.New("no se pudo ajustar la frecuencia")
	}
	s.trimValue = [2]byte{byte(ua.trim), byte(ua.rango)}
	s.trimFreq = pyRound(float64(uc) * float64(s.hs) / 2)
	info("%.3f MHz\n", float64(s.trimFreq)/1e6)

	info("Cambiando a %d baudios: ", s.baud)
	pk = []byte{0x01, byte(pa.trim), byte(pa.rango)}
	if s.magia>>8 == 0xf2 {
		pk = binary.BigEndian.AppendUint16(pk, uint16(65536-programa/float64(s.baud)))
		pk = binary.BigEndian.AppendUint16(pk, uint16(65536-programa/2*3/float64(s.baud)))
	} else {
		pk = binary.BigEndian.AppendUint16(pk, uint16(65536-programa/float64(s.baud*4)))
		pk = append(pk, byte(ua.rango), byte(ua.trim))
	}
	pk = append(pk, iapDelay(programa))
	if _, err := s.pedir(pk, 0, 0x01); err != nil {
		return err
	}
	time.Sleep(100 * time.Millisecond)
	return s.p.velocidad(s.baud)
}

func (s *stc) calibracion(pk []byte, epilogo int) ([]byte, error) {
	s.escribirPaquete(pk, epilogo)
	if err := s.pulso(0xfe, time.Second); err != nil {
		return nil, err
	}
	r, err := s.leerPaquete(15 * time.Second)
	if err != nil {
		return nil, err
	}
	if len(r) < 2 || r[0] != 0x00 {
		return nil, errors.New("respuesta inesperada en la calibracion")
	}
	return r, nil
}

func (s *stc) calibrar8() error {
	if s.relojMCU == 0 && s.trimFreq <= 0 {
		return errors.New("chip sin calibrar: indique la frecuencia")
	}
	usuario := float64(s.trimFreq)
	obj := pyRound(usuario / float64(s.hs))
	g := s.fam == "stc8g" && s.trimFreq >= 27360000

	info("Ajustando el oscilador a %.3f MHz: ", usuario/1e6)
	var pk []byte
	ep1, ep2 := 0, 0
	if g {
		pk = []byte{0x00, 0x05, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0xff, 0x00}
		ep1, ep2 = 12, 19
	} else {
		pk = []byte{0x00, 0x08,
			0x00, 0x00, 0xff, 0x00,
			0x00, 0x10, 0xff, 0x10,
			0x00, 0x20, 0xff, 0x20,
			0x00, 0x30, 0xff, 0x30}
	}
	r, err := s.calibracion(pk, ep1)
	if err != nil {
		return err
	}
	var u *ajuste
	for d := 1; d <= 5; d++ {
		if u, err = rango8d(pk, r, obj*d); err != nil {
			return err
		}
		if u != nil {
			s.trimDivider = byte(d)
			break
		}
	}
	if u == nil {
		return errors.New("no se pudo ajustar la frecuencia")
	}
	pk = []byte{0x00, 0x0c}
	for i := -6; i < 6; i++ {
		pk = append(pk, byte(u.trim+i), byte(u.rango))
	}
	if r, err = s.calibracion(pk, ep2); err != nil {
		return err
	}
	a, c := mejorAjuste(pk, r, obj*int(s.trimDivider))
	if a == nil {
		return errors.New("no se pudo ajustar la frecuencia")
	}
	s.trimValue = [2]byte{byte(a.trim), byte(a.rango)}
	s.trimFreq = pyRound(float64(c) * float64(s.hs) / float64(s.trimDivider))
	info("%.3f MHz (%+.2f %%)\n", float64(s.trimFreq)/1e6, float64(s.trimFreq)*100/usuario-100)

	info("Cambiando a %d baudios: ", s.baud)
	pk = []byte{0x01, 0x00, 0x00}
	pk = binary.BigEndian.AppendUint16(pk, uint16(pyRound(65536-24e6/float64(s.baud*4))))
	pk = append(pk, byte(a.rango), byte(a.trim), 0x98)
	if _, err := s.pedir(pk, 0, 0x01); err != nil {
		return err
	}
	return s.p.velocidad(s.baud)
}

func (s *stc) baudExterno() error {
	info("Reloj externo. Cambiando a %d baudios: ", s.baud)
	pk := []byte{0x01, s.freqCount24, 0x40}
	b := int(65536 - s.relojMCU/float64(s.baud)/4)
	if b >= 65536 {
		return errors.New("no se puede usar esa velocidad")
	}
	pk = binary.BigEndian.AppendUint16(pk, uint16(b))
	pk = append(pk, 0x00, 0x00, iapDelay(s.relojMCU))
	if _, err := s.pedir(pk, 0, 0x01); err != nil {
		return err
	}
	time.Sleep(100 * time.Millisecond)
	s.trimValue = [2]byte{s.freqCount24, 0x40}
	s.trimFreq = 24000000
	return s.p.velocidad(s.baud)
}

// ---------------------------------------------------------------- grabacion

func (s *stc) clave() []byte {
	if s.bsl >= 0x72 {
		return []byte{0x5a, 0xa5}
	}
	return nil
}

func (s *stc) grabar(datos []byte) error {
	var err error
	switch {
	case s.fam == "stc15" && s.externo:
		err = s.baudExterno()
	case s.fam == "stc15":
		err = s.calibrar15()
	default:
		err = s.calibrar8()
	}
	if err != nil {
		return err
	}

	pk := []byte{0x05}
	if s.bsl >= 0x72 {
		pk = append(pk, 0x00, 0x00, 0x5a, 0xa5)
	}
	if _, err := s.pedir(pk, 0, 0x05); err != nil {
		return err
	}
	info("listo\n")

	info("Borrando: ")
	pk = append([]byte{0x03, 0x00}, func() []byte {
		if s.bsl >= 0x72 {
			return []byte{0x00, 0x5a, 0xa5}
		}
		return nil
	}()...)
	r, err := s.pedir(pk, 0, 0x03)
	if err != nil {
		return err
	}
	if len(r) >= 8 {
		s.uid = r[1:8]
	}
	info("listo\n")

	const bloque = 64
	info("Grabando %d bytes: ", len(datos))
	for i := 0; i < len(datos); i += bloque {
		c := byte(0x02)
		if i == 0 {
			c = 0x22
		}
		pk = []byte{c}
		pk = binary.BigEndian.AppendUint16(pk, uint16(i))
		pk = append(pk, s.clave()...)
		pk = append(pk, datos[i:i+bloque]...)
		r, err := s.pedir(pk, 0, 0x02)
		if err != nil {
			return err
		}
		if len(r) < 2 || r[1] != 0x54 {
			return errors.New("error al grabar")
		}
		if (i/bloque)%16 == 15 {
			info(".")
		}
	}
	info(" listo\n")
	if s.bsl >= 0x72 {
		r, err := s.pedir([]byte{0x07, 0x00, 0x00, 0x5a, 0xa5}, 0, 0x07)
		if err != nil {
			return err
		}
		if len(r) < 2 || r[1] != 0x54 {
			return errors.New("error al terminar la grabacion")
		}
	}

	info("Opciones: ")
	pk = append([]byte{0x04, 0x00, 0x00}, s.clave()...)
	if s.fam == "stc15" {
		pk = append(pk, s.opciones15()...)
	} else {
		pk = append(pk, s.opciones8()...)
	}
	r, err = s.pedir(pk, 0, 0x04)
	if err != nil {
		return err
	}
	if len(r) < 2 || r[1] != 0x54 {
		return errors.New("error al escribir las opciones")
	}
	info("listo\n")
	if len(s.uid) > 0 {
		depurar("UID: % X\n", s.uid)
	}
	return nil
}

// opciones15: stcgal Stc15Protocol.build_options (las opciones del chip no cambian)
func (s *stc) opciones15() []byte {
	m := s.msr
	f := uint32(s.trimFreq)
	pk := relleno(23)
	pk = append(pk, byte(f>>24), 0xff, byte(f>>16), 0xff, byte(f>>8), 0xff, byte(f), 0xff)
	pk = append(pk, m[3])
	pk = append(pk, relleno(23)...)
	if len(m) > 4 {
		pk = append(pk, m[4])
	} else {
		pk = append(pk, 0xff)
	}
	pk = append(pk, relleno(3)...)
	pk = append(pk, s.trimValue[0], s.trimValue[1]+0x3f)
	pk = append(pk, m[0:3]...)
	return pk
}

// opciones8: stcgal Stc8dProtocol.build_options
func (s *stc) opciones8() []byte {
	m := s.msr
	pk := relleno(40)
	pk[3], pk[6], pk[22] = 0, 0, 0
	binary.BigEndian.PutUint32(pk[24:28], uint32(s.trimFreq))
	pk[28], pk[29] = s.trimValue[0], s.trimValue[1]
	pk[30] = s.trimDivider
	pk[32] = m[0]
	copy(pk[36:40], m[1:5])
	return pk
}

func (s *stc) desconectar() {
	if s.p == nil {
		return
	}
	if s.fam == "stc15" {
		s.escribirPaquete([]byte{0x82}, 0)
	} else {
		s.escribirPaquete([]byte{0xff}, 0)
	}
	time.Sleep(50 * time.Millisecond)
}
