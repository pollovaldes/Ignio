#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ========================================================================
// MÁQUINA DE ESTADOS - ORDEN DE PRIORIDAD
// ========================================================================
// 1. ALERTA ACTIVA (MODE_ALERT)
//    - Máxima prioridad, no puede ser interrumpida por nada
//    - Física: LED rojo + buzzer + strobe
//    - Si POST/PUT falla: entra en HTTP_WARNING pero alerta sigue
//    - Security warnings se cancelan si hay alerta
//    - Senseo se detiene durante alerta
//
// 2. HTTP WARNING (MODE_HTTP_WARNING)
//    - Activo solo si NO hay alerta
//    - Reintenta operación fallida cada 3 segundos
//    - Si alerta inicia durante WARNING: alerta toma prioridad
//    - Física: LED amarillo + buzzer pausa lenta
//
// 3. SECURITY WARNING (MODE_SECURITY_WARNING)
//    - Activo solo si NO hay alerta ni HTTP_WARNING
//    - Dura exactamente 10 segundos (no bloqueante)
//    - Física: LED naranja + patrón 3 pitidos
//    - Guarda tipo de warning (distancia o movimiento)
//
// 4. IDLE (MODE_IDLE)
//    - Senseo cada 7 segundos
//    - Escucha botones para alertas manuales
//    - Física: LED verde (WiFi ok) o rojo (WiFi down)
//
// EDGE CASES MANEJADOS:
// - Alerta sin timestamp: física activa, HTTP_WARNING reintenta
// - WiFi cae durante alerta: alerta sigue, HTTP_WARNING activo
// - Alerta inicia mientras HTTP_WARNING: alerta tiene prioridad
// - Security warning durante alerta: se cancela
// - Múltiples HTTPs fallidos: timeout y reintento indefinido
// ========================================================================

// Tiempos base
#define WIFI_RETRY_DELAY_MS 500
#define SENSE_INTERVAL_MS 7000
#define BUTTON_POLL_DELAY_MS 20
#define HTTP_TIMEOUT_MS 5000

// Hardware central
#define LED_R D0
#define LED_G D5
#define LED_B D6
#define BUZZER D3
#define STROBE D2

#define BTN_START D1
#define BTN_REAL D7
#define BTN_FALSE D4

#define POT_PIN A0

// Configuracion WiFi
const char *WIFI_SSID = "iPhone de Arturo";
const char *WIFI_PASSWORD = "123456789";

// API central
#define CENTRAL_DEVICE_ID 1
const char *API_BASE = "http://172.20.10.5:5075";
const char *TIME_ENDPOINT = "/Time";
const char *READINGS_SINCE_BASE = "/Readings/since/";
const char *ALERT_ENDPOINT = "/Alert";
const char *WARNING_ENDPOINT = "/Warning";

// Umbrales de alerta
#define TEMP_THRESHOLD_C 32.0f
#define HUM_THRESHOLD_PERCENT 40.0f
#define LIGHT_THRESHOLD 300.0f
#define SMOKE_THRESHOLD 350.0f

// Umbrales de seguridad
#define DISTANCE_CHANGE_THRESHOLD_CM 10.0f
#define PIR_MOTION_THRESHOLD 1              // Al menos 1 movimiento real (> 3s)
#define SECURITY_WARNING_DURATION_MS 10000

// Patron de alerta para buzzer y estrobo
#define ALERT_BEEP_MIN_HZ 1200
#define ALERT_BEEP_MAX_HZ 2600
#define ALERT_BEEP_ON_MS 120
#define ALERT_BEEP_OFF_MS 180
#define ALERT_STROBE_ON_MS 80
#define ALERT_STROBE_OFF_MS 120

// Patron de HTTP warning (LED amarillo parpadeante)
#define HTTP_WARNING_BEEP_MIN_HZ 800
#define HTTP_WARNING_BEEP_MAX_HZ 1200
#define HTTP_WARNING_BLINK_MS 400
#define HTTP_WARNING_BEEP_DURATION_MS 150
#define HTTP_WARNING_RETRY_DELAY_MS 3000

// Patron de SECURITY warning (LED naranja fuerte: ON-OFF-ON-OFF-ON-OFF-pausa)
#define SECURITY_WARNING_ON_MS 150
#define SECURITY_WARNING_OFF_MS 150
#define SECURITY_WARNING_PAUSE_MS 400
#define SECURITY_WARNING_BEEP_HZ 1500

// Estados de la central
enum CentralMode
{
    MODE_IDLE = 0,
    MODE_ALERT = 1,
    MODE_HTTP_WARNING = 2,
    MODE_SECURITY_WARNING = 3
};

// Variables globales de estado
CentralMode currentMode = MODE_IDLE;
bool alertActive = false;
bool alertIsManual = false;
String currentAlertUuid = "";
String alertStartTimestamp = "";
unsigned long alertStartMillis = 0;

// Control de alertas
unsigned long lastSenseMillis = 0;
String lastWindowTimestamp = "";
#define ALERT_ORPHAN_TIMEOUT_MS 120000  // 2 minutos sin comunicación = alerta huérfana termina

// Estados de botones
bool lastStartPressed = false;
bool lastRealPressed = false;
bool lastFalsePressed = false;

// Control del patron de alerta
unsigned long lastAlertPatternMillis = 0;
bool alertBeepOn = false;
bool alertStrobeOn = false;

