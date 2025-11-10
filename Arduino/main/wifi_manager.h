// ============================================
// WIFI_MANAGER.H - Gestión de WiFi (ESP8266)
// ============================================

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>
#include "config.h"

unsigned long lastWiFiCheck = 0;

void WiFiManager_Init()
{
    Serial.println("Conectando a WiFi...");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n✓ WiFi Conectado");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\n✗ No se pudo conectar a WiFi");
        Serial.println("Continuando sin conexión...");
    }
}

void WiFiManager_CheckConnection()
{
    if (millis() - lastWiFiCheck >= WIFI_CHECK_INTERVAL)
    {
        lastWiFiCheck = millis();
        
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("WiFi desconectado, reconectando...");
            WiFi.reconnect();
        }
        else
        {
            Serial.println("✓ WiFi OK");
        }
    }
}

bool WiFiManager_IsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

#endif // WIFI_MANAGER_H