# Firmware

Raise's ESP32 firmware for testing <https://github.com/raisedevs/raise-dev-library>.

## Building and Developing (on macOS)

We recommend building this by using VSCode and its PlatformIO extension:

```console
# Install VSCode from Homebrew Cask
$ brew install --cask visual-studio-code

# Install the PlatformIO VSCode plugin
$ code --install-extension platformio.platformio-ide
```

Alternatively, you can build from the Terminal with:

```console
# Install PlatformIO from Homebrew
$ brew install --formula platformio

# Build using PlatformIO
$ pio run
```

Regardless, you'll need to manually add a file to `include/wifi_credentials.h` with something like:

```c
#define WIFI_SSID "Your WiFi Network"
#define WIFI_PASSWORD "your-wifi-network-password"
```

## Building (on Linux)

See the [GitHub Actions `build.yml` workflow using Ubuntu](https://github.com/raisedevs/firmware/blob/main/.github/workflows/build.yml).