// Variables para modo HTTP WARNING
bool httpWarningActive = false;
String httpWarningFailureReason = "";
unsigned long lastHttpWarningRetryMillis = 0;
String pendingHttpMethod = "";
String pendingPath = "";
String pendingPayload = "";
unsigned long lastHttpWarningPatternMillis = 0;
bool httpWarningBeepOn = false;

// Variables para modo SECURITY WARNING
bool securityWarningActive = false;
unsigned long securityWarningStartMillis = 0;
unsigned long securityWarningEndMillis = 0;
float baselineDistance = 0.0f;
bool baselineSet = false;
unsigned long lastSecurityPatternMillis = 0;
const char *securityWarningType = "";
#define SECURITY_WARNING_BASELINE_SAMPLES 3

// Prototipos
void connectWiFi();
void ensureWiFi();
void updateConnectionLed();
void setLedWifiConnected();
void setLedWifiDisconnected();
void setLedAlertMode();
void setLedHttpWarningMode();
void setLedSecurityWarningMode();
void clearAlertOutputs();
void clearWarningOutputs();

bool httpGetJson(const String &path, DynamicJsonDocument &doc);
bool httpPostJson(const String &path, const String &payload);
bool httpPutJson(const String &path, const String &payload);

bool getServerTime(String &outTimestamp);
bool getReadingsSince(
    const String &sinceTimestamp,
    float &avgTemp, bool &hasTemp,
    float &avgHum, bool &hasHum,
    float &avgLight, bool &hasLight,
    float &avgSmoke, bool &hasSmoke,
    float &avgPir, bool &hasPir,
    float &avgDistance, bool &hasDistance,
    int &motionCount
);

String generateUuid();
bool sendCreateAlert(const String &uuid, const String &timestampStarted, int numSensorsTriggered, const char *alertType);
bool sendUpdateAlert(const String &uuid, const String &timestampEnded, bool isReal, int responseSeconds);

void runSenseCycle();
void handleButtons();
void startAlert(bool manual, int numSensorsTriggered);
void endAlert(bool isReal);
void updateAlertEffects();
void handleHttpWarningMode();
void updateHttpWarningEffects();
void enterHttpWarningMode(const String &reason, const String &httpMethod, const String &path, const String &payload);
void retryPendingHttpRequest();
void postSecurityWarningToApi(const char *warningType);

bool httpGetJsonRaw(const String &path, DynamicJsonDocument &doc);
bool httpPostJsonRaw(const String &path, const String &payload);
bool httpPutJsonRaw(const String &path, const String &payload);

void postWarningToApi()
{
    DynamicJsonDocument doc(512);
    
    doc["idDevice"] = CENTRAL_DEVICE_ID;
    doc["message"] = "Fallo HTTP al conectar con API central";
    
    if (pendingHttpMethod == "GET")
    {
        doc["warningType"] = "http_get_failed";
    }
    else if (pendingHttpMethod == "POST")
    {
        doc["warningType"] = "http_post_failed";
    }
    else if (pendingHttpMethod == "PUT")
    {
        doc["warningType"] = "http_put_failed";
    }
    else
    {
        doc["warningType"] = "connection_lost";
    }
    
    doc["httpMethod"] = pendingHttpMethod;
    doc["httpEndpoint"] = pendingPath;

    String payload;
    serializeJson(doc, payload);

    Serial.println("\n[WARNING] Registrando fallo HTTP en BD...");
    Serial.println(payload);

    bool success = httpPostJsonRaw(String(WARNING_ENDPOINT), payload);
    
    if (success)
    {
        Serial.println("[WARNING] Registrado exitosamente en BD");
    }
    else
    {
        Serial.println("[WARNING] Fallo al registrar en BD (pero reintentó la operación original)");
    }
}



bool httpGetJsonRaw(const String &path, DynamicJsonDocument &doc);
bool httpPostJsonRaw(const String &path, const String &payload);
bool httpPutJsonRaw(const String &path, const String &payload);

// ========================================================================
// SETUP
// ========================================================================
void setup()
{
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(STROBE, OUTPUT);

    pinMode(BTN_START, INPUT_PULLUP);
    pinMode(BTN_REAL, INPUT_PULLUP);
    pinMode(BTN_FALSE, INPUT_PULLUP);

    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
    digitalWrite(BUZZER, LOW);
    digitalWrite(STROBE, LOW);

    Serial.begin(115200);
    delay(400);
    Serial.println("\nCentral IGNIO iniciando...\n");

    connectWiFi();

    if (!getServerTime(lastWindowTimestamp))
    {
        Serial.println("No se pudo obtener timestamp inicial del servidor, usando valor vacio.\n");
        lastWindowTimestamp = "";
    }
    else
    {
        currentMode = MODE_IDLE;
        updateConnectionLed();
    }
}

