#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "config.h"
#include "pages.h"


AsyncWebServer webServer(80);
Preferences prefs;
WiFiManager wm;

void setup() {
  debugSerial.begin(115200);
  dbgln("[wifi] start");
  WiFi.mode(WIFI_STA);
  wm.setClass("invert");
  wm.autoConnect();
  dbgln("[wifi] finished");
  
  setupPages(&webServer, &wm);
  webServer.begin();
  pylonSerial.begin(115200, SERIAL_8N1);
  dbgln("[setup] finished");
}

void loop() {
  // put your main code here, to run repeatedly:
}