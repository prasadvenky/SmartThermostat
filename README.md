# Smart Thermostat

An ESP8266 WiFi thermostat that reads room temperature, switches a heater relay, and is controlled from an [Adafruit IO](https://io.adafruit.com) dashboard.

## Hardware

| Part | Connection |
|------|------------|
| ESP8266 board (e.g. Adafruit Feather HUZZAH ESP8266) | — |
| MCP9808 I2C temperature sensor | I2C (SDA GPIO 4, SCL GPIO 5) |
| SSD1306-style I2C OLED display | SDA GPIO 4, SCL GPIO 5 |
| Relay module for the heater | GPIO 16 |

## Adafruit IO feeds

Create these feeds in your Adafruit IO account:

| Feed | Direction | Purpose |
|------|-----------|---------|
| `settemp` | dashboard → device | Target temperature (°F). Receiving a value puts the thermostat in automatic mode. |
| `heatswitch` | dashboard → device | Manual heater ON/OFF. Overrides automatic control until a new `settemp` value is sent. |
| `roomtemp` | device → dashboard | Current room temperature (°F), published whenever the whole-degree value changes. |

## How it works

- In automatic mode the heater turns on when the room is at or below `settemp − 1 °F` and turns off at or above `settemp + 1 °F`. The 1 °F band (`HYSTERESIS`) stops the relay from switching on and off rapidly.
- The thermostat does nothing until it has received a `settemp` value.
- Sending a value on `heatswitch` switches to manual mode; the heater follows the switch until a new `settemp` value is sent.
- The OLED briefly shows heater state, set temperature and current temperature when feed messages arrive.

## Setup

1. Install the ESP8266 board core and these libraries in the Arduino IDE (or `arduino-cli`):
   - Adafruit IO Arduino
   - Adafruit MCP9808
   - An `OLED.h` SSD1306 library providing `OLED display(sda, scl)`
2. Edit `HomeTempControl/config.h` and fill in `IO_USERNAME`, `IO_KEY`, `WIFI_SSID` and `WIFI_PASS`.
3. Open `HomeTempControl/HomeTempControl.ino`, select your board and port, and upload.
4. Open the serial monitor at 115200 baud to watch connection status and temperature readings.

> The sketch waits for a serial connection at startup (`while(!Serial)`) and halts if the MCP9808 is not detected.

## Credits

Based on Adafruit IO example code by Todd Treece (Adafruit Industries, MIT license). Modified by Venkatesh Prasad for EcoBots.Co.