// ========================================================================
// LOOP PRINCIPAL - MÁQUINA DE ESTADOS ROBUSTA
// ========================================================================
void loop()
{
    handleButtons();

    unsigned long now = millis();

    // PRIORIDAD 1: ALERTA ACTIVA (máxima prioridad, no puede ser interrumpida)
    if (alertActive)
    {
        updateAlertEffects();
        
        // Si hay security warning activo, cancelarlo
        if (securityWarningActive)
        {
            securityWarningActive = false;
            baselineSet = false;
            clearWarningOutputs();
        }
        
        // Si entra en HTTP_WARNING, mantener alerta pero el HTTP_WARNING se maneja en paralelo
        if (currentMode != MODE_ALERT && currentMode != MODE_HTTP_WARNING)
        {
            currentMode = MODE_ALERT;
        }
    }
    // PRIORIDAD 2: HTTP_WARNING (solo si no hay alerta)
    else if (httpWarningActive && currentMode == MODE_HTTP_WARNING)
    {
        handleHttpWarningMode();
    }
    // PRIORIDAD 3: SECURITY_WARNING (solo si no hay alerta ni HTTP_WARNING)
    else if (securityWarningActive && currentMode == MODE_SECURITY_WARNING)
    {
        updateSecurityWarningEffects();
        
        if (now >= securityWarningEndMillis)
        {
            Serial.println("[SECURITY] Warning expired");
            postSecurityWarningToApi(securityWarningType);
            securityWarningActive = false;
            baselineSet = false;
            currentMode = MODE_IDLE;
            clearWarningOutputs();
            updateConnectionLed();
        }
    }
    // PRIORIDAD 4: IDLE (senseo y preparación)
    else
    {
        currentMode = MODE_IDLE;
        noTone(BUZZER);
        digitalWrite(STROBE, LOW);
        updateConnectionLed();
        
        // Solo senseo si no hay nada activo
        if (now - lastSenseMillis >= SENSE_INTERVAL_MS)
        {
            lastSenseMillis = now;
            runSenseCycle();
        }
    }

    delay(BUTTON_POLL_DELAY_MS);
}

// ========================================================================
// GESTION DE WiFi
// ========================================================================
void connectWiFi()
{
    Serial.println("\nConectando a WiFi...\n");
    setLedWifiDisconnected();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(WIFI_RETRY_DELAY_MS);
        Serial.print(".");
    }

    Serial.println("\n\nWiFi conectado");
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());

    setLedWifiConnected();
}

void ensureWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("\nWiFi perdido, reintentando conexion...\n");
        connectWiFi();
    }
}

void updateConnectionLed()
{
    // PRIORIDAD: si hay HTTP WARNING activo, LED amarillo (no importa nada)
    if (httpWarningActive)
    {
        setLedHttpWarningMode();
        return;
    }

    // ALERTA activa: LED rojo (se maneja en updateAlertEffects)
    if (alertActive)
    {
        return;
    }

    // SECURITY WARNING: se maneja en updateSecurityWarningEffects
    if (securityWarningActive)
    {
        return;
    }

    // IDLE: mostrar estado de WiFi
    if (WiFi.status() == WL_CONNECTED)
    {
        setLedWifiConnected();
    }
    else
    {
        setLedWifiDisconnected();
    }
}

void setLedWifiConnected()
{
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
}

void setLedWifiDisconnected()
{
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
}

void setLedAlertMode()
{
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_B, LOW);
}

void setLedHttpWarningMode()
{
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
}

void setLedSecurityWarningMode()
{
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_B, LOW);
}

void clearAlertOutputs()
{
    noTone(BUZZER);
    digitalWrite(STROBE, LOW);
}

void clearWarningOutputs()
{
    noTone(BUZZER);
    digitalWrite(STROBE, LOW);
}

