// ============================================
// API_CLIENT.H - Envío de datos a la API (ESP8266)
// Versión con manejo de validez
// ============================================

#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "wifi_manager.h"

// Cliente WiFi global
WiFiClient wifiClient;

// ============================================
// FUNCIÓN AUXILIAR: Enviar JSON genérico
// ============================================
bool APIClient_SendRequest(const char* endpoint, const char* jsonPayload)
{
    if (!WiFiManager_IsConnected())
    {
        Serial.println("✗ WiFi no conectado, no se puede enviar");
        return false;
    }
    
    HTTPClient http;
    String url = String(SERVER_ADDRESS) + endpoint;
    
    http.begin(wifiClient, url);
    http.addHeader("Content-Type", "application/json");
    
    Serial.print("POST: ");
    Serial.println(url);
    Serial.print("Payload: ");
    Serial.println(jsonPayload);
    
    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0)
    {
        if (httpResponseCode == 200 || httpResponseCode == 201)
        {
            Serial.print("✓ Enviado (");
            Serial.print(httpResponseCode);
            Serial.println(")");
            http.end();
            return true;
        }
        else
        {
            Serial.print("✗ Error HTTP: ");
            Serial.println(httpResponseCode);
            http.end();
            return false;
        }
    }
    else
    {
        Serial.print("✗ Error conexión");
        http.end();
        return false;
    }
}

// ============================================
// ENVIAR: TEMPERATURA
// ============================================
void APIClient_SendTemperature(int idDispositivo, float temperatura, bool esValida)
{
    StaticJsonDocument<200> doc;
    doc["idDispositivo"] = idDispositivo;
    doc["esValida"] = esValida;
    doc["temperatura"] = temperatura;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    APIClient_SendRequest("/temperatura", jsonString.c_str());
}

// ============================================
// ENVIAR: HUMEDAD
// ============================================
void APIClient_SendHumidity(int idDispositivo, float humedad, bool esValida)
{
    StaticJsonDocument<200> doc;
    doc["idDispositivo"] = idDispositivo;
    doc["esValida"] = esValida;
    doc["humedad"] = humedad;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    APIClient_SendRequest("/humedad", jsonString.c_str());
}

// ============================================
// ENVIAR: HUMO
// ============================================
void APIClient_SendSmoke(int idDispositivo, int valor, bool esValida)
{
    StaticJsonDocument<200> doc;
    doc["idDispositivo"] = idDispositivo;
    doc["esValida"] = esValida;
    doc["valor"] = valor;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    APIClient_SendRequest("/humo", jsonString.c_str());
}

// ============================================
// ENVIAR: PIR
// ============================================
void APIClient_SendPIR(int idDispositivo, bool movimiento, bool esValida)
{
    StaticJsonDocument<200> doc;
    doc["idDispositivo"] = idDispositivo;
    doc["esValida"] = esValida;
    doc["movimiento"] = movimiento;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    APIClient_SendRequest("/pir", jsonString.c_str());
}

// ============================================
// ENVIAR: DISTANCIA
// ============================================
void APIClient_SendDistance(int idDispositivo, float distancia, bool esValida)
{
    StaticJsonDocument<200> doc;
    doc["idDispositivo"] = idDispositivo;
    doc["esValida"] = esValida;
    doc["distanciaCm"] = distancia;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    APIClient_SendRequest("/distancia", jsonString.c_str());
}

// ============================================
// ENVIAR: BOTÓN
// ============================================
void APIClient_SendButton(int idDispositivo, bool presionado, bool esValida)
{
    StaticJsonDocument<200> doc;
    doc["idDispositivo"] = idDispositivo;
    doc["esValida"] = esValida;
    doc["presionado"] = presionado;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    APIClient_SendRequest("/boton", jsonString.c_str());
}

// ============================================
// ENVIAR: ALERTA
// ============================================
void APIClient_SendAlert(int idDispositivo, int numeroSensores)
{
    StaticJsonDocument<200> doc;
    doc["idDispositivo"] = idDispositivo;
    doc["numeroSensores"] = numeroSensores;
    doc["respondida"] = false;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    APIClient_SendRequest("/alerta", jsonString.c_str());
}

#endif // API_CLIENT_H