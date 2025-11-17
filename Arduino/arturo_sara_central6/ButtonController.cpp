#include "ButtonController.h"

ButtonController::ButtonController()
{
    lastStateStart = HIGH;
    lastStateReal = HIGH;
    lastStateFalse = HIGH;
    lastDebounceStart = 0;
    lastDebounceReal = 0;
    lastDebounceFalse = 0;
}

void ButtonController::init()
{
    // Configurar botones con pull-up interno
    pinMode(BTN_START, INPUT_PULLUP);
    pinMode(BTN_REAL, INPUT_PULLUP);
    pinMode(BTN_FALSE, INPUT_PULLUP);
    
    Serial.println("Button Controller inicializado");
}

void ButtonController::update()
{
    // No hay procesamiento continuo, solo lectura on-demand
}

bool ButtonController::isStartPressed()
{
    bool currentState = digitalRead(BTN_START);
    
    // Detectar flanco descendente (HIGH a LOW)
    if (lastStateStart == HIGH && currentState == LOW)
    {
        if (millis() - lastDebounceStart > debounceDelay)
        {
            lastDebounceStart = millis();
            lastStateStart = currentState;
            Serial.println("Boton START presionado");
            return true;
        }
    }
    
    lastStateStart = currentState;
    return false;
}

bool ButtonController::isRealPressed()
{
    bool currentState = digitalRead(BTN_REAL);
    
    // Detectar flanco descendente (HIGH a LOW)
    if (lastStateReal == HIGH && currentState == LOW)
    {
        if (millis() - lastDebounceReal > debounceDelay)
        {
            lastDebounceReal = millis();
            lastStateReal = currentState;
            Serial.println("Boton REAL presionado");
            return true;
        }
    }
    
    lastStateReal = currentState;
    return false;
}

bool ButtonController::isFalsePressed()
{
    bool currentState = digitalRead(BTN_FALSE);
    
    // Detectar flanco descendente (HIGH a LOW)
    if (lastStateFalse == HIGH && currentState == LOW)
    {
        if (millis() - lastDebounceFalse > debounceDelay)
        {
            lastDebounceFalse = millis();
            lastStateFalse = currentState;
            Serial.println("Boton FALSE presionado");
            return true;
        }
    }
    
    lastStateFalse = currentState;
    return false;
}
