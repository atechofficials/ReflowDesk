# Contributing to ReflowDesk

Thank you for your interest in contributing to ReflowDesk. This project is currently in active development, with the first hardware revision, **ReflowDesk AT-MK1**, available as prototype motherboard and daughterboard manufacturing files.

Contributions are welcome, but changes should stay focused, practical, and aligned with the current firmware and hardware direction.

---

## Project Status

ReflowDesk is a desktop SMD reflow soldering hot plate controller. The current firmware targets ESP32-based hardware and supports:

- MAX6675 thermocouple temperature sensing.
- ADS1115 based NTC temperature sensing.
- Optional ADS1115 ALERT/RDY pin assignment for future firmware features.
- OLED display user interface.
- Rotary encoder input.
- Heater control with time-windowed PID output.
- Cooling fan PWM control, 12V fan power control, and tachometer feedback.
- ReflowDesk AT-MK1 motherboard cooling fan PWM control and tachometer feedback.
- ReflowDesk AT-MK1 dual NTC behavior: ambient NTC for PID compensation and motherboard NTC for ReflowDesk enclosure cooling.
- Buzzer and status LED alerts.
- NVS-based settings storage with save-skipping for unchanged settings.
- Hardware selection and pin assignment through `src/config.h`.

The current hardware release includes Gerber packages and schematic PDFs for:

- ReflowDesk AT-MK1 Motherboard v1.
- ReflowDesk AT-MK1 Daughterboard v1.

These files are intended for prototype validation. Please avoid treating any PCB, schematic, or connector layout as production-ready unless it is explicitly marked as stable.

---

## Documentation

Developer guides, hardware notes, and other project-related guides are kept in the `docs/` folder.

Start here:

- [README](/docs/README.md)
- [Hardware Build Guide](/docs/Hardware_Build_Guide.md)

When adding larger explanations, hardware notes, build instructions, fabrication notes, or design decisions, prefer adding them under `docs/` instead of overloading the root README.

---

## Project Layout

