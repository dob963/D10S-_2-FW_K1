#!/usr/bin/env python3
"""Validate the raw application vector table for the UV-K1/PY32F071 layout.
This is a structural guard, not cryptographic proof of silicon identity and
not a DFU/container packer.
"""
import struct
import sys
from pathlib import Path

APP_BASE = 0x08002800
APP_LIMIT = APP_BASE + 96 * 1024
RAM_BASE = 0x20000000
RAM_LIMIT = 0x20004000
MAX_IMAGE = 96 * 1024


def fail(message: str) -> None:
    raise SystemExit(f"TARGET CHECK FAILED: {message}")


if len(sys.argv) != 2:
    fail("usage: check_uvk1_image.py firmware.bin")
path = Path(sys.argv[1])
data = path.read_bytes()
if len(data) < 8:
    fail("immagine troppo corta: manca la tabella vettori")
if len(data) > MAX_IMAGE:
    fail(f"immagine oltre il limite applicativo K1 ({len(data)} byte)")
sp, reset = struct.unpack_from("<II", data, 0)
if sp & 0x7:
    fail(f"initial SP non allineato a 8 byte: 0x{sp:08X}")
if not (RAM_BASE <= sp <= RAM_LIMIT):
    fail(f"initial SP fuori dalla RAM PY32F071: 0x{sp:08X}")
if not (reset & 1):
    fail(f"reset vector Thumb non impostato: 0x{reset:08X}")
reset_addr = reset & ~1
if not (APP_BASE <= reset_addr < APP_LIMIT):
    fail(f"reset vector fuori dall'app K1 a 0x{APP_BASE:08X}: 0x{reset:08X}")
print(f"TARGET CHECK PASS: {path.name}")
print(f"  image bytes : {len(data)}")
print(f"  initial SP  : 0x{sp:08X} (PY32F071 RAM range)")
print(f"  reset vector: 0x{reset:08X} (UV-K1 app range)")
print("  format      : raw application image; not DFU; no vendor header")
