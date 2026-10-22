# Water Temperature Node v1.0: design report (blind test of the LibrePCB MCP tools)

Project: `projects/Water Temperature Node/Water Temperature Node.lpp`
Production data: `projects/Water Temperature Node/output/v1.0/` (Gerber/Excellon, BOM, pick&place, schematic PDF, assembly PDF, .lppz/.zip archives)
Custom library: `ws/data/libraries/local/Water_Node_Parts.lplib`, created only through the MCP tools.

## 1. Summary

What I achieved:
- A complete 2-layer board in the exact outline from the brief: 46 parts, 32 nets, 277 trace segments, 161 vias, GND planes on both layers.
- ERC: 0 messages.
- DRC: 0 open messages. 1 approved warning: "Board outline inner radius < 1.0 mm". It comes from the outline the brief prescribes, see section 5.
- Unrouted connections: 0. Unplaced parts: 0.
- Production data generated with `output_jobs_run`, with no output-job warnings.
- Parts not in the installed libraries, created with the library tools:
  - LPC11U24FBD48/401: symbol, component, device on the existing LQFP-48 package.
  - Abracon ABM8G crystal: grounded-lid symbol, component, package, device.
  - SMA 142-0701-801 device. I first used the library package. I then made my own land pattern because the library one causes 12 DRC errors (section 8).
- Silkscreen: product name "Water Temperature Node v1.0" on top (under the cell) and on the bottom side. Every header pin is labelled. Each header has a "Jx FUNCTION" label. LEDs carry the colour letters R/Y/G. "SWD" and "ANT" labels.

What I did not achieve, or only partly:
- The RF path is about 19 mm long, not "short". The SMA ground legs sit on both sides of the centre pin and touch the board edge, so the trace has to approach the centre pad from the -X side and walk around the upper ground leg (D3, partly met).
- The 50 Ω line is a hand-estimated grounded coplanar waveguide (1.0 mm trace, 0.2 mm gap, 1.6 mm FR4). The tools have no impedance calculator or field solver.
- The LPC11U24 has only one USART, which talks to the modem. UART1 (F3) is wired to two GPIOs and needs a software UART in firmware.
- The module decoupling caps (C10, C11) sit at the top edge of the RN2483, not next to its VDD pads. There is no free top-side area next to those pads: the battery is on one side and the board edge on the other.
- The schematic is functional but has only one page with net labels. Readability is mediocre: the tools offer no auto-tidy and no way to move or delete schematic texts.

## 2. Design decisions per block

### A. Power
- **A1 Battery:** BT1 is the Keystone 54 clip pair from the library ("18650 65mm (unprotected)" footprint). It is placed at (22.05, 38.2) with rotation 0, so the + clip is at +Y as required. The three + pads are tied together on top with 0.8 mm traces. The three - pads are joined by the GND planes.
- **A2 Reverse protection:** Q1 is a Toshiba SSM3J338R P-MOSFET (-12 V, VGS ±8 V) in the high side.
  - Connections: drain to the battery + (net BAT_RAW), source to VBAT, gate to GND.
  - Normal insertion: the body diode conducts first, then VGS = -VBAT (-2.8 to -4.2 V) fully enhances the channel. RDS(on) is about 30 to 50 mΩ (assumed), so the drop at the 40 mA TX peak is about 2 mV.
  - Reversed cell: VGS ≈ 0 V and the body diode is reverse-biased, so VBAT stays at 0 V. Only Q1's drain and the clip see -4.2 V. |VDS| and |VGS| stay below the ratings.
- **A3 Regulator:** U2 is the LP5907QMFX-3.3Q1. EN is tied to IN, so the node is always on.
  - Input: C2 1 µF directly at the pin, plus C1 10 µF 0805 as a VBAT bulk cap for the TX peaks and the cell/clip impedance.
  - Output: C3 1 µF at the pin. All 3V3 capacitors add up to about 7.4 µF nominal, less after DC-bias derating. That keeps the output inside the stable range I assumed for the LP5907 (≤ 10 µF).
  - Below about 3.35 V the LDO drops out and 3V3 follows VBAT. The MCU (≥ 1.8 V), the RN2483 and the SHT21 (≥ 2.1 V) keep working down to the 2.8 V end-of-discharge.
