# Build firmware Spectrum UV-K1 V3 / PY32F071.
# Il risultato .bin e' un'immagine ARM raw a partire dall'offset applicazione.
# Non e' un contenitore DFU Quansheng automaticamente impacchettato.
CROSS ?= arm-none-eabi-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
SIZE    := $(CROSS)size

CPUFLAGS := -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft
CFLAGS := $(CPUFLAGS) -std=c11 -Os -ffunction-sections -fdata-sections \
          -Wall -Wextra -Wno-unused-parameter -Iinc
ASFLAGS := $(CPUFLAGS) -x assembler-with-cpp
LDFLAGS := $(CPUFLAGS) -nostartfiles -Tld/py32f071_app.ld -Wl,--gc-sections \
           -Wl,-Map=build/spectrum_k1_v3.map -Wl,--print-memory-usage \
           --specs=nano.specs --specs=nosys.specs
LDLIBS := -lc -lgcc -lnosys

C_SOURCES := $(wildcard src/*.c)
ASM_SOURCES := startup/startup_py32f071xx.s
OBJECTS := $(patsubst src/%.c,build/%.o,$(C_SOURCES)) build/startup_py32f071xx.o

.PHONY: all clean size
all: build/spectrum_k1_v3.bin

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/startup_py32f071xx.o: $(ASM_SOURCES) | build
	$(CC) $(ASFLAGS) -c $< -o $@

build/spectrum_k1_v3.elf: $(OBJECTS) ld/py32f071_app.ld
	$(CC) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@
	$(SIZE) $@

build/spectrum_k1_v3.bin: build/spectrum_k1_v3.elf
	$(OBJCOPY) -O binary $< $@
	@bytes=$$(wc -c < $@); \
	if [ $$bytes -gt 98304 ]; then \
	  echo "ERRORE: BIN $$bytes byte supera il limite prudenziale K1 di 98304 byte"; \
	  rm -f $@; exit 1; \
	fi
	@echo "Creato raw application BIN: $@ ($$(wc -c < $@) byte)"
	@echo "ATTENZIONE: driver BK4829 e rendering ST7565 non sono ancora completi/verificati."

size: build/spectrum_k1_v3.elf
	$(SIZE) -A $<

clean:
	rm -rf build
