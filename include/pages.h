#ifndef PAGES_H
    #define PAGES_H
    #include <WiFiManager.h>
    #include <ESPAsyncWebServer.h>
    #include <Update.h>
    #include "config.h"

    void setupPages(AsyncWebServer *server, WiFiManager *wm);
    void sendResponseHeader(AsyncResponseStream *response, const char *title);
    void sendResponseTrailer(AsyncResponseStream *response);
    void sendButton(AsyncResponseStream *response, const char *title, const char *action, const char *css = "");
    void sendTableRow(AsyncResponseStream *response, const char *name, uint32_t value);
    void sendDebugForm(AsyncResponseStream *response, String command);
#endif /* PAGES_H */