- **Power traces:** BAT_RAW 0.8 mm; VBAT 0.4 to 0.5 mm; 3V3 trunks 0.3 to 0.4 mm. The 40 mA TX peak causes only millivolt drops.
- **Sleep budget (assumed):** LDO Iq 12 µA, MCU deep power-down below 1 µA, RN2483 sleep about 1.3 µA, divider off below 1 µA. Total about 15 µA. With TX bursts (40 mA × 1.5 s every 5 min, about 0.2 mA average) one 2500 mAh cell lasts more than a year.

### B. Battery voltage measurement
- **Topology:** a high-side switched divider. The P half of Q2 (DMC2053UVT, a complementary 20 V N+P pair in SOT-23-6) connects VBAT to the divider. The N half, driven by MCU pin BAT_MEAS_EN (PIO1_22), pulls the P gate low.
  - R1 = 100 kΩ (VBAT to P gate) keeps the P-FET off.
  - R2 = 100 kΩ pulls the N gate down, so the divider stays off while the MCU is in reset.
  - The circuit sits downstream of Q1, so a reversed cell cannot reach it.
- **Divider:** R3 = R4 = 100 kΩ, 0.1 %, 25 ppm/K (Panasonic ERA-3AEB104V), ratio 0.5. VBAT 2.8 to 4.2 V gives an ADC input of 1.40 to 2.10 V, inside 0 to 3.3 V even when VDD sags in dropout.
- **B3 accuracy:**
  - Worst-case ratio error with two 0.1 % resistors: (1 - 0.999/1.001)·0.5 ≈ 0.1 %. Tracking TCR adds less than 0.01 % over 0 to 40 °C.
  - Q2 RDS(on) (below 1 Ω) in series with 200 kΩ adds less than 0.001 %.
  - ADC input leakage (≤ 10 nA assumed) into 50 kΩ Thevenin gives 0.5 mV, about 0.025 %.
  - Total: about 0.13 % worst case, which meets the 0.2 % requirement.
- **C4:** 100 nF at the ADC pin makes the sample-and-hold charge kickback negligible (C_sample/C4 is about 5e-5). τ = 50 kΩ × 100 nF = 5 ms, so firmware waits at least 50 ms (10 τ) after enabling, then samples.
- **B2 off current:** IDSS leakage of the two off MOSFETs only, typically nA (datasheet maximum 1 µA, assumed). On current during the measurement: 42 µA in R1 plus 21 µA in the divider.
- **Note:** the ADC reference is VDD (3V3 from the LDO, about ±1 to 2 %). The divider ratio meets B3, but the absolute VBAT reading is only as good as VDD unless it is calibrated (e.g. once in production against a reference).

### C. Microcontroller
- **C1:** U1 is the LPC11U24FBD48/401, using my own symbol and component on the library LQFP50P900X900X140-48 package.
- **C2:** Y1 is the ABM8G-12.000MHZ-4Y-T3 (own 4-pad footprint, pads 1.2 × 1.0 mm at ±1.1/±0.8 mm, lid pads 2 and 4 to GND).
  - Load caps C8 = C9 = 27 pF, from C = 2·(CL - Cstray) with an assumed CL = 18 pF and Cstray ≈ 3 to 4 pF, giving 28 to 30 pF.
  - **Verify CL for the "-4Y" suffix:** I could not check it. If CL = 10 pF, change C8/C9 to 12 to 15 pF.
- **C3 decoupling:**
  - VDD pin 8: C5 100 nF, 1.5 mm from the pin, with its own GND via.
  - VDD pin 44: C6 100 nF directly at the pin, plus C7 1 µF bulk next to it.
  - VSS pins 5 and 41 have short traces to GND vias and caps.
- **Reset:** R5 = 10 kΩ pull-up on RESET, near the SWD header. PIO0_1 (the ISP entry pin) is left open with its internal pull-up.
- **C4 SWD:** J6 is the CNC Tech 3220-10-0300-00 (2×5, 1.27 mm SMD) with the standard Cortex Debug pinout: 1 = 3V3, 2 = SWDIO, 3/5/9 = GND, 4 = SWCLK, 10 = nRESET, 6/7/8 open.
- **C5:** USB_DM/USB_DP are unconnected.

