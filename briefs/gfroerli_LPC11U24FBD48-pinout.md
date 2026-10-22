# NXP LPC11U24FBD48/401 — pinout excerpt

Transcribed from the NXP LPC11U2x datasheet (pin description table, LQFP48
package). Package: LQFP-48, 7 × 7 mm body, 0.5 mm pitch, 9 × 9 mm over leads.
Pin 1 is the top pin of the left side, numbering counter-clockwise (top view), 12
pins per side.

| Pin | Name (default function first) | Other functions |
|----:|-------------------------------|-----------------|
| 1 | PIO1_25 | CT32B0_MAT1 |
| 2 | PIO1_19 | DTR, SSEL1 |
| 3 | RESET / PIO0_0 | active-low reset input |
| 4 | PIO0_1 | CLKOUT, CT32B0_MAT2, USB_FTOGGLE |
| 5 | VSS | ground |
| 6 | XTALIN | crystal oscillator input |
| 7 | XTALOUT | crystal oscillator output |
| 8 | VDD | supply |
| 9 | PIO0_20 | CT16B1_CAP0 |
| 10 | PIO0_2 | SSEL0, CT16B0_CAP0 |
| 11 | PIO1_26 | CT32B0_MAT2, RXD |
| 12 | PIO1_27 | CT32B0_MAT3, TXD |
| 13 | PIO1_20 | DSR, SCK1 |
| 14 | PIO0_3 | USB_VBUS |
| 15 | PIO0_4 | SCL (I²C) |
| 16 | PIO0_5 | SDA (I²C) |
| 17 | PIO0_21 | CT16B1_MAT0, MOSI1 |
| 18 | PIO1_23 | CT16B1_MAT1, SSEL1 |
| 19 | USB_DM | USB D− |
| 20 | USB_DP | USB D+ |
| 21 | PIO1_24 | CT32B0_MAT0 |
| 22 | PIO0_6 | USB_CONNECT, SCK0 |
| 23 | PIO0_7 | CTS |
| 24 | PIO1_28 | CT32B0_CAP0, SCLK |
| 25 | PIO1_31 | — |
| 26 | PIO1_21 | DCD, MISO1 |
| 27 | PIO0_8 | MISO0, CT16B0_MAT0 |
| 28 | PIO0_9 | MOSI0, CT16B0_MAT1 |
| 29 | SWCLK / PIO0_10 | SCK0, CT16B0_MAT2 |
| 30 | PIO0_22 | AD6, CT16B1_MAT1, MISO1 |
| 31 | PIO1_29 | SCK0, CT32B0_CAP1 |
| 32 | TDI / PIO0_11 | AD0, CT32B0_MAT3 |
| 33 | TMS / PIO0_12 | AD1, CT32B1_CAP0 |
| 34 | TDO / PIO0_13 | AD2, CT32B1_MAT0 |
| 35 | TRST / PIO0_14 | AD3, CT32B1_MAT1 |
| 36 | PIO1_13 | DTR, CT16B0_MAT0, TXD |
| 37 | PIO1_14 | DSR, CT16B0_MAT1, RXD |
| 38 | PIO1_22 | RI, MOSI1 |
| 39 | SWDIO / PIO0_15 | AD4, CT32B1_MAT2 |
| 40 | PIO0_16 | AD5, CT32B1_MAT3, WAKEUP |
| 41 | VSS | ground |
| 42 | PIO0_23 | AD7 |
| 43 | PIO1_15 | DCD, CT16B0_MAT2, SCK1 |
| 44 | VDD | supply |
| 45 | PIO0_17 | RTS, CT32B0_CAP0, SCLK |
| 46 | PIO0_18 | RXD, CT32B0_MAT0 |
| 47 | PIO0_19 | TXD, CT32B0_MAT1 |
| 48 | PIO1_16 | RI, CT16B0_CAP0 |

## Notes from the datasheet

- Supply 1.8 V to 3.6 V on both VDD pins.
- Crystal oscillator: 1 MHz to 25 MHz on XTALIN/XTALOUT.
- PIO0_1 is sampled at reset: low enters the ISP bootloader (internal pull-up
  enabled by default).
- PIO0_4/PIO0_5 are true open-drain I²C pins; they need external pull-ups.
- There is one USART. Its RXD/TXD can be routed to PIO0_18/PIO0_19,
  PIO1_14/PIO1_13 or PIO1_26/PIO1_27 (selected in IOCON at runtime).
- ADC: 10-bit, inputs AD0 to AD7, input range 0 V to VDD.
