**The board is done and clean, but `BLIND/REPORT.md` was not written.** The Write tool refused it ("Subagents should return findings as text, not write report files"). I did not work around that refusal, so the full report is below; please save it as `BLIND/REPORT.md` (sections 1–10 as the brief requires).

**Status:**
- **ERC:** 0 messages.
- **DRC:** 0 open, 21 approved, all explained in section 5:
  - 14 come from net-less copper strips in the library SMA footprint.
  - 5 are the edge-mount SMA pads touching the board edge, which is intended.
  - 1 is the sharp inner corners of the brief's outline.
  - 1 is a harmless junction via.
- **Unrouted:** 0. Unplaced parts: 0.
- **Production data:** output jobs ran, 18 files in `projects/WaterTemperatureNode/output/v1.0/` (Gerber/Excellon, BOM, pick&place, PDFs, archives).
- **Project:** `BLIND/projects/WaterTemperatureNode/WaterTemperatureNode.lpp`. My own parts are in the local library "Water Node Parts".

**Main gap:** F3 is only partly met. The LPC11U24 has one USART, so UART1 sits on the alternate USART pins and firmware must time-share the USART or bit-bang a debug console. Everything else is met; D3 has a caveat (RF line about 18 mm with 4 bends).

---

# Water Temperature Node v1.0 – Design Report (blind tool evaluation)

## 1. Summary

Achieved:

- A 4-page schematic (power, MCU, radio, sensors/IO) with 46 parts. ERC is clean: 0 messages.
- A 2-layer board with the fixed outline, the M3 holes, the 18650 clips, the edge SMA in the notch and the SHT21 on the tab. The tab is thermally isolated by an internal 2 mm slot and has no planes.
- Everything is routed: `board_status` reports 0 unrouted connections and 0 unplaced parts.
- Top and bottom GND planes. A separate high-priority top GND plane with 0.2 mm clearance turns the RF line into a grounded coplanar waveguide (1.2 mm trace, 0.2 mm gap, about 50 Ω).
- DRC: 0 open messages and 21 approved ones (see section 5).
- Silkscreen carries "Water Temperature Node v1.0", a label for every header pin, the header titles (PROBE, MODEM, UART0, I2C, SWD) and designators. All of it is DRC-clean.
- Production data was generated with the default output jobs: 18 files.

Not achieved / compromises:

- **F3 (UART1):** the LPC11U24 has only one USART. UART1 is wired to the alternate USART pins PIO0_18/PIO0_19. Firmware must either re-route the USART at runtime (IOCON) or bit-bang the debug console.
- **RF path length** is about 18 mm with four 45° bends. The SMA position is fixed and the module's RF pad faces the notch corner, so the trace has to go around the SMA's ground pad and stitching holes. The impedance is calculated, not field-solved.
- **ISP pull-up R7 removed** to free routing space. PIO0_1 relies on its internal pull-up, which the datasheet says is enabled by default.
- **Vias inside footprints:** some signals use vias placed inside the LQFP and module footprints (tented area under the body). Acceptable for prototypes but not ideal.

## 2. Design decisions per block

### A – Power

- **BT1:** Keystone 54 clip pair, 18650 65 mm footprint, at (22.05, 38.2) with + towards +Y.
- **A2 – reverse protection:** Q1 = SSM3J338R P-MOSFET in the high side.
  - Connections: drain to the battery +, source to VBAT, gate to GND through R1 = 100 kΩ.
  - Correct insertion: the body diode conducts first, then Vgs = −2.8…−4.2 V enhances the FET. Assumed Rds(on) is about 40–60 mΩ, so the drop at 40 mA TX peaks is about 2 mV. There is no series diode.
  - Reversed cell: Vgs ≈ 0 and the body diode is reverse biased, so the board sees no voltage.
  - Vgs never exceeds the assumed ±8 V rating, so no Zener is needed.
- **A3 – LDO:** U2 = LP5907QMFX-3.3Q1 with EN tied to IN, so the node runs whenever a cell is inserted.
  - Capacitors: C2 = 1 µF at the input, C3 = 1 µF at the output, C1 = 4.7 µF bulk on VBAT for the TX bursts.
  - Total capacitance on 3V3 is 6.2 µF, inside the assumed stable range of 1–10 µF.
  - Iq is about 12 µA, which suits multi-month operation.
