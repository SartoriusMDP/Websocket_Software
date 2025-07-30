#include <Arduino.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

#include "wifiManager.h"
#include "websocketManager.h"
#include "ControllerConfig.h"

const char *ssid = "chuck";
const char *password = "diesel2424";
ControllerConfig controllerConfig; // Global instance

AsyncWebServer server(80);

void setup()
{
  Serial.begin(115200);

  if (!LittleFS.begin(true))
  {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // Initialize Wi-Fi
  connectToWiFi(ssid, password);

  // Initialize WebSocket
  initWebSocket();
  server.addHandler(&ws); // 'ws' comes from websocketManager.h/cpp

  // Serve main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->redirect("/website/index.html"); });

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->redirect("/website/index.html"); });

  server.on("/website/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    AsyncWebServerResponse* response = request->beginResponse(LittleFS, "/website/index.html", "text/html");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    request->send(response); });

  // Serve all static files in /website/
  server.serveStatic("/", LittleFS, "/");

  server.begin();

  // ControllerConfig example usage
  // controllerConfig.init(); // Initialize hardware/data

  // if (LittleFS.remove("/State/Config.txt"))
  // {
  //   Serial.println("File deleted successfully");
  // }
  // else
  // {
  //   Serial.println("Failed to delete file");
  // }

  controllerConfig.startup(); // Load saved state

  File debugFile = LittleFS.open("/State/Config.txt", "r");
  if (!debugFile)
  {
    Serial.println("Failed to open /State/Config.txt for reading");
    return;
  }

  Serial.println("GRRRRHEHFLKJDSLJFLKDSJ of /State/Config.txt:");
  while (debugFile.available())
  {
    Serial.println(debugFile.readStringUntil('\n'));
  }
  debugFile.close();
  Serial.println("GRRRRHEHFLKJDSLJFLKDSJ of /State/Config.txt:");

  // controllerConfig.startupPackager(); // Prepare startup messages

  // Broadcast state over WebSocket
  // for (const auto &msg : temp.startupStorage)
  // {
  //   Serial.println(msg);
  //   notifyClients(msg); // send to all connected WebSocket clients
  // }
}

void loop()
{
  ws.cleanupClients(); // Keep WebSocket connections clean
}
