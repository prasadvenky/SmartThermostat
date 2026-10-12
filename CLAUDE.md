# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

A single Arduino sketch (`HomeTempControl/`) for an ESP8266-based smart thermostat (Feather HUZZAH ESP8266 class board). It reads room temperature from an MCP9808 I2C sensor, shows status on an I2C OLED, drives a heater relay, and syncs with Adafruit IO over WiFi. There is no build system, test suite, or linter in the repo.

## Building / flashing

Build with the Arduino IDE or `arduino-cli` (neither is configured in this repo). The sketch directory name must match the `.ino` name, so compile the `HomeTempControl/` folder as a unit. Example with arduino-cli:

```
arduino-cli compile --fqbn esp8266:esp8266:huzzah HomeTempControl
arduino-cli upload  --fqbn esp8266:esp8266:huzzah -p /dev/ttyUSB0 HomeTempControl
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200
```

Required libraries: ESP8266 board core, `Adafruit IO Arduino` (provides `AdafruitIO_WiFi.h`), `Adafruit MCP9808`, and an `OLED.h` library exposing `OLED display(sda, scl)` with `begin/clear/on/off/print(text, row[, col])` (ESP8266 OLED SSD1306 driver style).

## Architecture

- `config.h` — Adafruit IO credentials (`IO_USERNAME`, `IO_KEY`), WiFi credentials, and instantiation of the global `io` client. Transport is chosen by commenting/uncommenting the WiFi / FONA / Ethernet blocks; only WiFi is active.
- `HomeTempControl.ino` — all application logic:
  - Pins: OLED on GPIO 4 (SDA) / 5 (SCL); relay on GPIO 16 (`RELAY_PIN`).
  - Adafruit IO feeds: `heatswitch` (subscribed; sets `man_set` and drives the relay via `handleMessage`), `settemp` (subscribed; stores `Set_Temp`, clears `man_set`, shows on OLED via `handleSetTemp`), `roomtemp` (published; current °F, sent only when the integer value changes).
  - Control: when `!man_set && Set_Temp > 0`, `loop()` runs hysteresis control (±`HYSTERESIS` °F around `Set_Temp`). All relay changes go through `setHeater()`, which keeps `heater_on` in sync.
  - Don't publish to `heatswitch` from the device: Adafruit IO echoes it back to the subscription, which would trigger `handleMessage` and force manual mode.
  - `loop()` must call `io.run()` first every iteration to keep the MQTT connection alive and dispatch feed callbacks.
  - `setup()` blocks on `while(!Serial)` and halts forever if the MCP9808 isn't found.

## Known state / gotchas

- Feed callbacks use `delay(5000)` for OLED messages, which stalls `io.run()` and can cause Adafruit IO disconnects; prefer non-blocking timing (`millis()`) when changing display behavior.
- `config.h` holds real WiFi credentials in git history; don't add further secrets to it in commits.
