# Integrazione dei moduli DSP forniti

Data: 2026-11-16

## Importazione

I seguenti file sono stati aggiunti alla repository `dob963/D10S-_2-FW_K1` copiando il testo fornito dall'utente senza modificarne la logica:

- `inc/agc_sw.h`, `src/agc_sw.c`
- `inc/median3.h`, `src/median3.c`
- `inc/spur_reject.h`, `src/spur_reject.c`
- `inc/sweep_engine.h`, `src/sweep_engine.c`
- `inc/ui_spectrum.h`, `src/ui_spectrum.c`
- `inc/waterfall.h`, `src/waterfall.c`
- `inc/calib.h`, `src/calib.c`

I moduli sono codice fornito dall'utente; non sono codice prelevato da terzi. Non è stata eseguita una compilazione ARM.

## Audit d'integrazione: problemi da non nascondere

Queste osservazioni non modificano i moduli. Sono blocchi da risolvere o accettare esplicitamente durante l'integrazione.

1. **Dipendenza RF:** `src/sweep_engine.c` include `bk4819_drv.h`. Il target V3 di riferimento usa il bus RF della board UV-K1 e il driver upstream denominato `bk4829.c`, con API legacy `BK4819_*`. Serve un adapter compatibile e verificato; non si può collegare un driver generico solo per far compilare.
2. **Sweep adattivo:** `step_coarse = span / (SPEC_BINS/4)` e `step_fine = span / SPEC_BINS` possono diventare zero per span piccoli. Inoltre lo step cambia in funzione dell'RSSI: i bin non rappresentano più intervalli di frequenza uniformi. Il rendering deve quindi conoscere la frequenza reale associata a ogni bin, oppure lo sweep va corretto con una strategia esplicita.
3. **Filtro mediano in-place:** il ciclo di `main.c` fornito aggiorna `spec_avg[i]` mentre legge i vicini. Dal secondo bin, un vicino può essere già stato filtrato. Per un median 3-tap corretto serve un buffer sorgente separato; questa modifica non è stata applicata per rispettare il vincolo di non alterare la logica DSP senza revisione.
4. **Traccia senza cascata:** `ui_spectrum_draw()` applica cross-fade temporale e clamp del gradiente alla stessa traccia visualizzata. Queste operazioni introducono dipendenza dai frame precedenti e modificano i picchi stretti. Sono in tensione con il requisito rigoroso “nessuna scia/cascata”. Peak Hold deve restare un livello separato e non retroagire sulla traccia live.
5. **Conversione a int8:** `spur_update()` converte `clean` in `int8_t`; i valori oltre 127 non sono rappresentabili in modo portabile. L'AGC produce un valore massimo 255. Occorre concordare la scala/saturazione del buffer di uscita prima di collegare il renderer.
6. **Calibrazione:** il `calib_load()` fornito azzera gli offset in RAM; non legge flash. Il salvataggio flash non è implementato. Non è stato scelto alcun settore perché le regioni dati/calibrazione del firmware UV-K1 devono essere preservate.
7. **Waterfall:** il buffer occupa 16 x 128 byte e `waterfall_push()` fa scorrere 16 righe a ogni chiamata. Non esiste ancora un interruttore di abilitazione o un renderer; la separazione dal ciclo di sweep va implementata senza cambiare la misura live.

## Regola di integrazione

I sorgenti sono stati conservati. Le correzioni sopra elencate devono essere isolate in patch piccole, accompagnate da test host, e non confuse con una semplice integrazione. Nessun driver RF, accesso flash, startup completo, Makefile o BIN è dichiarato pronto.
