# Build and startup integration gate

Date: 2026-11-16  
Target repository: `dob963/D10S-_2-FW_K1`, branch `main`.

## Verified MCU facts

Puya's official PY32F071 datasheet (revision 1.4, published 2026-08-18) gives:

- Main Flash: `0x08000000–0x0801FFFF` (128 KiB).
- SRAM: `0x20000000–0x20003FFF` (16 KiB).
- System memory bootloader: `0x1FFF0000–0x1FFF2FFF` (12 KiB).

The system-memory bootloader address range is not the same thing as the radio's installed application bootloader version. It does not establish the UV-K1 7.03.01 application slot, protected pages, calibration storage, or update protocol.

Official device page: https://www.puyasemi.com/mcu_weichuliqi/3843.html  
Official datasheet: https://www.puyasemi.com/download_path/%E6%95%B0%E6%8D%AE%E6%89%8B%E5%86%8C/MCU%20%E5%BE%AE%E5%A4%84%E7%90%86%E5%99%A8/PY32F071%E7%B3%BB%E5%88%97%E6%95%B0%E6%8D%AE%E6%89%8B%E5%86%8C_V1.4.pdf

## Upstream startup/linker audit

Read-only inspection of Armel's public V3 source at revision
`3261ca7883d5b0b60c0905d8766612dee7b4191d` found:

- `Core/py32f071xb.ld` uses RAM origin `0x20000000`, length 16 KiB.
- That project's application linker region starts at `0x08002800`, length 118 KiB.
- Its startup copies `.data` from Flash to RAM and clears `.bss` before calling `main`.
- The linker/startup carry Puya and ST copyright/license notices. They are not copied into this repository by this audit.

The 0x08002800 origin is an upstream radio firmware layout. It is **not** proven for the user's exact installed bootloader 7.03.01. This repository must not emit a flashable image until the app slot, vector relocation, calibration/EEPROM pages and image format are verified against a known-good UV-K1 V3 image or bootloader documentation.

## Startup correctness requirements

The final startup/linker pair must pass all of these checks:

1. Vector table is linked at the verified application vector address and contains the correct initial stack pointer and Reset_Handler.
2. `.data` load and run addresses are valid; copy loop handles the section bounds correctly.
3. `.bss` and any NOLOAD sections are zeroed exactly once and within SRAM.
4. Stack plus static buffers plus any heap reservation fit within 16 KiB with measured headroom.
5. Linker asserts fail on Flash or RAM overflow.
6. No floating-point instructions or hard-float ABI are emitted; compile for Cortex-M0+ with soft-float.
7. ELF, map and size reports are retained; the BIN is checked for expected origin, length and vector words.
8. No factory calibration, option bytes, system memory, or user EEPROM region is erased or overwritten by the release procedure.

## Host-side DSP test gate

Before target hardware is available, compile the user's fixed-point DSP modules as host tests with strict warnings and sanitizers where available. Required cases:

- fixed-point arithmetic boundaries and saturation;
- median filter input preservation and boundary behavior;
- sweep bin-to-frequency uniformity and endpoints;
- peak confirmation rejects isolated one-sample impulses;
- adaptive floor follows slow baseline changes without swallowing a persistent signal;
- spur rejection does not suppress a stable narrowband carrier;
- live trace is a function of the current sweep only (no previous-sweep contribution);
- Peak Hold reset does not alter live trace;
- Waterfall on/off does not alter sweep frequency list, RSSI sample count, or live trace.

The user's original DSP source was specified as immutable except for proven integration defects. The exact source payload is not present in the current repository yet, so no source-level tests or integration build are claimed here.

## Release blockers

- [ ] Obtain/import the exact user-supplied DSP source into this repository without changing its algorithm.
- [ ] Audit headers, types, symbol visibility, includes and compile warnings.
- [ ] Implement and test the LCD framebuffer renderer (128x64 ST7565) separately from acquisition.
- [ ] Verify target board pinout from the exact UV-K1 V3 hardware revision.
- [ ] Verify BK4829 register semantics and tuning path; do not assume BK4819 definitions.
- [ ] Confirm bootloader 7.03.01 app slot and protected/calibration storage.
- [ ] Build with a pinned ARM GNU toolchain and record version.
- [ ] Pass host tests, target link, map/size checks and hardware RF/display smoke tests.
- [ ] Produce BIN, SHA-256, build log, map file and safe flashing/restore instructions.

No firmware binary is produced by this document. No successful build or radio test is implied.
