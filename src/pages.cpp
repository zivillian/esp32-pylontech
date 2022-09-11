#include "pages.h"

void setupPages(AsyncWebServer *server, WiFiManager *wm, Pylonclient *client){
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "Main");
    sendButton(response, "Status", "status");
    sendButton(response, "Config", "config");
    sendButton(response, "Debug", "debug");
    sendButton(response, "Firmware update", "update");
    sendButton(response, "WiFi reset", "wifi", "r");
    sendButton(response, "Reboot", "reboot", "r");
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
  server->on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /reboot");
    request->redirect("/");
    dbgln("[webserver] rebooting...")
    ESP.restart();
    dbgln("[webserver] rebooted...")
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
    response->print("<form method=\"post\" enctype=\"multipart/form-data\">"
      "<input type=\"file\" name=\"file\" id=\"file\" required/>"
      "<p></p>"
      "<button class=\"r\">Upload</button>"
      "</form>"
      "<p></p>");
    sendButton(response, "Back", "/");
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
  server->on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    dbgln("[webserver] GET /wifi");
    auto *response = request->beginResponseStream("text/html");
    sendResponseHeader(response, "WiFi reset");
    response->print("<p class=\"e\">"
        "This will delete the stored WiFi config<br/>"
        "and restart the ESP in AP mode.<br/> Are you sure?"
      "</p>");
    sendButton(response, "Back", "/");
    response->print("<p></p>"
      "<form method=\"post\">"
        "<button class=\"r\">Yes, do it!</button>"
      "</form>");    
    sendResponseTrailer(response);
    request->send(response);
  });
  server->on("/wifi", HTTP_POST, [wm](AsyncWebServerRequest *request){
    dbgln("[webserver] POST /wifi");
    request->redirect("/");
    wm->erase();
    dbgln("[webserver] erased wifi config");
    dbgln("[webserver] rebooting...");
    ESP.restart();
    dbgln("[webserver] rebooted...");
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
    response->print("<link rel=\"stylesheet\" href=\"style.css\">"
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