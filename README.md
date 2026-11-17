# D10S Spectrum — Quansheng UV-K1 V3

Firmware sperimentale di analizzatore Spectrum autonomo per:
- MCU PY32F071 (Cortex-M0+), 128 KiB Flash / 16 KiB RAM
- Front-end RF target BK4829
- Display monocromatico ST7565 128×64
- Bootloader UV-K1 7.03.01

## Funzioni implementate

- Sweep uniforme su 128 bin con copertura degli estremi.
- Filtro mediano su bin adiacenti.
- Scala RSSI grafica fissa; nessuna RBW dichiarata senza calibrazione.
- Noise floor percentile 20%.
- Conferma picchi su due sweep consecutivi e interpolazione parabolica sub-bin.
- Peak hold e indicazione frequenza del picco.
- Avvio esplicito RX del BK e filtro RF wide 25 kHz.
- Backlight PF8 attivata dopo l'inizializzazione ST7565.
- HSI forzato a 8 MHz e VTOR spostato all'offset applicativo `0x08002800`.
- Matrice keypad UV-K1 con debounce e controlli Spectrum.

## Controlli

- **UP / DOWN**: centro frequenza ±25 kHz.
- **MENU**: ciclo span 0,5 / 1 / 2 / 4 MHz.
- **Tasti laterali**: span precedente / successivo.
- **F**: abilita/disabilita Peak Hold.
- **STAR**: azzera Peak Hold.
- **EXIT**: centro 146 MHz e span 4 MHz.

La regolazione è attualmente limitata alla banda iniziale 144–148 MHz; non si presume che ogni segmento dichiarato dal datasheet sia già validato dal driver.

## Build

Eseguire: make clean all

Output: build/spectrum_k1_v3.bin, immagine ARM raw all'offset applicativo definito dal linker. Non è un contenitore DFU Quansheng e non deve essere rinominato come tale.

## Stato e sicurezza

I test host e la build CI verificano compilazione, mapping sweep, driver simulato e DSP. Non sostituiscono la prova sulla radio reale. Il clock e la tabella vettori sono inizializzati dal firmware secondo i registri Puya; l'effettivo handoff dal bootloader, RF, display, keypad, calibrazione e procedura DFU/recovery restano soggetti a verifica fisica e backup completo.

**Non flashare il BIN raw finché bootloader, formato e percorso di recupero non sono stati verificati sul dispositivo esatto.** Non scrivere nelle aree di calibrazione o riservate.

Fonti e stato licenze: docs/THIRD_PARTY.md.
