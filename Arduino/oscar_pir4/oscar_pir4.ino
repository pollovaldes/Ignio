#define LED_VERDE D5
#define LED_ROJO D6
#define PIR D2

void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(PIR, INPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO, LOW);
  Serial.begin(115200);
}

void loop() {
  int motion = digitalRead(PIR);

  if (motion == HIGH) {
    digitalWrite(LED_ROJO, LOW);
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Movimiento detectado");
  } else {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);
    Serial.println("Sin movimiento");
  }

  delay(300);
}
