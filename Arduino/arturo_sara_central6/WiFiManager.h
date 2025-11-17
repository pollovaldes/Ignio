#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include "Config.h"

class WiFiManager
{
private:
    bool isConnected;
    unsigned long lastCheckTime;
    
public:
    WiFiManager();
    void init();
    void update();
    bool getConnectionStatus();
    void reconnect();
};

#endif
