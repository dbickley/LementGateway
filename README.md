# LementGateway

LementGateway is a clean, modular ESP32-S3 sensor node designed for automotive and environmental instrumentation. The firmware reads proximity, environmental, engine, radio, and vehicle data while keeping the project organized by function rather than by a single monolithic sensor file.

## Overview

This project is built around an ESP32-S3 DevKit-C compatible board running Arduino via PlatformIO. The system currently includes:

- Primary RCWL-1655 distance sensor
- Five backup RCWL-1655 sensors
- APDS9960 proximity / ambient light sensor
- BME280 environmental sensor
- SHT20 temperature / humidity sensor
- DS18B20 engine compartment temperature sensor
- RDA5807M FM radio tuner
- OBD2 interface for vehicle data
- Data-out serial interface for downstream telemetry
- WiFi provisioning + OTA update support
- State/event publishing for listeners that only receive changed values

The current architecture intentionally keeps the “sensor core” clean and avoids display/LED logic, so the firmware stays focused on data acquisition and telemetry.

## Project goals

- Keep hardware mapping centralized and explicit
- Separate drivers by sensor or interface category
- Publish only meaningful state changes instead of dumping full sensor state on every loop
- Maintain a clean base for future expansion without coupling unrelated logic
- Keep network/OTA paths stable while leaving room for future hard-wired or BLE transports

## Board and firmware configuration

- Board: ESP32-S3-N16R8 / ESP32-S3 DevKit-C compatible
- Flash: 16 MB
- Framework: Arduino / PlatformIO
- Build target: esp32s3_n16r8
- Flash configuration: 16 MB app flash with qio mode and custom partitioning
- Core: Arduino ESP32 2.0.14

Relevant project configuration is in [platformio.ini](platformio.ini), and the central hardware pin map is in [src/sensor_config.h](src/sensor_config.h).

## File layout

The codebase is organized around sensor and interface modules rather than a single giant file.

- [src/main.cpp](src/main.cpp): setup/loop orchestration and sensor polling cadence
- [src/sensor_config.h](src/sensor_config.h): pin definitions and shared board constants
- [src/sensor_data.h](src/sensor_data.h): aggregated sensor state model
- [src/state_event.h](src/state_event.h): listener interface and state bus contract
- [src/state_event.cpp](src/state_event.cpp): change detection and JSON serialization
- [src/networkUtils.h](src/networkUtils.h) / [src/networkUtils.cpp](src/networkUtils.cpp): WiFi, OTA, captive portal, and provisioning logic
- [src/rda5807m_sensor.h](src/rda5807m_sensor.h) / [src/rda5807m_sensor.cpp](src/rda5807m_sensor.cpp): FM tuner control and status reads
- [src/rcwl_sensor.h](src/rcwl_sensor.h) and associated modules: primary + backup RCWL logic
- [src/bme280_sensor.h](src/bme280_sensor.h): BME280 driver wrapper
- [src/apds_sensor.h](src/apds_sensor.h): APDS9960 driver wrapper
- [src/ds18b20_sensor.h](src/ds18b20_sensor.h): DS18B20 engine temp interface
- [src/sht20_sensor.h](src/sht20_sensor.h): SHT20 humidity/temperature interface
- [src/obd2_interface.h](src/obd2_interface.h): OBD2 decode and polling layer
- [src/data_out.h](src/data_out.h): downstream serialized data output

## Runtime behavior

### Sensor polling model

The main loop polls devices on different intervals to keep sampling realistic and avoid unnecessary load:

- APDS9960: every 100 ms
- RCWL primary sensor: every 500 ms
- BME280: every 5000 ms
- SHT20: every 5000 ms
- RDA5807M radio: every 2000 ms
- DS18B20: every 5000 ms
- Backup RCWL sensors: every 1000 ms (approximately twice the RCWL interval)

This keeps fast motion sensing responsive without overloading the I2C/UART bus.

### State publishing

The project includes a listener-based state bus. Any listener can register to receive state events, and the bus only emits when a meaningful change is detected.

Current state groups include:

- APDS data
- BME280 data
- SHT20 data
- Radio data
- Engine temp
- RCWL primary and backup sensor data
- OBD2 data
- dataOut status

The serialization is implemented in [src/state_event.cpp](src/state_event.cpp). It compares previous vs current state and publishes only on changed values.

## Network and OTA details

### WiFi provisioning

The board starts in a provisioning flow if no saved WiFi credentials exist.

- Access point name: LementGateway_Setup
- Captive portal IP: 192.168.4.1
- DNS server intercepts unknown requests and redirects to the setup page
- Credentials are stored in the Preferences namespace named gnome
- Saved values include:
  - ssid
  - pass
  - server_ip
  - server_port

### OTA updates

OTA is configured with:

- hostname: LementGateway-S3
- password: lementgateway
- ArduinoOTA enabled in setup

The OTA handler is invoked in the main loop via handleOTA().

### Transport status

