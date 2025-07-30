#include "websocketManager.h"
#include "ControllerConfig.h"

#include <Arduino.h>

AsyncWebSocket ws("/ws");

// External instance from main.cpp
extern ControllerConfig controllerConfig;
void sendBatchStartupMessages(AsyncWebSocketClient *client);

void notifyClients(const String &message)
{
    ws.textAll(message);
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("Client connected: %u\n", client->id());
        controllerConfig.startupPackager();
        sendBatchStartupMessages(client);
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("Client disconnected: %u\n", client->id());
        Serial.println("Saving contents to file");
        controllerConfig.save_to_file();
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
        {
            String msg = String((char *)data).substring(0, len);
            Serial.printf("Received: %s\n", msg.c_str());

            String returnmsg = controllerConfig.process_command(msg);
            ws.textAll(returnmsg);
        }
    }
}

void initWebSocket()
{
    ws.onEvent(onWebSocketEvent);
}

void sendBatchStartupMessages(AsyncWebSocketClient *client)
{
    JsonDocument batchDoc; // Adjust size as needed
    JsonArray arr = batchDoc.to<JsonArray>();

    for (const auto &msg : controllerConfig.startupStorage)
    {
        // Parse each JSON string into JsonDocument, then add to array
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, msg);
        if (!err)
        {
            arr.add(doc);
        }
        else
        {
            Serial.println("JSON parse error in batch creation");
        }
    }

    String batchString;
    serializeJson(batchDoc, batchString);

    client->text(batchString);
}