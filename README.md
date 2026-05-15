<img src="hardware/images/logos/ReflowDesk_logo_1.png" alt="ReflowDesk Logo" width="140">

# ReflowDesk

<p align="center">
  <img src="hardware/images/PCB/ReflowDesk_AT-MK1/ReflowDesk_v1_PCB_5.png" alt="ReflowDesk AT-MK1 Motherboard v1 fully populated PCB render" width="900">
</p>

ReflowDesk is a small desktop SMD reflow soldering hot plate designed for makers, electronics hobbyists, and small workshop use. The goal is to make PCB assembly easier by providing a compact, controlled heating platform for solder paste reflow work.

The project is currently in active development. The first hardware revision, **ReflowDesk AT-MK1**, is available as early hardware manufacturing files for prototype validation.

Current firmware version: **v0.7.0**.

---

## What It Is

ReflowDesk is intended to sit on a workbench and provide a controlled heating surface for small PCB reflow jobs. It is designed around a simple workflow:

1. Place the PCB on the hot plate.
2. Start a reflow cycle.
3. Let the controller handle heating, reflow timing, cooldown, alerts, and safety monitoring.

---

## Features

- Compact desktop hot plate format for small PCB assembly.
- Reflow process control for preheat, soak, reflow, and cooldown stages.
- OLED display for live process status.
- Rotary encoder based user interface.
- ESP32-S3 hosted ReflowDesk Web Interface for AT-MK1 and ESP32-S3 development targets.
- WiFiManager based first-boot WiFi onboarding with password-protected setup AP.
- Local web PIN authentication for protected device controls.
- Web PIN change flow with automatic re-lock after credential updates.
- Light and dark Web Interface themes using local icon assets.
- Live REST/WebSocket sync between the OLED GUI and Web Interface.
- Browser-based reflow start, emergency stop, live telemetry, and event notifications.
- Realtime Chart.js reflow process graph for hot plate, set target, and ambient room temperatures.
- Browser-based reflow profile editor with profile renaming and editable curve graph.
- Web OTA firmware upload for PlatformIO app `firmware.bin` images.
- Web controls for reboot and factory reset with safety lockouts.
- Configurable WiFiManager setup AP name/password, with MAC-suffixed default SSID.
- Four saved solder paste reflow profiles with selectable active profile.
- Reflow profile presets for preheat, soak, reflow, and cooling behavior.
- JSON-based reflow profile provisioning through `data/profiles/profile-1.json` to `profile-4.json`.
- On-device reflow profile editing for stage temperatures, stage times, and cooling profile.
- Configurable cooldown behavior:
  - Rapid cooling
  - Normal cooling
  - Silent gradual cooling
- OLED GUI text auto-scroll for long focused labels and values.
- Settings UI layout that adapts labels and values to the available 128x64 display space.
- Configurable OLED display auto-sleep during inactivity, with rotary-encoder wake behavior.
- Configurable OLED display brightness with OLED GUI and Web Interface controls.
- Web-controlled physical input lock for rotary encoder and push-button controls.
- Optional Web Interface mode to keep the OLED display off while physical controls are locked.
- Cooling fan support with speed control and fan status monitoring.
- ReflowDesk AT-MK1 motherboard cooling fan support with independent PWM/tach monitoring.
- Ambient and motherboard NTC temperature sensing support on AT-MK1 hardware.
- Adjustable visual status indication with addressable RGB LED support and synced Status LED brightness controls from both the OLED GUI and Web Interface.
- Configurable buzzer sound level for user alerts and process notifications.
- Safety cutoff support for over-temperature and fault conditions.
- Compact serial event logging for settings changes, OLED/Web commands, reflow start/stage/abort/cooldown events, faults, OTA, OLED sleep/wake, and physical-control lock events.
- Designed to support AC and DC PTC heating element options.
- Breadboard-friendly reference pinout resources for common ESP32 boards and project modules.
- Hardware visual assets including project logos and PCB render images.

---

## Firmware Changelog

