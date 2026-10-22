# Design brief: 555 LED Blinker

Build a small board that makes an LED blink about 1.5 times per second: a
2-layer PCB of 40 × 30 mm with production data (Gerber/Excellon, BOM,
pick&place, PDFs). It is the classic first project with LibrePCB: you touch
the libraries, your own library part, the schematic, the board and the
production data.

Coordinates are in mm, board coordinate system, X to the right, Y up.

## The circuit

A 555 timer in astable mode switches an LED on and off. R1, R2 and C1 set the
blink rate:

    f = 1.44 / ((R1 + 2·R2) · C1) = 1.44 / ((1 kΩ + 94 kΩ) · 10 µF) ≈ 1.5 Hz

Power comes from a 9 V battery or a lab supply (4.5 V to 15 V).

### Parts

| Designator | Part | Value | Device to use (library) |
|---|---|---|---|
| U1 | 555 timer | NE555P | **You create it** (in your own library "My Parts") |
| R1 | Resistor | 1 kΩ | Resistor THT 0207 (LibrePCB Base) |
| R2 | Resistor | 47 kΩ | Resistor THT 0207 (LibrePCB Base) |
| R3 | Resistor (LED current) | 470 Ω | Resistor THT 0207 (LibrePCB Base) |
| C1 | Electrolytic capacitor (timing) | 10 µF | Capacitor Radial ⌀5.0x11.0/2.0mm (LibrePCB Base) |
| C2 | Ceramic capacitor (control voltage) | 10 nF | Capacitor 2012 (0805) (LibrePCB Base) |
| C3 | Ceramic capacitor (decoupling) | 100 nF | Capacitor 2012 (0805) (LibrePCB Base) |
| LED1 | LED, red, 5 mm | red | LED ⌀5.0x8.7/2.54mm Red (LibrePCB Base) |
| J1 | 2-pin header (power input) | – | Generic Pin Header 2.54mm 1x02 ⌀1.0mm (LibrePCB Connectors) |

### Connections (netlist)

Pin names of U1 are the signal names you create for the 555.

| Net | Connected pins |
|---|---|
| VCC | J1:1, U1:VCC, U1:RESET, R1:1, C3:1 |
| GND | J1:2, U1:GND, C1:−, C2:2, C3:2, LED1:C |
| *(any name)* | U1:DIS, R1:2, R2:1 |
| **TIMING** | U1:TRIG, U1:THR, R2:2, C1:+ |
| *(any name)* | U1:CV, C2:1 |
| *(any name)* | U1:OUT, R3:1 |
| *(any name)* | R3:2, LED1:A |

## Tasks

Each task has a goal and the expected result.

### 1. Library part for the 555

No library contains a 555 timer, so create one in a new local library
**My Parts** (description "Parts for the 555 tutorial") that depends on
**LibrePCB Base**:

1. **Symbol** `NE555` (description "555 timer IC", category *Integrated
   Circuits › Clock, Timing › Timers, Oscillators*), 8 pins on the 2.54 mm
   grid, rectangular body, name above and value below the body. Suggested
   layout: VCC, RESET, TRIG, THR on the left; DIS, OUT, CV, GND on the right;
   body from (−7.62, 7.62) to (7.62, −7.62).
2. **Component** `NE555` using this symbol, prefix `U`, default value
   `{{MPN or DEVICE}}`, same category, 8 signals named like the pins.
3. **Device** `NE555 DIP-8` connecting the component to the DIP-8 package
   **DIP762W55P254L965H533Q8** from LibrePCB Base, with part number
   **NE555P** from **Texas Instruments**:

   | Pad | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
   |---|---|---|---|---|---|---|---|---|
   | **Signal** | GND | TRIG | OUT | RESET | CV | THR | DIS | VCC |

✅ Expected: symbol, component and device pass their library checks.

### 2. Project and schematic

1. Create a project **555 Blinker** with one schematic and one board.
2. Put an **A4 Landscape** schematic frame on the sheet.
3. Place every part from the parts table with the correct value (e.g. *R2
   47kΩ*, *C1 10μF*), plus VCC and GND supply symbols.
4. Connect everything as in the netlist.
5. Name the net of U1:TRIG, U1:THR, R2 and C1:+ **TIMING**; the name must be
   visible in the schematic.
6. Run the electrical rule check (ERC).

✅ Expected: every netlist connection exists, ERC shows no messages (or only
messages you approved with a reason), project saved.

### 3. Board

1. The board has 2 copper layers and default design rules.
2. Board outline exactly **40 × 30 mm**.
3. Place all 9 parts inside the outline with these devices and footprints:

   | Part | Device | Footprint |
   |---|---|---|
   | U1 | NE555 DIP-8 | hand soldering |
   | R1, R2, R3 | Resistor THT 0207 | Horizontal, 10.16mm, Compact |
   | C1 | Capacitor Radial ⌀5.0x11.0/2.0mm | default |
   | C2, C3 | Capacitor 2012 (0805) | Density Level B (median protrusion) |
   | LED1 | LED ⌀5.0x8.7/2.54mm Red | Vertical |
   | J1 | Generic Pin Header 2.54mm 1x02 ⌀1.0mm | (only option) |

   Placement hints: J1 at the left edge, LED1 at the right edge, C3 right next
   to U1 pin 8 (VCC), R1, R2, C1 and C2 near U1 pins 2, 5, 6 and 7.
4. Route all nets except GND with copper traces.
5. Fill the board with a GND copper plane on both copper layers. If a GND pad
   stays unconnected, connect it (e.g. with a via into the plane).
6. Run the design rule check (DRC) and fix every error.

✅ Expected: no unplaced parts, no unrouted connections, DRC shows no messages
(or only messages you approved with a reason), project saved.

### 4. Production data

Generate Gerber/Excellon files, a bill of materials, pick&place files and
PDFs into the project's output directory.

✅ Expected: `output/v1/` in the project folder contains the Gerber and drill
files, BOM and pick&place CSV files, the schematic and assembly PDFs, a `.zip`
archive and a `.lppz` project archive. The schematic shows U1 with the value
*NE555P*.
