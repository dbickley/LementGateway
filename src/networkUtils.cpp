#include "networkUtils.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Preferences.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

#include "rda5807m_sensor.h"

// Classic Bluetooth is not supported for this ESP32-S3 target in the current Arduino core.
// The project keeps the stable WiFi + OTA path active and leaves a clean slot for BLE later.

// Forward declaration so the anonymous-namespace API handlers can reference the global server instance.
extern WebServer server;

namespace {
bool radioApiRoutesConfigured = false;
bool webServerRunning = false;

String extractJsonValue(const String& json, const String& key) {
  String needle = "\"" + key + "\"";
  int keyIndex = json.indexOf(needle);
  if (keyIndex < 0) {
    return "";
  }

  int valueIndex = json.indexOf(':', keyIndex + needle.length());
  if (valueIndex < 0) {
    return "";
  }

  int start = valueIndex + 1;
  while (start < json.length() &&
         (json[start] == ' ' || json[start] == '\n' || json[start] == '\r' || json[start] == '\t')) {
    ++start;
  }

  if (start >= json.length()) {
    return "";
  }

  if (json[start] == '"') {
    ++start;
    int end = json.indexOf('"', start);
    if (end < 0) {
      return "";
    }
    String value = json.substring(start, end);
    value.trim();
    return value;
  }

  int end = start;
  while (end < json.length() && json[end] != ',' && json[end] != '}' && json[end] != ']') {
    ++end;
  }
  String value = json.substring(start, end);
  value.trim();
  return value;
}

bool parseBoolValue(const String& value) {
  if (value.length() == 0) {
    return false;
  }

  String lower = value;
  lower.toLowerCase();
  return value == "1" || lower == "true" || lower == "on" || lower == "yes";
}

String buildRadioStateJson() {
  RDA5807M_DATA radio = readRDA5807M();
  String json = "{";
  json += "\"powered\":" + String(radio.powered ? 1 : 0) + ",";
  json += "\"frequencyMHz\":" + String(radio.frequencyMHz, 2) + ",";
  json += "\"volume\":" + String(radio.volume) + ",";
  json += "\"muted\":" + String(radio.muted ? 1 : 0) + ",";
  json += "\"stereo\":" + String(radio.stereo ? 1 : 0) + ",";
  json += "\"signalStrength\":" + String(radio.signalStrength) + ",";
  json += "\"rdsStation\":\"" + String(radio.rdsStation) + "\",";
  json += "\"rdsText\":\"" + String(radio.rdsText) + "\",";
  json += "\"amplifierEnabled\":" + String(radio.amplifierEnabled ? 1 : 0) + ",";
  json += "\"valid\":" + String(radio.valid ? 1 : 0);
  json += "}";
  return json;
}

void sendJsonResponse(int code, const String& body) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,PUT,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(code, "application/json", body);
}

bool applyRadioCommandFromValue(const String& key, const String& value) {
  if (key == "frequency" || key == "frequencyMHz") {
    return radioSetFrequency(value.toFloat());
  }

  if (key == "volume") {
    return radioSetVolume(static_cast<uint8_t>(value.toInt()));
  }

  if (key == "mute") {
    return radioSetMute(parseBoolValue(value));
  }

  if (key == "power") {
    return radioSetPowered(parseBoolValue(value));
  }

  if (key == "amplifier" || key == "amplifierEnabled") {
    return radioSetAmplifierEnabled(parseBoolValue(value));
  }

  if (key == "seek") {
    String lower = value;
    lower.toLowerCase();
    if (lower == "up") {
      return radioSeekUp();
    }
    if (lower == "down") {
      return radioSeekDown();
    }
  }

  return false;
}

bool applyRadioCommandsFromJson(const String& jsonBody) {
  bool applied = false;

  for (const String& key : {String("power"), String("mute"), String("volume"), String("frequency"),
                             String("frequencyMHz"), String("amplifier"), String("amplifierEnabled"),
                             String("seek")}) {
    String rawValue = extractJsonValue(jsonBody, key);
    if (rawValue.length() > 0) {
      applied = applyRadioCommandFromValue(key, rawValue) || applied;
    }
  }

  return applied;
}

bool applyRadioCommandsFromFormData() {
  bool applied = false;
  for (int i = 0; i < server.args(); ++i) {
    String key = server.argName(i);
    String value = server.arg(i);
    if (key.length() > 0 && value.length() > 0) {
      applied = applyRadioCommandFromValue(key, value) || applied;
    }
  }
  return applied;
}

