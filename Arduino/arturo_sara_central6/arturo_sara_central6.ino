// Pines RGB
#define LED_R D0
#define LED_G D5
#define LED_B D6

// Actuadores
#define BUZZER D3
#define STROBE D2

// Botones
#define BTN_START D1
#define BTN_REAL D7
#define BTN_FALSE D4

// Estado de flancos
bool lastStart = false;
bool lastReal = false;
bool lastFalse = false;

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(STROBE, OUTPUT);

  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(BTN_REAL, INPUT_PULLUP);
  pinMode(BTN_FALSE, INPUT_PULLUP);

  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
  digitalWrite(BUZZER, LOW);
  digitalWrite(STROBE, LOW);

  Serial.begin(115200);
  delay(300);
  Serial.println("Test CENTRAL con potenciómetro listo");
}

void loop() {
  bool startPressed = digitalRead(BTN_START) == LOW;
  bool realPressed  = digitalRead(BTN_REAL)  == LOW;
  bool falsePressed = digitalRead(BTN_FALSE) == LOW;

  // ----- Leer potenciómetro -----
  int pot = analogRead(A0);                       // 0–1023
  int buzzerVolume = map(pot, 0, 1023, 0, 200);   // Volumen seguro de noche
  Serial.print("Pot: ");
  Serial.print(pot);
  Serial.print("  Volumen mapeado: ");
  Serial.println(buzzerVolume);

  // ---- INICIAR ALERTA ----
  if (startPressed && !lastStart) {
    Serial.println(">> INICIAR ALERTA");

    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);

    digitalWrite(STROBE, HIGH);

    analogWrite(BUZZER, buzzerVolume);
    delay(80);
    analogWrite(BUZZER, 0);
  }

  // ---- FIN ALERTA REAL ----
  if (realPressed && !lastReal) {
    Serial.println(">> FIN ALERTA REAL");

    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_B, LOW);

    digitalWrite(STROBE, LOW);

    analogWrite(BUZZER, buzzerVolume);
    delay(60);
    analogWrite(BUZZER, 0);
  }

  // ---- FIN ALERTA FALSA ----
  if (falsePressed && !lastFalse) {
    Serial.println(">> FIN ALERTA FALSA");

    digitalWrite(LED_B, HIGH);
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);

    digitalWrite(STROBE, LOW);

    analogWrite(BUZZER, buzzerVolume);
    delay(60);
    analogWrite(BUZZER, 0);
  }

  // Restablecer LEDs si no hay botones presionados
  if (!startPressed && !realPressed && !falsePressed) {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
  }

  // Guardar últimos estados
  lastStart = startPressed;
  lastReal  = realPressed;
  lastFalse = falsePressed;

  delay(80);
}