// ========================================================================
// HELPERS HTTP - RAW (sin warning)
// ========================================================================
bool httpGetJsonRaw(const String &path, DynamicJsonDocument &doc)
{
    ensureWiFi();

    HTTPClient http;
    WiFiClient client;

    String url = String(API_BASE) + path;

    http.setTimeout(HTTP_TIMEOUT_MS);
    http.begin(client, url);

    int code = http.GET();

    if (code != HTTP_CODE_OK)
    {
        Serial.print("GET fallo. URL: ");
        Serial.println(url);
        Serial.print("Codigo HTTP: ");
        Serial.println(code);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    DeserializationError err = deserializeJson(doc, payload);
    if (err)
    {
        Serial.print("Error parseando JSON GET: ");
        Serial.println(err.c_str());
        return false;
    }

    return true;
}

bool httpPostJsonRaw(const String &path, const String &payload)
{
    ensureWiFi();

    HTTPClient http;
    WiFiClient client;

    String url = String(API_BASE) + path;

    http.setTimeout(HTTP_TIMEOUT_MS);
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    int code = http.POST(payload);

    Serial.print("POST ");
    Serial.print(url);
    Serial.print(" -> codigo: ");
    Serial.println(code);

    http.end();

    return (code >= 200 && code < 300);
}

bool httpPutJsonRaw(const String &path, const String &payload)
{
    ensureWiFi();

    HTTPClient http;
    WiFiClient client;

    String url = String(API_BASE) + path;

    http.setTimeout(HTTP_TIMEOUT_MS);
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    int code = http.PUT(payload);

    Serial.print("PUT ");
    Serial.print(url);
    Serial.print(" -> codigo: ");
    Serial.println(code);

    http.end();

    return (code >= 200 && code < 300);
}

// ========================================================================
// HELPERS HTTP - CON WARNING AUTOMATICO
// ========================================================================
bool httpGetJson(const String &path, DynamicJsonDocument &doc)
{
    bool success = httpGetJsonRaw(path, doc);
    
    if (!success && !httpWarningActive)
    {
        String reason = "HTTP GET fallo";
        enterHttpWarningMode(reason, "GET", path, "");
    }

    return success;
}

bool httpPostJson(const String &path, const String &payload)
{
    bool success = httpPostJsonRaw(path, payload);
    
    if (!success && !httpWarningActive)
    {
        String reason = "HTTP POST fallo";
        enterHttpWarningMode(reason, "POST", path, payload);
    }

    return success;
}

bool httpPutJson(const String &path, const String &payload)
{
    bool success = httpPutJsonRaw(path, payload);
    
    if (!success && !httpWarningActive)
    {
        String reason = "HTTP PUT fallo";
        enterHttpWarningMode(reason, "PUT", path, payload);
    }

    return success;
}

// ========================================================================
// /Time
// ========================================================================
bool getServerTime(String &outTimestamp)
{
    DynamicJsonDocument doc(256);

    if (!httpGetJson(String(TIME_ENDPOINT), doc))
    {
        Serial.println("No se pudo obtener /Time");
        return false;
    }

    if (!doc.containsKey("timestamp"))
    {
        Serial.println("Respuesta /Time sin campo 'timestamp'");
        return false;
    }

    outTimestamp = doc["timestamp"].as<String>();

    Serial.print("Hora servidor: ");
    Serial.println(outTimestamp);

    return true;
}

// ========================================================================
// /Readings/since/{timestamp}
// ARREGLADO PARA:
// 1. Buscar "dht11" en lugar de "temperature"
// 2. Contar eventos PIR (motionCount)
// ========================================================================
bool getReadingsSince(
    const String &sinceTimestamp,
    float &avgTemp, bool &hasTemp,
    float &avgHum, bool &hasHum,
    float &avgLight, bool &hasLight,
    float &avgSmoke, bool &hasSmoke,
    float &avgPir, bool &hasPir,
    float &avgDistance, bool &hasDistance,
    int &motionCount
)
{
    hasTemp = false;
    hasHum = false;
    hasLight = false;
    hasSmoke = false;
    hasPir = false;
    hasDistance = false;
    motionCount = 0;

    avgTemp = NAN;
    avgHum = NAN;
    avgLight = NAN;
    avgSmoke = NAN;
    avgPir = NAN;
    avgDistance = NAN;

    String path = String(READINGS_SINCE_BASE) + sinceTimestamp;

    DynamicJsonDocument doc(8192);

    if (!httpGetJson(path, doc))
    {
        Serial.println("No se pudieron obtener lecturas desde /Readings/since");
        return false;
    }

    double sumTemp = 0.0;
    int countTemp = 0;

    double sumHum = 0.0;
    int countHum = 0;

    double sumLight = 0.0;
    int countLight = 0;

    double sumSmoke = 0.0;
    int countSmoke = 0;

    double sumPir = 0.0;
    int countPir = 0;

    double sumDistance = 0.0;
    int countDistance = 0;

    // ========== TEMPERATURA (bajo "dht11", no "temperature") ==========
    if (doc.containsKey("dht11"))
    {
        JsonArray arr = doc["dht11"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            double val = obj["value"].as<double>();
            sumTemp += val;
            countTemp++;
        }
    }

    // ========== HUMEDAD (bajo "humidity") ==========
    if (doc.containsKey("humidity"))
    {
        JsonArray arr = doc["humidity"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            double val = obj["value"].as<double>();
            sumHum += val;
            countHum++;
        }
    }

    // ========== LUZ (bajo "light") ==========
    if (doc.containsKey("light"))
    {
        JsonArray arr = doc["light"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            double val = obj["value"].as<double>();
            sumLight += val;
            countLight++;
        }
    }

    // ========== HUMO (bajo "smoke") ==========
    if (doc.containsKey("smoke"))
    {
        JsonArray arr = doc["smoke"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            double val = obj["value"].as<double>();
            sumSmoke += val;
            countSmoke++;
        }
    }

    // ========== PIR (bajo "pir") - PARSEA duration_seconds Y FILTRA RUIDO ==========
    // IMPORTANTE: Filtrar movimientos cortos (ruido) vs movimientos reales
    #define PIR_REAL_MOTION_MIN_SECONDS 3.0f  // Solo contar duraciones > 3 segundos
    
    if (doc.containsKey("pir"))
    {
        JsonArray arr = doc["pir"].as<JsonArray>();
        motionCount = 0;
        int totalPirEvents = 0;
        
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            
            // Ahora "value" es duration_seconds (float), no boolean
            double durationSeconds = obj["value"].as<double>();
            
            // Usar duración como indicador de movimiento
            sumPir += durationSeconds;
            countPir++;
            totalPirEvents++;
            
            // Contar SOLO movimientos reales (> PIR_REAL_MOTION_MIN_SECONDS)
            if (durationSeconds >= PIR_REAL_MOTION_MIN_SECONDS)
            {
                motionCount++;
                Serial.print("  [PIR] Movimiento real detectado: ");
                Serial.print(durationSeconds);
                Serial.println("s");
            }
            else
            {
                Serial.print("  [PIR] Ruido/falso positivo: ");
                Serial.print(durationSeconds);
                Serial.println("s (descartado)");
            }
        }
        
        if (totalPirEvents > 0)
        {
            Serial.print("  [PIR] Total eventos en ventana: ");
            Serial.print(totalPirEvents);
            Serial.print(" | Movimientos reales: ");
            Serial.println(motionCount);
        }
    }

    // ========== DISTANCIA (bajo "distance") ==========
    if (doc.containsKey("distance"))
    {
        JsonArray arr = doc["distance"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            double val = obj["value"].as<double>();
            sumDistance += val;
            countDistance++;
        }
    }

    // ========== CALCULAR PROMEDIOS ==========
    if (countTemp > 0)
    {
        avgTemp = (float)(sumTemp / countTemp);
        hasTemp = true;
    }
    if (countHum > 0)
    {
        avgHum = (float)(sumHum / countHum);
        hasHum = true;
    }
    if (countLight > 0)
    {
        avgLight = (float)(sumLight / countLight);
        hasLight = true;
    }
    if (countSmoke > 0)
    {
        avgSmoke = (float)(sumSmoke / countSmoke);
        hasSmoke = true;
    }
    if (countPir > 0)
    {
        avgPir = (float)(sumPir / countPir);
        hasPir = true;
    }
    if (countDistance > 0)
    {
        avgDistance = (float)(sumDistance / countDistance);
        hasDistance = true;
    }

    // ========== IMPRIMIR RESUMEN ==========
    Serial.println("\nResumen promedio de la ventana:");
    if (hasTemp) { Serial.print("  Temp promedio: "); Serial.println(avgTemp); }
    else { Serial.println("  Temp promedio: sin datos"); }
    
    if (hasHum) { Serial.print("  Humedad promedio: "); Serial.println(avgHum); }
    else { Serial.println("  Humedad promedio: sin datos"); }
    
    if (hasLight) { Serial.print("  Luz promedio: "); Serial.println(avgLight); }
    else { Serial.println("  Luz promedio: sin datos"); }
    
    if (hasSmoke) { Serial.print("  Humo promedio: "); Serial.println(avgSmoke); }
    else { Serial.println("  Humo promedio: sin datos"); }
    
    if (hasPir) { Serial.print("  PIR promedio duración (s): "); Serial.println(avgPir); }
    else { Serial.println("  PIR promedio: sin datos"); }
    
    if (hasDistance) { Serial.print("  Distancia promedio (cm): "); Serial.println(avgDistance); }
    else { Serial.println("  Distancia promedio: sin datos"); }
    
    Serial.print("  Movimientos REALES detectados (>3s): ");
    Serial.println(motionCount);

    return true;
}

