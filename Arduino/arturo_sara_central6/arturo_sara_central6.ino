#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Tiempos base
#define WIFI_RETRY_DELAY_MS 500      // Tiempo entre reintentos de WiFi
#define SENSE_INTERVAL_MS 7000       // Intervalo entre rondas de senseo
#define BUTTON_POLL_DELAY_MS 20      // Pequeño delay para el loop
#define HTTP_TIMEOUT_MS 5000         // Timeout razonable para peticiones HTTP

// Hardware central
#define LED_R D0                     // Rojo RGB
#define LED_G D5                     // Verde RGB
#define LED_B D6                     // Azul RGB
#define BUZZER D3                    // Buzzer
#define STROBE D2                    // Estrobo LED

#define BTN_START D1                 // Boton iniciar alerta
#define BTN_REAL D7                  // Boton finalizar alerta REAL
#define BTN_FALSE D4                 // Boton finalizar alerta FALSA

#define POT_PIN A0                   // Potenciometro para patron del buzzer

// Configuracion WiFi
const char *WIFI_SSID = "Mi perro cuando";
const char *WIFI_PASSWORD = "SggUD6o4rWN?7IaOdHqkXv2HB";

// API central
#define CENTRAL_DEVICE_ID 1          // id_device de la central en la tabla device
const char *API_BASE = "http://192.168.1.166:5075";
const char *TIME_ENDPOINT = "/Time";
const char *READINGS_SINCE_BASE = "/Readings/since/";
const char *ALERT_ENDPOINT = "/Alert";
const char *WARNING_ENDPOINT = "/Warning";

// Umbrales de alerta
#define TEMP_THRESHOLD_C 32.0f
#define HUM_THRESHOLD_PERCENT 40.0f
#define LIGHT_THRESHOLD 300.0f
#define SMOKE_THRESHOLD 350.0f

// Patron de alerta para buzzer y estrobo
#define ALERT_BEEP_MIN_HZ 1200
#define ALERT_BEEP_MAX_HZ 2600
#define ALERT_BEEP_ON_MS 120
#define ALERT_BEEP_OFF_MS 180
#define ALERT_STROBE_ON_MS 80
#define ALERT_STROBE_OFF_MS 120

// Patron de warning (cuando falla una operacion HTTP)
#define WARNING_BEEP_MIN_HZ 800
#define WARNING_BEEP_MAX_HZ 1200
#define WARNING_BLINK_MS 400       // Tiempo de cada parpadeo (prendido Y apagado = 400ms total = 0.4s)
#define WARNING_BEEP_DURATION_MS 150  // Duracion del beep cuando pita
#define WARNING_RETRY_DELAY_MS 3000

// Estados de la central
enum CentralMode
{
    MODE_IDLE = 0,
    MODE_ALERT = 1,
    MODE_WARNING = 2
};

// Variables globales de estado
CentralMode currentMode = MODE_IDLE;
bool alertActive = false;
bool alertIsManual = false;
String currentAlertUuid = "";
String alertStartTimestamp = "";
unsigned long alertStartMillis = 0;

// Control de senseo
unsigned long lastSenseMillis = 0;
String lastWindowTimestamp = "";

// Estados de botones
bool lastStartPressed = false;
bool lastRealPressed = false;
bool lastFalsePressed = false;

// Control del patron de alerta
unsigned long lastAlertPatternMillis = 0;
bool alertBeepOn = false;
bool alertStrobeOn = false;

// Variables para modo WARNING
bool warningActive = false;
String warningFailureReason = "";
unsigned long lastWarningRetryMillis = 0;
String pendingOperationType = "";
String pendingHttpMethod = "";  // "GET", "POST", "PUT"
String pendingPath = "";
String pendingPayload = "";

// Control del patron de warning
unsigned long lastWarningPatternMillis = 0;
bool warningBeepOn = false;
bool warningStrobeOn = false;

// Prototipos
void connectWiFi();
void ensureWiFi();
void updateConnectionLed();
void setLedWifiConnected();
void setLedWifiDisconnected();
void setLedAlertMode();
void setLedWarningMode();
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
    float &avgDistance, bool &hasDistance
);

String generateUuid();
bool sendCreateAlert(const String &uuid, const String &timestampStarted, int numSensorsTriggered, const char *alertType);
bool sendUpdateAlert(const String &uuid, const String &timestampEnded, bool isReal, int responseSeconds);

