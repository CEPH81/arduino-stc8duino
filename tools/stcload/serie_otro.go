//go:build !linux && !windows

package main

import (
	"errors"
	"time"
)

type puerto struct{}

func abrirPuerto(string, int, bool) (*puerto, error) { return nil, errors.New("sistema no soportado") }
func (p *puerto) velocidad(int) error                { return nil }
func (p *puerto) setParidad(bool) error              { return nil }
func (p *puerto) pendientes() int                    { return 0 }
func (p *puerto) leer([]byte, time.Duration) int     { return 0 }
func (p *puerto) escribir([]byte) error              { return nil }
func (p *puerto) vaciarSalida()                      {}
func (p *puerto) vaciarEntrada()                     {}
func (p *puerto) lineas(bool, bool)                  {}
func (p *puerto) cerrar()                            {}
