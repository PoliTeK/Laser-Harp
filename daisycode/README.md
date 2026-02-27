# LaserHarp Project

Un progetto di arpa laser utilizzando la piattaforma Daisy Seed.

## Descrizione

Questo progetto implementa un'arpa laser che rileva l'interruzione di raggi laser per generare suoni e note musicali. La Daisy Seed viene utilizzata per l'elaborazione audio e il controllo del sistema.

## Struttura del Progetto

- `LaserHarp.cpp` - File principale con il codice C++
- `Makefile` - File di configurazione per la compilazione
- `README.md` - Questo file

## Come Compilare

1. Assicurati di avere il toolchain ARM installato (`arm-none-eabi-gcc`)
2. Naviga nella directory del progetto
3. Esegui la compilazione:
   ```bash
   make
   ```

## Come Flashare sulla Daisy Seed

1. Metti la Daisy Seed in modalità DFU:
   - Tieni premuto il pulsante `BOOT`
   - Premi e rilascia `RESET`
   - Rilascia `BOOT`

2. Flash del firmware:
   ```bash
   make program-dfu
   ```

## Funzionalità Previste

- [ ] Rilevamento interruzione raggi laser
- [ ] Generazione di note musicali
- [ ] Output MIDI
- [ ] Effetti audio in tempo reale
- [ ] Controlli per la configurazione

## Hardware Necessario

- Daisy Seed
- Moduli laser
- Fotoresistori o fotodiodi
- Circuiti di condizionamento del segnale
- Amplificatore audio (opzionale)

## Note

Il codice attuale implementa un semplice pass-through audio e fa lampeggiare il LED per verificare che il sistema funzioni. Le funzionalità specifiche dell'arpa laser verranno implementate progressivamente.