#include "AlertManager.h"

AlertManager::AlertManager()
{
    apiClient = nullptr;
    ledController = nullptr;
    buzzerController = nullptr;
    sensorDataManager = nullptr;
    alertActive = false;
    alertUuid[0] = '\0';
    isManualAlert = false;
    alertStartMillis = 0;
    lastRetryTime = 0;
    alertRegisteredInServer = false;
}

void AlertManager::init(APIClient* api, LEDController* led, BuzzerController* buzzer, SensorDataManager* sensors)
{
    apiClient = api;
    ledController = led;
    buzzerController = buzzer;
    sensorDataManager = sensors;
    
    Serial.println("Alert Manager inicializado");
}

void AlertManager::startManualAlert()
{
    if (alertActive)
    {
        Serial.println("Ya hay una alerta activa, ignorando");
        return;
    }
    
    Serial.println("Iniciando alerta manual");
    
    alertActive = true;
    isManualAlert = true;
    alertStartMillis = millis();
    alertRegisteredInServer = false;
    
    // Activar señalizacion fisica inmediatamente
    activatePhysicalAlert();
    
    // Generar UUID
    generateUUID(alertUuid);
    Serial.print("UUID generado: ");
    Serial.println(alertUuid);
    
    // Intentar obtener timestamp y registrar
    String timestamp;
    if (apiClient->getServerTime(timestamp))
    {
        alertStartTimestamp = timestamp;
        alertRegisteredInServer = registerAlertInServer("manual", 0);
    }
    else
    {
        Serial.println("No se pudo obtener timestamp, se reintentara");
    }
}

void AlertManager::startAutomaticAlert(int numSensorsTriggered)
{
    if (alertActive)
    {
        Serial.println("Ya hay una alerta activa, ignorando");
        return;
    }
    
    Serial.print("Iniciando alerta automatica con ");
    Serial.print(numSensorsTriggered);
    Serial.println(" sensores");
    
    alertActive = true;
    isManualAlert = false;
    alertStartMillis = millis();
    alertRegisteredInServer = false;
    
    // Activar señalizacion fisica inmediatamente
    activatePhysicalAlert();
    
    // Generar UUID
    generateUUID(alertUuid);
    Serial.print("UUID generado: ");
    Serial.println(alertUuid);
    
    // Intentar obtener timestamp y registrar
    String timestamp;
    if (apiClient->getServerTime(timestamp))
    {
        alertStartTimestamp = timestamp;
        alertRegisteredInServer = registerAlertInServer("automatic", numSensorsTriggered);
    }
    else
    {
        Serial.println("No se pudo obtener timestamp, se reintentara");
    }
}

void AlertManager::endAlert(bool isReal)
{
    if (!alertActive)
    {
        Serial.println("No hay alerta activa para finalizar");
        return;
    }
    
    Serial.print("Finalizando alerta. Es real: ");
    Serial.println(isReal ? "SI" : "NO");
    
    // Desactivar señalizacion fisica inmediatamente
    deactivatePhysicalAlert();
    
    // Obtener timestamp final
    String timestampEnd;
    if (apiClient->getServerTime(timestampEnd))
    {
        // Calcular duracion (aproximada en segundos)
        unsigned long durationMillis = millis() - alertStartMillis;
        int durationSeconds = durationMillis / 1000;
        
        // Enviar PUT para finalizar alerta
        bool success = apiClient->putAlert(alertUuid, timestampEnd, isReal, durationSeconds);
        
        if (success)
        {
            Serial.println("Alerta finalizada en servidor");
        }
        else
        {
            Serial.println("Error finalizando alerta en servidor");
        }
    }
    else
    {
        Serial.println("No se pudo obtener timestamp final");
    }
    
    // Limpiar estado
    alertActive = false;
    alertUuid[0] = '\0';
    alertStartTimestamp = "";
    alertRegisteredInServer = false;
    
    // Limpiar datos de sensores
    sensorDataManager->clearAll();
}

bool AlertManager::isActive()
{
    return alertActive;
}

void AlertManager::update()
{
    if (!alertActive)
    {
        return;
    }
    
    // Si la alerta no esta registrada en servidor, reintentar periodicamente
    if (!alertRegisteredInServer)
    {
        if (millis() - lastRetryTime >= HTTP_RETRY_INTERVAL_MS)
        {
            lastRetryTime = millis();
            Serial.println("Reintentando registrar alerta en servidor...");
            
            String timestamp;
            if (apiClient->getServerTime(timestamp))
            {
                alertStartTimestamp = timestamp;
                const char* type = isManualAlert ? "manual" : "automatic";
                int numSensors = isManualAlert ? 0 : MIN_SENSORS_FOR_FIRE;
                alertRegisteredInServer = registerAlertInServer(type, numSensors);
            }
        }
    }
}

const char* AlertManager::getCurrentUUID()
{
    return alertUuid;
}

void AlertManager::activatePhysicalAlert()
{
    ledController->setAlertMode();
    ledController->setStrobeState(true);
    buzzerController->setPattern(BUZZER_ALERT);
    Serial.println("Señalizacion fisica de alerta activada");
}

void AlertManager::deactivatePhysicalAlert()
{
    ledController->returnToBaseMode();
    ledController->setStrobeState(false);
    buzzerController->setPattern(BUZZER_OFF);
    Serial.println("Señalizacion fisica de alerta desactivada");
}

bool AlertManager::registerAlertInServer(const char* alertType, int numSensors)
{
    return apiClient->postAlert(alertUuid, alertStartTimestamp, numSensors, alertType);
}

void AlertManager::generateUUID(char* buffer)
{
    // Generar UUID version 4 manualmente
    const char* hexChars = "0123456789abcdef";
    
    // Formato: xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // donde y es 8, 9, a, o b
    
    for (int i = 0; i < 36; i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            buffer[i] = '-';
        }
        else if (i == 14)
        {
            buffer[i] = '4';
        }
        else if (i == 19)
        {
            buffer[i] = hexChars[(random(4) + 8)];
        }
        else
        {
            buffer[i] = hexChars[random(16)];
        }
    }
    
    buffer[36] = '\0';
}
