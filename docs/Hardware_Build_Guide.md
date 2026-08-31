# ReflowDesk Hardware Build Guide

This guide shows you step by step how to assemble and solder the ReflowDesk AT-MK1 Motherboard and Daughterboard PCBs. It covers component placement, soldering order, jumper configuration, firmware flashing, and initial testing so you can build your own ReflowDesk hardware with fewer assembly mistakes.

I recommend using the included Interactive BOM files along with this Hardware Build Guide. The Interactive BOM makes it easier to track which electronic components have been sourced and which components have already been soldered during assembly.

Available Interactive BOM Files:
| File Name | Hardware |
| --- | --- |
| [ReflowDesk_AT-MK1_v1.0_ibom.html](../hardware/ReflowDesk_v1/bom/ReflowDesk_AT-MK1_v1.0_ibom.html) | ReflowDesk AT-MK1 Motherboard v1.0 |
| [ReflowDesk_AT-MK1_Daughterboard_v1.0_ibom.html](../hardware/ReflowDesk_Daughterboard_v1/bom/ReflowDesk_AT-MK1_Daughterboard_v1.0_ibom.html) | ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB |

---

## Tools Required

- Temperature-controlled soldering iron (fine tip: C210 knife tip and C210 conical point curved tip)
- SMD hot air rework station (optional)
- Desoldering wick
- Solder wire (leaded)
- Solder paste (leaded)
- Fine-tip tweezers
- Spatula or solder paste spreader
- Diagonal wire cutter or cutting pliers
- Digital multimeter
- 0.3mm double sided foam tape
- 6mm high density foam tape (speaker gasketing tape)

---

## Prepare the SMD Soldering Equipment

#### SMD Reflow Hot Plate

To make soldering the ReflowDesk Motherboard and Daughterboard PCBs easier, you can assemble a makeshift SMD reflow hot plate. You can build the ReflowDesk controller circuit on a breadboard. The breadboard connections will not be as solid as PCB connections and may introduce some noise, but it will still make the soldering process easier than using only a hot air SMD rework station or manually hand soldering every SMD part with a temperature-controlled soldering iron.

![ReflowDesk Prototype Image-1](../hardware/images/assembly/ReflowDesk_Prototype_1.jpg)

![ReflowDesk Prototype Image-2](../hardware/images/assembly/ReflowDesk_Prototype_2.jpg)

![ReflowDesk Prototype Image-3](../hardware/images/assembly/ReflowDesk_Prototype_3.jpg)

#### Hot Air SMD Rework Station

You can also use a hot air SMD rework station to solder the SMD components on the ReflowDesk AT-MK1 Motherboard and Daughterboard PCBs. This method is a bit tricky and requires some experience with SMD hot air soldering. Calibrate your hot air rework station and set the appropriate temperature and air speed levels. Each hot air rework station requires its own calibration, so one ideal set of values may not give the same results on another similar model.

<details open>
<summary>

## ReflowDesk AT-MK1 Motherboard Assembly

</summary>

### Electronics Components Required

- ReflowDesk AT-MK1 PCB rev1.0
- Momentary Push Buttons
- 5VDC, 240VAC/2A Solid State Relay Module
- MAX6675 Thermocouple Module
- ADS1115 16-bit ADC Module
- Other electronics components as per BOM

---

### Assembly Steps

- [Step 1: Prepare the PCB for Soldering](#step-1-prepare-the-pcb-for-soldering)
  - [Build an SMT Stencil Alignment Jig](#build-an-smt-stencil-alignment-jig)
  - [Apply the Solder Paste Using the SMT Stencil](#apply-the-solder-paste-using-the-smt-stencil)
- [Step 2: SMD Components Placement (PCB Top Layer)](#step-2-smd-components-placement-pcb-top-layer)
- [Step 3: SMD Components Reflow Soldering (PCB Top Layer)](#step-3-smd-components-reflow-soldering-pcb-top-layer)
- [Step 4: SMD Components Placement and Hand Soldering (PCB Bottom Layer)](#step-4-smd-components-placement-and-hand-soldering-pcb-bottom-layer)
- [Step 5: THT Components Placement and Hand Soldering](#step-5-tht-components-placement-and-hand-soldering)
- [Step 6: ReflowDesk Motherboard Jumper Configuration](#step-6-reflowdesk-motherboard-jumper-configuration)
- [Step 7: Flash the Firmware](#step-7-flash-the-firmware)
- [Step 8: Initial Testing (Optional)](#step-8-initial-testing-optional)

---

### Step 1: Prepare the PCB for Soldering

#### Build an SMT Stencil Alignment Jig

![AT-MK1 PCB SMT Stencil Jig Items](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_3.jpg)

Use the spare ReflowDesk Daughterboard PCBs and masking tape to secure the ReflowDesk AT-MK1 PCB in place.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_4.jpg)

Do not secure the AT-MK1 PCB too tightly between the spare ReflowDesk Daughterboard PCBs. The AT-MK1 PCB should slide out of the jig easily after the solder paste application step is complete.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_5.jpg)

Now align the SMT stencil over the ReflowDesk AT-MK1 PCB and secure it from one side with masking tape.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_6.jpg)

Make sure the SMT stencil openings align properly with the PCB footprints and solder paste areas.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_7.jpg)

Make sure the SMT stencil can be lifted without losing alignment. The masking tape placed on one side of the stencil works as a hinge.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_8.jpg)

#### Apply the Solder Paste Using the SMT Stencil

Now grab the solder paste and a spatula or solder paste spreader.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_10.jpg)

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/Solder_Paste_1.jpg)

Here I am using a clone ChipQuik REP183AX50T4 Sn63/Pb37 leaded solder paste. I recommend using genuine ChipQuik TS391AX50 Sn63/Pb37 solder paste if you can source it.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/Solder_Paste_2.jpg)

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/Solder_Paste_3.jpg)

Place a small amount of solder paste along one edge of the SMT stencil. Hold the stencil flat against the PCB, then spread the paste across the openings with firm, even pressure.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_11.jpg)

> [!WARNING]
> For safety, wear disposable gloves while working with solder paste. Leaded solder paste is toxic, so avoid skin contact and wash your hands after handling it.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_12.jpg)

Now carefully lift the SMT stencil without smudging the solder paste applied to the PCB.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_13.jpg)

> [!NOTE]
> Wipe the stencil with wipes or towels dipped in isopropyl alcohol to remove solder paste residue. This keeps the stencil clean for the next use and helps extend its life.

---

### Step 2: SMD Components Placement (PCB Top Layer)

Now we will start placing the SMD electronics components one by one on the top layer/front side of the ReflowDesk AT-MK1 Motherboard Rev1.0 PCB, where we just applied fresh solder paste using the SMT stencil.

Since I am going to use my makeshift SMD reflow hot plate to solder the components on the ReflowDesk AT-MK1 PCB, I will place components only on the upper half of the PCB top layer first. The full 100 mm x 100 mm PCB does not fit on my makeshift SMD reflow hot plate, so I will solder the ReflowDesk AT-MK1 PCB top layer in two halves.

#### 3.1 ESP32-S3 MCU Module Placement

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_18.jpg)

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_19.jpg)

First we are going to place the ESP32-S3 MCU module. Use tweezers to hold the ESP32-S3 module by its shield. Then identify pin 1 of the ESP32-S3 module and pin 1 of its PCB footprint, align the module, and place it in position without smudging or spreading the applied solder paste.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_20.jpg)

#### 3.2 LMR51420 Buck IC Placement

Next we are going to place the LMR51420 Buck IC. Here I am going to salvage this IC from a LMR51420 12V, 2A Buck Module.

![LMR51420 Buck Module (Back)](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_22.jpg)

Secure the buck module in a PCB holder so that the PCB does not move during IC desoldering.

![LMR51420 Buck Module (Front)](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_21.jpg)

Apply some flux paste on the LMR51420 IC for easier desoldering. I am going to use the SMD hot air rework station to desolder the LMR51420 IC, but you can also use a temperature-controlled soldering iron.

![LMR51420 IC Desoldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_23.jpg)

![LMR51420 IC Package Size](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_24.jpg)

Now identify the 1st pin of the LMR51420 IC (marked with a solid circle on the IC) and align it with the 1st pin of its footprint on the PCB. Then place the LMR51420 IC without smudging the applied solder paste.

![LMR51420 Buck Module](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_25.jpg)

