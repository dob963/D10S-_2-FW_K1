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
| 0x7E, 0x46, 0x4A, 0x07 | 0x303E, 0x600A, 0x5430, 0x61CE | righe 193-196 |
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

## Dump stock: verifica incrociata

File ufficiale incluso nell'archivio upstream:
`archive/quansheng.k1.stock.firmware.v7.03.01.bin`

- Repository: `armel/uv-k1-k5v3-firmware-custom`
- Commit: `3261ca7883d5b0b60c0905d8766612dee7b4191d`
- Dimensione: **71.268 byte**
- SHA-256 calcolato sul contenuto binario: **`55ec0daffc5668bdb41dcc118475d7e6e23ad953d70f57a9bca64a6202734ba0`**
- Esito: corrisponde esattamente all'hash indicato nel briefing.

È stata eseguita una ricerca byte-level dei valori little-endian presenti nella sequenza di init upstream. Il dump contiene un gruppo contiguo di costanti compatibili con diversi valori usati da `BK4819_Init()` in `App/driver/bk4829.c`, concentrato circa agli offset `0xB818–0xB890`. Tra i valori trovati nel gruppo: `0x9D1F`, `0x033A`, `0x2AB2`, `0x73DC`, `0xE920`, `0x33A8`, `0x3516`, diversi coefficienti DTMF, `0xE555`, `0x4C58`, `0xC65A`, `0x94C6`, `0x4691`, `0x88EF`, `0x1822`, `0x9890`, `0x5430` e `0x61CE`.

**Limite importante:** trovare una costante nel firmware compilato non prova da solo quale registro SPI la riceva. La ricerca binaria non è stata elevata a prova di sequenza di bus né è stata usata per inventare registri.

Il valore `0x1041` compare una sola volta come coppia di byte little-endian all'offset `0x537`, fuori dal gruppo contiguo di costanti di init. Non è quindi una conferma byte-level indipendente della scrittura REG 0x19. La scrittura REG 0x19 = `0x1041` resta supportata dal sorgente F4HWN del driver UV-K1, riga 186, ma non è stata confermata come costante della sequenza stock.

Nessun pattern AFC/AFCR è stato identificato in modo univoco nel dump. L'AFC enable/disable resta confermato dal sorgente F4HWN (REG 0x73 bit 4); la lettura offset AFC e l'eventuale scala AFCR 0-7 restano TODO.

## Pin RF

La mappa upstream in `App/driver/bk4829.c` assegna CSN=PF9, SCL=PB8, SDA=PB9. Il display ST7565 usa PA5/PA7 per SPI e PA6 per A0, PB2 per CS secondo il driver upstream. Il pinout RF PA4–PA7 proposto nel briefing non viene usato.

## Gate prima della prova radio

- Confermare il marking del chip RF sulla specifica scheda.
- Verificare elettricamente PF9/PB8/PB9 e le linee display sulla revisione PCB.
- Verificare il protocollo di lettura/scrittura e il clock del bus sul target.
- Salvare e verificare backup calibrazione/configurazione prima di qualsiasi flash.
- Non considerare il driver validato su hardware finché non sono state eseguite prove RF reali.


## Correzione debug 2026-11-17

Il confronto puntuale con `BK4819_PickRXFilterPathBasedOnFrequency()` nel driver UV-K1 upstream mostra che la soglia è **28.000.000 Hz**, non 280 MHz. La scelta GPIO è quindi:
- sotto 28 MHz: GPIO4 VHF;
- da 28 MHz in su: GPIO3 UHF.

Il precedente confronto a 280 MHz era un errore di fattore 10 ed è stato corretto in `src/bk4829_drv.c`; il test ora verifica 146 MHz (UHF) e 27 MHz (VHF).

Il confronto del bus seriale con `BK4819_ReadU16()`, `BK4819_WriteU8()`, `BK4819_WriteU16()`, `BK4819_ReadRegister()` e `BK4819_WriteRegister()` ha inoltre portato a correggere:
- campionamento SDA nella fase SCL bassa, prima del fronte di salita;
- temporizzazione `SHORT_DELAY()` di 35 NOP per ciascuna fase;
- sequenza CS/SCL di inizio e fine transazione e ripristino SDA/SCL.

Queste sono correzioni della temporizzazione e della selezione del ramo RF basate sul driver di riferimento; non certificano il marking del chip, la compatibilità elettrica o il funzionamento sul PCB specifico.


## Avvio ricevitore — correzione debug 2026-11-17

È stato individuato un problema funzionale più grave: il firmware inizializzava i registri base ma si limitava poi a leggere REG 0x30. Non poteva assumere che il bootloader avesse lasciato il ricetrasmettitore in ricezione.

È stata aggiunta la sequenza di accensione RX documentata nella funzione `BK4819_RX_TurnOn()` del driver UV-K1 upstream:
- REG 0x37 = `0x9F1F`
- REG 0x30 = `0x0000`
- REG 0x30 = `0xBFF1`

Per la scansione il bit AF DAC (bit 9) viene disabilitato conservando il resto del valore: REG 0x30 = `0xBDF1`. I test host verificano la sequenza e il valore ripristinato al retune.

La sequenza è una reimplementazione dei valori pubblicati nel driver UV-K1 (Apache-2.0, attribuzione sopra); non è una prova del ricevitore sul chip fisico. La larghezza filtro RX e l'intero percorso RF devono ancora essere verificati con il dump stock e strumentazione.


## Filtro di scansione

Il driver UV-K1 upstream documenta REG 0x43 per la larghezza del filtro RF. Per la modalità wide 25 kHz la configurazione è `0x3028` (RF passband 7 kHz secondo la descrizione dei bit e modo 25k/20k). Il firmware standalone scrive esplicitamente questo valore dopo l'accensione RX, evitando di dipendere dal filtro lasciato dal bootloader o da un precedente VFO. Il test host verifica la scrittura. La larghezza effettiva e la sensibilità vanno comunque misurate sul ricevitore reale.
