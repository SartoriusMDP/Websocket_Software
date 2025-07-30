#include "ControllerConfig.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <vector>
using namespace std;

// Constructor: initialize member variables here
ControllerConfig::ControllerConfig()
    : is_started(false),
      devmode_enabled(false),
      averageTemperature(0.0),
      averageHumidity(0.0),
      carbonDioxideReading(0.0),
      currentAmps(0.0),
      systemStatus("Off"),
      maxCurrentValue(0.0)
{
}

void ControllerConfig::init()
{
    actuator_storage.clear();
    pump_storage.clear();
    waterlevel_storage.clear();
    sensor_storage.clear();

    for (const auto &name : actuator_names)
    {
        actuator_storage.push_back({name, false, false, 0.0, 0.0, 0.0, 0.0, 0.0});
    }

    for (const auto &name : pump_names)
    {
        pump_storage.push_back({name, false});
    }

    for (const auto &name : waterlevel_names)
    {
        waterlevel_storage.push_back({name, false});
    }

    for (const auto &name : sensor_names)
    {
        sensor_storage.push_back({name, ""});
    }
}

// Save current configuration to LittleFS file
void ControllerConfig::save_to_file()
{
    LittleFS.mkdir("/State");
    File storageFile = LittleFS.open("/State/Config.txt", "w");
    if (!storageFile)
    {
        Serial.println("Failed to open /State/Config.txt for writing");
        init();
        return;
    }

    storageFile.println("is_started " + String(is_started));
    storageFile.println("devmode_enabled " + String(devmode_enabled));
    storageFile.println("averageTemperature " + String(averageTemperature));
    storageFile.println("averageHumidity " + String(averageHumidity));
    storageFile.println("carbonDioxideReading " + String(carbonDioxideReading));
    storageFile.println("systemStatus " + systemStatus);
    storageFile.println("maxCurrentValue " + String(maxCurrentValue));

    for (const auto &actuator : actuator_storage)
    {
        String line = actuator.name + " " +
                      String(actuator.is_online) + " " +
                      String(actuator.is_auto) + " " +
                      String(actuator.P_gain) + " " +
                      String(actuator.I_gain) + " " +
                      String(actuator.D_gain) + " " +
                      String(actuator.setpoint) + " " +
                      String(actuator.actual);
        storageFile.println(line);
    }

    for (const auto &pump : pump_storage)
    {
        storageFile.println(pump.name + " " + String(pump.is_online));
    }

    for (const auto &waterlevel : waterlevel_storage)
    {
        storageFile.println(waterlevel.name + " " + String(waterlevel.is_online));
    }

    for (const auto &sensor : sensor_storage)
    {
        storageFile.println(sensor.name + " " + sensor.value);
    }

    storageFile.close();

    // Debug: Read back the file and print to Serial
    File debugFile = LittleFS.open("/State/Config.txt", "r");
    if (!debugFile)
    {
        Serial.println("Failed to open /State/Config.txt for reading");
        return;
    }

    Serial.println("Contents of /State/Config.txt:");
    while (debugFile.available())
    {
        Serial.println(debugFile.readStringUntil('\n'));
    }
    debugFile.close();
}
void ControllerConfig::startup()
{
    actuator_storage.clear();
    pump_storage.clear();
    waterlevel_storage.clear();
    sensor_storage.clear();

    File file = LittleFS.open("/State/Config.txt", "r");
    if (!file)
    {
        Serial.println("Config file not found. Initializing default values.");
        init();
        return;
    }

    // --- Configuration constants ---
    const int NUM_ACTUATORS = 10; // 5 normal + 5 humidity actuators
    const int NUM_PUMPS = 5;
    const int NUM_WATERLEVELS = 5;
    const int NUM_SENSORS = 16;
    const int META_LINES = 7;

    const int ACTUATOR_START = META_LINES + 1;
    const int ACTUATOR_END = ACTUATOR_START + NUM_ACTUATORS;

    const int PUMP_START = ACTUATOR_END;
    const int PUMP_END = PUMP_START + NUM_PUMPS;

    const int WATERLEVEL_START = PUMP_END;
    const int WATERLEVEL_END = WATERLEVEL_START + NUM_WATERLEVELS;

    const int SENSOR_START = WATERLEVEL_END;
    const int SENSOR_END = SENSOR_START + NUM_SENSORS;

    int lineCount = 0;

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0)
            continue;

        lineCount++;

        if (lineCount == 1)
            is_started = line.substring(line.indexOf(' ') + 1).toInt();
        else if (lineCount == 2)
            devmode_enabled = line.substring(line.indexOf(' ') + 1).toInt();
        else if (lineCount == 3)
            averageTemperature = line.substring(line.indexOf(' ') + 1).toFloat();
        else if (lineCount == 4)
            averageHumidity = line.substring(line.indexOf(' ') + 1).toFloat();
        else if (lineCount == 5)
            carbonDioxideReading = line.substring(line.indexOf(' ') + 1).toFloat();
        else if (lineCount == 6)
            systemStatus = line.substring(line.indexOf(' ') + 1);
        else if (lineCount == 7)
            maxCurrentValue = line.substring(line.indexOf(' ') + 1).toFloat();
        else if (lineCount >= ACTUATOR_START && lineCount < ACTUATOR_END)
        {
            actuators actuator;
            int spaceIndex;
            String parts[8];
            int idx = 0;

            // Extract name
            spaceIndex = line.indexOf(' ');
            actuator.name = line.substring(0, spaceIndex);
            line = line.substring(spaceIndex + 1);

            // Extract 7 remaining values
            while (line.length() > 0 && idx < 7)
            {
                spaceIndex = line.indexOf(' ');
                if (spaceIndex == -1)
                {
                    parts[idx++] = line;
                    break;
                }
                parts[idx++] = line.substring(0, spaceIndex);
                line = line.substring(spaceIndex + 1);
            }

            actuator.is_online = parts[0].toInt();
            actuator.is_auto = parts[1].toInt();
            actuator.P_gain = parts[2].toFloat();
            actuator.I_gain = parts[3].toFloat();
            actuator.D_gain = parts[4].toFloat();
            actuator.setpoint = parts[5].toFloat();
            actuator.actual = parts[6].toFloat();

            actuator_storage.push_back(actuator);
        }
        else if (lineCount >= PUMP_START && lineCount < PUMP_END)
        {
            int split = line.lastIndexOf(' ');
            pumps pump;
            pump.name = line.substring(0, split);
            pump.is_online = line.substring(split + 1).toInt();
            pump_storage.push_back(pump);
        }
        else if (lineCount >= WATERLEVEL_START && lineCount < WATERLEVEL_END)
        {
            int split = line.lastIndexOf(' ');
            waterlevel wls;
            wls.name = line.substring(0, split);
            wls.is_online = line.substring(split + 1).toInt();
            waterlevel_storage.push_back(wls);
        }
        else if (lineCount >= SENSOR_START && lineCount < SENSOR_END)
        {
            sensor s;
            s.name = line;
            s.value = ""; // Default empty value
            sensor_storage.push_back(s);
        }
    }

    file.close();
    Serial.println("Configuration successfully loaded (no-space mode).");
}

