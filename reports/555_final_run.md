## 1. Summary

I built the whole design using only the MCP tools. Final status:

- **ERC:** 0 messages (none approved).
- **DRC:** 0 messages (none approved).
- **Routing:** `board_status` shows 0 unrouted connections and 0 unplaced parts.
- **Saved:** `project_summary` shows `unsavedChanges: false`.

What was built:
- **Library "My Parts"** ("Parts for the 555 tutorial", depends on LibrePCB Base). It holds symbol NE555, component NE555 and device NE555 DIP-8 (NE555P, Texas Instruments). All three library checks returned no messages.
- **Project "555 Blinker"** at `BLIND/projects/555_Blinker/555_Blinker.lpp`.
  - Schematic: A4 Landscape frame, the 9 parts and one VCC and one GND supply symbol. The netlist matches the brief exactly, and the TIMING label is visible at U1.TRIG and at R2.2.
  - Board: 40 × 30 mm, 2 copper layers, default rules. All signal nets are routed with 0.5 mm traces, mostly on top; VCC to RESET runs on the bottom layer under the DIP. GND planes fill both layers, and every GND pad connects through them, so no via was needed.
- **Production data:** `output/v1/` has the Gerber and drill files, the standard BOM and pick&place CSVs (top and bottom), the schematic and assembly PDFs, `555_Blinker_v1.zip` and `555_Blinker_v1.lppz`. The schematic PDF shows U1 with the value NE555P.

Not achieved: nothing from the brief. One cosmetic blemish needed a heavy workaround (see sections 4–6).

## 2. Task compliance

| Step | Status | Reason |
|---|---|---|
| 1.1 Symbol NE555 | met | 8 pins on the 2.54 grid; body (−7.62,7.62)…(7.62,−7.62); VCC/RESET/TRIG/THR on the left, DIS/OUT/CV/GND on the right; category "Timers, Oscillators"; name/value texts added automatically; check clean. |
| 1.2 Component NE555 | met | Prefix U, default value `{{MPN or DEVICE}}`, same category, 8 signals. Roles I added: VCC/GND power+required, OUT output, DIS opendrain, the rest input. Check clean. |
| 1.3 Device NE555 DIP-8 | met | Package DIP762W55P254L965H533Q8, pad map as in the brief, part NE555P / Texas Instruments. Check clean. |
| 2.1 Project | met | One schematic ("Main"), one board ("default"). |
| 2.2 A4 frame | met | `schematic_add_part` with component "Schematic Frame" and symbolVariant "A4 Landscape". |
| 2.3 Parts and values | met | R1 1kΩ, R2 47kΩ, R3 470Ω, C1 10μF, C2 10nF, C3 100nF, LED1 red, U1 NE555P, J1 (empty value, since the generic header has no MPN), plus VCC1 and GND1. |
| 2.4 Connections | met | `netlist_get` matches the brief exactly. Free nets named DIS, CV, OUT, LED_A. |
| 2.5 TIMING visible | met | Net labels "TIMING" at U1.TRIG and R2.2; C1+ is wired directly to U1.THR. |
| 2.6 ERC | met | 0 messages. |
| 3.1 2 layers, default rules | met | innerLayers 0, rules not changed. |
| 3.2 Outline 40 × 30 | met | `board_set_outline {"width":40,"height":30}`. |
| 3.3 Placement and footprints | met | All 9 inside the outline with the requested footprints. J1 at the left edge, LED1 at the right edge, C3 between pins 8 and 1 next to pin 8, R1/R2/C1/C2 around pins 2/5/6/7. |
| 3.4 Route all nets except GND | met | 0 unrouted. C3.2 also has a short GND trace to U1.1 for decoupling. |
| 3.5 GND plane on both layers | met | Two whole-board planes; no unconnected GND pad, no via needed. |
| 3.6 DRC | met | Three silkscreen warnings at first; fixed by moving texts, not by approving them. Now 0 messages. |
| 4 Production data | met | All required files present (list in section 1). |

