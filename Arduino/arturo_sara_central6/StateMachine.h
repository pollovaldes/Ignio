#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "Config.h"
#include "WiFiManager.h"
#include "LEDController.h"
#include "BuzzerController.h"
#include "ButtonController.h"
#include "APIClient.h"
#include "SensorDataManager.h"
#include "AlertManager.h"
#include "WarningManager.h"

enum SystemState
{
    STATE_IDLE,
    STATE_SENSING,
    STATE_ALERT_ACTIVE,
    STATE_WARNING_ACTIVE
};

class StateMachine
{
private:
    WiFiManager *wifiManager;
    LEDController *ledController;
    BuzzerController *buzzerController;
    ButtonController *buttonController;
    APIClient *apiClient;
    SensorDataManager *sensorDataManager;
    AlertManager *alertManager;
    WarningManager *warningManager;

    SystemState currentState;
    unsigned long sensingWindowStart;
    String lastReadingTimestamp;
    int fireDetectionRounds;
    unsigned long lastSensingTime;

    void handleIdleState();
    void handleSensingState();
    void handleAlertState();
    void handleWarningState();
    void checkButtons();
    bool evaluateFireConditions();
    bool evaluateWarningConditions();
    void transitionToState(SystemState newState);

public:
    StateMachine();
    void init(WiFiManager *wm, LEDController *lc, BuzzerController *bc, ButtonController *btnc,
              APIClient *api, SensorDataManager *sdm, AlertManager *am, WarningManager *wrn);
    void update();
    SystemState getCurrentState();
};

#endif