void runSenseCycle();
void handleButtons();
void startAlert(bool manual, int numSensorsTriggered);
void endAlert(bool isReal);
void updateAlertEffects();
void handleWarningMode();
void updateWarningEffects();
void enterWarningMode(const String &reason, const String &httpMethod, const String &path, const String &payload);
void retryPendingHttpRequest();

// Helpers HTTP internos sin warning
bool httpGetJsonRaw(const String &path, DynamicJsonDocument &doc);
bool httpPostJsonRaw(const String &path, const String &payload);
bool httpPutJsonRaw(const String &path, const String &payload);

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------
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
        // Si falla aquí, ya debería estar en MODE_WARNING gracias a httpGetJson
        // No sobrescribir con updateConnectionLed()
    }
    else
    {
        // Solo actualizar LED si NO estamos en warning
        currentMode = MODE_IDLE;
        updateConnectionLed();
    }
}

// -----------------------------------------------------------------------------
// Loop principal
// -----------------------------------------------------------------------------
void loop()
{
    handleButtons();

    if (currentMode == MODE_WARNING)
    {
        handleWarningMode();
    }
    else if (alertActive)
    {
        updateAlertEffects();
    }
    else
    {
        noTone(BUZZER);
        digitalWrite(STROBE, LOW);
    }

    unsigned long now = millis();

    if (!alertActive && currentMode != MODE_WARNING && (now - lastSenseMillis >= SENSE_INTERVAL_MS))
    {
        lastSenseMillis = now;
        runSenseCycle();
    }

    if (!alertActive && currentMode != MODE_WARNING)
    {
        updateConnectionLed();
    }

    delay(BUTTON_POLL_DELAY_MS);
}

// -----------------------------------------------------------------------------
// Gestion de WiFi
// -----------------------------------------------------------------------------
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
    if (currentMode == MODE_ALERT)
    {
        setLedAlertMode();
        return;
    }

    if (currentMode == MODE_WARNING)
    {
        setLedWarningMode();
        return;
    }

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

void setLedWarningMode()
{
    // Amarillo (Rojo + Verde)
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

// -----------------------------------------------------------------------------
// Helpers HTTP INTERNOS (Sin warning - para reintentos)
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Helpers HTTP PUBLICOS (Con warning automático)
// -----------------------------------------------------------------------------
bool httpGetJson(const String &path, DynamicJsonDocument &doc)
{
    bool success = httpGetJsonRaw(path, doc);
    
    // Entrar a warning SIEMPRE que falle, excepto si ya estamos en warning
    // (evita re-entrar constantemente)
    if (!success && !warningActive)
    {
        String reason = "HTTP GET fallo";
        enterWarningMode(reason, "GET", path, "");
    }

    return success;
}

bool httpPostJson(const String &path, const String &payload)
{
    bool success = httpPostJsonRaw(path, payload);
    
    if (!success && !warningActive)
    {
        String reason = "HTTP POST fallo";
        enterWarningMode(reason, "POST", path, payload);
    }

    return success;
}

bool httpPutJson(const String &path, const String &payload)
{
    bool success = httpPutJsonRaw(path, payload);
    
    if (!success && !warningActive)
    {
        String reason = "HTTP PUT fallo";
        enterWarningMode(reason, "PUT", path, payload);
    }

    return success;
}

// -----------------------------------------------------------------------------
// /Time
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// /Readings/since/{timestamp}
// -----------------------------------------------------------------------------
bool getReadingsSince(
    const String &sinceTimestamp,
    float &avgTemp, bool &hasTemp,
    float &avgHum, bool &hasHum,
    float &avgLight, bool &hasLight,
    float &avgSmoke, bool &hasSmoke,
    float &avgPir, bool &hasPir,
    float &avgDistance, bool &hasDistance
)
{
    hasTemp = false;
    hasHum = false;
    hasLight = false;
    hasSmoke = false;
    hasPir = false;
    hasDistance = false;

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

    if (doc.containsKey("temperature"))
    {
        JsonArray arr = doc["temperature"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            sumTemp += obj["value"].as<double>();
            countTemp++;
        }
    }

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
            sumHum += obj["value"].as<double>();
            countHum++;
        }
    }

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
            sumLight += obj["value"].as<double>();
            countLight++;
        }
    }

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
            sumSmoke += obj["value"].as<double>();
            countSmoke++;
        }
    }

    if (doc.containsKey("pir"))
    {
        JsonArray arr = doc["pir"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
                continue;
            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
                continue;
            sumPir += obj["value"].as<bool>() ? 1.0 : 0.0;
            countPir++;
        }
    }

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
            sumDistance += obj["value"].as<double>();
            countDistance++;
        }
    }

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

    Serial.println("\nResumen promedio de la ventana:");
    if (hasTemp) { Serial.print("  Temp promedio: "); Serial.println(avgTemp); }
    else { Serial.println("  Temp promedio: sin datos"); }
    
    if (hasHum) { Serial.print("  Humedad promedio: "); Serial.println(avgHum); }
    else { Serial.println("  Humedad promedio: sin datos"); }
    
    if (hasLight) { Serial.print("  Luz promedio: "); Serial.println(avgLight); }
    else { Serial.println("  Luz promedio: sin datos"); }
    
    if (hasSmoke) { Serial.print("  Humo promedio: "); Serial.println(avgSmoke); }
    else { Serial.println("  Humo promedio: sin datos"); }
    
    if (hasPir) { Serial.print("  PIR promedio (0-1): "); Serial.println(avgPir); }
    else { Serial.println("  PIR promedio: sin datos"); }
    
    if (hasDistance) { Serial.print("  Distancia promedio (cm): "); Serial.println(avgDistance); }
    else { Serial.println("  Distancia promedio: sin datos"); }

    return true;
}

