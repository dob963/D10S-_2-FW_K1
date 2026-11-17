# Audit clock e handoff applicazione — PY32F071

Data verifica: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`, branch `main`

## Evidenze dal codice corrente

- `src/system_init.c` contiene una `SystemInit()` vuota.
- `src/hw_init.c` definisce `HW_CORE_CLOCK_HZ = 8000000` come valore predefinito e usa tale costante per i ritardi SysTick.
- Il timer SysTick viene interrogato in polling; non viene abilitato il suo interrupt.
- Il firmware viene collegato con vettore applicativo a `0x08002800`.
- `startup/startup_py32f071xx.s` non programma il registro VTOR.

## Riferimento ufficiale consultato

Repository Puya: `OpenPuya/PY32F071_Firmware`, revisione
`0ed2f4b4d3391eccfd4491006a30295fd78e32c2`, file
`Templates/PY32F071xx_Templates/Src/system_py32f071.c`.

Il file ufficiale documenta:
- HSI nominale a 8 MHz come valore iniziale di riferimento;
- selezione della frequenza HSI tramite `RCC->ICSCR`;
- divisore HSI tramite `RCC->CR`;
- selezione della sorgente SYSCLK tramite `RCC->CFGR`;
- eventuale moltiplicazione PLL e prescaler AHB;
- configurazione VTOR nel modello template, con offset predefinito zero.

Il riferimento è distribuito sotto BSD-3-Clause (avvisi Puya e ST). È stato consultato per l'audit; non è stato copiato codice nel firmware.

## Esito

**Clock non certificato.** Il fatto che il riferimento Puya usi HSI nominale da 8 MHz non dimostra che il bootloader Quansheng 7.03.01 consegni il controllo all'applicazione con HSI/HSIDIV/AHB nelle stesse condizioni. Pertanto la costante fissa da 8 MHz non può essere considerata una misura verificata.

**VTOR da verificare.** Il vettore dell'applicazione è collocato a `0x08002800`, mentre lo startup non imposta esplicitamente VTOR. Prima di abilitare interrupt o usare handler applicativi, va verificato il comportamento del bootloader al salto e, se necessario, impostata la base vettori dell'applicazione secondo il reference manual e il protocollo di avvio effettivo.

## Azioni richieste prima della release

1. Implementare una lettura indipendente dei registri clock PY32F071 e calcolare HCLK, oppure configurare esplicitamente un clock noto con sequenza Puya documentata e verificata.
2. Derivare i tempi SysTick dal clock effettivo, senza mantenere una costante presunta.
3. Verificare l'allineamento e il valore VTOR all'ingresso dell'applicazione; mantenere il polling finché il vettore interrupt non è confermato.
4. Verificare con strumentazione i tempi RF 3-wire e il refresh LCD.
5. Eseguire una prova di avvio reale con bootloader 7.03.01, senza scrivere nelle aree di calibrazione.

## Decisione

Nessuna modifica di clock viene applicata alla cieca: forzare HSI o PLL senza conoscere lo stato lasciato dal bootloader potrebbe alterare i tempi RF e compromettere il funzionamento. Questo audit è un blocco di release, non una dichiarazione di guasto del microcontrollore.
