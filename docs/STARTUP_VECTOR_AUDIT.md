# Audit startup e tabella vettori PY32F071

Data: 2026-11-17  
Target: PY32F071xB, Cortex-M0+

## Fonte tecnica consultata

Repository ufficiale Puya:
- Repository: https://github.com/OpenPuya/PY32F071_Firmware
- File: `Templates/PY32F071xx_Templates/EIDE/startup_py32f071xx.s`
- Revisione consultata: `0ed2f4b4d3391eccfd4491006a30295fd78e32c2`
- Licenza dichiarata nel file upstream: BSD-3-Clause, copyright Puya Semiconductor e STMicroelectronics.

È stata usata come riferimento per confrontare la sequenza vettori e i nomi degli IRQ. Il file del progetto resta una riscrittura indipendente; non è stato copiato il sorgente upstream.

## Difformità rilevata e corretta

Nel file locale la voce IRQ RCC era nominata `RCC_IRQHandler`, mentre la tabella ufficiale PY32F071 usa `RCC_CTC_IRQHandler` (RCC e CTC). È stato corretto sia il simbolo della voce vettore sia l'alias weak.

Commit correzione:
`b1426d5b0de5cb783e019a48c42279a065a1f509`

## Verifiche ancora aperte

- Confermare il dispositivo esatto PY32F071xB e il package montato sulla scheda.
- Verificare allineamento e posizione della tabella vettori nel contesto del bootloader Quansheng 7.03.01.
- Verificare il comportamento di reset e l'eventuale necessità di VTOR/relocation: non assumere che il solo offset linker dimostri il corretto handoff del bootloader.
- Eseguire test su hardware con debugger/lettura memoria prima di considerare validato l'avvio.
- La funzione `SystemInit` locale e il clock core restano da completare/verificare; il delay attuale non è calibrato su clock misurato.

## Stato

La corrispondenza dei nomi della tabella IRQ è stata migliorata rispetto alla fonte ufficiale. Non equivale a validazione elettrica, di bootloader o di avvio su radio.