// Placeholder for command processing function
String ControllerConfig::process_command(String &command)
{
    JsonDocument doc;
    deserializeJson(doc, command);

    if (doc["id"] == "UpdateActuatorPower" || doc["id"] == "UpdateActuatorMode" || doc["id"] == "UpdatePIDInput" || doc["id"] == "UpdatePIDSetpoint" || doc["id"] == "UpdatePIDActual")
    {
        UpdateActuatorHelper(doc);
    }
    else if (doc["id"] == "UpdateSystemOverview")
    {
        UpdateSystemHelper(doc);
    }
    else if (doc["id"] == "UpdateEnvironmentSensor")
    {
        UpdateEnvironmentHelper(doc);
    }
    else if (doc["id"] == "UpdatePumpStatus" || doc["id"] == "UpdateWaterLevel")
    {
        UpdatePumpWaterHelper(doc);
    }
    else if (doc["id"] == "UpdateDevMode" || doc["id"] == "UpdateStart" || doc["id"] == "UpdateStop")
    {
        UpdateMainButtonHelper(doc);
    }

    if (command.indexOf("Update") != -1)
    {
        return command;
    }

    JsonDocument output_JSON;

    if (doc["id"] == "LogStart")
    {
        is_started = true;
        output_JSON["id"] = "UpdateStart";
    }
    else if (doc["id"] == "LogStop")
    {
        is_started = false;
        output_JSON["id"] = "UpdateStop";
    }
    else if (doc["id"] == "LogDeveloperMode")
    {
        devmode_enabled = !devmode_enabled;
        output_JSON["id"] = "UpdateDevMode";
        output_JSON["state"] = (devmode_enabled == true) ? "On" : "Off";
    }
    else if (doc["id"] == "LogPumpStatus")
    {
        for (auto &pumps : pump_storage)
        {
            if (pumps.name == doc["name"])
            {
                pumps.is_online = !pumps.is_online;
                output_JSON["id"] = "UpdatePumpStatus";
                output_JSON["name"] = doc["name"];
                output_JSON["state"] = (pumps.is_online == true) ? "On" : "Off";
            }
        }
    }
    else if (doc["id"] == "LogActuatorPower" || doc["id"] == "LogActuatorMode")
    {
        for (auto &actuators : actuator_storage)
        {
            if (actuators.name == doc["name"])
            {
                if (doc["id"] == "LogActuatorPower")
                {
                    actuators.is_online = !actuators.is_online;
                    output_JSON["id"] = "UpdateActuatorPower";
                    output_JSON["name"] = doc["name"];
                    output_JSON["state"] = (actuators.is_online == true) ? "On" : "Off";
                }
                else
                {
                    actuators.is_auto = !actuators.is_auto;
                    output_JSON["id"] = "UpdateActuatorMode";
                    output_JSON["name"] = doc["name"];
                    output_JSON["state"] = (actuators.is_auto == true) ? "Auto" : "Manual";
                }
                break;
            }
        }
    }
    else if (doc["id"] == "LogActuatorPID" || doc["id"] == "LogActuatorSetpoint")
    {
        UpdateActuatorHelper(doc);
        if (doc["id"] == "LogActuatorPID")
        {
            output_JSON["id"] = "UpdatePIDInput";
            output_JSON["name"] = doc["name"];
            output_JSON["P"] = doc["P"];
            output_JSON["I"] = doc["I"];
            output_JSON["D"] = doc["D"];
        }
        else
        {
            output_JSON["id"] = "UpdatePIDSetpoint";
            output_JSON["name"] = doc["name"];
            output_JSON["value"] = doc["value"];
        }
    }

    String outputstr;
    serializeJson(output_JSON, outputstr);
    return outputstr;
}

