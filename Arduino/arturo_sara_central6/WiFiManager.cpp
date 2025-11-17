#include "WiFiManager.h"

WiFiManager::WiFiManager()
{
    isConnected = false;
    lastCheckTime = 0;
}

void WiFiManager::init()
{
    Serial.println("Conectando a WiFi...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Esperar conexion inicial (timeout 20 segundos)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED)
    {
        isConnected = true;
        Serial.println("");
        Serial.print("WiFi conectado. IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        isConnected = false;
        Serial.println("");
        Serial.println("No se pudo conectar a WiFi");
    }
}

void WiFiManager::update()
{
    // Verificar estado periodicamente
    if (millis() - lastCheckTime >= WIFI_CHECK_INTERVAL_MS)
    {
        lastCheckTime = millis();
        
        bool currentStatus = (WiFi.status() == WL_CONNECTED);
        
        if (currentStatus != isConnected)
        {
            isConnected = currentStatus;
            
            if (isConnected)
            {
                Serial.println("WiFi reconectado");
            }
            else
            {
                Serial.println("WiFi desconectado");
            }
        }
        
        // Intentar reconectar si esta desconectado
        if (!isConnected)
        {
            reconnect();
        }
    }
}

bool WiFiManager::getConnectionStatus()
{
    return isConnected;
}

void WiFiManager::reconnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Intentando reconectar WiFi...");
        WiFi.reconnect();
    }
}
