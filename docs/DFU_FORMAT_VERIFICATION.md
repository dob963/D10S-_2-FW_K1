# Verifica formato BIN / DFU — UV-K1 7.03.01

Data: 2026-11-17  
Target dichiarato: UV-K1 standard, PY32F071, bootloader 7.03.01  
Immagine analizzata: `spectrum_k1_v3.bin`, 8.632 byte

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
- Reset vector `0x08004775`, handler in area immagine.
- Immagine BIN: 8.632 byte; 34 pagine di trasporto (33 complete + 184 byte nell'ultima prima del padding).
- Fine immagine: `0x080049B8`, entro la regione applicativa dichiarata dallo script linker locale.

## Limiti della conclusione

Questa è una verifica statica del formato e una verifica documentale del flusso DFU. Non sono stati eseguiti:
- handshake con un bootloader UV-K1 reale;
- invio delle 34 pagine;
- verifica ACK/CRC sul dispositivo;
- riavvio e controllo dell'esecuzione;
- prova RF, display o tastiera.

Il toolkit stesso avverte che il flashing può danneggiare la radio e richiede backup EEPROM/calibrazione. L'allowlist del bootloader identifica la famiglia, ma non certifica un'immagine custom arbitraria.

**Conclusione: il file è un raw application BIN destinato a essere passato a un flasher DFU che supporti Custom BIN; non è un pacchetto DFU autonomo. Non è certificato per il flash né per il funzionamento finché non viene provato su hardware con recovery disponibile.**