## 3. Placement and routing approach

- **U1:** at (20, 15), rotated 90°. The top row (left to right) is then VCC, DIS, THR, CV and the bottom row GND, TRIG, OUT, RESET. This puts the timing parts above the chip, the output toward the LED (lower right) and VCC on the side facing J1.
- **Other parts:**
  - J1 at (2.75, 15); LED1 at (35.5, 15), rotated 270°.
  - R1 at (13.65, 23) and R2 at (23.81, 27), horizontal, with their DIS pads on a vertical line straight above U1 pin 7.
  - C1 at (29, 21.5); C2 at (27.5, 16.5) next to CV; C3 at (13, 15), vertical between pin 8 and pin 1.
  - R3 at (27, 5.5) below the chip.
- **Top layer:**
  - VCC runs J1.1 → U1.8 along y = 18.81, with T-branches to C3.1 and R1.1.
  - DIS is a straight vertical: R2.1 → R1.2 → U1.7.
  - TIMING goes R2.2 → C1+ → U1.6 (along y = 21.5), then diagonally through the DIP's inner area to U1.2.
  - CV, OUT (U1.3 → R3.1) and LED_A (R3.2 → LED A) are short traces.
- **Bottom layer:** U1.4 (RESET) → U1.8 (VCC) under the chip along y = 15. The 555's diagonally opposite RESET and VCC pins make one crossing unavoidable.
- **Planes:** whole-board GND planes on top and bottom.
- **Silkscreen:**
  - Resistor designators moved into their bodies; U1 name and value moved inside the DIP outline.
  - LED1 name, C1 value and C3 texts moved away from pads and the board edge.
  - Added "555 Blinker {{VERSION}}" and +/− labels at J1.

## 4. Tool gaps

1. **Schematic wires can only go pin to pin, so T-junctions are impossible.**
   - Tried: wiring the R1/R2 DIS node to U1.DIS, and C1+ into the TRIG–THR pair.
   - Missing: a way to end `schematic_draw_wire` at an existing wire or junction, or to place a junction.
   - Impact: most connections had to be net labels via `schematic_connect`. That is readable, but less like a hand-drawn schematic.
2. **No visible label can be added to an existing wire.**
   - Tried: `schematic_connect` on pins that already had a wire. They join the named net but get no label (this is documented).
   - Missing: a "net label" tool, or an option to label wired pins.
   - Impact: a named net can be invisible on a wired segment. This matters for requirement 2.5 (visible name). I avoided it by using labels instead of wires on U1.
3. **No way to delete individual schematic wires or segments.**
   - Missing: a schematic equivalent of `board_list_items` / `board_remove_items`.
   - Impact: orphan wires (see bug 1) could only be removed with 18 undos.
4. **No preview of library elements.**
   - Missing: a render of a symbol, footprint or device, and anything showing the symbol orientation at rotation 0 before placing.
   - Impact: I checked the new symbol only from `library_get` coordinates. I found the default orientations (resistor horizontal, capacitors vertical, LED horizontal) by trial and moved parts afterwards.
5. **No explicit schematic-frame tool.**
   - What works: the frame is placed through `schematic_add_part` with component "Schematic Frame" and a symbolVariant.
   - Impact: it is found only by searching components for "frame"; no tool description mentions it.
6. **Unclear which layers end up on the silkscreen.**
   - Values sit on `top_values`, and the realistic render and Gerber legend did not show them.
   - Missing: nothing in the tools says which layers go to the silkscreen Gerber.
   - Impact: I spent effort placing value texts that turned out to be assembly-only.

## 5. Workarounds and their downsides

