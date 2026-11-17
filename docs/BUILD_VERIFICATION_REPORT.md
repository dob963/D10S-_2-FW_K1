# Rapporto verifica build — UV-K1 V3 Spectrum

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`, branch `main`

## Stato build

Ultima build ARM completata con esito SUCCESS:
- Run: https://github.com/dob963/D10S-_2-FW_K1/actions/runs/44040415949
- Commit compilato: `4b8fd0b34d61105bf0fe8974df2a9a6fbf384f33`
- Toolchain: `arm-none-eabi-gcc`, `-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft`
- Test host BK4829, DSP/no-cascade/waterfall e sweep: PASS
- Dimensione riportata dal job: raw BIN 6.692 byte; RAM statica 6.064 byte (da log del job).

Modifiche successive al display:
- Commit `bf6defd` / `55c09e5`: rimosso il comando software ST7565 `0xE2`, non ammesso dal requisito di reset hardware tramite RES.
- Test host sul commit successivo: PASS.
- La nuova build ARM per il commit sorgente `55c09e5` e per il ramo aggiornato `2fbe97b` è in corso; finché non termina non attribuire a questi commit l'esito ARM SUCCESS.

## Pinout documentato

- RF: PB8=SCK, PB9=SDIO bidirezionale, PF9=SCN/CSN.
- ST7565: PA5=SCK, PA7=MOSI, PA6=A0, PB2=CS.
- Fonte: driver target UV-K1 upstream `App/driver/bk4829.c`, `App/driver/st7565.c`, e audit `docs/UPSTREAM_PINOUT_VERIFICATION.md`.
- I bus RF e LCD sono separati. La mappa fisica della singola revisione PCB richiede ancora verifica elettrica.

## Driver BK4829

- Sequenza init basata sul file target UV-K1 `App/driver/bk4829.c`, commit upstream `3261ca7883d5b0b60c0905d8766612dee7b4191d`, funzione `BK4819_Init()`, righe 100–201.
- REG 0x19 = 0x1041, fonte: stesso file, riga 186.
- AFC disable: REG 0x73 bit 4; la sequenza scrive 0x4691, bit 4 impostato. `bk4829_read_afc()` restituisce 0 neutro; `bk4829_afc_is_valid()` restituisce 0. Il DSP non usa la misura AFC e la pipeline non abilita la tripla coerenza basata su AFC.
- Il dump stock 7.03.01 verificato ha SHA-256 `55ec0daffc5668bdb41dcc118475d7e6e23ad953d70f57a9bca64a6202734ba0`. La ricerca byte-level non ha dimostrato autonomamente la scrittura REG 0x19; quel valore è documentato dal sorgente F4HWN, non spacciato per disassemblaggio stock.
- Licenza upstream registrata: Apache-2.0; implementazione locale indipendente e inventario delle fonti in `docs/BK4829_REGISTER_EVIDENCE.md`.

## Display e rendering

Implementati:
- framebuffer ST7565 128x64, 1.024 byte, 8 pagine;
- blit completo di 1.024 byte;
- font 5x7 per frequenza, span e RBW;
- barre spettro live e Peak Hold separato;
- waterfall 16 righe con dithering Bayer 4x4;
- frame pacing nominale di 67 ms (circa 15 fps), subordinato al clock reale.

**TODO hardware bloccante:** il pin fisico RES non è identificato nelle fonti disponibili. Il comando software E2 è stato rimosso; senza un impulso hardware RES l'inizializzazione del controller non è garantita. Individuare RES su schema/PCB della revisione esatta e integrarlo solo con evidenza del pin.

## Linker e memoria

- FLASH applicazione: 0x08002800, limite prudenziale immagine 96 KiB.
- RAM: 0x20000000, 16 KiB.
- Stack riservato: 1.536 byte.
- Offset 0x08002800 deriva dal linker upstream UV-K1; non dimostra che un raw BIN sia un pacchetto DFU valido.

## Limiti e sicurezza

- Nessuna prova elettrica, RF o display eseguita sulla radio fisica.
- Clock core assunto 8 MHz per SysTick; da verificare sul target.
- Keypad event scan ancora TODO.
- Backup calibrazione/configurazione obbligatorio prima di qualsiasi flash.
- Il raw BIN non va flashato direttamente né rinominato come DFU. Prima verificare formato immagine, bootloader, vettori e strumento DFU compatibile.
- Non definire il firmware pronto all'uso finché RES, pinout della scheda, RF, bootloader e ripristino calibrazione non sono stati verificati su hardware.
