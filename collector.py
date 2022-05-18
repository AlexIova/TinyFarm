#! /usr/bin/env python3

# NOTA: Per identazione vengono usati 4 spazi
import sys, struct, socket, threading

HOST = "127.0.0.1"
PORT = 65432


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



def stampaSomme(conn, addr, fine):
    while True:
        try:
            data = recv_all(conn, 4)
            assert len(data) == 4   # Deve essere un intero
            byteNome = struct.unpack("!i", data)[0]     #"!i" network byte int | grandezza nome
            # print(f"byteNome: {byteNome}")
            if byteNome == -1:
                fine.set()
                s = socket.socket(socket.AF_INET,socket.SOCK_STREAM)    
                s.connect((HOST,PORT))  # Connessione locale per non far bloccare main sull'accept. Francamente non avevo in mente altre idee
                return
            data = recv_all(conn, byteNome + 8)     # stringa + long
            nomeFile = data[:byteNome].decode("utf-8")
            somma = struct.unpack("!q", data[-8:])[0]   # "!q" network byte long long
            print(f"{somma}\t\t{nomeFile}")
            conn.sendall(struct.pack("!i",1))   # invio ACK
        except RuntimeError:    # La connessione con il thread client si è chiusa
            break


# Codice thread per gestione un client, sottoclasse di threading.Thread
class ClientThread(threading.Thread):
    def __init__(self, conn, addr, fine):
        threading.Thread.__init__(self)     # Inizializzazione superclasse
        self.conn = conn
        self.addr = addr
        self.fine = fine
    def run(self):
        stampaSomme(self.conn, self.addr, self.fine)
        


def main():
    fine = threading.Event()    # Stato evento interno thread
    # Creazione server socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:    # SOCK_STREAM == TCP
        # print(f"TIMEOUT: {s.getdefaulttimeout()}")
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((HOST, PORT))
            s.listen()
            while True:
                # print("In attesa di un client...")
                conn, addr = s.accept()
                if fine.isSet():
                    break
                t = threading.Thread(target=stampaSomme, args=(conn,addr,fine))
                t = ClientThread(conn,addr,fine)
                t.start()
        except KeyboardInterrupt:
            pass
        # print("\nVa bene smetto...\n")
        s.shutdown(socket.SHUT_RDWR)


# collector.py non prende argomenti
main()


