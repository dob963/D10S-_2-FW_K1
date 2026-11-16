# Rapporto verifica build — UV-K1 V3 Spectrum

Data verifica: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`  
Branch: `main`  
Workflow: [43982994745](https://github.com/dob963/D10S-_2-FW_K1/actions/runs/43982994745)  
Commit del workflow: `c73733dbb58cb333108c447fcded1722d47d83b0`  
Codice ST7565 incluso: `5f85afa06f545d58175d578d9e4744d0fd7cedcc`

## Esito compilazione

GitHub Actions ha completato con esito `success` installazione toolchain, compilazione, linking, generazione BIN raw e caricamento artefatti.

Configurazione:
- `arm-none-eabi-gcc`
- `-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft`
- `-std=c11 -Os`
- linker `ld/py32f071_app.ld`
- output ELF, BIN raw e MAP

Metriche riportate dal linker:
- FLASH: 5.024 byte su 96 KiB (5,11%)
- RAM `.data + .bss`: 5.804 byte su 16 KiB (35,42%)
- `text`: 5.020 byte
- `data`: 4 byte
- `bss`: 5.800 byte
- stack riservato dal linker: 1.536 byte
- RAM residua dopo `.data/.bss` e riserva stack: 9.044 byte
- BIN raw: 5.024 byte

Artefatto GitHub Actions:
- Nome: `uv-k1-v3-spectrum-build`
- ID: `14285100231`
- [Artefatto della build](https://github.com/dob963/D10S-_2-FW_K1/actions/runs/43982994745/artifacts/14285100231)

## Warning

Nessun warning del compilatore C è stato riportato nella build. È stato rimosso il precedente warning `-Wmisleading-indentation` in `src/agc_sw.c` separando due istruzioni `if` su righe distinte, senza modificare la logica AGC.

## Funzioni incluse nella build

- Moduli DSP e stabilizzatore integrati; l'adattatore d'uscita mantiene il dominio 0..127 richiesto dall'interfaccia `int8_t` della reiezione spurie.
- Framebuffer ST7565 128x64 da 1.024 byte, font 5x7, rendering live, Peak Hold separato e Waterfall 16 livelli con dithering 4x4.
- Trasporto display bit-bang SPI mode 3, pin PA5/PA6/PA7/PB2, offset colonna +4, sequenza ST7565 derivata come riferimento dal driver upstream K1 e reimplementata autonomamente.
- `hw_init()` inizializza GPIO e richiama `st7565_init()`.

## Limiti funzionali: non è un firmware pronto per la radio

La build prova la compilabilità, non il funzionamento fisico. In particolare:
- Il trasporto ST7565 è compilato, ma non è stato testato sul display reale; contrasto, orientamento e tempi richiedono verifica sulla specifica revisione PCB.
- Il driver BK4829 non è completo: `bk4829_init()`, tuning, RSSI e AFC restano stub. Il bus 3-wire è predisposto, ma non validato con logic analyzer o confronto stock.
- `sweep_adaptive()` non può ancora produrre misure RF reali perché il driver restituisce valori stub.
- `calib_load()` inizializza ancora offset a zero; lettura/scrittura flash della calibrazione non implementate.
- L'immagine prodotta è un BIN ARM raw, non un pacchetto DFU Quansheng verificato.
- Nessun test di avvio, ricezione, display o RF è stato eseguito sulla radio.

## Gate prima del flash

1. Ottenere e conservare il dump stock completo UV-K1 bootloader 7.03.01, inclusa la calibrazione.
2. Verificare formato di aggiornamento, offset e tabella vettori rispetto al bootloader installato. `0x08002800` è il riferimento upstream del progetto, non autorizza da solo il flash di un raw BIN.
3. Confermare la revisione PCB e i pin reali; il riferimento software K1 usa RF PB8/PB9/PF9 e LCD PA5/PA6/PA7/PB2.
4. Verificare la sequenza di tuning/RSSI/AFC BK4829 tramite dump stock o misura controllata. Non assumere equivalenza BK4819 e non scrivere registri non verificati.
5. Testare il rendering ST7565 sul pannello reale e misurare il rate di aggiornamento.
6. Definire un'area calibrazione sicura e non sovrapposta a bootloader, applicazione o dati radio prima di implementare flash write.
7. Aggiungere test host DSP/sweep e prove hardware di sensibilità, spurie, no-cascade e assenza di deformazione della traccia live.
8. Validare il contenitore/DFU con il tool appropriato e mantenere il backup stock per il ripristino.

## Regola di rilascio

Il BIN da 5.024 byte è un artefatto di compilazione sperimentale e NON deve essere flashato sulla radio. Nessun file sarà etichettato come flashabile finché tutti i gate hardware, RF, display e bootloader non saranno superati.
