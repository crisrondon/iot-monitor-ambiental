#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "DHT.h"

#define DHTPIN 26
#define DHTTYPE DHT11

// ========================================
// IDENTIFICACION DEL NODO IoT
// ========================================

const char* deviceId = "ESP32-01";
const char* ubicacion = "Laboratorio";

// ========================================
// CONFIGURACION WIFI
// ========================================

// No publicar estas credenciales en GitHub
const char* ssid = "TU_RED_WIFI";
const char* password = "TU_PASSWORD_WIFI"";

// ========================================
// AZURE FUNCTION
// ========================================

const char* azureUrl =
  "https://func-iot-monitor-ambiental.azurewebsites.net/api/recibir_datos_esp32";

// Envio a Azure cada 10 segundos
unsigned long ultimoEnvioAzure = 0;
const unsigned long intervaloEnvioAzure = 10000;

// ========================================
// SENSOR
// ========================================

DHT dht(DHTPIN, DHTTYPE);

// ========================================
// SERVIDOR WEB LOCAL
// ========================================

WebServer server(80);

// ========================================
// VARIABLES DE MEDICION
// ========================================

float temperatura = 0;
float humedad = 0;

String estadoAmbiental = "SIN DATOS";

unsigned long ultimoUpdate = 0;

const unsigned long intervaloLectura = 2000;

// ========================================
// CLASIFICACION EDGE
// ========================================

String calcularEstado(float temperatura) {

  if (temperatura >= 35.0) {
    return "CRITICO";
  }

  if (temperatura >= 28.0) {
    return "ADVERTENCIA";
  }

  return "NORMAL";
}

// ========================================
// ENVIO DE DATOS A AZURE
// ========================================

void enviarDatosAzure() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Azure: WiFi no disponible.");

    return;
  }

  WiFiClientSecure client;

  // SOLO PARA PRUEBAS
  // La conexion sigue siendo HTTPS,
  // pero no se valida el certificado del servidor.
  client.setInsecure();

  HTTPClient http;

  Serial.println();
  Serial.println("================================");
  Serial.println(" ENVIANDO DATOS A AZURE");
  Serial.println("================================");

  if (!http.begin(client, azureUrl)) {

    Serial.println("Error iniciando conexion HTTPS con Azure.");

    return;
  }

  http.addHeader("Content-Type", "application/json");

  String json = "{";

  json += "\"dispositivo\":\"";
  json += deviceId;
  json += "\",";

  json += "\"ubicacion\":\"";
  json += ubicacion;
  json += "\",";

  json += "\"temperatura\":";
  json += String(temperatura, 1);
  json += ",";

  json += "\"humedad\":";
  json += String(humedad, 1);
  json += ",";

  json += "\"estado\":\"";
  json += estadoAmbiental;
  json += "\"";

  json += "}";

  Serial.print("JSON enviado: ");
  Serial.println(json);

  int codigoHTTP = http.POST(json);

  Serial.print("Codigo HTTP Azure: ");
  Serial.println(codigoHTTP);

  if (codigoHTTP > 0) {

    String respuesta = http.getString();

    Serial.print("Respuesta Azure: ");
    Serial.println(respuesta);

  } else {

    Serial.print("Error enviando a Azure: ");
    Serial.println(http.errorToString(codigoHTTP));
  }

  http.end();

  Serial.println("================================");
}

// ========================================
// DASHBOARD WEB LOCAL
// ========================================