#### 3.3 100nF 25V 0603 Capacitor Placement

Now we are going to place the 100nF 25V 0603 package MLCC. We need to place a total of 6 capacitors at C7, C20, C23, C30, C33, and C36 on the PCB.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_100nf_0603.png)

#### 3.4 10uF 16V 1206 Capacitor Placement

Now we are going to place the 10uF 16V 1206 package MLCC. We need to place only one capacitor at C22 on the PCB.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_26.jpg)

#### 3.5 22uF 16V 0805 Capacitor Placement

Now we are going to place the 22uF 16V 0805 package MLCC. We need to place only one capacitor at C29 on the PCB.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_27.jpg)

#### 3.6 10K Resistor Placement

Now we are going to place the 10K 0603 package resistor. We need to place a total of 12 resistors at R4, R6, R15, R16, R19, R20, R24, R29, R32, R33, R34, and R41 on the PCB.

![10K Resistor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_10K.png)

![10K Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_29.jpg)

#### 3.7 1uF 16V 0603 Capacitor Placement

Now we are going to place the 1uF 16V 0603 package MLCC. We need to place only one capacitor at C28 on the PCB.

![ReflowDesk AT-MK1 PCB Solder Paste Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_31.jpg)

#### 3.8 100K Resistor Placement

Now we are going to place the 100K 0603 package resistor. We need to place a total of 8 resistors at R7, R13, R17, R23, R28, R35, R36, and R39 on the PCB.

![100K Resistor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_100K.png)

![100K Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_34.jpg)

#### 3.9 4.7K Resistor Placement

Now we are going to place the 4.7K 0603 package resistor. We need to place a total of 6 resistors at R10, R18, R21, R31, R38, and R40 on the PCB.

![4.7K Resistor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_4K7.png)

![4.7K Resistor Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_86.jpg)

![4.7K Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_38.jpg)

#### 3.10 1K Resistor Placement

Now we are going to place the 1K 0603 package resistor. We need to place a total of 5 resistors at R11, R12, R37, R42, and R44 on the PCB.

![1K Resistor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_1K.png)

![1K Resistor Package/case marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_90.jpg)

![1K Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_39.jpg)

#### 3.11 470R Resistor Placement

Now we are going to place the 470R 0603 package resistor. We need to place a total of 3 resistors at R3, R5, and R26 on the PCB.

![470R Resistor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_470R.png)

![470R Resistor Package/case marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_92.jpg)

![470R Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_42.jpg)

#### 3.12 Power Status LED Placement

Now we are going to place SMD LED 0603 package (Color: Red). We need to place a total of 6 LEDs at D5, D7, D8, D9 and D15 on the PCB.

![Power Status LED Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_PWR_LED.png)

![Red LED 0603](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_43.jpg)

![Red LED 0603 Package Size](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_44.jpg)

Make sure to keep in mind the LED Cathode marking (normally marked at the bottom of the SMD LED in green color.)

![SMD 0603 LED Cathode Marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_45.jpg)

Now place the LED on the PCB according to the cathode marking of its footprint on the PCB

![Power Status LED Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_46.jpg)

#### 3.13 ESD Protection IC Placement

Now we are going to place the USBLC6-2SC6 ESD Protection IC. We need to place only one IC at U1 on the PCB.

![USBLC6-2SC6 ESD Protection IC](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_50.jpg)

Make sure to identify the 1st pin of the IC and then align it with the 1st pin of its footprint on the PCB and then place the IC with the help of tweezers.

![ESD Protection IC Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_52.jpg)

In my variant of the ReflowDesk AT-MK1 PCB I have made some mistakes with the USBLC6-2SC6 IC connections. So that is why in my case I need to completely bypass this IC and connect the required traces directly in order for the ReflowDesk Motherboard USB to UART Bridge IC connection to work. This way I am able to connect the ReflowDesk Motherboard to my computer and easily flash the firmware on its MCU.

![ESD Protection IC Bypass/Mistake Fix](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_53.jpg)

> [!NOTE]
> The USBLC6-2SC6 ESD protection IC connection mistakes shown above have been fixed/rectified in the released ReflowDesk AT-MK1 Motherboard Rev1.0 PCB Gerber files and ReflowDesk AT-MK1 Motherboard Rev1.0 schematic PDF. If you are assembling or soldering your own ReflowDesk Motherboard PCB from the released files, no USBLC6-2SC6 ESD protection IC bypass is required.

#### 3.14 SS8050 Transistor Placement

Now we are going to place the SS8050 Transistor (case marking: J3Y). We need to place a total of 4 transistors at Q1, Q2, Q4, and Q9 on the PCB.

![SS8050 Transistor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_SS8050.png)

![SS8050 Transistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_56.jpg)

#### 3.15 2N7002 MOSFET Placement

Now we are going to place the 2N7002 MOSFET (case marking: 12W). We need to place a total of 2 MOSFETs at Q7, and Q8 on the PCB.

![2N7002 MOSFET Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_2N7002.png)

![2N7002 MOSFET Package Size](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_58.jpg)

![2N7002 MOSFET Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_59.jpg)

#### 3.16 DMP3098L-7 P-Channel MOSFET Placement

Now we are going to place the DMP3098L-7 P-channel MOSFET (case marking: DMB). We need to place only one MOSFET at Q6 on the PCB.

![DMP3098L-7 P-Channel MOSFET case marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_60.jpg)

![DMP3098L-7 P-Channel MOSFET Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_61.jpg)

#### 3.17 NTC Thermistor Placement

Now we are going to place the 100K NTC Thermistor (0603 Package). We need to place only one NTC Thermistor at TH1 on the PCB.

![NTC Thermistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_62.jpg)

#### 3.18 100nF 50V 0805 Capacitor Placement

Now we are going to place the 100nF 50V 0805 package MLCC. We need to place a total of 3 capacitors at C10, C11, and C16 on the PCB.

![100nF 50V 0805 Capacitor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_100nf_0805.png)

![100nF 50V 0805 Capacitor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_72.jpg)

#### 3.19 22uF 35V 1210 Capacitor Placement

Now we are going to place the 22uF 35V 1210 package MLCC. We need to place a total of 2 capacitors at C12 and C13 on the PCB.

![22uF 35V 1210 Capacitor](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_76.jpg)

![22uF 35V 1210 Capacitor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_75.jpg)

#### 3.20 10uF 50V 1210 Capacitor Placement

Now we are going to place the 10uF 50V 1210 package MLCC. We need to place only one capacitor at C9 on the PCB.

![10uF 50V 1210 Capacitor](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_76.jpg)

![10uF 50V 1210 Capacitor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_77.jpg)

#### 3.21 100nF 25V 0805 Capacitor Placement

Now we are going to place the 100nF 25V 0805 package MLCC. We need to place only one capacitor at C17 on the PCB.

![100nF 25V 0805 Capacitor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_78.jpg)

#### 3.22 22uF 16V 1206 Capacitor Placement

Now we are going to place the 22uF 16V 1206 package MLCC. We need to place only one capacitor at C18 on the PCB.

![22uF 16V 1206 Capacitor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_79.jpg)

#### 3.23 10uF 16V 1206 Capacitor Placement

Now we are going to place the 10uF 16V 1206 package MLCC. We need to place only one capacitor at C22 on the PCB.

![10uF 16V 1206 Capacitor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_10uf_1206.png)

#### 3.24 15K Resistor Placement

Now we are going to place the 15K 0603 package resistor. We need to place a total of 2 resistors at R9 and R25 on the PCB.

![15K Resistor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_iBOM_15K.png)

![15K Resistor Package/case marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_94.jpg)

![15K Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_95.jpg)

#### 3.25 5.23K Resistor Placement

Now we are going to place the 5.23K 0603 package resistor. We need to place only one resistor at R8 on the PCB.

I am going to salvage this 5.23K resistor from the same Buck Converter Board from which I salvaged the LMR51420 IC.

![LMR51420 Buck Module (Front)](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_154.jpg)

![5.23K Resistor Package/case marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_155.jpg)

![5.23K Resistor Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_156.jpg)

![5.23K Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_157.jpg)

#### 3.26 22.1K Resistor Placement

Now we are going to place the 22.1K 0603 package resistor. We need to place only one resistor at R14 on the PCB.

![22.1K Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_97.jpg)

#### 3.27 0R Resistor/Jumper Placement

Now we are going to place the 0R 0603 package resistor. We need to place only one resistor at R22 on the PCB.

