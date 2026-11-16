# Hardware and implementation research

Date: 2026-11-16

This document records observations from public source repositories. It is a research record, not proof that every UV-K1 board revision has identical components.

## Public technical references inspected

1. Armel UV-K1/K5 V3 firmware custom port
   - URL: https://github.com/armel/uv-k1-k5v3-firmware-custom
   - Branch inspected: main
   - Relevant paths: `App/board.c`, `App/driver/bk4829.c`, `App/driver/bk4819.h`, `App/driver/st7565.c`, `App/driver/st7565.h`, `Core/py32f071xb.ld`, `Core/startup_py32f071xx.s`, `App/app/spectrum.c`, `App/CMakeLists.txt`
   - Repository declares Apache-2.0 licensing. Preserve the license and attribution if any source is reused; this project currently records it as a reference only.
2. GOGUFW UV-K1 Messenger
   - URL: https://github.com/Gogu-Qs/GOGUFW-UV-K1-Messenger
   - Branch inspected: main
   - Used as a second implementation reference only. No source copied.

## Findings and cautions

- The Armel project contains a PY32F071 device linker script and startup assembly, plus board initialization and ST7565 display driver paths. These are useful evidence for the MCU family and LCD software interface.
- Its board GPIO initialization configures LCD A0 on PA6, LCD CS on PB2, RF serial clock/data on PB8/PB9, and RF chip select on PF9. These pin assignments are specific to the inspected source revision and must not be assumed for a different PCB revision without corroboration.
- The same source tree contains a file named `bk4829.c`, but its visible API and comments still use BK4819 identifiers. A filename alone is not proof of the physical RF IC model or register compatibility.
- The board source labels the RF serial pins and CS as BK4819. This conflicts with informal descriptions calling the V3 RF front end BK4829. Treat the exact transceiver identity and register map as UNCONFIRMED until verified from the board schematic, BOM, chip marking, or reliable hardware-specific documentation.
- The LCD source path is ST7565. Confirm the exact LCD glass/controller variant and wiring for the user's radio before relying on initialization timing or offsets.
- The inspected Spectrum source is under Apache-2.0 and includes Fagci attribution. It is a technical reference only; this project must not copy it without preserving license and notices and making an explicit reuse decision.

## Consequences for this project

1. Do not write or enable RF register sequences based only on a guessed BK4819/BK4829 identity.
2. Build the Spectrum processing and display-independent tests separately from the hardware driver.
3. Keep the live trace independent from Peak Hold and Waterfall. No cascade, trail, lateral smearing, or feedback from display history into the live signal.
4. Do not produce a flashable BIN until MCU, memory map, bootloader offset, RF chip, LCD controller, pinout, and startup sequence are verified for the exact target.
5. Preserve source provenance, revisions, licenses, and modifications in this directory before any third-party code is reused.

## Current release blockers

- No firmware source tree has yet been added to this repository; it currently contains the README and project specification.
- The exact radio board revision and RF chip identity are not confirmed by the repository itself.
- The bootloader application start address and protected calibration/data regions have not yet been independently established for the exact unit.
- No cross-compilation or hardware test has been performed for this repository.