### D. LoRaWAN modem
- **D1:** U3 is the RN2483A-I/RM103 (library device).
- **D2 UART:** module UART_TX goes to MCU PIO0_18 (RXD). MCU PIO0_19 (TXD) goes through R6 = 1 kΩ to module UART_RX. R6 lets an external adapter on J3/J5 drive the module RX without fighting the MCU and limits contention current. RTS/CTS are not connected.
- **Module reset:**
  - Q3 (SSM3K329R N-FET, 30 V) pulls !RESET low when MCU PIO0_6 goes high. R7 = 100 kΩ gate pull-down keeps the module running while the MCU is in reset. R8 = 10 kΩ pull-up to 3V3.
  - The open-drain FET isolates the MCU pin from a PIC programmer that may drive MCLR above VDD during ICSP.
- **D3 RF path:** RFH goes to the J7 SMA centre pad.
  - The trace is 1.0 mm wide on top with 0.2 mm gap to a local high-priority top GND plane, over a solid bottom GND plane. That is a grounded coplanar waveguide, estimated 52 to 56 Ω (hand calculation from the CPW formula; no solver available).
  - There is a GND via fence along the trace (9 vias at about 1.4 mm from the centre line).
  - The pad exits straight down at 1.0 mm width (narrower than the 1.27 mm pad pitch allows for wider).
  - Length about 19 mm, roughly 0.1 λ_eff at 868 MHz.
- **SMA:** J7 sits on edge X = 39 with its centre at Y = 17 and points +X. I used my own footprint CINCH_142-0701-801_PULLBACK: the library pads pulled back 0.35 mm from the edge and without the library's net-less copper strips.
- **Decoupling:** C10 4.7 µF and C11 100 nF at the module's top edge. The 3V3 path from them to VDD pads 12 and 34 is 0.4 mm wide, through vias under the module edge. The RN2483 has internal decoupling (assumed).
- **D4:** J5 "Modem service" header. RESET, PGC and PGD are routed from the module's right-side pads under the module (bottom layer) to the left wing.

### E. Sensors
- **E1 SHT21:** U4 sits at the far right of the tab, at (52.8, 4.0). Thermal decoupling:
  - a 2.0 × 4.4 mm milled slot (cutout X 40 to 42, Y 2.6 to 7.0) separates the tab from the main body, leaving only a 2.6 mm bottom bridge and a 1.0 mm top bridge;
  - noPlanes keep-outs on the whole tab on both layers, so there is no copper pour;
  - only four thin traces (0.2 to 0.25 mm) cross the bridges: SCL, SDA and 3V3 on the bottom bridge, GND on the top bridge;
  - C12 100 nF sits next to VCC.
- **E2 Water probe:** J1 (1×3 THT) is at X = 2.6, Y 19 to 24 in the lower half of the left wing. The cable leaves to the left. Pinout: 1 = GND, 2 = DQ, 3 = 3V3. R11 = 4.7 kΩ pull-up (standard for DS18B20 at 3.3 V with a 1 m cable, no parasite power). C13 = 100 nF across the probe supply at the header. DQ goes to PIO1_23 (bit-banged 1-Wire).
- **E3 Expansion:** J2 is the I²C header (3V3, SDA, SCL, GND). Pull-ups R9/R10 = 4.7 kΩ to 3V3. Bus capacitance is about 15 pF of traces plus the devices; with up to about 70 pF in total the rise time is about 280 ns, which is fine at 100 kHz and borderline at 400 kHz. Recommendation: 100 kHz with external sensors.

### F. User interface and development
- **F1 LEDs:** red LS R976-NR-1, yellow LY R976-PS-36, green LG R971-KN-1, each driven active-high from a GPIO through 680 Ω. The current is (3.3 V - about 1.9 to 2.0 V)/680 Ω ≈ 1.9 to 2.1 mA, "a few mA". GPIOs: red PIO1_13, yellow PIO1_29, green PIO0_22.
- **F2:** J3 "UART0" pin 1 is RX (= module TX, net MODEM_TX); pin 2 is TX (= module RX, net MODEM_RX, after R6).
- **F3:** J4 "UART1" pin 1 is RX on PIO0_3, pin 2 is TX on PIO1_20. The LPC11U24 has a single USART, used by the modem, so UART1 needs a timer-based software UART.

## 3. MCU pin assignment (LPC11U24FBD48, LQFP-48)

