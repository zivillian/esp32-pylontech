#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncMqttClient.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Ticker.h>
#include "config.h"
#include "pages.h"
#include "pylontech.h"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
AsyncWebServer webServer(80);
Config config;
Preferences prefs;
WiFiManager wm;
Pylonclient client;

void connectToMqtt() {
  dbgln("Connecting to MQTT...");
  mqttClient.connect();
}

void mqttPublish(String topic, String value){
  mqttClient.publish((config.getMqttPrefix() + "/" + topic).c_str(), 0, false, value.c_str());
}

void onMqttConnect(bool sessionPresent) {
  mqttClient.setWill((config.getMqttPrefix() + "/online").c_str(), 0, false, "0");
  mqttPublish("online", "1");
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  debugSerial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void setup() {
  debugSerial.begin(115200);
  dbgln();
  dbgln("[config] load")
  prefs.begin("pylontech");
  config.begin(&prefs);
  dbgln("[wifi] start");
  WiFi.mode(WIFI_STA);
  wm.setClass("invert");
  wm.autoConnect();
  dbgln("[wifi] finished");
  if (config.getMqttHost().length() > 0){
    dbgln("[mqtt] start");
    dbg("connecting to ");
    dbg(config.getMqttHost().c_str());
    dbg(":");
    dbgln(config.getMqttPort());
    IPAddress ip;
    if (ip.fromString(config.getMqttHost())){
      mqttClient.setServer(ip, config.getMqttPort());
    }
    else{
      mqttClient.setServer(config.getMqttHost().c_str(), config.getMqttPort());
    }
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    connectToMqtt();
    dbgln("[mqtt] finished");
  }  
  client.Begin(&pylonSerial);
  setupPages(&webServer, &wm, &config, &client, &mqttClient);
  webServer.begin();
  pylonSerial.begin(115200, SERIAL_8N1);
  dbgln("[setup] finished");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (mqttClient.connected()){
    for (size_t i = 0; i < config.getModuleCount(); i++)
    {
      dbg("polling pylontech module ");
      dbgln(i);
      mqttPublish("polling", String(i));
      auto frame = client.SendCommand(Pylonframe(2 + i, CommandInformation::ProtocolVersion));
      if (frame.HasError){
        dbgln("version failed");
        mqttPublish("error/id", String(i));
        mqttPublish("error/version", String(frame.Cid2, HEX));
        continue;
      }
      frame = client.SendCommand(Pylonframe(frame.MajorVersion, frame.MinorVersion, 2 + i, CommandInformation::ManufacturerInfo));
      if (frame.HasError){
        dbgln("manufacturer failed");
        mqttPublish("error/id", String(i));
        mqttPublish("error/manufacturer", String(frame.Cid2, HEX));
        continue;
      }
      auto manufacturer = Pylonframe::PylonManufacturerInfo(frame.Info);
      manufacturer.publish([i](String name, String value){mqttPublish(String(i) + "/" + name, value);});
    }
    
    delay(config.getInterval());
  }
}