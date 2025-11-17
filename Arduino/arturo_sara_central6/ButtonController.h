#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

class ButtonController
{
private:
    bool lastStateStart;
    bool lastStateReal;
    bool lastStateFalse;
    unsigned long lastDebounceStart;
    unsigned long lastDebounceReal;
    unsigned long lastDebounceFalse;
    const unsigned long debounceDelay = 50;
    
public:
    ButtonController();
    void init();
    void update();
    bool isStartPressed();
    bool isRealPressed();
    bool isFalsePressed();
};

#endif
