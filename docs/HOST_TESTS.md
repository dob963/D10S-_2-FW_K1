# Test automatici host

I test host sono eseguiti dal workflow `.github/workflows/arm-build.yml` prima della compilazione ARM. Compilano con GCC host, C11, `-Wall -Wextra -Werror` e nessuna dipendenza esterna.

## BK4829 adapter (`tests/test_bk4829_driver.c`)

- Conversione Hz -> unità da 10 Hz e split registri 0x38/0x39.
- Sequenza REG 0x30 reset/restore e mascheramento AF DAC bit 9.
- RSSI grezzo a 9 bit da REG 0x67.
- Rifiuto della lettura bus `0xFFFF` per evitare falsi picchi.
- Attesa: 1 ms, polling REG 0x63 con soglia 200 e massimo 50 letture, scarto del primo RSSI.
- Blocco delle scritture di tuning se REG 0x30 non è leggibile.
- AFC resta placeholder: il test verifica che `bk4829_afc_is_valid()` resti falso e che il firmware non applichi una falsa terza coerenza.

## DSP e Waterfall (`tests/test_dsp_no_cascade.c`)

- Median 3-tap su casi ordinati e misti.
- Calibrazione DC: default zero, clamp negativo a zero e controllo dei limiti bin.
- Un picco isolato nello stabilizzatore non si propaga ai bin adiacenti.
- La fase hold/release non crea valori nei bin vicini.
- Waterfall disabilitato: zero chiamate di disegno pixel.
- Waterfall abilitato: esattamente 16 x 128 pixel elaborati.

## Limite dei test

I test verificano il comportamento software deterministico, non il bus elettrico, il display, la ricezione RF o il bootloader reali. Per il rilascio restano obbligatori dump stock, verifica pin/clock, prove su radio e validazione DFU.