- **Trace widths:** VBAT and 3V3 trunks are 0.4–0.5 mm; branches are ≥0.25 mm. That is plenty for 40 mA peaks.

### B – Battery voltage measurement

- **Switch:** Q2 (SSM3J338R, P-channel) has its source on VBAT and its drain on VBAT_SW.
  - R2 = 100 kΩ from gate to source keeps Q2 off.
  - Q3 (SSM3K329R, N-channel) pulls the gate low when MEAS_EN (PIO0_23) is high. R3 = 100 kΩ pulls MEAS_EN down, so the switch is off during reset.
- **B2 – idle current:** when off, no current flows through R2 (gate = source). Only the FET leakage remains, which is ≪1 µA. When on, the circuit draws about 250 µA, but only for about 1 ms per measurement.
- **Divider:** R4 = R5 = 10 kΩ, 0.1 %, 25 ppm/K (Panasonic ERA-3AEB103V), ratio 0.5.
  - 4.2 V becomes 2.1 V and 2.8 V becomes 1.4 V. Both stay below VDD even when the LDO is in dropout.
  - **B3:** worst-case ratio error is ½·(0.1 % + 0.1 %) = 0.1 %. Q2's Rds(on) adds about 2.5 ppm. The requirement of ≤0.2 % is met.
  - No filter capacitor was needed: the 5 kΩ source impedance gives a fast settle.
- **ADC input:** VBAT_ADC goes to PIO0_16 / AD5.
- **Note:** the ADC reference is VDD, so the absolute accuracy also depends on the LDO tolerance.

### C – Microcontroller

- **C1:** no library part existed, so I created a symbol, component and device for the LPC11U24FBD48/401. They sit on the official `LQFP50P900X900X140-48` package; VDD and VSS each map to two pads.
- **C2 – crystal:** ABM8G-12.000MHZ-4Y-T3 on a custom footprint (pads 1.3 × 1.1 mm). Pads 1/3 are the crystal, pads 2/4 are GND.
  - Load capacitors C7 = C8 = 27 pF C0G: C = 2·(18 pF − 4.5 pF) ≈ 27 pF.
  - XTALIN is routed through the channel between the crystal's pad columns, so the two crystal traces do not cross. Both are about 4 mm long.
- **C3 – decoupling:** C4 = 100 nF directly at VDD pin 8, and C5 = 100 nF directly at VDD pin 44 / VSS pin 41.
- **RESET:** R6 = 10 kΩ pull-up.
- **C4 – SWD:** J1 = CNC Tech 3220-10-0300-00 with the standard pinout:
  - Pin 1: 3V3. Pins 3, 5 and 9: GND.
  - Pin 2: SWDIO. Pin 4: SWCLK. Pin 10: nRESET.
  - Pins 6, 7 and 8 are unconnected (an M0 has no SWO).
- **C5:** the USB pins are not connected.

### D – LoRaWAN modem

- **D1:** RN2483A-I/RM103; RFH is used.
- **D2 – UART:**
  - PIO1_13 (TXD) → R11 = 1 kΩ → modem UART_RX. R11 lets a PC adapter on J4/J6 overdrive the MCU's TX.
  - Modem UART_TX → PIO1_14 (RXD).
  - No flow control.
- **Modem reset:** PIO1_22 → R12 = 4.7 kΩ → modem /RESET, with R13 = 100 kΩ pull-up.
- **Decoupling:** C10 = 4.7 µF, C9 = C6 = 100 nF at the top edge of the module. The VDD pads themselves are side pads hemmed in by the battery and the board edge.
- **D3 – RF path:**
  - J7 = Cinch 142-0701-801. I created a device on the library package `CINCH_142-0701-80X`.
  - Placed at (39, 17), rotation 270, so the jack points +X into the notch.
  - Trace: 1.0 mm wide at the module pad, then 1.2 mm.
  - Grounded CPW with w = 1.2 mm and s = 0.2 mm on 1.6 mm FR4 (εr 4.5) gives about 50.7 Ω (conformal-mapping calculation).
  - The bottom GND plane is solid under the whole RF path, and 8 stitching vias run alongside it.
  - No matching network: RFH is a 50 Ω output.
