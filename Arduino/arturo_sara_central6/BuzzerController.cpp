#include "BuzzerController.h"

BuzzerController::BuzzerController()
{
    currentPattern = BUZZER_OFF;
    lastToneTime = 0;
    toneState = false;
    potValue = 512;
    warningCounter = 0;
}

void BuzzerController::init()
{
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    pinMode(POTENTIOMETER_PIN, INPUT);
    
    Serial.println("Buzzer Controller inicializado");
}

void BuzzerController::update()
{
    // Leer potenciometro
    readPotentiometer();
    
    // Ejecutar patron actual
    switch (currentPattern)
    {
        case BUZZER_OFF:
            stopTone();
            break;
            
        case BUZZER_WARNING:
            // Tono suave cada 3 segundos (cada 3 parpadeos del LED)
            if (millis() - lastToneTime >= 3000)
            {
                lastToneTime = millis();
                int freq = 800 + (potValue / 4);
                playTone(freq, 200);
            }
            break;
            
        case BUZZER_ALERT:
            // Rafagas rapidas de tonos agudos
            unsigned long interval = 100 + (1023 - potValue) / 4;
            
            if (millis() - lastToneTime >= interval)
            {
                lastToneTime = millis();
                
                if (toneState)
                {
                    stopTone();
                    toneState = false;
                }
                else
                {
                    int freq = 2000 + (potValue / 2);
                    playTone(freq, interval - 10);
                    toneState = true;
                }
            }
            break;
    }
}

void BuzzerController::setPattern(BuzzerPattern pattern)
{
    if (currentPattern != pattern)
    {
        currentPattern = pattern;
        lastToneTime = 0;
        toneState = false;
        warningCounter = 0;
        
        if (pattern == BUZZER_OFF)
        {
            stopTone();
        }
        
        Serial.print("Buzzer patron cambiado a: ");
        Serial.println(pattern);
    }
}

void BuzzerController::readPotentiometer()
{
    potValue = analogRead(POTENTIOMETER_PIN);
}

int BuzzerController::getPotValue()
{
    return potValue;
}

void BuzzerController::playTone(int frequency, int duration)
{
    tone(BUZZER_PIN, frequency, duration);
}

void BuzzerController::stopTone()
{
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);
}