void handleRadioApiGet() {
  String response = "{\"radio\":" + buildRadioStateJson() + "}";
  sendJsonResponse(200, response);
}

void handleRadioApiPost() {
  bool applied = false;
  String body = server.arg("plain");

  if (body.length() > 0) {
    applied = applyRadioCommandsFromJson(body);
  }

  if (!applied) {
    applied = applyRadioCommandsFromFormData();
  }

  String response = "{\"ok\":" + String(applied ? "true" : "false") + ",\"radio\":" + buildRadioStateJson() + "}";
  sendJsonResponse(200, response);
}

void handleRadioApiOptions() {
  sendJsonResponse(200, "{} ");
}

void configureRadioApiRoutes() {
  if (radioApiRoutesConfigured) {
    return;
  }

  radioApiRoutesConfigured = true;
  server.on("/api/radio", HTTP_GET, handleRadioApiGet);
  server.on("/api/radio", HTTP_POST, handleRadioApiPost);
  server.on("/api/radio", HTTP_PUT, handleRadioApiPost);
  server.on("/api/radio", HTTP_OPTIONS, handleRadioApiOptions);
}
}  // namespace

// Configuration Storage
Preferences preferences;
char server_ip[40];
int server_port;

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

#ifndef OTA_PASSWORD
#define OTA_PASSWORD "your-ota-password"
#endif

#ifndef SERVER_IP
#define SERVER_IP "192.168.86.138"
#endif

#ifndef SERVER_PORT
#define SERVER_PORT 6660
#endif

const char* otaPassword = OTA_PASSWORD;

// Portal/Network details
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

const char *endpoint_log_data = "/sensor";
const char *endpoint_current_time = "/current_time";

bool provisioningMode = false;

void loadConfiguration() {
  preferences.begin("lementgateway", true);
  String savedIP = preferences.getString("server_ip", "192.168.86.138");
  server_port = preferences.getInt("server_port", 6660);
  strncpy(server_ip, savedIP.c_str(), sizeof(server_ip) - 1);
  server_ip[sizeof(server_ip) - 1] = '\0';
  preferences.end();
}

bool connectWiFi(String ssid, String pass) {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("LementGateway-S3");
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.printf("Connecting to %s with password %s (timeout 10s)", ssid.c_str(),pass.c_str());
  unsigned long start = millis();
  // Decrease timeout to 10 seconds
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print('.');
    // Optional: add a yield to keep the watchdog happy on long loops
    yield();
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("WiFi connection failed");
  return false;
}

void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:sans-serif;background:#f4f4f4;padding:20px;}input{width:100%;padding:10px;margin:5px 0;box-sizing:border-box;}button{width:100%;padding:10px;background:#2ecc71;color:white;border:none;cursor:pointer;margin-top:10px;}.tgl{width:auto;margin-right:10px;vertical-align:middle;}</style>";
  html += "<script>function toggle(){var x=document.getElementById('p');x.type=x.type==='password'?'text':'password';}</script></head><body>";
  html += "<h2>LementGateway Setup</h2><form action='/save' method='POST'>";
  html += "WiFi SSID:<br><input type='text' name='s' placeholder='Network Name' autocorrect='off' autocapitalize='none' spellcheck='false'>";
  html += "WiFi Password:<br><input type='password' name='p' id='p' placeholder='Password' autocorrect='off' autocapitalize='none' spellcheck='false'>";
  html += "<div style='margin-bottom:10px'><input type='checkbox' class='tgl' onclick='toggle()'>Show Password</div>";
  html += "Server IP:<br><input type='text' name='ip' value='" + String(server_ip) + "' autocorrect='off' autocapitalize='none' spellcheck='false'>";
  html += "Server Port:<br><input type='number' name='port' value='" + String(server_port) + "' min='1' max='65535'>";
  html += "<br><br><button type='submit'>Save & Connect</button></form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  String ssid = server.arg("s");
  String pass = server.arg("p");
  String ip = server.arg("ip");
  int port = server.arg("port").toInt();

  preferences.begin("lementgateway", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.putString("server_ip", ip);
  preferences.putInt("server_port", port);
  preferences.end();

  server.send(200, "text/html", "<html><body>Config Saved! Restarting...</body></html>");
  delay(2000);
  ESP.restart();
}

