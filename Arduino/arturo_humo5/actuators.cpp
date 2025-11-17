#include "actuators.h"

Actuators::Actuators() 
    : strobeStartTime(0), strobeState(false), lastBuzzerErrorTime(0), 
      buzzerErrorPending(false), errorBuzzerDelay(ERROR_BUZZER_MIN_DELAY) {
}

void Actuators::init() {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(STROBE, OUTPUT);
    
    // Apagar todo
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(STROBE, LOW);
    
    Serial.println("[ACTUATORS] Inicializado");
}

// ==================== LED RGB ====================

void Actuators::setLEDRed() {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
}

void Actuators::setLEDGreen() {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
}

void Actuators::setLEDYellow() {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
}

void Actuators::setLEDOff() {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
}

void Actuators::flashGreen() {
    Serial.println("[ACTUATORS] Flash VERDE (éxito)");
    setLEDGreen();
    delay(FLASH_DURATION_MS);
    setLEDGreen();  // Vuelve a verde (asume que estaba en NORMAL)
}

void Actuators::flashRed() {
    Serial.println("[ACTUATORS] Flash ROJO (error)");
    setLEDRed();
    delay(FLASH_DURATION_MS);
    setLEDRed();  // Vuelve a rojo (asume que estaba en NO_WIFI o error)
    
    // Trigger buzzer error
    buzzerErrorPending = true;
    lastBuzzerErrorTime = millis();
    errorBuzzerDelay = ERROR_BUZZER_MIN_DELAY;
}

// ==================== BUZZER ====================

void Actuators::buzzerTone(int frequency, int durationMs) {
    // Usar tone() de Arduino para generar la frecuencia
    tone(BUZZER, frequency, durationMs);
}

void Actuators::warningBeep(int frequency) {
    // Bip corto para warning
    buzzerTone(frequency, 200);  // 200ms
}

void Actuators::fireBuzzerChaos(int frequency) {
    // Patrón caótico: variar frecuencia aleatoriamente
    int randomFreq = frequency + random(-100, 100);
    randomFreq = constrain(randomFreq, BUZZER_FIRE_MIN, BUZZER_FIRE_MAX);
    
    // Duración aleatoria corta
    int duration = random(100, 300);
    buzzerTone(randomFreq, duration);
}

void Actuators::click() {
    // Mini beep
    buzzerTone(800, 50);
}

void Actuators::errorBuzzer() {
    // Bip leve ocasional, sin bloquear
    // Se ejecuta en update()
}

// ==================== ESTROBO ====================

void Actuators::strobeStart() {
    strobeStartTime = millis();
    strobeState = false;
    Serial.println("[ACTUATORS] Estrobo iniciado");
}

void Actuators::strobeStop() {
    digitalWrite(STROBE, LOW);
    strobeState = false;
    Serial.println("[ACTUATORS] Estrobo detenido");
}

void Actuators::strobeUpdate() {
    if (strobeStartTime == 0) return;  // No está activo
    
    unsigned long elapsed = millis() - strobeStartTime;
    unsigned long cyclePeriodMs = 1000 / STROBE_FREQ_HZ;  // Período en ms
    unsigned long phaseMs = elapsed % cyclePeriodMs;
    
    // 50% encendido, 50% apagado
    bool shouldBeOn = phaseMs < (cyclePeriodMs / 2);
    
    if (shouldBeOn != strobeState) {
        strobeState = shouldBeOn;
        digitalWrite(STROBE, strobeState ? HIGH : LOW);
    }
}

// ==================== UTILIDADES ====================

void Actuators::allOff() {
    setLEDOff();
    digitalWrite(BUZZER, LOW);
    noTone(BUZZER);
    strobeStop();
    Serial.println("[ACTUATORS] Todo apagado");
}

void Actuators::update() {
    // Actualizar estrobo
    strobeUpdate();
    
    // Actualizar buzzer de error (repetido con delay creciente)
    if (buzzerErrorPending) {
        unsigned long now = millis();
        if (now - lastBuzzerErrorTime >= errorBuzzerDelay) {
            // Sonar buzzer leve
            buzzerTone(600, 100);
            Serial.println("[ACTUATORS] Buzzer de error");
            
            // Incrementar delay para próxima vez
            errorBuzzerDelay = min(errorBuzzerDelay + 1000, (int)ERROR_BUZZER_MAX_DELAY);
            lastBuzzerErrorTime = now;
            
            // Después de varios intentos, desactivar
            if (errorBuzzerDelay > ERROR_BUZZER_MAX_DELAY) {
                buzzerErrorPending = false;
                Serial.println("[ACTUATORS] Buzzer de error desactivado");
            }
        }
    }
}
