# Rapporto verifica build — UV-K1 V3 Spectrum

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`  
Branch: `main`  
Build verificata: commit `c104a8e884fba3c5fdde957fc01041aa64f79ff5`  
Head successivo del ramo: `bb40109745f8a2c7b3823ff45a3fcccc93e15a0b` (solo chiarimento commento sul ritardo RF; nessuna modifica binaria).

## Esito CI

**SUCCESS** — GitHub Actions run [44056550974](https://github.com/dob963/D10S-_2-FW_K1/actions/runs/44056550974).

Tutti gli step sono terminati con esito SUCCESS:
- test host driver BK4829;
- test DSP no-cascade e isolamento waterfall;
- test mapping sweep;
- test stabilizzatore per segnali deboli e decadimento;
- test peak tracker;
- installazione ARM GCC;
- compilazione immagine applicativa;
- caricamento artefatti.

## Artefatto corrente

- Nome: `uv-k1-v3-spectrum-build`
- ID: `14316181679`
- SHA-256 ZIP: `163357cbc2cd718485659a370801cd60ddaf541b640ce57d89cb910d7b15ab5f`
- BIN: **8.820 byte**
- ELF: 93.704 byte
- MAP: 48.570 byte
- SHA-256 BIN: `58097bc4554c5177c8552ed3750e4ce8836f5d6203e35d948bc78b4e84b8698a`

## Verifica statica immagine

- Target ELF: ARM 32-bit little-endian EABI5, Cortex-M0+, soft-float.
- Origine applicazione e tabella vettori: `0x08002800`.
- Sezione `.isr_vector`: `0x08002800`, dimensione `0xC0`, allineamento 128 byte.
- Primo vettore: stack iniziale `0x20004000`, top dei 16 KiB RAM.
- Reset vector: `0x0800483D` (bit Thumb impostato); handler `0x0800483C`, interno all'immagine.
- Fine immagine BIN: `0x08004A74`, entro la regione FLASH applicativa.
- `.bss`: `0x20000008`, dimensione `0x13A8`, fine `0x200013B0`; riserva stack 1.536 byte e ASSERT linker superato.
- Trasporto DFU: **35 pagine da 256 byte**; ultima pagina con 116 byte utili, completata con padding dal protocollo.

La verifica statica dimostra coerenza strutturale di ELF/BIN e linker; non prova l'avvio del firmware sul dispositivo.

## Correzioni debug incluse in questa build

1. Corretto il layout UI: la riga NF/PK non viene più sovrascritta dal Waterfall.
2. Corretto lo stabilizzatore: segnali deboli e incrementi graduali non restano bloccati sotto la soglia di salto.
3. Corretta la selezione LNA: soglia documentata 28 MHz, non 280 MHz; 146 MHz seleziona UHF GPIO3, 27 MHz VHF GPIO4.
4. Corretta la lettura seriale RF: SDA viene campionato nella fase SCL bassa; transazioni e ritardi sono allineati al comportamento documentato UV-K1.
5. Aggiunto avvio esplicito RX: REG37=`0x9F1F`, REG30=`0x0000` poi `0xBFF1`; per lo sweep AF DAC disabilitato con REG30=`0xBDF1`.
6. Impostato il filtro RF wide tramite REG43=`0x3028`.
7. Inizializzazione PY32F071: HSI forzato a 8 MHz, HCLK /1 e VTOR spostato a `0x08002800`.
8. Backlight LCD PF8 attivato dopo l'inizializzazione ST7565.

## Formato BIN e DFU

Il file prodotto da `make` è un'immagine applicativa ARM raw, non un contenitore di aggiornamento vendor con intestazione/versione/CRC.

La documentazione e il codice pubblico di Quansheng Toolkit descrivono il flashing UV-K1 tramite protocollo DFU seriale a pagine da 256 byte e prevedono anche la selezione di un file `Custom .bin`. Il flasher incapsula i byte raw nei messaggi DFU; il file non deve essere rinominato come contenitore `.dfu`.

La stessa documentazione avverte che i BIN personalizzati non ricevono la stessa garanzia anti-brick dei firmware inclusi. La compatibilità della famiglia bootloader UV-K1 7.03.01 e del trasporto è documentata; l'accettazione e l'avvio di QUESTO specifico BIN non sono stati provati su radio reale.

Riferimento: https://github.com/knoinok/quansheng_toolkit (README e `kradio/dfu.py`).

## Blocchi aperti — non certificato per uso radio

1. Nessuna sessione DFU è stata eseguita sul dispositivo; nessun ACK reale del bootloader verificato.
2. Nessun avvio fisico del BIN, né verifica display/tastiera/RF.
3. HSI/VTOR sono configurati nel codice, ma l'handoff effettivo del bootloader 7.03.01 va verificato sulla radio.
4. Keypad e ST7565 sono implementati sulla mappa pubblicata; manca il collaudo sulla specifica revisione PCB.
5. RF/LNA, sensibilità e stabilità RSSI richiedono misure con generatore RF e carico/attenuatore.
6. AFC offset non validato; rigetto spur basato su AFC resta disabilitato.
7. Nessuna calibrazione viene scritta; backup e recovery devono essere salvati e verificati prima di qualsiasi aggiornamento.
8. Licenza complessiva delle contribuzioni originali non dichiarata: repository privo di LICENSE/NOTICE radice.

## Decisione

**Build CI, test host e verifiche statiche superati. Formato raw coerente con il flusso Custom BIN documentato. Firmware NON certificato per il flash o per il funzionamento sulla radio.** Non procedere senza backup verificato della calibrazione e una procedura di recovery disponibile per il modello esatto.
