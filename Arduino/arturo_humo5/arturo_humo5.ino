/*
 * IGNIO CENTRAL - PRUEBA OFFLINE
 * 
 * Sin WiFi, sin APIs, solo hardware local
 * Ciclo de prueba automático: NORMAL -> WARNING -> FIRE -> NORMAL
 * Controles manuales con botones
 */

#include <Arduino.h>
#include "constants.h"
#include "actuators.h"
#include "buttons.h"
#include "potentiometer.h"
#include "state_machine.h"

// Objetos globales
Actuators actuators;
Buttons buttons;
Potentiometer potentiometer;
StateMachine stateMachine;

// Estados de prueba automática
#define TEST_NORMAL   0
#define TEST_WARNING  1
#define TEST_FIRE     2

int testMode = TEST_NORMAL;
unsigned long testModeChangeTime = 0;
unsigned long testModeDuration = 5000;  // 5 segundos por modo

// Flags para control
bool inAutoTest = true;

// ==================== SETUP ====================

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("IGNIO CENTRAL - PRUEBA OFFLINE");
    Serial.println("========================================");
    Serial.println("Inicializando hardware...");
    
    // Inicializar componentes
    actuators.init();
    buttons.init();
    potentiometer.init();
    stateMachine.init(&actuators);
    
    Serial.println("✓ Hardware listo");
    Serial.println("\nModos disponibles:");
    Serial.println("- BTN_START (D1): Iniciar alerta manual FIRE");
    Serial.println("- BTN_REAL (D7): Confirmar alerta real");
    Serial.println("- BTN_FALSE (D4): Confirmar falsa alarma");
    Serial.println("- POT (A0): Regula frecuencia del buzzer");
    Serial.println("\nIniciando prueba automática...");
    Serial.println("Ciclo: NORMAL (5s) -> WARNING (5s) -> FIRE (5s) -> NORMAL");
    Serial.println("========================================\n");
    
    testModeChangeTime = millis();
}

// ==================== LOOP PRINCIPAL ====================

void loop() {
    // Actualizar entrada
    buttons.update();
    potentiometer.update();
    actuators.update();
    
    // Actualizar máquina de estados (timers)
    stateMachine.updateWarningTimer(&actuators);
    
    // ==================== CONTROL MANUAL ====================
    
    if (buttons.isStartPressed()) {
        Serial.println("\n>>> BTN_START presionado - Iniciando FIRE manual");
        inAutoTest = false;
        stateMachine.setStateFire(&actuators, 1);
    }
    
    if (buttons.isRealPressed() && stateMachine.isFireActive()) {
        Serial.println("\n>>> BTN_REAL presionado - Alerta REAL");
        inAutoTest = false;
        stateMachine.exitFire(&actuators, true);
    }
    
    if (buttons.isFalsePressed() && stateMachine.isFireActive()) {
        Serial.println("\n>>> BTN_FALSE presionado - FALSA ALARMA");
        inAutoTest = false;
        stateMachine.exitFire(&actuators, false);
    }
    
    // ==================== MODO AUTOMÁTICO ====================
    
    if (inAutoTest) {
        unsigned long elapsed = millis() - testModeChangeTime;
        
        if (elapsed >= testModeDuration) {
            // Cambiar de modo
            testMode = (testMode + 1) % 3;
            testModeChangeTime = millis();
            
            switch (testMode) {
                case TEST_NORMAL:
                    Serial.println("\n>>> [AUTO-TEST] Cambiando a NORMAL");
                    stateMachine.setStateNormal(&actuators);
                    break;
                case TEST_WARNING:
                    Serial.println("\n>>> [AUTO-TEST] Cambiando a WARNING");
                    stateMachine.setStateWarning(&actuators);
                    break;
                case TEST_FIRE:
                    Serial.println("\n>>> [AUTO-TEST] Cambiando a FIRE");
                    stateMachine.setStateFire(&actuators, 3);
                    break;
            }
        }
    }
    
    // ==================== ACTUALIZAR ACTUADORES ====================
    
    // LED según estado
    if (stateMachine.isFireActive()) {
        stateMachine.updateFireLED(&actuators);
        
        // Buzzer caótico en FIRE
        static unsigned long lastFireBuzzer = 0;
        if (millis() - lastFireBuzzer > 320) {
            actuators.fireBuzzerChaos(potentiometer.getFireFrequency());
            lastFireBuzzer = millis();
        }
        
        // Estrobo
        actuators.strobeUpdate();
        
    } else if (stateMachine.isWarningActive()) {
        stateMachine.updateWarningLED(&actuators);
        
        // Buzzer warning cada 3 parpadeos (300ms x 3 = 900ms)
        static unsigned long lastWarningBuzzer = 0;
        unsigned long elapsed = (millis() / 300) % 3;
        if (elapsed == 0 && millis() - lastWarningBuzzer > 900) {
            actuators.warningBeep(potentiometer.getWarningFrequency());
            lastWarningBuzzer = millis();
        }
    } else {
        // NORMAL
        actuators.setLEDGreen();
    }
    
    // ==================== DEBUG INFO ====================
    
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime > 2000) {
        printDebugInfo();
        lastDebugTime = millis();
    }
    
    delay(10);  // Pequeño delay para no saturar
}

// ==================== FUNCIONES AUXILIARES ====================

void printDebugInfo() {
    Serial.println("\n--- DEBUG INFO ---");
    Serial.print("Estado actual: ");
    switch (stateMachine.getCurrentState()) {
        case STATE_NORMAL:
            Serial.println("NORMAL");
            break;
        case STATE_WARNING:
            Serial.println("WARNING");
            Serial.print("  Tiempo: ");
            Serial.print(stateMachine.getWarningElapsedTime() / 1000);
            Serial.println("s / 30s");
            break;
        case STATE_FIRE:
            Serial.println("FIRE");
            Serial.print("  Tiempo: ");
            Serial.print(stateMachine.getFireElapsedTime() / 1000);
            Serial.println("s");
            Serial.print("  Sensores: ");
            Serial.println(stateMachine.getNumSensorsTriggered());
            break;
        default:
            Serial.println("DESCONOCIDO");
    }
    
    Serial.print("Potenciómetro: ");
    Serial.print(potentiometer.getRawValue());
    Serial.print(" | Warning: ");
    Serial.print(potentiometer.getWarningFrequency());
    Serial.print("Hz | Fire: ");
    Serial.print(potentiometer.getFireFrequency());
    Serial.println("Hz");
    
    if (inAutoTest) {
        Serial.print("AUTO-TEST en modo: ");
        switch (testMode) {
            case TEST_NORMAL:
                Serial.println("NORMAL");
                break;
            case TEST_WARNING:
                Serial.println("WARNING");
                break;
            case TEST_FIRE:
                Serial.println("FIRE");
                break;
        }
    }
    
    Serial.println("--- END DEBUG ---\n");
}