![0R Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_98.jpg)

#### 3.28 330R Resistor Placement

Now we are going to place the 330R 0603 package resistor. We need to place only one resistor at R27 on the PCB.

![330R Resistor Package/case marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_99.jpg)

![330R Resistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_100.jpg)

#### 3.29 Status LED Placement

Now we are going to place SMD LED 0603 package (Color: Green). We need to place a total of 2 LEDs at D10 and D12 on the PCB.

![Green LED 0603](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_104.jpg)

![Green LED 0603 Package Size and Marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_105.jpg)

Make sure to keep in mind the LED Cathode marking (normally marked at the bottom of the SMD LED in green color.)

Now place the LED on the PCB according to the cathode marking of its footprint on the PCB

![Status LED Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_107.jpg)

#### 3.30 SS8550 Transistor Placement

Now we are going to place the SS8550 PNP transistor (case marking: 2TY). We need to place only one transistor at Q3 on the PCB.

![SS8550 Transistor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_113.jpg)

#### 3.31 22uH Inductor Placement

Now we are going to place the 22uH, 3A inductor (case marking: 220). We need to place only one inductor at L1 on the PCB.

![22uH Inductor Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_116.jpg)

![22uH Inductor Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_117.jpg)

![22uH Inductor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_122.jpg)

#### 3.32 33uH Inductor Placement

Now we are going to place the 33uH, 4A inductor (case marking: 330). We need to place only one inductor at L2 on the PCB.

![33uH Inductor Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_116.jpg)

![33uH Inductor Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_122.jpg)

![33uH Inductor Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_123.jpg)

#### 3.33 SS54 Diode Placement

Now we are going to place the SS54 Schottky Diode (case marking: SS54). We need to place a total of 2 diodes at D3 and D4 on the PCB.

![SS54 Diode Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_124.jpg)

![SS54 Diode Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_125.jpg)

#### 3.34 LM2596S-5 Buck IC Placement

Now we are going to place the LM2596S-5 Buck IC (case marking: LM2596S-5.0). We need to place only one IC at U5 on the PCB.

![LM2596S-5 IC Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_126.jpg)

![LM2596S-5 Buck IC Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_128.jpg)

Apply some flux on the LM2596S-5 IC footprint on the PCB, for even solder paste flow and good solder joints.

![LM2596S-5 Buck IC Flux Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_127.jpg)

![LM2596S-5 IC Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_129.jpg)

#### 3.35 AOD4184A MOSFET Placement

Now we are going to place the AOD4184A N-Channel MOSFET (case marking: OD4184A). We need to place only one MOSFET at Q5 on the PCB.

![AOD4184A MOSFET Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_131.jpg)

![AOD4184A MOSFET Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_132.jpg)

#### 3.36 MAX6675 Thermocouple IC Placement

Now we are going to place the MAX6675 Thermocouple IC (case marking: MAX6675). We need to place only one IC at U10 on the PCB.

I am going to salvage this IC from a MAX6675 Module/Board.

![MAX6675 Thermocouple Module](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_158.jpg)

![MAX6675 IC Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_160.jpg)

Make sure to identify the pin-1 of the MAX6675 IC and align it with the pin-1 of its footprint on the PCB and then place the MAX6675 IC carefully using tweezers.

![MAX6675 IC Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_163.jpg)

#### 3.37 USB Type-C Connector Placement
Now we are going to place the USB4105-GF-A USB Type-C Connector. We need to place only one connector at J2 on the PCB.

![USB Type-C Connector Bottom](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_167.jpg)

![USB Type-C Connector Pins](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_169.jpg)

![USB Type-C Connector Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_170.jpg)

The pitch between the USB Type-C connector pins is very narrow. If a solder bridge is created between the connector pins, use a fine conical tip soldering iron to wick away the extra solder and remove the bridge.

![USB Type-C Connector Solder Bridge Removal](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_171.jpg)

![USB Type-C Connector Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_172.jpg)

![USB Type-C Connector Front View](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_173.jpg)

Now flip the ReflowDesk PCB and secure the USB Type-C Connector shield legs by filling solder inside the footprint holes.

![USB Type-C Connector Shield/Housing Legs](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_176.jpg)

![USB Type-C Connector Shield/Housing Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_177.jpg)

### Step 3: SMD Components Reflow Soldering (PCB Top Layer)

![Partially Populated ReflowDesk AT-MK1 PCB Top Layer](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_134.jpg)

As mentioned above, my makeshift SMD reflow hot plate has a small heating area, so I am going to solder the ReflowDesk AT-MK1 PCB in two halves, one side at a time.

#### AT-MK1 PCB Top Layer (1st Half Soldering)

![AT-MK1 PCB Top Layer 1st Half Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_64.jpg)

![AT-MK1 PCB Top Layer 1st Half Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_65.jpg)

#### AT-MK1 PCB Top Layer (2nd Half Soldering)

![AT-MK1 PCB Top 2nd Half Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_138.jpg)

![AT-MK1 PCB Top 2nd Half Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_142.jpg)

![Soldered AT-MK1 PCB Top Layer View-1](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_143.jpg)

![Soldered AT-MK1 PCB Top Layer View-2](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_146.jpg)

Now inspect the PCB carefully for solder bridges between IC pins or nearby components. Use a knife-tip (K-tip) soldering iron to wick away the extra solder and remove any bridges.

![Solder Bridges on AT-MK1 PCB](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_150.jpg)

Now we are done with soldering SMD electronics components on the Top Layer of the ReflowDesk AT-MK1 PCB. Let's proceed with soldering SMD electronics components on the Bottom Layer of the ReflowDesk AT-MK1 PCB.

![Fully populated Top Layer of AT-MK1 PCB](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_181.jpg)

### Step 4: SMD Components Placement and Hand Soldering (PCB Bottom Layer)

#### 5.1 5.1K Resistor Placement

Now we are going to place the 5.1K 0603 package resistor. We need to place a total of 2 resistors at R1 and R2 on the PCB.

![5.1K Resistors Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_182.jpg)

#### 5.2 CH340K USB UART Bridge IC Placement

Now we are going to place the CH340K USB UART Bridge IC (case marking: CH340K). We need to place only one IC at U3 on the PCB.

![CH340K IC Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_183.jpg)

> [!NOTE]
> To solder this CH340K IC, you need a hot air SMD rework station because the CH340K package has a thermal relief pad on the bottom that must be connected to its PCB footprint. This is not practical to hand solder with a soldering iron.

Apply some flux on the CH340K footprint on the PCB, for easy solder flow.

![CH340K IC Flux Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_184.jpg)

First apply a small amount of solder to every pad of the CH340K footprint on the PCB, so the CH340K IC can be soldered more easily using the hot air method. Then apply some more flux to the CH340K footprint pads. Hold the CH340K IC with tweezers in one hand and the hot air station handle in the other hand, then start heating the CH340K footprint area on the PCB. Once the solder starts to melt, quickly lower the CH340K IC into place. Make sure pin 1 of the CH340K IC is aligned with the pin 1 marking on its PCB footprint.

![CH340K IC Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_185.jpg)

#### 5.3 TVS Diode Placement

Now we are going to place the SMAJ5.0A TVS Diode (case marking: AES). We need to place a total of 2 diodes at D1 and D2 on the PCB.

![SMAJ5.0A TVS Diode Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_186.jpg)

![SMAJ5.0A TVS Diodes Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_188.jpg)

#### 5.4 100nF 25V 0603 Capacitor Placement

Now we are going to place the 100nF 25V 0603 package MLCC. We need to place a total of 3 capacitors at C6, C24, and C35 on the PCB.

![100nF 25V 0603 Capacitor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_Bottom_iBOM_100nf_0603.png)

![100nF 25V 0603 Capacitors Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_190.jpg)

#### 5.5 8.2uH Inductor Placement

Now we are going to place the 8.2uH, 3A inductor (case marking: 8R2). We need to place only one inductor at L3 on the PCB.

![8.2uH Inductor Package](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_193.jpg)

![8.2uH Inductor Bottom](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_194.jpg)

![8.2uH Inductor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_197.jpg)

#### 5.6 6V PPTC Fuse Placement

Now we are going to place the 6VDC, 2A PPTC Fuse (case marking: HQ). We need to place only one fuse at F3 on the PCB.

![6V PPTC Fuse Package Size](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_199.jpg)

![6V PPTC Fuse Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_201.jpg)

#### 5.7 Zener Diode Placement

