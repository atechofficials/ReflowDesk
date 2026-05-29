# Contributing to ReflowDesk

Thank you for your interest in contributing to ReflowDesk. This project is currently in active development, with the first hardware revision, **ReflowDesk AT-MK1**, available as prototype motherboard and daughterboard manufacturing files.

Contributions are welcome, but changes should stay focused, practical, and aligned with the current firmware and hardware direction.

---

## Project Status

ReflowDesk is a desktop SMD reflow soldering hot plate controller. The current firmware targets ESP32-based hardware and supports:

- MAX6675 thermocouple temperature sensing with validity checks and spike rejection.
- ADS1115 based NTC temperature sensing with signed ADC diagnostics, ambient fallback handling, and filtered motherboard temperature readings.
- Optional ADS1115 ALERT/RDY pin assignment for future firmware features.
- OLED display user interface.
- Rotary encoder input.
- ESP32-S3 hosted ReflowDesk Web Interface for AT-MK1 and ESP32-S3 development targets.
- WiFiManager router onboarding with a password-protected setup AP.
- Local web PIN authentication, Web PIN change/re-lock behavior, REST APIs, and WebSocket telemetry/events.
- Configurable setup AP credentials for future WiFiManager sessions.
- Light and dark Web Interface themes served from local assets.
- Web OTA upload for app-only PlatformIO `firmware.bin` images.
- Heater control with time-windowed PID output for SSR-driven AC PTC heaters and 1 kHz PWM output for MOSFET-driven DC PTC heaters.
- Heater-type-specific default PID values for 220V AC PTC, 12V DC PTC, and 24V DC PTC firmware builds.
- Configurable SSR output polarity for active-low and active-high SSR modules through `src/config.h`.
- Improved heater-control behavior with staged warm-up assist, conditional integral anti-windup, per-window SSR duty latching, and duty slew limiting.
- Four saved solder paste reflow profiles stored in NVS.
- JSON-based reflow profile provisioning from LittleFS.
- On-device editing of profile stage temperatures, stage times, and cooling behavior.
- Cooling fan PWM control, 12V fan power control, and tachometer feedback.
- ReflowDesk AT-MK1 motherboard cooling fan PWM control and tachometer feedback.
- ReflowDesk AT-MK1 dual NTC behavior: ambient NTC for PID compensation and motherboard NTC for ReflowDesk enclosure cooling.
- Configurable buzzer sound level, Status LED brightness, Status LED color order, OLED brightness, and user alert behavior.
- Configurable OLED display auto-sleep with rotary-encoder wake behavior.
- Web-managed physical-controls lock for the rotary encoder and push button, with optional OLED-off web-only operation.
- NVS-based settings storage with save-skipping for unchanged settings.
- Smooth OLED auto-scroll for long focused text and adaptive settings row layout.
- OLED/Web Interface synchronization for settings, profile changes, process state, telemetry, and event toasts.
- Web settings cards with focused save actions for safety, feedback, OLED/display, device controls, PID, setup AP, and access changes.
- Structured serial event logs for settings changes, OLED/Web commands, reflow process transitions, aborts, faults, OTA, reboot, factory reset, OLED sleep/wake, and physical-control lock transitions.
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
|-- data/                                      LittleFS data files, web assets, and JSON reflow profile presets
|   |-- css/                                   Web Interface stylesheets
|   |-- fonts/                                 Locally served Web Interface fonts
|   |-- images/                                Web Interface logos and icons
|   |-- js/                                    Web Interface JavaScript and vendor libraries
|   `-- profiles/                              Four optional profile JSON files imported into NVS when changed
|-- lib/                                       Project-local libraries if needed
|-- scripts/                                   PlatformIO helper scripts for release and factory firmware images
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

ReflowDesk AT-MK1 Motherboard (ESP32-S3) environment:

```powershell
pio run -e at-mk1
```

ReflowDesk AT-MK1 Motherboard (ESP32-S3) DC PTC heater environment:

```powershell
pio run -e at-mk1-dcptc
```

Only use a DC PWM firmware build on AT-MK1 hardware with JP3 jumper shorted and JP2 jumper open.

ESP32 4 MB flash / no PSRAM development environment:
```powershell
pio run -e development
```

ESP32-S3 16 MB flash / 2 MB PSRAM development environment:

```powershell
pio run -e development2
```

ESP32-S3 DC PTC heater PWM development environment:

```powershell
pio run -e development2-dcptc
```

Only use a DC PWM firmware build on development hardware that has a MOSFET module connected.

Build LittleFS image for Web Interface assets and JSON profile files:

```powershell
pio run -e at-mk1 -t buildfs
```

Upload firmware and LittleFS assets:

```powershell
pio run -e at-mk1 -t upload
pio run -e at-mk1 -t uploadfs
```

Firmware builds also run `scripts/merge_factory_bin.py` as a PlatformIO post-build script. Successful builds place merged factory firmware images in `release_bins/` for release assets and external flashing at offset `0x0`. Browser OTA still requires the app-only `.pio/build/<environment>/firmware.bin` file, not a merged factory image.

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
- `ESP32_S3_PICO`

Pin assignments must stay inside the matching hardware block in `src/config.h`. When changing pins, check the ESP32 restrictions already guarded in the file:

- Do not use GPIO6 to GPIO11. They are connected to the ESP32 flash chip.
- Avoid GPIO12 for external circuits that can affect boot or flash voltage strapping.
- Do not use GPIO34 to GPIO39 as outputs.

For `ReflowDesk AT-MK1`, the firmware uses ESP32-S3 pin assignments, an 8 MB OTA partition layout, a second NTC channel for motherboard temperature, and a second PWM fan channel for motherboard cooling. For `FireBeetle2 ESP32-E` development hardware, the firmware uses a 4 MB OTA partition layout and a single ADS1115 NTC channel for ambient temperature/PID compensation. The `ESP32_S3_PICO` target is for ESP32-S3 development boards and does not use the AT-MK1 second NTC or second PWM fan path.

Heater-output behavior is also configured from `src/config.h`. Keep SSR polarity and heater-type options centralized there instead of hardcoding GPIO levels in `heater.cpp` or other modules.

### AT-MK1 hardware jumper configuration:

The AT-MK1 heater jumper and firmware mode must match:

- JP2 shorted and JP3 open: AC PTC heater path through the zero-cross SSR driver, with `REFLOW_HEATER_DC_PWM` undefined or set to `0`.
- JP3 shorted and JP2 open: DC PTC heater path through the EL817-isolated MOSFET driver, with `REFLOW_HEATER_DC_PWM` set to `1`.
- Never short JP2 and JP3 together. The AC and DC driver paths must not be connected to `HEATER_CTRL` at the same time.

### For SSR-controlled AC heaters:

- `REFLOW_HEATER_DC_PWM=0` should only be used when the JP2 AC PTC heater jumper is shorted/closed and JP3 jumper is open.
- `REFLOW_SSR_ACTIVE_LOW` should describe the actual SSR input module polarity.
- Active-low SSR modules must drive the heater ON when the GPIO is LOW and OFF when the GPIO is HIGH.
- Active-high SSR modules must drive the heater ON when the GPIO is HIGH and OFF when the GPIO is LOW.
- Firmware code should treat `_outputOn`, `outputOn()`, and `ssrCommandedOn()` as logical heater ON/OFF state, not as raw GPIO HIGH/LOW state.
- Diagnostic code that checks the physical GPIO level should compare against the configured `SSR_DRIVER_ON_LEVEL`/`SSR_DRIVER_OFF_LEVEL` instead of assuming one polarity.

### For MOSFET-controlled DC heaters:

- `REFLOW_HEATER_DC_PWM=1` should only be used when the JP3 DC PTC heater jumper is shorted/closed and JP2 jumper is open.
- The default DC output is 1 kHz, 10-bit LEDC PWM on `HEATER_CTRL`.
- Use the same firmware output path for 12V and 24V DC PTC heaters; tune the PID/profile for the actual heater wattage and plate thermal mass.
- Do not raise the PWM frequency, unless future hardware testing proves the EL817 optocoupler and MOSFET gate path switch cleanly and remain thermally safe.

### Temperature-sensor and status-LED:

Temperature-sensor and status-LED hardware options should also stay centralized in `src/config.h`:

- Sensor validation limits should remain configurable through the project-level sensor limit constants instead of being hardcoded into `sensors.cpp`.
- NTC divider constants must match the actual resistor values, beta value, and divider supply voltage used by the hardware.
- `RGB_LED_COLOR_ORDER` should match the addressable RGB LED package/order used by the selected board or development module, for example `NEO_GRB` or `NEO_RGB`.

### Partition tables:

| File | Intended Use |
| --- | --- |
| `partitions_8mb_ota.csv` | Primary AT-MK1 ESP32-S3-WROOM-1U-N8R8 firmware layout. |
| `partitions_4mb_ota.csv` | ESP32 4 MB flash / no PSRAM development layout. |
| `partitions_16mb_ota.csv` | ESP32-S3 16 MB flash / 2 MB PSRAM experimental development layout. |

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
- Make sure heater output is forced off during faults, aborts, invalid plate-sensor states, and idle states.
- Do not allow the heater PID loop to continue using stale plate temperature when the thermocouple read is invalid. Ambient NTC failure may use a safe fallback for compensation, but plate thermocouple failure must stop heating.
- Keep fan failure handling intact: failed cooling must show a warning and keep the heater off.
- Keep AT-MK1 motherboard fan behavior independent from the hot-plate fan power switch. The motherboard fan is controlled by PWM and tach feedback only.
- Keep motherboard NTC failure behavior configurable through `src/config.h`.
- Preserve MAX6675 thermocouple validation, plate temperature spike rejection, ADS1115 signed raw diagnostics, ambient NTC fallback behavior, and board NTC filtering unless the replacement is tested on real hardware.
- Keep sensor limit constants configurable and documented. Changes to valid temperature ranges, spike thresholds, or NTC filtering limits should include test notes.
- Use time-windowed heater control for zero-cross AC SSR output. Do not use MCU high-frequency PWM for normal AC SSR operation.
- Use DC heater PWM only when `REFLOW_HEATER_DC_PWM=1` and the hardware is jumpered for the JP3 MOSFET path.
- Keep the default DC heater PWM at 1 kHz unless the EL817 optocoupler and MOSFET gate path are validated at a different frequency.
- Keep SSR polarity configurable. Do not assume that GPIO HIGH always means heater ON or that GPIO LOW always means heater OFF.
- Keep logical heater state separate from electrical pin state. `_outputOn` should mean firmware-commanded heater ON/OFF, while GPIO HIGH/LOW should be derived from the configured SSR polarity.
- Preserve PID anti-windup behavior, staged warm-up assist, duty slew limiting, and per-window duty latching unless a replacement is tested on real heater hardware.
- Keep heater-type-specific PID defaults centralized in `src/config.h`. 220V AC PTC, 12V DC PTC, and 24V DC PTC builds may have different default PID values, while user-saved NVS PID settings should continue to override those defaults until changed or factory reset.
- Avoid aggressive forced duty near the target temperature. Warm-up assist may help far below target, but near the setpoint the PID loop should control the final approach.
- Treat target overshoot cutoff and absolute safety cutoff as separate concepts. Normal PID behavior should not repeatedly hit the emergency safety cutoff.
- Keep settings validation strict. Stored settings should be clamped to safe ranges before use.
- Preserve NVS flash wear protection by avoiding unnecessary writes when settings have not changed.
- Keep OLED text readable on a 128x64 display. Long focused labels or values may scroll, but stable rows should use available space before relying on scrolling.
- Keep OLED auto-sleep conservative and predictable. The display should not sleep during active reflow, cooldown, or fault states, and rotary encoder button presses must be ignored while the display is asleep.
- When OLED sleep occurs inside a nested settings/editor screen, discard unsaved draft values and wake back to the main Settings page instead of applying an unconfirmed edit.
- Keep OLED brightness clamped to the range defined by `Limits::OLED_BRIGHTNESS_MIN_PERCENT`, `REFLOW_OLED_BRIGHTNESS_MAX_PERCENT`, and `Limits::OLED_BRIGHTNESS_STEP_PERCENT`.
- Keep the Web Interface physical-controls lock authoritative. While locked, rotary encoder rotation and button input must not start, stop, or edit settings. When the lock is released, route the OLED UI back to the main menu instead of restoring a nested editor.
- Treat the optional OLED-off locked mode as stronger than OLED auto-sleep. If enabled, the display should remain off while physical controls are locked, including during reflow, cooldown, and fault states.
- Keep reflow profile names display-only on device unless a proper text-entry UI is added.
- When changing profile JSON schema or defaults, update the files in `data/profiles/`, settings validation, migration behavior, and README notes together.
- Preserve legacy settings migration when changing `SettingsData`; older global curve settings should continue to migrate into Profile-1 when possible.
- Keep serial debug output useful for hardware validation, especially temperature, heater state, fan power, fan duty, RPM, settings source, OLED/Web commands, process transitions, faults, OTA, OLED sleep/wake, and control-lock changes. Prefer compact `EV ...` event lines over noisy repeated prose.

---

## Heater Control Validation

Changes to `src/heater.cpp`, `src/heater.h`, heater-related settings, SSR polarity, PID behavior, or temperature limits require hardware-aware validation.

When changing heater control, test and document:

- SSR module polarity used during testing: active-low or active-high.
- Heating element type: AC PTC through SSR, low-voltage DC PTC, or another controlled load.
- Firmware heater-output mode: `AC_SSR` or `DC_PWM`.
- Physical heater jumper installed: JP2 for AC SSR or JP3 for DC MOSFET.
- Mains/heater voltage, DC supply voltage where applicable, approximate heater wattage, and approximate heater temperature capability.
- Reflow target tested, especially high-temperature behavior above 200°C.
- Whether preheat, soak, and reflow stages overshoot or undershoot their targets.
- Whether the Web Interface/OLED GUI SSR state matches the actual commanded heater state.
- Whether abort, fault, invalid sensor, and idle states force the heater output OFF.
- Whether the heater stays OFF during boot/reset until firmware intentionally enables output.

For AC zero-cross SSR testing, the expected control path is:

```text
PID output percent
  -> time-proportional SSR duty
  -> fixed SSR window
  -> configured active-low/active-high GPIO level
  -> SSR-controlled heater power
