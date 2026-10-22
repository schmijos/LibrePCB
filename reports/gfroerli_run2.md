The board is complete: ERC has 0 messages, DRC has 0 open messages (21 approved, explained below), and 0 connections are unrouted. Production data was generated with no warnings.

`BLIND/REPORT.md` was **not written**. A hook refused my Write call ("Subagents should return findings as text, not write report files"), and I did not get around it via Bash. The full report is below with all 10 required sections; please save it to `BLIND/REPORT.md`.

- **Project:** `BLIND/projects/Water Temperature Node/Water Temperature Node.lpp` (saved, then closed so the lock is released).
- **Production data:** `.../output/v1.0/` (Gerber/Excellon, assembly and schematic PDFs, BOM, pick&place, .lppz/.zip).
- **Approved DRC messages:** 16 come from net-less copper strips inside the library SMA footprint; 5 are the edge-mount SMA pads touching the board edge, which is by design.
- **Partly met:** D3 (RF line about 19 mm, 50 Ω only hand-estimated), E1 (thermal decoupling can't be verified with the tools) and F3 (the MCU has one USART, so firmware must switch it or bit-bang the debug UART).
- **Biggest tool gap:** there is no router. About 260 manual route/via calls, and I reassigned MCU pins three times when routes would have had to cross.

---

# Water Temperature Node v1.0 – design report (blind tool evaluation)

Project: `projects/Water Temperature Node/Water Temperature Node.lpp`
Production data: `projects/Water Temperature Node/output/v1.0/` (Gerber/Excellon, assembly and schematic PDF, BOM, pick&place, .lppz, .zip)

## 1. Summary

**What I finished**
- A complete schematic: 48 parts, 34 nets. ERC reports 0 messages.
- A 2-layer board with the outline from the brief. I rounded the five inner corners with a 0.5 mm radius, which is the router radius. Both mounting holes are in place. All 48 parts are placed on the top side.
- Routing is 100 % complete: `board_status` reports 0 unrouted connections. The board has 82 vias and about 300 trace segments, with GND planes on both layers.
- DRC: 0 non-approved messages. I approved 21 messages and explain them below. All of them come from the SMA library footprint.
- Production data was generated with `output_jobs_run`, with no warnings.
- I created custom library elements for the parts that were missing: the LPC11U24 symbol, component and device, a 3.2×2.5 mm ABM8G crystal package and device, and a device for the Cinch 142-0701-801 SMA jack.

**What I did not finish, or only partly**
- The RF line is about 19 mm long. It is a grounded coplanar waveguide (1.0 mm trace, 0.25 mm gap) that I sized from a hand estimate, not with a field solver. The SMA pad geometry forces a detour around the jack's ground pad and plated hole.
- The LPC11U24 has only one USART. The UART1 debug header therefore sits on the alternative USART pins (PIO1_13/14). Firmware has to switch IOCON between the modem and the debug pins, or bit-bang the debug console. This is a limit of the MCU, not of the tools.
- The schematic is functional but not pretty: one page, connections made with net labels, plus block titles I added.
- I could not check the thermal decoupling of the SHT21 or the RF match with the tools.

## 2. Design decisions per block

### A – Power
- **Battery clips (A1):** the library device "Keystone 54 Battery Clips", footprint "18650 65mm (unprotected)". It is placed at (22.05, 38.2) with rotation 0, so the + clip is at the top.
  - The three pins of each clip are joined by 0.8 mm top-layer traces.
- **Reverse-polarity protection (A2):** Q1 is a Toshiba SSM3J338R P-MOSFET in the high side.
  - Drain goes to the battery +, source to VBAT. The gate goes to GND through R1 = 10 kΩ, which carries no static current and limits current if the gate is ever damaged.
  - With a correctly inserted cell, VGS = −2.8 to −4.2 V and RDS(on) is about 20–30 mΩ. The drop at 40 mA TX is about 1 mV, so the brief's "no noticeable loss" holds.
  - With a reversed cell, VGS = 0 and the body diode is reverse biased. The reverse voltage of 4.2 V is far below |VDS| = 12 V.
- **LDO (A3):** U1 is a TI LP5907QMFX-3.3Q1 (library device "TI LP5907-Q1", MPN selected).
  - EN is tied to VIN, so there is no switch.
  - Capacitors: C2 = 1 µF at the input, C1 = 10 µF (0805) bulk on VBAT for the transmit peaks, C3 = 1 µF at the output.
  - C7 at the output was 4.7 µF. I lowered it to 1 µF so the total output capacitance stays below about 10 µF. Together with C11 (4.7 µF at the module) and seven 100 nF capacitors, the nominal total is about 7.4 µF.
  - Below about 3.35 V cell voltage the LDO is in dropout and 3.3 V follows the cell. This is acceptable: the MCU runs down to 1.8 V, and the SHT21 and RN2483 down to about 2.1 V.
- Power traces: VBAT_RAW 0.8 mm, VBAT 0.5 mm, +3V3 trunk 0.5 mm, +3V3 to the module 0.4 mm. Signals are 0.25 mm (0.2 mm in tight fan-outs).

### B – Battery voltage measurement
Topology: Q2 (SSM3J338R) is a high-side P-MOSFET switch.
- R2 = 1 MΩ pulls its gate to VBAT.
- Q3 (Toshiba SSM3K329R N-MOSFET) pulls the gate low when the MCU drives BAT_MEAS_EN high.
- R3 = 100 kΩ keeps Q3 off.

Behind the switch:
- Divider: R4 = 100 kΩ (top) and R5 = 280 kΩ (bottom), both 0.1 % / 25 ppm thin film. I entered them as Panasonic ERA-3AEB104V and ERA-3AEB2803V and set the value to "100k 0.1%".
- C4 = 100 nF at the ADC node.

Calculations:
- **Ratio:** k = 280/380 = 0.73684.
  - 4.2 V → 3.095 V and 2.8 V → 2.063 V at the ADC, below the 3.3 V full scale with margin for LDO tolerance.
  - 1 LSB of 3.22 mV equals 4.4 mV of battery voltage.
- **B3:** the ratio error is ±(1−k)·(δR4+δR5) = ±0.263·0.2 % = ±0.053 % worst case, inside the 0.2 % requirement.
  - Q2's RDS(on) of 30 mΩ in series with 380 kΩ adds less than 0.00001 %.
  - Pin leakage of 10 nA max across the 73.7 kΩ Thevenin resistance adds 0.74 mV, or 0.024 %. This assumes the AD0 pin's pull-up is disabled in IOCON. Firmware must do this.
- **B2:** when off, the gate is at VBAT, so R2 carries no current, and R3 and the divider carry none either. What remains is MOSFET leakage: nA typical at 4.2 V, against a datasheet IDSS limit of 1 µA at full rated VDS. The target of < 1 µA is met in practice.
  - When on, the circuit draws 11 µA (divider) + 4.2 µA (R2) for a few tens of ms.
  - Settling: τ = 73.7 kΩ · 100 nF = 7.4 ms. Wait at least 60 ms after enabling before sampling.
- **Caveat:** LPC11U pins have their pull-up enabled during reset, so Q3 may switch on briefly during boot. This costs a few µA for milliseconds until firmware drives the pin low.

### C – Microcontroller
- **C1:** U2 is the LPC11U24FBD48/401. I built my own symbol, component and device, using the existing LQFP50P900X900X140-48 package from the ST library.
- **C2 crystal:** Y1 is an Abracon ABM8G-12.000MHZ-4Y-T3 on my own 4-pad footprint. Pads 1 and 3 are the crystal; pads 2 and 4 are GND (lid).
  - Load capacitors C8 and C9 are 27 pF C0G: 2·(CL − Cstray) = 2·(18 − 4.5) ≈ 27 pF, assuming CL = 18 pF.
- **C3 decoupling:**
  - VDD1 (pin 8): C5 = 100 nF, about 2 mm of trace from the pin.
  - VDD2 (pin 44): C6 = 100 nF, directly at the pins 41/44 pair.
  - LDO output: C3 = 1 µF and C7 = 1 µF.
  - Module: C11 = 4.7 µF, C12 = C13 = 100 nF. SHT21: C14 = 100 nF.
- **C4 SWD:** J1 is a CNC Tech 3220-10-0300-00 with the standard Cortex pinout. Pin 1 goes to +3V3, pins 3/5/9 to GND, pin 2 to SWDIO, pin 4 to SWCLK and pin 10 to nRESET. Pins 6/7/8 are not connected.
  - R6 = 10 kΩ pulls up nRESET. I removed the planned 10 nF reset capacitor for space and so it would not interfere with debugger resets.
- **C5:** USB_DM and USB_DP are left open.
- PIO0_1, the ISP strap, is left open with its internal pull-up so the bootloader is not entered by accident.

### D – LoRaWAN modem
- **D1:** U3 is the RN2483 library device with MPN RN2483A-I/RM103. RFH is used; RFL is left open.
- **D2:** the modem UART is on the USART's PIO0_18/PIO0_19 location (57600 baud, RTS/CTS unused).
  - R7 and R8 (1 kΩ) are in series between the MCU and the module.
  - The UART0 header J4 sits on the MCU side; the service header J6 sits on the module side. A USB-serial adapter or programmer can then overdrive the other party through 1 kΩ (at most 3.3 mA of contention) without damage.
- **D3:** J7 is the Cinch 142-0701-801. Its edge sits at X = 39 and its centre at Y = 17, pointing +X.
  - The RF line is 1.0 mm wide with a 0.25 mm gap to the top GND pour over a solid bottom GND. My estimate is about 50 Ω (coplanar k = 0.667, εeff ≈ 2.75 → about 54 Ω, a little lower with the bottom ground).
  - Five GND stitching vias run along the line.
  - Length is about 19 mm, about 0.1 λ_eff at 868 MHz.
- **D4:** J6 (1×7) carries module RESET (with R9 = 1 kΩ to MCU PIO0_21 and R10 = 10 kΩ pull-up), UART_RX, UART_TX, PGC, PGD, 3V3 and GND.
- Module GND: every GND pad has its own via to the bottom plane.

### E – Sensors
- **E1:** U4 (SHT21) is at (52.5, 4.0), at the far end of the tab. Measures to decouple it thermally:
  - a 1 × 2 mm milled slot (X 40–41, Y 4.4–6.4) across the tab entrance;
  - a keep-out for planes on both layers over the whole tab (X 39.3–56), so no copper pour conducts heat;
  - only five thin traces (0.25–0.3 mm) cross the remaining bridges: SCL, SDA, +3V3 on the bottom and GND, +3V3 on the top;
  - C14 is right next to the sensor.
- **E2:** probe header J2 (1×3): pin 1 GND, pin 2 DQ, pin 3 3V3. R13 = 4.7 kΩ pulls DQ up to 3.3 V, the usual value for a 1 m cable. DQ goes to PIO0_3.
- **E3:** I²C expansion J3 (1×4): pin 1 3V3, pin 2 SDA, pin 3 SCL, pin 4 GND. Pull-ups R11 and R12 are 4.7 kΩ: rise time about 0.4 µs at 100 pF, fine for 100/400 kHz, and 0.7 mA only while low.
  - The SHT21 bus runs from J3 to the tab on the bottom layer.

### F – User interface
- **F1:** LED1 red (LS R976-NR-1), LED2 yellow (LY R976-PS-36), LED3 green (LG R971-KN-1).
  - Each runs from its own GPIO through R14–R16 = 680 Ω: (3.3 − 1.9) V / 680 Ω ≈ 2 mA, a few mA as requested.
  - They are placed at the top of the left wing.
- **F2:** UART0 header J4 (1×2): pin 1 RX = module TX, pin 2 TX = module RX, on the MCU side of R7/R8.
- **F3:** UART1 header J5 (1×2): pin 1 RX, pin 2 TX, on PIO1_14 (RXD) and PIO1_13 (TXD).
  - The single USART must be time-multiplexed between the two pin locations, or the debug console is bit-banged. PIO1_13 and PIO1_14 also have CT16B0 match/capture functions, which help with a software UART.

Power budget (sleep, estimate): LDO 12 µA + MCU deep power-down about 2 µA + RN2483 sleep about 1.3 µA + SHT21 0.15 µA + DS18B20 0.75 µA + divider < 0.1 µA ≈ 16 µA. With a 2500 mAh cell, sleep current alone would last more than a year, so transmissions set the real lifetime. Several months is clearly achievable.

## 3. MCU pin assignment (LPC11U24FBD48)

| Pin | Name | Net / use |
|---:|---|---|
| 3 | RESET/PIO0_0 | nRESET (J1.10, R6 10 k pull-up) |
| 4 | PIO0_1 | not connected (ISP strap, internal pull-up) |
| 5, 41 | VSS | GND |
| 6 / 7 | XTALIN / XTALOUT | Y1 12 MHz, C8/C9 27 pF |
| 8, 44 | VDD | +3V3 (C5, C6 100 nF) |
| 14 | PIO0_3 | OW_DQ (J2.2, R13 4.7 k) |
| 15 | PIO0_4/SCL | I2C_SCL (R11, J3.3, U4) |
| 16 | PIO0_5/SDA | I2C_SDA (R12, J3.2, U4) |
| 17 | PIO0_21 | MODEM_RST_MCU → R9 1 k → RN2483 !RESET |
| 19 / 20 | USB_DM / USB_DP | not connected (USB unused) |
| 29 | SWCLK/PIO0_10 | SWCLK (J1.4) |
| 31 | PIO1_29 | BAT_MEAS_EN (Q3 gate) |
| 32 | TDI/PIO0_11/AD0 | VBAT_ADC (divider output) |
| 33 | TMS/PIO0_12 | LED_GRN (R16 → LED3) |
| 34 | TDO/PIO0_13 | LED_YEL (R15 → LED2) |
| 35 | TRST/PIO0_14 | LED_RED (R14 → LED1) |
| 36 | PIO1_13 (TXD alt.) | U1_TXD → J5.2 (debug) |
| 37 | PIO1_14 (RXD alt.) | U1_RXD → J5.1 (debug) |
| 39 | SWDIO/PIO0_15 | SWDIO (J1.2) |
| 46 | PIO0_18/RXD | U0_RXD ← R8 1 k ← RN2483 UART_TX; J4.1 |
| 47 | PIO0_19/TXD | U0_TXD → R7 1 k → RN2483 UART_RX; J4.2 |
| all others | – | not connected |

I chose pins mostly so that routing would be possible without crossings, with the MCU rotated 90°:
- the crystal on the side facing the module;
- SWD and measurement on the top;
- I²C and the probe line on the right, so they can run in a corridor around the MCU;
- the UART lines on the left, towards the battery and wing channel.

The JTAG-function pins 32–35 must be switched to GPIO/AD in IOCON, which is normal with SWD.

## 4. Placement and routing approach

**Zones**
- Top band, above the cell (X 13–33, Y 75–90): Q1/LDO and the measurement switch.
- MCU at (42.5, 68), rotated 90°, between the module and J1. J1 (SWD) is at the top right.
- RN2483 at (41.05, 41.2), right of the cell and just above the notch. Its RF pins face down towards the SMA. It fills the 20 mm between the cell (X ≈ 31.4) and the board edge.
- Left wing, headers top to bottom: J5 UART1, J4 UART0, J6 modem service, J2 probe (lower half, at the left edge), J3 I²C. LEDs are at the top of the wing.
- Tab: only U4 and C14.

**Layers**
- Under the cell, the bottom layer carries the horizontal signal bundle to the wing. The top layer carries the I²C/1-Wire corridor (OW, SCL, SDA) and a +3V3 feed down to the tab.
- The +3V3 tree runs mostly on the bottom.
- Both layers are poured with GND (clearance 0.25 mm). GND pads are stitched with vias.
- In the wing, crossings are resolved by hopping onto the top layer: RX/TX and PGC/PGD/RST into J6.

**Method**
- I routed entirely by hand with explicit corner coordinates (`board_route` with points), checking clearances with `board_query` and renders.
- Rough count: 264 route/via calls, plus many repositioning and query calls.
- I moved several parts and reassigned MCU pins three times when the routing turned out to be non-planar.

## 5. Requirement compliance

| ID | Status | Reason |
|---|---|---|
| A1 | met | Keystone 54 clips (library device), 18650 65 mm footprint |
| A2 | met | Q1 PMOS ideal-diode style, about 1 mV drop at 40 mA, survives reverse cell |
| A3 | met | LP5907QMFX-3.3Q1, EN tied to VIN, no switch |
| B1 | met | Divider to AD0 (pin 32), 2.06–3.10 V range |
| B2 | met | Switched high side. Only MOSFET leakage when off (nA typical, ≤ 1 µA datasheet worst case) |
| B3 | met | 0.1 % resistors give ±0.053 % ratio error |
| C1 | met | LPC11U24FBD48/401 (custom library part on the existing LQFP-48 package) |
| C2 | met | ABM8G 12 MHz on a custom 4-pad footprint, 27 pF load capacitors (assuming CL = 18 pF) |
| C3 | met | 100 nF at each VDD pin plus bulk |
| C4 | met | 3220-10-0300-00, standard Cortex 10-pin pinout including nRESET |
| C5 | met | USB pins not connected |
| D1 | met | RN2483A-I/RM103, RFH |
| D2 | met | USART on PIO0_18/19, no flow control |
| D3 | partly | Edge SMA placed as specified. 50 Ω GCPW is a hand estimate and the line (about 19 mm) is longer than ideal because of the SMA pad geometry |
| D4 | met | J6 with RESET, UART RX/TX, PGC, PGD, 3V3, GND |
| E1 | partly | On the tab, far end, with a slot, no pours on the tab and thin traces. Effectiveness not verifiable with the tools; the bridges still carry 5 traces |
| E2 | met | J2 GND/DQ/3V3 with 4.7 k pull-up |
| E3 | met | J3 3V3/SDA/SCL/GND, pull-ups on board |
| F1 | met | Three LEDs, each on its own GPIO, about 2 mA |
| F2 | met | J4 on the MCU↔modem link (RX/TX from the MCU's view) |
| F3 | partly | J5 on the alternative USART pins. The MCU has only one USART, so the debug console needs IOCON multiplexing or a software UART |
| M1 | met | NPTH Ø3.2 at (34.5, 82.5) and (34.5, 7.5) |
| M2 | met | Cell centre at (22.05, 38.2), + at the top. No components in X 13–31, Y 3–73. The module courtyard starts at X 31.15 and its pads at X ≥ 31.38 |
| M3 | met | J7 at X = 39, Y = 17, pointing +X |
| M4 | met | SHT21 at (52.5, 4.0) on the tab |
| M5 | met | J2 at X = 2.54, Y 27–32 (lower half of the wing), cable exits to the left |
| M6 | met | All parts on top (bottom pick&place is empty) |
| PCB technology | met | 2 layers, 1.6 mm. Minimum trace and clearance 0.2 mm. Vias 0.6/0.3 mm; I set the DRC annular ring rule to 0.15 to match. Power traces 0.4–0.8 mm. GND planes on both layers. Silkscreen has the name and version plus pin labels for every header |
| Deliverable 1 | met | Project in `projects/` with schematic and board |
| Deliverable 2 | met | ERC 0 messages. DRC 0 non-approved; 21 approved (section 8) |
| Deliverable 3 | met | 0 unrouted connections |
| Deliverable 4 | met | `output_jobs_run`, no warnings |
| Deliverable 5 | not met | `REPORT.md` was not written: a hook refused the file write. The report content is delivered here instead |

## 6. Tool gaps

1. **No autorouter, and no "route this connection" helper.**
   - I tried: manual `board_route` with explicit points, `board_query` to look for free space, and renders.
   - Missing: an automatic or interactive router, even a simple maze router for a single connection, with layer change.
   - Impact: by far the biggest cost. About 260 routing calls; three rounds of MCU pin reassignment and part moves because of planarity conflicts I could only find by reasoning about coordinates; much hand geometry (clearance arithmetic for 0.5 mm-pitch fan-outs).
2. **Library elements cannot be edited or deleted.**
   - I tried: re-running `symbol_create` with categories after the first call reported "No categories set".
   - Missing: `symbol_update`, `library_element_delete`, or an `overwrite` flag.
   - Impact: the library now contains two symbols named "LPC11U24FBD48". I had to reference the right one by UUID. A package warning (legend over pads) also cannot be fixed.
3. **Parts are missing from the installed libraries:** LPC11U24 (no NXP MCU), any 3.2×2.5 mm 4-pad crystal, and a device for the existing 142-0701-80X SMA package.
   - Workaround: built them with `symbol_create`, `component_create`, `package_create` and `device_create`, which worked well.
4. **No control of via tenting or stop mask.**
   - Impact: silkscreen near vias is flagged as "silkscreen ↔ stop mask"; I had to keep designators away from vias by trial and error.
   - Under-module vias may be untented, which is unknown and not settable.
5. **No designator auto-placement.**
   - I wrote my own loop: `board_get_part` courtyard → candidate positions → `board_query` → `board_set_part_text`. It took several passes before the DRC was clean.
   - Two designators (LED2, R4) had to be hidden.
6. **No impedance or RF helpers** (trace width calculator, or a net-specific plane gap for coplanar lines).
   - The plane clearance is per plane, so the 0.25 mm RF gap applies to every net.
7. **The schematic can only be built as net labels** (`schematic_connect`). There is no auto-layout and no way to get a "nice" schematic without hundreds of wire calls.
   - Result: a correct but label-heavy schematic. I added block titles with `schematic_add_text`.
8. **No mechanical or 3D height checks.** I could not verify that the RN2483 (3 mm) fits under the curved surface of the cell next to it, or the height of the SMA body.

## 7. Workarounds and their downsides

- **Library SMA footprint with net-less copper strips.**
  - Planes did not connect J7 pad 1/3 to PTH1, so `board_status` showed "J7.1 ↔ J7.3" unrouted. I added explicit 0.5 mm traces J7.1/J7.3 → PTH1 and J7.2/J7.4 → PTH2.
  - Downside: 16 approved clearance messages, "GND ↔ polygon". The polygons and the traces are physically the same GND copper. A clean custom package would avoid them, but because of gap 2 that would have meant another duplicate library element.
- **Edge-mount SMA pads reach the board edge.** This is by design; I approved 5 "pad ↔ board outline < 0.3 mm" messages.
- **DRC rules adapted to the brief.**
  - Annular ring 0.2 → 0.15 mm, because a 0.6/0.3 mm via is specified.
  - Outline tool diameter 2.0 → 1.0 mm, because of the 1 mm thermal slot.
  - Downside: the fab must support a 0.15 mm annular ring, which is standard at low-cost fabs.
- **Outline inner corners rounded with 0.5 mm arcs.** This silences "inner radius" and "plane ↔ outline" messages. It deviates from the brief's polygon by at most 0.2 mm, which the router would produce anyway.
- **Pin reassignment for routability:**
  - LEDs on the JTAG pins 33–35 and the ADC on AD0;
  - OW moved onto PIO0_3 so the OW/SCL/SDA corridor is planar;
  - the modem moved onto PIO0_18/19.
  - Downside: firmware has to reconfigure the JTAG pins.
- **Many layer hops (82 vias).** Inner lines cross through vias, for example the RX/TX swap in the wing and SCL/SDA to the tab on the bottom. Downside: the bottom GND plane is cut by the bundle under the cell. The top pour plus stitching vias keep the returns short.
- **The +3V3 tab "island"** (J3.1 → bottom line → U4) is tied to the main +3V3 by a separate 0.3 mm top trace from module pad 12 down to y = 0.8. Downside: a long trace, but it is only for the sensor.

## 8. Bugs or confusing tool behaviour

1. **`symbol_create` creates the element even when its own check reports an "error"**, and a second call with the same name silently creates a duplicate.
   - Call: `symbol_create {"library":"Water Node Parts","name":"LPC11U24FBD48",...}` (without categories).
   - Response: `{"messages":[{"severity":"error","message":"No categories set"...}],"uuid":"bdbc8523-..."}`. The same call with categories returned a new UUID `3a3b6e57-...`, and `library_search` then listed two "LPC11U24FBD48" symbols.
2. **`package_create` with `"legend":true` fails its own check.** It draws the silkscreen body outline across the pads.
   - Response for ABRACON_ABM8G (body 3.2×2.5, pads 1.4×1.15 at ±1.1/±0.8): 4 × "Clearance of pad 'n' in 'default' to legend" (warning).
3. **`board_route` returns more trace IDs than it created.**
   - `board_route {"from":[46.8,2.6],"to":"via:44d5…","layer":"top","points":[[38.8,2.6]]}` (a 2-segment route) returned 7 trace IDs.
   - `C8.2 → via` returned 5 IDs, including the pre-existing dangling trace `trace:d0d80a0e…`. It is unclear which IDs are new, so "undo by ID" is risky.
4. **`board_remove_items` on a via leaves the traces attached to it dangling**, without a warning. I removed `via:826cf776…`, and a 0.9 mm GND stub to its old position stayed and had to be found with `board_query`.
5. **A via lying on a trace of the same net does not connect to it.**
   - Example: `board_add_via` +3V3 at [9.0, 31.775], placed on an existing +3V3 trace. The quick check reported no problem, but `board_status` later listed `{"from":{"via":"via:d5bb…"}, "to":{"point":…,"position":[9,26.96]}}` as unrouted.
   - The same happened for the U0_RXD/U0_TXD vias placed on their own traces.
   - The tools could warn, or merge automatically.
6. **`board_modify_item` moves a via and the trace ends follow.** This is documented, but it silently turned a horizontal GND trace into a diagonal that cut across the planned slot area (via moved from [38,2.6] to [38,4.6]).
7. **Inconsistent response formats.** `board_set_part_text`, `board_add_text`, `board_set_settings` and `project_save` return plain text ("Text changed.") while most tools return JSON. My JSON-parsing script crashed on it.
8. **DRC did not flag the overlap between the RN2483 courtyard (X ≥ 31.148) and the battery courtyard (X ≤ 31.45)**, while it did flag the J4/J5 and R4/R5 courtyard overlaps. The overlap is physically harmless (the cell surface curves away), but the behaviour is inconsistent.
9. **`board_get_part` returns pads without size or rotation; `board_query` includes them.** I had to look up pad sizes with `library_get` before doing clearance calculations. Pad numbering is also easy to mix up: I assumed RN2483 pad 41 was at y = 44.375, but it is pad 40. The violation message "trace ↔ U3:40" was what revealed it.
10. **Library search noise.** `library_search {"query":"SMA","type":"package"}` returns 243 hits (it matches "SMA" inside "SMALL" and other descriptions), and the real SMA connector package has no device.
11. **Rotated text anchoring is hard to predict.** For `board_add_text` with rotation 90 and align center, I could not tell which side of the anchor the glyphs extend to. It took three DRC rounds to fit 0.8 mm titles between the header pads and the board edge.

## 9. Suggestions, ranked by impact

1. **A router:** at least a point-to-point maze router (`board_autoroute_connection` with a layer change cost) and an "autoroute net" mode that respects existing copper. This would cut the effort by 80 % or more.
2. **Planarity and fan-out hints:** for example, a `board_suggest_pin_order` or a crossing check between air-wires before routing, so pin assignment can be optimised before routing starts.
3. **Library maintenance tools:** update or delete symbols, packages, components and devices; add categories after creation; fix the `legend` geometry in `package_create`.
4. **Connectivity fixes:** auto-connect a via or trace end placed on same-net copper, or warn. When a via is removed, remove its traces or report them.
5. **A designator auto-placer**, plus via tenting and stop-mask settings in `board_set_settings`.
6. **Clear `board_route` output:** return only the IDs it created; return the same JSON shape from every tool.
7. **RF helpers:** a trace impedance calculator from the stack-up, and a per-net plane clearance or coplanar keep-out.
8. **A device for the CINCH_142-0701-80X SMA package**, with the net-less copper polygons in the footprint replaced by pads of the shield signal.
9. **Pad sizes and rotations in `board_get_part`.**
10. **Schematic layout helpers:** place blocks or auto-arrange, or generate wires instead of only labels.

## 10. Assumptions about datasheet facts (from memory, no internet)

- **LPC11U24:**
  - pinout as in the provided excerpt;
  - one USART that can be mapped to three pin pairs, as the excerpt says;
  - pin pull-ups enabled by default after reset;
  - input leakage about 10 nA max with the pull disabled;
  - GPIO drive about 4 mA;
  - USB pins can be left open when USB is unused;
  - JTAG pins usable as GPIO/AD after IOCON reconfiguration.
- **RN2483:**
  - pinout taken from the library footprint: RFH is pad 23; VDD is pads 12 and 34; !RESET is pad 32; PGC/PGD are pads 30/31;
  - supply 2.1–3.6 V;
  - about 40 mA TX peak (brief);
  - firmware update over PGC/PGD is low-voltage ICSP, so no 9 V VPP reaches the MCU pin behind R9.
- **LP5907:** 250 mA; EN can be tied to VIN; stable with about 1 µF ceramic capacitors in and out and up to about 10 µF on the output; Iq about 12 µA.
- **SSM3J338R:** P-channel, VDS −12 V, VGS ±8 V, RDS(on) about 20–30 mΩ at VGS −2.5 to −4.5 V, threshold below 1 V.
- **SSM3K329R:** N-channel, 30 V, logic level at 3.3 V gate drive.
- **ABM8G-12.000MHZ-4Y-T3:**
  - CL = 18 pF (standard load, not encoded in the MPN);
  - pad pitch 2.2 × 1.6 mm with pads about 1.4 × 1.15 mm (my land pattern);
  - pads 1/3 are the crystal and 2/4 the lid (from the brief).
- **SHT21:** library pinout (1 SDA, 2 VSS, 5 VDD, 6 SCL, exposed pad VSS); VDD 2.1–3.6 V; sleep current about 0.15 µA.
- **DS18B20:** a 4.7 kΩ pull-up suits about 1 m of cable; standby about 0.75 µA. There is no ESD/TVS protection on DQ because no suitable part was found; I recommend adding one in v1.1.
- **OSRAM LS/LY/LG R97x:** Vf about 1.8–2.0 V at 2 mA.
- **Panasonic resistors:** ERA-3AEB104V and ERA-3AEB2803V are 0603, ±0.1 %, 25 ppm/K.
- **GCPW on FR4:** εr ≈ 4.5 and 1.5 mm dielectric; W = 1.0 mm with a 0.25 mm gap gives about 50 Ω.
- **Keystone 54 clip, CNC Tech 3220-10 and Cinch 142-0701-801:** footprints and pin mappings taken from the installed LibrePCB libraries as they are.