// ========================================================================
// UUID
// ========================================================================
String generateUuid()
{
    const char *hexChars = "0123456789abcdef";
    String uuid = "";

    for (int i = 0; i < 36; i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            uuid += '-';
        }
        else
        {
            int r = random(0, 16);
            uuid += hexChars[r];
        }
    }

    return uuid;
}

// ========================================================================
// /Alert (POST y PUT)
// ========================================================================
bool sendCreateAlert(const String &uuid, const String &timestampStarted, int numSensorsTriggered, const char *alertType)
{
    DynamicJsonDocument doc(256);

    doc["alertUuid"] = uuid;
    doc["idDevice"] = CENTRAL_DEVICE_ID;
    doc["timestampStarted"] = timestampStarted;
    doc["numSensorsTriggered"] = numSensorsTriggered;
    doc["alertType"] = alertType;

    String payload;
    serializeJson(doc, payload);

    Serial.println("\n[ALERT-POST] Sending alert creation...");
    Serial.println(payload);

    bool ok = httpPostJson(String(ALERT_ENDPOINT), payload);

    if (ok)
    {
        Serial.println("[ALERT-POST] Alert created successfully");
    }
    else
    {
        Serial.println("[ALERT-POST] Alert creation failed, entering HTTP_WARNING");
    }

    return ok;
}

bool sendUpdateAlert(const String &uuid, const String &timestampEnded, bool isReal, int responseSeconds)
{
    DynamicJsonDocument doc(256);

    doc["timestampEnded"] = timestampEnded;
    doc["IsReal"] = isReal;
    doc["ResponseTimeSeconds"] = responseSeconds;

    String payload;
    serializeJson(doc, payload);

    String path = String(ALERT_ENDPOINT) + "/" + uuid;

    Serial.println("\n[ALERT-PUT] Sending alert end...");
    Serial.print("[ALERT-PUT] UUID: ");
    Serial.println(uuid);
    Serial.println(payload);

    bool ok = httpPutJson(path, payload);

    if (ok)
    {
        Serial.println("[ALERT-PUT] Alert ended successfully");
    }
    else
    {
        Serial.println("[ALERT-PUT] Alert end failed, entering HTTP_WARNING");
    }

    return ok;
}

