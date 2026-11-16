# Stato integrazione ST7565 e rendering

## Implementato nel ramo main

- Framebuffer monocromatico ST7565: 128 x 64 pixel, 8 pagine, 1.024 byte.
- API framebuffer: clear, pixel, linee orizzontali/verticali, testo.
- Font 5x7 minimale per cifre, lettere maiuscole e punteggiatura usata dalla UI.
- Spettro live disegnato dalle misure stabilizzate, senza cross-fade tra sweep.
- Peak Hold memorizzato separatamente e rappresentato come marker; non modifica il vettore live.
- Waterfall separato: 16 righe, 128 colonne, 16 livelli da campioni DSP 0..127, matrice Bayer 4x4.
- La schermata usa 16 righe intestazione, 16 righe waterfall e 32 righe traccia live.
- Frequenza centrale e span sono visualizzati in MHz; RBW è visualizzata in kHz. RBW zero è indicata come sconosciuta (---).
- Trasporto display bit-bang SPI mode 3 (CPOL=HIGH, CPHA=2EDGE) su PA5 (SCK), PA7 (MOSI), PA6 (A0), PB2 (CS), 8 pagine x 128 byte. Ogni pagina usa offset colonna +4.
- Sequenza di init ST7565: reset software E2, display off/on, bias 1/9, COM normale, SEG invertito, start line, resistor ratio 5.0, contrasto iniziale 31/63, power control con stadi e tempi di attesa. Parametri ricavati come riferimento dal driver upstream `App/driver/st7565.c` (Apache-2.0), reimplementati autonomamente.
- hw_init configura GPIO e chiama st7565_init; il main esegue il blit dopo aver composto waterfall e traccia.

## Pin e bus

Il sorgente hardware configura il bus RF su PB8 (clock), PB9 (SDIO bidirezionale), PF9 (SCN/CSN). Il display usa PA5/PA6/PA7 e PB2: non c'è sovrapposizione fra i due gruppi nel pinout attualmente adottato dal progetto. Questa mappatura è distinta dall'ipotesi PA4..PA7 per il bus RF, che non va utilizzata.

## Limiti hardware ancora aperti

1. Il reset implementato è software ST7565 (comando E2). Il pin RESET fisico del modulo non è stato identificato e non viene pilotato.
2. Bias 1/9, COM normale, SEG invertito, resistor ratio 5.0 e contrasto 31/63 seguono la configurazione software upstream K1; contrasto e orientamento vanno comunque verificati sul pannello della specifica revisione PCB.
3. SPI mode 3 e offset colonna +4 sono riferimenti del driver upstream K1; non è stata eseguita una cattura logica sul dispositivo specifico, quindi timing e risposta del pannello devono essere misurati.
4. Non è stata eseguita prova fisica sul dispositivo: il driver è compilato ma non ancora dichiarato validato elettricamente.

## Memoria

Il solo framebuffer occupa 1.024 byte. Il buffer waterfall occupa altri 2.048 byte statici. La build ARM precedente a questa implementazione display fisica occupava 5.800 byte di BSS; la nuova misura BSS e il margine stack vanno presi dalla build corrente. Il linker riserva 1.536 byte di stack e verifica che .data/.bss non invadano tale riserva.

## Frequenza aggiornamento

Il display riceve un frame completo di 1.024 byte per iterazione. Non è stato aggiunto un limitatore software a 10-15 fps; il rate dipende dallo sweep e dal tempo reale di trasferimento. Va misurato sul dispositivo, evitando di dichiarare 10-15 fps senza prova.

## Scala e fedeltà live

L'adattatore DSP esistente consegna alla UI campioni 0..127 per rispettare l'API int8_t di reiezione spurie. La UI scala 0..127 sull'intera altezza grafica e sul Waterfall a 16 livelli. Il disegno non miscela sweep successivi e Peak Hold resta separato dalla traccia live. Non sono stati modificati i moduli DSP forniti.
