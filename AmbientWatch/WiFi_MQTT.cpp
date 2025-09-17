#include "WiFi_MQTT.h"
#include <PubSubClient.h>
#include <WiFi.h>

WiFi_MQTT::WiFi_MQTT(const char *SSID, const char *password, const char *mqttServer, int mqttPort): PubSubClient(_wifiClient) {
  _ssid = SSID;
  _password = password;
  _mqttServer = mqttServer;
  _mqttPort = mqttPort;
  PubSubClient::setServer(_mqttServer, _mqttPort);
}


bool WiFi_MQTT::publishData(String topicPrefix, uint16_t co2, float temperature, float humidity, float decibel) {

  bool ris = PubSubClient::connected();

  if (ris) {
    char msg[16];

    // CO2
    snprintf(msg, sizeof(msg), "%u", co2);
    PubSubClient::publish((topicPrefix + "/co2").c_str(), msg, true);

    // Temperatura
    snprintf(msg, sizeof(msg), "%.2f", temperature);
    PubSubClient::publish((topicPrefix + "/temperature").c_str(), msg, true);

    // Umidità
    snprintf(msg, sizeof(msg), "%.2f", humidity);
    PubSubClient::publish((topicPrefix + "/humidity").c_str(), msg, true);

    // // Decibel
    snprintf(msg, sizeof(msg), "%.2f", decibel);
    PubSubClient::publish((topicPrefix + "/decibel").c_str(), msg, true);
  }

  Serial.println("MQTT publish data");
  mqttStatus = ris;
  return ris;
}

bool WiFi_MQTT::publishAlertLimits(String topicPrefix, uint16_t co2_max, float temp_min, float temp_max, float hum_min, float hum_max, float dec_max) {

  bool ris = PubSubClient::connected();

  if (ris) {
    char msg[16];

    // CO2 minimo
    snprintf(msg, sizeof(msg), "%u", co2_max);
    PubSubClient::publish((topicPrefix + "/co2_max").c_str(), msg, true);

    // Temperatura minima
    snprintf(msg, sizeof(msg), "%.2f", temp_min);
    PubSubClient::publish((topicPrefix + "/temp_min").c_str(), msg, true);

    // Temperatura massima
    snprintf(msg, sizeof(msg), "%.2f", temp_max);
    PubSubClient::publish((topicPrefix + "/temp_max").c_str(), msg, true);

    // Umidità minima
    snprintf(msg, sizeof(msg), "%.2f", hum_min);
    PubSubClient::publish((topicPrefix + "/hum_min").c_str(), msg, true);

    // Umidità massima
    snprintf(msg, sizeof(msg), "%.2f", hum_max);
    PubSubClient::publish((topicPrefix + "/hum_max").c_str(), msg, true);

    // Decibel minimo
    snprintf(msg, sizeof(msg), "%.2f", dec_max);
    PubSubClient::publish((topicPrefix + "/dec_max").c_str(), msg, true);
  }

  Serial.println("MQTT publish alert limits");
  mqttStatus = ris;
  return ris;
}

bool WiFi_MQTT::publishAlertStatus(String topic, bool active) {

  bool ris = PubSubClient::connected();

  if (ris) {
    PubSubClient::publish(topic.c_str(), active ? "1" : "0", true);
  }

  Serial.println("MQTT publish alert status");
  mqttStatus = ris;
  return ris;
}

bool WiFi_MQTT::updateSubscribe(){

  bool ris = PubSubClient::connected();
  if (ris) {
    for (int i=0; i<10; i++) {
      PubSubClient::loop();
      delay(10);
    }
  }
  mqttStatus = ris;
  return ris;
}


bool WiFi_MQTT::mqttConnect(const char* topic) {
  if (PubSubClient::connected()){
    return true;
  }
  for (int i=0; i<3; i++) {
    Serial.print("Attempting MQTT connection...");
    if (PubSubClient::connect("Pippo")) {
      Serial.println("connected");
      PubSubClient::subscribe(topic);
      return true;
    } else {
      Serial.println("Try again in 5 seconds");
      delay(3000); // Wait 3 seconds before retrying
    }
  }
  return false;
}


void WiFi_MQTT::wifiDisconnect() {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi disconnect");
  }
}

bool WiFi_MQTT::wifiConnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    Serial.println();
    Serial.print("Connecting");
    for (int i=0; i<30; i++) {
      if (WiFi.status() == WL_CONNECTED) {
        break;
      }
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.print("Connected, IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println();
      Serial.println("WiFi not connected");
    }
    
    wifiStatus = WiFi.status() == WL_CONNECTED;
  }
  return WiFi.status() == WL_CONNECTED;
}

void WiFi_MQTT::disconnect() {
  PubSubClient::disconnect();
  wifiDisconnect();
}

bool WiFi_MQTT::connect(const char* topic) {
  bool ris = wifiConnect();
  if (!ris) {
    return false;
  }
  return mqttConnect(topic);
}