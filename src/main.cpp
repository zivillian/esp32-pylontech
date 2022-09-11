#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "config.h"
#include "pages.h"
#include "pylontech.h"


AsyncWebServer webServer(80);
Config config;
Preferences prefs;
WiFiManager wm;
Pylonclient client;

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
  client.Begin(&pylonSerial);
  setupPages(&webServer, &wm, &client);
  webServer.begin();
  pylonSerial.begin(115200, SERIAL_8N1);
  dbgln("[setup] finished");
}

void loop() {
  // put your main code here, to run repeatedly:
}