// ========================================================================
// Ronda de senseo
// ========================================================================
void runSenseCycle()
{
    Serial.println("\n================================");
    Serial.println("SENSE CYCLE");
    Serial.println("================================");

    if (lastWindowTimestamp.length() == 0)
    {
        Serial.println("No previous timestamp, requesting /Time...");
        if (!getServerTime(lastWindowTimestamp))
        {
            Serial.println("Failed to get timestamp, aborting cycle.\n");
            return;
        }
    }

    float avgTemp, avgHum, avgLight, avgSmoke, avgPir, avgDistance;
    bool hasTemp, hasHum, hasLight, hasSmoke, hasPir, hasDistance;
    int motionCount = 0;

    bool ok = getReadingsSince(
        lastWindowTimestamp,
        avgTemp, hasTemp,
        avgHum, hasHum,
        avgLight, hasLight,
        avgSmoke, hasSmoke,
        avgPir, hasPir,
        avgDistance, hasDistance,
        motionCount
    );

    if (!ok)
    {
        Serial.println("Failed to get readings, skipping automatic alert check.\n");
        String newTs;
        if (getServerTime(newTs))
        {
            lastWindowTimestamp = newTs;
        }
        return;
    }

    Serial.println("\nEvaluating fire alert conditions (OR logic)...");

    bool fire = false;
    int sensorsTriggered = 0;

    if (hasTemp && avgTemp > TEMP_THRESHOLD_C)
    {
        fire = true;
        sensorsTriggered++;
        Serial.print("[CONDITION] High temperature: ");
        Serial.println(avgTemp);
    }

    if (hasHum && avgHum < HUM_THRESHOLD_PERCENT)
    {
        fire = true;
        sensorsTriggered++;
        Serial.print("[CONDITION] Low humidity: ");
        Serial.println(avgHum);
    }

    if (hasLight && avgLight > LIGHT_THRESHOLD)
    {
        fire = true;
        sensorsTriggered++;
        Serial.print("[CONDITION] High light: ");
        Serial.println(avgLight);
    }

    if (hasSmoke && avgSmoke > SMOKE_THRESHOLD)
    {
        fire = true;
        sensorsTriggered++;
        Serial.print("[CONDITION] High smoke: ");
        Serial.println(avgSmoke);
    }

    if (fire && !alertActive)
    {
        Serial.print("\n[AUTO-ALERT] Triggered! Sensors: ");
        Serial.println(sensorsTriggered);
        startAlert(false, sensorsTriggered);
    }
    else if (fire && alertActive)
    {
        Serial.println("\n[AUTO-ALERT] Would trigger but alert already active");
    }
    else
    {
        Serial.println("\nNo fire conditions detected");
    }

    // Evaluar seguridad
    checkSecurityWarning(avgDistance, hasDistance, motionCount);

    String newTs;
    if (getServerTime(newTs))
    {
        lastWindowTimestamp = newTs;
    }

    Serial.println("\nSense cycle complete.\n");
}

// ========================================================================
// Manejo de botones
// ========================================================================
void handleButtons()
{
    bool startPressed = (digitalRead(BTN_START) == LOW);
    bool realPressed = (digitalRead(BTN_REAL) == LOW);
    bool falsePressed = (digitalRead(BTN_FALSE) == LOW);

    if (startPressed && !lastStartPressed && !alertActive && currentMode != MODE_HTTP_WARNING)
    {
        Serial.println("\nBoton iniciar alerta MANUAL presionado.");
        startAlert(true, 0);
    }

    if (alertActive)
    {
        if (realPressed && !lastRealPressed)
        {
            Serial.println("\nBoton fin REAL presionado.");
            endAlert(true);
        }
        if (falsePressed && !lastFalsePressed)
        {
            Serial.println("\nBoton fin FALSO presionado.");
            endAlert(false);
        }
    }

    lastStartPressed = startPressed;
    lastRealPressed = realPressed;
    lastFalsePressed = falsePressed;
}

// ========================================================================
// Inicio y fin de alertas
// ========================================================================
void startAlert(bool manual, int numSensorsTriggered)
{
    if (alertActive)
    {
        return;
    }

    alertActive = true;
    alertIsManual = manual;
    currentMode = MODE_ALERT;
    alertStartMillis = millis();

    setLedAlertMode();

    alertBeepOn = false;
    alertStrobeOn = false;
    lastAlertPatternMillis = millis();

    String serverNow;
    bool timeOk = getServerTime(serverNow);
    
    if (!timeOk)
    {
        Serial.println("[ALERT] No server time, pero la alerta física está ACTIVA. Se reintentará registro después.");
        alertStartTimestamp = "";
        currentAlertUuid = "";
        return;
    }

    alertStartTimestamp = serverNow;
    currentAlertUuid = generateUuid();

    const char *typeStr = manual ? "manual" : "automatic";

    bool alertSent = sendCreateAlert(currentAlertUuid, alertStartTimestamp, numSensorsTriggered, typeStr);
    
    if (!alertSent)
    {
        Serial.println("[ALERT] POST de creación falló, pero alerta FÍSICA sigue activa. HTTP WARNING activo.");
    }
}

void endAlert(bool isReal)
{
    if (!alertActive)
    {
        return;
    }

    clearAlertOutputs();

    unsigned long now = millis();
    unsigned long elapsedMillis = now - alertStartMillis;
    int responseSeconds = (int)(elapsedMillis / 1000);

    Serial.print("[ALERT] Duration: ");
    Serial.print(elapsedMillis);
    Serial.print("ms | Response: ");
    Serial.print(responseSeconds);
    Serial.println("s");

    // Si no hay UUID, no se puede hacer PUT, pero se desactiva la alerta física
    if (currentAlertUuid.length() == 0)
    {
        Serial.println("[ALERT] No UUID stored (failed to create), ending physical alert");
        alertActive = false;
        
        if (!httpWarningActive)
        {
            currentMode = MODE_IDLE;
            updateConnectionLed();
        }
        return;
    }

    // Intentar obtener timestamp de fin
    String serverNow;
    bool timeOk = getServerTime(serverNow);

    if (!timeOk)
    {
        Serial.println("[ALERT] No server time for END, ending physical alert");
        alertActive = false;
        
        if (!httpWarningActive)
        {
            currentMode = MODE_IDLE;
            updateConnectionLed();
        }
        return;
    }

    // Intentar enviar PUT de fin
    bool updateSent = sendUpdateAlert(currentAlertUuid, serverNow, isReal, responseSeconds);

    if (!updateSent)
    {
        Serial.println("[ALERT] PUT failed, HTTP_WARNING activo");
    }

    alertActive = false;
    
    if (!httpWarningActive)
    {
        currentMode = MODE_IDLE;
        updateConnectionLed();
    }
}

