#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Times
#define WIFI_RETRY_DELAY 500
#define SENSOR_INTERVAL 800  // ← 800ms
#define POST_BLINK_TIME 100

// Pins
#define LED_VERDE D6
#define LED_ROJO D7
#define TRIG D4
#define ECHO D5

// WiFi
const char* ssid = "iPhone de Arturo";
const char* password = "123456789";

// API endpoint
const char *apiUrl = "http://172.20.10.5:5073/Distance";

// Smooth filter
float emaDist = -1;
const float alpha = 0.25;

unsigned long lastRead = 0;

void connectWiFi()
{
    Serial.println("\nConectando a WiFi...\n");

    WiFi.begin(ssid, password);

    digitalWrite(LED_ROJO, HIGH);
    digitalWrite(LED_VERDE, LOW);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(WIFI_RETRY_DELAY);
        Serial.print(".");
    }

    Serial.println("\n\nConectado al WiFi");
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());

    digitalWrite(LED_ROJO, LOW);
    digitalWrite(LED_VERDE, HIGH);
}

void restoreWiFiState()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        digitalWrite(LED_VERDE, HIGH);
        digitalWrite(LED_ROJO, LOW);
    }
    else
    {
        digitalWrite(LED_VERDE, LOW);
        digitalWrite(LED_ROJO, HIGH);
    }
}

void blinkOK()
{
    digitalWrite(LED_VERDE, LOW);
    delay(POST_BLINK_TIME);
    digitalWrite(LED_VERDE, HIGH);
    delay(POST_BLINK_TIME);
    restoreWiFiState();
}

void blinkError()
{
    digitalWrite(LED_ROJO, LOW);
    delay(POST_BLINK_TIME);
    digitalWrite(LED_ROJO, HIGH);
    delay(POST_BLINK_TIME);
    restoreWiFiState();
}

float readRawDistance()
{
    digitalWrite(TRIG, LOW);
    delayMicroseconds(4);

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(12);
    digitalWrite(TRIG, LOW);

    long duration = pulseIn(ECHO, HIGH, 30000);

    if (duration == 0)
    {
        return -1;
    }

    float dist = duration * 0.034 / 2;
    return dist;
}

float readFilteredDistance()
{
    float sum = 0;
    int validCount = 0;

    // Aumenté a 10 muestras para más datos
    for (int i = 0; i < 10; i++)
    {
        float d = readRawDistance();
        // Rango mucho más tolerante: 1cm a 500cm
        if (d > 1 && d < 500)
        {
            sum += d;
            validCount++;
        }
        delay(5);
    }

    // Si al menos 3 lecturas válidas, usa el promedio
    // Si menos de 3, devuelve -1
    if (validCount < 3)
    {
        return -1;
    }

    float avg = sum / validCount;

    // Primera inicialización del EMA
    if (emaDist < 0)
    {
        emaDist = avg;
        return avg;
    }

    // Aplicar suavizado exponencial
    emaDist = alpha * avg + (1 - alpha) * emaDist;

    return emaDist;
}

void setup()
{
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);

    Serial.begin(115200);
    delay(300);

    Serial.println("\nNodo Ultrasonico iniciado...\n");

    connectWiFi();
}

void loop()
{
    unsigned long now = millis();

    if (now - lastRead >= SENSOR_INTERVAL)
    {
        lastRead = now;

        Serial.println("\n-----------------------------");
        Serial.println("Leyendo HC-SR04...");

        float dist = readFilteredDistance();

        if (dist < 0)
        {
            Serial.println("Lectura inválida del sensor ultrasónico");
        }
        else
        {
            Serial.print("Distancia (cm): ");
            Serial.println(dist);
        }

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Sin conexión, reintentando WiFi...");
            connectWiFi();
            return;
        }

        HTTPClient http;
        WiFiClient client;

        http.begin(client, apiUrl);
        http.addHeader("Content-Type", "application/json");

        String json;

        if (dist < 0)
        {
            json = "{\"idDevice\":4,\"distanceCm\":null}";
        }
        else
        {
            json = String("{\"idDevice\":4,\"distanceCm\":") + dist + "}";
        }

        Serial.println("\nEnviando POST...");
        Serial.println(json);

        int code = http.POST(json);

        Serial.print("Código respuesta servidor: ");
        Serial.println(code);

        if (code == 200)
        {
            blinkOK();
        }
        else
        {
            blinkError();
        }

        http.end();
    }
}