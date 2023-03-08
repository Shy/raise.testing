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
const String current_firmware_version = FIRMWARE_VERSION;
const String firmware_url =
    "https://raisedevs.github.io/github_pages_firmware/firmware.bin";
const char *firmware_url_root_ca_certificate =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEvjCCA6agAwIBAgIQBtjZBNVYQ0b2ii+nVCJ+xDANBgkqhkiG9w0BAQsFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
    "QTAeFw0yMTA0MTQwMDAwMDBaFw0zMTA0MTMyMzU5NTlaME8xCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxKTAnBgNVBAMTIERpZ2lDZXJ0IFRMUyBS\n"
    "U0EgU0hBMjU2IDIwMjAgQ0ExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"
    "AQEAwUuzZUdwvN1PWNvsnO3DZuUfMRNUrUpmRh8sCuxkB+Uu3Ny5CiDt3+PE0J6a\n"
    "qXodgojlEVbbHp9YwlHnLDQNLtKS4VbL8Xlfs7uHyiUDe5pSQWYQYE9XE0nw6Ddn\n"
    "g9/n00tnTCJRpt8OmRDtV1F0JuJ9x8piLhMbfyOIJVNvwTRYAIuE//i+p1hJInuW\n"
    "raKImxW8oHzf6VGo1bDtN+I2tIJLYrVJmuzHZ9bjPvXj1hJeRPG/cUJ9WIQDgLGB\n"
    "Afr5yjK7tI4nhyfFK3TUqNaX3sNk+crOU6JWvHgXjkkDKa77SU+kFbnO8lwZV21r\n"
    "eacroicgE7XQPUDTITAHk+qZ9QIDAQABo4IBgjCCAX4wEgYDVR0TAQH/BAgwBgEB\n"
    "/wIBADAdBgNVHQ4EFgQUt2ui6qiqhIx56rTaD5iyxZV2ufQwHwYDVR0jBBgwFoAU\n"
    "A95QNVbRTLtm8KPiGxvDl7I90VUwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQG\n"
    "CCsGAQUFBwMBBggrBgEFBQcDAjB2BggrBgEFBQcBAQRqMGgwJAYIKwYBBQUHMAGG\n"
    "GGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBABggrBgEFBQcwAoY0aHR0cDovL2Nh\n"
    "Y2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9vdENBLmNydDBCBgNV\n"
    "HR8EOzA5MDegNaAzhjFodHRwOi8vY3JsMy5kaWdpY2VydC5jb20vRGlnaUNlcnRH\n"
    "bG9iYWxSb290Q0EuY3JsMD0GA1UdIAQ2MDQwCwYJYIZIAYb9bAIBMAcGBWeBDAEB\n"
    "MAgGBmeBDAECATAIBgZngQwBAgIwCAYGZ4EMAQIDMA0GCSqGSIb3DQEBCwUAA4IB\n"
    "AQCAMs5eC91uWg0Kr+HWhMvAjvqFcO3aXbMM9yt1QP6FCvrzMXi3cEsaiVi6gL3z\n"
    "ax3pfs8LulicWdSQ0/1s/dCYbbdxglvPbQtaCdB73sRD2Cqk3p5BJl+7j5nL3a7h\n"
    "qG+fh/50tx8bIKuxT8b1Z11dmzzp/2n3YWzW2fP9NsarA4h20ksudYbj/NhVfSbC\n"
    "EXffPgK2fPOre3qGNm+499iTcc+G33Mw+nur7SpZyEKEOxEXGlLzyQ4UfaJbcme6\n"
    "ce1XR2bFuAJKZTRei9AqPCCcUZlM51Ke92sRKw2Sfh3oius2FkOH6ipjv3U/697E\n"
    "A7sKPPcw7+uvTPyLNhBzPvOk\n"
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
  Serial.println("HTTPUpdate: firmware download started");
}

void httpUpdateOnEnd()
{
  Serial.println("HTTPUpdate: firmware download completed");
}

void httpUpdateOnProgress(int current_bytes, int total_bytes)
{
  Serial.printf("HTTPUpdate: firmware downloading: %d of %d bytes...\n", current_bytes, total_bytes);
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
  wifi_client.setCACert(firmware_url_root_ca_certificate);

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
