#include <Arduino.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "Only ESP32 or ESP8266 currently supported!"
#endif

#include <RaiseDev.h>

#include "wifi_credentials.h"

const char *wifi_ssid = WIFI_SSID;
const char *wifi_password = WIFI_PASSWORD;
bool was_wifi_connected_message_output = false;

// defined by -DFIRMWARE_VERSION in platformio.ini using `git describe`
const String current_firmware_version = FIRMWARE_VERSION;

RaiseDev raiseDev;

void setup()
{
  // Set the serial port's baud-rate to the same as in platformio.ini
  Serial.begin(115200);

  Serial.println(String("WiFi connecting to ") + wifi_ssid);

  // Start connection to (hardcoded) WiFi nextwork.
  WiFi.begin(wifi_ssid, wifi_password);

  raiseDev.begin();
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    // Only output this message once rather than every loop.
    if (!was_wifi_connected_message_output)
    {
      Serial.println(String("WiFi Connected to ") + wifi_ssid);
      was_wifi_connected_message_output = true;
    }
  }
  else
  {
    Serial.println(String("WiFi still connecting to ") + wifi_ssid);
    yield();
    delay(1000);
    return;
  }

  raiseDev.updateFirmware("johndbritton-s-account", current_firmware_version);
}
