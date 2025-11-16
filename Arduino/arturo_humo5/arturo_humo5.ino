#define LED_VERDE D4
#define LED_ROJO D5
#define SMOKE A0

void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO, LOW);
  Serial.begin(115200);
}

void loop() {
  int value = analogRead(SMOKE);

  if (value < 0 || value > 1023) {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ROJO, HIGH);
    Serial.println("Lectura invalida MQ2");
  } else {
    digitalWrite(LED_ROJO, HIGH);
    digitalWrite(LED_VERDE, HIGH);
    Serial.print("Humo: ");
    Serial.println(value);
  }

  delay(700);
}