| Version | Notes |
| --- | --- |
| v0.2.1 | Initial public firmware release for ReflowDesk with ESP32 development hardware support, OLED UI, rotary encoder control, heater control, ADS1115/MAX6675 sensing, fan PWM control, tach feedback, and safety handling. |
| v0.3.0 | Added ReflowDesk AT-MK1 ESP32-S3 hardware support, 8 MB OTA partition layout, hardware-selectable pin configuration, ADS1115 ALERT/RDY pin definition, second PWM cooling fan support, dual NTC support for ambient and motherboard temperature sensing, motherboard cooling fan failsafe behavior, and GUI warning for motherboard fan failure. |
| v0.4.0 | Added four stored solder paste reflow profiles, JSON profile provisioning from LittleFS, on-device profile editing, version-3 settings migration from legacy global curve settings, smooth auto-scrolling OLED text, adaptive settings/profile editor layout, settings cursor reset on menu entry, ESP32-S3 Pico development target fixes, and 16 MB / 4 MB OTA development partition tables. |
| v0.5.4 | Added the ESP32-S3 hosted ReflowDesk Web Interface with WiFiManager onboarding, local PIN authentication, REST APIs, WebSocket telemetry/events, live Chart.js reflow graphs, web reflow controls, synchronized settings/profile edits, profile renaming, web OTA firmware upload, device reboot/factory reset controls, route persistence, hardware-originated settings change toasts, safer emergency-stop lockouts, stage-target notifications, and ambient NTC transient filtering. |
| v0.5.7 | Refined the Web Interface with PIN change/re-lock behavior, light/dark theme switching, themed sidebar and control styling, per-card settings save actions, configurable setup AP credentials, static safety action button colors, improved OTA card details, LED brightness and buzzer level sliders, and OLED WiFi setup portal IP display. Updated settings storage to version 4 for buzzer level and LED brightness behavior. |
| v0.6.0 | Added configurable OLED display auto-sleep with selectable inactivity timeouts, OLED and Web Interface settings support, encoder-rotation wake behavior, sleeping-button lockout to prevent accidental saves/actions, safe wake routing back to the Settings page from nested editors, and display-awake protection during reflow, cooldown, and fault states. Updated settings storage to version 5 for OLED sleep timeout persistence. |
| v0.7.0 | Added OLED brightness control, exposed Status LED brightness on the OLED settings UI, added Web Interface based device physical-control lock and optional OLED-off locked mode, improved the OLED locked-controls screen and unlock routing, refined Web Interface control styling, and expanded serial event logs for OLED/Web commands, settings sources, reflow/cooldown/abort events, OTA, reboot, factory reset, OLED sleep/wake, and control-lock transitions. Updated settings storage to version 6 for display/control-lock persistence. |

---

## Web Interface

Firmware v0.5.4 introduced the ReflowDesk Web Interface for ESP32-S3 targets. On first boot, the device starts a password-protected WiFiManager setup access point so router credentials can be configured. After joining the router, the web console is hosted from LittleFS at the device IP address.

The Web Interface mirrors the same firmware state used by the OLED GUI:

- Start a reflow process from the browser and monitor it on the OLED.
- Stop an active heating stage from the browser or physical input.
- Edit global settings from either UI and keep the other UI synchronized.
- Select, rename, and edit reflow profiles from the browser.
- View live process telemetry, faults, fan state/RPM, SSR state/duty, stage timing, and safety status.
- Change the Web Interface PIN and sign in again after the interface re-locks.
- Switch between local light and dark themes.
- Adjust buzzer sound level and status LED brightness.
- Adjust OLED display brightness and auto-sleep timeout.
- Lock the physical rotary encoder controls when the device is being operated from the Web Interface.
- Optionally turn the OLED display off while physical controls are locked for fully web-controlled operation.
- Change setup AP credentials used by future WiFiManager sessions.
- Upload app-only PlatformIO `firmware.bin` images through web OTA when the hot plate is idle and safe.

The web console uses local assets stored under `data/` and does not require internet access after the files are uploaded to LittleFS.

### Web Asset Versions

| Asset | Version |
| --- | --- |
| `data/index.html` | v1.2.6 |
| `data/js/app.js` | v1.2.7 |
| `data/css/style.css` | v1.1.7 |

---

## OLED Display And Control Lock

Firmware v0.6.0 added OLED display auto-sleep mode to reduce display wear and avoid leaving the screen on when the device is idle. The default timeout is configured in `src/config.h` with `REFLOW_OLED_SLEEP_DEFAULT_SECONDS`, which is exposed internally as `Timing::OLED_SLEEP_DEFAULT_SECONDS`.

