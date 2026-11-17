# Build firmware Spectrum UV-K1 V3 / PY32F071.
# Raw application image at 0x08002800; no invented vendor/DFU header.
CROSS ?= arm-none-eabi-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
SIZE    := $(CROSS)size
PYTHON  ?= python3

TARGET := uvk1_spectrum
CPUFLAGS := -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft
CFLAGS := $(CPUFLAGS) -std=c11 -Os -ffunction-sections -fdata-sections \\
          -Wall -Wextra -Wno-unused-parameter -Iinc
ASFLAGS := $(CPUFLAGS) -x assembler-with-cpp
LDFLAGS := $(CPUFLAGS) -nostartfiles -Tld/py32f071_app.ld -Wl,--gc-sections \\
           -Wl,-Map=build/$(TARGET).map -Wl,--print-memory-usage \\
           --specs=nano.specs --specs=nosys.specs
LDLIBS := -lc -lgcc -lnosys

C_SOURCES := $(wildcard src/*.c)
ASM_SOURCES := startup/startup_py32f071xx.s
OBJECTS := $(patsubst src/%.c,build/%.o,$(C_SOURCES)) build/startup_py32f071xx.o

.PHONY: all clean size check-image
all: build/$(TARGET).bin

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/startup_py32f071xx.o: $(ASM_SOURCES) | build
	$(CC) $(ASFLAGS) -c $< -o $@

build/$(TARGET).elf: $(OBJECTS) ld/py32f071_app.ld
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@
	$(SIZE) $@

build/$(TARGET).bin: build/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
	$(PYTHON) tools/check_uvk1_image.py $@
	@echo "Creato raw application BIN K1/V3: $@ ($$(wc -c < $@) byte)"
	@echo "NOTA: non e' un contenitore DFU; nessun header inventato e' stato aggiunto."

check-image: build/$(TARGET).bin
	$(PYTHON) tools/check_uvk1_image.py $<

size: build/$(TARGET).elf
	$(SIZE) -A $<

clean:
	rm -rf build
