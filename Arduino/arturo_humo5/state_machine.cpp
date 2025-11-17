#include "state_machine.h"
#include "actuators.h"

StateMachine::StateMachine() 
    : currentState(STATE_NORMAL), warningStartTime(0), warningActive(false),
      fireStartTime(0), fireActive(false), numSensorsTriggered(0), actuator(nullptr) {
}

void StateMachine::init(Actuators* act) {
    actuator = act;
    setStateNormal(act);
}

// ==================== TRANSICIONES ====================

void StateMachine::setStateNormal(Actuators* act) {
    if (currentState == STATE_NORMAL) return;
    
    currentState = STATE_NORMAL;
    warningActive = false;
    fireActive = false;
    act->allOff();
    act->setLEDGreen();
    Serial.println("[STATE_MACHINE] -> NORMAL");
}

void StateMachine::setStateWarning(Actuators* act) {
    if (warningActive) {
        // Reiniciar timer
        warningStartTime = millis();
        Serial.println("[STATE_MACHINE] Warning reiniciado");
        return;
    }
    
    currentState = STATE_WARNING;
    warningActive = true;
    fireActive = false;
    warningStartTime = millis();
    act->allOff();
    act->setLEDYellow();
    Serial.println("[STATE_MACHINE] -> WARNING (30s)");
}

void StateMachine::setStateFire(Actuators* act, int numSensors) {
    if (fireActive) return;  // Ya está en fuego
    
    // Si hay warning, cancelarlo
    if (warningActive) {
        warningActive = false;
        Serial.println("[STATE_MACHINE] Warning cancelado por FIRE");
    }
    
    currentState = STATE_FIRE;
    fireActive = true;
    warningActive = false;
    fireStartTime = millis();
    numSensorsTriggered = numSensors;
    
    act->allOff();
    act->strobeStart();
    Serial.print("[STATE_MACHINE] -> FIRE (sensores: ");
    Serial.print(numSensors);
    Serial.println(")");
}

void StateMachine::exitFire(Actuators* act, bool isReal) {
    if (!fireActive) return;
    
    unsigned long elapsedSeconds = (millis() - fireStartTime) / 1000;
    
    fireActive = false;
    act->allOff();  // Apagar TODO incluyendo estrobo
    act->strobeStop();
    
    if (isReal) {
        Serial.print("[STATE_MACHINE] ALERTA REAL - Tiempo de respuesta: ");
        Serial.print(elapsedSeconds);
        Serial.println("s");
    } else {
        Serial.print("[STATE_MACHINE] FALSA ALARMA - Tiempo de respuesta: ");
        Serial.print(elapsedSeconds);
        Serial.println("s");
    }
    
    setStateNormal(act);
}

void StateMachine::exitWarning(Actuators* act) {
    if (!warningActive) return;
    
    warningActive = false;
    Serial.println("[STATE_MACHINE] Warning finalizado");
    setStateNormal(act);
}

// ==================== GETTERS ====================

int StateMachine::getCurrentState() {
    return currentState;
}

bool StateMachine::isFireActive() {
    return fireActive;
}

bool StateMachine::isWarningActive() {
    return warningActive;
}

unsigned long StateMachine::getWarningElapsedTime() {
    if (!warningActive) return 0;
    return millis() - warningStartTime;
}

unsigned long StateMachine::getFireElapsedTime() {
    if (!fireActive) return 0;
    return millis() - fireStartTime;
}

int StateMachine::getNumSensorsTriggered() {
    return numSensorsTriggered;
}

// ==================== UPDATES ====================

void StateMachine::updateWarningTimer(Actuators* act) {
    if (!warningActive) return;
    
    unsigned long elapsed = getWarningElapsedTime();
    
    // Si pasaron 30 segundos, salir
    if (elapsed >= WARNING_DURATION_MS) {
        exitWarning(act);
    }
}

void StateMachine::updateFireLED(Actuators* act) {
    if (!fireActive) return;
    
    // Patrón caótico rojo+amarillo
    unsigned long elapsed = millis() % 500;  // Ciclo de 500ms
    
    if (elapsed < 100) {
        act->setLEDRed();
    } else if (elapsed < 200) {
        act->setLEDYellow();
    } else if (elapsed < 300) {
        act->setLEDRed();
    } else if (elapsed < 400) {
        act->setLEDOff();
    } else {
        act->setLEDYellow();
    }
}

void StateMachine::updateWarningLED(Actuators* act) {
    if (!warningActive) return;
    
    // Amarillo parpadeante: 300ms on, 300ms off
    unsigned long elapsed = millis() % 600;
    
    if (elapsed < 300) {
        act->setLEDYellow();
    } else {
        act->setLEDOff();
    }
}