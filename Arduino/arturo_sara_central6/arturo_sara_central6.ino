#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "uuid_utils.h"

#include "pins.h"
#include "constants.h"
#include "central_state.h"
#include "wifi_client.h"
#include "utils_leds.h"
#include "utils_buzzer.h"
#include "utils_strobe.h"
#include "server_time.h"
#include "api_client.h"
#include "button_logic.h"
#include "warning_logic.h"
#include "fire_logic.h"

unsigned long lastWindowStart = 0;
bool windowActive = false;

void setup()
{
    Serial.begin(115200);
    delay(300);

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(STROBE, OUTPUT);

    pinMode(BTN_START, INPUT_PULLUP);
    pinMode(BTN_REAL,  INPUT_PULLUP);
    pinMode(BTN_FALSE, INPUT_PULLUP);

    pinMode(POT, INPUT);

    setLEDRed();
    connectWiFi();

    Serial.println("\nCentral lista\n");
}

void loop()
{
    ensureWiFi();

    updateButtons();

    unsigned long now = millis();

    if (!fireActive && !warningActive && (now - lastWindowStart >= READ_WINDOW_MS))
    {
        Serial.println("\nINICIANDO NUEVA VENTANA DE LECTURAS...");
        lastWindowStart = now;
        windowActive = true;
    }

    if (windowActive && !fireActive)
    {
        static unsigned long windowReadCount = 0;
        windowReadCount++;

        int smokeVal = analogRead(A0) % 500;

        if (smokeVal > SMOKE_THRESHOLD)
        {
            triggerWarning();
        }

        if (windowReadCount >= 5)
        {
            Serial.println("Evaluando resultados ventana...");

            if (smokeVal > FIRE_SMOKE_THRESHOLD)
            {
                startFireAlert(1, "automatic");
            }

            windowReadCount = 0;
            windowActive = false;
        }
    }

    if (fireActive)
    {
        updateFireAlert();
    }
    else
    {
        updateWarning();
    }

    delay(10);
}
