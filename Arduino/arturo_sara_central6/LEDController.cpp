#include "LEDController.h"

LEDController::LEDController()
{
    currentMode = LED_MODE_CONNECTION;
    currentColor = LED_OFF;
    strobeState = false;
    lastBlinkTime = 0;
    confirmationStartTime = 0;
    isConfirmationActive = false;
    confirmationColor = LED_OFF;
    connectionState = false;
}

void LEDController::init()
{
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(STROBE_PIN, OUTPUT);
    
    turnOffRGB();
    digitalWrite(STROBE_PIN, LOW);
    
    Serial.println("LED Controller inicializado");
}

void LEDController::update()
{
    // Manejar confirmacion temporal
    if (isConfirmationActive)
    {
        if (millis() - confirmationStartTime >= LED_CONFIRMATION_MS)
        {
            isConfirmationActive = false;
            // Volver al modo actual sin confirmation
        }
        else
        {
            setColor(confirmationColor);
            return;
        }
    }
    
    // Comportamiento segun modo
    switch (currentMode)
    {
        case LED_MODE_CONNECTION:
            if (connectionState)
            {
                setColor(LED_GREEN);
            }
            else
            {
                setColor(LED_RED);
            }
            break;
            
        case LED_MODE_WARNING:
            // Parpadeo amarillo lento
            if (millis() - lastBlinkTime >= LED_BLINK_SLOW_MS)
            {
                lastBlinkTime = millis();
                if (currentColor == LED_YELLOW)
                {
                    setColor(LED_OFF);
                }
                else
                {
                    setColor(LED_YELLOW);
                }
            }
            break;
            
        case LED_MODE_ALERT:
            // Parpadeo rojo rapido
            if (millis() - lastBlinkTime >= LED_BLINK_FAST_MS)
            {
                lastBlinkTime = millis();
                if (currentColor == LED_RED)
                {
                    setColor(LED_OFF);
                }
                else
                {
                    setColor(LED_RED);
                }
            }
            break;
            
        case LED_MODE_CONFIRMATION:
            // No hacer nada, la confirmacion se maneja arriba
            break;
    }
}

void LEDController::setConnectionState(bool connected)
{
    connectionState = connected;
}

void LEDController::setWarningMode()
{
    currentMode = LED_MODE_WARNING;
    lastBlinkTime = millis();
    Serial.println("LED en modo advertencia");
}

void LEDController::setAlertMode()
{
    currentMode = LED_MODE_ALERT;
    lastBlinkTime = millis();
    Serial.println("LED en modo alerta");
}

void LEDController::showConfirmation(bool success)
{
    isConfirmationActive = true;
    confirmationStartTime = millis();
    confirmationColor = success ? LED_GREEN : LED_RED;
}

void LEDController::returnToBaseMode()
{
    currentMode = LED_MODE_CONNECTION;
    strobeState = false;
    digitalWrite(STROBE_PIN, LOW);
    Serial.println("LED retorna a modo base");
}

void LEDController::setStrobeState(bool state)
{
    strobeState = state;
    digitalWrite(STROBE_PIN, state ? HIGH : LOW);
}

void LEDController::setColor(LEDColor color)
{
    currentColor = color;
    
    switch (color)
    {
        case LED_OFF:
            turnOffRGB();
            break;
            
        case LED_RED:
            digitalWrite(LED_R, HIGH);
            digitalWrite(LED_G, LOW);
            digitalWrite(LED_B, LOW);
            break;
            
        case LED_GREEN:
            digitalWrite(LED_R, LOW);
            digitalWrite(LED_G, HIGH);
            digitalWrite(LED_B, LOW);
            break;
            
        case LED_YELLOW:
            digitalWrite(LED_R, HIGH);
            digitalWrite(LED_G, HIGH);
            digitalWrite(LED_B, LOW);
            break;
    }
}

void LEDController::turnOffRGB()
{
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
}
