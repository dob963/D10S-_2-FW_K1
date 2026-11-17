# Build instructions and current release gate

## Toolchain

Install:
- GNU Make
- `arm-none-eabi-gcc`
- `arm-none-eabi-binutils`
- newlib nano runtime supplied by the GNU Arm Embedded package

Build:
```sh
make clean all
make size
```

Outputs:
- `build/spectrum_k1_v3.elf`
- `build/spectrum_k1_v3.bin` (raw application image linked for 0x08002800)
- `build/spectrum_k1_v3.map`

The Makefile enforces a 98,304-byte raw image ceiling as a conservative limit matching the K1 DFU toolchain profile. The linker reserves 96 KiB for the application.

## Flash warning

A raw application BIN is not necessarily the same thing as a Quansheng packed DFU container. The toolkit distinguishes raw ARM images from encoded containers and performs target/bootloader checks. Do not rename the raw file and flash it blindly. First identify the radio as UV-K1, read bootloader 7.03.01, back up calibration/configuration and use a compatible DFU tool.

## Current limitations

This is an experimental raw application build, not a release firmware:
- BK4829 init values and AFC-disable bit are taken from the UV-K1 target driver source at the documented commit; AFC offset measurement remains TODO and returns neutral zero. The target silicon/register map still requires hardware confirmation.
- ST7565 framebuffer, full-page blit, 5x7 text, spectrum bars and 4x4-dither waterfall are implemented. **Hardware RES is not mapped**, so the required reset pulse cannot yet be generated; the unverified software E2 reset command has been removed. Verify RES on the exact PCB before calling LCD initialization complete.
- Keypad event scan remains TODO.
- HSI/core clock handoff is not verified; SysTick delay currently assumes 8 MHz.
- Calibration flash persistence is intentionally disabled.
- No RF/display hardware validation or flash test has been performed.

A successful CI build proves only that the source compiles and links. It does not prove RF operation, display operation, bootloader acceptance or safe flashing.
