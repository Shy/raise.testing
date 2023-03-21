#include "RaiseDev.h"

// TODO: use our Rails server
const String FIRMWARE_URL =
    "https://raisedevs.github.io/github_pages_firmware/firmware.bin";
const char *FIRMWARE_URL_ROOT_CA_CERTIFICATE =
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
const long GMT_OFFSET_SECONDS = 0;
const int DAYLIGHT_OFFSET_SECONDS = 0;
const char *NTP_SERVER_PRIMARY = "pool.ntp.org";
const char *NTP_SERVER_SECONDARY = "time.nist.gov";

// Callback when HTTPUpdate starts
const void httpUpdateOnStart()
{
  log_i("Firmware download started");
}

// Callback when HTTPUpdate ends
const void httpUpdateOnEnd()
{
  log_i("Firmware download completed");
}

// Callback when HTTPUpdate progresses
const void httpUpdateOnProgress(const int current_bytes, const int total_bytes)
{
  log_d("Firmware downloading: (%d/%d bytes)...", current_bytes, total_bytes);
}

// Callback when HTTPUpdate has an error
const void httpUpdateOnError(const int error_code)
{
  log_e("Firmware download error (HttpUpdate code %d)", error_code);
}

// Set the system time using NTP
// Necessary to perform SSL validation
const void setClockViaNTP()
{
  // Set device time to UTC.
  configTime(GMT_OFFSET_SECONDS, DAYLIGHT_OFFSET_SECONDS, NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY);

  log_i("NTP time sync requested");

  time_t current_unix_time = time(nullptr);

  // Check that the system time is above a sensible floor.
  const int minimum_unix_time = 8 * 3600 * 2;
  while (current_unix_time < minimum_unix_time)
  {
    yield();
    delay(1000);

    log_d("Still waiting for updated system time");
    current_unix_time = time(nullptr);
  }

  // Convert Unix time to time structure.
  struct tm current_time_info;
  gmtime_r(&current_unix_time, &current_time_info);

  // Convert time structure to a human-readable string.
  const char *current_time_string = asctime(&current_time_info);
  log_i("System time set to %s", current_time_string);
}

/// @brief Updates the current firmware to one from Raise.dev
/// @param current_firmware_version A String to ensure that the firmware is not repeatedly updated to the same version
/// @return nothing, this method will restart on success
const void RaiseDev::updateFirmware(const String current_firmware_version)
{
  // Required for SSL certificate validation
  setClockViaNTP();

  WiFiClientSecure wifi_client;

  // Set SSL root certificate so we can validate HTTPS connections
  wifi_client.setCACert(FIRMWARE_URL_ROOT_CA_CERTIFICATE);

  // Reading data over SSL may be slow, use a longer timeout (in seconds)
  wifi_client.setTimeout(10);

  httpUpdate.onStart(httpUpdateOnStart);
  httpUpdate.onEnd(httpUpdateOnEnd);
  httpUpdate.onProgress(httpUpdateOnProgress);
  httpUpdate.onError(httpUpdateOnError);

  log_i("Updating current firmware version %s from %s",
        current_firmware_version,
        FIRMWARE_URL.c_str());

  // This will update and reboot automatically on a successful, new firmware download.
  const t_httpUpdate_return http_update_return_code =
      httpUpdate.update(wifi_client, FIRMWARE_URL, current_firmware_version);

  switch (http_update_return_code)
  {
  case HTTP_UPDATE_FAILED:
    log_e("Firmware download error: %s (HttpUpdate code %d)",
          httpUpdate.getLastErrorString(),
          httpUpdate.getLastError());
  case HTTP_UPDATE_NO_UPDATES:
    log_i("Firmware already up-to-date");
  case HTTP_UPDATE_OK:
    // This should not happen, instead it should have already begun updating and rebooting.
    log_i("Firmware updated OK");
  }
}
