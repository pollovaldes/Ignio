#ifndef WARNING_MANAGER_H
#define WARNING_MANAGER_H

#include <Arduino.h>
#include "Config.h"
#include "APIClient.h"
#include "LEDController.h"
#include "BuzzerController.h"

class WarningManager
{
private:
    APIClient* apiClient;
    LEDController* ledController;
    BuzzerController* buzzerController;
    
    bool warningActive;
    unsigned long warningStartTime;
    bool warningRegisteredInServer;
    String warningSensorType;
    String warningMessage;
    
    void activatePhysicalWarning();
    void deactivatePhysicalWarning();
    
public:
    WarningManager();
    void init(APIClient* api, LEDController* led, BuzzerController* buzzer);
    void startWarning(const char* sensorType, const char* message);
    void update();
    bool isActive();
    void forceEnd();
};

#endif
