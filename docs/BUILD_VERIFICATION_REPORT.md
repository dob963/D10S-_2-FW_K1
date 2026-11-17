# Rapporto verifica build — UV-K1 V3 Spectrum

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`  
Branch: `main`  
Commit compilato e verificato: `258194824b2c7b4dece78a84190de7d960e521f1`

## Esito CI

**SUCCESS** — GitHub Actions run #178:
https://github.com/dob963/D10S-_2-FW_K1/actions/runs/44048608935

Tutti gli step sono terminati con esito SUCCESS sul commit `258194824b2c7b4dece78a84190de7d960e521f1`:
- test BK4829 adapter;
- test DSP no-cascade e isolamento Waterfall;
- test mapping sweep;
- test noise floor, conferma picchi e interpolazione;
- compilazione ARM GCC e upload artefatti.

## Artefatti

Artefatto GitHub Actions: `uv-k1-v3-spectrum-build`  
ID: `14311639205`  
Dimensione archivio ZIP: 30.840 byte  
Scadenza indicata da GitHub: 2027-02-15  
SHA-256 ZIP: `e6b877c69af0ffb876875edc2078f5d8b76ea6ccfa0aa33f8aa02c92f7050463`  
SHA-256 BIN: `c02a561d4383d7356c84abc18f5f13ae4e8b42cbd6ee02648ee5f20f91218686`

Contenuti:
- `spectrum_k1_v3.bin`: 8.624 byte
- `spectrum_k1_v3.elf`: 93.732 byte
- `spectrum_k1_v3.map`: 48.770 byte

Il BIN è un'immagine ARM raw all'offset applicativo previsto dal linker. **Non è un contenitore DFU Quansheng** e non va rinominato come DFU.

## Memoria

Dalla mappa linker:
- FLASH vettori: 192 byte
- `.text`: 7.932 byte
- `.rodata`: 484 byte
- `.data`: 8 byte
- `.bss`: 5.036 byte
- fine `.bss`: `0x200013B4`
- limite inferiore stack riservato: `0x20003A00`
- stack riservato: 1.536 byte

La verifica ASSERT del linker per la separazione RAM/stack è passata.

## Correzioni software incluse

1. Sweep uniforme con un campione reale per ogni bin e inclusione esatta degli estremi banda.
2. Rimossi i buchi creati dalla scansione adattiva che saltava bin o terminava prima della fine banda.
3. Mediana calcolata su bin realmente adiacenti, senza compattare campioni distanti.
4. Rimosso l'AGC software che modificava il guadagno a ogni campione e falsava i rapporti d'ampiezza tra frequenze.
5. RSSI grezzo normalizzato a una scala grafica fissa 0..127; RBW non dichiarata quando non è calibrata.
6. Selezione del percorso RF VHF/UHF via REG 0x33: GPIO4/VHF sotto 280 MHz e GPIO3/UHF da 280 MHz, preservando gli altri bit e bloccando la scrittura se REG 0x30 non è leggibile.
7. Aggiunto inventario di provenienza/licenze in `docs/THIRD_PARTY.md`.
8. Corretta la direzione GPIO della matrice keypad: colonne output, righe input pull-up; implementati debounce e mappa logica UV-K1.
9. Aggiunti controlli Spectrum: UP/DOWN centro a passi 25 kHz, MENU seleziona span 0,5/1/2/4 MHz, EXIT resetta, F commuta peak hold, STAR azzera i picchi, tasti laterali cambiano span.
10. Aggiunto tracker DSP: noise floor percentile 20%, conferma del picco su due sweep consecutivi e interpolazione parabolica sub-bin; visualizzazione frequenza picco e noise floor.

## Blocchi ancora aperti prima della prova radio

La build e i test host dimostrano la compilabilità e il comportamento dei moduli simulati, **non** il funzionamento sul dispositivo fisico.

- Clock e tempi SysTick: 8 MHz resta un'assunzione; verificare handoff del bootloader 7.03.01.
- Keypad: scansione e controlli software implementati; da verificare fisicamente su tastiera e revisione PCB reale.
- AFC: misura/offset non validati; AFC-based spur rejection resta disabilitata. Il noise floor percentile e la conferma dei picchi riducono falsi positivi, ma non sostituiscono una misura AFC validata.
- ST7565: init e rendering compilano; manca la verifica elettrica/visiva sul pannello.
- RF: la commutazione LNA segue il driver UV-K1 di riferimento, ma registri, tempi e sensibilità vanno verificati con radio e strumentazione.
- Calibrazione: nessuna area flash viene modificata; backup e layout devono essere verificati prima del flash.
- Packaging: raw BIN non equivale a DFU; occorre validare il flusso di flashing e il recovery per il target effettivo.

**Decisione:** build e test automatici superati sul commit indicato. Vettore iniziale, indirizzo Reset_Handler e assert RAM/stack sono stati controllati staticamente nell'artefatto. Nessuna prova radio o flash è stata eseguita; i blocchi hardware/bootloader sopra elencati restano aperti.
