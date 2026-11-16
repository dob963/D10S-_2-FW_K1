# DSP integration update

Date: 2026-11-17

## Numeric boundary between AGC and spur rejection

The supplied `spur_update()` API writes to `int8_t`, while AGC can produce 0..255. The integration now saturates the calibrated sample to 0..127 immediately before calling the unchanged spur module. This prevents signed wraparound and keeps the stabilizer input non-negative.

Trade-off: values above 127 are saturated at the adapter boundary. This is a temporary explicit display/DSP scale contract, not a claim that the AGC's full 0..255 dynamic range is preserved. If full-scale fidelity is required, the public spur output type and downstream stabilizer/display representation must be widened together in a separately reviewed API change.

## Waterfall independence

Added:
- `waterfall_set_enabled(int)`
- `waterfall_is_enabled(void)`
- `waterfall_clear(void)`

When disabled, `waterfall_push()` returns immediately and does not shift or write the history buffer. The caller's sweep, RSSI acquisition, live spectrum path and peak-hold path are separate and continue normally.

The waterfall currently stores a 16x128 history buffer in RAM. A renderer for the ST7565 has not yet been connected; these APIs establish independent acquisition control only.

## Build/test status

The GitHub Actions workflow is compiling the recent commits. A successful compile validates syntax/linking and memory limits only; it does not validate RF acquisition or display rendering on the radio.
