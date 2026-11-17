# Rapporto verifica build — UV-K1 V3 Spectrum

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`  
Branch: `main`  
Commit compilato: `f5918780ecf8648025378c7ea8ffca8728ab2757`

## Esito CI

<badge>SUCCESS</badge> GitHub Actions run #141:
https://github.com/dob963/D10S-_2-FW_K1/actions/runs/44047292574

Tutti gli step sono terminati con esito SUCCESS:
- test host BK4829, inclusa selezione LNA VHF/UHF;
- test DSP no-cascade e isolamento Waterfall;
- test sweep uniforme, copertura completa e mapping frequenza;
- installazione toolchain ARM GCC;
- compilazione ARM e generazione raw BIN;
- pubblicazione artefatto.

## Artefatti

Artefatto GitHub Actions: `uv-k1-v3-spectrum-build`  
ID: `14312066975`  
Dimensione archivio ZIP: 22.851 byte  
Scadenza indicata da GitHub: 2027-02-15  
SHA-256 BIN: `bf4fcd2adb922b3d799c4899aac682c06ae4cc2221381b73eb12673203d1a58e`

Contenuti:
- `spectrum_k1_v3.bin`: 6.168 byte
- `spectrum_k1_v3.elf`: 85.020 byte
- `spectrum_k1_v3.map`: 39.245 byte

Il BIN è un'immagine ARM raw all'offset applicativo previsto dal linker. **Non è un contenitore DFU Quansheng** e non va rinominato come DFU.

## Memoria

Dalla mappa linker:
- FLASH vettori: 192 byte
- `.text`: 5.604 byte
- `.rodata`: 360 byte
- `.data`: 4 byte
- `.bss`: 4.880 byte
- fine `.bss`: `0x20001314`
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

## Blocchi ancora aperti prima della prova radio

La build e i test host dimostrano la compilabilità e il comportamento dei moduli simulati, **non** il funzionamento sul dispositivo fisico.

- Clock e tempi SysTick: 8 MHz resta un'assunzione; verificare handoff del bootloader 7.03.01.
- Keypad: la matrice è documentata, ma la scansione eventi Spectrum non è ancora implementata.
- AFC: misura/offset non validati; AFC-based spur rejection resta disabilitata.
- ST7565: init e rendering compilano; manca la verifica elettrica/visiva sul pannello.
- RF: la commutazione LNA segue il driver UV-K1 di riferimento, ma registri, tempi e sensibilità vanno verificati con radio e strumentazione.
- Calibrazione: nessuna area flash viene modificata; backup e layout devono essere verificati prima del flash.
- Packaging: raw BIN non equivale a DFU; occorre validare il flusso di flashing e il recovery per il target effettivo.

**Decisione:** build compilata e test automatici superati. Non dichiarare il firmware pronto per il flash finché i blocchi hardware/bootloader sopra elencati non sono chiusi.