void ControllerConfig::startupPackager()
{
    startupStorage.clear();

    JsonDocument StartStopJSON;
    StartStopJSON["id"] = (is_started == true) ? "UpdateStart" : "UpdateStop";
    startupStorage.push_back(ConvertJsonHelper(StartStopJSON));

    JsonDocument DevModeJSON;
    DevModeJSON["id"] = "UpdateDevMode";
    DevModeJSON["state"] = (devmode_enabled == true) ? "On" : "Off";
    startupStorage.push_back(ConvertJsonHelper(DevModeJSON));

    JsonDocument systemTempJSON;
    systemTempJSON["id"] = "UpdateSystemOverview";
    systemTempJSON["name"] = "averageTemperature";
    systemTempJSON["value"] = averageTemperature;
    startupStorage.push_back(ConvertJsonHelper(systemTempJSON));

    JsonDocument systemHumJSON;
    systemHumJSON["id"] = "UpdateSystemOverview";
    systemHumJSON["name"] = "averageHumidity";
    systemHumJSON["value"] = averageHumidity;
    startupStorage.push_back(ConvertJsonHelper(systemHumJSON));

    JsonDocument systemCo2JSON;
    systemCo2JSON["id"] = "UpdateSystemOverview";
    systemCo2JSON["name"] = "carbonDioxideReading";
    systemCo2JSON["value"] = carbonDioxideReading;
    startupStorage.push_back(ConvertJsonHelper(systemCo2JSON));

    JsonDocument systemCurrentAmpsJSON;
    systemCurrentAmpsJSON["id"] = "UpdateSystemOverview";
    systemCurrentAmpsJSON["name"] = "currentAmps";
    systemCurrentAmpsJSON["value"] = currentAmps;
    startupStorage.push_back(ConvertJsonHelper(systemCurrentAmpsJSON));

    JsonDocument systemStatusJSON;
    systemStatusJSON["id"] = "UpdateSystemOverview";
    systemStatusJSON["name"] = "systemStatus";
    systemStatusJSON["value"] = systemStatus;
    startupStorage.push_back(ConvertJsonHelper(systemStatusJSON));

    JsonDocument systemMaxAmpsJSON;
    systemMaxAmpsJSON["id"] = "UpdateSystemOverview";
    systemMaxAmpsJSON["name"] = "maxCurrentValue";
    systemMaxAmpsJSON["value"] = maxCurrentValue;
    startupStorage.push_back(ConvertJsonHelper(systemMaxAmpsJSON));

    JsonDocument environmentJSON;
    environmentJSON["id"] = "UpdateEnvironmentSensor";

    for (auto &sensors : sensor_storage)
    {
        environmentJSON["name"] = sensors.name;
        environmentJSON["value"] = sensors.value;
        startupStorage.push_back(ConvertJsonHelper(environmentJSON));
    }

    JsonDocument pumpJSON;
    pumpJSON["id"] = "UpdatePumpStatus";
    for (auto &pumps : pump_storage)
    {
        pumpJSON["name"] = pumps.name;
        pumpJSON["state"] = (pumps.is_online == true) ? "On" : "Off";
        startupStorage.push_back(ConvertJsonHelper(pumpJSON));
    }

    JsonDocument waterlevelJSON;
    waterlevelJSON["id"] = "UpdateWaterLevel";
    for (auto &waterlevel : waterlevel_storage)
    {
        waterlevelJSON["name"] = waterlevel.name;
        waterlevelJSON["state"] = (waterlevel.is_online == true) ? "On" : "Off";
        startupStorage.push_back(ConvertJsonHelper(waterlevelJSON));
    }

    JsonDocument actuatorPowerJSON;
    actuatorPowerJSON["id"] = "UpdateActuatorPower";
    JsonDocument actuatorModeJSON;
    actuatorModeJSON["id"] = "UpdateActuatorMode";
    JsonDocument actuatorPIDJSON;
    actuatorPIDJSON["id"] = "UpdatePIDInput";
    JsonDocument actuatorPIDSetpointJSON;
    actuatorPIDSetpointJSON["id"] = "UpdatePIDSetpoint";
    JsonDocument actuatorPIDActualJSON;
    actuatorPIDActualJSON["id"] = "UpdatePIDActual";

    for (auto &actuators : actuator_storage)
    {
        actuatorPowerJSON["name"] = actuators.name;
        actuatorPowerJSON["state"] = (actuators.is_online == true) ? "On" : "Off";

        actuatorModeJSON["name"] = actuators.name;
        actuatorModeJSON["state"] = (actuators.is_auto == true) ? "Auto" : "Manual";

        actuatorPIDJSON["name"] = actuators.name;
        actuatorPIDJSON["P"] = actuators.P_gain;
        actuatorPIDJSON["I"] = actuators.I_gain;
        actuatorPIDJSON["D"] = actuators.D_gain;

        actuatorPIDSetpointJSON["name"] = actuators.name;
        actuatorPIDSetpointJSON["value"] = actuators.setpoint;

        actuatorPIDActualJSON["name"] = actuators.name;
        actuatorPIDActualJSON["value"] = actuators.actual;

        startupStorage.push_back(ConvertJsonHelper(actuatorPowerJSON));
        startupStorage.push_back(ConvertJsonHelper(actuatorModeJSON));
        startupStorage.push_back(ConvertJsonHelper(actuatorPIDJSON));
        startupStorage.push_back(ConvertJsonHelper(actuatorPIDSetpointJSON));
        startupStorage.push_back(ConvertJsonHelper(actuatorPIDActualJSON));
    }
}

