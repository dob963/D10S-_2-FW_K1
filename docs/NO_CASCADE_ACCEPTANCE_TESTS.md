# No-Cascade Spectrum Acceptance Tests

Date: 2026-11-17  
Target: UV-K1 standard, PY32F071, BK4829, ST7565 128x64.

This document defines acceptance criteria for the Spectrum data path. Host tests cover selected software properties; hardware tests remain open.

## Required data separation

Maintain four independent data products:

- `measurement[b]`: raw/accepted measurement for bin `b` from the current completed sweep.
- `display_stable[b]`: same-bin output from the stabilizer. It may apply the configured hold/release/deadband, but may not spread values to neighboring bins or average two sweeps.
- `peak_hold[b]`: visual maximum accepted for bin `b`, independent from the live trace.
- `waterfall[row][b]`: snapshot of a completed sweep, stored only when Waterfall is enabled.

The live measurement path copies the current sweep directly; `sweep_average()` is not used by `main.c`. Adaptive coarse samples are mapped to their true frequency bins and marked valid; unsampled bins remain empty rather than being shifted to the left or interpolated. Median3 runs on a compact snapshot of valid samples in frequency order, never in-place. Stabilizer hold/release is allowed only per bin, before rendering. No cross-fade, temporal EMA, gradient clamp or neighbor propagation may modify the live measurement.

## Deterministic host tests

### T1 — single-bin impulse

Input sweep A: every bin = 10, except bin 20 = 90.
Expected measurement A: bin 20 = 90; all other bins = 10.
Expected display A: the stabilizer may accept/hold bin 20 according to its thresholds; every other bin remains 10 or its same-bin baseline.

Input sweep B: every bin = 10.
Expected measurement B: every bin = 10. No trace may remain in the measurement array at bin 20 and no neighboring bin may inherit it.
Expected display B: bin 20 may decay only according to the configured same-bin hold/release; bins 19 and 21 must not inherit its value.
Expected peak B: bin 20 retains its independent visual maximum until peak reset.

### T2 — moving impulse

Sweep A: impulse at bin 20. Sweep B: impulse at bin 21.
Expected measurement B: only bin 21 contains the new impulse.
Expected display B: any residual hold remains confined to bin 20; the new peak is confined to bin 21. No lateral movement or trail is permitted.
Expected peak hold: bins 20 and 21 retain independent maxima.

### T3 — waterfall isolation

Run identical input sweeps with Waterfall enabled and disabled.
The sequence of tune commands, dwell times, RSSI reads, measurement bins and stabilizer output must be identical. Only waterfall memory and rendering may differ.

### T4 — peak reset isolation

After T1, reset peak hold.
Expected: every peak-hold bin equals the documented reset baseline or current live value, according to the chosen reset contract.
The measurement and stabilizer arrays must remain byte-for-byte unchanged.

### T5 — interrupted sweep

Abort after bin 31.
Expected: no partial sweep is published as a completed measurement frame and no waterfall row is committed. The next completed sweep replaces measurement data in full.

### T6 — candidate confirmation

A single unconfirmed RSSI excursion must not be published as a confirmed station peak. A candidate that passes the configured confirmation sequence may be accepted. Confirmation must not smear values into adjacent bins. Triple coherence remains disabled until AFC is verified.

### T7 — bounds and fixed-point safety

Exercise minimum/maximum RSSI conversion, floor estimation, thresholds and peak refinement at their numeric limits. Assert no signed overflow, wraparound or out-of-range array access. All production DSP remains integer/fixed-point.

## On-radio measurements required before release

- Confirm exact BK4829 register behavior and settle timing on the target board revision.
- Record raw RSSI, calibrated dBm, glitch-indicator values and tune-to-read timing across quiet and strong-signal conditions.
- Test weak signals, adjacent strong signals, image/spurious responses and band/path boundaries.
- Compare identical sweeps with Waterfall toggled; measurement and stabilizer values and radio measurement timing must remain unchanged.
- Verify the radio returns to normal VFO reception and audio state after leaving Spectrum.

## Release rule

A passing host test is necessary but not sufficient. Release requires host tests, successful target build with map/size report, firmware-container validation for UV-K1 bootloader 7.03.01, and physical radio tests. Never label an untested binary as flash-ready.
