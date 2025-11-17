#include "Config.h"
#include "WiFiManager.h"
#include "LEDController.h"
#include "BuzzerController.h"
#include "ButtonController.h"
#include "APIClient.h"
#include "SensorDataManager.h"
#include "AlertManager.h"
#include "WarningManager.h"
#include "StateMachine.h"

// Instancias globales
WiFiManager wifiManager;
LEDController ledController;
BuzzerController buzzerController;
ButtonController buttonController;
APIClient apiClient;
SensorDataManager sensorDataManager;
AlertManager alertManager;
WarningManager warningManager;
StateMachine stateMachine;

void setup()
{
    Serial.begin(115200);
    delay(100);
    
    Serial.println("=== IGNIO Central v6.0 ===");
    Serial.println("Inicializando sistema...");
    
    // Inicializar componentes
    ledController.init();
    buzzerController.init();
    buttonController.init();
    
    // Mostrar LED rojo mientras no hay WiFi
    ledController.setConnectionState(false);
    
    // Inicializar WiFi
    wifiManager.init();
    
    // Inicializar API client
    apiClient.init(&wifiManager, &ledController);
    
    // Inicializar managers
    sensorDataManager.init();
    alertManager.init(&apiClient, &ledController, &buzzerController, &sensorDataManager);
    warningManager.init(&apiClient, &ledController, &buzzerController);
    
    // Inicializar maquina de estados
    stateMachine.init(&wifiManager, &ledController, &buzzerController, &buttonController, 
                      &apiClient, &sensorDataManager, &alertManager, &warningManager);
    
    Serial.println("Sistema inicializado correctamente");
}

void loop()
{
    // Actualizar estado de WiFi
    wifiManager.update();
    
    // Actualizar controladores fisicos
    ledController.update();
    buzzerController.update();
    buttonController.update();
    
    // Ejecutar maquina de estados principal
    stateMachine.update();
    
    yield();
}