// ========================================================================
// Patron de alerta
// ========================================================================
void updateAlertEffects()
{
    unsigned long now = millis();

    int pot = analogRead(POT_PIN);

    int freq = map(pot, 0, 1023, ALERT_BEEP_MIN_HZ, ALERT_BEEP_MAX_HZ);
    if (freq < ALERT_BEEP_MIN_HZ)
        freq = ALERT_BEEP_MIN_HZ;
    if (freq > ALERT_BEEP_MAX_HZ)
        freq = ALERT_BEEP_MAX_HZ;

    if (alertBeepOn)
    {
        if (now - lastAlertPatternMillis >= ALERT_BEEP_ON_MS)
        {
            alertBeepOn = false;
            lastAlertPatternMillis = now;
            noTone(BUZZER);
        }
    }
    else
    {
        if (now - lastAlertPatternMillis >= ALERT_BEEP_OFF_MS)
        {
            alertBeepOn = true;
            lastAlertPatternMillis = now;
            tone(BUZZER, freq);
        }
    }

    static unsigned long lastStrobeMillis = 0;
    static bool strobeState = false;

    if (strobeState)
    {
        if (now - lastStrobeMillis >= ALERT_STROBE_ON_MS)
        {
            strobeState = false;
            lastStrobeMillis = now;
            digitalWrite(STROBE, LOW);
        }
    }
    else
    {
        if (now - lastStrobeMillis >= ALERT_STROBE_OFF_MS)
        {
            strobeState = true;
            lastStrobeMillis = now;
            digitalWrite(STROBE, HIGH);
        }
    }

    setLedAlertMode();
}

// ========================================================================
// MODO HTTP WARNING - Reintento de operaciones fallidas
// ========================================================================
void enterHttpWarningMode(const String &reason, const String &httpMethod, const String &path, const String &payload)
{
    if (httpWarningActive)
    {
        return;
    }

    httpWarningActive = true;
    currentMode = MODE_HTTP_WARNING;
    httpWarningFailureReason = reason;
    pendingHttpMethod = httpMethod;
    pendingPath = path;
    pendingPayload = payload;
    lastHttpWarningRetryMillis = millis();
    lastHttpWarningPatternMillis = millis();
    httpWarningBeepOn = false;

    setLedHttpWarningMode();

    Serial.println("\n=====================================");
    Serial.println("HTTP WARNING ACTIVATED");
    Serial.println("=====================================");
    Serial.print("Reason: ");
    Serial.println(reason);
    Serial.print("Method: ");
    Serial.println(httpMethod);
    Serial.print("Path: ");
    Serial.println(path);
    if (alertActive)
    {
        Serial.println("NOTE: ALERT STILL ACTIVE - physical alert continues");
    }
    Serial.println("Retrying every 3 seconds...");
    Serial.println("LED: YELLOW | Buzzer: slow pattern");
    Serial.println("=====================================\n");
}

void handleHttpWarningMode()
{
    updateHttpWarningEffects();

    unsigned long now = millis();

    if (now - lastHttpWarningRetryMillis >= HTTP_WARNING_RETRY_DELAY_MS)
    {
        lastHttpWarningRetryMillis = now;
        retryPendingHttpRequest();
    }
}

void postSecurityWarningToApi(const char *warningType)
{
    DynamicJsonDocument doc(512);

    doc["idDevice"] = CENTRAL_DEVICE_ID;
    doc["warningType"] = warningType;
    doc["message"] = "Security warning event detected";

    String payload;
    serializeJson(doc, payload);

    Serial.print("[SECURITY] Posting warning to API: ");
    Serial.println(warningType);

    bool success = httpPostJsonRaw(String(WARNING_ENDPOINT), payload);

    if (success)
    {
        Serial.println("[SECURITY] Warning posted successfully");
    }
    else
    {
        Serial.println("[SECURITY] Failed to post warning");
    }
}

void retryPendingHttpRequest()
{
    if (!httpWarningActive)
    {
        return;
    }

    bool success = false;

    Serial.print("\n[RETRY] Attempting ");
    Serial.print(pendingHttpMethod);
    Serial.print(" to ");
    Serial.println(pendingPath);

    if (pendingHttpMethod == "GET")
    {
        DynamicJsonDocument doc(8192);
        success = httpGetJsonRaw(pendingPath, doc);
    }
    else if (pendingHttpMethod == "POST")
    {
        success = httpPostJsonRaw(pendingPath, pendingPayload);
    }
    else if (pendingHttpMethod == "PUT")
    {
        success = httpPutJsonRaw(pendingPath, pendingPayload);
    }

    if (success)
    {
        postWarningToApi();

        Serial.println("\n=====================================");
        Serial.println("HTTP WARNING RESOLVED");
        Serial.println("Operation completed successfully!");
        Serial.println("=====================================\n");
        
        httpWarningActive = false;
        pendingHttpMethod = "";
        pendingPath = "";
        pendingPayload = "";
        httpWarningFailureReason = "";
        
        // Resetear timestamp para senseo limpio
        if (getServerTime(lastWindowTimestamp))
        {
            Serial.println("[RECOVERY] Timestamp reset, clean sense cycle");
        }
        
        // Decidir siguiente modo
        if (alertActive)
        {
            currentMode = MODE_ALERT;
            Serial.println("[STATE] Alert still active, returning to MODE_ALERT");
        }
        else
        {
            currentMode = MODE_IDLE;
        }
        
        clearWarningOutputs();
        updateConnectionLed();
    }
    else
    {
        Serial.println("[RETRY] Failed again. Retrying in 3 seconds...");
    }
}