- **D4:** J6 1 × 7 modem service header with the brief pinout.

### E – Sensors

- **E1 – SHT21:** at (51.5, 4) on the tab, the farthest point from the module, MCU and LDO.
  - An internal slot at X 40–42 / Y 3–6.8 leaves a 3 mm bottom bridge and a 1.2 mm copper-free top bridge.
  - A keep-out bans planes on both layers over the whole tab.
  - Only four thin traces cross the bridge: 3V3 0.3 mm, SCL/SDA 0.25 mm and GND 0.3 mm on the bottom.
  - The only parts on the tab are U4 and C11 = 100 nF.
- **E2 – DS18B20:** J2 at (2.0, 18.5) with GND, DQ, 3V3. DQ goes to PIO0_21 with R16 = 4.7 kΩ pull-up.
- **E3 – I²C expansion:** J3 with 3V3, SDA, SCL, GND.
  - 4.7 kΩ pull-ups: R14 near J3 and R15 just before the tab bridge.

### F – User interface

- **F1 – LEDs:** each on its own GPIO (PIO1_27, PIO1_20, PIO0_3) through 470 Ω to GND.
  - Current: I = (3.3 − 2.0 V)/470 Ω ≈ 2.8 mA.
  - Placed in the upper left wing.
- **F2:** J4 sits on the MCU↔modem link; pin 1 is the modem TX and pin 2 is the modem RX.
- **F3:** J5 is on PIO0_18/PIO0_19 (alternate USART pins).

## 3. MCU pin assignment

| Pin | Port | Use | Net |
|---|---|---|---|
| 3 | RESET/PIO0_0 | nRESET | RESET (R6, J1.10) |
| 4 | PIO0_1 | ISP strap | open (internal pull-up) |
| 5, 41 | VSS | GND | |
| 6/7 | XTALIN/XTALOUT | crystal | Y1, C7/C8 |
| 8, 44 | VDD | 3V3 | C4/C5 |
| 12 | PIO1_27 | GPIO | LED_RED |
| 13 | PIO1_20 | GPIO | LED_YEL |
| 14 | PIO0_3 | GPIO | LED_GRN |
| 15 | PIO0_4 | SCL | I2C_SCL |
| 16 | PIO0_5 | SDA | I2C_SDA |
| 17 | PIO0_21 | 1-Wire | DQ |
| 29 | SWCLK | SWD | J1.4 |
| 36 | PIO1_13 | TXD | MCU_TXD → R11 → modem RX |
| 37 | PIO1_14 | RXD | MODEM_TX |
| 38 | PIO1_22 | GPIO | MODEM_RST via R12 |
| 39 | SWDIO | SWD | J1.2 |
| 40 | PIO0_16 | AD5 | VBAT_ADC |
| 42 | PIO0_23 | GPIO | MEAS_EN |
| 46 | PIO0_18 | RXD (alternate) | DBG_RX (J5.1) |
| 47 | PIO0_19 | TXD (alternate) | DBG_TX (J5.2) |
| 19, 20 | USB_DM/DP | – | not connected |
| others | – | – | not connected |

The MCU is rotated 270°. The assignment follows that orientation: the left side feeds the left-wing bus, the bottom-right corner faces the modem, the right side faces SWD/UART1, and the crystal sits above pins 6/7.

## 4. Placement and routing approach

**Placement**
- The RN2483 only fits vertically in the 20 mm strip right of the cell (courtyard X 31.1–50.9, pads 0.35 mm from the edge), with the RF row facing the SMA.
- The MCU is above the module, and SWD and UART1 are at the top right.
- The LDO, the reverse-protection FET and the measurement switch are above the cell next to the + clip.
- The left wing holds the LEDs and the I2C, UART0, service and probe headers.
- There are no components in X 13–31 / Y 3–73.

