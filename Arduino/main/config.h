// ============================================
// CONFIG.H - Configuración global
// ============================================

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// CONFIGURACIÓN WiFi
// ============================================
const char* WIFI_SSID = "Mi perro cuando";
const char* WIFI_PASSWORD = "SggUD6o4rWN?7IaOdHqkXv2HB";

// ============================================
// CONFIGURACIÓN DEL SERVER
// ============================================
const char* SERVER_ADDRESS = "http://192.168.1.166:5074";
// O si lo pruebas en local desde otra máquina, usa la IP de esa máquina
// Ej: "http://192.168.0.50:5000"

const int idDispositivo = 1; // ID del dispositivo en la BD (debe existir)

// ============================================
// CONFIGURACIÓN DE PINES (ESP8266)
// ============================================

// DHT11 (Temperatura y Humedad)
#define PIN_DHT D4          // GPIO2

// Sensor de Humo (Analógico)
#define PIN_HUMO A0         // A0 del ESP8266

// Sensor PIR (Digital)
#define PIN_PIR D1          // GPIO5

// Sensor Ultrasónico HC-SR04
#define PIN_ECHO D6         // GPIO12
#define PIN_TRIGGER D7      // GPIO13

// Botón de activación manual
#define PIN_BOTON D5        // GPIO14

// ============================================
// CONFIGURACIÓN DE TIMING
// ============================================
#define SENSOR_READ_INTERVAL 5000    // Leer sensores cada 5 segundos
#define WIFI_CHECK_INTERVAL 30000    // Verificar WiFi cada 30 segundos

// ============================================
// CONFIGURACIÓN DE SENSORES
// ============================================
#define DHT_TYPE DHT11              // Tipo de sensor DHT
#define DISTANCE_TIMEOUT 10000       // Timeout para HC-SR04 en microsegundos
#define DISTANCE_SPEED 0.034         // Velocidad del sonido / 2 (cm/µs)

#endif // CONFIG_H
