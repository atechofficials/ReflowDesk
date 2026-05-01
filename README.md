# ReflowDesk

ReflowDesk is a small desktop SMD reflow soldering hot plate designed for makers, electronics hobbyists, and small workshop use. The goal is to make PCB assembly easier by providing a compact, controlled heating platform for solder paste reflow work.

The project is currently in active development. The first hardware version, **ReflowDesk AT-MK1**, is being designed and validated before any PCB or hardware production files are released.

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
- Visual status indication with addressable RGB LED support.
- System buzzer for user alerts and process notifications.
- Safety cutoff support for over-temperature conditions.
- Designed to support both AC and DC PTC heating element options.

---

## Hardware

| Hardware | Description | Status |
| --- | --- | --- |
| ReflowDesk AT-MK1 | First controller PCB and hot plate hardware design | Work in progress |

The hardware design is still being refined. PCB files, schematics, manufacturing files, and final assembly documentation are not released yet.

---

## Supported Heating Elements

| Heating Element | Intended Use | Status |
| --- | --- | --- |
| 12VDC PTC Heating Element | Low-voltage DC hot plate build | Planned support |
| 24VDC PTC Heating Element | Higher-power low-voltage DC hot plate build | Planned support |
| 220VAC PTC Heating Element | AC-powered hot plate build | Planned support |

---

## Project Structure

| Folder | Purpose |
| --- | --- |
| `hardware/` | Hardware references, datasheets, and design-related files |
| `src/` | Device firmware workspace |
| `include/` | Shared project headers and support files |
| `lib/` | Project-local libraries if needed |
| `test/` | Test and validation workspace |

---

## Development Status

ReflowDesk is not a finished release yet. The project is currently focused on:

- Finalizing the controller PCB design.
- Validating heater control options.
- Validating cooling fan control and RPM feedback.
- Refining the user interface and safety behavior.
- Preparing a reliable first hardware revision.

---

## Safety Notice

ReflowDesk can involve high temperatures, high current DC power, and, depending on the selected heater option, mains AC voltage. The project should only be built and tested by people who understand the risks of hot surfaces and electrical power electronics.

Use proper insulation, fusing, grounding, strain relief, thermal protection, and safe enclosure practices. Never leave a heating device unattended while testing.

---

## Release Plan

Hardware files for **ReflowDesk AT-MK1** will be released after the PCB design and prototype validation are complete.