| Pin | Function used | Net | Remark |
|----:|---------------|-----|--------|
| 3 | RESET | MCU_RESET | SWD pin 10, 10 kΩ pull-up R5 |
| 5, 41 | VSS | GND | |
| 6 | XTALIN | XTAL_IN | Y1 pad 1, C8 27 pF |
| 7 | XTALOUT | XTAL_OUT | Y1 pad 3, C9 27 pF |
| 8 | VDD | 3V3 | C5 100 nF |
| 44 | VDD | 3V3 | C6 100 nF + C7 1 µF |
| 13 | PIO1_20 | DBG_TX | UART1 TX (soft-UART) |
| 14 | PIO0_3 | DBG_RX | UART1 RX (soft-UART) |
| 15 | PIO0_4 / SCL | I2C_SCL | true open-drain I²C pin |
| 16 | PIO0_5 / SDA | I2C_SDA | true open-drain I²C pin |
| 18 | PIO1_23 | ONEWIRE_DQ | DS18B20 1-Wire |
| 22 | PIO0_6 | MODEM_RST_DRV | high = RN2483 held in reset (via Q3) |
| 29 | SWCLK | SWCLK | SWD pin 4 |
| 30 | PIO0_22 | LED_GRN | |
| 31 | PIO1_29 | LED_YEL | |
| 36 | PIO1_13 | LED_RED | |
| 38 | PIO1_22 | BAT_MEAS_EN | high = divider on |
| 39 | SWDIO | SWDIO | SWD pin 2 |
| 42 | PIO0_23 / AD7 | VBAT_ADC | |
| 46 | PIO0_18 / RXD | MODEM_TX | USART RX ← RN2483 UART_TX |
| 47 | PIO0_19 / TXD | MCU_TXD | USART TX → R6 1 kΩ → RN2483 UART_RX |
| 19, 20 | USB_DM/DP | n.c. | USB unused (C5) |
| others | | n.c. | Firmware should set them to output-low or enable pull-ups for minimum sleep current. Do not use PIO0_16 (WAKEUP) for loads that pull it low. |

Why I chose these pins: I made several placement and routing passes. The final choice minimises crossings for the MCU's fixed position:
- UART to the modem on the top-left pins, leaving towards the module's left side.
- The I²C/1-Wire/debug bus on the bottom pins.
- LEDs and SWCLK on the right side.
- Battery measurement and SWDIO on the top side.
- AD7 instead of AD5, because PIO0_16 is the WAKEUP pin and must stay high in deep power-down.

## 4. Placement and routing approach

**Placement** follows the fixed mechanics:
- Battery clips in the middle (X 13 to 31).
- RN2483 in the right column directly above the notch, so its RF pad (23) is as close to the SMA as the outline allows.
- MCU above the module, with the crystal and its caps on the MCU's left (towards the cell).
- Battery measurement block and SWD header in the top band right of H1.
- Power path (Q1, C1, C2, U2, C3) and the LEDs in the top band left of H1, next to the + clip.
- All headers in the left wing at X = 2.6, from top to bottom: J5 service, J3 UART0, J4 UART1, J2 I²C, J1 probe (lower half). Pin labels at X 4.2, function labels vertical at X 7.9.
- Pull-ups for I²C and 1-Wire next to their headers.
- SHT21 alone at the far end of the tab.
- No components inside X 13 to 31, Y 3 to 73.

**Routing:** fully manual through `board_route`, one segment list at a time, with `board_query` to look for free space before each step. There is no autorouter. Layer strategy:
- Top: component-local routing; the RF trace; the modem UART (two parallel verticals along the cell edge at X 29.9/30.6); vertical runs under the cell.
- Bottom: long horizontal runs under the cell and the module to the left wing and the tab; the 3V3 distribution; LED nets (along the top edge under the SWD header).
- Under the cell I used a Manhattan scheme: bottom horizontals, top verticals, a via at every bend. This avoided crossings among the 15 nets that go to the left wing.
- About 70 signal vias and about 90 GND stitching or fence vias. Vias are 0.6 mm pad / 0.3 mm drill throughout.
- GND planes on both layers (clearance 0.3 mm), plus a top GND plane with 0.2 mm clearance around the RF line (priority 1).
- noPlanes keep-outs on the tab and at three concave outline corners.
- noCopper/noPlanes keep-outs with radius 3.0 mm around the M3 holes, so a metal screw head or standoff cannot touch traces.

