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
- Sequenza comandi ST7565: display off/on, bias 1/9, COM normale, SEG invertito, start line, resistor ratio 5.0, contrasto iniziale 31/63, power control con stadi e tempi di attesa. Parametri ricavati come riferimento dal driver upstream `App/driver/st7565.c` (Apache-2.0), reimplementati autonomamente. Il comando software E2 e' stato rimosso: il datasheet richiede reset tramite pin RES.
- hw_init configura GPIO e chiama st7565_init; il main esegue il blit dopo aver composto waterfall e traccia.

## Pin e bus

Il sorgente hardware configura il bus RF su PB8 (clock), PB9 (SDIO bidirezionale), PF9 (SCN/CSN). Il display usa PA5/PA6/PA7 e PB2: non c'è sovrapposizione fra i due gruppi nel pinout attualmente adottato dal progetto. Questa mappatura è distinta dall'ipotesi PA4..PA7 per il bus RF, che non va utilizzata.

## Limiti hardware ancora aperti

1. **TODO bloccante per init completa:** il pin hardware RES non è identificato nelle fonti disponibili. Il datasheet richiede reset tramite RES; il comando software E2 non viene inviato. La routine attende 120 ms e prosegue con i comandi standard, ma senza impulso RES l'avvio del controller non è garantito. Verificare schema e continuità sulla revisione PCB prima di dichiarare completo il driver.
2. Bias 1/9, COM normale, SEG invertito, resistor ratio 5.0 e contrasto 31/63 seguono la configurazione software upstream K1; contrasto e orientamento vanno comunque verificati sul pannello della specifica revisione PCB.
3. SPI mode 3 e offset colonna +4 sono riferimenti del driver upstream K1; non è stata eseguita una cattura logica sul dispositivo specifico, quindi timing e risposta del pannello devono essere misurati.
4. Non è stata eseguita prova fisica sul dispositivo: il driver è compilato ma non ancora dichiarato validato elettricamente.

## Memoria

Il solo framebuffer occupa 1.024 byte. Il buffer waterfall occupa altri 2.048 byte statici. La misura aggiornata di .data/.bss va letta dall'ultima build ARM riuscita; il linker riserva 1.536 byte di stack e verifica che .data/.bss non invadano tale riserva. Il linker riserva 1.536 byte di stack e verifica che .data/.bss non invadano tale riserva.

## Frequenza aggiornamento

Il display riceve un frame completo di 1.024 byte per iterazione. Il main inserisce una pausa minima di 67 ms dopo ogni blit, per limitare il refresh a circa 10-15 fps assumendo il clock core di riferimento a 8 MHz. Il clock effettivo non è ancora misurato sul dispositivo: il limite temporale va verificato con oscilloscopio/logic analyzer sulla radio reale.

## Scala e fedeltà live

L'adattatore DSP esistente consegna alla UI campioni 0..127 per rispettare l'API int8_t di reiezione spurie. La UI scala 0..127 sull'intera altezza grafica e sul Waterfall a 16 livelli. Il disegno non miscela sweep successivi e Peak Hold resta separato dalla traccia live. Non sono stati modificati i moduli DSP forniti.
