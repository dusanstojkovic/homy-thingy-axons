/*
  Homy Panel

  Homy Panel sketch - Handles panel light control using Thingy.IO

  Copyright (c) 2017 Dusan Stojkovic
*/

#define   BUT_1_LED   D1
#define   BUT_1_OUT   D2
#define   BUT_1_COMMAND "brain/xxxxx/axon/xxxxx/stream/xxxxx"
#define   BUT_1_STATUS "brain/xxxxx/axon/xxxxx/stream/xxxxx"

#define   BUT_2_LED   D3
#define   BUT_2_OUT   D4
#define   BUT_2_COMMAND "brain/xxxxx/axon/xxxxx/stream/xxxxx"
#define   BUT_2_STATUS "brain/xxxxx/axon/xxxxx/stream/xxxxx"

#define   BUT_3_LED   D5
#define   BUT_3_OUT   D6
#define   BUT_3_COMMAND "brain/xxxxx/axon/xxxxx/stream/xxxxx"
#define   BUT_3_STATUS "brain/xxxxx/axon/xxxxx/stream/xxxxx"

#define   BUT_4_LED   D7
#define   BUT_4_OUT   D8
#define   BUT_4_COMMAND "brain/xxxxx/axon/xxxxx/stream/xxxxx"
#define   BUT_4_STATUS "brain/xxxxx/axon/xxxxx/stream/xxxxx"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>
#include <Streaming.h>

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

bool s1 = false, s2 = false, s3 = false, s4 = false;
bool s1_ = false, s2_ = false, s3_ = false, s4_ = false;
int b1 = false, b2 = false, b3 = false, b4 = false;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin("<SSID>", "<PASS>");
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach();
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t packetIdSub;
  packetIdSub = mqttClient.subscribe(BUT_1_STATUS, 2);
  Serial << "Subscribing to button 1 status (" << packetIdSub << ")" << endl;
  packetIdSub = mqttClient.subscribe(BUT_2_STATUS, 2);
  Serial << "Subscribing to button 2 status (" << packetIdSub << ")" << endl;
  packetIdSub = mqttClient.subscribe(BUT_3_STATUS, 2);
  Serial << "Subscribing to button 3 status (" << packetIdSub << ")" << endl;
  packetIdSub = mqttClient.subscribe(BUT_4_STATUS, 2);
  Serial << "Subscribing to button 4 status (" << packetIdSub << ")" << endl;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t pID, uint8_t qos) 
{
  Serial << "Subscribe ACK (" << pID << ") QoS=" << qos << endl;  
}

void onMqttUnsubscribe(uint16_t pID) 
{
  Serial << "Unsubscribe ACK (" << pID << ")" << endl;
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);

  if (strcmp(topic, BUT_1_STATUS) == 0 )
  {
    if (strcmp(payload, "ON") == 0) { s1_ = true; return; }
    if (strcmp(payload, "OFF") == 0) { s1_ = false; return; }
    if (strcmp(payload, "TOGGLE") == 0) { s1_ = !s1; return; }
    return;
  }
  if (strcmp(topic, BUT_2_STATUS) == 0 )
  {
    if (strcmp(payload, "ON") == 0) { s2_ = true; return; }
    if (strcmp(payload, "OFF") == 0) { s2_ = false; return; }
    if (strcmp(payload, "TOGGLE") == 0) { s2_ = !s2; return; }
    return;
  }
  if (strcmp(topic, BUT_3_STATUS) == 0 )
  {
    if (strcmp(payload, "ON") == 0) { s3_ = true; return; }
    if (strcmp(payload, "OFF") == 0) { s3_ = false; return; }
    if (strcmp(payload, "TOGGLE") == 0) { s3_ = !s3; return; }
    return;
  }
  if (strcmp(topic, BUT_4_STATUS) == 0 )
  {
    if (strcmp(payload, "ON") == 0) { s4_ = true; return; }
    if (strcmp(payload, "OFF") == 0) { s4_ = false; return; }
    if (strcmp(payload, "TOGGLE") == 0) { s4_ = !s4; return; }
    return;
  }      
  
}