void ControllerConfig::UpdateActuatorHelper(JsonDocument &doc)
{
    for (auto &actuator : actuator_storage)
    {
        if (actuator.name == doc["name"])
        {
            if (doc["id"] == "UpdateActuatorPower")
            {
                actuator.is_online = (doc["state"] == "On") ? true : false;
                ;
            }
            else if (doc["id"] == "UpdateActuatorMode")
            {
                actuator.is_auto = (doc["state"] == "Auto") ? true : false;
                ;
            }
            else if (doc["id"] == "UpdatePIDInput" || doc["id"] == "LogActuatorPID")
            {
                actuator.P_gain = doc["P"].as<double>();
                actuator.I_gain = doc["I"].as<double>();
                actuator.D_gain = doc["D"].as<double>();
            }
            else if (doc["id"] == "UpdatePIDSetpoint" || doc["id"] == "LogActuatorSetpoint")
            {
                actuator.setpoint = doc["value"].as<double>();
            }
            else if (doc["id"] == "UpdatePIDActual")
            {
                actuator.actual = doc["value"].as<double>();
            }
            break;
        }
    }
}

void ControllerConfig::UpdateSystemHelper(JsonDocument &doc)
{
    if (doc["name"] == "averageTemperature")
    {
        averageTemperature = doc["value"].as<double>();
    }
    else if (doc["name"] == "averageHumidity")
    {
        averageHumidity = doc["value"].as<double>();
    }
    else if (doc["name"] == "carbonDioxideReading")
    {
        carbonDioxideReading = doc["value"].as<double>();
    }
    else if (doc["name"] == "currentAmps")
    {
        currentAmps = doc["value"].as<double>();
    }
    else if (doc["name"] == "systemStatus")
    {
        systemStatus = doc["value"].as<String>();
    }
    else if (doc["name"] == "maxCurrentValue")
    {
        maxCurrentValue = doc["value"].as<double>();
    }
}

void ControllerConfig::UpdateEnvironmentHelper(JsonDocument &doc)
{
    for (auto &sensor : sensor_storage)
    {
        if (sensor.name == doc["name"])
        {
            sensor.value = doc["value"].as<double>();
            break;
        }
    }
}

void ControllerConfig::UpdatePumpWaterHelper(JsonDocument &doc)
{
    for (auto &pump : pump_storage)
    {
        if (pump.name == doc["name"])
        {
            pump.is_online = (doc["state"] == "On") ? true : false;
        }
    }
}

void ControllerConfig::UpdateMainButtonHelper(JsonDocument &doc)
{
    if (doc["id"] == "UpdateStart")
    {
        is_started = true;
    }
    else if (doc["id"] == "UpdateStop")
    {
        is_started = false;
    }
    else if (doc["id"] == "UpdateDevMode")
    {
        devmode_enabled = (doc["state"] == "On") ? true : false;
    }
}

String ControllerConfig::ConvertJsonHelper(JsonDocument &doc)
{
    String tempstring;
    serializeJson(doc, tempstring);
    return tempstring;
}