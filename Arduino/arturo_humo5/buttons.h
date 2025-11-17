#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include "constants.h"

class Button {
private:
    int pin;
    int lastState;
    unsigned long lastChangeTime;
    bool debounced;

public:
    Button(int p);
    void init();
    bool isPressed();  // Retorna true si fue presionado (flanco descendente)
    void update();
};

class Buttons {
private:
    Button btnStart;
    Button btnReal;
    Button btnFalse;

public:
    Buttons();
    void init();
    void update();
    
    bool isStartPressed();
    bool isRealPressed();
    bool isFalsePressed();
};

#endif
