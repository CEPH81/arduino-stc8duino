/*
 * W5500_ServidorWeb - pagina web en el puerto 80 con la lectura de A0, el
 * tiempo encendido y dos enlaces para encender y apagar el LED.
 * Abra en el navegador:  http://192.168.1.177
 *
 * Conexiones: VCC 3,3 V, GND, CS -> P2.2, MISO -> P1.4, MOSI -> P1.3,
 *   SCLK -> P1.5, RST -> 3,3 V. Cambie IP, PUERTA y MASCARA segun su red.
 */
#include <SPI.h>
#include <W5500.h>

#define PIN_CS P2_2
#define SOCKET 0

uint8_t mac[6]     = { 0x02, 0x53, 0x54, 0x43, 0x00, 0x01 };
uint8_t ip[4]      = { 192, 168, 1, 177 };
uint8_t puerta[4]  = { 192, 168, 1, 1 };
uint8_t mascara[4] = { 255, 255, 255, 0 };

char peticion[40];

/* escribe un numero sin signo en texto (para la pagina) */
void enviarNumero(unsigned long v) {
  char b[11];
  uint8_t n = 10;
  b[10] = 0;
  do {
    b[--n] = '0' + (char)(v % 10);
    v /= 10;
  } while (v);
  w5500_writeStr(SOCKET, b + n);
}

void atender(void) {
  uint8_t n = 0;
  int c;
  /* primera linea: "GET /on HTTP/1.1" */
  while ((c = w5500_readByte(SOCKET)) >= 0 && c != '\n') {
    if (n < sizeof(peticion) - 1) peticion[n++] = (char)c;
  }
  peticion[n] = 0;
  while (w5500_readByte(SOCKET) >= 0)   /* el resto de la peticion */
    ;
  if (n > 7 && peticion[5] == 'o' && peticion[6] == 'n') digitalWrite(LED_BUILTIN, HIGH);
  if (n > 8 && peticion[5] == 'o' && peticion[6] == 'f') digitalWrite(LED_BUILTIN, LOW);
  Serial.print("peticion: ");
  Serial.println(peticion);

  w5500_writeStr(SOCKET, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
  w5500_writeStr(SOCKET, "<html><head><meta name=viewport content='width=device-width'>"
                         "<title>STC</title></head><body><h2>Servidor web STC</h2><p>A0 = ");
  enviarNumero(analogRead(A0));
  w5500_writeStr(SOCKET, "</p><p>Encendido hace ");
  enviarNumero(millis() / 1000);
  w5500_writeStr(SOCKET, " s</p><p>LED: ");
  w5500_writeStr(SOCKET, digitalRead(LED_BUILTIN) ? "encendido" : "apagado");
  w5500_writeStr(SOCKET, "</p><p><a href='/on'>Encender</a> | <a href='/off'>Apagar</a></p></body></html>");
  w5500_disconnect(SOCKET);
}

void setup() {
  Serial.begin(115200);
  Serial.println("W5500_ServidorWeb: http://192.168.1.177");
  pinMode(LED_BUILTIN, OUTPUT);
  SPI.begin();
  if (w5500_begin(PIN_CS, mac, ip, puerta, mascara) != 0) {
    Serial.println("el W5500 no responde: revise las conexiones");
  }
  w5500_tcpListen(SOCKET, 80);
}

void loop() {
  uint8_t st = w5500_status(SOCKET);
  if (st == W5500_ESTABLISHED) {
    if (w5500_available(SOCKET)) atender();
  } else if (st == W5500_CLOSE_WAIT) {
    w5500_disconnect(SOCKET);
  } else if (st == W5500_CLOSED) {
    w5500_tcpListen(SOCKET, 80);         /* esperar al siguiente navegador */
  }
}
