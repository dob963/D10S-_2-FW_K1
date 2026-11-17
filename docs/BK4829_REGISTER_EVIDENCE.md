# BK4829 UV-K1: registro RF e fonti verificate

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`, ramo `main`  
Target dichiarato: UV-K1 V3 / PY32F071 / bootloader 7.03.01.

## Fonte primaria software usata

- Repository: https://github.com/armel/uv-k1-k5v3-firmware-custom
- Commit fissato: `3261ca7883d5b0b60c0905d8766612dee7b4191d`
- File: [App/driver/bk4829.c](https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/3261ca7883d5b0b60c0905d8766612dee7b4191d/App/driver/bk4829.c)
- File: [App/driver/bk4819-regs.h](https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/3261ca7883d5b0b60c0905d8766612dee7b4191d/App/driver/bk4819-regs.h)
- File: [App/radio.c](https://github.com/armel/uv-k1-k5v3-firmware-custom/blob/3261ca7883d5b0b60c0905d8766612dee7b4191d/App/radio.c)
- Licenza upstream: Apache-2.0. I sorgenti contengono avvisi copyright Dual Tachyon e muzkr.

Attenzione alla nomenclatura: il file specifico del target è `bk4829.c`, ma le funzioni e l'header usano nomi legacy `BK4819_*`. I valori qui riportati provengono esclusivamente dal file del driver V3/K1 `bk4829.c`, non dal driver V1/V2 `bk4819.c`. Non è una validazione indipendente del silicio o della register map del produttore.

## Init RF implementata

In `src/bk4829_drv.c`, `bk4829_init()` applica la sequenza di inizializzazione pubblicata in `BK4819_Init()` nel file upstream `App/driver/bk4829.c`, righe 100-201.

Valori e riferimenti principali:

| Registro | Valore | Fonte upstream |
|---|---:|---|
| 0x00 | 0x8000, poi 0x0000 | `bk4829.c`, righe 106-107 |
| 0x37 | 0x9D1F | riga 109 |
| 0x36 | 0x0022 | riga 110 |
| 0x10–0x14 | 0x0318, 0x033A, 0x03DB, 0x03DF, 0x0210 | righe 114-118 |
| 0x49 | 0x2AB2 | riga 119 |
| 0x7B | 0x73DC | riga 120 |
| 0x7D | 0xE920 | riga 124 |
| 0x48 | 0x33A8 | riga 144 |
| 0x40 | 0x3516 | riga 151 |
| 0x09 | coefficienti DTMF 16 step | righe 154-156 |
| 0x1C–0x1E | 0x07C0, 0xE555, 0x4C58 | righe 177-179 |
| 0x1F | 0xC65A | riga 181 |
| 0x3E | 0x94C6 | riga 182 |
| 0x73 | 0x4691 | riga 184 |
| 0x77 | 0x88EF | riga 185 |
| **0x19** | **0x1041** | **riga 186** |
| 0x28–0x2A | 0x0B40, 0xAA00, 0x6600 | righe 187-189 |
| 0x2C, 0x2F, 0x53 | 0x1822, 0x9890, 0x2028 | righe 190-192 |
| 0x7E, 0x46, 0x4A, 0x07 | 0x303E, 0x600A, 0x5430, 0x61CE | righe 200-201 |
| 0x33, 0x3F | 0x9000, 0x0000 | righe 200-201 |

Il valore richiesto per REG 0x19 è quindi **0x1041**. È il valore scritto dal driver V3/K1 upstream; il commento nella stessa funzione collega il bit 15 al MIC AGC. Non è stato dedotto da un driver BK4819 separato.

La sequenza è stata reimplementata come tabella di valori in questo progetto; non è stato copiato il corpo della funzione upstream. L'attribuzione e la licenza della fonte sono conservate in questo documento.

## AFC implementato

Il file upstream `App/driver/bk4819-regs.h`, riga 31, dichiara:

`afcDisableRegSpec = {"AFC Disable", 0x73, 4, 1, 1}`

Quindi:
- Registro: `0x73`
- Bit: `4`
- Maschera: `0x0010`
- Bit 4 = 1: AFC disabilitato
- Bit 4 = 0: AFC abilitato

L'uso effettivo è visibile in:
- `App/radio.c`, chiamata `BK4819_SetRegValue(afcDisableRegSpec, modulation != MODULATION_FM)`
- `App/driver/bk4829.c`, chiamate che impostano AFC attivo/disattivo nei profili RAW/bypass.

È stata aggiunta `bk4829_set_afc_enabled()`, che esegue read-modify-write su REG 0x73 e preserva gli altri bit. L'inizializzazione upstream scrive REG 0x73 = `0x4691`, il cui bit 4 è 1: AFC parte disabilitato.

## AFCR 0-7 e misura AFC: TODO

Nel codice F4HWN consultato non è stata trovata una lettura documentata del valore/offset AFC con registro, segno e scala verificati. La ricerca di `AFCR` nel repository restituisce anche simboli del chip FM BK1080: non sono applicabili al BK4829.

L'indizio 4PDA “AFCR 0-7” resta non confermato. Non viene assegnato a un registro. `bk4829_read_afc()` rimane TODO e restituisce 0; non usare il candidato REG 0x6D senza una fonte BK4829 primaria e confronto indipendente.

Per completare il dato cercare:
1. una revisione ufficiale del datasheet BK4829 con descrizione AFC/AFCR;
2. il dump stock UV-K1 7.03.01 verificato con SHA-256 `55ec0daffc5668bdb41dcc118475d7e6e23ad953d70f57a9bca64a6202734ba0`;
3. confronto delle letture prima/dopo variazione controllata di frequenza e modalità, su hardware con backup e strumentazione.

## Dump stock: stato verifica

L'URL fornito `http://armel.github.io/uvstudio/assets/factory/quansheng.k1.stock.firmware.v7.03.01.bin` è stato aperto, ma il recupero binario non è riuscito in questo ambiente; il servizio web ha restituito URL non accessibile. Di conseguenza:
- lo SHA-256 indicato dall'utente non è stato verificato;
- non è stata eseguita analisi byte-level delle sequenze SPI;
- non si afferma alcuna conferma AFC dal dump.

I valori di init e AFC sopra sono supportati dal sorgente V3/K1 fissato al commit indicato, non dal dump stock.

## Pin RF

La mappa upstream in `App/driver/bk4829.c` assegna CSN=PF9, SCL=PB8, SDA=PB9. Il display ST7565 usa PA5/PA7 per SPI e PA6 per A0, PB2 per CS secondo il driver upstream. Il pinout RF PA4–PA7 proposto nel briefing non viene usato.

## Gate prima della prova radio

- Confermare il marking del chip RF sulla specifica scheda.
- Verificare elettricamente PF9/PB8/PB9 e le linee display sulla revisione PCB.
- Verificare il protocollo di lettura/scrittura e il clock del bus sul target.
- Salvare e verificare backup calibrazione/configurazione prima di qualsiasi flash.
- Non considerare il driver validato su hardware finché non sono state eseguite prove RF reali.
