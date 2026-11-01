# stc8duino — STC8 Core by MikrocontrollerCephs

Núcleo Arduino (IDE 2.x) para microcontroladores **STC 8051 de un ciclo por instrucción**,
compilado con **SDCC**. Hecho por [MikrocontrollerCephs](https://www.youtube.com/@mikrocontrollersceph).
Misma organización que el [sduino para IDE 2.x](https://github.com/CEPH81/sduino) (los scripts de
compilación vienen de allí). No es un producto oficial de STC.

> **Estado: en desarrollo (0.1.0).** Compila para las 5 placas; falta el programa de carga
> (`stcload`) y probarlo en hardware.

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

## Licencias

Núcleo: LGPL 2.1. Partes tomadas de sduino (Michael Mayer, LGPL 2.1) y de Arduino.
