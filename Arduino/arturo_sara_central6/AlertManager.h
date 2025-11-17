#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "APIClient.h"
#include "LEDController.h"
#include "BuzzerController.h"
#include "SensorDataManager.h"

class AlertManager
{
private:
    APIClient* apiClient;
    LEDController* ledController;
    BuzzerController* buzzerController;
    SensorDataManager* sensorDataManager;
    
    bool alertActive;
    char alertUuid[37];
    String alertStartTimestamp;
    bool isManualAlert;
    unsigned long alertStartMillis;
    unsigned long lastRetryTime;
    bool alertRegisteredInServer;
    
    void generateUUID(char* buffer);
    void activatePhysicalAlert();
    void deactivatePhysicalAlert();
    bool registerAlertInServer(const char* alertType, int numSensors);
    
public:
    AlertManager();
    void init(APIClient* api, LEDController* led, BuzzerController* buzzer, SensorDataManager* sensors);
    void startManualAlert();
    void startAutomaticAlert(int numSensorsTriggered);
    void endAlert(bool isReal);
    bool isActive();
    void update();
    const char* getCurrentUUID();
};

#endif
