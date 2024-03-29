#include "librerie.h"


void chiudi(const char *messaggio)
{
    fprintf(stderr,"%s\n", messaggio);
    exit(1);
}

bool isBigEndian()
{
  int parola = 1;   // 0x00000001
  char *b = (char *)&parola;  // dereferenzio e referenzio di nuovo. il tipo char mi serve per avere un solo byte
  if (b[0] == 1)    // se BE allora 0x00 se LE allora 0x01
    return false;
  else
    return true;
}

uint64_t hRltonl(uint64_t hostReallyLong)
{

  if(isBigEndian()) return hostReallyLong;    // già in big endian
  uint64_t n = hostReallyLong;
  return bswap_64(n);   // libreria <byteswap.h>
}

/* Funzioni per lettura/scrittura continuativa */
ssize_t readn(int fd, void *ptr, size_t n)
{  
   size_t   nleft;
   ssize_t  nread;
 
   nleft = n;
   while (nleft > 0) {
     if((nread = read(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount read so far */
     } else if (nread == 0) break; /* EOF */
     nleft -= nread;
     ptr   += nread;
   }
   return(n - nleft); /* return >= 0 */
}

ssize_t writen(int fd, void *ptr, size_t n)
{  
   size_t   nleft;
   ssize_t  nwritten;
 
   nleft = n;
   while (nleft > 0) {
     if((nwritten = write(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount written so far */
     } else if (nwritten == 0) break; 
     nleft -= nwritten;
     ptr   += nwritten;
   }
   return(n - nleft); /* return >= 0 */
}

void socketWritenLong(int fd_skt, long n)
{
    long tmp = hRltonl(n);
    ssize_t e = writen(fd_skt, &tmp, sizeof(long));
    if(e != sizeof(long)) termina ("Errore socketWritenLong");
}

void socketWritenInt(int fd_skt, int n)
{
    int tmp = htonl(n);
    ssize_t e = writen(fd_skt, &tmp, sizeof(tmp));
    if(e != sizeof(int)) termina ("Errore socketWritenInt");
}

void socketReadnInt(int fd_skt, int* n)
{
    int tmp;
    ssize_t e = readn(fd_skt, &tmp, sizeof(int));
    if(e != sizeof(int)) termina ("Errore socketReadnInt");
    *n = ntohl(tmp);
    
}

void socketWritenString(int fd_skt, char *s)
{
    ssize_t e = writen(fd_skt, s, strlen(s)+1);
    if(e != strlen(s)+1) termina ("Errore socketWritenString");
}


/* Apre connessione, in caso di fallimento termina programma */
int beginSocketConnection(char* host, int port)
{
  int fd_skt = 0;
  struct sockaddr_in serv_addr;

  // Inizializzazione socket TCP
  if ((fd_skt = socket(AF_INET, SOCK_STREAM, 6)) < 0) // 6 è il numero di protocollo per TCP
    termina("Errore creazione socket");
    
  // Assegnamento indirizzo 
  serv_addr.sin_family = AF_INET;   // Famiglia protoccoli ipv4
  serv_addr.sin_port = htons(port); // Numero porta scritto in network byte order (big-endian)
  serv_addr.sin_addr.s_addr = inet_addr(host);  // Indirizzo ipv4

  // apertura connessione
  if (connect(fd_skt, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    termina("Errore apertura connessione");

  return fd_skt;
}


void closeSocketConnection(int fd_skt)
{
  if(close(fd_skt) < 0){
    termina("Errore chiusura socket");
  }
}

void shutdownServer(int fd_skt)
{
  socketWritenInt(fd_skt, -1);
}

// modo per comunicare handler e main 
bool sigBool(int set)
{
  static int a = 0;
  if (set > 0) a++;
  if(a > 0) return true;  // E' stato settato almeno una volta da un handler
  return false;   // Non è mai stato settato da nessuno (da handler)
}

bool checkFileExists(char *nome)
{
  FILE *f = fopen(nome, "r");
  if(f != NULL){
    fclose(f);
    return true;
  }
  return false;
}


/**************************** Presi da xerrori ****************************/

void termina(const char *messaggio) {
  if(errno==0)  fprintf(stderr,"== %d == %s\n",getpid(), messaggio);
  else fprintf(stderr,"== %d == %s: %s\n",getpid(), messaggio,
              strerror(errno));
  exit(1);
}

// termina un processo con eventuale messaggio d'errore + linea e file
void xtermina(const char *messaggio, int linea, char *file) {
  if(errno==0)  fprintf(stderr,"== %d == %s\n",getpid(), messaggio);
  else fprintf(stderr,"== %d == %s: %s\n",getpid(), messaggio,
               strerror(errno));
  fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);

  exit(1);
}

// ---------- operazioni su FILE *
FILE *xfopen(const char *path, const char *mode, int linea, char *file) {
  FILE *f = fopen(path,mode);
  if(f==NULL) {
    perror("Errore apertura file");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return f;
}

// ----------- operazioni su file descriptors
void xclose(int fd, int linea, char *file) {
  int e = close(fd);
  if(e!=0) {
    perror("Errore chiusura file descriptor");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    exit(1);
  }
  return;
}


// ---- semafori POSIX (solo unnamed)

int xsem_init(sem_t *sem, int pshared, unsigned int value, int linea, char *file) {
  int e = sem_init(sem,pshared,value);
  if (e!=0) {
    perror("Errore sem_init");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}

int xsem_post(sem_t *sem, int linea, char *file) {
  int e = sem_post(sem);
  if (e!=0) {
    perror("Errore sem_post");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}

int xsem_wait(sem_t *sem, int linea, char *file) {
  int e = sem_wait(sem);
  if (e!=0) {
    perror("Errore sem_wait");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}




// ----- funzioni per thread (non scrivono il codice d'errore in errno) 


// stampa il messaggio d'errore associato al codice en 
// in maniera simile a perror
#define Buflen 100
void xperror(int en, char *msg) {
  char buf[Buflen];
  
  char *errmsg = strerror_r(en, buf, Buflen);
  if(msg!=NULL)
    fprintf(stderr,"%s: %s\n",msg, errmsg);
  else
    fprintf(stderr,"%s\n",errmsg);
}


int xpthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg, int linea, char *file) {
  int e = pthread_create(thread, attr, start_routine, arg);
  if (e!=0) {
    xperror(e, "Errore pthread_create");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;                       
}

                          
int xpthread_join(pthread_t thread, void **retval, int linea, char *file) {
  int e = pthread_join(thread, retval);
  if (e!=0) {
    xperror(e, "Errore pthread_join");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}


// mutex 
int xpthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr, int linea, char *file) {
  int e = pthread_mutex_init(mutex, attr);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_init");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }  
  return e;
}

int xpthread_mutex_destroy(pthread_mutex_t *mutex, int linea, char *file) {
  int e = pthread_mutex_destroy(mutex);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_destroy");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}

int xpthread_mutex_lock(pthread_mutex_t *mutex, int linea, char *file) {
  int e = pthread_mutex_lock(mutex);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_lock");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}

int xpthread_mutex_unlock(pthread_mutex_t *mutex, int linea, char *file) {
  int e = pthread_mutex_unlock(mutex);
  if (e!=0) {
    xperror(e, "Errore pthread_mutex_unlock");
    fprintf(stderr,"== %d == Linea: %d, File: %s\n",getpid(),linea,file);
    pthread_exit(NULL);
  }
  return e;
}
