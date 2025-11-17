#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <Arduino.h>
#include "constants.h"

class Potentiometer {
private:
    int rawValue;
    int mappedWarningFreq;
    int mappedFireFreq;

public:
    Potentiometer();
    void init();
    void update();
    
    int getRawValue();
    int getWarningFrequency();
    int getFireFrequency();
};

#endif
