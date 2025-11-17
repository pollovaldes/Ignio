#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "WiFiManager.h"
#include "LEDController.h"

class APIClient
{
private:
    WiFiManager* wifiMgr;
    LEDController* ledCtrl;
    WiFiClient wifiClient;
    
    bool makeRequest(const char* method, const char* endpoint, const char* payload, String& response);
    
public:
    APIClient();
    void init(WiFiManager* wm, LEDController* lc);
    bool getServerTime(String& timestamp);
    bool getReadingsSince(const String& timestamp, String& response);
    bool postAlert(const char* uuid, const String& timestampStarted, int numSensors, const char* alertType);
    bool putAlert(const char* uuid, const String& timestampEnded, bool isReal, int responseTime);
    bool postWarning(const char* sensorType, const char* message);
};

#endif