Design-rule settings changed from the defaults:
- Default trace width 0.25 mm.
- DRC minimum annular ring lowered from 0.2 mm to 0.15 mm, to permit the 0.6 mm / 0.3 mm vias the brief allows.
- Net classes: "Power" (BAT_RAW, VBAT, 3V3; default width 0.5 mm) and "RF" (RF_ANT; 1.2 mm default, routed at 1.0 mm).

## 5. Requirement compliance

| ID | Status | Reason / evidence |
|----|--------|-------------------|
| A1 | met | BT1 Keystone 54 pair, 18650 footprint, centre (22.05, 38.2), + towards +Y |
| A2 | met | P-MOSFET Q1 high-side ideal-diode connection; no series diode; drop about 2 mV at 40 mA |
| A3 | met | LP5907QMFX-3.3Q1, EN tied to VIN, no switch |
| B1 | met | 1:2 divider into AD7 (PIO0_23), 1.4 to 2.1 V |
| B2 | met (by datasheet typ.) | Switched by the P/N pair, off current = MOSFET leakage (nA typical; datasheet maximum about 1 µA, assumed) |
| B3 | met | 0.1 % / 25 ppm resistors give a ratio error of ±0.1 %; switch and leakage errors below 0.03 %. The absolute reading still depends on VDD (calibrate). |
| C1 | met | LPC11U24FBD48/401 with its own symbol/component/device |
| C2 | met (value to verify) | ABM8G 12 MHz, own footprint, lid to GND, 27 pF load caps (CL = 18 pF assumed) |
| C3 | met | 100 nF at each VDD pin, 1 µF bulk, VSS pins via-connected |
| C4 | met | CNC 3220-10-0300-00, standard 10-pin Cortex Debug, nRESET wired |
| C5 | met | USB pins open |
| D1 | met | RN2483A-I/RM103, RFH used, RFL open |
| D2 | met | USART PIO0_18/19, 57600 baud is a firmware setting, no flow control |
| D3 | partly | SMA 142-0701-801 at X = 39, Y = 17, pointing +X; GCPW about 50 Ω (estimated) with via fence. Path about 19 mm (geometry-limited), impedance not verified by a solver. |
| D4 | met | J5 1×7: RESET, UART_RX, UART_TX, PGC, PGD, 3V3, GND |
| E1 | met | SHT21 at the far end of the tab, milled slot, no planes on the tab, thin traces only |
| E2 | met | J1 1×3 (GND, DQ, 3V3), 4.7 kΩ pull-up, lower half of the left wing, cable to the left |
| E3 | met | J2 1×4 (3V3, SDA, SCL, GND), 4.7 kΩ pull-ups on board |
| F1 | met | 3 LEDs, own GPIOs, about 2 mA each, 0805 parts as specified |
| F2 | met | J3 pin 1 = MODEM_TX (MCU RX), pin 2 = MODEM_RX (MCU TX) |
| F3 | partly | J4 wired (pin 1 RX, pin 2 TX) but to plain GPIOs; needs a software UART because the single USART serves the modem |
| M1 | met | NPTH Ø3.2 mm at (34.5, 82.5) and (34.5, 7.5), plus 3 mm copper keep-outs |
| M2 | met | Cell footprint at the given position, nothing else inside X 13 to 31, Y 3 to 73 (only traces, vias and silkscreen under the cell) |
| M3 | met | J7 origin on X = 39, Y = 17, rotation 270 (points +X into the notch) |
| M4 | met | U4 at (52.8, 4.0), far end of the tab |
| M5 | met | J1 at X 2.6, Y 19 to 24 |
| M6 | met | All components on top; the bottom has only copper, vias and silkscreen |
| PCB: 2 layers, 1.6 mm, 35 µm | met | Board setup: 0 inner layers, thickness 1.6 mm |
| PCB: min 0.2 mm trace/space, via 0.6/0.3 mm | met | Narrowest trace 0.2 mm; DRC clearance 0.2 mm; all vias 0.6/0.3 mm (annular-ring rule lowered to 0.15 mm to match) |
| PCB: power traces for 40 mA | met | VBAT 0.4 to 0.5 mm, 3V3 0.3 to 0.4 mm, BAT_RAW 0.8 mm |
| PCB: ground planes | met | GND on both layers, stitched; local RF ground |
| PCB: silkscreen name/version + header labels | met | "Water Temperature Node v1.0" top (under the cell, visible with the cell removed) and bottom; every header pin labelled |
| Deliverable 1 | met | Project with schematic and board in `projects/` |
| Deliverable 2 | met | ERC 0; DRC 0 open, 1 approved warning (the outline inner radius below 1 mm comes from the outline polygon the brief prescribes; the mill rounds the three concave corners, which is harmless) |
| Deliverable 3 | met | `board_status`: 0 unrouted connections |
| Deliverable 4 | met | Gerber/Excellon, BOM, pick&place, PDFs in `output/v1.0` |
| Deliverable 5 | met | This file |

