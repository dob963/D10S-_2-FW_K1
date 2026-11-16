# Identificazione RF e gate del driver

Data: 2026-11-16

## Identificazione del componente

La documentazione di teardown pubblicata nel repository `armel/k1-teardown` include il datasheet Beken BK4829 (DS-BK4829-E01 V1.0) tra i documenti del UV-K1:
- Repository: https://github.com/armel/k1-teardown
- Datasheet mirror: https://manuals.plus/m/b03fb6dba016158ab9f4b5b3d1aeb1d9ea178efc44cc11c2c08398621553cdc9.pdf
- Secondo riferimento datasheet: https://www.alldatasheet.com/datasheet-pdf/pdf/2171272/BEKEN/BK4829.html

L'atlante hardware V3/K1 conferma esplicitamente il bus:
- PB8 = BK4829 SCK
- PB9 = BK4829 SDIO bidirezionale
- PF9 = chip select

Riferimento: https://github.com/Brico-Hams/Radio-Web-install/blob/c8466471014bddc94fc08673894c94d77c0f68ba/docs/i18n/en/ATLAS-UV-K5-V3.md

Per il target standard UV-K1 V3, la scelta progettuale è quindi **BK4829**, non un BK4819 generico. Resta possibile una variazione tra revisioni PCB: prima della release fisica va controllato il marking o il BOM della revisione esatta.

## Perché il driver non viene ancora implementato

Nel sorgente upstream `App/driver/bk4829.c` il bus è coerente con il BK4829 e il commento AGC dichiara di mantenere il profilo stock BK4829, ma le API e le costanti continuano a chiamarsi `BK4819_*` e includono `bk4819.h`. Questa nomenclatura storica è un adapter software, non una prova che ogni registro BK4819 sia intercambiabile.

Il sorgente upstream esegue una sequenza di reset e molte scritture di registri; copiarla in un driver minimale “Spectrum only” senza portare le dipendenze, verificare le definizioni del datasheet e capire quali blocchi sono indispensabili potrebbe compromettere la ricezione. Per questo non vengono inventati:
- formula PLL/frequency word;
- valori LNA/IF e filtro canale;
- sequenza di accensione RX;
- codifica AFC;
- timing RSSI e glitch settling.

Il codice Spectrum3D upstream descrive un fast-scan con attesa del glitch indicator, scarto della prima lettura RSSI potenzialmente stale e conferma del candidato. È un comportamento utile da riprodurre in modo indipendente, ma non sostituisce la verifica delle unità e dei registri sul datasheet BK4829.

## Gate di accettazione del driver

Prima di abilitare la scrittura RF:
1. Mappare ogni registro usato a una sezione/pagina del datasheet BK4829.
2. Confermare il riferimento di clock del chip e l'unità della frequenza richiesta dal firmware.
3. Verificare la formula di conversione su più frequenze note e sui limiti RF supportati.
4. Confrontare la sequenza RX con il percorso upstream K1 e isolare solo le scritture necessarie.
5. Validare su hardware RSSI, AFC, glitch flag e tempo di settling.
6. Mantenere il ricevitore in RX sicuro e non abilitare TX.
7. Non dichiarare “massima sensibilità” senza misure RF ripetibili con generatore calibrato e livelli noti.

## Stato

Il target BK4829 e il pinout sono documentati da fonti specifiche. Il driver runtime resta non implementato e non verificato; questo è intenzionale per evitare scritture arbitrarie su registri RF.
