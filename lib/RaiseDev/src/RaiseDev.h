#ifndef ___RAISE_DEV_H___
#define ___RAISE_DEV_H___

#include <Arduino.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <WiFiClientSecure.h>

#include <HTTPUpdate.h>

// RaiseDev - API for Raise.dev
class RaiseDev
{
public:
  const void begin();
  const void updateFirmware(String currentFirmwareVersion);

private:
  WiFiUDP ntpUDP;
  NTPClient ntpClient = NTPClient(ntpUDP);
  WiFiClientSecure wifiClientSecure;
  bool begin_method_called = false;
  unsigned long last_update_attempt_milliseconds = 0;
  t_httpUpdate_return last_update_attempt_return_code = HTTP_UPDATE_OK;
};

#endif // ___RAISE_DEV_H___
