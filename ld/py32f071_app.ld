/*
 * Linker script indipendente per UV-K1 standard / PY32F071.
 *
 * Layout applicazione derivato dal linker upstream Armel:
 *   FLASH ORIGIN = 0x08002800, LENGTH = 96 KiB (limite prudenziale DFU K1)
 *   RAM          = 0x20000000, LENGTH = 16 KiB
 *
 * Il toolkit Quansheng documenta la combinazione UV-K1 + bootloader 7.03.01
 * + F4HWN Fusion 5.4 come validata. L'indirizzo 0x08002800 e' quindi il
 * riferimento upstream per questo target, non una garanzia che qualunque
 * immagine raw o pacchetto DFU sia intercambiabile.
 *
 * Non sovrascrivere dati/calibrazione: le relative aree devono essere
 * definite separatamente dopo verifica del layout DFU e della radio.
 *
 * Script scritto ex novo; non e' una copia del linker upstream Puya.
 */

ENTRY(Reset_Handler)

_Min_Stack_Size = 0x600;
_estack = ORIGIN(RAM) + LENGTH(RAM);

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08002800, LENGTH = 96K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 16K
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(128);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    .noinit (NOLOAD) :
    {
        . = ALIGN(4);
        *(.noinit)
        *(.noinit*)
        . = ALIGN(4);
    } > RAM

    . = ALIGN(8);
    __StackTop = _estack;
    __StackLimit = _estack - _Min_Stack_Size;

    ASSERT(ADDR(.isr_vector) == 0x08002800,
           "Vector table non allineata all'origine applicazione UV-K1")
    ASSERT((_ebss <= __StackLimit),
           "RAM overflow: .data/.bss invade la riserva stack")
    ASSERT((LOADADDR(.data) + SIZEOF(.data)) <= (ORIGIN(FLASH) + LENGTH(FLASH)),
           "FLASH overflow: immagine applicazione oltre 96 KiB (limite pacchetto K1 documentato)")

    /DISCARD/ :
    {
        *(.note*)
        *(.comment*)
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
