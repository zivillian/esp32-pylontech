#include "pages.h"

extern unsigned long wifiReconnectCount;

static void renderWifiPage(AsyncResponseStream *response){
  sendResponseHeader(response, "WiFi");
  response->print("<h4>Current connection</h4>");
  response->print("<table>");
  response->printf("<tr><td>SSID:</td><td>%s</td></tr>", WiFi.SSID().c_str());
  response->printf("<tr><td>BSSID:</td><td>%s</td></tr>", WiFi.BSSIDstr().c_str());
  response->printf("<tr><td>RSSI:</td><td>%d dBm</td></tr>", WiFi.RSSI());
  response->printf("<tr><td>Channel:</td><td>%d</td></tr>", WiFi.channel());
  response->printf("<tr><td>IP:</td><td>%s</td></tr>", WiFi.localIP().toString().c_str());
  response->print("</table><p></p>");

  response->print("<h4>Networks</h4>");
  int scanState = WiFi.scanComplete();
  if (scanState == WIFI_SCAN_RUNNING) {
    response->print("<meta http-equiv=\"refresh\" content=\"1; url=/wifi\">");
    response->print("<p>Scanning...</p>");
  } else if (scanState == WIFI_SCAN_FAILED) {
    response->print("<p>No scan yet. Click Rescan.</p>");
  } else if (scanState == 0) {
    response->print("<p>No networks found.</p>");
  } else {
    response->print("<table>"
      "<tr><th>SSID</th><th>RSSI</th><th>Ch</th><th>Lock</th></tr>");
    for (int i = 0; i < scanState; i++) {
      String ssid = WiFi.SSID(i);
      String escSsid = ssid;
      escSsid.replace("\"", "&quot;");
      escSsid.replace("<", "&lt;");
      escSsid.replace(">", "&gt;");
      bool locked = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
      response->printf(
        "<tr class=\"wn\" data-ssid=\"%s\" style=\"cursor:pointer\">"
        "<td>%s</td><td>%d dBm</td><td>%d</td><td>%s</td>"
        "</tr>",
        escSsid.c_str(), escSsid.c_str(), WiFi.RSSI(i), WiFi.channel(i),
        locked ? "yes" : "no");
    }
    response->print("</table>");
  }
  response->print("<p></p>"
    "<form method=\"get\" action=\"/wifi/scan\">"
      "<button>Rescan</button>"
    "</form><p></p>");

  response->print("<h4>Edit</h4>");
  response->print("<form method=\"post\" action=\"/wifi\">"
    "<table>"
      "<tr>"
        "<td><label for=\"ssid\">SSID</label></td>"
        "<td>");
  response->printf("<input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"%s\">",
    WiFi.SSID().c_str());
  response->print("</td>"
      "</tr>"
      "<tr>"
        "<td><label for=\"psk\">Password</label></td>"
        "<td><input type=\"password\" id=\"psk\" name=\"psk\" value=\"\"></td>"
      "</tr>"
    "</table>"
    "<button>Save and reboot</button>"
    "</form><p></p>");

  sendButton(response, "Back", "/");
  response->print("<p></p>"
    "<form method=\"post\" action=\"/wifi/erase\">"
      "<button class=\"r\">Erase WiFi config</button>"
    "</form>");

  response->print("<script>"
    "(function(){"
      "var rows=document.querySelectorAll('tr.wn');"
      "var input=document.getElementById('ssid');"
      "rows.forEach(function(r){"
        "r.addEventListener('click',function(){"
          "input.value=r.dataset.ssid;"
          "document.getElementById('psk').focus();"
        "});"
      "});"
    "})();"
    "</script>");
  sendResponseTrailer(response);
}

