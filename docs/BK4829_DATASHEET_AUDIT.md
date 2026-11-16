# BK4829 datasheet audit and implementation boundary

Date: 2026-11-17
Project: `dob963/D10S-_2-FW_K1`, branch `main`.

## Primary source inspected

Beken, **DS-BK4829-E01 V1.0**, dated 2023-05-26:
https://manuals.plus/m/b03fb6dba016158ab9f4b5b3d1aeb1d9ea178efc44cc11c2c08398621553cdc9.pdf

The public datasheet verifies:
- RF operating bands 18–580 MHz and 760–1160 MHz.
- 26 MHz nominal crystal; tolerance ±2.5 ppm.
- 3-wire interface: SCK, SCN, bidirectional SDATA.
- Frame fields: R/W, A[6:0], D[15:0], MSB-first as drawn.
- Data is latched on SCK rising edge and output on SCK falling edge.
- Maximum SCK 8 MHz; minimum high/low pulse 25 ns.
- Read turnaround includes tNXT minimum 1 µs.
- PLL settle time is specified below 0.3 ms.

The public document does **not** publish a register programming table for:
- reset / power-up / RX mode sequence;
- frequency fractional-N word and divider programming;
- RSSI register address, bitfield, units or calibration;
- AFC register address, bitfield and signedness;
- LNA/IF gain and channel filter register values;
- glitch/settling status register.

Consequently the driver intentionally implements only a conservative API shell. No BK4819 register map or UV-K5 register sequence has been copied. `bk4829_init()`, `bk4829_set_freq()`, `bk4829_read_rssi()` and `bk4829_read_afc()` remain blocked TODOs. RSSI/AFC return zero until a verified map exists; this is not a working receiver driver.

## Board bus mapping

The UV-K1 V3 hardware audit identifies:
- PB8 = SCK
- PB9 = bidirectional SDATA
- PF9 = SCN/CSN

The supplied PA4–PA7 suggestion is not used. PA5/PA6/PA7 belong to the ST7565 display bus/control, and PB2 is display CS.

## PY32F071 register evidence

Puya PY32F07X Reference Manual:
https://www.puyasemi.com/download_path/%E7%94%A8%E6%88%B7%E6%89%8B%E5%86%8C/MCU%20%E5%BE%AE%E5%A4%84%E7%90%86%E5%99%A8/PY32F07X_Reference_Manual_V0.2.pdf

Verified addresses:
- RCC base 0x40021000; RCC_IOPENR offset 0x34.
- GPIOA 0x50000000, GPIOB 0x50000400, GPIOF 0x50001400.
- RCC_IOPENR bits: GPIOA bit 0, GPIOB bit 1, GPIOF bit 5.
- GPIO register offsets: MODER 0x00, OTYPER 0x04, OSPEEDR 0x08, PUPDR 0x0C, IDR 0x10, BSRR 0x18.

The current hardware init uses these documented memory addresses. It uses SysTick with a compile-time 8 MHz assumption; actual core clock after bootloader handoff remains to be measured/configured. Therefore millisecond delays are not yet certified for RF timing.

## Calibration policy

No calibration flash address is selected. A dedicated page cannot be chosen until the full K1 bootloader/application/data layout is mapped and the stock calibration area is excluded. `calib_load()` currently initializes zero offsets; `calib_save()` returns -1 and performs no flash erase/program operation. This is intentional fail-safe behavior.

## Next evidence required

1. Stock UV-K1 V3 SPI capture (power-up, RX tune, RSSI read) with timestamps and chip select.
2. Verified register documentation from Beken or a legally obtained stock firmware dump.
3. Confirmed crystal and system clock on the exact board revision.
4. Calibration backup, page map, and non-overlap proof before flash write support.

## Current implementation status

- Linker: present, application origin 0x08002800 from the upstream K1 linker layout; not yet proven as a raw-image flash contract by itself.
- GPIO/RF bus init: implemented for PB8/PB9/PF9; not electrically tested on the radio.
- BK4829 wire transaction: basic 3-wire framing scaffold; R/W polarity and read turnaround still require a logic-analyzer capture.
- BK4829 tuning/init/RSSI/AFC: intentionally unimplemented pending register evidence.
- Calibration: read-only safe fallback; flash persistence disabled.
- No complete firmware link, BIN packaging, or physical radio test has been performed.
