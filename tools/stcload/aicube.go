// aicube.go - carga con el programa oficial AiCube-ISP de STC (modo linea de comandos)
// y el programador STC-USB Link1D por HID: AiCube controla la alimentacion del chip.
// AiCube no se incluye en el paquete: hay que descargarlo de www.stcai.com.
package main

import (
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"sort"
)

func buscarAiCube() string {
	if r := os.Getenv("STC8DUINO_AICUBE"); r != "" {
		return r
	}
	dirs := []string{}
	if exe, err := os.Executable(); err == nil {
		dirs = append(dirs, filepath.Dir(exe))
	}
	if h, err := os.UserHomeDir(); err == nil {
		for _, d := range []string{"Desktop", "Escritorio", "Downloads", "Descargas", "Documents", "Documentos", "OneDrive/Desktop", "OneDrive/Escritorio"} {
			dirs = append(dirs, filepath.Join(h, d))
		}
	}
	dirs = append(dirs, `C:\STC`, `C:\AiCube`, `C:\Program Files\STC`, `C:\Program Files (x86)\STC`)
	for _, d := range dirs {
		for _, patron := range []string{"AiCube-ISP*.exe", "AiCube*/AiCube-ISP*.exe", "AIapp-ISP*.exe", "AIapp*/AIapp-ISP*.exe"} {
			m, _ := filepath.Glob(filepath.Join(d, patron))
			if len(m) > 0 {
				sort.Strings(m)
				return m[len(m)-1] // la version mas reciente
			}
		}
	}
	return ""
}

func cargarAiCube(ruta, chip, hexf string) {
	if runtime.GOOS != "windows" {
		fail("AiCube-ISP solo funciona en Windows: use el programador \"STC-USB Link1D (puerto CDC)\"")
	}
	if chip == "" || hexf == "" {
		fail("faltan -chip o -hex")
	}
	if ruta == "" {
		ruta = buscarAiCube()
	}
	if ruta == "" {
		fail("no se encontro AiCube-ISP*.exe. Descarguelo de www.stcai.com y dejelo en el Escritorio, en Descargas o en C:\\STC, o indique su ruta en la variable de entorno STC8DUINO_AICUBE")
	}
	info("Cargando con %s por el Link1D (HID).\n", ruta)
	info("Si no arranca solo, apague y encienda la placa.\n")
	cmd := exec.Command(ruta, "-port=Link1D", "-mcu="+chip, "-codefile="+hexf)
	cmd.Stdout, cmd.Stderr, cmd.Stdin = os.Stdout, os.Stderr, os.Stdin
	if err := cmd.Run(); err != nil {
		fail("AiCube-ISP termino con error: %v", err)
	}
	info("AiCube-ISP termino.\n")
}
