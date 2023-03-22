#ifndef RaiseDev_h
#define RaiseDev_h

#include <Arduino.h>

// For NTP
#include <time.h>

#include <WiFiClientSecure.h>

#include <HTTPUpdate.h>

// RaiseDev - API for Raise.dev
class RaiseDev
{
public:
  const void updateFirmware(String current_firmware_version);
};

#endif