The project currently keeps WiFi + OTA as the reliable transport path. Classic Bluetooth is intentionally not enabled for the ESP32-S3 target in the current Arduino core because the required Classic BT symbols do not link correctly on this board configuration.

## GPIO map and wiring

### All configured GPIO usage

| GPIO | Destination | Description | Status |
| --- | --- | --- | --- |
| GPIO 0 | Boot strap / ROM | Strapping pin (kept free to avoid bootloader entry) | Reserved / System |
| GPIO 1 | RCWL primary echo | Echo signal for primary distance sensor | Assigned |
| GPIO 2 | Unused | General purpose I/O | Spare |
| GPIO 3 | Backup RCWL 1 echo | Echo for backup sensor #1 | Assigned |
| GPIO 4 | Backup RCWL 1 trigger | Trigger for backup sensor #1 | Assigned |
| GPIO 5 | Backup RCWL 2 trigger | Trigger for backup sensor #2 | Assigned |
| GPIO 6 | I2C SDA | Shared bus for BME280, APDS9960, SHT20, and RDA5807M | Assigned |
| GPIO 7 | I2C SCL | Shared bus clock line | Assigned |
| GPIO 8 | Backup RCWL 4 trigger | Trigger for backup sensor #4 | Assigned |
| GPIO 9 | Backup RCWL 5 trigger | Trigger for backup sensor #5 | Assigned |
| GPIO 10 | Backup RCWL 2 echo | Echo for backup sensor #2 | Assigned |
| GPIO 11 | Backup RCWL 3 echo | Echo for backup sensor #3 | Assigned |
| GPIO 12 | Backup RCWL 4 echo | Echo for backup sensor #4 | Assigned |
| GPIO 13 | Backup RCWL 5 echo | Echo for backup sensor #5 | Assigned |
| GPIO 14 | OBD2 RX | UART receive for vehicle data | Assigned |
| GPIO 15 | OBD2 TX | UART transmit for vehicle data | Assigned |
| GPIO 16 | DS18B20 data | Engine compartment temperature sensor (1-Wire) | Assigned |
| GPIO 17 | OEM amplifier enable | Enables/disables amplifier stage for radio | Assigned |
| GPIO 18 | Backup RCWL 3 trigger | Trigger for backup sensor #3 | Assigned |
| GPIO 19 | USB D- | Native USB CDC / JTAG (kept clear of external I/O) | System / USB |
| GPIO 20 | USB D+ | Native USB CDC / JTAG (kept clear of external I/O) | System / USB |
| GPIO 21 | Data out TX | UART transmit for downstream output | Assigned |
| GPIO 22–25 | N/A | Not physically present on ESP32-S3 silicon | Non-existent |
| GPIO 26–32 | SPI Flash | Internal SPI Flash interface | Internal Bus |
| GPIO 33–37 | Octal PSRAM | Internal OPI PSRAM interface (N16R8) | Internal Bus |
| GPIO 38 | Data out RX | UART receive for downstream data output | Assigned |
| GPIO 39 | Unused | General purpose I/O | Spare |
| GPIO 40 | Unused | General purpose I/O | Spare |
| GPIO 41 | Unused | General purpose I/O | Spare |
| GPIO 42 | Unused | General purpose I/O | Spare |
| GPIO 43 | UART0 TX | Default Serial monitor / debug transmit | System / Serial |
| GPIO 44 | UART0 RX | Default Serial monitor / debug receive | System / Serial |
| GPIO 45 | Strapping | VDD_SPI strapping pin | System |
| GPIO 46 | Strapping | ROM messages boot strap | System |
| GPIO 47 | RCWL primary trigger | Trigger pin for primary distance sensor | Assigned |
| GPIO 48 | Unused | Spare / on-board RGB LED pad | Spare |

### Functional grouping

#### I2C

| Device | Function | Pins |
| --- | --- | --- |
| BME280 | Temperature, pressure, humidity | GPIO 6 (SDA), GPIO 7 (SCL) |
| APDS9960 | Ambient light, proximity, gestures | GPIO 6 (SDA), GPIO 7 (SCL) |
| SHT20 | Humidity and temperature | GPIO 6 (SDA), GPIO 7 (SCL) |
| RDA5807M | FM tuner | GPIO 6 (SDA), GPIO 7 (SCL) |

#### Primary RCWL-1655

| Signal | GPIO |
| --- | --- |
| Trigger | GPIO 47 |
| Echo | GPIO 1 |

#### Backup RCWL sensors (5 total)

| Sensor | Trigger | Echo |
| --- | --- | --- |
| Backup 1 | GPIO 4 | GPIO 3 |
| Backup 2 | GPIO 5 | GPIO 10 |
| Backup 3 | GPIO 18 | GPIO 11 |
| Backup 4 | GPIO 8 | GPIO 12 |
| Backup 5 | GPIO 9 | GPIO 13 |

#### DS18B20 engine compartment temp

