# Rapporto verifica build — UV-K1 V3 Spectrum

Data verifica: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`  
Branch: `main`  
Workflow: [43984129008](https://github.com/dob963/D10S-_2-FW_K1/actions/runs/43984129008)  
Commit verificato: `1b2ebbf908820fc14393ec1e04913dc9eb293bf9`

## Esito compilazione e test

GitHub Actions ha completato con esito `success` i test host, installazione toolchain, compilazione ARM, linking, generazione BIN raw e caricamento artefatti.

Configurazione:
- `arm-none-eabi-gcc`
- `-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft`
- `-std=c11 -Os`
- linker `ld/py32f071_app.ld`
- output ELF, BIN raw e MAP

Test host con `gcc -std=c11 -Wall -Wextra -Werror`:
- BK4829 adapter: PASS (tuning 0x38/0x39/0x30, RSSI 9 bit, glitch wait, bus read non valido, limiti banda)
- DSP no-cascade/Waterfall: PASS (median, picco isolato, assenza propagazione laterale, Waterfall disabilitato senza scritture)
- Calibrazione DC: PASS (offset default, saturazione negativa, limiti indice)

Metriche riportate dal linker:
- FLASH: 5.636 byte su 96 KiB (5,73%)
- RAM `.data + .bss`: 5.808 byte su 16 KiB (35,45%)
- `text`: 5.632 byte
- `data`: 4 byte
- `bss`: 5.804 byte
- stack riservato dal linker: 1.536 byte
- RAM residua dopo `.data/.bss` e riserva stack: 9.040 byte
- BIN raw: 5.636 byte

Artefatto GitHub Actions:
- Nome: `uv-k1-v3-spectrum-build`
- ID: `14283952896`
- [Artefatto della build](https://github.com/dob963/D10S-_2-FW_K1/actions/runs/43984129008/artifacts/14283952896)

## Warning

Nessun warning del compilatore C è stato riportato nella build. I warning di Node.js mostrati dal runner riguardano le action di GitHub e non il firmware.

## Funzioni incluse nella build

- Moduli DSP e stabilizzatore integrati; l'adattatore d'uscita mantiene il dominio 0..127 richiesto dall'interfaccia `int8_t` della reiezione spurie.
- Framebuffer ST7565 128x64 da 1.024 byte, font 5x7, rendering live, Peak Hold separato e Waterfall 16 livelli con dithering 4x4.
- Trasporto display bit-bang SPI mode 3, pin PA5/PA6/PA7/PB2, offset colonna +4, sequenza di init derivata come riferimento dal driver upstream K1 e reimplementata autonomamente.
- Aggiornamento display limitato con pausa minima di 67 ms, dipendente dal clock core di riferimento 8 MHz.
- Bus RF 3-wire BK4829 predisposto su PB8/PB9/PF9; turnaround tNXT=1 us dal datasheet, sintonia con REG 0x38/0x39/0x30, polling glitch REG 0x63 e lettura RSSI grezza REG 0x67, adattati da riferimenti software K1.
- Guardia di sintonia: accetta soltanto le bande BK4829 documentate 18-580 MHz e 760-1160 MHz.
- Calibrazione DC in RAM con aritmetica allargata e protezione limiti; offset predefiniti a zero.

## Limiti funzionali: non è un firmware pronto per la radio

La build e i test host provano la compilabilità e il comportamento software isolato, non il funzionamento fisico. In particolare:
- Il display ST7565 è compilato, ma non è stato testato sul pannello reale; contrasto, orientamento, timing e refresh richiedono verifica sulla specifica revisione PCB.
- Il driver BK4829 non è completo: la sequenza di init RF resta TODO; `bk4829_read_afc()` resta TODO e restituisce placeholder 0. Le letture/tuning derivano da sorgenti K1 che mantengono nomi BK4819 e devono essere confermate sul dump stock 7.03.01.
- `bk4829_init()` conserva REG 0x30 solo se la lettura è valida; non inizializza i registri RF.
- Il bus 3-wire è implementato secondo datasheet e sorgenti K1, ma non è stato verificato con logic analyzer sulla radio.
- La scansione keypad Fn1/Fn2/F-Lock non è implementata: i GPIO sono predisposti, gli eventi UI restano TODO.
- `calib_load()` usa ancora offset a zero; lettura/scrittura flash della calibrazione non implementate.
- Il clock core viene assunto a 8 MHz per i delay; va confermato che il bootloader 7.03.01 non lo riconfiguri prima del salto all'app.
- L'immagine prodotta è un BIN ARM raw, non un pacchetto DFU Quansheng verificato.
- Nessun test di avvio, ricezione, display o RF è stato eseguito sulla radio.

## Gate prima del flash

1. Ottenere e conservare il dump stock completo UV-K1 bootloader 7.03.01, inclusa la calibrazione.
2. Verificare formato di aggiornamento, offset e tabella vettori rispetto al bootloader installato. `0x08002800` è il riferimento upstream del progetto, non autorizza da solo il flash di un raw BIN.
3. Confermare la revisione PCB e i pin reali; il riferimento software K1 usa RF PB8/PB9/PF9 e LCD PA5/PA6/PA7/PB2.
4. Verificare init, sintonia, RSSI e AFC BK4829 tramite dump stock o misura controllata. Non assumere equivalenza BK4819 e non scrivere registri non verificati.
5. Testare il rendering ST7565 sul pannello reale e misurare refresh e clock effettivo.
6. Definire un'area calibrazione sicura e non sovrapposta a bootloader, applicazione o dati radio prima di implementare flash write.
7. Completare la scansione keypad e prove hardware di sensibilità, spurie, no-cascade e fedeltà della traccia live.
8. Validare il contenitore/DFU con il tool appropriato e mantenere il backup stock per il ripristino.

## Regola di rilascio

Il BIN da 5.636 byte è un artefatto di compilazione sperimentale e NON deve essere flashato sulla radio. Nessun file sarà etichettato come flashabile finché tutti i gate hardware, RF, display e bootloader non saranno superati.