**Routing**
- Everything was routed by hand with explicit coordinates.
- **Layer planning:** under the cell, horizontals run on top and verticals on the bottom; the through-hole header pads double as layer changes.
- **Main corridors:**
  - A top bus from the MCU's left side to the left wing.
  - Bottom verticals at X 26.6–29.6 (DQ, SDA, SCL, 3V3) to the probe and the tab.
  - A bottom corridor along the top edge for VBAT_ADC/MEAS_EN.
  - Bottom diagonals under the module, reached through stubs to vias inside the footprint.
- **Planes:** GND on both layers, an RF GND plane with 0.2 mm clearance, a keep-out on the tab, and small keep-outs at the inner corners.

## 5. Requirement compliance

| ID | Status | Reason |
|---|---|---|
| A1 | met | Keystone 54, 18650 |
| A2 | met | P-FET, about 2 mV drop |
| A3 | met | LP5907-3.3, EN tied to IN |
| B1 | met | AD5, ≤2.1 V |
| B2 | met | P-FET switch, ≪1 µA leakage |
| B3 | met | 0.1 % resistors, ≤0.1 % ratio error |
| C1 | met | custom device |
| C2 | met | ABM8G + 27 pF |
| C3 | met | C4/C5 at each VDD pin |
| C4 | met | 3220-10 with nRESET |
| C5 | met | USB unconnected |
| D1 | met | |
| D2 | met | |
| D3 | met (caveat) | GCPW about 50 Ω, about 18 mm long |
| D4 | met | |
| E1 | met | slot, no planes, thin traces |
| E2 | met | |
| E3 | met | |
| F1 | met | about 2.8 mA per LED |
| F2 | met | |
| F3 | partly | single USART |
| M1 | met | |
| M2 | met | |
| M3 | met | |
| M4 | met | |
| M5 | met | |
| M6 | met | |
| PCB technology | met | 0.2 mm min width/clearance, 0.7/0.3 vias, power 0.4–0.5 mm, silkscreen name + pin labels |
| Deliverables 1–5 | met | REPORT.md content given here |

**Approved DRC messages:**
1. **14 × "plane/pad ↔ polygon" at J7.** The library footprint has net-less copper strips that overlap the GND legs, so they are GND in reality.
2. **5 × "pad ↔ board outline" at the edge SMA.** This is intended for an edge-mount jack.
3. **1 × outline inner radius.** The brief's sharp corners will be rounded by the fab's router tool.
4. **1 × useless via.** A bottom-only junction via; harmless.

## 6. Tool gaps

1. **No autorouter or routing assistance.** I made about 150 manual `board_route` calls and about 70 vias.
2. **Library elements cannot be edited or deleted.** Re-creating a device to add the missing categories left same-name duplicates, and the crystal's legend overlaps its pads.
3. **`board_get_part` lacks pad sizes.** I had to cross-reference `library_get` and rotate by hand.
4. **A via does not join a trace it sits on.** Joining needed small detours.
5. **No impedance or length helper.** I calculated the GCPW width and the RF length offline.
6. **No via tenting control.** Vias have mask openings, which limited silkscreen placement.
7. **Net labels cannot be moved and pages cannot be renamed.** "Main" became the power page.
8. **No pin-swap helper.** Each reassignment needed `schematic_disconnect` + `schematic_connect`.
9. **No mechanical/3D check.** Overlap with the cell had to be judged by hand.

## 7. Workarounds and their downsides

- **Custom parts:** MCU, crystal and SMA devices created by me; this left duplicate names in the library.
- **Vias under the RN2483 and LQFP bodies:** harder rework.
- **Internal slot instead of an open slot:** keeps the brief's outline, but the 1.2 mm top bridge remains a small heat path.
- **Separate RF plane polygon:** one clearance value applies to every net inside it.
- **Plane keep-outs at the inner corners:** needed to clear the plane-fill DRC errors.
- **R7 removed:** the ISP strap relies on the internal pull-up.
- **UART1 on the alternate pins:** needs firmware remapping or a soft UART.
- **DRC approvals:** a reviewer has to trust the explanations.
- **Python wrapper scripts** (`lp.py`, `rt.py`, `calls.log`) batched the MCP calls. All design data went through the MCP tools only.

## 8. Bugs and confusing behaviour