void setupPages(AsyncWebServer *server, WiFiManager *wm, Config *config, Pylonclient *client, AsyncMqttClient *mqtt, std::function<void()> onMqttConfigChanged){
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Main");
    sendButton(response, "Info", "info");
    sendButton(response, "Config", "config");
    sendButton(response, "Debug", "debug");
    sendButton(response, "Firmware update", "update");
    sendButton(response, "WiFi", "wifi");
    sendButton(response, "Reboot", "reboot", "r");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/info", HTTP_GET, [config, mqtt](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /info");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Info");
    unsigned long uptimeMs = millis();
    unsigned long seconds = uptimeMs / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    response->print("<table>");
    response->printf(
      "<tr><td>Uptime:</td><td>%lud %luh %lum %lus</td></tr>",
      days, hours % 24, minutes % 60, seconds % 60);
    response->printf(
      "<tr><td>WiFi Reconnects:</td><td>%lu</td></tr>",
      wifiReconnectCount);
    response->printf(
      "<tr><td>SSID:</td><td>%s</td></tr>",
      WiFi.SSID().c_str());
    response->printf(
      "<tr><td>BSSID:</td><td>%s</td></tr>",
      WiFi.BSSIDstr().c_str());
    response->printf(
      "<tr><td>RSSI:</td><td>%d dBm</td></tr>",
      WiFi.RSSI());
    response->printf(
      "<tr><td>WiFi Channel:</td><td>%d</td></tr>",
      WiFi.channel());
    response->printf(
      "<tr><td>IP:</td><td>%s</td></tr>",
      WiFi.localIP().toString().c_str());
    response->printf(
      "<tr><td>Netmask:</td><td>%s</td></tr>",
      WiFi.subnetMask().toString().c_str());
    response->printf(
      "<tr><td>Gateway:</td><td>%s</td></tr>",
      WiFi.gatewayIP().toString().c_str());
    response->printf(
      "<tr><td>MAC:</td><td>%s</td></tr>",
      WiFi.macAddress().c_str());
    if (config->getMqttHost().length() == 0) {
      response->print(
        "<tr><td>MQTT Status:</td><td>Not configured</td></tr>");
    } else {
      response->printf(
        "<tr><td>MQTT Status:</td><td>%s</td></tr>",
        mqtt->connected() ? "Connected" : "Disconnected");
      response->printf(
        "<tr><td>MQTT Host:</td><td>%s</td></tr>",
        config->getMqttHost().c_str());
      response->printf(
        "<tr><td>MQTT Port:</td><td>%u</td></tr>",
        config->getMqttPort());
      response->printf(
        "<tr><td>MQTT Prefix:</td><td>%s</td></tr>",
        config->getMqttPrefix().c_str());
    }
    response->print("</table><p></p>");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /reboot");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Really?");
    sendButton(response, "Back", "/");
    response->print("<form method=\"post\">"
        "<button class=\"r\">Yes, do it!</button>"
      "</form>");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/reboot", HTTP_POST, [mqtt](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /reboot");
    request->redirect("/");
    dbgln("[webserver] rebooting...")
    mqtt->disconnect();
    ESP.restart();
    dbgln("[webserver] rebooted...")
  });
  server->on("/config", HTTP_GET, [config](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /config");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Config");
    response->print("<form method=\"post\">");
    response->print("<table>"
      "<tr>"
        "<td>"
          "<label for=\"i\">Polling interval (ms)</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"1\" id=\"i\" name=\"i\" value=\"%u\">", config->getInterval());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
          "<label for=\"c\">Nr of modules</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"1\" max=\"12\" id=\"c\" name=\"c\" value=\"%u\">", config->getModuleCount());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
          "<label for=\"h\">MQTT host</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"text\" id=\"h\" name=\"h\" value=\"%s\">", config->getMqttHost().c_str());
    response->print("</td>"
      "</tr>"
      "<tr>"
        "<td>"
          "<label for=\"p\">MQTT port</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"1\" max=\"65535\" id=\"p\" name=\"p\" value=\"%d\">", config->getMqttPort());
    response->print("</td>"
        "</tr>"
        "<tr>"
          "<td>"
            "<label for=\"pr\">MQTT prefix</label>"
          "</td>"
          "<td>");
    response->printf("<input type=\"text\" id=\"pr\" name=\"pr\" value=\"%s\">", config->getMqttPrefix().c_str());
    response->print("</td>"
          "</tr>"
        "<tr>"
          "<td>"
          "<label for=\"u\">MQTT username</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"text\" id=\"u\" name=\"u\" value=\"%s\">", config->getMqttUsername().c_str());
    response->print("</td>"
      "</tr>"
        "<tr>"
          "<td>"
          "<label for=\"pa\">MQTT password</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"password\" id=\"pa\" name=\"pa\" value=\"%s\">", config->getMqttPassword().c_str());
    response->print("</td>"
      "</tr>"
        "</table>");
    response->print("<h4>Polled info types</h4>");
    response->print("<table>");
    struct InfoTypeRow { const char *name; const char *label; uint8_t flag; };
    const InfoTypeRow infoTypes[] = {
      {"it_man", "Manufacturer info",        INFO_FLAG_MANUFACTURER},
      {"it_fw",  "Firmware info",            INFO_FLAG_FIRMWARE},
      {"it_sn",  "Serial number",            INFO_FLAG_SERIALNUMBER},
      {"it_sys", "System parameter",         INFO_FLAG_SYSTEM_PARAMETER},
      {"it_cd",  "Charge/discharge mgmt",    INFO_FLAG_CHARGE_DISCHARGE},
      {"it_an",  "Analog values",            INFO_FLAG_ANALOG_VALUE},
      {"it_al",  "Alarm info",               INFO_FLAG_ALARM},
    };
    for (auto &row : infoTypes) {
      response->printf(
        "<tr><td><label for=\"%s\">%s</label></td>"
        "<td><input type=\"checkbox\" id=\"%s\" name=\"%s\" value=\"1\"%s></td></tr>",
        row.name, row.label, row.name, row.name,
        config->isInfoTypeEnabled(row.flag) ? " checked" : "");
    }
    response->print("</table>");
    response->print("<button class=\"r\">Save</button>"
      "</form>"
      "<p></p>");
    sendButton(response, "Back", "/");
    // response->print("<script>"
    //   "(function(){"
    //     "var s = document.querySelectorAll('select[data-value]');"
    //     "for(d of s){"
    //       "d.querySelector(`option[value='${d.dataset.value}']`).selected=true"
    //   "}})();"
    //   "</script>");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/config", HTTP_POST, [config, onMqttConfigChanged](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /config");
    if (request->hasParam("c", true)){
      auto nr = request->getParam("c", true)->value().toInt();
      config->setModuleCount(nr);
      dbgln("[webserver] saved number of modules");
    }
    if (request->hasParam("i", true)){
      auto interval = request->getParam("i", true)->value().toInt();
      config->setInterval(interval);
      dbgln("[webserver] saved interval");
    }
    if (request->hasParam("h", true)){
      auto host = request->getParam("h", true)->value();
      config->setMqttHost(host);
      dbgln("[webserver] saved mqtt host");
    }
    if (request->hasParam("p", true)){
      auto port = request->getParam("p", true)->value().toInt();
      config->setMqttPort(port);
      dbgln("[webserver] saved mqtt port");
    }
    if (request->hasParam("pr", true)){
      auto prefix = request->getParam("pr", true)->value();
      config->setMqttPrefix(prefix);
      dbgln("[webserver] saved mqtt prefix");
    }
    if (request->hasParam("u", true)){
      auto username = request->getParam("u", true)->value();
      config->setMqttUsername(username);
      dbgln("[webserver] saved mqtt username");
    }
    if (request->hasParam("pa", true)){
      auto password = request->getParam("pa", true)->value();
      config->setMqttPassword(password);
      dbgln("[webserver] saved mqtt password");
    }
    uint8_t infoFlags = 0;
    if (request->hasParam("it_man", true)) infoFlags |= INFO_FLAG_MANUFACTURER;
    if (request->hasParam("it_fw",  true)) infoFlags |= INFO_FLAG_FIRMWARE;
    if (request->hasParam("it_sn",  true)) infoFlags |= INFO_FLAG_SERIALNUMBER;
    if (request->hasParam("it_sys", true)) infoFlags |= INFO_FLAG_SYSTEM_PARAMETER;
    if (request->hasParam("it_cd",  true)) infoFlags |= INFO_FLAG_CHARGE_DISCHARGE;
    if (request->hasParam("it_an",  true)) infoFlags |= INFO_FLAG_ANALOG_VALUE;
    if (request->hasParam("it_al",  true)) infoFlags |= INFO_FLAG_ALARM;
    config->setInfoFlags(infoFlags);
    dbgln("[webserver] saved info flags");
    if (onMqttConfigChanged) {
      onMqttConfigChanged();
    }
    request->redirect("/");    
  });
  server->on("/debug", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /debug");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Debug");
    sendDebugForm(response, "2", "0", "2", "4F", "");
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/debug", HTTP_POST, [client](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /debug");
    String major = "2";
    if (request->hasParam("v1", true)){
      major = request->getParam("v1", true)->value();
    }
    String minor = "0";
    if (request->hasParam("v2", true)){
      minor = request->getParam("v2", true)->value();
    }
    String address = "2";
    if (request->hasParam("a", true)){
      address = request->getParam("a", true)->value();
    }
    String cid2 = "4F";
    if (request->hasParam("c2", true)){
      cid2 = request->getParam("c2", true)->value();
    }
    String info = "";
    if (request->hasParam("i", true)){
      info = request->getParam("i", true)->value();
    }
    Pylonframe frame = Pylonframe();
    frame.MajorVersion = major.toInt();
    frame.MinorVersion = minor.toInt();
    frame.Address = strtoul(address.c_str(), 0, HEX);
    frame.Cid2 = (CommandInformation)strtoul(cid2.c_str(), 0, HEX);
    auto type = frame.Cid2;
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Debug");
    response->print("<pre>&gt;&nbsp;");
    frame.WriteTo(response);
    response->print("<br/>&lt;&nbsp;");
    frame = client->SendCommand(frame);
    frame.print(response);
    if (frame.HasError){
      response->print("<br/>Something went wrong - unable to parse response<br/><br/>");
    }
   
    switch(type){
      case CommandInformation::Serialnumber:
      {
        auto serialnumber = Pylonframe::PylonSerialnumber(frame.Info);
        serialnumber.print(response);
        break;
      }
      case CommandInformation::ManufacturerInfo:
      {
        auto manufacturer = Pylonframe::PylonManufacturerInfo(frame.Info);
        manufacturer.print(response);
        break;
      }
      case CommandInformation::FirmwareInfo:
      {
        auto firmware = Pylonframe::PylonFirmwareInfo(frame.Info);
        firmware.print(response);
        break;
      }
      case CommandInformation::SystemParameterFixedPoint:
      {
        auto system = Pylonframe::PylonSystemParameter(frame.Info);
        system.print(response);
        break;
      }
      case CommandInformation::GetChargeDischargeManagementInfo:
      {
        auto charge = Pylonframe::PylonChargeDischargeManagementInfo(frame.Info);
        charge.print(response);
        break;
      }
      case CommandInformation::AlarmInfo:
      {
        auto alarm = Pylonframe::PylonAlarmInfo(frame.Info);
        alarm.print(response);
        break;
      }
      case CommandInformation::AnalogValueFixedPoint:
      {
        auto analog = Pylonframe::PylonAnalogValue(frame.Info);
        analog.print(response);
        break;
      }
      case CommandInformation::ProtocolVersion:
      {
        response->printf("<br/>Major: %u<br/>Minor: %u",
          frame.MajorVersion, frame.MinorVersion);
        break;
      }
    }
    response->print("</pre>");
    sendDebugForm(response, major, minor, address, cid2, info);
    sendButton(response, "Back", "/");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /update");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Firmware Update");
    response->print(
      "<form id=\"otaForm\" method=\"post\" enctype=\"multipart/form-data\">"
        "<input type=\"file\" name=\"file\" id=\"file\" required/>"
        "<p></p>"
        "<button class=\"r\" id=\"otaSubmit\">Upload</button>"
      "</form>"
      "<progress id=\"otaProgress\" value=\"0\" max=\"100\" "
        "style=\"width:100%;display:none\"></progress>"
      "<div id=\"otaStatus\"></div>"
      "<p></p>");
    sendButton(response, "Back", "/");
    response->print(
      "<script>"
      "(function(){"
        "var form=document.getElementById('otaForm');"
        "var btn=document.getElementById('otaSubmit');"
        "var prog=document.getElementById('otaProgress');"
        "var stat=document.getElementById('otaStatus');"
        "form.addEventListener('submit',function(e){"
          "e.preventDefault();"
          "var f=document.getElementById('file');"
          "if(!f.files.length){return;}"
          "btn.disabled=true;"
          "var data=new FormData(form);"
          "var xhr=new XMLHttpRequest();"
          "xhr.open('POST','/update');"
          "prog.style.display='block';"
          "prog.value=0;"
          "stat.textContent='Uploading...';"
          "var rebooting=false;"
          "var startWaiting=function(){"
            "if(rebooting)return;"
            "rebooting=true;"
            "prog.value=100;"
            "var start=Date.now();"
            "var poll=function(){"
              "var elapsed=Math.round((Date.now()-start)/1000);"
              "stat.textContent='Flashing and rebooting... waiting for device ('+elapsed+'s)';"
              "fetch('/favicon.ico?_='+Date.now(),{cache:'no-store'}).then(function(r){"
                "if(Date.now()-start>5000){location.replace('/');}"
                "else{setTimeout(poll,1000);}"
              "}).catch(function(){setTimeout(poll,1000);});"
            "};"
            "stat.textContent='Flashing and rebooting...';"
            "setTimeout(poll,3000);"
          "};"
          "xhr.upload.onprogress=function(ev){"
            "if(ev.lengthComputable){"
              "var pct=Math.round(ev.loaded/ev.total*100);"
              "prog.value=pct;"
              "if(pct<100)stat.textContent='Uploading '+pct+'%';"
              "if(pct>=100)startWaiting();"
            "}"
          "};"
          "xhr.upload.onload=function(){startWaiting();};"
          "xhr.onload=function(){"
            "if(xhr.status>=200&&xhr.status<400){"
              "startWaiting();"
            "}else if(!rebooting){"
              "btn.disabled=false;"
              "stat.textContent='Upload failed: '+xhr.status+' '+xhr.statusText;"
            "}"
          "};"
          "xhr.onerror=function(){"
            "if(rebooting)return;"
            "btn.disabled=false;"
            "stat.textContent='Upload failed (network error)';"
          "};"
          "xhr.send(data);"
        "});"
      "})();"
      "</script>");
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    dbgln("[webserver] OTA finished");
    if (Update.hasError()){
      auto *response = request->beginResponse(500, "text/plain", "Ota failed");
      response->addHeader("Connection", "close");
      request->send(response);
    }
    else{
      request->redirect("/");
    }
    ESP.restart();
  }, [&](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    dbg("[webserver] OTA progress ");dbgln(index);
    if (!index) {
      //TODO add MD5 Checksum and Update.setMD5
      int cmd = (filename == "filesystem") ? U_SPIFFS : U_FLASH;
      if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
        Update.printError(Serial);
        return request->send(400, "text/plain", "OTA could not begin");
      }
    }
    // Write chunked data to the free sketch space
    if(len){
      if (Update.write(data, len) != len) {
        return request->send(400, "text/plain", "OTA could not write data");
      }
    }
    if (final) { // if the final flag is set then this is the last frame of data
      if (!Update.end(true)) { //true to set the size to the current progress
        Update.printError(Serial);
        return request->send(400, "text/plain", "Could not end OTA");
      }
    }else{
      return;
    }
  });
  // IMPORTANT: more specific routes must be registered before /wifi, because
  // ESPAsyncWebServer's /wifi handler also matches /wifi/* (it accepts URLs
  // that start with "/wifi/"). Registering sub-routes first ensures they win.
  server->on("/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /wifi/scan");
    WiFi.scanDelete();
    WiFi.scanNetworks(true /* async */);
    request->redirect("/wifi");
  });
  server->on("/wifi/erase", HTTP_POST, [wm](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /wifi/erase");
    request->redirect("/");
    wm->erase();
    dbgln("[webserver] erased wifi config");
    dbgln("[webserver] rebooting...");
    ESP.restart();
    dbgln("[webserver] rebooted...");
  });
  server->on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /wifi");
    auto *response = request->beginResponseStream("text/html");
    renderWifiPage(response);
    request->send(response);
  });
  server->on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /wifi");
    String ssid = request->hasParam("ssid", true) ? request->getParam("ssid", true)->value() : String();
    String psk  = request->hasParam("psk",  true) ? request->getParam("psk",  true)->value()  : String();
    if (ssid.length() == 0) {
      request->redirect("/wifi");
      return;
    }
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "WiFi");
    response->print("<p>Saved. Rebooting and reconnecting to <b>");
    response->print(ssid);
    response->print("</b>...</p>");
    sendResponseTrailer(response);
    request->send(response);

    WiFi.disconnect(true /* wifi off */, true /* erase ap */);
    WiFi.begin(ssid.c_str(), psk.c_str());
    delay(200);
    ESP.restart();
  });
  server->on("/favicon.ico", [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /favicon.ico");
    request->send(204);//TODO add favicon
  });
  server->on("/style.css", [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /style.css");
    auto *response = request->beginResponse(200, "text/css",
    "body{"    
      "font-family:sans-serif;"
	    "text-align: center;"
      "background: #252525;"
	    "color: #faffff;"
    "}"
    "#content{"
	    "display: inline-block;"
	    "min-width: 340px;"
    "}"
    "button{"
	    "width: 100%;"
	    "line-height: 2.4rem;"
	    "background: #1fa3ec;"
	    "border: 0;"
	    "border-radius: 0.3rem;"
	    "font-size: 1.2rem;"
      "-webkit-transition-duration: 0.4s;"
      "transition-duration: 0.4s;"
	    "color: #faffff;"
    "}"
    "button:hover{"
	    "background: #0e70a4;"
    "}"
    "button.r{"
	    "background: #d43535;"
    "}"
    "button.r:hover{"
	    "background: #931f1f;"
    "}"
    "table{"
      "text-align:left;"
      "width:100%;"
    "}"
    "input{"
      "width:100%;"
    "}"
    ".e{"
      "color:red;"
    "}"
    "pre{"
      "text-align:left;"
    "}"
    );
    response->addHeader("ETag", __DATE__ "" __TIME__);
    request->send(response);
  });
  server->onNotFound([](AsyncWebServerRequest *request){
    dbg("[webserver] request to ");dbg(request->url());dbgln(" not found");
    request->send(404, "text/plain", "404");
  });
}


