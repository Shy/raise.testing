#include "RaiseDev.h"

const String DASHBOARD_RAISE_DEV_DOMAIN =
    "https://dashboard.raise.dev";
const char *DASHBOARD_RAISE_DEV_ROOT_CA_CERTIFICATE =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDzTCCArWgAwIBAgIQCjeHZF5ftIwiTv0b7RQMPDANBgkqhkiG9w0BAQsFADBa\n"
    "MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl\n"
    "clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw\n"
    "MDEyNzEyNDgwOFoXDTI0MTIzMTIzNTk1OVowSjELMAkGA1UEBhMCVVMxGTAXBgNV\n"
    "BAoTEENsb3VkZmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkZmxhcmUgSW5jIEVD\n"
    "QyBDQS0zMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEua1NZpkUC0bsH4HRKlAe\n"
    "nQMVLzQSfS2WuIg4m4Vfj7+7Te9hRsTJc9QkT+DuHM5ss1FxL2ruTAUJd9NyYqSb\n"
    "16OCAWgwggFkMB0GA1UdDgQWBBSlzjfq67B1DpRniLRF+tkkEIeWHzAfBgNVHSME\n"
    "GDAWgBTlnVkwgkdYzKz6CFQ2hns6tQRN8DAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0l\n"
    "BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwNAYI\n"
    "KwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j\n"
    "b20wOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL09t\n"
    "bmlyb290MjAyNS5jcmwwbQYDVR0gBGYwZDA3BglghkgBhv1sAQEwKjAoBggrBgEF\n"
    "BQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzALBglghkgBhv1sAQIw\n"
    "CAYGZ4EMAQIBMAgGBmeBDAECAjAIBgZngQwBAgMwDQYJKoZIhvcNAQELBQADggEB\n"
    "AAUkHd0bsCrrmNaF4zlNXmtXnYJX/OvoMaJXkGUFvhZEOFp3ArnPEELG4ZKk40Un\n"
    "+ABHLGioVplTVI+tnkDB0A+21w0LOEhsUCxJkAZbZB2LzEgwLt4I4ptJIsCSDBFe\n"
    "lpKU1fwg3FZs5ZKTv3ocwDfjhUkV+ivhdDkYD7fa86JXWGBPzI6UAPxGezQxPk1H\n"
    "goE6y/SJXQ7vTQ1unBuCJN0yJV0ReFEQPaA1IwQvZW+cwdFD19Ae8zFnWSfda9J1\n"
    "CZMRJCQUzym+5iPDuI9yP+kHyCREU3qzuWFloUwOxkgAyXVjBYdwRVKD05WdRerw\n"
    "6DEdfgkfCv4+3ao8XnTSrLE=\n"
    "-----END CERTIFICATE-----\n";
const unsigned long DEFAULT_UPDATE_INTERVAL_MILLISECONDS = 1000 * 15;
const unsigned long FAILED_UPDATE_INTERVAL_MILLISECONDS = 1000 * 60 * 2;

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

const void RaiseDev::begin()
{
  // Don't run begin() twice.
  if (begin_method_called)
  {
    return;
  }

  // Set SSL root certificate so we can validate HTTPS connections.
  wifiClientSecure.setCACert(DASHBOARD_RAISE_DEV_ROOT_CA_CERTIFICATE);

  // Reading data over SSL may be slow, use a longer timeout (in seconds).
  wifiClientSecure.setTimeout(10);

  // Setup HTTP Updater callback methods.
  httpUpdate.onStart(httpUpdateOnStart);
  httpUpdate.onEnd(httpUpdateOnEnd);
  httpUpdate.onProgress(httpUpdateOnProgress);
  httpUpdate.onError(httpUpdateOnError);

  // Follow correctly implemented redirects.
  httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // Setup NTP client for SSL certificate validation..
  ntpClient.begin();

  begin_method_called = true;
}

/// @brief Updates the current firmware to one from Raise.dev
/// @param account A String containing the Raise.dev account name
/// @param current_firmware_version A String to ensure that the firmware is not repeatedly updated to the same version
/// @return nothing, this method will restart on success
const void RaiseDev::updateFirmware(const String account, const String current_firmware_version)
{
  // Run begin() if consumers forgot to run it.
  if (!begin_method_called)
  {
    begin();
  }

  // Correct system time is required for SSL certificate validation
  ntpClient.update();
  if (!ntpClient.isTimeSet())
  {
    log_d("Device time not yet set from NTP...");
    return;
  }

  // Wait longer before checking for updates if the last one failed.
  unsigned long update_interval_microseconds = DEFAULT_UPDATE_INTERVAL_MILLISECONDS;
  if (last_update_attempt_return_code == HTTP_UPDATE_FAILED)
  {
    update_interval_microseconds = FAILED_UPDATE_INTERVAL_MILLISECONDS;
  }

  // Check if we've updated in the last interval and early return if so.
  // Will always check immediately on first boot.
  const unsigned long current_milliseconds = millis();
  const unsigned long waited_since_last_update_milliseconds = current_milliseconds - last_update_attempt_milliseconds;
  if (waited_since_last_update_milliseconds < update_interval_microseconds)
  {
    return;
  }
  last_update_attempt_milliseconds = current_milliseconds;

  const String updater_url = String(DASHBOARD_RAISE_DEV_DOMAIN + "/accounts/" + account + "/updater");
  log_i("Updating current firmware version %s from %s", current_firmware_version, updater_url.c_str());

  // This will update and reboot automatically on a successful, new firmware download.
  last_update_attempt_return_code = httpUpdate.update(
      wifiClientSecure, updater_url, current_firmware_version);

  switch (last_update_attempt_return_code)
  {
  case HTTP_UPDATE_FAILED:
    log_e("Firmware download error: %s (HttpUpdate code %d)",
          httpUpdate.getLastErrorString().c_str(),
          httpUpdate.getLastError());
    break;
  case HTTP_UPDATE_NO_UPDATES:
    log_i("Firmware already up-to-date");
    break;
  case HTTP_UPDATE_OK:
    // This should not happen, instead it should have already begun updating and rebooting.
    log_i("Firmware updated OK");
    break;
  }
}
