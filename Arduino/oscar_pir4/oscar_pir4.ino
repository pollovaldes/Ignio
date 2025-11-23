#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Times
#define WIFI_RETRY_DELAY 500
#define DEBOUNCE_TIME 600
#define POST_BLINK_TIME 100

// Pins
#define PIR D8
#define LED_VERDE D2
#define LED_ROJO D0
#define LED_MOVIMIENTO D1

// WiFi
const char *ssid = "Mi perro cuando";
const char *password = "SggUD6o4rWN?7IaOdHqkXv2HB";

// API URL
const char *apiUrl = "http://192.168.1.166:5074/Pir";

// State
int lastReadState = LOW;
int lastStableState = LOW;
bool movementActive = false;
unsigned long lastChangeMs = 0;
unsigned long movementStartMs = 0;
int eventCounter = 0;


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
    pinMode(PIR, INPUT);
    pinMode(LED_VERDE, OUTPUT);
    pinMode(LED_ROJO, OUTPUT);
    pinMode(LED_MOVIMIENTO, OUTPUT);

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);
    digitalWrite(LED_MOVIMIENTO, LOW);

    Serial.begin(115200);
    delay(300);
    Serial.println("\nNodo PIR iniciado...\n");

    connectWiFi();
}

void loop()
{
    int current = digitalRead(PIR);
    unsigned long now = millis();

    // Cambio crudo detectado
    if (current != lastReadState)
    {
        lastReadState = current;
        lastChangeMs = now;
    }

    // Estabilización por debounce
    if ((now - lastChangeMs) >= DEBOUNCE_TIME && current != lastStableState)
    {
        lastStableState = current;

        // INICIO movimiento
        if (current == HIGH && !movementActive)
        {
            movementActive = true;
            movementStartMs = now;
            eventCounter++;

            digitalWrite(LED_MOVIMIENTO, HIGH);

            Serial.println("\n-----------------------------");
            Serial.print("Evento PIR #");
            Serial.println(eventCounter);
            Serial.println("Movimiento detectado");
        }

        // FIN movimiento → manda POST
        else if (current == LOW && movementActive)
        {
            movementActive = false;
            digitalWrite(LED_MOVIMIENTO, LOW);

            float durationSec = (now - movementStartMs) / 1000.0;

            Serial.println("\nMovimiento finalizado:");
            Serial.print("Duración: ");
            Serial.print(durationSec, 2);
            Serial.println(" s");

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

            // <-- FORMATO EXACTO QUE PIDE TU API
            String json =
                String("{\"idDevice\":5,\"durationSeconds\":") +
                durationSec +
                ",\"eventNumber\":" +
                eventCounter +
                "}";

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
}
