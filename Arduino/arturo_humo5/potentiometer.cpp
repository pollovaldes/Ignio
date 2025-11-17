#include "potentiometer.h"

Potentiometer::Potentiometer() 
    : rawValue(0), mappedWarningFreq(80), mappedFireFreq(120) {
}

void Potentiometer::init() {
    // A0 es entrada analógica por defecto
    Serial.println("[POTENTIOMETER] Inicializado");
}

void Potentiometer::update() {
    rawValue = analogRead(POT);
    
    // Mapear 0-1023 a frecuencias
    // WARNING: 80-250 Hz
    mappedWarningFreq = map(rawValue, 0, 1023, BUZZER_WARNING_MIN, BUZZER_WARNING_MAX);
    
    // FIRE: 120-500 Hz
    mappedFireFreq = map(rawValue, 0, 1023, BUZZER_FIRE_MIN, BUZZER_FIRE_MAX);
}

int Potentiometer::getRawValue() {
    return rawValue;
}

int Potentiometer::getWarningFrequency() {
    return mappedWarningFreq;
}

int Potentiometer::getFireFrequency() {
    return mappedFireFreq;
}
