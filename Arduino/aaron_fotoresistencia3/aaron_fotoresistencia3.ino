#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Times
#define WIFI_RETRY_DELAY 500
#define SENSOR_INTERVAL 2500
#define POST_BLINK_TIME 100

// Pins
#define LED_VERDE D6
#define LED_ROJO D7
#define PHOTO A0

// WiFi
const char *ssid = "Mi perro cuando";
const char *password = "SggUD6o4rWN?7IaOdHqkXv2HB";

// API endpoint
const char *apiUrl = "http://192.168.1.166:5071/Light";

// Smoothing filter
float emaValue = -1;
const float alpha = 0.25; // suavizado exponencial

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

int readFilteredLight()
{
    int raw1 = analogRead(PHOTO);
    delay(5);
    int raw2 = analogRead(PHOTO);
    delay(5);
    int raw3 = analogRead(PHOTO);

    int avg = (raw1 + raw2 + raw3) / 3;

    if (emaValue < 0)
    {
        emaValue = avg;
        return avg;
    }

    emaValue = alpha * avg + (1 - alpha) * emaValue;

    return (int)emaValue;
}

void setup()
{
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);

    Serial.begin(115200);
    delay(300);

    Serial.println("\nNodo Light iniciado...\n");

    connectWiFi();
}

void loop()
{
    unsigned long now = millis();

    if (now - lastRead >= SENSOR_INTERVAL)
    {
        lastRead = now;

        Serial.println("\n-----------------------------");
        Serial.println("Leyendo fotoresistencia...");

        int value = readFilteredLight();

        Serial.print("Valor luz (0-1023): ");
        Serial.println(value);

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

        String json =
            String("{\"idDevice\":2,\"value\":") + value + "}";

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
