#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AP3216_WE.h>
#include <TinyGPS++.h>

// Configuracion Wi-Fi y Servidor
const char* SSID_WIFI     = "UPBWiFi";
const char* PASSWORD_WIFI = "";
const char* URL_SERVIDOR  = "http://34.227.231.151/api/plantas";

// Intervalo de lectura/envio: 5 minutos
#define INTERVALO_LECTURA_MS (5UL * 60UL * 1000UL)

// I2C
#define SDA_PIN 21
#define SCL_PIN 22

// Sensor de humedad de suelo (solo salida analogica, ADC1)
#define PIN_HUMEDAD_ANALOG 39
#define HUMEDAD_RAW_SECO   4095
#define HUMEDAD_RAW_MOJADO 2300   // provisional
#define UMBRAL_SECO_PCT    30     // provisional

// GPS (T-Beam)
#define GPS_RX_PIN 34
#define GPS_TX_PIN 12
#define GPS_BAUD   9600

#define HDC1080_ADDR 0x40

AP3216_WE ap3216 = AP3216_WE(&Wire);
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

float leerTemperaturaHDC1080() {
  Wire.beginTransmission(HDC1080_ADDR);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) return -999;
  delay(20);
  if (Wire.requestFrom(HDC1080_ADDR, 2) == 2) {
    uint16_t rawTemp = (Wire.read() << 8) | Wire.read();
    return (rawTemp / 65536.0) * 165.0 - 40.0;
  }
  return -999;
}

float leerHumedadAireHDC1080() {
  Wire.beginTransmission(HDC1080_ADDR);
  Wire.write(0x01);
  if (Wire.endTransmission() != 0) return -999;
  delay(20);
  if (Wire.requestFrom(HDC1080_ADDR, 2) == 2) {
    uint16_t rawHum = (Wire.read() << 8) | Wire.read();
    return (rawHum / 65536.0) * 100.0;
  }
  return -999;
}

void conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Conectando a Wi-Fi: ");
  Serial.println(SSID_WIFI);
  WiFi.begin(SSID_WIFI, PASSWORD_WIFI);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[OK] Conectado a Wi-Fi.");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[ERROR] No se pudo conectar a Wi-Fi.");
  }
}

void enviarDatosServidor(float temp, float humAire, float lux, int humSueloPorc, int humSueloRaw, bool seco, double lat, double lng, double alt, int sats) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(URL_SERVIDOR);
  http.addHeader("Content-Type", "application/json");

  char jsonPayload[384];
  snprintf(jsonPayload, sizeof(jsonPayload),
    "{\"temperatura_aire\":%.2f,\"humedad_aire\":%.2f,\"luz_lux\":%.2f,\"humedad_suelo_pct\":%d,\"humedad_suelo_raw\":%d,\"alerta_seco\":%s,\"latitud\":%.6f,\"longitud\":%.6f,\"altitud\":%.2f,\"satelites\":%d}",
    temp, humAire, lux, humSueloPorc, humSueloRaw, seco ? "true" : "false", lat, lng, alt, sats
  );

  int httpCode = http.POST(jsonPayload);
  if (httpCode > 0) {
    Serial.printf("[HTTP] Respuesta servidor: %d\n", httpCode);
  } else {
    Serial.printf("[HTTP] Error enviando POST: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n--- Sistema de Monitoreo con GPS y Wi-Fi ---");

  pinMode(PIN_HUMEDAD_ANALOG, INPUT);

  Wire.begin(SDA_PIN, SCL_PIN, 100000);
  ap3216.init();
  ap3216.setMode(AP3216_ALS_PS);

  SerialGPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  conectarWiFi();
}

void loop() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  static unsigned long ultimaLectura = 0;
  static bool primeraLectura = true;

  if (primeraLectura || millis() - ultimaLectura >= INTERVALO_LECTURA_MS) {
    primeraLectura = false;
    ultimaLectura = millis();

    if (WiFi.status() != WL_CONNECTED) conectarWiFi();

    float tempAire = leerTemperaturaHDC1080();
    float humAire  = leerHumedadAireHDC1080();
    float lux      = ap3216.getAmbientLight();

    int humedadSueloRaw = analogRead(PIN_HUMEDAD_ANALOG);
    int humedadSueloPorcentaje = map(humedadSueloRaw, HUMEDAD_RAW_SECO, HUMEDAD_RAW_MOJADO, 0, 100);
    humedadSueloPorcentaje = constrain(humedadSueloPorcentaje, 0, 100);
    bool estaSeco = (humedadSueloPorcentaje < UMBRAL_SECO_PCT);

    double latitud  = gps.location.isValid() ? gps.location.lat() : 0.0;
    double longitud = gps.location.isValid() ? gps.location.lng() : 0.0;
    double altitud  = gps.altitude.isValid() ? gps.altitude.meters() : 0.0;
    int satelites   = gps.satellites.isValid() ? gps.satellites.value() : 0;

    Serial.println("=== MONITOREO DE PLANTA ===");
    Serial.printf("Temp / Hum Aire : %.2f C | %.2f %%\n", tempAire, humAire);
    Serial.printf("Luz Ambiental   : %.2f Lux\n", lux);
    Serial.printf("Humedad Suelo   : %d %% (RAW: %d)\n", humedadSueloPorcentaje, humedadSueloRaw);
    Serial.printf("Alerta seco     : %s\n", estaSeco ? "SI" : "NO");

    if (gps.location.isValid()) {
      Serial.printf("Ubicacion GPS   : Lat: %.6f, Lng: %.6f, Alt: %.1fm (%d sats)\n", latitud, longitud, altitud, satelites);
    } else {
      Serial.printf("GPS             : Buscando satelites... (%d detectados)\n", satelites);
    }

    enviarDatosServidor(tempAire, humAire, lux, humedadSueloPorcentaje, humedadSueloRaw, estaSeco, latitud, longitud, altitud, satelites);

    Serial.println("---------------------------\n");
  }
}
