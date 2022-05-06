#! /usr/bin/env python3

# NOTA: Per identazione vengono usati 4 spazi
import sys, struct, socket, threading

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)



# Analogo readn
def recv_all(conn,n):
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    chunk = conn.recv(min(n - bytes_recd, 1024))
    if len(chunk) == 0:
      raise RuntimeError("socket connection broken")
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks



def stampaSomme(conn, addr):
    with conn:
        # Dati ricevuti contengono handshake(int) + stringa + somma(long)
        data = recv_all(conn, 4)
        assert len(data) == 4   # Deve essere un intero
        byteNome = struct.unpack("!i", data)[0]     #"!i" network byte int 
        # print(f"Stringa passata lunga {byteNome}")
        data = recv_all(conn, byteNome + 8)     # stringa + long
        nomeFile = data[:byteNome].decode("utf-8")
        # print(f"Stringa passata {nomeFile}")
        # print(f"Grandezza DATA = {len(data)}")
        # print(f"{data}")
        somma = struct.unpack("!q", data[-8:])[0]
        # print(f"Somma {somma}")
        print(f"{somma}\t{nomeFile}")



# Codice thread per gestione un client, sottoclasse di threading.Thread
class ClientThread(threading.Thread):
    def __init__(self, conn, addr):
        threading.Thread.__init__(self)     # Inizializzazione superclasse
        self.conn = conn
        self.addr = addr
    def run(self):
        # print(f"Sono {self.ident}, gestisco {self.addr}")
        stampaSomme(self.conn, self.addr)
        


def main():
    # Creazione server socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:    # SOCK_STREAM == TCP
        try:  
            s.bind((HOST, PORT))
            s.listen()
            while True:
                # print("In attesa di un client...")
                conn, addr = s.accept()
                # print(f"Apparso un client\nconn: {conn}\taddr: {addr}")
                t = threading.Thread(target=stampaSomme, args=(conn,addr))
                t = ClientThread(conn,addr)
                t.start()
        except KeyboardInterrupt:
            pass
    print('Va bene smetto...')
    s.shutdown(socket.SHUT_RDWR)


# collector.py non prende argomenti
main()


