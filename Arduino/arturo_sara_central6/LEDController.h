#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include "Config.h"

enum LEDMode
{
    LED_MODE_CONNECTION,
    LED_MODE_WARNING,
    LED_MODE_ALERT,
    LED_MODE_CONFIRMATION
};

enum LEDColor
{
    LED_OFF,
    LED_RED,
    LED_GREEN,
    LED_YELLOW
};

class LEDController
{
private:
    LEDMode currentMode;
    LEDColor currentColor;
    bool strobeState;
    unsigned long lastBlinkTime;
    unsigned long confirmationStartTime;
    bool isConfirmationActive;
    LEDColor confirmationColor;
    bool connectionState;
    
    void setColor(LEDColor color);
    void turnOffRGB();
    
public:
    LEDController();
    void init();
    void update();
    void setConnectionState(bool connected);
    void setWarningMode();
    void setAlertMode();
    void showConfirmation(bool success);
    void returnToBaseMode();
    void setStrobeState(bool state);
};

#endif