void handleRoot() {

  String html = "<!DOCTYPE html><html lang='es'>";

  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Monitor Ambiental IoT</title>";

  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";

  html += "<meta http-equiv='refresh' content='5'>";

  html += "<style>";

  html += "*{box-sizing:border-box;}";

  html +=
  "body{"
  "font-family:Arial,sans-serif;"
  "background:linear-gradient(135deg,#0f172a,#1f2933);"
  "color:#e5e7eb;"
  "min-height:100vh;"
  "display:flex;"
  "align-items:center;"
  "justify-content:center;"
  "margin:0;"
  "padding:24px;"
  "}";

  html +=
  ".card{"
  "background:#111827;"
  "padding:32px;"
  "border-radius:20px;"
  "box-shadow:0 18px 40px rgba(0,0,0,.5);"
  "max-width:520px;"
  "width:100%;"
  "text-align:center;"
  "}";

  html += "h1{margin-bottom:6px;}";

  html +=
  ".subtitle{"
  "color:#9ca3af;"
  "margin-bottom:24px;"
  "}";

  html +=
  ".values{"
  "display:flex;"
  "gap:16px;"
  "margin-top:20px;"
  "}";

  html +=
  ".chip{"
  "flex:1;"
  "background:#020617;"
  "padding:18px;"
  "border-radius:16px;"
  "border:1px solid #1f2937;"
  "}";

  html +=
  ".label{"
  "font-size:.8rem;"
  "color:#9ca3af;"
  "text-transform:uppercase;"
  "}";

  html +=
  ".value{"
  "font-size:1.8rem;"
  "font-weight:bold;"
  "margin-top:5px;"
  "}";

  html +=
  ".info{"
  "margin-top:20px;"
  "font-size:.9rem;"
  "color:#9ca3af;"
  "}";

  html += "</style>";

  html += "</head>";

  html += "<body>";

  html += "<div class='card'>";

  html += "<h1>Monitor Ambiental IoT</h1>";

  html +=
  "<div class='subtitle'>"
  "ESP32 + DHT11"
  "</div>";

  html += "<p><b>Dispositivo:</b> ";
  html += deviceId;
  html += "</p>";

  html += "<p><b>Ubicacion:</b> ";
  html += ubicacion;
  html += "</p>";

  html += "<div class='values'>";

  // Temperatura

  html += "<div class='chip'>";

  html += "<div class='label'>Temperatura</div>";

  html += "<div class='value'>";
  html += String(temperatura, 1);
  html += " &deg;C</div>";

  html += "</div>";

  // Humedad

  html += "<div class='chip'>";

  html += "<div class='label'>Humedad</div>";

  html += "<div class='value'>";
  html += String(humedad, 1);
  html += " %</div>";

  html += "</div>";

  html += "</div>";

  html += "<div class='info'>";

  html += "<p><b>Estado ambiental:</b> ";
  html += estadoAmbiental;
  html += "</p>";

  html += "<p><b>IP:</b> ";
  html += WiFi.localIP().toString();
  html += "</p>";

  html += "<p>Lectura cada 2 segundos</p>";
  html += "<p>Envio a Azure cada 10 segundos</p>";

  html += "</div>";

  html += "</div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ========================================
// API JSON LOCAL
// ========================================

void handleData() {

  String json = "{";

  json += "\"deviceId\":\"";
  json += deviceId;
  json += "\",";

  json += "\"ubicacion\":\"";
  json += ubicacion;
  json += "\",";

  json += "\"temperatura\":";
  json += String(temperatura, 1);
  json += ",";

  json += "\"humedad\":";
  json += String(humedad, 1);
  json += ",";

  json += "\"estado\":\"";
  json += estadoAmbiental;
  json += "\"";

  json += "}";

  server.send(200, "application/json", json);
}

// ========================================
// SETUP
// ========================================

void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println();

  Serial.println("==============================");
  Serial.println(" SISTEMA DE MONITOREO IoT");
  Serial.println("==============================");

  Serial.print("Dispositivo: ");
  Serial.println(deviceId);

  Serial.print("Ubicacion: ");
  Serial.println(ubicacion);

  // Sensor

  dht.begin();

  // WiFi

  Serial.print("Conectando a WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();

  Serial.println("WiFi conectado.");

  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  // mDNS

  if (MDNS.begin("esp32")) {

    Serial.println("mDNS: http://esp32.local");
  }

  // Rutas HTTP

  server.on("/", handleRoot);

  server.on("/data", handleData);

  server.begin();

  Serial.println("Servidor HTTP iniciado.");

  Serial.println();
  Serial.println("Dashboard local:");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  Serial.println("API JSON local:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/data");

  Serial.println();
  Serial.println("Azure Function configurada:");
  Serial.println(azureUrl);
}

// ========================================
// LOOP
// ========================================

void loop() {

  server.handleClient();

  unsigned long ahora = millis();

  // ========================================
  // LECTURA DEL SENSOR
  // ========================================

  if (ahora - ultimoUpdate >= intervaloLectura) {

    ultimoUpdate = ahora;

    float h = dht.readHumidity();

    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {

      Serial.println("Error leyendo DHT11");

    } else {

      humedad = h;

      temperatura = t;

      // Procesamiento Edge

      estadoAmbiental = calcularEstado(temperatura);

      Serial.println("------------------------------");

      Serial.print("IP ESP32: ");
      Serial.println(WiFi.localIP());

      Serial.print("Device ID: ");
      Serial.println(deviceId);

      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.println(" C");

      Serial.print("Humedad: ");
      Serial.print(humedad);
      Serial.println(" %");

      Serial.print("Estado: ");
      Serial.println(estadoAmbiental);
    }
  }

  // ========================================
  // ENVIO PERIODICO A AZURE
  // ========================================

  if (
    ahora - ultimoEnvioAzure >= intervaloEnvioAzure &&
    estadoAmbiental != "SIN DATOS"
  ) {

    ultimoEnvioAzure = ahora;

    enviarDatosAzure();
  }
}