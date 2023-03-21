#include <Arduino.h>

// For NTP
#include <time.h>

#include <WiFiClientSecure.h>

#include <HTTPUpdate.h>

class RaiseDev
{
public:
  const void updateFirmware(String current_firmware_version);
};