Now we are going to place the MMBZ5242BLT1G Zener Diode (case marking: 8S). We need to place only one diode at D14 on the PCB.

![MMBZ5242BLT1G Zener Diode Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_202.jpg)

![MMBZ5242BLT1G Zener Diode Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_203.jpg)

#### 5.8 10uF 16V 0805 Capacitor Placement

Now we are going to place the 10uF 16V 0805 package MLCC. We need to place only one capacitor at C5 on the PCB.

![10uF 16V 0805 Capacitor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_204.jpg)

#### 5.9 4.7K Resistor Placement

Now we are going to place the 4.7K 0603 package resistor. We need to place only one resistor at R30 on the PCB.

![4.7K Resistor Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_205.jpg)

![4.7K Resistor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_206.jpg)

#### 5.10 10K Resistor Placement

Now we are going to place the 10K 0603 package resistor. We need to place only one resistor at R43 on the PCB.

![10K Resistor Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_207.jpg)

![10K Resistor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_208.jpg)

#### 5.11 SS54 Diode Placement

Now we are going to place the SS54 SMC package diode (case marking: SS54). We need to place only one diode at D6 on the PCB.

![SS54 Diode Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_209.jpg)

![SS54 Diode Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_210.jpg)

Now we have successfully soldered SMD electronics components on the Bottom Layer of the ReflowDesk AT-MK1 PCB. Let's proceed with soldering THT electronics components on the Top Layer of the ReflowDesk AT-MK1 PCB.

![Fully populated Bottom Layer of AT-MK1 PCB](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_211.jpg)

---

### Step 5: THT Components Placement and Hand Soldering

![AT-MK1 PCB Top Layer](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_213.jpg)

#### 6.1 1N5242B Diode Placement

Now we are going to place the 1N5242B Zener Diode. We need to place only one diode at D13 on the PCB.

![1N5242B Zener Diode](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_214.jpg)

Insert the 1N5242B Zener diode into its correct holes and make sure the cathode orientation is correct. Hold the diode in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the diode legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the diode legs in place with a soldering iron.

![1N5242B Zener Diode Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_215.jpg)

![1N5242B Zener Diode Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_216.jpg)

#### 6.2 EL817 Optocoupler Placement

Now we are going to place the EL817 Optocoupler (case marking: EL817). We need to place only one optocoupler at U9 on the PCB.

![EL817 Optocoupler](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_217.jpg)

Insert the optocoupler into its correct holes on the PCB and make sure the optocoupler pin 1 is aligned with the pin 1 marking on its footprint on the PCB. Hold the optocoupler in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and with the help of a soldering iron, solder the optocoupler legs in place.

![EL817 Optocoupler Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_218.jpg)

![EL817 Optocoupler Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_219.jpg)

#### 6.3 Momentary Push Button Placement

Now we are going to place the THT Momentary Push Buttons (6mm x 6mm x 5mm). We need to place a total of 2 momentary push buttons at S1 and S2 on the PCB.

![Momentary Push Buttons Placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_221.jpg)

Insert the momentary push button into its correct holes on the PCB and make sure the push button orientation is correct. Hold the push button in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and solder the push button legs in place with a soldering iron.

![Momentary Push Buttons Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_222.jpg)

#### 6.4 10uF 16V Electrolytic Capacitor Placement

Now we are going to place the 10uF 16V Electrolytic Capacitor. We need to place a total of 3 capacitors at C21, C31 and C32 on the PCB.

![10uF 16V Electrolytic Capacitor Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_THT_iBOM_10uf_16v.png)

![10uF 16V Electrolytic Capacitors](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_223.jpg)

Insert the 10uF 16V electrolytic capacitor into its correct holes and make sure the polarity is correct. Hold the capacitor in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the capacitor legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the capacitor legs in place with a soldering iron.

![10uF 16V Electrolytic Capacitor Legs Shortening](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_224.jpg)

![10uF 16V Electrolytic Capacitor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_225.jpg)

#### 6.5 100uF 35V Electrolytic Capacitor Placement

Now we are going to place the 100uF 35V Electrolytic Capacitor. We need to place a total of 2 capacitors at C14 and C37 on the PCB.

![100uF 35V Electrolytic Capacitor](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_227.jpg)

Insert the 100uF 35V electrolytic capacitor into its correct holes and make sure the polarity is correct. Hold the capacitor in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the capacitor legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the capacitor legs in place with a soldering iron.

![100uF 35V Electrolytic Capacitors Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_231.jpg)

![100uF 35V Electrolytic Capacitors Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_230.jpg)

#### 6.5 100uF 16V Electrolytic Capacitor Placement

Now we are going to place the 100uF 16V Electrolytic Capacitor. We need to place a total of 2 capacitors at C27 and C34 on the PCB.

![100uF 16V Electrolytic Capacitor](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_232.jpg)

Insert the 100uF 16V electrolytic capacitor into its correct holes and make sure the polarity is correct. Hold the capacitor in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the capacitor legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the capacitor legs in place with a soldering iron.

![100uF 16V Electrolytic Capacitors Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_233.jpg)

> [!NOTE]
> Capacitor C34 needs to be soldered in a horizontal orientation. If it is soldered vertically, the 100uF 16V electrolytic capacitor is tall enough to interfere with the placement of the ADS1115 ADC module.

![100uF 16V Electrolytic Capacitor height interfering with ADC module placement](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_235.jpg)

Bend the legs of the C34 100uF 16V electrolytic capacitor to 90 degrees with tweezers so that the capacitor can sit in its holes horizontally. Now flip the AT-MK1 PCB and cut the C34 capacitor legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the C34 capacitor legs in place with a soldering iron.

![C34 Electrolytic Capacitor Soldered in horizontal orientation](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_236.jpg)

#### 6.6 10uF 50V Electrolytic Capacitor Placement

Now we are going to place the 10uF 50V Electrolytic Capacitor. We need to place only one capacitor at C8 on the PCB.

![10uF 50V Electrolytic Capacitors](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_237.jpg)

Insert the 10uF 50V electrolytic capacitor into its correct holes and make sure the polarity is correct. Hold the capacitor in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the capacitor legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the capacitor legs in place with a soldering iron.

![10uF 50V Electrolytic Capacitor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_239.jpg)

#### 6.7 220uF 50V Electrolytic Capacitor Placement

Now we are going to place the 220uF 50V Electrolytic Capacitor. We need to place only one capacitor at C15 on the PCB.

![220uF 50V Electrolytic Capacitors](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_240.jpg)

Insert the 220uF 50V electrolytic capacitor into its correct holes and make sure the polarity is correct. Hold the capacitor in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the capacitor legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the capacitor legs in place with a soldering iron.

![220uF 50V Electrolytic Capacitors Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_241.jpg)

![220uF 50V Electrolytic Capacitor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_242.jpg)

#### 6.8 220uF 16V Electrolytic Capacitor Placement

Now we are going to place the 220uF 16V Electrolytic Capacitor. We need to place only one capacitor at C19 on the PCB.

![220uF 16V Electrolytic Capacitors](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_244.jpg)

Insert the 220uF 16V electrolytic capacitor into its correct holes and make sure the polarity is correct. Hold the capacitor in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the capacitor legs to an appropriate length using a wire cutter, so that only very short legs stick out of the holes. Then solder the capacitor legs in place with a soldering iron.

![220uF 16V Electrolytic Capacitors Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_245.jpg)

![220uF 16V Electrolytic Capacitor Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_246.jpg)

#### 6.9 30V PPTC Fuse Placement

Now we are going to place the 30VDC, 2.5A PPTC Fuse (case marking: U250). We need to place only one fuse at F1 on the PCB.

![30VDC PPTC Fuse](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_247.jpg)

Insert the 30VDC PPTC Fuse into its correct place/holes. Hold the fuse in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the fuse legs to appropriate length using a wire cutter, so that only very short fuse legs stick out of the holes. Then with the help of a soldering iron, solder the fuse legs in place.

![30VDC PPTC Fuse Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_248.jpg)

![30VDC PPTC Fuse Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_250.jpg)

#### 6.10 30V PPTC Fuse Placement

Now we are going to place the 30VDC, 3A PPTC Fuse (case marking: UF300). We need to place only one fuse at F2 on the PCB.

![30VDC PPTC Fuse Placement Location](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_251.jpg)

Insert the 30VDC PPTC Fuse into its correct place/holes. Hold the fuse in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and cut the fuse legs to appropriate length using a wire cutter, so that only very short fuse legs stick out of the holes. Then with the help of a soldering iron, solder the fuse legs in place.