## 6. Tool gaps (needed but not available)

1. **No routing assistance at all.**
   - Tried: `board_route` with hand-computed corner lists, plus `board_query` for free-space checks.
   - Missing: an autorouter, or at least "route this air wire on layer X avoiding obstacles" with automatic via insertion; push-and-shove.
   - Impact: the largest effort by far. About 1,300 tool calls in total (223 `board_route`, 183 `board_add_via`, 365 `board_query`, partly automated stitching checks). Pin assignment and part positions had to be changed four times because of crossings I only found while routing.
2. **No way to edit or delete library elements.**
   - Tried: calling `symbol_create` again with the same name to add the forgotten category. That silently created a second symbol, so the local library now holds two "LPC11U24FBD48" symbols.
   - Missing: `*_update` / `*_delete` for symbols, packages, components and devices.
   - Impact: library clutter. The broken library SMA footprint could not be fixed, only replaced by a new element with a different name.
3. **No device swap for a schematic part.**
   - Tried: `board_place_device` with a new `device` swapped the footprint on the board.
   - Missing: the BOM then showed an empty MPN for J7, although `part_get` still reported the MPN. `part_set_properties` did not help.
   - Workaround: `part_remove` + `schematic_add_part` + reconnect + re-route.
4. **No editing of schematic texts.** Only `schematic_add_text` exists, so a misplaced title had to be removed with `undo`. There is also no schematic auto-tidy, so net-label overlaps remain.
5. **No RF/impedance helper.** The trace/gap for 50 Ω are a hand estimate.
6. **No keep-out option on `board_add_hole`.** I drew 24-vertex circular keep-outs by hand around the M3 holes.
7. **No via tenting / solder-mask control.** Vias under the RN2483 (VDD, RESET, PGC, PGD) and under the cell stay unmasked. The risk is low but real.
8. **No filters on `board_remove_items`.** `net:GND` removes every GND trace and via on the board with no layer/area filter. I removed the CPU-area GND routing by accident and recovered it with `undo`.
9. **Off-centre bodies are impossible with `package_create`.** The body is always centred on the origin, so the SMA housing that overhangs the edge cannot be described. The courtyard only covers the pads.
10. **No per-net length report.** I estimated the RF length by hand.
11. **No silkscreen auto-placement.** 24 designator/pad overlaps had to be fixed by hiding or moving names one at a time.

## 7. Workarounds and their downsides

- **Own SMA footprint** (pads pulled back 0.35 mm from the edge, no net-less strips, PTH1/PTH2 kept as GND stitch holes). Downside: it deviates slightly from the vendor land pattern; the solder joints on the centre pin and ground legs are 0.3 mm shorter.
- **Via hops and the Manhattan scheme under the cell.** Downside: many vias; the bottom GND plane is fragmented under the cell and in the left wing. Acceptable for slow signals, and the RF ground stays intact.
- **noPlanes keep-outs at the concave corners** (13, 76), (13, 14) and (39, 8), to work around the plane-fill corner violation. Downside: small areas without copper.
- **Remove and re-add J7** to get a correct BOM line. Downside: the schematic symbol position changed.
- **`undo`** to remove a mis-placed schematic text and to recover accidentally deleted GND items.
- **Hidden values and most passive designators on the silkscreen.** Downside: hand assembly needs the assembly PDF.
- **Product name under the cell (top) and on the bottom side**, because there is no free visible top area. Downside: the top copy is only visible with the cell removed.
- **Module decoupling caps at the module's top edge.** Downside: about 15 to 20 mm of trace to the VDD pads.

## 8. Bugs or confusing tool behaviour

