# ReflowDesk Hardware

This directory contains ReflowDesk hardware manufacturing files, schematic PDFs, pinout references, PCB images, project logos, datasheets, and related hardware documentation.

The current hardware files are intended for prototype builds and validation of the **ReflowDesk AT-MK1** motherboard and daughterboard.

---

## Contents

```text
ReflowDesk/hardware/
|-- images/                                 Project logos and PCB render/reference images
|   |-- logos/                              ReflowDesk logo assets
|   |   |-- ReflowDesk_logo_1.png
|   |   |-- ReflowDesk_logo_2.png
|   |   `-- ReflowDesk_Github_QR.png
|   `-- PCB/                                PCB images grouped by board
|       |-- ReflowDesk_AT-MK1/              Motherboard v1 PCB renders/images
|       `-- ReflowDesk_MK1_Daughterboard/   Daughterboard v1 PCB renders/images
|-- pinouts/                                Pinout diagrams for ESP32 boards, ESP32-S3 boards, and project modules
|-- ReflowDesk_Daughterboard_v1/            ReflowDesk AT-MK1 Daughterboard v1 manufacturing and schematic files
|   |-- gerbers/
|   |   `-- ReflowDesk_MK1_Daughterboard_v1.zip
|   `-- schematics/
|       `-- ReflowDesk_AT-MK1_Daughterboard_v1_SCH.pdf
|-- ReflowDesk_v1/                          ReflowDesk AT-MK1 Motherboard v1 manufacturing and schematic files
|   |-- gerbers/
|   |   `-- ReflowDesk_AT-MK1_v1.zip
|   `-- schematics/
|       `-- ReflowDesk_AT-MK1_v1_SCH.pdf
`-- README.md                               Documentation for the ReflowDesk hardware directory
```

---

## Hardware Packages

| PCB | Purpose | Gerber Package | Schematic PDF |
| --- | --- | --- | --- |
| ReflowDesk AT-MK1 Motherboard v1 | Main controller board with MCU, power supply, heater control, fan control, and high-current AC/DC sections | `ReflowDesk_v1/gerbers/ReflowDesk_AT-MK1_v1.zip` | `ReflowDesk_v1/schematics/ReflowDesk_AT-MK1_v1_SCH.pdf` |
| ReflowDesk AT-MK1 Daughterboard v1 | User input and feedback board with OLED, rotary encoder, room-temperature NTC, reset/flash buttons, status LED, and buzzer | `ReflowDesk_Daughterboard_v1/gerbers/ReflowDesk_MK1_Daughterboard_v1.zip` | `ReflowDesk_Daughterboard_v1/schematics/ReflowDesk_AT-MK1_Daughterboard_v1_SCH.pdf` |

The ZIP files are intended to be uploaded directly to a PCB fabrication service. The schematic PDFs are included for review, wiring reference, and hardware validation.

---

## Images and Visual References

| Folder | Contents |
| --- | --- |
| `images/logos/` | ReflowDesk project logos and repository QR image |
| `images/PCB/ReflowDesk_AT-MK1/` | Motherboard v1 PCB render/reference images |
| `images/PCB/ReflowDesk_MK1_Daughterboard/` | Daughterboard v1 PCB render/reference images |

These images are included for project presentation, documentation, and visual hardware reference. They are not a substitute for inspecting the Gerber files before fabrication.

---

## Fabrication Notes

The current Gerber packages were prepared around JLCPCB fabrication capabilities. Before ordering PCBs from another manufacturer, check the Gerbers in that manufacturer's online viewer or a local Gerber viewer and confirm that their process supports the design rules used by these boards.

Recommended checks before ordering:

- Board outline and dimensions.
- Copper layers and copper pours.
- Drill files and plated holes.
- Solder mask openings.
- Silkscreen readability.
- Minimum trace width and clearance.
- Isolation around mains AC areas.
- Slot, cutout, and edge-clearance handling if applicable.

---

## Pinout Resources

The `pinouts/` folder contains diagrams for common ESP32 and ESP32-S3 development boards, plus sensor and module pinouts used by ReflowDesk. These references are useful when building a breadboard prototype before ordering the AT-MK1 PCBs.

---

## Hardware Status

ReflowDesk AT-MK1 v1 hardware is a prototype release. The PCB manufacturing files, schematics, and documentation may change as the hardware is built, tested, and refined.
