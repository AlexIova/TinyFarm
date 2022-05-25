# TinyFarms

Il progetto è composto da un client `farm` scritto in C ed un server `collector.py` scritto in python.

# Client farm
Il client farm è composto dal file principale con il main `farm.c` e da una libreria `libreria.c`.

## farm.c
### main
* Si fa un controllo con la funzione `getopt()` per avere i flag effettuando eventualmente controlli se necessario
* Prima di iniziare effettivamente con l'esecuzione si controlla la singola esistenza dei singoli file, nonostante si debba fare un ciclo con syscall assumo che i file siano relativamente pochi e che sia meno costoso fare un ciclo aggiuntivo prima di effettuare altre operazioni come apertura di socket, thread ecc.
* Si ha poi un inserimento standard nel buffer dei produttori/consumatori, il quale però viene bloccato nel caso si sia ricevuto un segnale di SIGINT.
* Una volta inserito tutto nel buffer (o prematuramente se si è ricevuto il segnale) si ha immediatamente l'invio del segnale di terminazione ai *threadWorker*.
* Una volta che i *threadWorker* hanno finito si ha a prescindere l'invio del segnale di terminazione del server da parte di *MasterWorker*

### thread
Ogni *threadWorker* entra in un loop infinito da cui può uscire o nel caso si riceva il segnale di terminazione oppure nel caso di errore. Creo una nuova socket solamente nel caso in cui quel thread non ha ancora avuto connessioni con il server.
Per mandare dati al server farò prima un handshake in cui dirò la grandezza in byte del nome (C usa la codifica ASCII quindi 1 Carattere = 1 Byte) successivamente invierò il nome insieme alla somma codificati opportunamente.

## libreria.c
In questo file sono contenute molte delle funzioni che sono utlizzate dal client. Sono messe in un file separato per maggiore chiarezza.
- `bool  isBigEndian()`: controlla l'endianess della macchina guardando come vengono codificati i byte di un intero piccolo.
- `uint64_t  hRltonl(uint64_t  hostReallyLong)`: cambia l'ordine dei byte in big endian se non sono già rappresentati in questo modo, nel caso si debba cambiare l'ordine viene utilizzata la libreria `byteswap.h`
- `bool  sigBool(int  set)`: è la funzione necessaria per comunicare tra l'handler ed il main. Dato che non vi è un modo per poter passare dati se non mediante variabili globali ho optato per l'uso di una variabile statica, essa manterrà il suo valore tra le diverse chiamate alla funzione, in questo modo è possibile verificare se essa è già stata chiamata. Viene chiamata settando la variabile statica solamente dall'handler, se si guarderà quindi successivamente il valore ed esso è stato modificato vuol dire che è stato mandato il segnale SIGINT di terminazione.
- `void  shutdownServer(int  fd_skt)`: manda il segnale di terminazione al server. Il segnale è il valore `-1` in quanto è impossibile che il numero di caratteri di un nome di un file sarà negativo.
- `int  beginSocketConnection(char* host, int  port)`: vengono fatte tutte le inizializzazioni del caso per la creazione di un socket. In essa la creazione `socket(AF_INET, SOCK_STREAM, 6)` ha parametro per il protocollo `6` in quanto è quello per il TCP.

Il resto delle funzioni sono solo per evitare di rendere troppo confusionario il main, sono anche presenti le funzioni della libreria `xerrori.c` che vengono utlizzate. 

# Server collector

Per la scrittura del server si usa l'approccio di creare delle socket con la libreria `socket` utilizzando anche i thread di python della libreria `threading` per gestire in maniera automatica lo scheduling di più richieste contemporaneamente.
L'unica parte degna di nota è la gestione del segnale di terminazione:
nel caso in cui si riceva che la lunghezza del nome è `-1` allora si usano gli `Event Object` della libreria `threading`, dei flag interni in comune a tutti i thread, per comunicare facilmente al main l'imminente terminazione.
Nel caso in cui uno dei thread riceva il segnale di terminazione setterà il flag. A questo punto server non effettuerà nuove connessioni ed i thread che avevano connessioni non continueranno.
Il thread che riceve il segnale di terminazione aprirà anche una connessione verso sè stesso, questo è per evitare il caso in cui il main si possa bloccare sulla `s.accept()`.

È anche necessario inserire l'invio di un ACK altrimenti si può cadere nel caso in cui i *threadWorker* terminano inviando tutto, a questo punto *MasterWorker* vedendo i *threadWorker* finire manda il segnale di terminazione, può accadere però a questo punto che il segnale di terminazione del *MasterWorker* arrivi prima dei dati che erano stati mandati dai *threadWorker* facendo termiare *Collector* prematuramente.

Nota: lo script è stato reso eseguibile mediante chmod.
