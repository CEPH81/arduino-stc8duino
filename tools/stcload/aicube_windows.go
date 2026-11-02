//go:build windows

package main

import (
	"os/exec"
	"syscall"
)

// lineaAiCube arma la linea de ordenes tal como la espera AiCube-ISP:
// -codefile="C:\ruta con espacios\prog.hex" (Go pondria las comillas alrededor
// de todo el argumento y AiCube no reconoceria la opcion).
func lineaAiCube(cmd *exec.Cmd, ruta, chip, hexf string) {
	cmd.SysProcAttr = &syscall.SysProcAttr{
		CmdLine: `"` + ruta + `" -port=Link1D -mcu=` + chip + ` -codefile="` + hexf + `"`,
	}
}