| Signal | GPIO |
| --- | --- |
| Data | GPIO 16 |
| Ground | GND |
| VCC | 3.3V or 5V depending on sensor variant and pull-up configuration |

A 4.7k pull-up resistor is typically required in a standard one-wire setup.

#### RDA5807M FM radio

| Signal | GPIO / wiring |
| --- | --- |
| SDA | GPIO 6 |
| SCL | GPIO 7 |
| OEM amplifier enable | GPIO 17 |
| Audio output | Analog stereo feed to OEM amplifier input |
| VCC | 3.3V |
| GND | GND |

The radio is designed to be treated as a normal stereo source feeding the OEM amplifier, without custom front/rear/sub control logic.

#### OBD2 and data out

| Interface | RX | TX |
| --- | --- | --- |
| OBD2 | GPIO 14 | GPIO 15 |
| Data out | GPIO 38 | GPIO 21 |

## Electrical notes and wiring guidance

- **ESP32-S3 Native USB (GPIO 19/20)**: Maintained exclusively for USB CDC and programming to prevent device crashes or serial loss.
- **Boot Strapping (GPIO 0, 45, 46)**: Kept clear of active pull-downs to avoid accidentally dropping the device into ROM bootloader mode.
- **Internal Flash & PSRAM (GPIO 26–37)**: Dedicated to the onboard 16MB Flash and 8MB Octal PSRAM on the N16R8 board; never routed externally.
- **RCWL echo/trigger wiring**: Kept on separate, verified digital GPIOs to minimize interference. The echo timeout is capped at 15 ms (~2.5 m range) to eliminate main loop stalling.
- **DS18B20 Non-blocking Reads**: Operates asynchronously with `setWaitForConversion(false)`, eliminating the 750 ms synchronous delay in the main loop.
- **The shared I2C bus**: SHT20, BME280, APDS9960, and RDA5807M all operate at 3.3V logic on GPIO 6 and 7.
- **The OEM amplifier enable output**: Active on GPIO 17 to control the OEM amplifier stage before/after radio initialization safely.

## Sensor data model

The central state model is defined in [src/sensor_data.h](src/sensor_data.h). It aggregates these structures:

- APDS_DATA
- BME280_DATA
- SHT20_DATA
- RDA5807M_DATA
- DS18B20_DATA
- RCWL_DATA
- backupSensors[]
- OBD2_INTERFACE_DATA
- DATA_OUT_DATA
- SENSOR_DATA

This is the canonical runtime data structure used for state comparisons and event publishing.

## State and event model

The event bus, defined in [src/state_event.h](src/state_event.h), allows modules to register listeners that receive state updates when the state actually changes.

Key ideas:

- State is serialized to JSON
- Listeners are registered by pointer
- Each event contains an event name and a state payload
- The payload is only emitted when a meaningful value changes

This reduces message spam and keeps downstream listeners focused on real changes.

## Power and startup expectations

On boot, the firmware does the following:

1. Starts serial output
2. Initializes I2C on GPIO 6 and GPIO 7
3. Initializes all sensor drivers
4. Initializes WiFi provisioning / connection logic
5. Enables OTA
6. Sets time from the network or saved source
7. Begins the main loop with sensor polling and state publication

The startup path is intentionally arranged so the radio and amplifier logic are initialized in a safe order rather than immediately driving the OEM amplifier output blindly.

## Build and flash

From the project root:

```bash
source .venv/bin/activate
pio run
pio run -t upload
```

For serial monitoring:

```bash
pio device monitor
```

PlatformIO project settings are in [platformio.ini](platformio.ini).

## Known caveats and notes

- The current build intentionally avoids Classic Bluetooth on the ESP32-S3 target because the ESP32-S3 Arduino stack does not provide the required Classic BT symbols in the current configuration.
- The project is focused on WiFi + OTA + local sensor telemetry rather than an onboard display UI.
- Ultrasonic sensors use a 15 ms timeout (~2.5 m range) to keep loop execution fast and deterministic.
- DS18B20 engine temperature reads are non-blocking (`waitForConversion = false`) to prevent 750 ms loop latency.
- State events use deadband filtering to prevent electrical sensor noise from triggering redundant event broadcasts.

## Current status

The project is in a clean, modular state with the core active systems working together:

- RCWL sensors active
- Backup sensor array defined
- I2C sensor stack active
- DS18B20 engine temp active
- OBD2 and data-out interfaces prepared
- FM radio integrated with a trusted library
- OTA and provisioning enabled
- State-driven event emission active

## Future expansion ideas

- Add a dedicated BLE transport layer once supported by the target stack
- Add a hard-wired serial or CAN car bus interface
- Split backup sensor handling into its own module if desired
- Add an explicit watchdog / device health monitor
- Add a richer runtime telemetry packet format for the data-out interface

## Summary

LementGateway is a modular automotive sensor node designed to be easy to wire, easy to extend, and easy to reason about. The system keeps the device core clean, keeps pin assignments centralized, and publishes state only when it actually changes.
