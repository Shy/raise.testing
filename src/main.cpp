#include <Arduino.h>
#include <WiFi.h>

#include <wifi_credentials.h>

void setup()
{
  // Set the serial port's baud-rate to the same as in platformio.ini
  Serial.begin(115200);

  const String wifi_ssid = WIFI_SSID;
  const String wifi_password = WIFI_PASSWORD;

  // Output some debugging information on serial port.
  Serial.println("WiFi: starting connection to " + wifi_ssid);

  // Start connection to (hardcoded) WiFi nextwork.
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  // Repeatedly check connection status.
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi: still trying to connect to " + wifi_ssid);

    // note: nothing else can happen during a delay.
    delay(500);
  }

  // Output that we're on WiFi.
  Serial.println("WiFi: connected to " + wifi_ssid);
}

void loop()
{
  // put your main code here, to run repeatedly:
}