void onMqttPublish(uint16_t pID) 
{
  Serial << "Publish ACK (" << pID << ")" << endl;
}


void setup()
{
	// Setup console
	Serial.begin(115200);
	delay(10);

	Serial << endl << endl  << F("Thingy.Panel") << " - " << __DATE__ << " " << __TIME__ << endl << "#" << endl << endl;

  // sensors
  pinMode(BUT_1_LED, OUTPUT);
  pinMode(BUT_2_LED, OUTPUT);
  pinMode(BUT_3_LED, OUTPUT);
  pinMode(BUT_4_LED, OUTPUT);

  pinMode(BUT_1_OUT, OUTPUT);
  pinMode(BUT_2_OUT, OUTPUT);
  pinMode(BUT_3_OUT, OUTPUT);
  pinMode(BUT_4_OUT, OUTPUT);  

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer("mqtt.thingy.io", 1883);
  mqttClient.setCredentials("<UiD>","<SiD>");

  connectToWifi();
}

void loop()
{
//  Serial << digitalRead(D2) << "  " << digitalRead(D4) << "  " << digitalRead(D6) << "  "  << digitalRead(D8) << endl;

  if (b1 != digitalRead(BUT_1_OUT))
  {
    b1 = digitalRead(BUT_1_OUT);
    uint16_t pID = mqttClient.publish(BUT_1_COMMAND, 2, false, s1 ? "OFF" : "ON");
    Serial << "Button 1 pressed (" << pID << ")" << endl;
  }
  if (b2 != digitalRead(BUT_2_OUT))
  {
    b2 = digitalRead(BUT_2_OUT);
    uint16_t pID = mqttClient.publish(BUT_2_COMMAND, 2, false, s2 ? "OFF" : "ON");
    Serial << "Button 2 pressed (" << pID << ")" << endl;
  }
  if (b3 != digitalRead(BUT_3_OUT))
  {
    b3 = digitalRead(BUT_3_OUT);
    uint16_t pID = mqttClient.publish(BUT_3_COMMAND, 2, false, s3 ? "OFF" : "ON");
    Serial << "Button 3 pressed (" << pID << ")" << endl;
  }
  if (b4 != digitalRead(BUT_4_OUT))
  {
    uint16_t pID;
    pID = mqttClient.publish(BUT_1_COMMAND, 2, false, s4 ? "OFF" : "ON");
    pID = mqttClient.publish(BUT_2_COMMAND, 2, false, s4 ? "OFF" : "ON");
    pID = mqttClient.publish(BUT_3_COMMAND, 2, false, s4 ? "OFF" : "ON");

    b4 = digitalRead(BUT_4_OUT);
    pID = mqttClient.publish(BUT_3_COMMAND, 2, false, s4 ? "OFF" : "ON");
    Serial << "Button 4 pressed (" << pID << ")" << endl;
  }

  // setting the state
  s4_ = s1_ || s2_ || s3_;
  if (s1 != s1_)
  {
    s1 = s1_;
    Serial << "Light 1 state - " << (s1 ? F("ON 1"): F("OFF 1")) << endl;
    digitalWrite(BUT_1_LED, s1 ? HIGH : LOW);
  }
  if (s2 != s2_)
  {
    s2 = s2_;
    Serial << "Light 2 state - " << (s2 ? F("ON 2"): F("OFF 2")) << endl;
    digitalWrite(BUT_2_LED, s2 ? HIGH : LOW);
  }
  if (s3 != s3_)
  {
    s3 = s3_;
    Serial << "Light 3 state - " << (s3 ? F("ON 3"): F("OFF 3")) << endl;
    digitalWrite(BUT_3_LED, s3 ? HIGH : LOW);
  }
  if (s4 != s4_)
  {
    s4 = s4_;
    Serial << "Light 4 state - " << (s4 ? F("ON 4"): F("OFF 4")) << endl;
    digitalWrite(BUT_4_LED, s4 ? HIGH : LOW);
  }
}
