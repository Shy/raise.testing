#include <Arduino.h>
#include <WiFi.h>

#include <RaiseDev.h>

#include "wifi_credentials.h"

const char *wifi_ssid = WIFI_SSID;
const char *wifi_password = WIFI_PASSWORD;
bool was_wifi_connected_message_output = false;

// defined by -DFIRMWARE_VERSION in platformio.ini using `git describe`
const String current_firmware_version = FIRMWARE_VERSION;

void setup()
{
  // Set the serial port's baud-rate to the same as in platformio.ini
  Serial.begin(115200);

  log_i("WiFi connecting to %s", wifi_ssid);

  // Start connection to (hardcoded) WiFi nextwork.
  WiFi.begin(wifi_ssid, wifi_password);
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    // Only output this message once rather than every loop.
    if (!was_wifi_connected_message_output)
    {
      log_i("WiFi Connected to %s", wifi_ssid);
      was_wifi_connected_message_output = true;
    }
  }
  else
  {
    log_d("WiFi still connecting to %s", wifi_ssid);
    yield();
    delay(1000);
    return;
  }

  RaiseDev raise_dev;
  raise_dev.updateFirmware(current_firmware_version);
}
