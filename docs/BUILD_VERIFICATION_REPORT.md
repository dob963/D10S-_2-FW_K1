# Rapporto verifica build — UV-K1 V3 Spectrum

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`  
Branch: `main`  
Commit verificato: `ad81d3d49ff70d887e6dab92b9f8c8d1f20a9303`

## Esito CI

**SUCCESS** — GitHub Actions run [44050742729](https://github.com/dob963/D10S-_2-FW_K1/actions/runs/44050742729).

Job `build` completato con esito SUCCESS. Step superati:
- test host driver BK4829;
- test DSP no-cascade e isolamento waterfall;
- test mapping sweep;
- test regressione peak tracker;
- installazione ARM GCC;
- compilazione immagine applicativa raw;
- caricamento artefatti.

Questi test verificano i moduli simulati e la compilazione; non sostituiscono la prova sulla radio fisica.

## Artefatto corrente

- Nome: `uv-k1-v3-spectrum-build`
- ID: `14313487292`
- SHA-256 ZIP: `2e54df616cae83b3c7211d9908c4acae59ab934567648669f55c0a48fd473e1e`
- BIN: 8.632 byte
- ELF: 93.680 byte
- MAP: 48.552 byte
- SHA-256 BIN: `b184693cd52e4e967e3a9fb67c42e82b2f0081c36da5d5c987da793ac7b5c05a`

## Verifica statica immagine

- Target ELF: ARM 32-bit little-endian EABI5, Cortex-M0+, soft-float.
- Origine applicazione e tabella vettori: `0x08002800`, come definito in `ld/py32f071_app.ld`.
- Primo vettore: stack iniziale `0x20004000`, top dei 16 KiB RAM.
- Reset vector: `0x08004775` (bit Thumb impostato); indirizzo handler `0x08004774`, interno all'immagine.
- Fine immagine BIN: `0x080049B8`, entro il limite FLASH applicativo conservativo del linker.
- RAM: `.bss` termina a `0x200013B4`; riserva stack 1.536 byte con limite `0x20003A00`; ASSERT linker superato.
- Dimensione trasporto DFU: 34 pagine da 256 byte; ultima pagina con 184 byte utili, completata con padding dal protocollo.

La verifica statica dimostra coerenza strutturale di ELF/BIN e linker; non prova l'avvio del firmware sul dispositivo.

## Formato BIN e DFU

Il file prodotto da `make` è un'immagine applicativa ARM raw, non un contenitore di aggiornamento vendor con intestazione/versione/CRC.

La documentazione e il codice pubblico di Quansheng Toolkit descrivono il flashing UV-K1 tramite protocollo DFU seriale a pagine da 256 byte e prevedono anche la selezione di un file `Custom .bin`. Quindi il fatto che il file non sia un pacchetto DFU autonomo non lo rende automaticamente incompatibile con il flusso DFU: è il programma flasher a incapsulare i byte raw nei messaggi DFU.

La stessa documentazione avverte che i BIN personalizzati non ricevono la stessa garanzia anti-brick dei firmware bundled. La compatibilità della famiglia bootloader UV-K1 7.03.01 e del trasporto è documentata; l'accettazione e l'avvio di QUESTO specifico BIN non sono stati provati su radio reale.

Riferimento tecnico: https://github.com/knoinok/quansheng_toolkit (README e `kradio/dfu.py`).

## Blocchi aperti — non certificato per uso radio

1. Nessuna sessione DFU è stata eseguita sul dispositivo dell'utente; nessun ACK reale del bootloader verificato.
2. Nessun avvio fisico del BIN, né verifica display/tastiera/RF.
3. Clock 8 MHz e handoff dal bootloader 7.03.01 restano assunzioni da provare.
4. Il keypad è implementato sulla mappa GPIO pubblicata, ma non è stato collaudato sulla specifica revisione PCB.
5. ST7565: init e rendering compilano; manca verifica visiva/elettrica sul pannello.
6. RF/LNA: comportamento ricostruito da sorgente UV-K1 di riferimento; sensibilità e registri richiedono misure RF.
7. AFC offset non validato; rigetto spur basato su AFC resta disabilitato.
8. Calibrazione: nessuna area viene scritta dal firmware; backup e recovery devono essere salvati e verificati prima di qualsiasi aggiornamento.
9. Licenza complessiva delle contribuzioni originali non dichiarata: repository privo di LICENSE/NOTICE radice.

## Decisione

**Build CI e verifiche statiche superate. Formato raw compatibile in linea generale con il flusso Custom BIN descritto dal toolkit, ma firmware NON certificato per il flash o per il funzionamento sulla radio.** Non procedere senza backup verificato della calibrazione e una procedura di recovery disponibile per il modello esatto.
