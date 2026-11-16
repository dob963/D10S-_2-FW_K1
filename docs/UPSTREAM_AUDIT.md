# Upstream implementation audit

Audit date: 2026-11-16  
Target repository: `dob963/D10S-_2-FW_K1` (branch `main`)  
Read-only upstream reference: `armel/uv-k1-k5v3-firmware-custom`, revision `3261ca7883d5b0b60c0905d8766612dee7b4191d`.

## Findings that affect implementation

### 1. MCU memory and application offset

The Puya PY32F071-E datasheet specifies a Cortex-M0+ device family with 128 KiB main Flash and 16 KiB SRAM. The upstream K1/K5 V3 linker script uses:

- SRAM: `0x20000000`, 16 KiB
- application Flash origin: `0x08002800`
- application Flash length: 118 KiB

The 0x08002800 origin is an upstream firmware/bootloader layout choice, not a value to generalize to another bootloader revision. The project must validate the installed UV-K1 bootloader 7.03.01 image layout and preserve calibration/EEPROM before generating a flashable image. No application offset will be changed based only on the MCU datasheet.

### 2. BK4829 driver is not a clean BK4829 API

The upstream file is named `App/driver/bk4829.c`, but it includes `driver/bk4819.h` and exposes functions/types named `BK4819_*`. It uses the legacy BK4819 register enum and register constants in its init, AGC, RSSI and tuning paths. It contains a V3-specific frequency scaling function and comments about keeping the runtime AGC profile aligned with stock BK4829 firmware.

This is useful evidence of the actual upstream V3 integration, but it does **not** independently prove that every inherited BK4819 register definition or sequence is valid for every BK4829 silicon/board revision. Therefore:

- do not import the BK4819 register map as a generic BK4829 datasheet;
- do not copy isolated register writes without tracing the complete V3 call path;
- keep RF register access behind a dedicated BK4829 backend;
- mark unverified registers and timing constants as unresolved until backed by a V3 implementation plus hardware validation.

The upstream board file configures PB8/PB9 and PF9 for the bit-banged RF serial bus and PA6/PB2 for LCD A0/CS. These are upstream source pin assignments, not yet a physical board-revision verification for this device.

### 3. Spectrum acquisition behavior found upstream

The upstream Spectrum implementation includes a settling guard based on register 0x63, discards one RSSI sample after tuning and uses the next sample. It also contains adaptive trigger/noise-floor logic and peak-hold rendering.

However, it maintains `rssiHistory` using attack/decay averaging and has a `SmoothTopY()` spatial smoothing path for automatic mode. Those display-history operations can leave a previous signal influencing later displayed bins. They conflict with this project's strict **NO CASCATA** requirement if used for the live trace.

Project decision:

- raw/current-sweep live trace is independent and has no temporal attack/decay or spatial trail;
- peak confirmation is a detector decision, not historical blending into the live trace;
- Peak Hold has its own buffer and reset;
- Waterfall consumes a completed sweep snapshot and can be disabled without changing acquisition;
- any optional smoothing must be an explicit separate display mode, disabled by default, and must never feed detector/RSSI decisions.

### 4. Sweep and RF verification gates

The public upstream developer discussion describes a fast-scan sequence of tune, wait for settling/glitch indication, discard the first RSSI read, take a second read, and re-check a candidate after a short delay. This sequence is a reference for test design, not a universal calibrated sensitivity measurement.

Before RF code is considered complete, tests must establish:

1. the correct BK4829 tune command and frequency scaling for the target board;
2. settling and glitch behavior over the intended sweep range;
3. RSSI repeatability and stale-read rejection;
4. candidate confirmation against the adaptive floor;
5. no change in acquisition cadence when Waterfall is disabled.

No RF register map or calibrated dBm conversion is asserted by this audit.

## Licensing and provenance

The upstream repository declares Apache-2.0 and includes a NOTICE file attributing work from Dual Tachyon, Egzumer, fagci, muzkr, Armel/F4HWN and others. The Spectrum source itself carries a fagci Apache-2.0 notice. This project records upstream behavior and file provenance; it does not copy upstream implementation source in this audit.

If source is later reused, retain the applicable LICENSE, copyright notices, NOTICE attributions, and prominent modified-file notices as required by Apache-2.0. Third-party files with different licenses must be inventoried separately.

## Current gate status

- [x] PY32F071 memory capacity confirmed from Puya datasheet.
- [x] Upstream linker application origin identified.
- [x] Upstream BK4829 compatibility-wrapper pattern inspected.
- [x] Upstream live-trace smoothing conflict identified.
- [ ] Exact target-board pinout physically confirmed.
- [ ] BK4829 register semantics independently verified.
- [ ] Bootloader 7.03.01 application/flash boundary confirmed.
- [ ] Project source, target build, hardware tests and release BIN completed.

This document is an engineering audit, not a claim that firmware has been built or tested on a radio.
