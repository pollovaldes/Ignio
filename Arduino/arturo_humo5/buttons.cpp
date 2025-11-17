#include "buttons.h"

// ==================== CLASE Button ====================

Button::Button(int p) 
    : pin(p), lastState(HIGH), lastChangeTime(0), debounced(false) {
}

void Button::init() {
    pinMode(pin, INPUT_PULLUP);
}

bool Button::isPressed() {
    // Retorna true solo una vez cuando detecta un flanco descendente
    if (debounced && lastState == LOW) {
        debounced = false;
        return true;
    }
    return false;
}

void Button::update() {
    int currentState = digitalRead(pin);
    unsigned long now = millis();
    
    // Detectar cambio de estado
    if (currentState != lastState) {
        lastChangeTime = now;
        debounced = false;
    }
    // Debounce: confirmar después de BUTTON_DEBOUNCE_MS sin cambios
    else if (!debounced && (now - lastChangeTime >= BUTTON_DEBOUNCE_MS)) {
        debounced = true;
        lastState = currentState;
    }
}

// ==================== CLASE Buttons ====================

Buttons::Buttons()
    : btnStart(BTN_START), btnReal(BTN_REAL), btnFalse(BTN_FALSE) {
}

void Buttons::init() {
    btnStart.init();
    btnReal.init();
    btnFalse.init();
    Serial.println("[BUTTONS] Inicializados");
}

void Buttons::update() {
    btnStart.update();
    btnReal.update();
    btnFalse.update();
}

bool Buttons::isStartPressed() {
    return btnStart.isPressed();
}

bool Buttons::isRealPressed() {
    return btnReal.isPressed();
}

bool Buttons::isFalsePressed() {
    return btnFalse.isPressed();
}
