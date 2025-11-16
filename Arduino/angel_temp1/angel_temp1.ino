#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// WiFi configuration
const char* ssid = "Mi perro cuando";
const char* password = "SggUD6o4rWN?7IaOdHqkXv2HB";
const char* apiServer = "http://192.168.1.166:5074/Dht11";

// Device configuration
#define DEVICE_ID 1 // 1 corresponds to the humidity & temperature sensor

// Timings
#define INTERVAL_MS 10000 // Interval to send POST requests
#define RECONNECT_DELAY_MS 5000 // Wait time before retrying WiFi connection

// Pin configuration
#define PIN_DHT D4 // D4 in NodeMCU ESP8266 board
#define PIN_RED D0 // D8
#define PIN_GREEN D1 // D6
#define PIN_BLUE D2 // D7

// DHT sensor setup
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

// State helpers
void setLedConnecting()
{
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 1023); // Full blue
}

void setLedConnected()
{
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 1023); // Full green
    analogWrite(PIN_BLUE, 0);
}

void setLedDisconnected()
{
    analogWrite(PIN_RED, 1023); // Full red
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
}

// WiFi connection loop
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
            Serial.println("WiFi connection failed. Retrying in 5 seconds...");
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

// Send JSON data to API
void sendReading(float temperature, float humidity)
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

    json["IdDevice"] = DEVICE_ID;

    if (isnan(temperature))
    {
        json["Temperature"] = nullptr;
    }
    else
    {
        json["Temperature"] = temperature;
    }

    if (isnan(humidity))
    {
        json["Humidity"] = nullptr;
    }
    else
    {
        json["Humidity"] = humidity;
    }

    String requestBody;
    serializeJson(json, requestBody);

    Serial.println("Sending POST request...");
    Serial.println("Payload:");
    Serial.println(requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0)
    {
        Serial.print("Server response code: ");
        Serial.println(httpResponseCode);
    }
    else
    {
        Serial.print("Error sending POST: ");
        Serial.println(httpResponseCode);
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

    dht.begin();

    connectToWiFi();
}

// Loop
void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi lost. Attempting reconnection...");
        setLedDisconnected();
        connectToWiFi();
    }

    Serial.println("Reading DHT11 sensor...");

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Invalid DHT11 reading. Temperature or humidity is NaN.");
    }
    else
    {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C | Humidity: ");
        Serial.print(humidity);
        Serial.println(" %");
    }

    sendReading(temperature, humidity);

    delay(INTERVAL_MS);
}
