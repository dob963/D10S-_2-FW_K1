# Rapporto verifica build — UV-K1 V3 Spectrum

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`, branch `main`

## Stato build

Ultima build ARM del codice renderer/RF completata con esito SUCCESS:
- Run: https://github.com/dob963/D10S-_2-FW_K1/actions/runs/44041340931
- Commit compilato: `2fbe97b897b2b56057c369dba24dd44654c735ec`
- Toolchain: `arm-none-eabi-gcc`, `-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft`
- Test host BK4829, DSP/no-cascade/waterfall e sweep: PASS
- Dimensione artefatto verificata: raw BIN 6.688 byte; `.data` 4 byte + `.bss` 6.060 byte = 6.064 byte RAM. Mappa linker: `_ebss=0x200017B0`, limite stack `0x20003A00`, stack riservato 1.536 byte.

Il commit `2fbe97b` include la rimozione del comando software ST7565 `0xE2`. La build ARM e tutti i test host sullo stesso commit sono terminati con esito SUCCESS. Il commit successivo `70c245c` aggiorna soltanto documentazione.

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


## Ricerca schema elettrico / reset ST7565 — 2026-11-17

Ricerca pubblica eseguita su GitHub e sul materiale di teardown Quansheng K1:
- Repository teardown: https://github.com/armel/k1-teardown
- Albero dei file: https://github.com/armel/k1-teardown/tree/main
- Contiene README, datasheet BK4829/PY32F071, dump firmware e fotografie PCB `K1_VIEW_*.png`; non è presente uno schema elettrico completo o una netlist pubblicata.
- Il README identifica K1 con PY32F071, 128 KiB Flash e 16 KiB SRAM.

**Evidenza importante sul reset LCD:** nel sorgente target UV-K1 upstream
`App/driver/st7565.c`, funzione `ST7565_Init()` e funzione
`ST7565_HardwareReset()`, il commento dice esplicitamente che sul K1
non esiste un pin hardware reset utilizzabile; `ST7565_HardwareReset()`
è un no-op. La routine upstream invia poi il comando ST7565 software reset
`0xE2`. File:
https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/main/App/driver/st7565.c

Questo è un dato software del firmware di riferimento, non una misura
elettrica e non dimostra se il pin RES del controller sia legato a VDD,
a un reset comune o lasciato non connesso. Non è stata trovata evidenza
sufficiente per assegnare RES a un GPIO PY32F071. **Non collegare né
pilotare un GPIO ipotetico.**

Il progetto locale mantiene quindi il TODO hardware e non inventa un pin.
La scelta di non inviare `0xE2` segue il requisito specifico di questo
progetto, ma differisce dall'inizializzazione upstream; di conseguenza
la funzionalità del display non può essere considerata confermata finché
non si verifica lo schema/net della revisione PCB oppure non si autorizza
una sequenza di reset alternativa compatibile con l'hardware effettivo.

Per chiudere con evidenza: occorre ottenere lo schema di servizio dal
produttore oppure ispezionare le fotografie teardown ad alta risoluzione
e misurare la continuità tra RES del pannello, GPIO MCU, VDD e reset comune.
