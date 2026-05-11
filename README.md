<img src="hardware/images/logos/ReflowDesk_logo_1.png" alt="ReflowDesk Logo" width="140">

# ReflowDesk

<p align="center">
  <img src="hardware/images/PCB/ReflowDesk_AT-MK1/ReflowDesk_v1_PCB_5.png" alt="ReflowDesk AT-MK1 Motherboard v1 fully populated PCB render" width="900">
</p>

ReflowDesk is a small desktop SMD reflow soldering hot plate designed for makers, electronics hobbyists, and small workshop use. The goal is to make PCB assembly easier by providing a compact, controlled heating platform for solder paste reflow work.

The project is currently in active development. The first hardware revision, **ReflowDesk AT-MK1**, is available as early hardware manufacturing files for prototype validation.

Current firmware version: **v0.3.0**.

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
- Adjustable reflow profile settings.
- Configurable cooldown behavior:
  - Rapid cooling
  - Normal cooling
  - Silent gradual cooling
- Cooling fan support with speed control and fan status monitoring.
- ReflowDesk AT-MK1 motherboard cooling fan support with independent PWM/tach monitoring.
- Ambient and motherboard NTC temperature sensing support on AT-MK1 hardware.
- Visual status indication with addressable RGB LED support.
- System buzzer for user alerts and process notifications.
- Safety cutoff support for over-temperature and fault conditions.
- Designed to support AC and DC PTC heating element options.
- Breadboard-friendly reference pinout resources for common ESP32 boards and project modules.
- Hardware visual assets including project logos and PCB render images.

---

## Firmware Changelog

| Version | Notes |
| --- | --- |
| v0.2.0 | Initial public firmware release for ReflowDesk with ESP32 development hardware support, OLED UI, rotary encoder control, heater control, ADS1115/MAX6675 sensing, fan PWM control, tach feedback, and safety handling. |
| v0.3.0 | Added ReflowDesk AT-MK1 ESP32-S3 hardware support, 8 MB OTA partition layout, hardware-selectable pin configuration, ADS1115 ALERT/RDY pin definition, second PWM cooling fan support, dual NTC support for ambient and motherboard temperature sensing, motherboard cooling fan failsafe behavior, and GUI warning for motherboard fan failure. |

---

## Hardware

| Hardware | Description | Release Contents | Status |
| --- | --- | --- | --- |
| ReflowDesk AT-MK1 Motherboard v1 | Main controller board with ESP32-S3, power supply, heater control, fan control, motherboard NTC sensing, and high-current AC/DC sections | Gerber ZIP, schematic PDF, and interactive BOM | Prototype hardware release |
| ReflowDesk AT-MK1 Daughterboard v1 | User interface and feedback board with OLED, rotary encoder, ambient NTC, reset/flash buttons, status LED, and buzzer | Gerber ZIP, schematic PDF, and interactive BOM | Prototype hardware release |

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
- Refining the user interface and safety behavior.
- Preparing reliable build and validation documentation.

---

## Safety Notice

ReflowDesk can involve high temperatures, high-current DC power, and, depending on the selected heater option, mains AC voltage. The project should only be built and tested by people who understand the risks of hot surfaces and electrical power electronics.

Use proper insulation, fusing, grounding, strain relief, thermal protection, and safe enclosure practices. Never leave a heating device unattended while testing.

---

## Release Plan

ReflowDesk AT-MK1 v1 hardware files are available for prototype validation. Future releases may update the PCB manufacturing files, documentation, firmware, and assembly guidance as testing continues.