void updateHttpWarningEffects()
{
    unsigned long now = millis();

    unsigned long cycleTime = (now - lastHttpWarningPatternMillis) % (HTTP_WARNING_BLINK_MS * 2);
    bool ledOn = cycleTime < HTTP_WARNING_BLINK_MS;

    if (ledOn)
    {
        setLedHttpWarningMode();
    }
    else
    {
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_B, LOW);
    }

    int freq = map(analogRead(POT_PIN), 0, 1023, HTTP_WARNING_BEEP_MIN_HZ, HTTP_WARNING_BEEP_MAX_HZ);
    if (freq < HTTP_WARNING_BEEP_MIN_HZ)
        freq = HTTP_WARNING_BEEP_MIN_HZ;
    if (freq > HTTP_WARNING_BEEP_MAX_HZ)
        freq = HTTP_WARNING_BEEP_MAX_HZ;

    unsigned long buzzerCycleTime = (now - lastHttpWarningPatternMillis) % (HTTP_WARNING_BLINK_MS * 4);
    bool buzzerTime = (buzzerCycleTime < HTTP_WARNING_BEEP_DURATION_MS);

    if (buzzerTime)
    {
        tone(BUZZER, freq);
    }
    else
    {
        noTone(BUZZER);
    }

    digitalWrite(STROBE, LOW);
}

// ========================================================================
// MODO SECURITY WARNING - Detección de intrusión/movimiento
// ========================================================================
void checkSecurityWarning(float avgDistance, bool hasDistance, int motionCount)
{
    if (securityWarningActive)
    {
        return;
    }

    if (!baselineSet)
    {
        if (hasDistance && avgDistance > 0)
        {
            baselineDistance = avgDistance;
            baselineSet = true;
            Serial.print("[SECURITY-INIT] Baseline distance: ");
            Serial.print(baselineDistance);
            Serial.println("cm");
        }
        return;
    }

    bool distanceAnomaly = false;
    if (hasDistance && avgDistance > 0)
    {
        float change = fabs(avgDistance - baselineDistance);
        if (change > DISTANCE_CHANGE_THRESHOLD_CM)
        {
            distanceAnomaly = true;
            Serial.print("[SECURITY-DIST] Anomaly - baseline: ");
            Serial.print(baselineDistance);
            Serial.print("cm -> current: ");
            Serial.print(avgDistance);
            Serial.print("cm (delta: ");
            Serial.print(change);
            Serial.println("cm)");
        }
        baselineDistance = avgDistance;
    }

    bool motionAnomaly = (motionCount >= PIR_MOTION_THRESHOLD);
    if (motionAnomaly)
    {
        Serial.print("[SECURITY-PIR] Real motion: ");
        Serial.print(motionCount);
        Serial.println(" events");
    }

    if (distanceAnomaly || motionAnomaly)
    {
        Serial.println("\n[SECURITY] ALERT TRIGGERED");
        if (distanceAnomaly && motionAnomaly)
        {
            startSecurityWarning("distance_anomaly");
        }
        else if (distanceAnomaly)
        {
            startSecurityWarning("distance_anomaly");
        }
        else
        {
            startSecurityWarning("motion_detected");
        }
    }
}

void startSecurityWarning(const char *warningType)
{
    if (securityWarningActive)
    {
        return;
    }

    securityWarningActive = true;
    currentMode = MODE_SECURITY_WARNING;
    securityWarningType = warningType;
    unsigned long now = millis();

    securityWarningStartMillis = now;
    securityWarningEndMillis = now + SECURITY_WARNING_DURATION_MS;
    lastSecurityPatternMillis = now;

    Serial.println("[SECURITY] Warning activated, 10s duration");
}

void updateSecurityWarningEffects()
{
    unsigned long now = millis();
    unsigned long elapsed = now - lastSecurityPatternMillis;

    unsigned long cycleTime = (SECURITY_WARNING_ON_MS + SECURITY_WARNING_OFF_MS) * 3 + SECURITY_WARNING_PAUSE_MS;
    unsigned long posInCycle = elapsed % cycleTime;

    bool ledOn = false;
    bool buzzerOn = false;

    for (int i = 0; i < 3; i++)
    {
        unsigned long start = i * (SECURITY_WARNING_ON_MS + SECURITY_WARNING_OFF_MS);
        unsigned long onEnd = start + SECURITY_WARNING_ON_MS;
        unsigned long offEnd = start + SECURITY_WARNING_ON_MS + SECURITY_WARNING_OFF_MS;

        if (posInCycle >= start && posInCycle < onEnd)
        {
            ledOn = true;
            buzzerOn = true;
            break;
        }
        else if (posInCycle >= onEnd && posInCycle < offEnd)
        {
            ledOn = false;
            buzzerOn = false;
            break;
        }
    }

    if (ledOn)
    {
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_B, LOW);
    }
    else
    {
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_B, LOW);
    }

    if (buzzerOn)
    {
        tone(BUZZER, SECURITY_WARNING_BEEP_HZ);
    }
    else
    {
        noTone(BUZZER);
    }
}