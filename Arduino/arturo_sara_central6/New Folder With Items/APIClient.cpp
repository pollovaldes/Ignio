#include "APIClient.h"

APIClient::APIClient()
{
    wifiMgr = nullptr;
    ledCtrl = nullptr;
}

void APIClient::init(WiFiManager *wm, LEDController *lc)
{
    wifiMgr = wm;
    ledCtrl = lc;
    Serial.println("API Client inicializado");
}

bool APIClient::makeRequest(const char *method, const char *endpoint, const char *payload, String &response)
{
    if (!wifiMgr->getConnectionStatus())
    {
        Serial.println("No hay conexion WiFi para hacer request");
        return false;
    }

    HTTPClient http;
    String url = String(API_BASE_URL) + endpoint;

    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);

    int httpCode = -1;

    if (strcmp(method, "GET") == 0)
    {
        httpCode = http.GET();
    }
    else if (strcmp(method, "POST") == 0)
    {
        httpCode = http.POST(payload);
    }
    else if (strcmp(method, "PUT") == 0)
    {
        httpCode = http.PUT(payload);
    }

    bool success = (httpCode >= 200 && httpCode < 300);

    if (success)
    {
        response = http.getString();
        Serial.print("HTTP ");
        Serial.print(method);
        Serial.print(" ");
        Serial.print(endpoint);
        Serial.print(" -> ");
        Serial.println(httpCode);

        if (ledCtrl != nullptr)
        {
            ledCtrl->showConfirmation(true);
        }
    }
    else
    {
        Serial.print("HTTP ");
        Serial.print(method);
        Serial.print(" ");
        Serial.print(endpoint);
        Serial.print(" FALLO: ");
        Serial.println(httpCode);

        if (ledCtrl != nullptr)
        {
            ledCtrl->showConfirmation(false);
        }
    }

    http.end();
    delay(50);
    return success;
}

bool APIClient::getServerTime(String &timestamp)
{
    String response;
    bool success = makeRequest("GET", "/Time", "", response);

    if (success)
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, response);

        if (!error)
        {
            timestamp = doc["timestamp"].as<String>();
            Serial.print("Timestamp del servidor: ");
            Serial.println(timestamp);
            return true;
        }
        else
        {
            Serial.println("Error parseando timestamp");
            return false;
        }
    }

    return false;
}

bool APIClient::getReadingsSince(const String &timestamp, String &response)
{
    String endpoint = "/Readings/since/" + timestamp;
    return makeRequest("GET", endpoint.c_str(), "", response);
}

bool APIClient::postAlert(const char *uuid, const String &timestampStarted, int numSensors, const char *alertType)
{
    StaticJsonDocument<300> doc;
    doc["alertUuid"] = uuid;
    doc["idDevice"] = DEVICE_ID;
    doc["timestampStarted"] = timestampStarted;
    doc["numSensorsTriggered"] = numSensors;
    doc["alertType"] = alertType;

    String payload;
    serializeJson(doc, payload);

    String response;
    return makeRequest("POST", "/Alert", payload.c_str(), response);
}

bool APIClient::putAlert(const char *uuid, const String &timestampEnded, bool isReal, int responseTime)
{
    StaticJsonDocument<300> doc;
    doc["timestampEnded"] = timestampEnded;
    doc["isReal"] = isReal;
    doc["responseTimeSeconds"] = responseTime;

    String payload;
    serializeJson(doc, payload);

    String endpoint = String("/Alert/") + uuid;
    String response;
    return makeRequest("PUT", endpoint.c_str(), payload.c_str(), response);
}

bool APIClient::postWarning(const char *sensorType, const char *message)
{
    StaticJsonDocument<300> doc;
    doc["idDevice"] = DEVICE_ID;
    doc["sensorType"] = sensorType;
    doc["message"] = message;

    String payload;
    serializeJson(doc, payload);

    String response;
    return makeRequest("POST", "/Warning", payload.c_str(), response);
}