1. **`board_route` fails when both ends are positions, even exactly on via centres.**
   - Call: `board_route {"from":[43.7,74.2],"to":[37.9,80.8],"layer":"bottom","width":0.25,"points":[]}`, where both points are via centres of the same net.
   - Response: `TOOL ERROR: At least one end must be a pad, via or junction.`
   - It works with `"from":"via:<id>"`. The same happens for `{"from":[50.1,63.6],"to":[47.2,81.5],...}` and `{"from":[8.7,37.3],"to":[8.7,36.17],...}`.
2. **A trace passing over a same-net via is not connected to it, and the fix is inconsistent.**
   - After `board_route {"from":"R6.2","to":[30.6,47.6],"points":[[30.6,69.95]]}`, `board_status` still listed the via at (30.6, 67.6) as unrouted against the trace.
   - Fix attempt 1: `board_route {"from":"via:19ffcf0b...","to":[30.6,67.6],"layer":"top"}` returned `{"traces":[],"violations":[]}` and silently created the junction.
   - Fix attempt 2, an identical case: `board_route {"from":"via:a86234fe...","to":[29.9,51.17],"layer":"top"}` returned `TOOL ERROR: BI_NetLine: both endpoints are the same.` That internal message leaks to the caller and gives no hint how to proceed. I had to delete the long trace and re-route it in two pieces.
3. **Unstable trace IDs and bloated responses.**
   - `board_route` responses list dozens of IDs: 46 for the single segment `via:1b9b...` → `via:1f48...`, 63 for a 3V3 tab segment. They appear to include every regenerated trace of the net.
   - IDs of untouched traces change after unrelated edits: the bottom 3V3 segment (46.54, 86) → (35.4, 86) was `trace:034428e8...`, later `trace:216a3621...`, then `trace:9f79015b...`.
   - Consequence: stored IDs are useless; you must re-list every time.
4. **Removing a via leaves its traces dangling.** After `board_remove_items {"ids":["via:85b81c20..."]}` the traces `c325e4c6...`/`cf9fcc41...` remained and later showed up as `Clearance on 'Top Copper': '3V3' trace ↔ 'U1:12' < 0.2 mm`.
5. **Plane fill violates the board-outline clearance at concave corners.** With a whole-board plane at clearance 0.3, DRC reported `Clearance plane ↔ board outline < 0.3 mm` at (13.09, 75.72) and (13.28, 14.09), and after a keep-out change at (38.71, 7.94). The fill is rounded but evidently 0.3 mm or less from the sharp corner.
6. **Library package CINCH_142-0701-80X has net-less copper strips** joining PTH1/PTH2 to pads 1/2. This gives 8 "pad ↔ polygon < 0.2 mm" errors plus 5 "plane ↔ polygon" errors once planes exist. Its pads also run to the board edge (5 "pad ↔ board outline < 0.3 mm" errors).
7. **BOM loses the MPN after a device swap.** After `board_place_device {"designator":"J7","device":"Cinch 142-0701-801 (edge pullback)",...}`, `part_get` shows `"mpn":"142-0701-801"` but the BOM line is `1,J7,CINCH_142-0701-801_PULLBACK,SMA,,`. Still empty after `part_set_properties {"designator":"J7","part":{"mpn":"142-0701-801",...}}` and re-running the output jobs.
8. **`symbol_create` with an existing name silently creates a duplicate** (new UUID, same name). `library_search` then shows two "LPC11U24FBD48".
9. **`package_create` with `"legend":true` draws the body outline through pads** when the pads extend beyond the body (ABM8G: 4× "Clearance of pad … to legend" warnings). There is no option to draw the legend around the pads.
10. **`board_query` "free" with `"net":"GND"` ignores same-net THT pads.** That is documented, but their drills still conflict: two stitching vias placed by "free" result ended up 0.3 mm from the battery-clip drills, giving `Clearance drill ↔ drill < 0.35 mm` and an isolated via. `planes` in the result lists plane polygons, not the actual fill, which is misleading for stitching.
11. **Default DRC minimum annular ring (0.2 mm) contradicts a 0.6/0.3 mm via** (0.15 mm ring). I had to change the DRC minimum; nothing points this out when you pass `size:0.6`.
12. **Keystone 54 library courtyard (X 12.65 to 31.45) extends beyond the board outline** at X < 13 for Y < 14. DRC is silent about it (this may be intended).

## 9. Suggestions to improve the tools (ranked by impact)

