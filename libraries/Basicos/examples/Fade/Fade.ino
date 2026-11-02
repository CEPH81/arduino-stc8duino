/*
 * Fade - el LED sube y baja de brillo con analogWrite() (PWM por hardware).
 *
 * En la STC8H8K64U el LED esta en P3.4, que es la salida PWM8 del modulo PWMB.
 * Patas con PWM en el STC8H: PWM5 P2.0/P1.7/P0.0/P7.4, PWM6 P2.1/P5.4/P0.1/P7.5,
 * PWM7 P2.2/P3.3/P0.2/P7.6, PWM8 P2.3/P3.4/P0.3/P7.7 (un canal en una pata a la vez).
 */
#define LED LED_BUILTIN

int brillo = 0;     /* 0 a 255 */
int paso = 5;

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  analogWrite(LED, brillo);
  brillo = brillo + paso;
  if (brillo <= 0 || brillo >= 255) {
    paso = -paso;   /* cambiar de sentido en los extremos */
  }
  delay(30);
}