// -----------------------------------------------------------------------------
// UUID
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// /Alert (POST y PUT)
// -----------------------------------------------------------------------------
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

    Serial.println("\nEnviando alerta (POST /Alert)...");
    Serial.println(payload);

    bool ok = httpPostJson(String(ALERT_ENDPOINT), payload);

    if (ok)
    {
        Serial.println("Alerta creada correctamente en el servidor.");
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

    Serial.println("\nActualizando alerta (PUT /Alert/{uuid})...");
    Serial.println(payload);

    bool ok = httpPutJson(path, payload);

    if (ok)
    {
        Serial.println("Alerta actualizada correctamente en el servidor.");
    }

    return ok;
}

// -----------------------------------------------------------------------------
// Ronda de senseo
// -----------------------------------------------------------------------------
void runSenseCycle()
{
    Serial.println("\n================================");
    Serial.println("Iniciando ronda de senseo simple");
    Serial.println("================================");

    if (lastWindowTimestamp.length() == 0)
    {
        Serial.println("No hay timestamp previo, solicitando /Time para referencia de ventana.");
        if (!getServerTime(lastWindowTimestamp))
        {
            Serial.println("No se pudo obtener timestamp inicial, abortando ronda.\n");
            return;
        }
    }

    float avgTemp, avgHum, avgLight, avgSmoke, avgPir, avgDistance;
    bool hasTemp, hasHum, hasLight, hasSmoke, hasPir, hasDistance;

    bool ok = getReadingsSince(
        lastWindowTimestamp,
        avgTemp, hasTemp,
        avgHum, hasHum,
        avgLight, hasLight,
        avgSmoke, hasSmoke,
        avgPir, hasPir,
        avgDistance, hasDistance
    );

    if (!ok)
    {
        Serial.println("Error obteniendo lecturas, no se evaluara alerta automatica en este ciclo.\n");
        String newTs;
        if (getServerTime(newTs))
        {
            lastWindowTimestamp = newTs;
        }
        return;
    }

    Serial.println("\nEvaluando condiciones de alerta automatica (OR)...");

    bool fire = false;
    int sensorsTriggered = 0;

    if (hasTemp && avgTemp > TEMP_THRESHOLD_C)
    {
        fire = true;
        sensorsTriggered++;
        Serial.println("Condicion de alerta: temperatura alta supero umbral.");
    }

    if (hasHum && avgHum < HUM_THRESHOLD_PERCENT)
    {
        fire = true;
        sensorsTriggered++;
        Serial.println("Condicion de alerta: humedad baja supero umbral.");
    }

    if (hasLight && avgLight > LIGHT_THRESHOLD)
    {
        fire = true;
        sensorsTriggered++;
        Serial.println("Condicion de alerta: luz baja supero umbral.");
    }

    if (hasSmoke && avgSmoke > SMOKE_THRESHOLD)
    {
        fire = true;
        sensorsTriggered++;
        Serial.println("Condicion de alerta: humo alto supero umbral.");
    }

    if (fire && !alertActive)
    {
        Serial.print("\nAlerta automatica disparada. Sensores involucrados: ");
        Serial.println(sensorsTriggered);
        startAlert(false, sensorsTriggered);
    }
    else
    {
        Serial.println("\nNo se disparo alerta automatica en esta ronda.");
    }

    String newTs;
    if (getServerTime(newTs))
    {
        lastWindowTimestamp = newTs;
    }

    Serial.println("\nFin de la ronda de senseo, vectores descartados.\n");
}

