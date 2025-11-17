#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>

// ==================== PINES ESP8266 ====================

// RGB LED (catodo común - HIGH = encendido)
#define LED_R D0
#define LED_G D5
#define LED_B D6

// Actuadores
#define BUZZER D3
#define STROBE D2

// Botones (INPUT_PULLUP - activos en LOW)
#define BTN_START D1    // Iniciar alerta manual
#define BTN_REAL D7     // Finalizar alerta real
#define BTN_FALSE D4    // Finalizar alerta falsa

// Potenciómetro
#define POT A0

// ==================== ESTADOS GLOBALES ====================

#define STATE_NO_WIFI   0
#define STATE_NORMAL    1
#define STATE_WARNING   2
#define STATE_FIRE      3

// ==================== UMBRALES DE SENSORES ====================

#define SMOKE_THRESHOLD       400
#define TEMP_THRESHOLD        45.0
#define HUMIDITY_THRESHOLD    25.0
#define LIGHT_THRESHOLD       800
#define DISTANCE_THRESHOLD    30    // cm

// ==================== TIMINGS ====================

#define WARNING_DURATION_MS   30000  // 30 segundos
#define BUTTON_DEBOUNCE_MS    50
#define SENSOR_CHECK_INTERVAL 10000  // 10 segundos
#define WIFI_RETRY_INTERVAL   500

// ==================== FRECUENCIAS BUZZER (Hz) ====================

#define BUZZER_WARNING_MIN    80
#define BUZZER_WARNING_MAX    250
#define BUZZER_FIRE_MIN       120
#define BUZZER_FIRE_MAX       500

// ==================== ESTROBO ====================

#define STROBE_FREQ_HZ        10    // 8-12 Hz

// ==================== FLASH VISUAL ====================

#define FLASH_DURATION_MS     100

// ==================== RETRY BUZZER ====================

#define ERROR_BUZZER_MIN_DELAY  2000  // 2 segundos primera vez
#define ERROR_BUZZER_MAX_DELAY  8000  // 8 segundos máximo

#endif