![30VDC PPTC Fuse Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_252.jpg)

![30VDC PPTC Fuse Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_254.jpg)

#### 6.11 Mini Blade Fuse Holder Placement

Now we are going to place the mini blade fuse holder. We need to place only one fuse holder at F4 on the PCB.

Insert the fuse holder into its correct holes. Hold the fuse holder in place with masking tape, then flip the AT-MK1 Motherboard PCB and solder the fuse holder legs in place with a soldering iron.

![Fuse Holder Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_255.jpg)

![Fuse Holder Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_257.jpg)

#### 6.12 Screw Terminal Connector Placement

##### 2-Position Screw Terminal Connector Placement

Now we are going to place the 2-Position Screw Terminal Connector. We need to place a total of 3 connectors at J9, J10, and J11 on the PCB.

![2Pos Screw Terminal Connector Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_THT_iBOM_2Pos_Scew_Terminal.png)

![2Pos Screw Terminal Connector](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_258.jpg)

Insert the 2-Position Screw Terminal Connector into its correct place/holes. Hold the connector in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and then with the help of a soldering iron, solder the connector legs in place.

![2Pos Screw Terminal Connector Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_265.jpg)

![2Pos Screw Terminal Connectors Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_267.jpg)

##### 3-Position Screw Terminal Connector Placement

Now we are going to place the 3-Position Screw Terminal Connector. We need to place only one connector at J3 on the PCB.

Insert the 3-Position Screw Terminal Connector into its correct place/holes. Hold the connector in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and then with the help of a soldering iron, solder the connector legs in place.

![3Pos Screw Terminal Connector Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_270.jpg)

![3Pos Screw Terminal Connector Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_268.jpg)

#### 6.13 4-Pin PWM Fan Connector Placement

Now we are going to place the 4-Pin PWM Fan Connector. We need to place a total of 3 connectors at J12, J13, J14 on the PCB.

![4-Pin PWM Fan Connector Placement Locations](../hardware/images/assembly/ReflowDesk_PCB/MK1_THT_iBOM_4Pin_Fan_Conc.png)

For this step I am going to modify 4-Pin Relitimate male connectors (RMC-2510) to create Molex-style 4-Pin PWM fan connectors (SD-47053-001), since I was not able to purchase the Molex 4-Pin PWM fan connectors locally.

![4-Pin Relitimate RMC-2510 Connector](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_271.jpg)

Follow the steps given below to modify the 4-Pin Relitimate RMC-2510 Connector:

1. First arrange a 4-Pin PWM fan. We are going to use its 4-Pin female connector as a reference.

![4-Pin PWM Fan Female Connector](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_273.jpg)

2. Align the 4-Pin Relitimate male connector (RMC-2510) close to the 4-Pin PWM fan female connector so that the fan connector slot/tab lock aligns with the plastic tab of the Relitimate male connector. Then use a fine-tip marker to draw a line on the extra tab area that extends outside the fan connector slot/tab lock.

![4-Pin Relitimate Male Connector Cut Marking](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_274.jpg)

3. Now with the help of a sharp slim scissor or a wire cutter, cut the tab along the line drawn during the previous step.

![Modified 4-Pin Relitimate RMC-2510 Connector](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_275.jpg)

4. Now bring your 4-Pin PWM fan and test-fit the newly modified 4-Pin Relitimate RMC-2510 connector by sliding it into the fan female connector. If it does not fit perfectly, trim the extra plastic from the Relitimate male connector tab area as needed.

![Modified 4-Pin Relitimate RMC-2510 Connector Fit Test](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_276.jpg)

5. Now repeat steps 2 to 4 to create the remaining two modified 4-Pin Relitimate connectors.

By following the above steps correctly you will get the required three 4-Pin PWM Fan Connectors.

![Modified 4-Pin Relitimate RMC-2510 Connectors](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_278.jpg)

Insert the modified 4-Pin PWM fan connector into its correct holes. Hold the connector in place with tweezers or masking tape, then flip the AT-MK1 PCB and solder the connector legs in place with a soldering iron.

![Modified 4-Pin Relitimate RMC-2510 Connector Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_279.jpg)

![4-Pin PWM Fan Connectors Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_283.jpg)

#### 6.14 SSR Placement

Now we are going to place the Omron G3MB-202P 5VDC SSR (Solid State Relay). We need to place only one SSR at U8 on the PCB.

I am going to salvage the G3MB-202P SSR from a 5VDC SSR module because I was unable to source the standalone G3MB-202P SSR from electronics component sellers in my region. Only SSR modules containing the G3MB-202P SSR were available.

![G3MB-202P SSR Module](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_287.jpg)

![G3MB-202P SSR Module Bottom](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_288.jpg)

Secure the SSR module in a PCB holder or clamp and apply some flux to its soldered legs. Melt some solder wire on the tip of the soldering iron, touch the iron to one SSR leg to heat the joint, then remove the melted solder with a desoldering pump.

![G3MB-202P SSR Desoldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_289.jpg)

![G3MB-202P SSR](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_290.jpg)

Insert the SSR into its correct place/holes on the PCB. Hold the SSR in place with your fingers or use masking tape, then flip the AT-MK1 Motherboard PCB and then with the help of a soldering iron, solder the SSR legs in place.

![G3MB-202P SSR Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_292.jpg)

![G3MB-202P SSR Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_293.jpg)

#### 6.15 ADS1115 ADC Module Placement

Now we are going to solder the ADS1115 ADC Module. We need to solder only one ADC Module at J8 on the PCB.

![ADS1115 ADC Module Top View](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_295.jpg)

![ADS1115 ADC Module Bottom View](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_296.jpg)

Grab some foam tape, cut a small piece the size of the ADS1115 module, and place it under the module to support it.

![ADS1115 ADC Module Foam Tape Application](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_297.jpg)

Insert the ADC module into its correct holes on the PCB. Make sure to identify pin 1 of the ADS1115 module and align it with the pin 1 marking on its PCB footprint. Hold the ADC module in place with your fingers or masking tape, then flip the AT-MK1 Motherboard PCB and solder the ADC module legs in place with a soldering iron.

![ADS1115 ADC Module Legs Soldering](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_298.jpg)

![ADS1115 ADC Module Soldered](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_299.jpg)

Now we have successfully soldered all the SMD and THT electronics components on the ReflowDesk AT-MK1 PCB, and the ReflowDesk Motherboard is ready for testing.

ReflowDesk Motherboard Top View
![ReflowDesk Motherboard Top View](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_300.jpg)

ReflowDesk Motherboard Bottom View
![ReflowDesk Motherboard Bottom View](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_301.jpg)

---

### Step 6: ReflowDesk Motherboard Jumper Configuration

The ReflowDesk AT-MK1 PCB Rev1.0 has 4 jumpers: JP1, JP2, JP3, and JP4. These can be used to configure the motherboard power input voltage and heater type.

#### JP1 Jumper: System Power Input Voltage Selection Jumper

![ReflowDesk Motherboard JP1 Jumper](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_302.jpg)

| Jumper Bridge | ReflowDesk Input Voltage |
| --- | --- |
| JP1 Pad1 and Pad2 Bridged | 24VDC |
| JP1 Pad2 and Pad3 Bridged | 12VDC |

#### JP2 and JP3 Jumpers: Heating Element Signal Type Selection Jumpers

| Jumper Bridge | Connected Heater Type |
| --- | --- |
| JP2 Bridged | AC Heater |
| JP3 Bridged | DC Heater |

> [!WARNING]
> Do not bridge both JP2 and JP3. Doing so enables both the AC PTC heating element control circuit and the DC PTC heating element control circuit, which is not intended.

#### JP4 Jumper: DC Heating Element Power Activation Jumper

Bridging the JP4 jumper delivers power to the DC PTC heating element if it is connected to the ReflowDesk Motherboard. This jumper should only be bridged if your ReflowDesk setup uses a DC PTC heating element instead of the AC PTC heating element.

![ReflowDesk Motherboard JP4 Jumper](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_303.jpg)

In my case, since I am building a ReflowDesk with an AC PTC heating element, I have bridged jumper JP2. Since I am using a 24VDC SMPS to power the ReflowDesk motherboard, I have also bridged JP1 Pad1 and Pad2 to set the input voltage configuration to 24VDC.

