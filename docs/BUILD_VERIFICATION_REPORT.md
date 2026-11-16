# Rapporto verifica build — UV-K1 V3 Spectrum

Data verifica: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`  
Branch: `main`  
Workflow run: [43976416935](https://github.com/dob963/D10S-_2-FW_K1/actions/runs/43976416935)  
Commit sorgente del workflow: `05060e65850067faa48e1f31068c587635f5e08c`

## Esito della compilazione

GitHub Actions ha completato con esito `success` i passaggi di installazione toolchain, compilazione, linking, generazione BIN e caricamento artefatti.

Configurazione osservata:
- `arm-none-eabi-gcc`
- `-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft`
- `-std=c11 -Os`
- linker script `ld/py32f071_app.ld`
- output ELF, BIN raw e MAP

Metriche riportate dal linker:
- FLASH: 2.280 byte su 96 KiB (2,32%)
- RAM: 4.508 byte su 16 KiB (27,51%)
- ELF: text 2.280, data 0, bss 4.508 byte
- BIN raw: 2.280 byte

Artefatto GitHub Actions:
- Nome: `uv-k1-v3-spectrum-build`
- ID: `14279979490`
- SHA-256 ZIP: `70af8fe7a61c16a167d1767bee36a4aa6534ca5dd02e64e9af9ec785419470e2`

## Avvertenza fondamentale

Questa è una verifica di compilazione e linking, NON una verifica funzionale sulla radio. Il BIN generato è uno scaffold minimale e NON deve essere flashato sul dispositivo.

Il log stesso segnala che driver BK4829 e rendering ST7565 non sono completi/verificati. Il driver RF corrente non esegue sintonia né lettura RSSI/AFC reali; le funzioni di bus RF sono stub. La UI non disegna ancora lo spettro sul display.

## Warning di compilazione

È presente un warning `-Wmisleading-indentation` in `src/agc_sw.c` sulla riga con due istruzioni `if` sulla stessa riga. Non blocca la build e il modulo DSP è stato lasciato invariato come richiesto.

## Verifiche statiche sull'immagine

L'immagine BIN scaricata è riconosciuta come ARM Cortex-M firmware. La tabella vettori indica:
- stack iniziale: `0x20004000` (fine dei 16 KiB SRAM)
- reset handler: `0x08003098`

Il reset handler è coerente con un'immagine linkata a partire dall'offset configurato nel linker, ma ciò NON dimostra che l'offset sia quello corretto per il bootloader installato né che il contenitore DFU sia compatibile.

## Gate ancora aperti prima di qualsiasi test RF/flash

1. Ottenere dump completo e verificato del firmware stock UV-K1 V3 e preservare la calibrazione.
2. Confermare offset applicazione e layout bootloader 7.03.01 con strumenti e immagine stock reali.
3. Acquisire protocollo e registri BK4829: dump stock, cattura logic analyzer o documentazione tecnica attendibile. Non usare registri BK4819.
4. Implementare e verificare bus BK4829, sintonia, RSSI e AFC.
5. Implementare il renderer ST7565 sul pinout K1 V3 verificato.
6. Verificare tabella vettori PY32F071, clock, GPIO e tempi sul dispositivo.
7. Correggere l'adattamento dei dati DSP senza riscrivere gli algoritmi forniti; in particolare verificare la conversione `int16_t` verso `int8_t` nel rejection stage.
8. Aggiungere test host per sweep, coerenza spurie, saturazioni, stabilizzatore e assenza di cascata.
9. Generare e validare il formato di aggiornamento Quansheng appropriato; il BIN raw non equivale a un DFU flashabile.

## Regola di rilascio

Nessun file va etichettato come firmware flashabile finché tutti i gate hardware, bootloader, RF e display non sono superati e il backup stock non è disponibile.