- **Net labels instead of wires** for most connections. The schematic is correct and readable, but it is a "label soup" rather than a drawn circuit.
- **18 × `undo`** to get rid of orphan wire segments and empty nets. It worked cleanly; the undo entries are named "Undone: MCP: schematic_connect" and so on. The downside: it throws away all work since that point and needs careful counting of steps.
- **Moved R2 to stop overlapping labels.** Two pins 10 mm apart with labels of the same net ("DIS") produced overlapping text, so I moved R2 further down.
- **Assumptions from my own knowledge:**
  - NE555P pinout 1 GND, 2 TRIG, 3 OUT, 4 RESET, 5 CV, 6 THR, 7 DIS, 8 VCC (also given in the brief).
  - Signal roles in the component as listed in 1.2.
  - f ≈ 1.44 / (95 kΩ · 10 µF) ≈ 1.52 Hz.
  - 0.5 mm traces are fine for an LED current of about 15 mA.

## 6. Bugs and confusing behaviour

1. **`schematic_disconnect` leaves orphan wire segments and empty nets.**
   - Before: `schematic_draw_wire {"from":"U1.VCC","to":"U1.RESET","points":[[111.76,106.68],[111.76,104.14]]}` and the same for TRIG–THR. Then `schematic_disconnect {"pins":["U1.VCC","U1.RESET","U1.TRIG","U1.THR","C1.+","R1.2","R2.1"]}` returned `Disconnected.`
   - `netlist_get` then still listed `{"name":"N2","pins":[]},{"name":"N4","pins":[]}`.
   - A render showed vertical wire stubs left at x = 111.76 between the former pins. Only the segments touching the pins were removed, not the middle segment between the two corners.
   - No tool removes them; only undo did.
2. **Inconsistent `library_get` courtyard schema.**
   - Most devices return `courtyard:[{"side","vertices":[...]}]`.
   - "Capacitor Radial ⌀5.0x11.0/2.0mm" returns `{"center":[0,0],"diameter":5.8,"side":"top"}`, so my parser hit a KeyError.
   - This is valid, but the circle variant is not mentioned in the tool description.
3. **Misleading DRC warning text for silkscreen at the board edge.**
   - `drc_run` reported "Clearance silkscreen text ↔ stop mask < 0.127 mm" at `{"center":[23.277,29.939],"size":[0.723,0.122]}` and at x = 39.939.
   - The cause was the R2 name (y = 28.95) and the LED1 name reaching the board outline, not a solder resist opening.
   - The message gives no hint about the board edge.
4. **Wired pins in `schematic_connect` get no label.** Documented, but surprising: `schematic_connect {"net":"VCC","pins":["U1.VCC","R1.1"]}` returned `{"net":"VCC"}`, yet U1.VCC, which had a wire, showed no VCC anywhere.
5. **BOM drops U1's value.** The BOM line for U1 has an empty Value with MPN NE555P, while the schematic shows "NE555P". Probably intentional de-duplication of `{{MPN or DEVICE}}`, but it is surprising.

## 7. Suggestions, ranked by impact

1. Add schematic wire and label management: list wires and labels with IDs, remove a wire or segment, add a net label at a point on a wire, and let `schematic_draw_wire` end on a wire or junction.
2. Fix `schematic_disconnect` so it removes the whole wire path between pins, including the segments between corners, and deletes nets that end up empty.
3. Add an option to `schematic_connect` (e.g. `label: true`) to put a visible label on pins that already have a wire.
4. Add a `render_library_element` tool (symbol, footprint) and document each component's symbol orientation at rotation 0 in `library_get`.
5. Make DRC messages name the actual conflicting object (board edge or a pad's stop mask), and let `board_set_part_text` / `board_list_items` say whether a layer is printed on the silkscreen.
6. Document the circle courtyard shape in `library_get`, and mention the "Schematic Frame" component and its symbolVariant in the `schematic_add_part` description.

Overall the board, library and output tools worked very well. Every board call returned clean quick-check results, `board_status` and DRC were precise, and output generation was one call. The pain points were all in schematic editing.
