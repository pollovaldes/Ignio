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
const char *WARNING_ENDPOINT = "/Warning";   // De momento no se usa en la logica

// Umbrales de alerta (puedes ajustarlos despues)
#define TEMP_THRESHOLD_C 32.0f       // Temperatura alta
#define HUM_THRESHOLD_PERCENT 40.0f  // Humedad baja
#define LIGHT_THRESHOLD 300.0f   // Luz baja (oscurecimiento por humo/fuego)
#define SMOKE_THRESHOLD 350.0f       // Humo alto

// Patron de alerta para buzzer y estrobo
#define ALERT_BEEP_MIN_HZ 1200       // Frecuencia minima del buzzer
#define ALERT_BEEP_MAX_HZ 2600       // Frecuencia maxima del buzzer
#define ALERT_BEEP_ON_MS 120         // Tiempo de tono encendido
#define ALERT_BEEP_OFF_MS 180        // Tiempo de silencio
#define ALERT_STROBE_ON_MS 80        // Tiempo estrobo encendido
#define ALERT_STROBE_OFF_MS 120      // Tiempo estrobo apagado

// Estados de la central
enum CentralMode
{
    MODE_IDLE = 0,
    MODE_ALERT = 1
};

// Variables globales de estado
CentralMode currentMode = MODE_IDLE;
bool alertActive = false;
bool alertIsManual = false;
String currentAlertUuid = "";
String alertStartTimestamp = "";
unsigned long alertStartMillis = 0;  // NUEVO: Tiempo local cuando comienza la alerta

// Control de senseo
unsigned long lastSenseMillis = 0;
String lastWindowTimestamp = "";   // Timestamp de referencia del servidor para /Readings/since/{timestamp}

// Estados de botones (para detectar flancos)
bool lastStartPressed = false;
bool lastRealPressed = false;
bool lastFalsePressed = false;

// Control del patron de alerta
unsigned long lastAlertPatternMillis = 0;
bool alertBeepOn = false;
bool alertStrobeOn = false;

// Prototipos
void connectWiFi();
void ensureWiFi();
void updateConnectionLed();
void setLedWifiConnected();
void setLedWifiDisconnected();
void setLedAlertMode();
void clearAlertOutputs();

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
bool sendWarning(const String &sensorType, const String &message); // Preparado para el futuro

void runSenseCycle();
void handleButtons();
void startAlert(bool manual, int numSensorsTriggered);
void endAlert(bool isReal);
void updateAlertEffects();

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

    // Intentar obtener un timestamp inicial del servidor para el primer /Readings/since
    if (!getServerTime(lastWindowTimestamp))
    {
        Serial.println("No se pudo obtener timestamp inicial del servidor, usando valor vacio.\n");
        lastWindowTimestamp = "";
    }

    currentMode = MODE_IDLE;
    updateConnectionLed();
}

// -----------------------------------------------------------------------------
// Loop principal
// -----------------------------------------------------------------------------
void loop()
{
    handleButtons();

    if (alertActive)
    {
        updateAlertEffects();
    }
    else
    {
        noTone(BUZZER);
        digitalWrite(STROBE, LOW);
    }

    unsigned long now = millis();

    if (!alertActive && (now - lastSenseMillis >= SENSE_INTERVAL_MS))
    {
        lastSenseMillis = now;
        runSenseCycle();
    }

    if (!alertActive)
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

void clearAlertOutputs()
{
    noTone(BUZZER);
    digitalWrite(STROBE, LOW);
}

// -----------------------------------------------------------------------------
// Helpers HTTP
// -----------------------------------------------------------------------------
bool httpGetJson(const String &path, DynamicJsonDocument &doc)
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

bool httpPostJson(const String &path, const String &payload)
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

bool httpPutJson(const String &path, const String &payload)
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
            {
                continue;
            }

            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
            {
                continue;
            }

            double val = obj["value"].as<double>();
            sumTemp += val;
            countTemp++;
        }
    }

    if (doc.containsKey("humidity"))
    {
        JsonArray arr = doc["humidity"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
            {
                continue;
            }

            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
            {
                continue;
            }

            double val = obj["value"].as<double>();
            sumHum += val;
            countHum++;
        }
    }

    if (doc.containsKey("light"))
    {
        JsonArray arr = doc["light"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
            {
                continue;
            }

            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
            {
                continue;
            }

            double val = obj["value"].as<double>();
            sumLight += val;
            countLight++;
        }
    }

    if (doc.containsKey("smoke"))
    {
        JsonArray arr = doc["smoke"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
            {
                continue;
            }

            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
            {
                continue;
            }

            double val = obj["value"].as<double>();
            sumSmoke += val;
            countSmoke++;
        }
    }

    if (doc.containsKey("pir"))
    {
        JsonArray arr = doc["pir"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
            {
                continue;
            }

            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
            {
                continue;
            }

            bool val = obj["value"].as<bool>();
            sumPir += val ? 1.0 : 0.0;
            countPir++;
        }
    }

    if (doc.containsKey("distance"))
    {
        JsonArray arr = doc["distance"].as<JsonArray>();
        for (JsonVariant v : arr)
        {
            if (!v.is<JsonObject>())
            {
                continue;
            }

            JsonObject obj = v.as<JsonObject>();
            if (obj["value"].isNull())
            {
                continue;
            }

            double val = obj["value"].as<double>();
            sumDistance += val;
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
    if (hasTemp)
    {
        Serial.print("  Temp promedio: ");
        Serial.println(avgTemp);
    }
    else
    {
        Serial.println("  Temp promedio: sin datos");
    }

    if (hasHum)
    {
        Serial.print("  Humedad promedio: ");
        Serial.println(avgHum);
    }
    else
    {
        Serial.println("  Humedad promedio: sin datos");
    }

    if (hasLight)
    {
        Serial.print("  Luz promedio: ");
        Serial.println(avgLight);
    }
    else
    {
        Serial.println("  Luz promedio: sin datos");
    }

    if (hasSmoke)
    {
        Serial.print("  Humo promedio: ");
        Serial.println(avgSmoke);
    }
    else
    {
        Serial.println("  Humo promedio: sin datos");
    }

    if (hasPir)
    {
        Serial.print("  PIR promedio (0-1): ");
        Serial.println(avgPir);
    }
    else
    {
        Serial.println("  PIR promedio: sin datos");
    }

    if (hasDistance)
    {
        Serial.print("  Distancia promedio (cm): ");
        Serial.println(avgDistance);
    }
    else
    {
        Serial.println("  Distancia promedio: sin datos");
    }

    return true;
}

// -----------------------------------------------------------------------------
// UUID simple (no es RFC perfecto pero sirve para identificar alertas)
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
    else
    {
        Serial.println("Error al crear alerta en el servidor.");
    }

    return ok;
}

