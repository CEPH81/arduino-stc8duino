//go:build linux

// serie_linux.go - puerto serie para Linux: velocidad y paridad configurables,
// lectura con plazo y bytes pendientes.
package main

import (
	"fmt"
	"syscall"
	"time"
	"unsafe"
)

type puerto struct {
	fd      int
	baud    int
	paridad bool
}

var velocidades = map[int]uint32{
	1200: syscall.B1200, 2400: syscall.B2400, 4800: syscall.B4800, 9600: syscall.B9600,
	19200: syscall.B19200, 38400: syscall.B38400, 57600: syscall.B57600,
	115200: syscall.B115200, 230400: syscall.B230400, 460800: syscall.B460800,
	500000: syscall.B500000, 576000: syscall.B576000, 921600: syscall.B921600,
	1000000: syscall.B1000000,
}

func ioctl(fd int, req uintptr, arg unsafe.Pointer) error {
	_, _, e := syscall.Syscall(syscall.SYS_IOCTL, uintptr(fd), req, uintptr(arg))
	if e != 0 {
		return e
	}
	return nil
}

func ioctlVal(fd int, req, val uintptr) {
	syscall.Syscall(syscall.SYS_IOCTL, uintptr(fd), req, val)
}

func abrirPuerto(nombre string, baud int, paridad bool) (*puerto, error) {
	fd, err := syscall.Open(nombre, syscall.O_RDWR|syscall.O_NOCTTY|syscall.O_NONBLOCK, 0)
	if err != nil {
		return nil, err
	}
	p := &puerto{fd: fd}
	if err := p.configurar(baud, paridad); err != nil {
		syscall.Close(fd)
		return nil, err
	}
	return p, nil
}

func (p *puerto) configurar(baud int, paridad bool) error {
	b, ok := velocidades[baud]
	if !ok {
		return fmt.Errorf("velocidad no soportada: %d", baud)
	}
	var t syscall.Termios
	if err := ioctl(p.fd, syscall.TCGETS, unsafe.Pointer(&t)); err != nil {
		return err
	}
	t.Iflag = 0
	t.Oflag = 0
	t.Lflag = 0
	t.Cflag = syscall.CS8 | syscall.CREAD | syscall.CLOCAL | b
	if paridad {
		t.Cflag |= syscall.PARENB // par
	}
	t.Ispeed = b
	t.Ospeed = b
	t.Cc[syscall.VMIN] = 0
	t.Cc[syscall.VTIME] = 0
	if err := ioctl(p.fd, syscall.TCSETS, unsafe.Pointer(&t)); err != nil {
		return err
	}
	p.baud, p.paridad = baud, paridad
	return nil
}

func (p *puerto) velocidad(baud int) error {
	p.vaciarSalida()
	return p.configurar(baud, p.paridad)
}

func (p *puerto) setParidad(par bool) error { return p.configurar(p.baud, par) }

func (p *puerto) pendientes() int {
	var n int32
	if ioctl(p.fd, syscall.TIOCINQ, unsafe.Pointer(&n)) != nil {
		return 0
	}
	return int(n)
}

// leer lee hasta len(b) bytes; espera como mucho 'plazo' a que llegue el primero.
func (p *puerto) leer(b []byte, plazo time.Duration) int {
	var fs syscall.FdSet
	fs.Bits[p.fd/64] |= 1 << (uint(p.fd) % 64)
	tv := syscall.NsecToTimeval(plazo.Nanoseconds())
	n, err := syscall.Select(p.fd+1, &fs, nil, nil, &tv)
	if err != nil || n <= 0 {
		return 0
	}
	r, _ := syscall.Read(p.fd, b)
	if r < 0 {
		return 0
	}
	return r
}

func (p *puerto) escribir(b []byte) error {
	for len(b) > 0 {
		n, err := syscall.Write(p.fd, b)
		if err == syscall.EAGAIN {
			time.Sleep(time.Millisecond)
			continue
		}
		if err != nil {
			return err
		}
		b = b[n:]
	}
	return nil
}

func (p *puerto) vaciarSalida() {
	ioctlVal(p.fd, 0x5409, 1) // TCSBRK con 1 = tcdrain
}

func (p *puerto) vaciarEntrada() {
	ioctlVal(p.fd, 0x540B, 0) // TCFLSH, TCIFLUSH
}

func (p *puerto) lineas(dtr, rts bool) {
	var bits int32
	if ioctl(p.fd, syscall.TIOCMGET, unsafe.Pointer(&bits)) != nil {
		return
	}
	bits &^= syscall.TIOCM_DTR | syscall.TIOCM_RTS
	if dtr {
		bits |= syscall.TIOCM_DTR
	}
	if rts {
		bits |= syscall.TIOCM_RTS
	}
	ioctl(p.fd, syscall.TIOCMSET, unsafe.Pointer(&bits))
}

func (p *puerto) cerrar() { syscall.Close(p.fd) }
