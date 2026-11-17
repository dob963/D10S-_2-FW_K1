# Audit clock e handoff applicazione — PY32F071

Data verifica: 2026-11-17  
Repository: `dob963/D10S-_2-FW_K1`, branch `main`

## Evidenze correnti nel codice

- Il linker colloca la tabella vettori a `0x08002800`.
- Lo startup chiama `SystemInit()` prima di inizializzare .data/.bss e prima di main.
- `SystemInit()` imposta `SCB->VTOR = 0x08002800`, coerentemente con il template Puya per il target K1.
- Non viene più forzato HSI a 8 MHz: il clock lasciato dal bootloader viene preservato.
- `SystemCoreClockUpdate()` decodifica SWS, HSI_FS, HSIDIV, PLL source/multiplier e AHB prescaler dai registri RCC PY32F071.
- `delay_us()` e `delay_ms()` usano il valore HCLK decodificato, non una costante fissa da 8 MHz.

## Fonte primaria consultata

Repository: `armel/uv-k1-k5v3-firmware-custom`, commit  
`3261ca7883d5b0b60c0905d8766612dee7b4191d`.

File:
- `Core/Src/system_py32f071.c`: definizioni HSI/HSE/LSI/LSE e algoritmo SystemCoreClockUpdate.
- `Drivers/CMSIS/Device/PY32F071/Include/py32f071xB.h`: offset RCC, campi CFGR/PLLCFGR/ICSCR/CR.
- `Core/py32f071xb.ld`: origine applicazione UV-K1 `0x08002800`.

Il codice clock locale è una reimplementazione indipendente delle formule e dei campi documentati; non è copiato il file Puya.

## Esito

**Clock handoff: migliorato, non certificato su radio.** Il software ora legge la configurazione effettiva invece di sovrascriverla e presume che il clock sia 8 MHz. La lettura del valore RCC deve ancora essere confrontata con una misura sul dispositivo e con lo stato lasciato dal bootloader 7.03.01.

Il riferimento upstream F4HWN v6.3.0 espone inoltre un servizio applicativo denominato `cycles_48mhz`; questo non prova da solo che ogni handoff o ogni revisione hardware abbia HCLK a 48 MHz. Non impostare PLL o flash latency senza una verifica specifica del bootloader e del reference manual.

## Gate prima della release

1. Misurare HCLK effettivo sulla radio, oppure confrontare il calcolo RCC con una lettura SWD affidabile.
2. Confrontare la temporizzazione SysTick con oscilloscopio/logic analyzer.
3. Verificare l'indirizzo vettori e il salto del bootloader sul dispositivo.
4. Verificare i tempi del bus RF e del display.
5. Non abilitare interrupt non necessari finché l'handoff vettori non è verificato.

## Decisione

Nessun clock fisso da 8 o 48 MHz viene imposto senza prova. Il firmware preserva il clock ereditato e ne calcola HCLK dai registri. Il test fisico resta un gate obbligatorio.
