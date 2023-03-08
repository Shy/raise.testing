#include <Arduino.h>
#include <WiFi.h>

// For NTP
#include <time.h>

#include <HTTPUpdate.h>

#include "wifi_credentials.h"

const String wifi_ssid = WIFI_SSID;
const String wifi_password = WIFI_PASSWORD;
bool was_wifi_connected_message_output = false;

const long gmt_offset_seconds = 0;
const int daylight_offset_seconds = 0;
const int minimum_unix_time = 8 * 3600 * 2;

// TODO: use our Rails server
const String firmware_url =
    "https://raisedevs.github.io/github_pages_firmware/firmware.bin";
const String current_firmware_version = FIRMWARE_VERSION;

const char *lets_encrypt_x3_cross_signed_root_ca_certificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
    "DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n"
    "SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n"
    "GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
    "AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n"
    "q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n"
    "SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n"
    "Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n"
    "a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n"
    "/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n"
    "AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n"
    "CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n"
    "bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n"
    "c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n"
    "VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n"
    "ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n"
    "MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n"
    "Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n"
    "AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n"
    "uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n"
    "wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n"
    "X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n"
    "PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n"
    "KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n"
    "-----END CERTIFICATE-----\n";

void setup()
{
  // Set the serial port's baud-rate to the same as in platformio.ini
  Serial.begin(115200);

  // Output some debugging information on serial port.
  Serial.println("WiFi: starting connection to " + wifi_ssid);

  // Start connection to (hardcoded) WiFi nextwork.
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}

// Nothing else can happen during a delay so yield in case something else needs done.
void yieldAndDelay(const unsigned long delay_milliseconds = 1000)
{
  yield();
  delay(delay_milliseconds);
}

// Required for SSL certificate validation
void setClockViaNTP()
{
  // Set device time to UTC.
  configTime(gmt_offset_seconds, daylight_offset_seconds, "pool.ntp.org", "time.nist.gov");

  Serial.println("setClockViaNTP: requested NTP time sync");

  time_t current_unix_time = time(nullptr);
  // Check that the system time is above a sensible floor.
  while (current_unix_time < minimum_unix_time)
  {
    yieldAndDelay();

    Serial.println("setClockViaNTP: still waiting for correct system time");
    current_unix_time = time(nullptr);
  }

  // Convert Unix time to time structure.
  struct tm current_time_info;
  gmtime_r(&current_unix_time, &current_time_info);

  // Convert time structure to a human-readable string.
  const char *current_time_string = asctime(&current_time_info);
  Serial.printf("setClockViaNTP: system time set to %s\n", current_time_string);
}

void httpUpdateOnStart()
{
  Serial.println("HTTPUpdate: started");
}

void httpUpdateOnEnd()
{
  Serial.println("HTTPUpdate: ended");
}

void httpUpdateOnProgress(int current_bytes, int total_bytes)
{
  Serial.printf("HTTPUpdate: %d of %d bytes...\n", current_bytes, total_bytes);
}

void httpUpdateOnError(int error_code)
{
  Serial.printf("HTTPUpdate: error (code %d)\n", error_code);
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!was_wifi_connected_message_output)
    {
      Serial.println("WiFi: connected to " + wifi_ssid);
      was_wifi_connected_message_output = true;

      setClockViaNTP();
    }
  }
  else
  {
    Serial.println("WiFi: still trying to connect to " + wifi_ssid);

    yieldAndDelay();

    return;
  }

  WiFiClientSecure wifi_client;

  // Set SSL root certificate so we can validate HTTPS connections
  wifi_client.setCACert(lets_encrypt_x3_cross_signed_root_ca_certificate);

  // Reading data over SSL may be slow, use a longer timeout (in seconds)
  wifi_client.setTimeout(10);

  httpUpdate.onStart(httpUpdateOnStart);
  httpUpdate.onEnd(httpUpdateOnEnd);
  httpUpdate.onProgress(httpUpdateOnProgress);
  httpUpdate.onError(httpUpdateOnError);

  Serial.printf("HTTPUpdate: attempting to update current firmware version %s from %s\n",
                current_firmware_version.c_str(),
                firmware_url.c_str());

  t_httpUpdate_return http_update_return_code =
      httpUpdate.update(wifi_client, firmware_url, current_firmware_version);

  switch (http_update_return_code)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTPUpdate: error: %s (code %d)\n",
                  httpUpdate.getLastErrorString().c_str(),
                  httpUpdate.getLastError());
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTPUpdate: no updates");
    break;
  case HTTP_UPDATE_OK:
    Serial.println("HTTPUpdate: OK");
    break;
  }
}
