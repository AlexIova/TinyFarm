#include "librerie.h"

void termina(const char *messaggio) {
  if(errno==0)  fprintf(stderr,"== %d == %s\n",getpid(), messaggio);
  else fprintf(stderr,"== %d == %s: %s\n",getpid(), messaggio, strerror(errno));
  exit(1);
}

void chiudi(const char *messaggio) {
    fprintf(stderr,"%s\n", messaggio);
    exit(1);
}