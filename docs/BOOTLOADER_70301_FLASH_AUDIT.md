# UV-K1 bootloader 7.03.01 and firmware-container audit

Audit date: 2026-11-16  
Project: `dob963/D10S-_2-FW_K1`, branch `main`  
Scope: public read-only inspection of `knoinok/quansheng_toolkit` at revision `ea13a5850d8f22372c7ad205db13610c083c4ee8`. No source code from that project is copied here.

## Confirmed compatibility evidence

The toolkit's DFU allowlist maps bootloader `7.03.01` to the standard UV-K1 target (`k1`). It explicitly separates that target from UV-K5 V3 bootloader `7.00.07` and legacy DP32G030 bootloaders `2.00.06` / `5.00.01`.

The toolkit manifest lists:
- UV-K1 stock 7.03.01: PY32F071, target `k1`, 71,286 bytes.
- F4HWN Fusion 5.4.0: PY32F071, targets `k5_v3` and `k1`, tested_on includes UV-K1, 91,948 bytes, Apache-2.0.

This is strong compatibility evidence for the user's stated model/bootloader pair. It is not a substitute for checking the actual radio's DFU identity immediately before flashing.

## Firmware container and DFU protocol findings

The inspected toolkit distinguishes three file forms:
1. Quansheng packed image with trailing CRC-CCITT and 128-byte XOR encoding.
2. Packed image without trailing CRC, using the same XOR transform.
3. Raw ARM image, identified heuristically by a Cortex-M stack pointer in the MCU RAM address range.

For packed images, its parser decodes the container, reads a 16-byte version block at offset `0x2000`, then removes that block before sending the payload to the bootloader. Raw images are sent unchanged. Therefore a firmware download must be classified and validated before any future release packaging; simply renaming a BIN is unsafe.

The DFU protocol implementation uses 256-byte pages, a bootloader version handshake, and per-page acknowledgements/retries. Its safe-flash allowlist refuses unknown bootloader versions and target mismatches. This project will preserve that principle: no blind write and no bypass of the target gate.

## Calibration and recovery safeguards

The toolkit documents a calibration dump/verify/restore workflow and recommends making a complete device backup before modifications. Its UV-K1 stock profile identifies calibration data in the `0x1E00..0x2000` range (512 bytes), with `0x1D00..0x1E00` reserved and not to be written.

Project release requirements:
- Keep the installed bootloader untouched.
- Back up the radio's calibration and configuration before any test flash.
- Verify backup integrity by reading it back and comparing byte-for-byte.
- Do not overwrite calibration, reserved memory, or system boot areas.
- Reject unknown bootloader strings and unknown firmware containers.
- Preserve a known-good recovery image and a documented restore path.

## Important limits

- The toolkit's documented compatibility list supports the stated UV-K1 / 7.03.01 combination, but this audit did not connect to the user's radio or read its live bootloader identity.
- The upstream linker origin `0x08002800` remains a source-layout observation, not a sufficient standalone flash instruction.
- A valid bootloader/MCU pairing does not prove the new Spectrum firmware's BK4829 register operations, LCD pinout, or RF behavior.
- No source firmware has been integrated, no target build has been run, and no BIN has been produced or tested.

## Public references

- Toolkit DFU implementation: https://github.com/knoinok/quansheng_toolkit/blob/ea13a5850d8f22372c7ad205db13610c083c4ee8/kradio/dfu.py
- Toolkit firmware parser: https://github.com/knoinok/quansheng_toolkit/blob/ea13a5850d8f22372c7ad205db13610c083c4ee8/kradio/firmware.py
- Toolkit model profiles: https://github.com/knoinok/quansheng_toolkit/blob/ea13a5850d8f22372c7ad205db13610c083c4ee8/kradio/models/__init__.py
- Toolkit firmware manifest: https://github.com/knoinok/quansheng_toolkit/blob/ea13a5850d8f22372c7ad205db13610c083c4ee8/firmwares/manifest.json
- Toolkit overview and validated combinations: https://github.com/knoinok/quansheng_toolkit/blob/ea13a5850d8f22372c7ad205db13610c083c4ee8/README.md

The toolkit repository is Apache-2.0. This audit records findings and links only; it does not copy its implementation.
