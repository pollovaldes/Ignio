#include "StateMachine.h"

StateMachine::StateMachine()
{
    wifiManager = nullptr;
    ledController = nullptr;
    buzzerController = nullptr;
    buttonController = nullptr;
    apiClient = nullptr;
    sensorDataManager = nullptr;
    alertManager = nullptr;
    warningManager = nullptr;

    currentState = STATE_IDLE;
    sensingWindowStart = 0;
    fireDetectionRounds = 0;
    lastSensingTime = 0;
}

void StateMachine::init(WiFiManager *wm, LEDController *lc, BuzzerController *bc, ButtonController *btnc,
                        APIClient *api, SensorDataManager *sdm, AlertManager *am, WarningManager *wrn)
{
    wifiManager = wm;
    ledController = lc;
    buzzerController = bc;
    buttonController = btnc;
    apiClient = api;
    sensorDataManager = sdm;
    alertManager = am;
    warningManager = wrn;

    currentState = STATE_IDLE;

    Serial.println("State Machine inicializada");
}

void StateMachine::update()
{
    // Actualizar estado de conexion en LED
    ledController->setConnectionState(wifiManager->getConnectionStatus());

    // Siempre verificar botones excepto en alertas (se maneja aparte)
    if (currentState != STATE_ALERT_ACTIVE)
    {
        checkButtons();
    }

    // Ejecutar logica segun estado
    switch (currentState)
    {
    case STATE_IDLE:
        handleIdleState();
        break;

    case STATE_SENSING:
        handleSensingState();
        break;

    case STATE_ALERT_ACTIVE:
        handleAlertState();
        break;

    case STATE_WARNING_ACTIVE:
        handleWarningState();
        break;
    }
}

SystemState StateMachine::getCurrentState()
{
    return currentState;
}

void StateMachine::handleIdleState()
{
    // Esperar a tener conexion WiFi para comenzar
    if (wifiManager->getConnectionStatus())
    {
        // Obtener timestamp inicial del servidor
        String timestamp;
        if (apiClient->getServerTime(timestamp))
        {
            lastReadingTimestamp = timestamp;
            sensingWindowStart = millis();
            transitionToState(STATE_SENSING);
        }
    }
}

void StateMachine::handleSensingState()
{
    // Verificar si hay alerta manual iniciada desde boton
    if (alertManager->isActive())
    {
        transitionToState(STATE_ALERT_ACTIVE);
        return;
    }

    // Ciclo de senseo cada 10 segundos
    if (millis() - lastSensingTime >= SENSING_WINDOW_MS)
    {
        lastSensingTime = millis();

        Serial.println("Obteniendo lecturas de sensores...");

        // Obtener lecturas desde el ultimo timestamp
        String response;
        if (apiClient->getReadingsSince(lastReadingTimestamp, response))
        {
            // Cargar datos en el manager
            sensorDataManager->loadFromJson(response);

            // Actualizar timestamp para proxima consulta
            String newTimestamp;
            if (apiClient->getServerTime(newTimestamp))
            {
                lastReadingTimestamp = newTimestamp;
            }

            // Evaluar condiciones de incendio
            if (evaluateFireConditions())
            {
                Serial.println("INCENDIO DETECTADO!");

                // Contar sensores que participaron
                int numSensors = 0;
                if (sensorDataManager->getAverageTemperature() > FIRE_TEMP_THRESHOLD)
                    numSensors++;
                if (sensorDataManager->getAverageHumidity() < FIRE_HUMIDITY_THRESHOLD &&
                    sensorDataManager->getAverageHumidity() > 0)
                    numSensors++;
                if (sensorDataManager->getAverageSmoke() > FIRE_SMOKE_THRESHOLD)
                    numSensors++;
                if (sensorDataManager->getAverageLight() > FIRE_LIGHT_THRESHOLD)
                    numSensors++;

                alertManager->startAutomaticAlert(numSensors);
                transitionToState(STATE_ALERT_ACTIVE);
                fireDetectionRounds = 0;
                return;
            }

            // Evaluar condiciones de advertencia
            if (evaluateWarningConditions() && !warningManager->isActive())
            {
                Serial.println("Condiciones de advertencia detectadas");

                // Determinar tipo de advertencia
                if (sensorDataManager->hasRecentMotion(3))
                {
                    warningManager->startWarning("pir", "Movimiento detectado");
                }
                else if (sensorDataManager->getAverageDistance(3) < WARNING_DISTANCE_THRESHOLD)
                {
                    warningManager->startWarning("distance", "Objeto cercano detectado");
                }

                transitionToState(STATE_WARNING_ACTIVE);
            }
        }
        else
        {
            Serial.println("Error obteniendo lecturas");
        }
    }
}

