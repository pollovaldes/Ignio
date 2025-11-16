#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// WiFi configuration
const char* ssid = "Tec-IoT";
const char* password = "spotless.magnetic.bridge";
const char* apiServer = "http://10.22.154.227:5074/Light"; // Endpoint correcto

// Device configuration
#define DEVICE_ID 3 // 3 corresponde a este sensor de luz

// Timings
#define INTERVAL_MS 3000 // Interval to send POST requests
#define RECONNECT_DELAY_MS 5000 // Wait time before retrying WiFi connection

// RGB LED pins
#define PIN_RED D5
#define PIN_GREEN D6
#define PIN_BLUE D7

// Photoresistor pin
#define PIN_LIGHT A0 // Entrada analógica

// LED state: connecting
void setLedConnecting()
{
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 1023);
}

// LED state: connected
void setLedConnected()
{
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 1023);
    analogWrite(PIN_BLUE, 0);
}

// LED state: disconnected
void setLedDisconnected()
{
    analogWrite(PIN_RED, 1023);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
}

// Connect to WiFi with retries
void connectToWiFi()
{
    Serial.println("");
    Serial.println("Starting WiFi connection...");
    WiFi.begin(ssid, password);

    Serial.print("Connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        setLedConnecting();
        delay(500);
        Serial.print(".");

        if (WiFi.status() == WL_CONNECT_FAILED)
        {
            Serial.println("");
            Serial.println("WiFi connection failed, retrying in 5 seconds...");
            setLedDisconnected();
            delay(RECONNECT_DELAY_MS);
            WiFi.begin(ssid, password);
        }
    }

    Serial.println("");
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
    setLedConnected();
}

// Read raw light analog value
int readRawLight()
{
    int raw = analogRead(PIN_LIGHT);

    if (raw < 0 || raw > 1023)
    {
        return -1;
    }

    return raw;
}

// Filtering: average multiple samples to reduce noise
int getFilteredLight()
{
    const int SAMPLES = 8;
    long sum = 0;
    int valid = 0;

    for (int i = 0; i < SAMPLES; i++)
    {
        int value = readRawLight();
        if (value >= 0)
        {
            sum += value;
            valid++;
        }
        delay(5);
    }

    if (valid == 0)
    {
        return -1;
    }

    return sum / valid;
}

// Send POST request to API
void sendReading(int lightValue)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Cannot send POST. WiFi is disconnected.");
        return;
    }

    WiFiClient client;
    HTTPClient http;

    http.begin(client, apiServer);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<200> json;

    json["idDevice"] = DEVICE_ID; // Nombre correcto según Swagger

    if (lightValue < 0)
    {
        json["value"] = nullptr; // Para lecturas inválidas
    }
    else
    {
        json["value"] = lightValue; // Fotoresistencia 0-1023
    }

    String body;
    serializeJson(json, body);

    Serial.println("Sending POST request...");
    Serial.println("Payload:");
    Serial.println(body);

    int httpResponse = http.POST(body);

    if (httpResponse > 0)
    {
        Serial.print("Server response code: ");
        Serial.println(httpResponse);
    }
    else
    {
        Serial.print("POST failed: ");
        Serial.println(httpResponse);
    }

    http.end();
}

// Setup
void setup()
{
    Serial.begin(115200);

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    connectToWiFi();
}

// Loop
void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi lost, reconnecting...");
        setLedDisconnected();
        connectToWiFi();
    }

    Serial.println("Reading photoresistor...");

    int filteredLight = getFilteredLight();

    if (filteredLight < 0)
    {
        Serial.println("Invalid light reading.");
    }
    else
    {
        Serial.print("Light value (0-1023): ");
        Serial.println(filteredLight);
    }

    sendReading(filteredLight);

    delay(INTERVAL_MS);
}
