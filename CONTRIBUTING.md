# Contributing to ReflowDesk

Thank you for your interest in contributing to ReflowDesk. This project is currently in active development, with the first hardware revision, **ReflowDesk AT-MK1**, still being designed and validated.

Contributions are welcome, but changes should stay focused, practical, and aligned with the current firmware and hardware direction.

---

## Project Status

ReflowDesk is a desktop SMD reflow soldering hot plate controller. The current firmware targets ESP32-based hardware and supports:

- MAX6675 thermocouple temperature sensing.
- ADS1115 based NTC temperature sensing.
- OLED display user interface.
- Rotary encoder input.
- Heater control with time-windowed PID output.
- Cooling fan PWM control, 12V fan power control, and tachometer feedback.
- Buzzer and status LED alerts.
- NVS-based settings storage with save-skipping for unchanged settings.
- Hardware selection and pin assignment through `src/config.h`.

The hardware files are not final yet. Please avoid treating any PCB, schematic, or connector layout as release-ready unless it is explicitly marked as released.

---

## Documentation

Developer guides, hardware notes, and other project-related guides are kept in the `docs/` folder.

Start here:

- [README](/docs/README.md)
- [Hardware Build Guide](/docs/Hardware_Build_Guide.md)

When adding larger explanations, hardware notes, build instructions, or design decisions, prefer adding them under `docs/` instead of overloading the root README.

---

## Project Layout

```text
ReflowDesk/
├─ docs/                               Developer, hardware, and project guides
│  ├─ Hardware_Build_Guide.md          Hardware assembly, wiring, and validation notes
│  ├─ README.md                        Documentation index and guide entry point
├─ hardware/                           Datasheets, hardware references, and design assets
├─ include/                            Shared headers and project support files
├─ lib/                                Project-local libraries if needed
├─ src/                                Main firmware source
│  ├─ alerts.cpp                       Buzzer, LED, and user alert behavior
│  ├─ alerts.h                         Alert controller interface
│  ├─ config.h                         Hardware selection, pin maps, and device constants
│  ├─ fan.cpp                          Cooling fan PWM, power, tachometer, and fault logic
│  ├─ fan.h                            Cooling fan controller interface
│  ├─ heater.cpp                       Heater output control and safety shutdown behavior
│  ├─ heater.h                         Heater controller interface
│  ├─ input.cpp                        Rotary encoder and push-button input handling
│  ├─ input.h                          Input controller interface
│  ├─ main.cpp                         Firmware startup, main loop, and module wiring
│  ├─ reflow.cpp                       Reflow process state machine and cooldown behavior
│  ├─ reflow.h                         Reflow controller interface and process state types
│  ├─ sensors.cpp                      MAX6675, ADS1115, and temperature reading logic
│  ├─ sensors.h                        Sensor interface and temperature data types
│  ├─ settings.cpp                     Settings storage, defaults, validation, and migration
│  ├─ settings.h                       Settings data model and option definitions
│  ├─ ui.cpp                           OLED menus, process screens, and settings UI
│  └─ ui.h                             UI controller interface
├─ test/                               Test and validation workspace
├─ CONTRIBUTING.md                     Contribution workflow and project conventions
├─ platformio.ini                      PlatformIO build, upload, and dependency configuration
└─ README.md                           Project introduction and public-facing overview
```

---

## Development Setup

Use PlatformIO to build the firmware.

Current main environment:

```powershell
pio run -e development
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

- `AT_REFLOW_MK1`
- `FIREBEETLE2_ESP32E`

Pin assignments must stay inside the matching hardware block in `src/config.h`. When changing pins, check the ESP32 restrictions already guarded in the file:

- Do not use GPIO6 to GPIO11. They are connected to the ESP32 flash chip.
- Avoid GPIO12 for external circuits that can affect boot or flash voltage strapping.
- Do not use GPIO34 to GPIO39 as outputs.

If a hardware change needs a new pin map, add it as a hardware target instead of scattering pin edits through the firmware.

---

## Firmware Guidelines

- Keep heater safety behavior conservative.
- Make sure heater output is forced off during faults, aborts, invalid sensor states, and idle states.
- Keep fan failure handling intact: failed cooling must show a warning and keep the heater off.
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

If a change reduces safety margin, it should not be merged.

---

## Contribution Checklist

Before submitting a change:

- Build the project with `pio run -e development`.
- Test relevant hardware behavior if the change touches IO, sensors, heater control, fan control, buzzer, or display behavior.
- Keep changes focused on one topic.
- Update `docs/` when the change affects setup, hardware, calibration, or operating behavior.
- Avoid unrelated formatting churn.
- Mention any hardware assumptions in the change notes.

---

## Reporting Issues

When reporting a problem, include:

- Hardware target selected in `src/config.h`.
- ESP32 board/module used.
- Heating element type.
- Power supply details.
- Relevant serial debug output.
- What you expected to happen.
- What actually happened.
- Whether the issue appears during boot, heating, reflow, cooldown, or idle.

For hardware issues, include a schematic snippet or clear wiring description when possible.

---

## Pull Request Style

Good pull requests are small, testable, and easy to reason about.

Prefer:

- One feature or fix per pull request.
- Clear description of the behavior change.
- Notes about hardware tested.
- Screenshots or serial logs for UI and process-control changes.

Avoid:

- Large unrelated rewrites.
- Silent changes to safety limits.
- Pin changes outside `src/config.h`.
- New dependencies unless they clearly solve a real project need.

---

## Current Release Policy

ReflowDesk AT-MK1 hardware is work in progress. Until the first hardware release is finalized, hardware files and documentation may change frequently. Contributions should support the current development board and AT-MK1 direction without assuming that the PCB design is final.