void StateMachine::handleAlertState()
{
    // Actualizar alert manager
    alertManager->update();

    // Verificar botones de finalizacion
    if (buttonController->isRealPressed())
    {
        alertManager->endAlert(true);
        transitionToState(STATE_SENSING);
        fireDetectionRounds = 0;
    }
    else if (buttonController->isFalsePressed())
    {
        alertManager->endAlert(false);
        transitionToState(STATE_SENSING);
        fireDetectionRounds = 0;
    }
}

void StateMachine::handleWarningState()
{
    // Actualizar warning manager
    warningManager->update();

    // Verificar si se inicio alerta durante advertencia
    if (alertManager->isActive())
    {
        warningManager->forceEnd();
        transitionToState(STATE_ALERT_ACTIVE);
        return;
    }

    // Si la advertencia termino, volver a senseo
    if (!warningManager->isActive())
    {
        transitionToState(STATE_SENSING);
    }

    // Continuar senseo en background
    handleSensingState();
}

void StateMachine::checkButtons()
{
    // Verificar si se presiono boton de inicio manual
    if (buttonController->isStartPressed())
    {
        Serial.println("Boton de inicio manual presionado");
        alertManager->startManualAlert();

        // Terminar advertencia si estaba activa
        if (warningManager->isActive())
        {
            warningManager->forceEnd();
        }

        transitionToState(STATE_ALERT_ACTIVE);
    }
}

bool StateMachine::evaluateFireConditions()
{
    // Verificar que hay suficientes lecturas
    if (sensorDataManager->getTotalValidReadings() < 1)
    {
        return false;
    }

    // OR simple: cualquier sensor dispara alerta

    // Evaluar temperatura
    float avgTemp = sensorDataManager->getAverageTemperature();
    if (avgTemp > FIRE_TEMP_THRESHOLD)
    {
        Serial.print("ALERTA: Temperatura alta: ");
        Serial.println(avgTemp);
        return true;
    }

    // Evaluar humedad
    float avgHumidity = sensorDataManager->getAverageHumidity();
    if (avgHumidity < FIRE_HUMIDITY_THRESHOLD && avgHumidity > 0)
    {
        Serial.print("ALERTA: Humedad baja: ");
        Serial.println(avgHumidity);
        return true;
    }

    // Evaluar humo
    float avgSmoke = sensorDataManager->getAverageSmoke();
    if (avgSmoke > FIRE_SMOKE_THRESHOLD)
    {
        Serial.print("ALERTA: Humo alto: ");
        Serial.println(avgSmoke);
        return true;
    }

    // Evaluar luz (fotoresistencia alta indica fuego)
    float avgLight = sensorDataManager->getAverageLight();
    if (avgLight > FIRE_LIGHT_THRESHOLD)
    {
        Serial.print("ALERTA: Luz alta (fuego): ");
        Serial.println(avgLight);
        return true;
    }

    return false;
}

bool StateMachine::evaluateWarningConditions()
{
    // Advertencias solo dependen de PIR y distancia

    // Verificar PIR (movimiento detectado)
    if (sensorDataManager->hasRecentMotion(3))
    {
        Serial.println("ADVERTENCIA: Movimiento detectado (PIR)");
        return true;
    }

    // Verificar distancia (objeto cercano)
    float avgDistance = sensorDataManager->getAverageDistance(3);
    if (avgDistance > 0 && avgDistance < WARNING_DISTANCE_THRESHOLD)
    {
        Serial.print("ADVERTENCIA: Objeto cercano: ");
        Serial.print(avgDistance);
        Serial.println(" cm");
        return true;
    }

    return false;
}

void StateMachine::transitionToState(SystemState newState)
{
    if (currentState != newState)
    {
        Serial.print("Transicion de estado: ");
        Serial.print(currentState);
        Serial.print(" -> ");
        Serial.println(newState);

        currentState = newState;

        // Resetear contadores segun estado
        if (newState == STATE_SENSING)
        {
            lastSensingTime = 0;
        }
    }
}