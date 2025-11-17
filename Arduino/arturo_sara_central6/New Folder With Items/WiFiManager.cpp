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
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
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
                Serial.print("WiFi reconectado. IP: ");
                Serial.println(WiFi.localIP());
            }
            else
            {
                Serial.println("WiFi desconectado. Reconexion automatica activa...");
            }
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