// -----------------------------------------------------------------------------
// Manejo de botones
// -----------------------------------------------------------------------------
void handleButtons()
{
    bool startPressed = (digitalRead(BTN_START) == LOW);
    bool realPressed = (digitalRead(BTN_REAL) == LOW);
    bool falsePressed = (digitalRead(BTN_FALSE) == LOW);

    if (startPressed && !lastStartPressed && !alertActive && currentMode != MODE_WARNING)
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

// -----------------------------------------------------------------------------
// Inicio y fin de alertas
// -----------------------------------------------------------------------------
void startAlert(bool manual, int numSensorsTriggered)
{
    alertActive = true;
    alertIsManual = manual;
    currentMode = MODE_ALERT;
    alertStartMillis = millis();

    setLedAlertMode();

    alertBeepOn = false;
    alertStrobeOn = false;
    lastAlertPatternMillis = millis();

    String serverNow;
    if (!getServerTime(serverNow))
    {
        Serial.println("No se pudo obtener timestamp de inicio, la alerta fisica se activa de todos modos.");
        alertStartTimestamp = "";
        currentAlertUuid = "";
        return;
    }

    alertStartTimestamp = serverNow;
    currentAlertUuid = generateUuid();

    const char *typeStr = manual ? "manual" : "automatic";

    sendCreateAlert(currentAlertUuid, alertStartTimestamp, numSensorsTriggered, typeStr);
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

    Serial.print("Tiempo de alerta activa: ");
    Serial.print(elapsedMillis);
    Serial.println(" ms");
    Serial.print("Tiempo de respuesta: ");
    Serial.print(responseSeconds);
    Serial.println(" segundos");

    String serverNow;
    if (!getServerTime(serverNow))
    {
        Serial.println("No se pudo obtener timestamp de fin. Si hay warning activo, permanece. Si no, se limpia.");
        // Solo limpiar si NO estamos en warning
        if (!warningActive)
        {
            alertActive = false;
            currentMode = MODE_IDLE;
            currentAlertUuid = "";
            alertStartTimestamp = "";
            alertStartMillis = 0;
            updateConnectionLed();
        }
        return;
    }

    if (currentAlertUuid.length() > 0)
    {
        sendUpdateAlert(currentAlertUuid, serverNow, isReal, responseSeconds);
    }
    else
    {
        Serial.println("No hay UUID de alerta almacenado, no se puede hacer PUT de fin.");
    }

    // Solo cambiar estado si no entramos en warning por el sendUpdateAlert
    if (!warningActive)
    {
        alertActive = false;
        currentMode = MODE_IDLE;
        currentAlertUuid = "";
        alertStartTimestamp = "";
        alertStartMillis = 0;
        updateConnectionLed();
    }
    else
    {
        Serial.println("Alerta desactivada pero en WARNING, esperando que se resuelva HTTP...");
    }
}

// -----------------------------------------------------------------------------
// Patron de alerta
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// Modo WARNING - Reintento de CUALQUIER operacion HTTP fallida
// Incluyendo GET, POST y PUT
// Se detiene TODO hasta poder postear el warning exitosamente
// -----------------------------------------------------------------------------
void enterWarningMode(const String &reason, const String &httpMethod, const String &path, const String &payload)
{
    if (warningActive)
    {
        return;
    }

    warningActive = true;
    currentMode = MODE_WARNING;
    warningFailureReason = reason;
    pendingHttpMethod = httpMethod;
    pendingPath = path;
    pendingPayload = payload;
    lastWarningRetryMillis = millis();
    lastWarningPatternMillis = millis();
    warningBeepOn = false;
    warningStrobeOn = false;

    setLedWarningMode();

    Serial.println("\n========================================");
    Serial.println("=== ENTRANDO EN MODO WARNING ===");
    Serial.println("========================================");
    Serial.print("Razon: ");
    Serial.println(reason);
    Serial.print("Metodo HTTP: ");
    Serial.println(httpMethod);
    Serial.print("Path: ");
    Serial.println(path);
    Serial.println("\nSistema BLOQUEADO. Reintentando cada 3 segundos...");
    Serial.println("LED: AMARILLO | Buzzer: ON (patron lento)");
    Serial.println("========================================\n");
}

void handleWarningMode()
{
    updateWarningEffects();

    unsigned long now = millis();

    if (now - lastWarningRetryMillis >= WARNING_RETRY_DELAY_MS)
    {
        lastWarningRetryMillis = now;
        retryPendingHttpRequest();
    }
}

void postWarningToApi()
{
    DynamicJsonDocument doc(512);
    
    doc["idDevice"] = CENTRAL_DEVICE_ID;
    doc["message"] = "Fallo HTTP al conectar con API central";
    
    // Mapear el método HTTP a warningType
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

void retryPendingHttpRequest()
{
    if (!warningActive)
    {
        return;
    }

    bool success = false;

    Serial.print("\n[RETRY] Reintentando ");
    Serial.print(pendingHttpMethod);
    Serial.print(" en: ");
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
        // Ahora que logro la operacion, posteo el warning a BD
        postWarningToApi();

        Serial.println("\n========================================");
        Serial.println("=== SALIENDO DE MODO WARNING ===");
        Serial.println("Operacion completada exitosamente!");
        Serial.println("========================================\n");
        
        // IMPORTANTE: Limpiar TODOS los estados
        warningActive = false;
        currentMode = MODE_IDLE;
        pendingHttpMethod = "";
        pendingPath = "";
        pendingPayload = "";
        warningFailureReason = "";
        
        // Resetear timestamp para evitar alertas falsas en el siguiente senseo
        // (después de un warning, los datos pueden ser antiguos)
        if (!getServerTime(lastWindowTimestamp))
        {
            Serial.println("[WARNING] No se pudo obtener timestamp nuevo, usando timestamp anterior");
        }
        else
        {
            Serial.println("[RECOVERY] Timestamp reseteado para senseo limpio");
        }
        
        // Si hay una alerta activa, volver a modo alerta
        if (alertActive)
        {
            currentMode = MODE_ALERT;
            Serial.println("Alerta sigue activa, volviendo a MODE_ALERT");
        }
        
        clearWarningOutputs();
        updateConnectionLed();
    }
    else
    {
        Serial.println("[RETRY] Fallo nuevamente. Reintentando en 3 segundos...");
    }
}

void updateWarningEffects()
{
    unsigned long now = millis();

    // Patrón de LED amarillo parpadeante
    // 0.7s prendido, 0.7s apagado
    unsigned long cycleTime = (now - lastWarningPatternMillis) % (WARNING_BLINK_MS * 2);
    bool ledOn = cycleTime < WARNING_BLINK_MS;

    if (ledOn)
    {
        setLedWarningMode();  // Amarillo ON
    }
    else
    {
        // LED OFF (negro)
        digitalWrite(LED_R, LOW);
        digitalWrite(LED_G, LOW);
        digitalWrite(LED_B, LOW);
    }

    // Buzzer cada 2 parpadeos (cada 2.8 segundos)
    // 1 parpadeo = 1.4s (700ms on + 700ms off)
    // 2 parpadeos = 2.8s
    unsigned long buzzerCycleTime = (now - lastWarningPatternMillis) % (WARNING_BLINK_MS * 4);
    bool buzzerTime = (buzzerCycleTime < WARNING_BEEP_DURATION_MS);

    int freq = map(analogRead(POT_PIN), 0, 1023, WARNING_BEEP_MIN_HZ, WARNING_BEEP_MAX_HZ);
    if (freq < WARNING_BEEP_MIN_HZ)
        freq = WARNING_BEEP_MIN_HZ;
    if (freq > WARNING_BEEP_MAX_HZ)
        freq = WARNING_BEEP_MAX_HZ;

    if (buzzerTime)
    {
        tone(BUZZER, freq);
    }
    else
    {
        noTone(BUZZER);
    }

    // El estrobo ya no se usa, lo dejamos OFF
    digitalWrite(STROBE, LOW);
}