# Verifica hardware da sorgenti pubblici UV-K1 V3

Data: 2026-11-16

Ambito: UV-K1 standard con MCU PY32F071 e bootloader 7.03.01. Questa verifica usa sorgenti pubblici GitHub; non implica ispezione fisica della radio dell'utente.

## Fonti consultate

1. Firmware upstream Armel/F4HWN:
   - Repository: https://github.com/armel/uv-k1-k5v3-firmware-custom
   - File: `App/board.c`, `App/driver/bk4829.c`, `App/driver/st7565.c`, `Core/py32f071xb.ld`
   - Licenza repository: Apache-2.0. I file sorgente riportano copyright Dual Tachyon e muzkr; il linker Puya riporta condizioni BSD-style proprie.
2. Toolkit Quansheng:
   - https://github.com/knoinok/quansheng_toolkit
   - README e `firmwares/README.md`: UV-K1 standard, MCU PY32F071; combinazione validata UV-K1 + bootloader 7.03.01 + F4HWN Fusion 5.4.
3. Atlante hardware comunitario:
   - https://github.com/Brico-Hams/Radio-Web-install/blob/main/docs/ATLAS-UV-K5-V3.md
   - Mappa pin derivata dal firmware V3/K1; utile come corroborazione, non sostituisce schema elettrico della specifica revisione PCB.

## Risultati con evidenza sorgente

### Display

Il driver upstream è esplicitamente `st7565.c`: il controller da usare come base software è **ST7565** (LCD monocromatico 128x64), non ST7735/ST7789/ILI9341.

Mappatura dal driver:
- PA5: SPI1 SCK
- PA7: SPI1 SDA/MOSI
- PA6: LCD A0 (dato/comando)
- PB2: LCD CS

Il driver configura SPI1 hardware, CPOL HIGH, CPHA 2EDGE, MSB first, prescaler /64. La sequenza ST7565 upstream usa bias 1/9, COM normale, SEG invertito, EV 31, resistor ratio 5.0, accensione a stadi del power circuit e reset software E2. La funzione di reset hardware è un no-op: il commento upstream dichiara che sul K1 non è disponibile un reset hardware utilizzabile. La scrittura delle pagine usa offset colonna +4. La nuova implementazione bit-bang replica questi parametri senza copiare il driver; il timing effettivo resta da misurare sulla revisione PCB reale.

### Tastiera e GPIO

Da `App/board.c` e `App/driver/keyboard.c`:
- Matrice keypad: colonne PB3..PB6, righe PB12..PB15.
- PTT: PB10.
- Backlight: PF8.
- GPIOA, GPIOB, GPIOC e GPIOF vengono abilitati nella routine GPIO upstream.

La mappa logica tasti è definita in `keyboard.c`; non esiste evidenza per tre pin separati denominati fn1, fn2 e F-lock. I tasti laterali e il tasto F fanno parte della matrice. La scansione locale ora rispetta la polarità attiva-bassa: colonne PB6..PB3 pilotate una alla volta, righe PB15..PB12 in ingresso con pull-up; i tasti laterali vengono letti con tutte le colonne alte. Il debounce applicativo richiede tre letture stabili separate dai cicli sweep.

### Interfaccia RF

Il codice upstream `App/driver/bk4829.c` usa:
- CSN: PF9
- SCL: PB8
- SDA bidirezionale: PB9

Attenzione: `App/board.c` commenta questi segnali come “BK4819”, mentre il driver si chiama `bk4829.c` ma include `driver/bk4819.h` e conserva simboli `BK4819_*`. Il software upstream dimostra la topologia del bus e i registri/operazioni che il firmware utilizza, ma questa nomenclatura mista **non basta da sola a provare il marking del chip montato né la piena correttezza della register map**. Non convertire automaticamente il driver in un driver BK4819 generico o BK4829 indipendente senza verificare il componente e la documentazione specifica.

Conseguenza: la proposta precedente PA4=CS, PA5=SCK, PA6=MOSI, PA7=MISO è **errata per la mappa V3/K1 upstream**: PA5/PA7 sono usati dal display e il bus RF è PB8/PB9/PF9. Non implementare quel pinout.

## Linker e bootloader

Il linker upstream `Core/py32f071xb.ld` dichiara:
- RAM: `0x20000000`, 16 KiB
- FLASH applicazione: `0x08002800`, 118 KiB

Questo è un riferimento concreto e molto più specifico delle ipotesi `0x08000000` e `0x08004000`. Il toolkit comunitario elenca UV-K1 standard + bootloader 7.03.01 + Fusion 5.4 come combinazione validata. Pertanto `0x08002800` è il layout di partenza corretto per una build che segue la stessa architettura upstream K1.

Non confondere questa evidenza con la prova che un'immagine raw arbitraria sia flashabile: devono ancora corrispondere vettori, formato immagine, protocollo DFU e aree dati/calibrazione. Non scrivere in flash aree di calibrazione sulla base del solo linker.

## Decisioni d'integrazione

1. Display: adottare ST7565 e la mappa PA5/PA6/PA7/PB2.
2. Keypad: adottare la matrice PB3..PB6 / PB12..PB15 e PTT PB10.
3. RF bus: adottare PB8/PB9/PF9 come interfaccia fisica di riferimento.
4. Non usare i pin PA4..PA7 proposti per il bus RF.
5. Linker: usare FLASH origin `0x08002800`, RAM `0x20000000` 16K, con assert di overflow e riserva stack.
6. Chip RF: conservare la denominazione BK4829 del target V3 solo dopo conferma tramite BOM/marking affidabile; la nomenclatura legacy interna upstream è ambigua. Non inventare registri, PLL o formula di tuning.
7. Tutto il codice di terzi resta soggetto alle rispettive licenze e avvisi. Questo documento riporta risultati e riferimenti; non copia sorgenti upstream.
