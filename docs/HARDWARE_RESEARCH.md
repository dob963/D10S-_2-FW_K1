# Hardware and implementation research

Date: 2026-11-16

Ricerca basata su repository pubblici. La mappa descrive la piattaforma di riferimento UV-K1 standard/V3; non sostituisce la verifica elettrica su una specifica revisione PCB.

## Fonti consultate

1. Armel/F4HWN UV-K1/K5 V3:
   https://github.com/armel/uv-k1-k5v3-firmware-custom
   File consultati: `App/board.c`, `App/driver/bk4829.c`, `App/driver/st7565.c`, `App/driver/keyboard.c`, `Core/py32f071xb.ld`.
   Licenza repository: Apache-2.0. I file upstream riportano avvisi e attribuzioni dei rispettivi autori.
2. Quansheng toolkit:
   https://github.com/knoinok/quansheng_toolkit
   README e manifest firmware: UV-K1 standard PY32F071; combinazione UV-K1 + bootloader 7.03.01 + F4HWN Fusion 5.4 indicata come validata.
3. Teardown UV-K1:
   https://github.com/armel/k1-teardown
   Include datasheet BK4829 e PY32F071.
4. Atlante V3/K1:
   https://github.com/Brico-Hams/Radio-Web-install/blob/main/docs/ATLAS-UV-K5-V3.md
   Corrobora la mappa di pin firmware e la famiglia di scheda.

## Mappa hardware di riferimento

### Display

Il driver upstream è `App/driver/st7565.c`: controller software ST7565, LCD monocromatico 128x64.
- PA5: SPI1 SCK
- PA7: SPI1 SDA/MOSI
- PA6: A0 dato/comando
- PB2: CS

Il driver usa SPI1 hardware, CPOL HIGH, CPHA 2EDGE, MSB first, prescaler /64. Non configurare PA5–PA7 come bus radio.

### RF

Il target V3/K1 viene identificato dalle fonti di teardown e dal driver upstream come piattaforma BK4829. Il file `App/driver/bk4829.c` implementa però un'interfaccia legacy con simboli `BK4819_*`, e `board.c` conserva commenti “BK4819”. Questi nomi legacy non devono essere interpretati come prova che il componente fisico sia BK4819: il teardown pubblica il datasheet BK4829 per il K1.

Bus RF nel driver upstream:
- PF9: CSN
- PB8: SCL
- PB9: SDA bidirezionale

**Decisione:** il target RF di questo progetto resta BK4829; la denominazione legacy BK4819 va isolata dietro l'adapter. Non copiare una tabella registri BK4819 o inventare una sequenza init/tuning: prima di abilitare scritture RF va verificata la compatibilità dei registri usati con il datasheet BK4829 e con il percorso di inizializzazione V3. La formula di tuning deve essere derivata dalla specifica XTAL e dall'unità di frequenza attesa, non dedotta da un helper per toni.

### Tastiera e GPIO

Da `App/board.c` e `App/driver/keyboard.c`:
- Colonne matrice: PB3, PB4, PB5, PB6
- Righe matrice: PB12, PB13, PB14, PB15
- PTT: PB10
- Backlight: PF8
- Clock GPIO abilitati upstream: GPIOA, GPIOB, GPIOC, GPIOF

Fn1/Fn2/F-lock non risultano tre ingressi GPIO separati: i tasti sono codificati dalla matrice e dalla logica tastiera upstream.

## Flash e bootloader

Il linker upstream `Core/py32f071xb.ld` dichiara:
- FLASH applicazione: `0x08002800`, lunghezza 118 KiB
- RAM: `0x20000000`, lunghezza 16 KiB

Il toolkit elenca UV-K1 standard con bootloader 7.03.01 e F4HWN Fusion 5.4 come combinazione validata. Il progetto usa quindi `0x08002800` come layout di riferimento K1, documentato anche in `ld/py32f071_app.ld`.

L'origine linker non prova da sola che un'immagine raw arbitraria sia flashabile: occorre rispettare vettori, formato raw/DFU, handshake, dimensioni e regioni riservate. La calibrazione non deve essere salvata in un settore flash scelto arbitrariamente.

## Implicazioni per l'integrazione

1. Display e keypad: adottare la mappa V3/K1 qui riportata.
2. RF: usare il bus PB8/PB9/PF9 e mantenere target BK4829; isolare i nomi legacy BK4819.
3. Clock/delay: non assumere 8 MHz. Il firmware upstream K1 usa un handoff clock configurato dal bootloader; il valore e la sorgente devono essere coerenti con il target e verificati nel runtime.
4. Non implementare pin RF PA4–PA7: sono in conflitto con la mappa LCD.
5. Tenere live trace, Peak Hold e Waterfall indipendenti.
6. Registrare licenze, copyright, NOTICE, revisioni e modifiche prima di riutilizzare codice terzo.

## Stato attuale

- Creato linker script di riferimento in `ld/py32f071_app.ld`.
- Creato documento di verifica pinout in `docs/UPSTREAM_PINOUT_VERIFICATION.md`.
- I moduli DSP incollati nella conversazione non sono ancora file presenti nella repository.
- Non sono ancora presenti driver runtime, startup completo, build system o immagine BIN.
- Nessuna compilazione ARM e nessun test radio sono stati eseguiti.
- Driver RF e calibrazione flash restano bloccati finché non sono verificate le sequenze BK4829 e le aree dati riservate.
