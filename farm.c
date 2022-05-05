#include "librerie.h"

#define HOST "127.0.0.1"  // indirizzo loopback
#define PORT 65432        // porta casuale

int main(int argc, char *argv[])
{
  /* controllo argomenti con getopt(3)*/
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
        chiudi("Uso: \n  farm [flags]\n\nFLAGS:\n -n: numero thread\n -q: lunghezza buffer prodcons\n -t: delay tra richieste\n");
    }
  }
	// fprintf(stderr,"Valore argomenti:\nnthread = %d\nqlen = %d\ndelay = %d\n\n", nthread, qlen, delay);



  /* Creazione socket connessione*/
  int fd_skt = 0;
  struct sockaddr_in serv_addr;
  
  // Inizializzazione socket TCP
  if ((fd_skt = socket(AF_INET, SOCK_STREAM, 6)) < 0) // 6 è il numero di protocollo per TCP https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
    termina("Errore creazione socket");

  // Assegnamento indirizzo 
  serv_addr.sin_family = AF_INET;   // Famiglia protoccoli ipv4
  serv_addr.sin_port = htons(PORT); // Numero porta scritto in network byte order (big-endian)
  serv_addr.sin_addr.s_addr = inet_addr(HOST);  // Indirizzo ipv4

  // apertura connessione
  if (connect(fd_skt, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    termina("Errore apertura connessione");
    
  /* Fine socket connessione */


  /* chiusure e deallocazioni */
  if(close(fd_skt)<0)   // Chiusura socket
    termina("Errore chiusura socket");
	return 0;
}