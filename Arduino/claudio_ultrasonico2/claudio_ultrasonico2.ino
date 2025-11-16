#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// WiFi config
const char* ssid = "Tec-IoT";
const char* password = "spotless.magnetic.bridge";
const char* apiServer = "http://10.22.154.227:5074/Distance";

// Device configuration
#define DEVICE_ID 2 // 2 corresponde al sensor ultrasónico

// Timing
#define INTERVAL_MS 3000 // Interval to send POST requests
#define RECONNECT_DELAY_MS 5000 // Wait time before retrying WiFi connection

// RGB LED pins
#define PIN_RED 5   // D1
#define PIN_GREEN 4 // D2
#define PIN_BLUE 0  // D3

// Ultrasonic pins
#define PIN_TRIG 14 // D5
#define PIN_ECHO 12 // D6

// LED states
void setLedConnecting()
{
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 1023); // 100% blue
}

void setLedConnected()
{
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 1023); // 100% green
    analogWrite(PIN_BLUE, 0);
}

void setLedDisconnected()
{
    analogWrite(PIN_RED, 1023); // 100% red
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE, 0);
}

// Connect to WiFi with retry logic
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

// Ultrasonic raw measurement
float readRawDistance()
{
    digitalWrite(PIN_TRIG, LOW);
    delayMicroseconds(5);

    digitalWrite(PIN_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIG, LOW);

    long duration = pulseIn(PIN_ECHO, HIGH, 25000);

    if (duration == 0)
    {
        return NAN;
    }

    float distance = duration * 0.0343f / 2.0f;

    if (distance < 1 || distance > 450)
    {
        return NAN;
    }

    return distance;
}

// Filtering logic
float filteredDistanceWithOutlierRemoval()
{
    const int SAMPLES = 7; // número de lecturas
    float readings[SAMPLES];
    int count = 0;

    // Leer varias veces
    for (int i = 0; i < SAMPLES; i++)
    {
        float r = readRawDistance();
        if (!isnan(r))
        {
            readings[count] = r;
            count++;
        }
        delay(10);
    }

    if (count == 0)
    {
        return NAN;
    }

    // Ordenar para obtener mediana
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (readings[j] < readings[i])
            {
                float temp = readings[i];
                readings[i] = readings[j];
                readings[j] = temp;
            }
        }
    }

    float median;
    if (count % 2 == 1)
    {
        median = readings[count / 2];
    }
    else
    {
        median = (readings[count/2 - 1] + readings[count/2]) / 2.0f;
    }

    // Calcular desviaciones absolutas
    float deviations[SAMPLES];
    for (int i = 0; i < count; i++)
    {
        deviations[i] = fabs(readings[i] - median);
    }

    // Ordenar desviaciones
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (deviations[j] < deviations[i])
            {
                float temp = deviations[i];
                deviations[i] = deviations[j];
                deviations[j] = temp;
            }
        }
    }

    // Mediana de desviaciones (MAD)
    float mad;
    if (count % 2 == 1)
    {
        mad = deviations[count / 2];
    }
    else
    {
        mad = (deviations[count/2 - 1] + deviations[count/2]) / 2.0f;
    }

    if (mad == 0)
    {
        mad = 0.0001; // evitar división entre cero
    }

    // Remover outliers usando k = 2.5
    float sum = 0.0;
    int valid = 0;
    float k = 2.5;

    for (int i = 0; i < count; i++)
    {
        float deviation = fabs(readings[i] - median);

        if (deviation <= k * mad)
        {
            sum += readings[i];
            valid++;
        }
    }

    if (valid == 0)
    {
        return NAN;
    }

    // Promedio sin outliers
    float avg = sum / valid;

    // Filtro EMA encima del resultado
    static bool hasPrev = false;
    static float ema = 0.0;

    float alpha = 0.25;

    if (!hasPrev)
    {
        ema = avg;
        hasPrev = true;
    }
    else
    {
        ema = alpha * avg + (1.0 - alpha) * ema;
    }

    return ema;
}

// Send POST request
void sendReading(float distance)
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

    if (isnan(distance))
    {
        json["DistanceCm"] = nullptr;
    }
    else
    {
        json["DistanceCm"] = distance;
    }

    String requestBody;
    serializeJson(json, requestBody);

    Serial.println("Sending POST request...");
    Serial.println("Payload:");
    Serial.println(requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0)
    {
        Serial.print("Server response: ");
        Serial.println(httpResponseCode);
    }
    else
    {
        Serial.print("POST failed: ");
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

    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

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

    Serial.println("Reading distance sensor...");

    float filteredDistance = filteredDistanceWithOutlierRemoval();

    if (isnan(filteredDistance))
    {
        Serial.println("Invalid distance reading.");
    }
    else
    {
        Serial.print("Distance: ");
        Serial.print(filteredDistance);
        Serial.println(" cm");
    }

    sendReading(filteredDistance);

    delay(INTERVAL_MS);
}
