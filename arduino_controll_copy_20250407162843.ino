#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#define DHTPIN 2    
#define DHTTYPE DHT22
 
const char* ssid = "Pixel 6 pro";
const char* password = "99887766";
 
const char* serverName = "https://script.google.com/macros/s/AKfycbwS868xBXSWuG_XznfyUaEfj_El7qOquG67M8Q0qhKPvosUh0rdcM_TsjHCyoAImS5S/exec";
 
 
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
 
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
 
  dht.begin();
  client.setInsecure();
}
 
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected, reconnecting...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Reconnected to WiFi");
  }
 
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
 
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
 
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    Serial.println("Starting connection to server...");
    http.begin(client, serverName);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.addHeader("Content-Type", "application/json");
 
    String postData = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    Serial.print("Sending data: ");
    Serial.println(postData);
 
    int httpResponseCode = http.POST(postData);
 
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      Serial.println(http.errorToString(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
 
  delay(30000);
}      