The timeout can also be changed from the OLED settings menu and the Web Interface settings page. Supported timeout options are:

- 15 seconds
- 30 seconds
- 1 minute
- 2 minutes
- 5 minutes
- 10 minutes
- 30 minutes

The display may sleep from normal menus and nested settings pages after inactivity. Rotary encoder rotation wakes the OLED and resets the inactivity timer. The encoder push button is ignored while the display is asleep, preventing accidental setting saves or action starts. If sleep occurs inside an individual settings editor, unsaved draft values are discarded and the OLED returns to the main Settings page after wake.

OLED sleep is disabled during active reflow, cooldown, and fault states so process and safety information remains visible.

Firmware v0.7.0 adds OLED brightness control. OLED brightness can be adjusted from the OLED settings menu and the Web Interface settings page. The minimum user brightness is fixed at 10%, the maximum is configured in `src/config.h`, and the selectable values move in 10% steps.

The Web Interface can also lock the physical rotary encoder and push button. When this lock is active, the OLED shows a locked-controls message and the device must be unlocked from the Web Interface. A second web-only option can keep the OLED display off while the controls are locked, including during active reflow, cooldown, and fault conditions. This mode is intended for bench setups where ReflowDesk is operated only from the browser.

---

## Firmware Profiles

Firmware v0.4.0 and newer replaces the old global preheat, soak, reflow, and cooling settings with four saved reflow profile slots. Each profile contains:

- Profile name.
- Preheat temperature and time.
- Soak temperature and time.
- Reflow temperature and time.
- Cooling profile selection.

On the OLED, the active profile is selected from **Settings > Reflow Profile** and profile values can be edited from **Settings > Edit Reflow Profile**. Profile names are display-only on the device because the rotary encoder UI is not intended for text entry.

On the Web Interface, profiles can be selected from the Reflow page and edited from the Profiles page. Web profile editing also supports live profile renaming, and saved names are reflected on the OLED GUI.

### JSON Profile Provisioning

Profiles can also be provisioned at firmware build/filesystem upload time from:

```text
data/profiles/profile-1.json
data/profiles/profile-2.json
data/profiles/profile-3.json
data/profiles/profile-4.json
```

The firmware imports a JSON file only when that slot's uploaded JSON content changes. On-device edits therefore survive normal reboot and save cycles until the matching JSON file is changed and uploaded again.

Factory reset restores built-in paste presets and then imports the current LittleFS JSON profiles if available.

The included starter presets are practical defaults for testing and should be tuned against the solder paste datasheet, PCB thermal mass, and actual hot plate behavior.

---

## Hardware

| Hardware | Description | Release Contents | Status |
| --- | --- | --- | --- |
| ReflowDesk AT-MK1 Motherboard v1 | Main controller board with ESP32-S3 MCU, power supply, heater control, temperature sensing, fan control, and high-current AC/DC sections | Gerber ZIP, schematic PDF, and interactive BOM | Prototype hardware release |
| ReflowDesk AT-MK1 Daughterboard v1 | User input and feedback board with OLED display, rotary encoder, ambient NTC, reset/flash buttons, status LED, and buzzer | Gerber ZIP, schematic PDF, and interactive BOM | Prototype hardware release |

The current hardware release is intended for prototype builds, testing, and validation. KiCad source files are not included in this release.

### Manufacturing Files

| PCB | Gerber Package | Schematic |
| --- | --- | --- |
| Motherboard | `hardware/ReflowDesk_v1/gerbers/ReflowDesk_AT-MK1_v1.zip` | `hardware/ReflowDesk_v1/schematics/ReflowDesk_AT-MK1_v1_SCH.pdf` |
| Daughterboard | `hardware/ReflowDesk_Daughterboard_v1/gerbers/ReflowDesk_MK1_Daughterboard_v1.zip` | `hardware/ReflowDesk_Daughterboard_v1/schematics/ReflowDesk_AT-MK1_Daughterboard_v1_SCH.pdf` |

The Gerber packages were prepared around JLCPCB fabrication capabilities. If ordering from another PCB manufacturer, inspect the Gerber files in that manufacturer's viewer before placing an order and adjust order settings according to their process limits.

### Interactive BOM Files