1. **Assisted routing:** a per-connection maze router ("route air wire N on top/bottom, insert vias as needed, respect clearances"), or at least "find a free path" in `board_query`. That would remove most of the effort and the iteration on placement and pinout.
2. **Library maintenance tools:** update and delete for symbols, packages, components and devices (rename, categories, pad edits), plus a "fork from library element" to fix broken library footprints such as CINCH_142-0701-80X.
3. **Robust connectivity semantics in `board_route`:**
   - accept positions on vias at both ends;
   - automatically join same-net vias and pads a new trace passes over (or offer `board_connect_junction`);
   - never surface internal messages such as "BI_NetLine".
4. **Stable item IDs, and `board_route` returning only the newly created traces.**
5. **A device/part swap for schematic parts that keeps the MPN and BOM consistent.**
6. **Fix the plane-fill clearance at concave outline corners.**
7. **Schematic editing:** move and delete texts, auto-arrange symbols and labels, multi-page helpers.
8. **`board_add_hole` with an optional copper keep-out radius; via tenting options.**
9. **RF helpers:** a microstrip/CPW impedance calculator and per-net length reporting.
10. **Filters for `board_remove_items`** (layer, area, type) and an "affected items" preview.
11. **Silkscreen auto-placement** for designators, and a DRC hint that the via size conflicts with the annular-ring rule.

Things that worked well: `board_query` (free space per layer, items in an area), DRC/ERC messages with exact locations, `render_board` with layer selection (essential to "see" the board), `undo`, `board_status` air-wire listing, `net_class_set`, high-priority local planes and keep-outs, and `output_jobs_run`, which created complete production data in one call.

## 10. Assumptions about datasheet facts (from memory, not verified; no internet)

- **LPC11U24:** 1.8 to 3.6 V; one USART (remappable to PIO0_18/19, PIO1_13/14, PIO1_26/27 as given in the excerpt); ADC reference = VDD; RESET has a weak internal pull-up; GPIO 4 mA drive is fine for 2 mA LEDs; PIO0_16 = WAKEUP must be high before deep power-down; pin input leakage ≤ 10 nA.
- **ABM8G-12.000MHZ-4Y-T3:** CL = 18 pF (the "4Y" code was not decoded); land pattern 1.2 × 1.0 mm pads at 2.2 × 1.6 mm pitch; pad 1 bottom-left, counter-clockwise numbering; pads 2/4 = lid.
- **LP5907-Q1:** VIN ≤ 5.5 V, Iq about 12 µA, dropout about 120 mV at 250 mA (much less at 40 mA), stable with 1 µF ceramic input/output and up to about 10 µF output.
- **SSM3J338R:** P-ch, VDS -12 V, VGS ±8 V, VGS(th) about -0.3 to -1.0 V, RDS(on) of tens of mΩ at VGS = -2.5 V. Pinout as in the library device (1 G, 2 S, 3 D).
- **SSM3K329R:** N-ch 30 V, logic-level gate.
- **DMC2053UVT:** complementary 20 V N+P pair, |VGS(th)| ≤ 1 V. Pin map as in the library (1 GN, 2 SP, 3 GP, 4 DP, 5 SN, 6 DN). IDSS ≤ 1 µA maximum, nA typical.
- **Panasonic ERA-3AEB104V:** 100 kΩ, 0.1 %, 25 ppm/K, 0603.
- **RN2483:** 2.1 to 3.6 V; TX current about 40 mA at +14 dBm; RFH = 868 MHz output (pad 23) needs a 50 Ω path; !RESET active low; PGC/PGD for the PIC ICSP; internal decoupling present; sleep about 1.3 µA. Pad map from the library device.
- **SHT21:** 2.1 to 3.6 V; I²C address 0x40; needs 100 nF decoupling; exposed pad to GND; NC pins left open; self-heating negligible at low duty cycle.
- **DS18B20 probe:** 3-wire (external supply), 4.7 kΩ pull-up suitable for 1 m of cable at 3.3 V.
- **OSRAM 0805 LEDs:** VF about 1.8 to 2.1 V at about 2 mA.
- **Cinch 142-0701-801:** SMA edge mount for 0.062" boards. The pad geometry taken from the library package (CINCH_142-0701-80X) is correct apart from the strips.
- **Keystone 54 + 18650:** the cell can is the negative terminal and is insulated by its sleeve, so traces and vias under the cell are acceptable.
- **FR4 εr ≈ 4.5** for the impedance estimate.