1. **Duplicate devices created silently.**
   - Call: `device_create` with a name that already exists ("Abracon ABM8G").
   - Response: success with a new UUID; `library_search` then lists two devices with identical names.
2. **Route to a nearby position resolves to the start via.**
   - Call: `board_route {"from":"via:1f20e655…","to":[27.8,65.0],"layer":"top"}`
   - Response: `TOOL ERROR: BI_NetLine: both endpoints are the same.`
   - The same pattern worked for SCL with `[28.8,65.8]`.
3. **Removing vias leaves dangling traces.**
   - Call: `board_remove_items {"ids":["via:438ff714…","via:5920a123…"]}` → `{"removed":2}`.
   - The attached traces stayed behind as floating stubs, which `board_status` reported as unrouted. Junctions behave differently, per the documentation.
4. **A via placed on a same-net trace does not connect to it.**
   - Call: `board_add_via {"net":"+3V3","position":[49.8,58.0]}` on a +3V3 bottom trace.
   - `board_status` then reported `point(49.8,59) ↔ via(49.8,58)` as unrouted.
5. **`board_route` returns more trace IDs than segments.** A one-segment route returned 16 IDs, another 53.
6. **The default plane fill violates its own DRC.** Planes produced `plane ↔ board outline < 0.3 mm` at the inner corners (13, 76) and (13, 14).
7. **Board edge reported as "stop mask".** Silkscreen near the edge gave `silkscreen text ↔ stop mask` at X 0.111.
8. **Text anchor semantics.** `align:"center"` centres horizontally but anchors at the bottom of the text, and the text box is larger than its nominal height.
9. **Library SMA footprint.** It has net-less copper strips and no device exists for it.
10. **Legend drawn over pads.** `package_create` with `legend:true` produced `Clearance of pad '1' in 'default' to legend` ×4.
11. **Route cannot end in free space.** `board_route` to `[49,75]` returned `No trace, junction or via of net 'MEAS_EN' on layer 'top' at [49, 75].`
12. **Stale 1-pin net.** After `part_remove` of R7, the ISP net remained with a single pin.

## 9. Suggestions (ranked by impact)

1. **Autorouter / obstacle-avoiding point-to-point router**, plus via fan-out.
2. **Library update and delete tools**, with duplicate-name protection.
3. **Pad geometry in `board_get_part` / `board_query`**, plus a "does this trace fit?" check.
4. **Vias should auto-join traces** they sit on; removing a via should also remove its traces.
5. **`board_route` should return only the new segment IDs.**
6. **RF helpers:** impedance calculator, per-net clearance in net classes, and length reporting.
7. **Via tenting control** and clearer DRC object names (e.g. "board edge").
8. **Schematic page rename and net-label editing.**
9. **Pin-swap tool.**
10. **Plane fill should respect its own DRC** at concave corners.

## 10. Datasheet assumptions

- **LPC11U24:**
  - ADC reference = VDD.
  - PIO0_1 pull-up active at reset.
  - USART can be routed to PIO0_18/19, PIO1_13/14 or PIO1_26/27.
- **ABM8G-12.000MHZ-4Y-T3:** CL = 18 pF; generic 3225 4-pad land pattern; stray capacitance 4–5 pF.
- **LP5907:** stable with 1–10 µF ceramic output capacitance; Iq ≈ 12 µA; pinout 1 IN, 2 GND, 3 EN, 5 OUT.
- **SSM3J338R:** Vgs ±8 V, Rds(on) about 50 mΩ at −2.5 V, Idss ≤ 1 µA, pins 1 G / 2 S / 3 D.
- **SSM3K329R:** Vth 0.4–1.0 V.
- **RN2483:**
  - RFH is 50 Ω; TX peak about 40 mA.
  - No guaranteed /RESET pull-up (hence R13).
  - LVP programming.
  - No exposed copper under the body.
- **SHT21:** exposed pad may go to GND; 100 nF decoupling.
- **DS18B20:** 4.7 kΩ pull-up suits a 1 m cable.
- **OSRAM 0805 LEDs:** Vf about 2.0 V at 3 mA.
- **FR4:** εr 4.5; copper thickness ignored in the Z₀ estimate (lowers Z₀ by about 1–2 Ω).