![ReflowDesk Motherboard JP4 Jumper 24V Configuration](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_304.jpg)

#### Initial Power-Up of the ReflowDesk Motherboard

Once you have configured the ReflowDesk motherboard jumpers, connect power to the ReflowDesk Motherboard J3 power terminal.

![ReflowDesk Motherboard Input Power Terminal](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_268.jpg)

| J3 Connector Pin | Connection |
| --- | --- |
| Pin-1 | +24VDC |
| Pin-2 | Ground (GND) |
| Pin-3 | +12VDC |

![ReflowDesk Motherboard Input Power Terminal](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_305.jpg)

Once you have connected the power supply wires to the ReflowDesk Motherboard J3 screw terminal power connector with the correct polarity, power up the SMPS and check whether all the ReflowDesk Motherboard power status LEDs light up.

> [!WARNING]
> Make sure the input voltage wires from the SMPS are connected with the correct polarity in the ReflowDesk motherboard power connector. Incorrect polarity can damage the ReflowDesk motherboard.

![ReflowDesk Motherboard Input Power Terminal](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_306.jpg)

| Power Status LED | Live Voltage Rails |
| --- | --- |
| D5 | +24VDC |
| D7 | +12VDC |
| D8 | +5VDC |
| D9 | +3.3VDC |

If all the power status LEDs on your ReflowDesk motherboard light up, the voltage rails are likely outputting their respective voltages. To confirm, use a digital multimeter to check each voltage rail output.

---

### Step 7: Flash the Firmware

Grab a USB Type-C to Type-A cable and connect your ReflowDesk Motherboard to your computer. Download the appropriate pre-compiled `.bin` file from this repo's [Releases](https://github.com/atechofficials/ReflowDesk/releases) page, or compile your own ReflowDesk firmware by cloning the [ReflowDesk repo](https://github.com/atechofficials/ReflowDesk).

![ReflowDesk Motherboard Firmware Upload](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_310.jpg)

> [!NOTE]
> The ReflowDesk motherboard onboard USB connector does not supply power to the motherboard while it is connected to a computer, so keep the SMPS turned on to keep ReflowDesk powered.

If you are using Windows, open Device Manager to find out which COM port is used by the CH340K USB-to-UART bridge IC.

#### Pre-compiled Firmware Upload

If you downloaded the pre-compiled ReflowDesk firmware `.bin` file, open a terminal and use esptool to upload the firmware to the ReflowDesk motherboard MCU.

| Firmware Name | Heater Configuration | Hardware |
| --- | --- | --- |
| ReflowDesk_AT-MK1-DC-PTC_8MB_v1.x.x_factory.bin | For DC PTC Heaters | ReflowDesk Motherboard Rev1.0 |
| ReflowDesk_AT-MK1_8MB_v1.x.x_factory.bin | For AC PTC Heaters | ReflowDesk Motherboard Rev1.0 |
| ReflowDesk_AT-MK1-DC-PTC_16MB_v1.x.x_factory.bin | For DC PTC Heaters | DIY Experimental ReflowDesk |
| ReflowDesk_AT-MK1_16MB_v1.x.x_factory.bin | For AC PTC Heaters | DIY Experimental ReflowDesk |

Example Command:
```bash
esptool.py --chip esp32s3 --port COM4 write-flash -ff 80m -fm dio -fs 8MB -e 0x0 ReflowDesk_AT-MK1_8MB_v1.0.0_factory.bin
```

