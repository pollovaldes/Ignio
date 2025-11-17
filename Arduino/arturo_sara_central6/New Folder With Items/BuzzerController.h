#ifndef BUZZER_CONTROLLER_H
#define BUZZER_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

enum BuzzerPattern
{
    BUZZER_OFF,
    BUZZER_WARNING,
    BUZZER_ALERT
};

class BuzzerController
{
private:
    BuzzerPattern currentPattern;
    unsigned long lastToneTime;
    bool toneState;
    int potValue;
    int warningCounter;
    
    void playTone(int frequency, int duration);
    void stopTone();
    
public:
    BuzzerController();
    void init();
    void update();
    void setPattern(BuzzerPattern pattern);
    void readPotentiometer();
    int getPotValue();
};

#endif