bool sendUpdateAlert(const String &uuid, const String &timestampEnded, bool isReal, int responseSeconds)
{
    DynamicJsonDocument doc(256);

    doc["timestampEnded"] = timestampEnded;
    doc["IsReal"] = isReal;                   // El C# usa IsReal (propiedad)
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
    else
    {
        Serial.println("Error al actualizar alerta en el servidor.");
    }

    return ok;
}

// -----------------------------------------------------------------------------
// /Warning (preparado para usarlo despues)
// -----------------------------------------------------------------------------
bool sendWarning(const String &sensorType, const String &message)
{
    DynamicJsonDocument doc(256);

    doc["idDevice"] = CENTRAL_DEVICE_ID;
    doc["sensorType"] = sensorType;
    doc["message"] = message;

    String payload;
    serializeJson(doc, payload);

    Serial.println("\nEnviando warning (POST /Warning)...");
    Serial.println(payload);

    bool ok = httpPostJson(String(WARNING_ENDPOINT), payload);

    if (ok)
    {
        Serial.println("Warning registrado en el servidor.");
    }
    else
    {
        Serial.println("Error al registrar warning en el servidor.");
    }

    return ok;
}

// -----------------------------------------------------------------------------
// Ronda de senseo simplificada (una sola ventana, promedios y OR)
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
// Manejo de botones (alerta manual y fin de alerta)
// -----------------------------------------------------------------------------
void handleButtons()
{
    bool startPressed = (digitalRead(BTN_START) == LOW);
    bool realPressed = (digitalRead(BTN_REAL) == LOW);
    bool falsePressed = (digitalRead(BTN_FALSE) == LOW);

    if (startPressed && !lastStartPressed && !alertActive)
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
    alertStartMillis = millis();  // NUEVO: Registrar el tiempo local cuando comienza

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

    // NUEVO: Calcular el tiempo de respuesta en segundos
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
        Serial.println("No se pudo obtener timestamp de fin, se desactiva alerta fisica pero no se registra fin en BD.");
        alertActive = false;
        currentMode = MODE_IDLE;
        currentAlertUuid = "";
        alertStartTimestamp = "";
        alertStartMillis = 0;
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

    alertActive = false;
    currentMode = MODE_IDLE;
    currentAlertUuid = "";
    alertStartTimestamp = "";
    alertStartMillis = 0;

    updateConnectionLed();
}

// -----------------------------------------------------------------------------
// Patron de alerta (buzzer + estrobo, mapeado con el potenciometro)
// -----------------------------------------------------------------------------
void updateAlertEffects()
{
    unsigned long now = millis();

    int pot = analogRead(POT_PIN);

    int freq = map(pot, 0, 1023, ALERT_BEEP_MIN_HZ, ALERT_BEEP_MAX_HZ);
    if (freq < ALERT_BEEP_MIN_HZ)
    {
        freq = ALERT_BEEP_MIN_HZ;
    }
    if (freq > ALERT_BEEP_MAX_HZ)
    {
        freq = ALERT_BEEP_MAX_HZ;
    }

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