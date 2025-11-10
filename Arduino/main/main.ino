// ============================================
// MAIN.INO - Sistema IoT ESP8266
// Archivo principal: conexión WiFi y loop
// ============================================

#include "config.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "api_client.h"

// Variables globales para timing
unsigned long lastSensorRead = 0;
unsigned long sensorReadInterval = 5000; // Leer sensores cada 5 segundos

void setup()
{
    Serial.begin(115200);
    delay(100);
    
    Serial.println("\n\n");
    Serial.println("=====================================");
    Serial.println("INICIANDO SISTEMA IoT ESP8266");
    Serial.println("=====================================");
    
    // Inicializar WiFi
    WiFiManager_Init();
    
    // Inicializar sensores
    SensorManager_Init();
    
    Serial.println("SETUP COMPLETADO");
    Serial.println("=====================================\n");
}

void loop()
{
    // Mantener conexión WiFi activa
    WiFiManager_CheckConnection();
    
    // Leer sensores cada X milisegundos
    if (millis() - lastSensorRead >= sensorReadInterval)
    {
        lastSensorRead = millis();
        
        Serial.println("\n--- CICLO DE LECTURA ---");
        
        // ============================================
        // TEMPERATURA
        // ============================================
        SensorReading tempReading = SensorManager_ReadTemperature();
        if (tempReading.valida)
        {
            Serial.print("Temp: ");
            Serial.print(tempReading.valor);
            Serial.println(" °C");
        }
        else
        {
            Serial.println("✗ Error leyendo temperatura");
        }
        APIClient_SendTemperature(idDispositivo, tempReading.valor, tempReading.valida);
        delay(500);
        
        // ============================================
        // HUMEDAD
        // ============================================
        SensorReading humReading = SensorManager_ReadHumidity();
        if (humReading.valida)
        {
            Serial.print("Humedad: ");
            Serial.print(humReading.valor);
            Serial.println(" %");
        }
        else
        {
            Serial.println("✗ Error leyendo humedad");
        }
        APIClient_SendHumidity(idDispositivo, humReading.valor, humReading.valida);
        delay(500);
        
        // ============================================
        // HUMO
        // ============================================
        SensorReading smokeReading = SensorManager_ReadSmoke();
        Serial.print("Humo: ");
        Serial.println((int)smokeReading.valor);
        APIClient_SendSmoke(idDispositivo, (int)smokeReading.valor, smokeReading.valida);
        delay(500);
        
        // ============================================
        // PIR
        // ============================================
        PIRReading pirReading = SensorManager_ReadPIR();
        Serial.print("PIR: ");
        Serial.println(pirReading.movimiento ? "MOVIMIENTO" : "SIN MOVIMIENTO");
        APIClient_SendPIR(idDispositivo, pirReading.movimiento, pirReading.valida);
        delay(500);
        
        // ============================================
        // DISTANCIA
        // ============================================
        SensorReading distReading = SensorManager_ReadDistance();
        if (distReading.valida)
        {
            Serial.print("Distancia: ");
            Serial.print(distReading.valor);
            Serial.println(" cm");
        }
        else
        {
            Serial.println("✗ Error leyendo distancia (timeout)");
        }
        APIClient_SendDistance(idDispositivo, distReading.valor, distReading.valida);
        delay(500);
        
        // ============================================
        // BOTÓN
        // ============================================
        ButtonReading buttonReading = SensorManager_ReadButton();
        Serial.print("Botón: ");
        Serial.println(buttonReading.presionado ? "PRESIONADO" : "NO PRESIONADO");
        APIClient_SendButton(idDispositivo, buttonReading.presionado, buttonReading.valida);
        delay(500);
        
        Serial.println("--- FIN CICLO ---\n");
    }
}