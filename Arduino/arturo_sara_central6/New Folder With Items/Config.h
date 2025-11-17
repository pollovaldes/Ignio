#ifndef CONFIG_H
#define CONFIG_H

// Pines del sistema
#define LED_R D0
#define LED_G D5
#define LED_B D6
#define STROBE_PIN D2
#define BUZZER_PIN D3
#define POTENTIOMETER_PIN A0
#define BTN_START D1
#define BTN_REAL D7
#define BTN_FALSE D4

// Credenciales WiFi
#define WIFI_SSID "Mi perro cuando"
#define WIFI_PASSWORD "SggUD6o4rWN?7IaOdHqkXv2HB"

// URL base del servidor
#define API_BASE_URL "http://192.168.1.166:5075"

// ID de la central (ajustar segun configuracion)
#define DEVICE_ID 1

// Tiempos y constantes
#define SENSING_WINDOW_MS 10000
#define WARNING_DURATION_MS 15000
#define HTTP_RETRY_INTERVAL_MS 5000
#define WIFI_CHECK_INTERVAL_MS 5000
#define LED_BLINK_FAST_MS 150
#define LED_BLINK_SLOW_MS 1000
#define LED_CONFIRMATION_MS 300

// Constantes de deteccion de incendio
#define FIRE_TEMP_THRESHOLD 35.0
#define FIRE_HUMIDITY_THRESHOLD 30.0
#define FIRE_SMOKE_THRESHOLD 600
#define FIRE_LIGHT_THRESHOLD 800
#define FIRE_DETECTION_ROUNDS 1
#define MIN_SENSORS_FOR_FIRE 1

// Constantes de advertencia (solo PIR y distancia)
#define WARNING_PIR_THRESHOLD 1
#define WARNING_DISTANCE_THRESHOLD 50.0

#endif