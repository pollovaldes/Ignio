#include "WarningManager.h"

WarningManager::WarningManager()
{
    apiClient = nullptr;
    ledController = nullptr;
    buzzerController = nullptr;
    warningActive = false;
    warningStartTime = 0;
    warningRegisteredInServer = false;
}

void WarningManager::init(APIClient* api, LEDController* led, BuzzerController* buzzer)
{
    apiClient = api;
    ledController = led;
    buzzerController = buzzer;
    
    Serial.println("Warning Manager inicializado");
}

void WarningManager::startWarning(const char* sensorType, const char* message)
{
    if (warningActive)
    {
        Serial.println("Ya hay una advertencia activa, ignorando");
        return;
    }
    
    Serial.print("Iniciando advertencia: ");
    Serial.print(sensorType);
    Serial.print(" - ");
    Serial.println(message);
    
    warningActive = true;
    warningStartTime = millis();
    warningRegisteredInServer = false;
    warningSensorType = String(sensorType);
    warningMessage = String(message);
    
    // Activar señalizacion fisica
    activatePhysicalWarning();
    
    // Intentar registrar en servidor
    bool success = apiClient->postWarning(sensorType, message);
    if (success)
    {
        warningRegisteredInServer = true;
        Serial.println("Advertencia registrada en servidor");
    }
    else
    {
        Serial.println("No se pudo registrar advertencia en servidor");
    }
}

void WarningManager::update()
{
    if (!warningActive)
    {
        return;
    }
    
    // Verificar si han pasado 15 segundos
    unsigned long elapsed = millis() - warningStartTime;
    if (elapsed >= WARNING_DURATION_MS)
    {
        Serial.println("Advertencia finalizada por timeout");
        deactivatePhysicalWarning();
        warningActive = false;
    }
}

bool WarningManager::isActive()
{
    return warningActive;
}

void WarningManager::forceEnd()
{
    if (warningActive)
    {
        Serial.println("Advertencia terminada forzadamente");
        deactivatePhysicalWarning();
        warningActive = false;
    }
}

void WarningManager::activatePhysicalWarning()
{
    ledController->setWarningMode();
    buzzerController->setPattern(BUZZER_WARNING);
    Serial.println("Señalizacion fisica de advertencia activada");
}

void WarningManager::deactivatePhysicalWarning()
{
    ledController->returnToBaseMode();
    buzzerController->setPattern(BUZZER_OFF);
    Serial.println("Señalizacion fisica de advertencia desactivada");
}