```text
ReflowDesk/
|-- docs/                                      Developer, hardware, and project guides
|   |-- Hardware_Build_Guide.md                Hardware assembly, wiring, and validation notes
|   `-- README.md                              Documentation index and guide entry point
|-- hardware/                                  Hardware manufacturing files, schematics, pinouts, images, and references
|   |-- images/                                Project logos and PCB render/reference images
|   |   |-- logos/                             ReflowDesk logo assets
|   |   `-- PCB/                               Motherboard and daughterboard PCB images
|   |-- pinouts/                               ESP32, ESP32-S3, sensor, and module pinout diagrams
|   |-- ReflowDesk_Daughterboard_v1/           ReflowDesk AT-MK1 Daughterboard v1 hardware package
|   |   |-- bom/                               Interactive BOM for component placement and soldering tracking
|   |   |-- gerbers/                           Daughterboard Gerber package directory
|   |   |   `-- ReflowDesk_MK1_Daughterboard_v1.zip
|   |   |-- schematics/                        Daughterboard Schematics directory
|   |   |   `-- ReflowDesk_AT-MK1_Daughterboard_v1_SCH.pdf
|   |-- ReflowDesk_v1/                         ReflowDesk AT-MK1 Motherboard v1 hardware package
|   |   |-- bom/                               Interactive BOM for component placement and soldering tracking
|   |   |-- gerbers/                           Motherboard Gerber package directory
|   |   |   `-- ReflowDesk_AT-MK1_v1.zip
|   |   |-- schematics/                        Motherboard Schematics directory
|   |   |   `-- ReflowDesk_AT-MK1_v1_SCH.pdf
|   `-- README.md                              Hardware folder overview and fabrication notes
|-- include/                                   Shared headers and project support files
|-- lib/                                       Project-local libraries if needed
|-- src/                                       Main firmware source
|   |-- alerts.cpp                             Buzzer, LED, and user alert behavior
|   |-- alerts.h                               Alert controller interface
|   |-- config.h                               Hardware selection, pin maps, and device constants
|   |-- fan.cpp                                Cooling fan PWM, power, tachometer, and fault logic
|   |-- fan.h                                  Cooling fan controller interface
|   |-- heater.cpp                             Heater output control and safety shutdown behavior
|   |-- heater.h                               Heater controller interface
|   |-- input.cpp                              Rotary encoder and push-button input handling
|   |-- input.h                                Input controller interface
|   |-- main.cpp                               Firmware startup, main loop, and module wiring
|   |-- reflow.cpp                             Reflow process state machine and cooldown behavior
|   |-- reflow.h                               Reflow controller interface and process state types
|   |-- sensors.cpp                            MAX6675, ADS1115, and temperature reading logic
|   |-- sensors.h                              Sensor interface and temperature data types
|   |-- settings.cpp                           Settings storage, defaults, validation, and migration
|   |-- settings.h                             Settings data model and option definitions
|   |-- ui.cpp                                 OLED menus, process screens, and settings UI
|   `-- ui.h                                   UI controller interface
|-- test/                                      Test and validation workspace
|-- CONTRIBUTING.md                            Contribution workflow and project conventions
|-- platformio.ini                             PlatformIO build, upload, and dependency configuration
`-- README.md                                  Project introduction and public-facing overview
```

---

## Development Setup

Use PlatformIO to build the firmware.

Current main environment:

```powershell
pio run -e development
```

ReflowDesk AT-MK1 ESP32-S3 environment:

```powershell
pio run -e at-mk1
```

Upload:

```powershell
pio run -e development -t upload
```

Serial monitor:

```powershell
pio device monitor -p COM<COM_PORT_NUMBER>
```

The configured upload and monitor port may need to be changed in `platformio.ini` for your local ESP32 board.

---

## Hardware Configuration

Hardware selection is controlled in `src/config.h`.

Only one hardware target should be enabled at a time:

- `REFLOW_AT_MK1`
- `FIREBEETLE2_ESP32E`

Pin assignments must stay inside the matching hardware block in `src/config.h`. When changing pins, check the ESP32 restrictions already guarded in the file:

- Do not use GPIO6 to GPIO11. They are connected to the ESP32 flash chip.
- Avoid GPIO12 for external circuits that can affect boot or flash voltage strapping.
- Do not use GPIO34 to GPIO39 as outputs.

For ReflowDesk AT-MK1, the firmware uses ESP32-S3 pin assignments, an 8 MB OTA partition layout, a second NTC channel for motherboard temperature, and a second PWM fan channel for motherboard cooling. For FireBeetle2 ESP32-E development hardware, the firmware uses a single ADS1115 NTC channel for ambient temperature/PID compensation.

If a hardware change needs a new pin map, add it as a hardware target instead of scattering pin edits through the firmware.

---

## Hardware File Guidelines

The `hardware/` folder currently contains prototype manufacturing packages and reference PDFs. When contributing hardware-related changes:

- Keep Gerber ZIP names versioned and descriptive.
- Keep schematic PDF names matched to the corresponding PCB version.
- Keep interactive BOM files under the matching board's `bom/` folder.
- Add fabrication notes when manufacturing assumptions change.
- Mention whether a Gerber package was checked against a specific PCB manufacturer's capabilities.
- Ask users to inspect Gerbers again when ordering from a manufacturer other than the one used during design validation.
- Keep breadboard and module pinout resources under `hardware/pinouts/`.
- Keep project logos and PCB render/reference images under `hardware/images/`.

Do not assume KiCad source files are present in the repository. Hardware documentation should be useful from the released Gerber packages, schematic PDFs, and pinout references.

---

## Firmware Guidelines

- Keep heater safety behavior conservative.
- Make sure heater output is forced off during faults, aborts, invalid sensor states, and idle states.
- Keep fan failure handling intact: failed cooling must show a warning and keep the heater off.
- Keep AT-MK1 motherboard fan behavior independent from the hot-plate fan power switch. The motherboard fan is controlled by PWM and tach feedback only.
- Keep motherboard NTC failure behavior configurable through `src/config.h`.
- Use time-windowed heater control for PTC heating elements and SSR output. Do not replace this with high-frequency heater PWM unless the hardware is specifically redesigned for it.
- Keep settings validation strict. Stored settings should be clamped to safe ranges before use.
- Preserve NVS flash wear protection by avoiding unnecessary writes when settings have not changed.
- Keep OLED text short and readable on a 128x64 display.
- Keep serial debug output useful for hardware validation, especially temperature, heater state, fan power, fan duty, and RPM.

---

## Cooling Fan Behavior

The firmware currently supports cooling profiles:

| Profile | Intent |
| --- | --- |
| Rapid | Fast cooldown with high fan power until the plate is much cooler |
| Normal | Effective cooldown without being as aggressive as Rapid |
| Silent | Slower and quieter cooldown with gentler fan speeds |

When changing cooling logic, preserve these expectations:

- Fan OFF means fan PWM is 0% and fan 12V power is OFF.
- Any running fan duty should respect the fan's minimum reliable running duty.
- Fan tachometer feedback should remain part of failure detection.

---

## UI and Settings Changes

When adding a user setting:

1. Add it to `SettingsData`.
2. Provide a default value.
3. Validate the stored value.
4. Show it in the settings UI if it is user-facing.
5. Keep existing settings migration behavior in mind.
6. Avoid unnecessary NVS writes.

Settings should be understandable from the device UI without requiring a user to read the firmware.

---

## Safety Expectations

This project controls heating elements and may involve mains AC, high-current DC, and hot metal surfaces. Contributions that affect heater control, power switching, temperature sensing, or safety limits should be reviewed carefully.

Hardware-related contributions should consider:

- Fusing.
- Grounding.
- Isolation.
- Creepage and clearance.
- Thermal fuse placement.
- Wire gauge and connector current rating.
- Hot-surface protection.
- Safe default states during reset and boot.
- PCB manufacturer process limits for high-current and mains-voltage areas.

If a change reduces safety margin, it should not be merged.

---

## Contribution Checklist

Before submitting a change:

- Build the project with `pio run -e development`.
- Test relevant hardware behavior if the change touches IO, sensors, heater control, fan control, buzzer, or display behavior.
- Keep changes focused on one topic.
- Update `docs/` when the change affects setup, hardware, calibration, fabrication, or operating behavior.
- Update `hardware/README.md` when adding or replacing hardware release files.
- Update image references when adding or replacing project logos or PCB render images.
- Avoid unrelated formatting churn.
- Mention any hardware assumptions in the change notes.

---

## Reporting Issues

When reporting a problem, include:

- Hardware target selected in `src/config.h`.
- ESP32 board/module used.
- PCB version or breadboard wiring used.
- Heating element type.
- Power supply details.
- Relevant serial debug output.
- What you expected to happen.
- What actually happened.
- Whether the issue appears during boot, heating, reflow, cooldown, or idle.

For hardware issues, include a schematic snippet, clear wiring description, PCB version, or Gerber/schematic reference when possible.

---

## Pull Request Style

Good pull requests are small, testable, and easy to reason about.

Prefer:

- One feature or fix per pull request.
- Clear description of the behavior change.
- Notes about hardware tested.
- Screenshots or serial logs for UI and process-control changes.
- Schematic or fabrication notes for hardware changes.

Avoid:

- Large unrelated rewrites.
- Silent changes to safety limits.
- Pin changes outside `src/config.h`.
- New dependencies unless they clearly solve a real project need.

---

## Current Release Policy

ReflowDesk AT-MK1 hardware is in prototype validation. Hardware files and documentation may change as testing continues. Contributions should support the current firmware, motherboard, and daughterboard direction without assuming that the PCB design is production-final.
