/*
 * EscanerI2C - busca dispositivos en el bus I2C y muestra sus direcciones
 * por el monitor serie (115200 baudios), cada 5 s.
 *
 * Conexiones (por defecto): SCL = P1.5, SDA = P1.4, GND y VCC del modulo.
 * Para otras patas: Wire.setPins(1) P2.5/P2.4, (2) P7.7/P7.6, (3) P3.2/P3.3.
 * Direcciones tipicas: 0x27 o 0x3F pantalla LCD con PCF8574/PCF8574A,
 * 0x20-0x27 expansor PCF8574, 0x38-0x3F PCF8574A, 0x3C pantalla OLED,
 * 0x68 reloj DS3231 / MPU6050, 0x76-0x77 BMP280/BME280.
 */
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Serial.println("stc32duino: EscanerI2C");
  /* Wire.setPins(1); */
  Wire.begin();
}

void loop() {
  uint8_t direccion, error;
  uint8_t encontrados = 0;

  Serial.println("Buscando...");
  for (direccion = 1; direccion < 127; direccion++) {
    Wire.beginTransmission(direccion);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("  dispositivo en 0x");
      if (direccion < 16) Serial.print("0");
      Serial.println(direccion, HEX);
      encontrados++;
    } else if (error == 5) {
      Serial.println("  el bus no responde: revise SDA, SCL, GND y las resistencias de pull-up");
      break;
    }
  }
  if (encontrados == 0) Serial.println("  no se encontro ningun dispositivo");
  else {
    Serial.print("  total: ");
    Serial.println(encontrados);
  }
  Serial.println();
  delay(5000);
}
