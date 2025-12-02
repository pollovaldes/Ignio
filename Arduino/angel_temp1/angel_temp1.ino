#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// Times
#define WIFI_RETRY_DELAY 500
#define SENSOR_INTERVAL 3500
#define POST_BLINK_TIME 100

// Pins
#define LED_VERDE D2
#define LED_ROJO D1
#define DHTPIN D5
#define DHTTYPE DHT11

// WiFi
const char* ssid = "iPhone de Arturo";
const char* password = "123456789";

// API endpoint
const char *apiUrl = "http://172.20.10.5:5072/Dht11";

DHT dht(DHTPIN, DHTTYPE);
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

void setup()
{
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);

    Serial.begin(115200);
    delay(300);

    Serial.println("\nNodo DHT11 iniciando...\n");

    dht.begin();
    connectWiFi();
}

void loop()
{
    unsigned long now = millis();

    if (now - lastRead >= SENSOR_INTERVAL)
    {
        lastRead = now;

        Serial.println("\n-----------------------------");
        Serial.println("Leyendo sensor DHT11...");

        float h = dht.readHumidity();
        float t = dht.readTemperature();

        Serial.print("Temperatura: ");
        Serial.println(t);
        Serial.print("Humedad: ");
        Serial.println(h);

        bool valid = !(isnan(h) || isnan(t));

        if (!valid)
        {
            Serial.println("Lectura inválida del DHT11");
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

        if (valid)
        {
            json =
                String("{\"idDevice\":3,\"temperature\":") + t +
                ",\"humidity\":" + h + "}";
        }
        else
        {
            json =
                "{\"idDevice\":3,\"temperature\":null,\"humidity\":null}";
        }

        Serial.println("\nEnviando POST...");
        Serial.println(json);

        int code = http.POST(json);

        Serial.print("Código respuesta servidor: ");
        Serial.println(code);

        if (code == 200)
        {
            blinkOK(); // Verde
        }
        else
        {
            blinkError(); // Rojo
        }

        http.end();
    }
}
