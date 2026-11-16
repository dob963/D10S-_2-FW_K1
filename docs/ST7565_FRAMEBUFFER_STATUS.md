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
- Il main richiama st7565_blit_full() dopo aver composto entrambe le viste.

## Limiti hardware non risolti

st7565_blit_full() è intenzionalmente un placeholder: il framebuffer viene composto, ma i byte non sono ancora trasmessi al pannello. Prima del driver fisico servono:
1. conferma del pin RESET del pannello e della sua polarità;
2. verifica elettrica della condivisione PA5/PA6/PA7 tra LCD e BK4829;
3. verifica dell'orientamento, offset colonne/pagine e polarità dei pixel;
4. cattura/riscontro dei tempi SPI e della sequenza init sul display reale.

Non inviare comandi ST7565 usando i pin RF finché il conflitto di bus non è risolto. Nessuna sequenza di reset o contrasto è stata dichiarata verificata.

## Memoria

Il solo framebuffer occupa 1.024 byte. Il buffer waterfall occupa altri 2.048 byte statici. La dimensione finale di .bss/.data e il margine stack devono essere letti dal file MAP e dall'output arm-none-eabi-size della build aggiornata; non dedurre il margine stack dal solo limite RAM nominale.

## Frequenza aggiornamento

Non è stato aggiunto un timer artificiale per il refresh. La frequenza reale dipenderà dal tempo sweep, dal tempo di assestamento RF e dal trasferimento SPI, quest'ultimo ancora da implementare e misurare. Il limite 10-15 fps resta un criterio da verificare su hardware.

## Scala numerica

L'adattatore DSP esistente consegna alla UI campioni 0..127 per rispettare l'API int8_t di reiezione spurie. La UI scala 0..127 sull'intera altezza grafica e sul Waterfall a 16 livelli. Non modifica i moduli DSP forniti.
