# Verifica formato BIN / DFU — UV-K1 7.03.01

Data: 2026-11-17  
Target dichiarato: UV-K1 standard, PY32F071, bootloader 7.03.01  
Immagine analizzata: `spectrum_k1_v3.bin`, 8.820 byte

## Distinzione dei formati

- **Raw application BIN:** byte dell'immagine MCU, con vettori all'origine applicativa. È il formato prodotto da `arm-none-eabi-objcopy -O binary`.
- **Pacchetto vendor:** può includere campi versione, trasformazioni/obfuscation e checksum; non è sinonimo di raw BIN.
- **Trasporto DFU:** protocollo seriale del bootloader che invia l'immagine in messaggi/pagine. I pacchetti di trasporto vengono costruiti dal flasher durante la scrittura.

Un file raw BIN non deve essere rinominato `.dfu` né presentato come contenitore vendor.

## Evidenza dal progetto Quansheng Toolkit

Repository pubblico: https://github.com/knoinok/quansheng_toolkit  
File protocollo: https://github.com/knoinok/quansheng_toolkit/blob/main/kradio/dfu.py  
README: https://github.com/knoinok/quansheng_toolkit

La documentazione del toolkit:
- include UV-K1 standard con bootloader 7.03.01 tra le combinazioni supportate;
- descrive una voce di caricamento **Custom .bin**;
- avverte che i BIN personalizzati non beneficiano della stessa allowlist/garanzia anti-brick dei firmware inclusi.

Il codice DFU descrive:
- seriale 38400 baud;
- pagine firmware da 256 byte;
- messaggio `PROG_FW` con indice pagina e numero totale;
- padding dell'ultima pagina;
- ACK per pagina e retry;
- allowlist del target `k1` che include bootloader 7.03.01.

La fonte toolkit è distribuita con licenza Apache-2.0. In questo progetto è stata consultata come documentazione tecnica; nessun codice del toolkit è stato importato.

## Verifiche sull'immagine locale

- ELF ARM 32-bit EABI5, Cortex-M0+, soft-float.
- Vettori all'indirizzo `0x08002800`.
- Stack iniziale `0x20004000`.
- Reset vector `0x0800483D`, handler in area immagine.
- Immagine BIN: 8.632 byte; 35 pagine di trasporto (34 complete + 116 byte nell'ultima prima del padding).
- Fine immagine: `0x08004A74`, entro la regione applicativa dichiarata dallo script linker locale.

## Limiti della conclusione

Questa è una verifica statica del formato e una verifica documentale del flusso DFU. Non sono stati eseguiti:
- handshake con un bootloader UV-K1 reale;
- invio delle 34 pagine;
- verifica ACK/CRC sul dispositivo;
- riavvio e controllo dell'esecuzione;
- prova RF, display o tastiera.

Il toolkit stesso avverte che il flashing può danneggiare la radio e richiede backup EEPROM/calibrazione. L'allowlist del bootloader identifica la famiglia, ma non certifica un'immagine custom arbitraria.

**Conclusione: il file è un raw application BIN destinato a essere passato a un flasher DFU che supporti Custom BIN; non è un pacchetto DFU autonomo. Non è certificato per il flash né per il funzionamento finché non viene provato su hardware con recovery disponibile.**


## Aggiornamento artefatto 2026-11-17

Build CI run: https://github.com/dob963/D10S-_2-FW_K1/actions/runs/44056550974  
Artifact ID: `14316181679`  
BIN size: `8.820` byte  
BIN SHA-256: `58097bc4554c5177c8552ed3750e4ce8836f5d6203e35d948bc78b4e84b8698a`  
ZIP SHA-256: `163357cbc2cd718485659a370801cd60ddaf541b640ce57d89cb910d7b15ab5f`

La verifica statica dell'ELF conferma `.isr_vector` a `0x08002800`, stack `0x20004000`, reset handler `0x0800483C` (vettore `0x0800483D`) e fine BIN a `0x08004A74`. Nessun test su radio fisica è stato eseguito.
