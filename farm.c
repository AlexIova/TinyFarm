#include "librerie.h"


#define QUI __LINE__, __FILE__
#define HOST "127.0.0.1"  // indirizzo loopback
#define PORT 65433        // porta casuale


/* handler per SIGINT */
void gestore(int s)
{
  sigBool(1);   // Dico a main che dovrà smettere, vedasi libreria
  return;
}


/* Funzione thread worker */
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

    if(strcmp(nomeFile, "$$$") == 0) break;   // Segnale terminazione

    somma = 0, i = 0;
    int f = open(nomeFile, O_RDONLY);   // Non funziona con funzioni di libreria perché binario
    if(f == -1) termina("Errore apertura file thread");

    /* Creazione socket connessione */
    if(fd_skt == 0)  fd_skt = beginSocketConnection(HOST, PORT);
    
    while(true)
    {
      /* Calcolo somma */
      e = read(f, &num, sizeof(long));
      if(e <= 0) break;
      somma += (i*num);
      i++;
    }
    
    /* Invio socket */
    int byteNome = strlen(nomeFile)+1;
    socketWritenInt(fd_skt, byteNome);    // handshake
    socketWritenString(fd_skt, nomeFile);
    socketWritenLong(fd_skt, somma);

    int ack;
    socketReadnInt(fd_skt, &ack);  // Aspettta ACK del server
    
    e = close(f);
    if(e != 0) termina("Errore chiusura fd");
  }


  if(fd_skt != 0) // Chiusura socket solo per thread che l'hanno aperta
    closeSocketConnection(fd_skt);
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

  /* Controllo immissione file */
  if(optind >= argc){
    chiudi("Uso: \n  farm [file ...] [flags]\n\nFLAGS:\n -n: numero thread\n -q: lunghezza buffer prodcons\n -t: delay tra richieste\n");
  }

  /* Controllo esistenza file */
  for(int i = optind; i < argc; i++){
    if(!checkFileExists(argv[i])){
      fprintf(stderr, "Errore: file \"%s\" non esiste\n", argv[i]);
      exit(1);
    }
  }

  /* Apertura socket MasterWorker*/
  int fd_skt = beginSocketConnection(HOST, PORT);   // Se fallisce perché server non risponde non eseguo nemmeno il resto

  /* definizione signal handler */
  struct sigaction sa;
  sa.sa_handler = gestore;
  sigaction(SIGINT, &sa, NULL);


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
    usleep(delay*1000);     // usleep lavora in microsecondi non millisecondi
    if(sigBool(0)) break;
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
  shutdownServer(fd_skt);
  closeSocketConnection(fd_skt);
  free(buffer);
	return 0;
}