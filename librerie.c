#include "librerie.h"

void termina(const char *messaggio)
{
  if(errno==0)  fprintf(stderr,"== %d == %s\n",getpid(), messaggio);
  else fprintf(stderr,"== %d == %s: %s\n",getpid(), messaggio, strerror(errno));
  exit(1);
}

void chiudi(const char *messaggio)
{
    fprintf(stderr,"%s\n", messaggio);
    exit(1);
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
