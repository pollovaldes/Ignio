#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "constants.h"

class Actuators;  // Forward declaration

class StateMachine {
private:
    int currentState;
    unsigned long warningStartTime;
    bool warningActive;
    unsigned long fireStartTime;
    bool fireActive;
    int numSensorsTriggered;

public:
    StateMachine();
    void init(Actuators* act);
    
    // Transiciones de estado
    void setStateNormal(Actuators* act);
    void setStateWarning(Actuators* act);
    void setStateFire(Actuators* act, int numSensors = 1);
    void exitFire(Actuators* act, bool isReal);
    void exitWarning(Actuators* act);
    
    // Getters
    int getCurrentState();
    bool isFireActive();
    bool isWarningActive();
    unsigned long getWarningElapsedTime();
    unsigned long getFireElapsedTime();
    int getNumSensorsTriggered();
    
    // Updates
    void updateWarningTimer(Actuators* act);
    void updateFireLED(Actuators* act);
    void updateWarningLED(Actuators* act);

private:
    Actuators* actuator;
};

#endif