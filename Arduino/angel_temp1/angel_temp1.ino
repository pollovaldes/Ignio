/*#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

// Time intervals
#define WIFI_RETRY_DELAY 500
#define SENSOR_INTERVAL 3500
#define POST_BLINK_TIME 80

// Pins
#define LED_VERDE D2
#define LED_ROJO D1
#define DHTPIN D5
#define DHTTYPE DHT11

// WiFi
const char *ssid = "Mi perro cuando";
const char *password = "SggUD6o4rWN?7IaOdHqkXv2HB";

// API endpoint
const char *apiUrl = "http://192.168.1.166:5072/Dht11";

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastRead = 0;

void connectWiFi()
{
    Serial.println("\nConectando al WiFi...\n");

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

        float h = dht.readHumidity();
        float t = dht.readTemperature();

        Serial.println("\n-----------------------------");
        Serial.println("Leyendo sensor DHT11...");
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

        // Blink verde for POST feedback
        digitalWrite(LED_VERDE, LOW);
        delay(POST_BLINK_TIME);
        digitalWrite(LED_VERDE, HIGH);

        http.end();
    }
}*/


#include <DHT.h>

#define LED_VERDE D2
#define LED_ROJO D1
#define DHTPIN D5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO, LOW);

  Serial.begin(115200);
  delay(300);

  Serial.println("\nIniciando prueba DHT11 (bare minimum)...\n");

  dht.begin();
}

void loop() {
  Serial.println("-----------------------------");
  Serial.println("Leyendo sensor DHT11...");

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("ERROR: no se pudo leer el DHT11");

    digitalWrite(LED_ROJO, HIGH);
    digitalWrite(LED_VERDE, LOW);

  } else {
    Serial.print("Temperatura: ");
    Serial.println(t);
    Serial.print("Humedad: ");
    Serial.println(h);

    digitalWrite(LED_ROJO, LOW);
    digitalWrite(LED_VERDE, HIGH);
  }

  delay(1500);
}
