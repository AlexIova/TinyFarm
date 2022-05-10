#define _GNU_SOURCE   // permette di usare estensioni GNU
#include <stdio.h>    // permette di usare scanf printf etc ...
#include <stdlib.h>   // conversioni stringa exit() etc ...
#include <stdbool.h>  // gestisce tipo bool
#include <assert.h>   // permette di usare la funzione ass
#include <string.h>   // funzioni per stringhe
#include <errno.h>    // richiesto per usare errno
#include <unistd.h>     // Per utilizzo di getopt(3)
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
/* socket*/
#include <arpa/inet.h>
#include <sys/socket.h>
/* Per invertire ordine byte */
#include <byteswap.h>

/* Struct thread Worker */
typedef struct {

    char **buffer;
    // socket
    sem_t *sem_posti_liberi;
    sem_t *sem_dati_presenti;
    pthread_mutex_t *tmutex;
    int *cindex;
    int qlen;

} dataWork;

/* Struct thread Segnali */
typedef struct {
    ;
} dataSig;

void chiudi(const char *messaggio);
bool isBigEndian();
uint64_t hRltonl(uint64_t hostReallyLong);
ssize_t readn(int fd, void *ptr, size_t n);
ssize_t writen(int fd, void *ptr, size_t n);
ssize_t socketWritenLong(int fd_skt, long n);
ssize_t socketReadnLong(int fd_skt, long n);
ssize_t socketWritenInt(int fd_skt, int n);
ssize_t socketReadnInt(int fd_skt, int n);
ssize_t socketWritenString(int fd_skt, char *s);
ssize_t socketReadnString(int fd_skt, char *s);
int beginSocketConnection(char* host, int port);
void closeSocketConnection(int fd_skt);
void shutdownServer(int fd_skt);
bool sigBool(int set);


/******* Presi da xerrori *******/
void termina(const char *s); 
void xtermina(const char *s, int linea, char *file); 

// operazioni su FILE *
FILE *xfopen(const char *path, const char *mode, int linea, char *file);

// operazioni su file descriptors
void xclose(int fd, int linea, char *file);


// semafori POSIX
sem_t *xsem_open(const char *name, int oflag, mode_t mode, unsigned int value, int linea, char *file);
int xsem_unlink(const char *name, int linea, char *file);
int xsem_close(sem_t *sem, int linea, char *file);
int xsem_init(sem_t *sem, int pshared, unsigned int value, int linea, char *file);
int xsem_post(sem_t *sem, int linea, char *file);
int xsem_wait(sem_t *sem, int linea, char *file);

// thread
void xperror(int en, char *msg);
int xpthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg, int linea, char *file);
int xpthread_join(pthread_t thread, void **retval, int linea, char *file);

// mutex 
int xpthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr, int linea, char *file);
int xpthread_mutex_destroy(pthread_mutex_t *mutex, int linea, char *file);
int xpthread_mutex_lock(pthread_mutex_t *mutex, int linea, char *file);
int xpthread_mutex_unlock(pthread_mutex_t *mutex, int linea, char *file);

