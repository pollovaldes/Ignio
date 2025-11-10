// ============================================
// SENSOR_MANAGER.H - Lectura de sensores
// ============================================

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"
#include "DHT.h"

// Instancia del DHT11
DHT dht(PIN_DHT, DHT_TYPE);

void SensorManager_Init()
{
    Serial.println("Inicializando sensores...");
    
    // DHT11
    dht.begin();
    
    // PIR
    pinMode(PIN_PIR, INPUT);
    
    // HC-SR04
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    
    // Botón
    pinMode(PIN_BOTON, INPUT_PULLUP);
    
    // Humo (analógico, no necesita setup)
    
    Serial.println("✓ Sensores inicializados");
}

// ============================================
// LECTURA: TEMPERATURA (DHT11)
// Retorna un struct con valor y validez
// ============================================
struct SensorReading {
    float valor;
    bool valida;
};

SensorReading SensorManager_ReadTemperature()
{
    float temp = dht.readTemperature();
    
    if (isnan(temp))
    {
        return {0.0, false};  // Inválida
    }
    
    return {temp, true};  // Válida
}

// ============================================
// LECTURA: HUMEDAD (DHT11)
// ============================================
SensorReading SensorManager_ReadHumidity()
{
    float humedad = dht.readHumidity();
    
    if (isnan(humedad))
    {
        return {0.0, false};  // Inválida
    }
    
    return {humedad, true};  // Válida
}

// ============================================
// LECTURA: HUMO (Analógico)
// ============================================
SensorReading SensorManager_ReadSmoke()
{
    int valor = analogRead(PIN_HUMO);
    return {(float)constrain(valor, 0, 1023), true};  // Siempre válida
}

// ============================================
// LECTURA: PIR (Digital)
// ============================================
struct PIRReading {
    bool movimiento;
    bool valida;
};

PIRReading SensorManager_ReadPIR()
{
    return {digitalRead(PIN_PIR) == HIGH, true};  // Siempre válida
}

// ============================================
// LECTURA: DISTANCIA (HC-SR04 Ultrasónico)
// ============================================
SensorReading SensorManager_ReadDistance()
{
    // Generar pulso en TRIGGER
    digitalWrite(PIN_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIGGER, LOW);
    
    // Medir duración del pulso en ECHO
    long duracion = pulseIn(PIN_ECHO, HIGH, DISTANCE_TIMEOUT);
    
    if (duracion == 0)
    {
        return {0.0, false};  // Inválida (timeout)
    }
    
    // Calcular distancia: tiempo * velocidad / 2
    float distancia = (duracion * DISTANCE_SPEED) / 2.0;
    
    return {distancia, true};  // Válida
}

// ============================================
// LECTURA: BOTÓN
// ============================================
struct ButtonReading {
    bool presionado;
    bool valida;
};

ButtonReading SensorManager_ReadButton()
{
    // INPUT_PULLUP significa que presionado = LOW
    return {digitalRead(PIN_BOTON) == LOW, true};  // Siempre válida
}

#endif // SENSOR_MANAGER_H