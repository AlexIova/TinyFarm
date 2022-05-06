#include "librerie.h"

#define QUI __LINE__, __FILE__
#define HOST "127.0.0.1"  // indirizzo loopback
#define PORT 65432        // porta casuale




void *sommaWorker(void *args)
{
  dataWork *a = (dataWork *) args;
  char *nomeFile;
  long somma, num;
  int i, e;
  int fd_skt = 0;
  while(true)
  {
    xsem_wait(a->sem_dati_presenti, QUI);
    xpthread_mutex_lock(a->tmutex, QUI);
    nomeFile = a->buffer[*(a->cindex) % a->qlen];
    *(a->cindex) += 1;
    assert(strlen(nomeFile) < 256);   // Nomi file al più 255
    xpthread_mutex_unlock(a->tmutex, QUI);
    xsem_post(a->sem_posti_liberi, QUI);

    fprintf(stderr, "Nomefile: %s\n", nomeFile);

    if(strcmp(nomeFile, "$$$") == 0) break;   // Segnale terminazione

    somma = 0, i = 0;
    int f = open(nomeFile, O_RDONLY);   // Non funziona con funzioni di libreria
    if(f == -1) termina("Errore apertura file thread");

    /* Creazione socket connessione */
    fd_skt = beginSocketConnection(HOST, PORT);
    
    while(true)
    {
      /* Calcolo somma */
      e = read(f, &num, sizeof(long));
      //fprintf(stderr, "valore di e: %d\n",e);
      //fprintf(stderr, "valore di num: %ld\n",num);
      if(e <= 0) break;
      //fprintf(stderr, "fammi uscire\n");
      somma += (i*num);
      i++;
    }
    
    /* Invio socket */
    
    // Fase di handshake
    int byteNome = strlen(nomeFile)+1;
    e = socketWritenInt(fd_skt, byteNome);
    e = socketWritenString(fd_skt, nomeFile);


    e = close(f);
    if(e != 0) termina("Errore chiusura fd");
    fprintf(stderr,"Somma: %ld\n", somma);
  }


  if(fd_skt != 0 && close(fd_skt)<0)   // Chiusura socket solo per thread che l'hanno aperta
    termina("Errore chiusura socket");
  pthread_exit(NULL);
}





int main(int argc, char *argv[])
{

  if(argc<2) {
    chiudi("Uso: \n  farm [file ...] [flags]\n\nFLAGS:\n -n: numero thread\n -q: lunghezza buffer prodcons\n -t: delay tra richieste\n");
  }

  /* Controllo argomenti con getopt(3) */
  int nthread = 4;
  int qlen = 8;
  int delay = 0;

  int opt; 
  
  while ((opt = getopt(argc, argv, "n:q:t:")) != -1) {
    switch(opt){
      case 'n':
        if (atoi(optarg) <= 0)
          chiudi("Il numero di thread non può essere negativo, terminazione...");
        else
          nthread = atoi(optarg);
        break;
      case 'q':
        if (atoi(optarg) <= 0)
          chiudi("La lunghezza del buffer non può essere negativa, terminazione...");
        else
          qlen = atoi(optarg);
        break;
      case 't':
        if (atoi(optarg) <= 0)
          chiudi("Il ritardo non può essere negativo, terminazione...");
        else
          delay = atoi(optarg);
        break;
      default:
        chiudi("Uso: \n  farm [file ...] [flags]\n\nFLAGS:\n -n: numero thread\n -q: lunghezza buffer prodcons\n -t: delay tra richieste\n");
    }
  }
	// fprintf(stderr,"Valore argomenti:\nnthread = %d\nqlen = %d\ndelay = %d\n\n", nthread, qlen, delay);

  /* Inizializzazioni thread Worker*/
  int cindex = 0;   // Indice consumatore (worker) per buffer
  int pindex = 0;   // Indice produttore (master) per buffer

  char **buffer = malloc(sizeof(char*)*qlen);

  pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;

  sem_t sem_posti_liberi, sem_dati_presenti;
  xsem_init(&sem_posti_liberi,  0,     qlen, QUI);
  xsem_init(&sem_dati_presenti, 0,        0, QUI);

  dataWork dw;
  dw.buffer = buffer;
  dw.sem_posti_liberi = &sem_posti_liberi;
  dw.sem_dati_presenti = &sem_dati_presenti;
  dw.tmutex = &tmutex;
  dw.cindex = &cindex;
  dw.qlen = qlen;

  pthread_t w[nthread-1];
  for(int i = 0; i < nthread; i++){
    xpthread_create(&w[i], NULL, sommaWorker, &dw, QUI);
  }

  /* inserimento in buffer */
  for(int i = optind; i<argc; i++){
    xsem_wait(&sem_posti_liberi, QUI);
    xpthread_mutex_lock(&tmutex, QUI);
    buffer[pindex++ % qlen] = argv[i];
    xpthread_mutex_unlock(&tmutex, QUI);
    xsem_post(&sem_dati_presenti, QUI);
  }

  /* terminazione */
  for(int i = 0; i < nthread; i++)
  {
    xsem_wait(&sem_posti_liberi, QUI);
    xpthread_mutex_lock(&tmutex, QUI);
    buffer[pindex++ % qlen] = "$$$";
    xpthread_mutex_unlock(&tmutex, QUI);
    xsem_post(&sem_dati_presenti, QUI);
  }

  /* join */
  for(int i = 0; i < nthread; i++){
      xpthread_join(w[i], NULL, QUI);
  }

  /* Chiusure e deallocazioni */

  free(buffer);
	return 0;
}