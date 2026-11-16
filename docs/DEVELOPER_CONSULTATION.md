# Developer consultation and verified design decisions

Research date: 2026-11-16
Target: standard Quansheng UV-K1 V3 / PY32F071 / bootloader reported as 7.03.01
Scope: this repository only. No other repository is modified.

## Primary developer reference: Armel FAUVEAU

- Spectrum manual: https://armel.github.io/manual/pages/Spectrum-analyzer
- UV-K1/K5 V3 firmware: https://github.com/armel/uv-k1-k5v3-firmware-custom
- Developer discussion #379 (fast scan / RSSI behavior): https://github.com/armel/uv-k1-k5v3-firmware-custom/discussions/379
- Developer discussion #36 (BK4829 register documentation): https://github.com/armel/uv-k1-k5v3-firmware-custom/discussions/36

## Confirmed from developer documentation and replies

1. **RF chip separation is real and mandatory.** Armel's Spectrum manual explicitly distinguishes BK4819 on V1/V2 from BK4829 on V3. The RF front end, LNA/PGA stages, AGC behavior and RSSI scaling differ. Do not reuse V1/V2 RSSI thresholds, dBm conversion, or gain presets on V3.
2. **Noise floor is hardware- and environment-dependent.** The developer notes that a high or unstable floor can be caused by the receiver front end and local RF environment, not necessarily an analyzer defect. AUTO trigger tracks the measured floor; MANUAL trigger is useful when AUTO cannot settle.
3. **Fast RSSI candidate detection must reject stale readings.** In discussion #379, Armel explains the BK4829 fast-scan sequence: program frequency, briefly wait for the glitch indicator to settle, discard the first potentially stale RSSI read, take a second reading, and recheck a possible candidate after a short delay. A candidate then goes through the full tuning/receive validation path.
4. **Fast scan is not a calibrated sensitivity measurement.** It is a quick candidate detector; timing, antenna coupling, calibration, squelch and local noise affect marginal signals.
5. **Spectrum curve should not be cosmetically smoothed in MANUAL mode.** The developer manual says the curve is drawn without cosmetic smoothing in MANUAL, to keep narrow peaks aligned with raw RSSI used by detection. This supports the project's no-cascade/no-trail requirement.
6. **Sweep direction alternation is used to reduce directional bias.** The developer manual describes alternating sweep direction after each full cycle.
7. **BK4829 register-map evidence is unresolved.** Discussion #36 asks whether registers 0x10, 0x11–0x14 and 0x48 differ from BK4819. A purported PDF was posted, but a participant questioned whether it was simply the BK4819 table with a new title. Therefore that PDF is not accepted as authoritative proof of BK4829 register semantics.

## Decisions applied to this project

- Keep BK4829-specific code isolated behind a driver interface; do not import BK4819 register constants or init tables on assumption.
- Until authoritative register documentation or board-verified behavior is available, do not write undocumented BK4829 registers. Mark hardware tuning as blocked rather than guessing.
- For every sweep point: tune; wait for receiver/glitch settling; discard the first RSSI sample; read a second sample; if above candidate threshold, wait briefly and confirm with another reading; only then accept a peak candidate.
- Use a separate adaptive floor and hysteresis for the V3 receiver. Do not transplant V1 RSSI/dBm calibration.
- Live trace is raw or minimally conditioned only: no cosmetic temporal persistence, no lateral interpolation that smears peaks, no cascade/trail. Any peak confirmation is a detection decision, not a display-history blend.
- Peak Hold is a separate overlay with independent reset/decay state. It must never feed the live trace.
- Waterfall is optional and consumes a copy of completed sweep data; enabling/disabling it must not alter tuning, sampling cadence, RSSI thresholds, or live trace.
- Alternate sweep direction only if it does not break uniform frequency-bin mapping or deterministic peak positioning.

## Important distinction: consultation status

These are public developer documents and community replies, not a private engineering review or a direct confirmation from the radio manufacturer. The register-map issue remains open. No manufacturer datasheet or independently verified BK4829 register table has yet been established.

## Sources

- Armel Spectrum manual, hardware differences and AUTO/MANUAL behavior: https://armel.github.io/manual/pages/Spectrum-analyzer
- Armel discussion #379, BK4829 stale RSSI discard and candidate recheck: https://github.com/armel/uv-k1-k5v3-firmware-custom/discussions/379
- Armel discussion #36, unresolved BK4829 register table: https://github.com/armel/uv-k1-k5v3-firmware-custom/discussions/36
- Armel UV-K1/K5 V3 firmware README and source: https://github.com/armel/uv-k1-k5v3-firmware-custom
- Quansheng toolkit hardware support and backup/DFU cautions: https://github.com/knoinok/quansheng_toolkit
