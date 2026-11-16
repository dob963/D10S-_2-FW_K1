# BK4829 Spectrum3D measurement-path audit

Date: 2026-11-16

## Scope

Read-only review of the public upstream repository
[armel/uv-k1-k5v3-firmware-custom](https://github.com/armel/uv-k1-k5v3-firmware-custom)
at revision `3261ca7883d5b0b60c0905d8766612dee7b4191d`. No upstream files were copied
into this project. This audit records implementation evidence and constraints
for the UV-K1 standard / PY32F071 / BK4829 target.

## Newly confirmed implementation evidence

The upstream Spectrum3D application contains a dedicated BK4829 measurement
path in `App/apps/spectrum3d/spectrum3d_app.c` (blob
`e2d4a9a0ca93502ace1838c7b53f2088efa924c9`):

- Frequency is programmed through registers `0x38`, `0x39`, followed by
  control register `0x30`.
- The implementation polls register `0x63` as a glitch/settling indicator,
  with a bounded guard, then discards one read of register `0x67` before
  obtaining the RSSI value through the resident calibrated `rssi_dbm()` API.
- A nonzero settle delay is used for selected signal-adjacent points; quiet
  points use the bounded glitch-indicator polling path. The code comment says
  this avoids measuring the previous frequency and smearing a strong signal
  into the following bin.
- The sweep uses a fixed frequency step for each selected span, and explicitly
  treats an unfinished sweep as invalid if the window moves or the app is
  interrupted.
- Its display history is a deliberate time-history/landscape feature. That
  history is NOT suitable for this project's live trace because the project
  requirement forbids cascade, trail, lateral propagation and artificial
  persistence.

This is stronger implementation evidence for the measurement sequence than
the earlier register-list discussion. It is still not a substitute for an
independent BK4829 datasheet/register specification or a test on the exact
radio hardware revision. Register numbers and timing must be isolated behind
a target-specific driver and verified before flash release.

## Design decisions for D10S

1. Keep the live trace as a direct per-bin representation of the current
   completed sweep. Do not import Spectrum3D's time-history rendering,
   interpolation, scrolling landscape, or any other visual persistence.
2. Keep Peak Hold in a separate array and provide an independent reset.
3. Keep Waterfall as an optional snapshot/history consumer. Disabling it must
   not change sweep tuning, dwell, RSSI sampling, or live-trace data.
4. Implement the upstream measurement concept as a candidate sequence only:
   tune -> bounded settle/glitch check -> discard potentially stale RSSI ->
   read calibrated RSSI -> optionally confirm a candidate after a controlled
   delay. Validate exact register semantics, thresholds and timeouts on device.
5. Do not treat the upstream `rssi_dbm()` conversion as a universal calibration
   constant. Preserve the radio's calibrated RSSI path and characterize the
   BK4829's noise floor separately from BK4819 values.
6. A strong signal may desensitize the wide-open RF front end. Software
   filtering cannot remove the receiver's physical front-end limitation.

## Ulteriori fonti community consultate (lettura, non riuso)

- Repository [fagci/k1-scaner](https://github.com/fagci/k1-scaner), README e `src/driver/bk4829.c` / `bk4829.h`: il README descrive BK4829 come compatibile BK4819, ma l'API, gli enum, la tabella registri e molte funzioni mantengono i nomi `BK4819_*` e includono `bk4819-regs.h`. È quindi un port/assunzione di compatibilità, non una prova indipendente della mappa registri BK4829. Nessun codice o valore è stato importato.
- [Discussione armel #36 “BK4829 register list?”](https://github.com/armel/uv-k1-k5v3-firmware-custom/discussions/36): viene indicato un PDF “1_BK4829 Registers Table.pdf”, ma nella discussione è anche contestato che possa essere la tabella BK4819 semplicemente rinominata. Non è una fonte sufficiente per autorizzare scritture.
- [Report reverse engineering UV-5RH](https://github.com/amoxu/Baofeng-UV-5RM-5RH-RE/issues/18): riporta un'ipotesi di compatibilità BK4829/FD6818B con BK4819 e test su una radio diversa. È una pista sperimentale su altro hardware, non trasferibile automaticamente a UV-K1 V3.

Conseguenza: i registri AM 0x31, 0x42, 0x2A, 0x2B, 0x2F e il registro filtro 0x43 presenti in sorgenti upstream/community restano riferimenti da investigare, non una specifica BK4829 autonoma. La sequenza Spectrum3D 0x38/0x39/0x30 e letture 0x63/0x67 è una pista di misura più pertinente, ma necessita ancora di verifica sul dump stock 7.03.01 o con lettura controllata del chip.

## License and attribution

The reviewed Spectrum3D source declares Apache License 2.0 and copyright
Armel F4HWN (2026). The upstream `bk4829.c` declares copyright muzkr (2025)
and Dual Tachyon (2023), also under Apache License 2.0. The reviewed
`bk4819.h` declares Dual Tachyon copyright and Apache License 2.0. No source
code has been copied into D10S. If any of these files or derived code is
subsequently reused, preserve the applicable copyright notices, Apache-2.0
license text, NOTICE requirements and source attribution; record exact
upstream revision and modifications in the project's license inventory.

## Files reviewed

- [Spectrum3D application](https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/3261ca7883d5b0b60c0905d8766612dee7b4191d/App/apps/spectrum3d/spectrum3d_app.c)
- [BK4829 driver](https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/3261ca7883d5b0b60c0905d8766612dee7b4191d/App/driver/bk4829.c)
- [BK4819 compatibility header used by upstream driver](https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/3261ca7883d5b0b60c0905d8766612dee7b4191d/App/driver/bk4819.h)
- [PY32F071 linker script](https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/3261ca7883d5b0b60c0905d8766612dee7b4191d/Core/py32f071xb.ld)

## Release blockers unchanged

- Exact app flash offset and container/update format must be verified against
  the installed UV-K1 bootloader 7.03.01 and the actual stock image.
- BK4829 register semantics and timing need independent confirmation.
- Il driver RF attuale resta uno stub: init, tuning, RSSI e AFC non sono implementati.
- Il repository possiede startup/linker e una build ARM di scaffold; le build precedenti hanno prodotto BIN raw sperimentali, non validati per il bootloader.
- Non esiste ancora un pacchetto DFU verificato, né una prova di avvio sulla radio.
- Nessun test RF o test display fisico è stato eseguito.

Nessun firmware flashabile è dichiarato da questo audit.

## Sweep-path detail (full source inspection)

The complete `sweep()` body also shows:

- A rising candidate is measured again after the configured signal settle delay.
- The receiver is parked back at the first sweep frequency after the sweep.
- The upstream history accumulator stores the maximum measured value per bin
  across sweeps before committing a history row. This is a peak-history
  mechanism, not a raw current-sweep live trace. D10S must not use that
  accumulator as its live trace: each live bin must be assigned from the
  current sweep's accepted sample/confirmation result, while peak hold remains
  in a separate buffer.
- A sweep interrupted by user input/window movement is discarded rather than
  treated as a complete spectrum.

These distinctions are important to preserve the no-cascade requirement.
