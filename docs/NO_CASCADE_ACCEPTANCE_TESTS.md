# No-Cascade Spectrum Acceptance Tests

Date: 2026-11-16
Target: UV-K1 standard, PY32F071, BK4829, ST7565 128x64.

This document defines executable acceptance criteria for the Spectrum data path.
It does not claim that firmware or hardware tests have already run.

## Required data separation

Maintain three independent data products:

- `live[b]`: the accepted measurement for bin `b` from the current completed
  sweep only.
- `peak_hold[b]`: maximum accepted value since the last explicit peak reset.
- `waterfall[row][b]`: a snapshot of a completed sweep, stored only when
  Waterfall is enabled.

No display interpolation, temporal smoothing, neighbor propagation or
cross-sweep maximum may write into `live[]`.

## Deterministic host tests

### T1 — single-bin impulse

Input sweep A: every bin = 10, except bin 20 = 90.
Expected live A: bin 20 = 90; all other bins = 10.
Expected peak A: same as live A.

Input sweep B: every bin = 10.
Expected live B: every bin = 10. In particular bins 19, 20 and 21 must all
be 10. No trace may remain at bin 20 and no neighboring bin may inherit it.
Expected peak B: bin 20 remains 90 until peak reset.

### T2 — moving impulse

Sweep A: impulse at bin 20. Sweep B: impulse at bin 21.
Expected live B: only bin 21 contains the impulse.
Expected peak hold: bins 20 and 21 retain their independent maxima.
No lateral movement or trail is permitted in live data.

### T3 — waterfall isolation

Run identical input sweeps with Waterfall enabled and disabled.
The sequence of tune commands, dwell times, RSSI reads, accepted live bins,
and peak-hold values must be identical. Only waterfall memory and rendering
may differ.

### T4 — peak reset isolation

After T1, reset peak hold.
Expected: every peak-hold bin equals the documented reset baseline or the
current live value, according to the chosen reset contract.
The live array must remain byte-for-byte unchanged.

### T5 — interrupted sweep

Abort after bin 31.
Expected: no partial sweep is published as a completed live frame and no
waterfall row is committed. The next completed sweep replaces live data in
full.

### T6 — candidate confirmation

A single unconfirmed RSSI excursion must not be published as a confirmed
station peak. A candidate that passes the configured confirmation sequence
may be accepted. Confirmation must not smear values into adjacent bins.

### T7 — bounds and fixed-point safety

Exercise minimum/maximum RSSI conversion, floor estimation, thresholds and
peak refinement at their numeric limits. Assert no signed overflow, wraparound
or out-of-range array access. All production DSP remains integer/fixed-point.

## On-radio measurements required before release

- Confirm the exact BK4829 register behavior and settle timing on the target
  board revision.
- Record raw RSSI, calibrated dBm, glitch-indicator values and tune-to-read
  timing across quiet and strong-signal conditions.
- Test weak signals, adjacent strong signals, image/spurious responses and
  band/path boundaries.
- Compare identical sweeps with Waterfall toggled; live values and radio
  measurement timing must remain unchanged.
- Verify the radio returns to normal VFO reception and audio state after
  leaving Spectrum.

## Release rule

A passing host test is necessary but not sufficient. Release requires the
host tests, a successful target build with map/size report, firmware-container
validation for UV-K1 bootloader 7.03.01, and physical radio tests. Never label
an untested binary as flash-ready.