void sendResponseHeader(AsyncResponseStream *response, const char *title){
    response->print("<!DOCTYPE html>"
      "<html lang=\"en\" class=\"\">"
      "<head>"
      "<meta charset='utf-8'>"
      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"/>");
    response->printf("<title>ESP32 Pylontech - %s</title>", title);
    response->print("<link rel=\"stylesheet\" href=\"/style.css\">"
      "</head>"
      "<body>"
      "<h2>ESP32 Pylontech</h2>");
    response->printf("<h3>%s</h3>", title);
    response->print("<div id=\"content\">");
}

void sendResponseTrailer(AsyncResponseStream *response){
    response->print("</div></body></html>");
}

void sendButton(AsyncResponseStream *response, const char *title, const char *action, const char *css){
    response->printf(
      "<form method=\"get\" action=\"%s\">"
        "<button class=\"%s\">%s</button>"
      "</form>"
      "<p></p>", action, css, title);
}

void sendTableRow(AsyncResponseStream *response, const char *name, uint32_t value){
    response->printf(
      "<tr>"
        "<td>%s:</td>"
        "<td>%d</td>"
      "</tr>", name, value);
}

void sendDebugForm(AsyncResponseStream *response, String major, String minor, String address, String cid2, String info){
    response->print("<form method=\"post\">");
    response->print("<table>"
      "<tr>"
        "<td>"
          "<label for=\"v1\">Major Version</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"9\" id=\"v1\" name=\"v1\" value=\"%s\">", major);
    response->print("</td>"
      "</tr>"
      "<tr>"
        "<td>"
          "<label for=\"v2\">Minor Version</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"0\" max=\"9\" id=\"v2\" name=\"v2\" value=\"%s\">", minor);
    response->print("</td>"
      "</tr>"
      "<tr>"
        "<td>"
          "<label for=\"a\">Address</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"number\" min=\"2\" max=\"13\" id=\"a\" name=\"a\" value=\"%s\">", address);
    response->print("</td>"
      "</tr>"
        "<tr>"
        "<td>"
          "<label for=\"c2\">CID2</label>"
        "</td>"
        "<td>");
    response->printf("<select id=\"c2\" name=\"c2\" data-value=\"%s\">", cid2);
    response->print("<option value=\"42\">Get analog value, fixed point</option>"
              "<option value=\"44\">Get alarm info</option>"
              "<option value=\"47\">Get system parameter, fixed point</option>"
              "<option value=\"4F\">Get protocol version</option>"
              "<option value=\"51\">Get manufacturer info</option>"
              "<option value=\"92\">Get charge, discharge management info</option>"
              "<option value=\"93\">Get SN number of battery</option>"
              "<option value=\"94\">Set value of charge, discharge management info</option>"
              "<option value=\"95\">Turnoff</option>"
              "<option value=\"96\">Get firmware info</option>"
            "</select>"
          "</td>"
        "<td>"
      "</tr>"
        "<tr>"
        "<td>"
          "<label for=\"i\">Info</label>"
        "</td>"
        "<td>");
    response->printf("<input type=\"text\" maxlength=\"4095\" id=\"i\" name=\"i\" value=\"%s\">", info);
    response->print("</td>"
          "</tr>"
        "</table>"
        "<button class=\"r\">Send</button>"
      "</form>"
      "<p></p>");
    response->print("<script>"
      "(function(){"
        "var s = document.querySelectorAll('select[data-value]');"
        "for(d of s){"
          "d.querySelector(`option[value='${d.dataset.value}']`).selected=true"
      "}})();"
      "</script>");
}