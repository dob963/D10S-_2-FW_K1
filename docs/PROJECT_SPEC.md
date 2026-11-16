# D10S Advanced Spectrum Analyzer — UV-K1 V3

## Project scope

Firmware project dedicated to the advanced spectrum analyzer for the standard Quansheng UV-K1 V3. Work is restricted to this repository: `dob963/D10S-_2-FW_K1`, branch `main`.

No other repository, branch, or existing firmware project is to be modified.

## Hardware target (must be verified against the exact board)

- MCU: PY32F071, ARM Cortex-M0+, no FPU
- Flash/RAM: 128 KiB / 16 KiB
- Bootloader: 7.03.01 (standard UV-K1; do not assume UV-K1(8) Mini Kong compatibility)
- RF transceiver: BK4829 — confirm register map and board wiring before enabling hardware writes
- LCD: ST7565, 128×64 monochrome — confirm controller wiring and initialization sequence from the target board sources

The BK4819/BK4829 distinction is a release blocker: no firmware image may be labelled flash-ready until the fitted RF IC is confirmed.

## Spectrum behavior requirements

- No cascade effect: no trails, lateral propagation, smearing, or artificial persistence in the live spectrum trace.
- Live trace and peak-hold are separate layers.
- Peak hold is independently resettable and must not feed back into the live trace.
- Waterfall is optional and independent; disabling it must not change sweep timing, RSSI processing, or the live trace.
- Stable, linear frequency axis and repeatable bin placement.
- Dynamic noise-floor estimate with bounded adaptation.
- Peak confirmation across repeated observations to reject transient noise.
- Frequency refinement around confirmed peaks.
- Fast sweep without accepting single-sample noise spikes.
- Readable frequency labels and trace weight; avoid excessive boldness.
- No false sensitivity increase by simply raising gain or lowering thresholds.

## DSP and implementation constraints

- C11, fixed-point arithmetic, Cortex-M0+ compatible; no floating-point dependency.
- Preserve the supplied DSP module logic unless a demonstrated integration or correctness defect requires a narrowly scoped correction.
- Keep the live trace free of temporal or spatial cascade effects.
- Explicit bounds and saturation for all display and signal-processing conversions.
- Keep ISR work bounded; do not perform full display rendering in interrupt context.
- Validate RAM and Flash use against the target limits.

## Required engineering checks

1. Confirm exact board revision, MCU, RF IC, LCD controller, and bootloader compatibility from reliable hardware/source evidence.
2. Audit the repository and preserve existing user files.
3. Implement hardware access only after register map, pin mapping, clocking, and bus timing are verified.
4. Verify startup initialization of vector table, .data, .bss, stack, and clock.
5. Build with a reproducible ARM GNU toolchain and record its version and complete build command.
6. Run compiler warnings, static checks, host-side DSP tests where feasible, and firmware size checks.
7. Produce a BIN only after a successful target build; record SHA-256 and build provenance.
8. Do not claim radio-level operation is verified until the image has been tested on the exact UV-K1 hardware.

## Third-party code and licensing

Use public repositories and documentation as technical references. Do not copy source code unless its license permits the intended use. For any permitted reuse, preserve copyright notices, license texts, NOTICE files, and required attribution. Record source URL, revision, license, and modifications in a third-party inventory. Do not combine incompatible licenses. The project's own distribution license remains undecided until explicitly established.

## Release gate

A release candidate must include complete source, linker/startup configuration, build scripts, memory report, test results, flashing/backup instructions, SHA-256, and the matching firmware BIN. A source-only or uncompiled artifact must never be described as a flashable firmware.
