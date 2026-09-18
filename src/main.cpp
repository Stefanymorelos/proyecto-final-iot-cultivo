// ============================================================
// Proyecto Final IoT - Sistema de monitoreo de cultivo
// Firmware para ESP32
//
// Lee: humedad de suelo, temperatura/humedad ambiente (DHT22),
//      y luz (LDR). Envia los datos por WiFi a un servidor Flask
//      via HTTP POST en formato JSON.
//
// Autor: Tefa
// ============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "config.h"

DHT dht(PIN_DHT, DHTTYPE);

unsigned long ultimaLectura = 0;

// ------------------------------------------------------------
// Conecta a la red WiFi. Se queda intentando hasta lograrlo,
// mostrando progreso por el monitor serial.
// ------------------------------------------------------------
void conectarWiFi() {
    Serial.print("Conectando a WiFi: ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 40) {
        delay(500);
        Serial.print(".");
        intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("Conectado. IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println();
        Serial.println("No se pudo conectar al WiFi. Reintentando en el proximo ciclo.");
    }
}

// ------------------------------------------------------------
// Convierte la lectura cruda del ADC de humedad de suelo (0-4095)
// a un porcentaje 0-100%, usando los valores de calibracion
// definidos en config.h.
// ------------------------------------------------------------
float leerHumedadSuelo() {
    int valorCrudo = analogRead(PIN_HUMEDAD_SUELO);

    // mapea: SECO -> 0%, MOJADO -> 100%
    float porcentaje = map(valorCrudo, HUMEDAD_VALOR_SECO, HUMEDAD_VALOR_MOJADO, 0, 100);

    // por si el mapeo se sale de rango (ruido electrico, sensor desconectado)
    porcentaje = constrain(porcentaje, 0, 100);

    return porcentaje;
}

// ------------------------------------------------------------
// Lee el sensor de luz (LDR). Por ahora devuelve el valor crudo
// del ADC (0-4095); mas adelante se puede convertir a lux si
// se conoce la curva de respuesta del LDR especifico.
// ------------------------------------------------------------
int leerLuz() {
    return analogRead(PIN_LUZ);
}

// ------------------------------------------------------------
// Arma el JSON con todas las lecturas y lo envia por HTTP POST
// al servidor Flask.
// ------------------------------------------------------------
void enviarDatos(float humedadSuelo, float temperatura, float humedadAmbiente, int luz) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Sin WiFi, no se puede enviar. Se omite este ciclo.");
        return;
    }

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["humedad_suelo"] = humedadSuelo;
    doc["temperatura"] = temperatura;
    doc["humedad_ambiente"] = humedadAmbiente;
    doc["luz"] = luz;
    doc["timestamp_uptime_ms"] = millis(); // el servidor le pone el timestamp real al llegar

    String payload;
    serializeJson(doc, payload);

    Serial.println("Enviando: " + payload);

    int codigoRespuesta = http.POST(payload);

    if (codigoRespuesta > 0) {
        Serial.printf("Respuesta del servidor: %d\n", codigoRespuesta);
        Serial.println(http.getString());
    } else {
        Serial.printf("Error al enviar: %s\n", http.errorToString(codigoRespuesta).c_str());
    }

    http.end();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== Sistema de monitoreo de cultivo ===");

    dht.begin();
    conectarWiFi();

    // primera lectura inmediata al arrancar, sin esperar el intervalo completo
    ultimaLectura = millis() - INTERVALO_LECTURA_MS;
}

void loop() {
    // reconecta si se cayo el WiFi
    if (WiFi.status() != WL_CONNECTED) {
        conectarWiFi();
    }

    if (millis() - ultimaLectura >= INTERVALO_LECTURA_MS) {
        ultimaLectura = millis();

        float humedadSuelo = leerHumedadSuelo();
        float temperatura = dht.readTemperature();
        float humedadAmbiente = dht.readHumidity();
        int luz = leerLuz();

        // el DHT22 a veces falla una lectura puntual; si pasa, se detecta con isnan()
        if (isnan(temperatura) || isnan(humedadAmbiente)) {
            Serial.println("Error leyendo el DHT22, se omite esta lectura de temp/humedad.");
        } else {
            Serial.printf("Humedad suelo: %.1f%% | Temp: %.1fC | Humedad amb: %.1f%% | Luz: %d\n",
                          humedadSuelo, temperatura, humedadAmbiente, luz);

            enviarDatos(humedadSuelo, temperatura, humedadAmbiente, luz);
        }
    }

    delay(100);
}