If esptool is not installed on your computer, you can download it from [here](https://github.com/espressif/esptool/releases).

#### Compile and Upload Firmware

1. Clone the ReflowDesk project to a preferred location on your computer.

```bash
git clone https://github.com/atechofficials/ReflowDesk.git
```

2. Open the ReflowDesk codebase in VS Code.

```bash
cd ReflowDesk
code .
```

3. Install the PlatformIO extension in VS Code.

4. Compile the ReflowDesk firmware. Open a terminal in VS Code and run the command below:

```bash
pio run -e at-mk1
``` 

5. Upload the firmware to the ReflowDesk motherboard.

```bash
pio run -e at-mk1 -t upload
```

---

### Step 8: Initial Testing (Optional)

Now let's prepare the ReflowDesk Motherboard for initial boot. Connect a 2.4GHz WiFi antenna to the ESP32-S3 module of the ReflowDesk motherboard using a U.FL IPEX male connector to SMA female connector cable.

![ReflowDesk Motherboard WiFi Antenna Installation](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_311.jpg)

- Now turn on the SMPS and power the ReflowDesk motherboard.

- Once the ReflowDesk motherboard is powered, the ReflowDesk firmware will boot up and the ReflowDesk WiFi Access Point `ReflowDesk-AT-MK1` will be discoverable.

- Connect your computer to the ReflowDesk WiFi AP `ReflowDesk-AT-MK1` using the default password `ReflowDesk@2037`

- Once your computer is connected to the ReflowDesk AP, open this IP address `192.168.4.1` in a web browser.

- The WiFiManager Portal will open up, select your router's WiFi AP and connect to it.

- Open the ReflowDesk web interface by visiting the IP address assigned to the ReflowDesk by your home router.

- Select any existing reflow profile and start the reflow process.

> [!NOTE]
> Do not connect any heating element or thermocouple to the ReflowDesk motherboard. This test only checks whether the ReflowDesk heater control circuit is receiving PWM signals from the MCU. We have not connected a ReflowDesk Daughterboard (user input board) to the ReflowDesk Motherboard yet, but it is not required for this test.

- Observe whether the heater power status green LED lights up during the reflow process test.

![ReflowDesk Motherboard Heater Power Status](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_313.jpg)

- Since we have not connected any K-Type thermocouple to the ReflowDesk motherboard, it will not receive any valid heater temperature readings and will activate the failure/error mode, in which it will cut off power to the heater control circuit and start the hot plate cooling fan at full speed.

- Connect a 4-Pin PWM cooling fan to J12 or J13 Fan connector (Hot Plate Cooling Fan Connectors). Observe whether the fan starts spinning when ReflowDesk is in failure/error mode.

![ReflowDesk Motherboard Cooling Fan Spin](../hardware/images/assembly/ReflowDesk_PCB/MK1_Assembly_315.jpg)

</details>

---

<details>
<summary>

## ReflowDesk AT-MK1 Daughterboard Assembly

</summary>

### Electronics Components Required

- ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB
- I2C or SPI SSD1306 OLED Display Module (0.96-inch or 1.3-inch)
- Rotary Encoder with push button
- Piezo Electric Buzzer
- Momentary Push Buttons
- Other electronics components as per BOM

---

### Assembly Steps

- [Step 1: SMD Components Placement and Hand Soldering (PCB Top Layer)](#step-1-smd-components-placement-and-hand-soldering-pcb-top-layer)
- [Step 2: SMD Components Placement and Hand Soldering (PCB Bottom Layer)](#step-2-smd-components-placement-and-hand-soldering-pcb-bottom-layer)
- [Step 3: THT Components Placement and Hand Soldering](#step-3-tht-components-placement-and-hand-soldering)
- [Step 4: ReflowDesk Daughterboard Jumper Configuration](#step-4-reflowdesk-daughterboard-jumper-configuration)

---

### Step 1: SMD Components Placement and Hand Soldering (PCB Top Layer)

Now we will start by hand soldering the SMD electronics components one by one on the top layer of the ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB.

![ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Rev1_PCB.jpg)

First secure the ReflowDesk Daughterboard PCB in a PCB holder so that it does not move when we start soldering.

![ReflowDesk AT-MK1 Daughterboard Secure](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_1.jpg)

#### 1.1 10K Resistor Placement

Now we are going to place the 10K 0603 package resistor. We need to place a total of 3 resistors at R6, R7, and R8 on the PCB.

![10K 0603 Resistors Placement Location](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_3.jpg)

![10K 0603 Resistors](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_2.jpg)

Now apply some flux on the resistor pads on which we need to solder these 10K 0603 resistors.

![10K 0603 Resistor Flux Application](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_4.jpg)

With the help of tweezers grab a 10K 0603 resistor from one end and align it on its respective pad on the PCB.

![10K 0603 Resistor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_5.jpg)

Now with the help of a soldering iron (C210 knife tip) solder one end of the 10K 0603 resistor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the resistor. 

![10K 0603 Resistor Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_6.jpg)

If large solder blobs form on your SMD resistor/capacitor joints, apply some flux to the joints and reflow the whole resistor/capacitor by heating both solder joints with a soldering iron (C210 knife tip). C210 knife tips can easily cover the whole component, especially 0603 and 0805 packages. This helps the soldering iron tip absorb the extra solder, removes solder blobs, and gives the joints a cleaner look.

> [!NOTE]
> Do not overheat the resistor/capacitor; otherwise it may dislodge from its PCB pads and stick to the soldering iron tip.

![10K 0603 Resistor Extra Solder Removal](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_7.jpg)

#### 1.2 100K Resistor Placement

Now we are going to place the 100K 0603 package resistor. We need to place a total of 2 resistors at R1 and R5 on the PCB.

![100K 0603 Resistors Placement Location](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_iBOM_100K.png)

![100K 0603 Resistors](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_9.jpg)

Apply some flux on the resistor pads on which we need to solder these 100K 0603 resistors.

With the help of tweezers grab a 100K 0603 resistor from one end and align it on its respective pad on the PCB.

![100K 0603 Resistor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_10.jpg)

Now with the help of a soldering iron (small knife tip) solder one end of the 100K 0603 resistor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the resistor. 

![100K 0603 Resistor Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_11.jpg)

#### 1.3 1K Resistor Placement

Now we are going to place the 1K 0603 package resistor. We need to place only one resistor at R4 on the PCB.

![1K 0603 Resistors](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_13.jpg)

Apply some flux on the resistor pads on which we need to solder these 1K 0603 resistors.

With the help of tweezers grab a 1K 0603 resistor from one end and align it on its respective pad on the PCB.

![1K 0603 Resistor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_14.jpg)

Now with the help of a soldering iron (small knife tip) solder one end of the 1K 0603 resistor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the resistor. 

![1K 0603 Resistor Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_15.jpg)

#### 1.4 SS8050 Transistor Placement

Now we are going to place the SS8050 NPN Transistor. We need to place only one transistor at Q1 on the PCB.

![SS8050 Transistors](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_16.jpg)

First apply some flux on the pads where the transistor needs to be soldered.

![SS8050 Transistor Flux Application](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_17.jpg)

Now grab the transistor with tweezers and align it on its respective pads. Keep holding the transistor with the tweezers and solder one leg first, then remove the tweezers. The transistor should now stay in place during the remaining soldering.

![SS8050 Transistor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_18.jpg)

![SS8050 Transistor Soldering Process](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_19.jpg)

Now with the help of a soldering iron (small knife tip) solder the remaining legs of the transistor.

![SS8050 Transistor Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_20.jpg)

#### 1.5 100K NTC Thermistor Placement

Now we are going to place the 100K 0603 NTC Thermistor. We need to place only one thermistor at TH1 on the PCB.

![100K NTC Thermistor](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_21.jpg)

First apply some flux on the pads where the thermistor needs to be soldered. Then grab the 100K 0603 NTC thermistor with tweezers and align it on its respective pads.

![100K NTC Thermistor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_22.jpg)

Now with the help of a soldering iron (small knife tip) solder one leg of the thermistor, then remove the tweezers, and then solder the other leg of the thermistor.

![100K NTC Thermistor Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_23.jpg)

#### 1.6 100R Resistor Placement

Now we are going to place the 100R 0603 package resistor. We need to place only one resistor at R2 on the PCB.

Apply some flux on the resistor pads where this 100R 0603 resistor will be soldered.

![100R 0603 Resistor Flux Application](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_24.jpg)

With the help of tweezers grab a 100R 0603 resistor from one end and align it on its respective pad on the PCB.

![100R 0603 Resistor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_25.jpg)

Now with the help of a soldering iron (small knife tip) solder one end of the 100R 0603 resistor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the resistor. 

![100R 0603 Resistor Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_26.jpg)

#### 1.7 0R Resistor/Jumper Placement

Now we are going to place the 0R 0603 package resistor/jumper. We need to place only one resistor at R9 on the PCB.

![0R 0603 Resistors](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_27.jpg)

Apply some flux on the resistor pads where this 0R 0603 resistor will be soldered.

With the help of tweezers grab a 0R 0603 resistor from one end and align it on its respective pad on the PCB.

![0R 0603 Resistor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_28.jpg)

Now with the help of a soldering iron (small knife tip) solder one end of the 0R 0603 resistor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the resistor.

![0R 0603 Resistor Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_29.jpg)

#### 1.8 100nF 50V 0603 Capacitor Placement

Now we are going to place the 100nF 50V 0603 package capacitor. We need to place only one capacitor at C7 on the PCB.

Apply some flux on the capacitor pads where this 100nF 50V 0603 capacitor will be soldered.

With the help of tweezers grab a 100nF 50V 0603 capacitor from one end and align it on its respective pad on the PCB.

![100nF 50V 0603 Capacitor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_30.jpg)

Now with the help of a soldering iron (small knife tip) solder one end of the 100nF 50V 0603 capacitor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the capacitor.

#### 1.9 WS2812B-2020 RGB LED Placement

Now we are going to place the WS2812B-2020 RGB LED. We need to place only one RGB LED at D2 on the PCB.

![WS2812B-2020 RGB LEDs](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_31.jpg)

I am not going to hand solder this WS2812B-2020 RGB LED. I am going to use my makeshift reflow soldering hot plate, but you can also use a hot air rework station.

With the help of a spatula/blade tool, take a very tiny amount of solder paste and apply it to the RGB LED pads without spreading it all over the pads.

![WS2812B-2020 RGB LED Solder Paste Application](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_33.jpg)

Now grab a WS2812B-2020 RGB LED with tweezers and place it on its respective pads on the PCB. Make sure the RGB LED orientation is correct and the RGB LED pin-1 is aligned with the pin-1 marker on the PCB pads.

RGB LED pin 1 is marked with a green triangle on the RGB LED package, on both the bottom and top sides.

![WS2812B-2020 RGB LED pin-1 marker](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_34.jpg)

Now apply some flux near the RGB LED pads on the ReflowDesk Daughterboard PCB.

![WS2812B-2020 RGB LED Flux Application](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_37.jpg)

As I have mentioned above I am going to use my makeshift Reflow Soldering Hot Plate to solder this WS2812B-2020 RGB LED on the Daughterboard PCB.

![WS2812B-2020 RGB LED Reflow Soldering](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_38.jpg)

You can also use a hot air rework station to solder this WS2812B-2020 RGB LED on the Daughterboard PCB. When using hot air, make sure the temperature and airflow are configured correctly. Too much heat can crack the RGB LED lens, and too much airflow can blow the RGB LED off the PCB.

![WS2812B-2020 RGB LED Hot Air Soldering](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_39.jpg)

![WS2812B-2020 RGB LED Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_40.jpg)

---

### Step 2: SMD Components Placement and Hand Soldering (PCB Bottom Layer)

Now we will start by hand soldering the SMD electronics components one by one on the bottom layer of the ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB.

Flip the ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB and secure it using a PCB holder. Make sure the PCB holder does not scratch or damage the SMD components soldered on the PCB top layer.

#### 2.1 100nF 50V 0603 Capacitor Placement

Now we are going to place the 100nF 50V 0603 package capacitor. We need to place a total of 4 capacitors at C2, C4, C6, and C8 on the PCB.

![100nF 50V 0603 Capacitor Placement Locations](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Bottom_iBOM_100nF_0603.png)

Apply some flux on the capacitor pads where this 100nF 50V 0603 capacitor will be soldered.

With the help of tweezers grab a 100nF 50V 0603 capacitor from one end and align it on its respective pad on the PCB.

![100nF 50V 0603 Capacitor Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_41.jpg)

Now with the help of a soldering iron (small knife tip) solder one end of the 100nF 50V 0603 capacitor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the capacitor.

![100nF 50V 0603 Capacitors Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_45.jpg)

#### 2.2 10uF 16V 0805 Capacitors Placement

Now we are going to place the 10uF 16V 0805 package capacitors. We need to place a total of 2 capacitors at C1 and C3 on the PCB.

![10uF 16V 0805 Capacitors](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_46.jpg)

Apply some flux on the capacitor pads on which we need to solder these 10uF 16V 0805 capacitors.

![10uF 16V 0805 Capacitor Flux Application](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_47.jpg)

With the help of tweezers grab a 10uF 16V 0805 capacitor from one end and align it on its respective pad on the PCB.

Now with the help of a soldering iron (small knife tip) solder one end of the 10uF 16V 0805 capacitor that is not covered by the tweezers, then let go of the tweezers and solder the other end of the capacitor.

![10uF 16V 0805 Capacitors Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_49.jpg)

#### 2.3 1N4148W Diode Placement

Now we are going to place the 1N4148W SOD-323 package diode. We need to place only one diode at D1 on the PCB.

![1N4148W SOD-323 Diode](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_50.jpg)

Apply some flux on the diode pads on which we need to solder this 1N4148W SOD-323 diode.

With the help of tweezers grab a 1N4148W SOD-323 diode from one end and align it on its respective pad on the PCB.

![1N4148W SOD-323 Diode Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_51.jpg)

Now with the help of a soldering iron (small knife tip) solder one end of the 1N4148W SOD-323 diode that is not covered by the tweezers, then let go of the tweezers and solder the other end of the diode.

![1N4148W SOD-323 Diode Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_52.jpg)

#### 2.4 Logic Level Translator IC Placement

Now we are going to place the 74AHCT125D Logic Level Translator IC. We need to place only one IC at U1 on the PCB.

![74AHCT125D Logic Level Translator IC Placement Location](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_54.jpg)

![74AHCT125D Logic Level Translator IC](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_53.jpg)

Apply some flux on the IC pads on which we need to solder this 74AHCT125D Logic Level Translator IC.

![74AHCT125D Logic Level Translator IC Flux Application](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_55.jpg)

With the help of tweezers grab the 74AHCT125D Logic Level Translator IC from its rectangular body and align its pin-1 with the pin-1 on its footprint/pads on the ReflowDesk Daughterboard PCB.

![74AHCT125D Logic Level Translator IC Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_56.jpg)

Now with the help of a soldering iron (small knife tip) solder one leg of the 74AHCT125D Logic Level Translator IC, then remove the tweezers.

![74AHCT125D Logic Level Translator IC Leg Soldering](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_57.jpg)

Now solder the remaining legs of the 74AHCT125D logic level translator IC.

![74AHCT125D Logic Level Translator IC Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_61.jpg)

---

### Step 3: THT Components Placement and Hand Soldering

Now we will start by hand soldering the THT electronics components one by one on the ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB.

#### 3.1 10uF 16V Electrolytic Capacitor Placement

Now we are going to place the 10uF 16V Electrolytic Capacitor. We need to place only one capacitor at C5 on the PCB.

![10uF 16V Electrolytic Capacitor](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_62.jpg)

Insert the 10uF 16V Electrolytic Capacitor into its correct place/holes on the PCB and make sure the capacitor polarity orientation is correct. Hold the capacitor in place with your fingers or use masking tape, then flip the AT-MK1 Daughterboard PCB and cut the capacitor legs to appropriate length using a wire cutter, so that only very short capacitor legs stick out of the holes. Then with the help of a soldering iron, solder the capacitor legs in place.

![10uF 16V Electrolytic Capacitors Legs Soldering](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_63.jpg)

![10uF 16V Electrolytic Capacitor Legs Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_64.jpg)

![10uF 16V Electrolytic Capacitor Soldered (Front View)](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_65.jpg)

#### 3.2 Momentary Push Button Placement

Now we are going to place the THT Momentary Push Buttons (6mm x 6mm x 5mm). We need to place a total of 2 momentary push buttons at S1 and S2 on the PCB.

![Momentary Push Buttons Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_66.jpg)

Insert the momentary push button into its correct place/holes on the PCB and make sure the push button orientation is correct. Hold the push button in place with your fingers or use masking tape, then flip the AT-MK1 Daughterboard PCB and then with the help of a soldering iron, solder the push button legs in place.

![Momentary Push Buttons Legs Soldering](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_67.jpg)

![Momentary Push Buttons Legs Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_68.jpg)

#### 3.3 Piezo Electric Buzzer Placement

Now we are going to place the THT Piezo Electric Buzzer. We need to place only one buzzer at LS1 on the PCB.

![Piezo Electric Buzzer Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_69.jpg)

Insert the buzzer into its correct place/holes on the PCB and make sure the buzzer is placed according to correct polarity. Hold the buzzer in place with your fingers or use masking tape, then flip the AT-MK1 Daughterboard PCB and cut the buzzer legs to appropriate length using a wire cutter, so that only very short buzzer legs stick out of the holes. Then with the help of a soldering iron, solder the buzzer legs in place.

![Piezo Electric Buzzer Legs Soldering](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_70.jpg)

![Piezo Electric Buzzer Legs Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_71.jpg)

#### 3.4 Rotary Encoder Placement

Now we are going to place the THT Rotary Encoder. We need to place only one rotary encoder at MT1 on the Daughterboard PCB.

![Rotary Encoder](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_73.jpg)

Insert the rotary encoder into its correct place/holes on the PCB. Hold the rotary encoder in place with your fingers or use masking tape, then flip the AT-MK1 Daughterboard PCB and cut the rotary encoder legs to appropriate length using a wire cutter, so that only very short rotary encoder legs stick out of the holes. Then with the help of a soldering iron, solder one of the mounting legs of the rotary encoder. Now let go of the rotary encoder, it won't fall out of the PCB.

![Rotary Encoder Legs Soldering](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_74.jpg)

Now solder the other remaining legs of the rotary encoder.

![Rotary Encoder Legs Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_75.jpg)

![Rotary Encoder Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_76.jpg)

#### 3.5 OLED Display Module Placement

Now we are going to place the OLED Display Module. We need to place only one OLED Display Module at J3 (for SPI display) or J4 (for I2C display) on the Daughterboard PCB.

In my case I am going to solder a 0.96-inch I2C OLED display module. The ReflowDesk Daughterboard PCB supports OLED display modules with either I2C or SPI interfaces and 0.96-inch or 1.3-inch display sizes.

![OLED Display Module (Front View)](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_77.jpg)

![OLED Display Module (Back View)](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_78.jpg)

Grab a 6mm high density foam tape (speaker gasketing tape) and cut a small piece the length of the OLED display module.

![Foam Tape Cut Piece](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_79.jpg)

Apply the foam tape at the center of the OLED display module placement area on the ReflowDesk Daughterboard PCB. Now test-fit the OLED display module and check whether it sits correctly on the Daughterboard PCB, with its legs coming out from the bottom of the PCB at an appropriate length for soldering. Make sure the OLED display module is not too far from or too close to the PCB surface. Reduce the height of the foam tape by cutting it with a box cutter as needed.

![Foam Tape Placement](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_80.jpg)

Hold the OLED display module in position with masking tape, then flip the ReflowDesk AT-MK1 Daughterboard PCB. Apply some flux to the OLED display module legs and solder them in place with a soldering iron.

![OLED Display Module Legs Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_81.jpg)

Now the ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB is ready for the next step: jumper configuration.

![OLED Display Module Soldered](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_83.jpg)

---

### Step 4: ReflowDesk Daughterboard Jumper Configuration

The ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB has 2 jumpers, JP1 and JP2, which can be used to configure the input power pin order for an I2C OLED display module.

![ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB Jumpers](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_84.jpg)

Jumper configuration table:

| Jumper Bridge | Pin Configuration |
| --- | --- |
| JP1 Pad1 and Pad2 Bridged | +3.3V DC |
| JP1 Pad2 and Pad3 Bridged | GND |
| JP2 Pad1 and Pad2 Bridged | +3.3V DC |
| JP2 Pad2 and Pad3 Bridged | GND |

> [!WARNING]
> Do not set both JP1 and JP2 jumpers to the same power signal. Doing this can short-circuit the OLED display module and may also short-circuit the ReflowDesk Motherboard.

My I2C OLED display module has the following pin order:

| Pin Number | Function |
| --- | --- |
| Pin-1 | GND |
| Pin-2 | 3.3V |
| Pin-3 | SCL |
| Pin-4 | SDA |

So I have set the JP1 jumper to GND and JP2 jumper to +3.3V DC.

![ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB Jumper Configuration](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_86.jpg)

Now we have successfully soldered all the SMD and THT electronics components on the ReflowDesk AT-MK1 Daughterboard Rev1.0 PCB, and the ReflowDesk Daughterboard is ready for testing.

![ReflowDesk AT-MK1 Daughterboard (Top Layer)](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_87.jpg)

![ReflowDesk AT-MK1 Daughterboard (Bottom Layer)](../hardware/images/assembly/Daughterboard_PCB/Daughterboard_Assembly_88.jpg)

---

</details>