void startProvisioningPortal() {
  Serial.println("\n[!] Starting Captive Portal...");

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("LementGateway_Setup", NULL); // Open network

  dnsServer.start(DNS_PORT, "*", apIP);

  configureRadioApiRoutes();
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound([]() {
    server.sendHeader("Location", String("http://") + apIP.toString(), true);
    server.send(302, "text/plain", "");
  });

  if (!webServerRunning) {
    server.begin();
    webServerRunning = true;
  }
  provisioningMode = true;
  Serial.println("Portal Ready. SSID: LementGateway_Setup");
}

void setupNetworkUtils() {
  loadConfiguration();
  
  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info){
    switch(event) {
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.printf("[WiFi] Disconnected, reason: %d\n", info.wifi_sta_disconnected.reason);
        break;
      default:
        break;
    }
  });

  preferences.begin("lementgateway", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid.length() > 0) {
    Serial.println("Already provisioned, connecting...");
    if (!connectWiFi(ssid, pass)) {
      startProvisioningPortal();
    } else {
      configureRadioApiRoutes();
      if (!webServerRunning) {
        server.begin();
        webServerRunning = true;
      }
    }
  } else {
    startProvisioningPortal();
  }
}

void setupOTA() {
  ArduinoOTA.setHostname("LementGateway-S3");
  ArduinoOTA.setPassword(otaPassword);
  ArduinoOTA.setPasswordHash(NULL);

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("OTA start: " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA complete");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA progress: %u%%\r", (progress / (total > 0 ? total : 1)) * 100);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive failed");
    else if (error == OTA_END_ERROR) Serial.println("End failed");
  });

  ArduinoOTA.begin();
  Serial.println("OTA ready. Password: lementgateway");
}

void broadcastStateEvent(const String& eventType, const String& stateJson) {
  String eventPayload = "{\"event\":\"" + eventType + "\",\"state\":" + stateJson + "}";

  if (WiFi.status() == WL_CONNECTED && server_ip[0] != '\0') {
    // WiFi transport remains available for the existing HTTP server path.
    // This method is intentionally kept transport-agnostic so a hard-wired output can be added later.
  }

  (void)eventPayload;
}

void handleNetworkPortal() {
  if (provisioningMode || webServerRunning) {
    if (provisioningMode) {
      dnsServer.processNextRequest();
    }
    server.handleClient();
  }
}

void handleOTA() {
  ArduinoOTA.handle();
}

bool isProvisioningModeActive() {
  return provisioningMode;
}

bool logData(const SENSOR_DATA& sensor_data) {
  bool retval = true;
  String jsonData = "{";
  jsonData += "\"sensorId\":1,";
  jsonData += "\"bme_humidity\":" + String(sensor_data.bme280.humidity) + ",";
  jsonData += "\"bme_temperature\":" + String(sensor_data.bme280.temperature) + ",";
  jsonData += "\"bme_pressure\":" + String(sensor_data.bme280.pressure) + ",";
  jsonData += "\"sht20_humidity\":" + String(sensor_data.sht20.humidityPercent) + ",";
  jsonData += "\"sht20_temperature\":" + String(sensor_data.sht20.temperatureC) + ",";
  jsonData += "\"proximity\":" + String(sensor_data.apds.proximity) + ",";
  jsonData += "\"ambientLight\":" + String(sensor_data.apds.ambientLight);
  jsonData += ",";
  jsonData += "\"rcwl_pulse\":" + String(sensor_data.rcwl.pulseWidth) + ",";
  jsonData += "\"rcwl_motion\":" + String(sensor_data.rcwl.motionDetected ? 1 : 0);
  jsonData += "}";

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(1500);
    http.begin("http://" + String(server_ip) + ":" + String(server_port) + endpoint_log_data);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode >= 200 && httpResponseCode < 300) {
      String response = http.getString();
      (void)response;
    } else {
      Serial.printf("Post failed, error: %d\n", httpResponseCode);
      retval = false;
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
    retval = false;
  }

  return retval;
}

time_t getCurrentTime() {
  time_t currentTime = 0;

  Serial.println("getCurrentTime");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; //
    http.begin("http://" + String(server_ip) + ":" + String(server_port) + endpoint_current_time);

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Current time (response): " + response);
      currentTime = (time_t)atoll(response.c_str());
    } else {
      Serial.printf("Error: %d\n", httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }

  return currentTime;
}