```

Do not report raw GPIO HIGH/LOW as heater ON/OFF unless the configured SSR polarity is also considered. Web UI/OLED GUI telemetry should report logical SSR/heater state.

For DC MOSFET PWM testing, the expected control path is:

```text
PID output percent
  -> 1 kHz 10-bit LEDC PWM duty
  -> HEATER_CTRL
  -> EL817 optocoupler
  -> MOSFET gate path
  -> DC PTC heater power
```

For DC heater tests, confirm 0%, 25%, 50%, and 100% duty at the optocoupler/MOSFET gate path where possible. Fault, abort, cooldown, idle, reset, and invalid-sensor states must force PWM to 0%. Also check that the optocoupler and MOSFET remain thermally reasonable during a real heating run. Record whether the existing PID defaults are acceptable for the tested heater or whether profile/PID tuning is needed for the heater wattage, supply voltage, and plate thermal mass.

If changing `Limits::REFLOW_MAX_C`, `Limits::SAFETY_MAX_C`, warm-up assist thresholds, heater-type PID defaults, or heater response checks, include test notes showing that the selected values match the real heating element and do not encourage unsafe operation beyond the heater's practical capability. Also verify that factory/default settings use the selected `HeaterTuning` PID values and that saved NVS PID settings still override compile-time defaults.

---

## Temperature Sensor Validation

Changes to `src/sensors.cpp`, `src/sensors.h`, thermocouple handling, ADS1115/NTC conversion, or sensor limit constants require reflow-aware validation.

When changing temperature-sensor behavior, preserve these expectations:

- Plate temperature comes from the MAX6675 thermocouple path and is the primary heater-control feedback signal.
- Invalid, open, no-communication, out-of-range, or repeated-spike plate readings must be treated as unsafe for heating.
- Ambient NTC readings are used for compensation and should remain stable; repeated ambient failures may use a safe fallback value, but should still be reported as not OK.
- Board/motherboard NTC readings should be filtered enough to avoid fan-control jitter while still responding to real board heating.
- ADS1115 raw NTC diagnostics should preserve signed raw values so negative/invalid readings remain visible during debugging.
- Sensor limit values such as valid plate range, spike threshold, and NTC filter rates should stay configurable through `src/config.h` or project-level sensor limit constants.
- NTC conversion math must match the actual divider orientation and resistor values used by the PCB or development wiring.

For sensor-related pull requests, include test notes showing at least one idle reading check and, when heater control is affected, one controlled heat/reflow run. Useful test notes include plate temperature progression, ambient stability, board temperature behavior, raw ADC values, thermocouple status, and whether any false sensor faults occurred.

---

## Web Interface Guidelines

The Web Interface is served locally from LittleFS and shares the same live firmware state as the OLED UI. When changing web or web-facing firmware behavior:

- Keep browser controls and OLED controls synchronized through the same settings, profile, and reflow controller paths.
- Do not add web-only settings that bypass validation or NVS save-skipping behavior.
- Keep unsafe operations locked out while the plate is active, cooling, cooldown-locked, or above safe-touch temperature. Abort/emergency stop should only be available during active heating stages.
- Keep OTA limited to app-only PlatformIO `firmware.bin` images. Merged factory images are for external flashing tools, not browser OTA.
- Keep WiFi setup AP credentials configurable through `src/config.h` and the Web Interface settings page.
- Preserve local PIN authentication for protected API and WebSocket access.
- When the Web PIN is changed, re-lock the browser session so the user must sign in with the new PIN.
- Keep physical-controls lock and OLED-off locked mode synchronized through the same settings path as the OLED GUI. Web-only operation must not bypass reflow safety, cooldown, or fault handling.
- Keep theme behavior local and deterministic. Theme-specific button, sidebar, input-focus, and slider colors should follow the documented ReflowDesk light/dark palette, while destructive/safety buttons keep their static warning colors.
- Keep all web assets local under `data/`; do not introduce CDN dependencies for runtime UI behavior.
- Update the asset version comments in `data/index.html`, `data/js/app.js`, and `data/css/style.css` when those files receive meaningful UI or behavior changes.
- Build and upload the LittleFS image when files under `data/` change.

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

Reflow profile settings are grouped under profile slots instead of being shown as global stage values. Keep global settings limited to values that are truly device-wide, such as safety limits, buzzer sound level, Status LED brightness, OLED brightness/sleep behavior, physical-control lock state, and PID tuning.

Hardware-specific values such as SSR polarity, heater type selection, board pin maps, OLED brightness maximum, setup AP defaults, and low-level heater capability limits should remain in `src/config.h` unless they are deliberately exposed as validated runtime settings.

When adding or changing feedback controls:

- Keep buzzer level in the 0 to 5 range, where 0 means muted.
- Keep LED brightness in the 0 to 100 range with 5-point increments, where 0 means the status LED is off.
- Keep addressable RGB LED color order configurable through `RGB_LED_COLOR_ORDER`; do not assume every ESP32-S3 development board uses the same NeoPixel color order.
- Keep OLED brightness in the configured 10 to 100 range with 10-point increments unless `REFLOW_OLED_BRIGHTNESS_MAX_PERCENT` changes the upper bound.
- Keep OLED numeric controls and Web Interface sliders synchronized through the same `SettingsData` fields.

When adding or changing OLED sleep behavior:

- Keep the default timeout configurable through `REFLOW_OLED_SLEEP_DEFAULT_SECONDS` in `src/config.h`.
- Keep supported user timeout options limited to 15s, 30s, 1m, 2m, 5m, 10m, and 30m unless the OLED and UX behavior are retested.
- Treat rotary encoder rotation and active button clicks as user activity while the display is awake.
- Use rotary encoder rotation only as the wake action while the display is asleep; sleeping button clicks should not mutate UI drafts, save settings, or start actions.
- Preserve OLED and Web Interface synchronization for `oledSleepTimeoutSeconds`.

When adding or changing profile fields:

- Update `ReflowProfile` and validation.
- Update factory defaults and JSON import.
- Update the profile editor UI.
- Keep the JSON files under `data/profiles/` in sync with the supported schema.
- Build both the firmware and LittleFS image when the data files change.

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
- External pull-up/pull-down requirements for SSR inputs, especially active-low SSR modules.
- External pull-up/pull-down and gate-state requirements for MOSFET inputs, especially during boot and reset.
- Whether the heater can accidentally turn ON while the MCU pin is floating, during bootloader entry, or before `pinMode()` is configured.
- PCB manufacturer process limits for high-current and mains-voltage areas.

If a change reduces safety margin, it should not be merged.

---

## Contribution Checklist

Before submitting a change:

- Build the main hardware target with `pio run -e at-mk1`.
- Build affected development targets, such as `pio run -e development` or `pio run -e development2`, when touching shared firmware or partition files.
- Build `pio run -e development2-dcptc` or `pio run -e at-mk1-dcptc` when touching heater-output code or DC PWM mode support.
- Build the filesystem image with `pio run -e at-mk1 -t buildfs` when changing files under `data/`.
- Check the generated `release_bins/` factory images when preparing a GitHub firmware release.
- Test relevant hardware behavior if the change touches IO, sensors, heater control, fan control, buzzer, or display behavior.
- For temperature-sensor changes, confirm thermocouple status, plate temperature progression, ambient/board NTC readings, signed ADC diagnostics, and absence of false sensor faults during at least one relevant hardware test.
- For heater-control changes, document SSR polarity or DC PWM mode, JP2/JP3 jumper state, heater type, target temperature tested, observed overshoot, and whether the heater output stayed OFF during idle/fault/abort states.
- For Web UI/OLED GUI telemetry changes, confirm that SSR/heater state reporting follows logical commanded state and does not confuse raw GPIO level with heater ON/OFF state.
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
- SSR module type/polarity or DC MOSFET jumper/mode, if the issue involves heater control.
- Power supply details.
- Relevant serial debug output, including plate status/raw thermocouple data and NTC ADC values when the issue involves temperature sensing.
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
