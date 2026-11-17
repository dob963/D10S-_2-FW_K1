# Inventario componenti e riferimenti di terze parti

Data: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`

Questo inventario distingue codice effettivamente incluso da documentazione/comportamenti consultati. Un riferimento tecnico non autorizza automaticamente la copia del relativo sorgente.

## Riferimenti consultati

| Progetto / fonte | Licenza dichiarata | Uso in questo repository |
|---|---|---|
| Armel/F4HWN UV-K1/K5 V3, commit `3261ca7883d5b0b60c0905d8766612dee7b4191d` | Apache-2.0 | Consultazione del comportamento del driver UV-K1 BK4829, sequenza init, lettura RSSI, selezione LNA VHF/UHF, ST7565 e spectrum. Implementazioni locali riscritte; nessun file sorgente upstream importato integralmente. |
| Puya `OpenPuya/PY32F071_Firmware`, commit `0ed2f4b4d3391eccfd4491006a30295fd78e32c2` | BSD-3-Clause (con avvisi Puya/ST nei file di riferimento) | Consultazione della tabella vettori, clock e inizializzazione PY32F071. Startup locale riscritto indipendentemente; nessun file assembly upstream copiato. |
| Quansheng Toolkit, commit `ea13a5850d8f22372c7ad205db13610c083c4ee8` | Apache-2.0 | Consultazione di profili bootloader, formati firmware e procedure di backup/DFU. Nessun codice del toolkit incluso. |
| Armel/F4HWN `App/driver/keyboard.c`, commit/branch `main` consultato il 2026-11-17 | Apache-2.0 | Consultazione della matrice keypad e mappa logica tasti. Scansione e debounce locali riscritti; nessun sorgente importato. |
| BK4829 datasheet, revisione citata nei documenti RF | Documento del produttore | Riferimento tecnico per limiti di frequenza e timing; non è codice software. |


## Tracciabilità delle implementazioni locali

- `src/bk4829_drv.c`: driver locale; la tabella di inizializzazione e le operazioni RF sono state ricostruite/riscritte usando le fonti target UV-K1 citate in `docs/BK4829_REGISTER_EVIDENCE.md`. La selezione del percorso RF usa i segnali GPIO3/UHF-LNA e GPIO4/VHF-LNA documentati dal driver target.
- `src/peak_tracker.c`: modulo originale locale per percentile 20% del rumore di fondo, conferma del picco su due sweep e interpolazione parabolica sub-bin; test host dedicati.
- `src/ui_spectrum.c`: scansione/debounce matrice keypad implementata indipendentemente sulla mappa pin pubblicata; controlli center/span, peak hold e reset picco.
- `src/sweep_engine.c`: implementazione locale di sweep uniforme completo; non deriva da una copia del codice Spectrum upstream.
- `src/st7565.c`: driver framebuffer/trasporto locale; parametri di inizializzazione confrontati con il driver UV-K1 upstream, come descritto in `docs/ST7565_FRAMEBUFFER_STATUS.md`.
- `startup/startup_py32f071xx.s`: startup riscritto; riferimenti Puya/ST dichiarati nel commento del file.
- `ld/py32f071_app.ld`: script linker locale; non è una copia del linker upstream.

## Stato licenze del progetto

Alla data di questo inventario il repository non contiene un file radice `LICENSE` né un file `NOTICE` radice. La licenza complessiva delle contribuzioni originali di questo repository non è quindi dichiarata in modo univoco. Non presentare il progetto come integralmente Apache-2.0 o BSD-3-Clause sulla sola base delle licenze delle fonti consultate.

Prima di una distribuzione pubblica:
1. stabilire e dichiarare la licenza delle contribuzioni originali, con autorizzazione del titolare;
2. aggiungere i testi integrali delle licenze applicabili e gli avvisi copyright richiesti;
3. verificare nuovamente ogni file importato, dipendenza e asset;
4. mantenere questo inventario aggiornato con file, commit sorgente, licenza, modifiche e avvisi;
5. non integrare codice con licenze incompatibili con la licenza scelta per la distribuzione.

Nessun sorgente di terzi viene aggiunto da questo inventario. Le fonti elencate sono riferimenti per audit e implementazione indipendente.