| PCB | Interactive BOM |
| --- | --- |
| Motherboard | `hardware/ReflowDesk_v1/bom/ReflowDesk_AT-MK1_v1.0_ibom.html` |
| Daughterboard | `hardware/ReflowDesk_Daughterboard_v1/bom/ReflowDesk_AT-MK1_Daughterboard_v1.0_ibom.html` |

The `bom/` folders contain interactive HTML BOM files for easier component placement, soldering, and assembly progress tracking while building the ReflowDesk PCBs.

### Visual Assets

| Asset | Path |
| --- | --- |
| ReflowDesk project logo | `hardware/images/logos/ReflowDesk_logo_1.png` |
| Motherboard PCB render images | `hardware/images/PCB/ReflowDesk_AT-MK1/` |
| Daughterboard PCB render images | `hardware/images/PCB/ReflowDesk_MK1_Daughterboard/` |

### Pinout Resources

The `hardware/pinouts/` folder contains pinout diagrams for common ESP32 and ESP32-S3 development boards, plus sensor and module references used by this project. These files are useful for breadboard testing before ordering the ReflowDesk PCBs.

---

## Supported Heating Elements

| Heating Element | Intended Use | Status |
| --- | --- | --- |
| 12VDC PTC Heating Element | Low-voltage DC hot plate build | Supported by hardware design |
| 24VDC PTC Heating Element | Higher-power low-voltage DC hot plate build | Supported by hardware design |
| 220VAC PTC Heating Element | AC-powered hot plate build | Supported by hardware design |

---

## Project Structure

| Folder | Purpose |
| --- | --- |
| `hardware/` | Hardware manufacturing files, schematics, pinouts, PCB images, logos, datasheets, and design-related references |
| `docs/` | Developer, hardware, and project guides |
| `src/` | Device firmware workspace |
| `data/` | LittleFS data files, including Web Interface assets and JSON reflow profile presets |
| `include/` | Shared project headers and support files |
| `lib/` | Project-local libraries if needed |
| `test/` | Test and validation workspace |

---

## Development Status

ReflowDesk is not a finished production release yet. The project is currently focused on:

- Validating the ReflowDesk AT-MK1 Motherboard v1 PCB.
- Validating the ReflowDesk AT-MK1 Daughterboard v1 PCB.
- Testing AC and DC heater control behavior.
- Testing Hot Plate cooling fan control and RPM feedback.
- Testing ReflowDesk AT-MK1 motherboard temperature monitoring and motherboard cooling fan behavior.
- Tuning solder paste reflow profiles against real paste and PCB thermal loads.
- Refining the OLED and Web Interface user experience and safety behavior.
- Refining web-only control workflows, OLED/display behavior, and serial diagnostics.
- Preparing reliable build and validation documentation.

---

## Firmware Build Targets

| Environment | Target | Partition Table | Notes |
| --- | --- | --- | --- |
| `at-mk1` | ReflowDesk AT-MK1 Motherboard | `partitions_8mb_ota.csv` | Primary ReflowDesk hardware target. |
| `development` | ESP32 4 MB flash / no PSRAM development board | `partitions_4mb_ota.csv` | Development and breadboard testing target. |
| `development2` | ESP32-S3 16 MB flash / 2 MB PSRAM development board | `partitions_16mb_ota.csv` | Experimental development target. ReflowDesk does not require this much flash. |

The final ReflowDesk AT-MK1 v1 motherboard uses the Espressif ESP32-S3-WROOM-1U-N8R8 module. The 8 MB flash partition table is enough for the current firmware and expected near-term features.

When Web Interface assets or JSON profile files change, upload the LittleFS image as well as the firmware:

```powershell
pio run -e at-mk1 -t upload
pio run -e at-mk1 -t uploadfs
```

---

## Safety Notice

ReflowDesk can involve high temperatures, high-current DC power, and, depending on the selected heater option, mains AC voltage. The project should only be built and tested by people who understand the risks of hot surfaces and electrical power electronics.

Use proper insulation, fusing, grounding, strain relief, thermal protection, and safe enclosure practices. Never leave a heating device unattended while testing.

---

## Release Plan

ReflowDesk AT-MK1 v1 hardware files are available for prototype validation. Future releases may update the PCB manufacturing files, documentation, firmware, and assembly guidance as testing continues.
