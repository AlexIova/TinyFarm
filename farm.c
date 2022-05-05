#include "librerie.h"

#define QUI __LINE__, __FILE__
#define HOST "127.0.0.1"  // indirizzo loopback
#define PORT 65432        // porta casuale




void *somma(void *args)
{
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


  /* Creazione socket connessione */
  int fd_skt = 0;
  struct sockaddr_in serv_addr;
  
  // Inizializzazione socket TCP
  if ((fd_skt = socket(AF_INET, SOCK_STREAM, 6)) < 0) // 6 è il numero di protocollo per TCP
    termina("Errore creazione socket");

  // Assegnamento indirizzo 
  serv_addr.sin_family = AF_INET;   // Famiglia protoccoli ipv4
  serv_addr.sin_port = htons(PORT); // Numero porta scritto in network byte order (big-endian)
  serv_addr.sin_addr.s_addr = inet_addr(HOST);  // Indirizzo ipv4

  // apertura connessione
  if (connect(fd_skt, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    termina("Errore apertura connessione");
  

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

  pthread_t w[nthread];
  for(int i = 0; i < nthread; i++){
    xpthread_create(&w[i], NULL, somma, &dw, QUI);
  }

  for(int i = 0; i < nthread; i++){
      xpthread_join(w[i], NULL, QUI);
  }

  /* Chiusure e deallocazioni */
  if(close(fd_skt)<0)   // Chiusura socket
    termina("Errore chiusura socket");
	return 0;
}