//go:build windows

// serie_windows.go - puerto serie para Windows: velocidad y paridad configurables,
// lectura con plazo y bytes pendientes.
package main

import (
	"strings"
	"syscall"
	"time"
	"unsafe"
)

var (
	k32                 = syscall.NewLazyDLL("kernel32.dll")
	pGetCommState       = k32.NewProc("GetCommState")
	pSetCommState       = k32.NewProc("SetCommState")
	pSetCommTimeouts    = k32.NewProc("SetCommTimeouts")
	pEscapeCommFunction = k32.NewProc("EscapeCommFunction")
	pClearCommError     = k32.NewProc("ClearCommError")
	pPurgeComm          = k32.NewProc("PurgeComm")
	pFlushFileBuffers   = k32.NewProc("FlushFileBuffers")
)

type dcb struct {
	DCBlength  uint32
	BaudRate   uint32
	Flags      uint32
	wReserved  uint16
	XonLim     uint16
	XoffLim    uint16
	ByteSize   byte
	Parity     byte
	StopBits   byte
	XonChar    byte
	XoffChar   byte
	ErrorChar  byte
	EofChar    byte
	EvtChar    byte
	wReserved1 uint16
}

type commTimeouts struct {
	ReadIntervalTimeout, ReadTotalTimeoutMultiplier, ReadTotalTimeoutConstant uint32
	WriteTotalTimeoutMultiplier, WriteTotalTimeoutConstant                    uint32
}

type comstat struct {
	Flags    uint32
	CbInQue  uint32
	CbOutQue uint32
}

type puerto struct {
	h       syscall.Handle
	baud    int
	paridad bool
	dtr     bool
	rts     bool
}

func abrirPuerto(nombre string, baud int, paridad bool) (*puerto, error) {
	if !strings.HasPrefix(nombre, `\\.\`) {
		nombre = `\\.\` + nombre
	}
	n, err := syscall.UTF16PtrFromString(nombre)
	if err != nil {
		return nil, err
	}
	h, err := syscall.CreateFile(n, syscall.GENERIC_READ|syscall.GENERIC_WRITE, 0, nil, syscall.OPEN_EXISTING, 0, 0)
	if err != nil {
		return nil, err
	}
	p := &puerto{h: h}
	if err := p.configurar(baud, paridad); err != nil {
		syscall.CloseHandle(h)
		return nil, err
	}
	return p, nil
}

func (p *puerto) configurar(baud int, paridad bool) error {
	var d dcb
	d.DCBlength = uint32(unsafe.Sizeof(d))
	if r, _, e := pGetCommState.Call(uintptr(p.h), uintptr(unsafe.Pointer(&d))); r == 0 {
		return e
	}
	d.BaudRate = uint32(baud)
	d.Flags = 1 // fBinary, sin control de flujo
	if p.dtr {
		d.Flags |= 1 << 4 // DTR_CONTROL_ENABLE
	}
	if p.rts {
		d.Flags |= 1 << 12 // RTS_CONTROL_ENABLE
	}
	d.ByteSize = 8
	d.Parity = 0
	if paridad {
		d.Parity = 2 // EVENPARITY
	}
	d.StopBits = 0
	if r, _, e := pSetCommState.Call(uintptr(p.h), uintptr(unsafe.Pointer(&d))); r == 0 {
		return e
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
	var errs uint32
	var st comstat
	pClearCommError.Call(uintptr(p.h), uintptr(unsafe.Pointer(&errs)), uintptr(unsafe.Pointer(&st)))
	return int(st.CbInQue)
}

func (p *puerto) leer(b []byte, plazo time.Duration) int {
	ms := uint32(plazo / time.Millisecond)
	if ms == 0 {
		ms = 1
	}
	t := commTimeouts{0xFFFFFFFF, 0xFFFFFFFF, ms, 0, 2000}
	pSetCommTimeouts.Call(uintptr(p.h), uintptr(unsafe.Pointer(&t)))
	var n uint32
	syscall.ReadFile(p.h, b, &n, nil)
	return int(n)
}

func (p *puerto) escribir(b []byte) error {
	for len(b) > 0 {
		var n uint32
		if err := syscall.WriteFile(p.h, b, &n, nil); err != nil {
			return err
		}
		b = b[n:]
	}
	return nil
}

func (p *puerto) vaciarSalida()  { pFlushFileBuffers.Call(uintptr(p.h)) }
func (p *puerto) vaciarEntrada() { pPurgeComm.Call(uintptr(p.h), 0x0008) } // PURGE_RXCLEAR

func (p *puerto) lineas(dtr, rts bool) {
	p.dtr, p.rts = dtr, rts
	f := func(c uintptr) { pEscapeCommFunction.Call(uintptr(p.h), c) }
	if dtr {
		f(5)
	} else {
		f(6)
	}
	if rts {
		f(3)
	} else {
		f(4)
	}
}

func (p *puerto) cerrar() { syscall.CloseHandle(p.h) }
