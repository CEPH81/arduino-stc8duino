# stc8duino — STC8 Core by MikrocontrollerCephs

Núcleo Arduino (IDE 2.x) para microcontroladores **STC 8051 de un ciclo por instrucción**,
compilado con **SDCC**. Hecho por [MikrocontrollerCephs](https://www.youtube.com/@mikrocontrollersceph).
Misma organización que el [sduino para IDE 2.x](https://github.com/CEPH81/sduino) (los scripts de
compilación vienen de allí). No es un producto oficial de STC.

> **Estado: en desarrollo (0.1.0).** Compila para las 5 placas y el programa de carga
> (`stcload`) está probado con un simulador del cargador de STC; falta probarlo en hardware.

| Placa | Familia | Flash | RAM (xdata) | ADC |
|---|---|---|---|---|
| STC8H8K64U (LQFP64) | STC8H | 63,5 KB | 8 KB | 12 bits |
| STC8A8K64D4 (LQFP48) | STC8A | 63,5 KB | 8 KB | 12 bits |
| STC8G1K08 (TSSOP20) | STC8G | 8 KB | 1 KB | 10 bits |
| STC15W4K56S4 (LQFP48) | STC15 | 56 KB | 3,75 KB | 10 bits |
| STC15F2K60S2 (LQFP44) | STC15 | 60 KB | 1,75 KB | 10 bits |

## Cómo se escribe

El sketch se compila como C, pero se escribe como en Arduino:

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(P5_5, OUTPUT);
}
void loop() {
  digitalToggle(P5_5);
  Serial.println(analogRead(A0));
  delay(500);
}
```

- **Patas**: con su nombre, `P1_0`, `P3_2`… o en hexadecimal `0x10`, `0x32` (puerto y bit).
- `pinMode`: `INPUT` (alta impedancia), `OUTPUT` (push-pull), `INPUT_PULLUP`,
  `OUTPUT_OPEN_DRAIN`, `QUASI_BIDIRECTIONAL` (el modo clásico del 8051).
- `Serial`: UART1, RX P3.0 / TX P3.1. `Serial.print(x)`, `println`, `write`, `read`, `available`.
- `analogRead(A0…)`: 10 bits por defecto; `analogReadResolution(12)` en STC8H/STC8A.
- `attachInterrupt(0, f, FALLING)`: INT0 (P3.2); `1` = INT1 (P3.3). `FALLING` o `CHANGE`.
- `millis`, `micros`, `delay`, `delayMicroseconds`, `shiftOut`, `shiftIn`, `pulseIn`, `random`, `map`.
- C, no C++: no hay clases ni `String`.

## Reloj y carga

El reloj es el oscilador interno: el programa de carga lo calibra a la frecuencia del menú
**Reloj** (como STC-ISP). **Reinicio automático**: si el programa usa `Serial` a la velocidad
elegida en el menú, el cargador envía `@STCISP#` y el chip entra solo en el modo de carga;
si no, hay que apagar y encender la placa cuando lo pida.

## Instalación (Gestor de placas, Arduino IDE 2.x)

1. **Archivo → Preferencias → URLs adicionales del gestor de placas**:
   ```
   https://raw.githubusercontent.com/CEPH81/arduino-stc8duino/main/package_stc8duino_index.json
   ```
2. **Herramientas → Placa → Gestor de placas**, busque **STC8 Core** e instale.
   Se descarga también SDCC 4.3 (mcs51) para Windows o Linux.
3. Las placas aparecen en **Herramientas → Placa → STC8 Core by MikrocontrollerCephs**.

## Carga (`stcload`)

`tools/stcload` es un programa en Go (Windows y Linux, sin Python) que habla con el cargador de
la ROM de STC: calibra el oscilador interno, borra, graba y conserva las opciones del chip.
Es un port del protocolo de [stcgal](https://github.com/grigorig/stcgal) (MIT) para STC15,
STC8A8K64D4/STC8H (stc8d) y STC8G (stc8g).

Menú **Programador**:

| Opción | Conexión | Cómo entra el chip en modo de carga |
|---|---|---|
| Adaptador USB-serie (CH340, CP2102…) | TX → **P3.0**, RX → **P3.1**, GND | reinicio automático (si el programa usa `Serial`) o apagar y encender |
| **STC-USB Link1D** (puerto CDC1) | S-P3.0 → **P3.0**, S-P3.1 → **P3.1**, GND (y S-Vcc si alimenta la placa) | igual que el adaptador |
| Adaptador con corte por RTS / DTR | circuito de 2 transistores que corta la alimentación | automático, siempre |

**STC-USB Link1D**: en Windows aparece como varios dispositivos; hay que instalar el driver de sus
dos puertos CDC desde la unidad de CD virtual que crea. En el IDE elija el puerto **CDC1** (el de
las patas S-P3.0/S-P3.1); el CDC2 (S-TxD/S-RxD) sirve para un segundo puerto serie. El corte de
alimentación automático del Link1D solo lo hace STC-ISP (por su interfaz HID, sin documentar):
con stc8duino se usa el reinicio automático o se apaga y enciende la placa.

## Licencias

Núcleo: LGPL 2.1. Partes tomadas de sduino (Michael Mayer, LGPL 2.1) y de Arduino.
