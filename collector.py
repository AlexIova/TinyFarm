#! /usr/bin/env python3

import sys, struct, socket, threading, socketserver

HOST = "127.0.0.1"
PORT = 65432 
 
class stampaSomme(socketserver.StreamRequestHandler):
  def handle(self):
    data = self.rfile.read(4)
    assert len(data) == 4   # Deve essere un intero
    byteNome = struct.unpack("!i", data)[0]     #"!i" network byte int
    print(f"byteNome: {byteNome}")
    if byteNome == -1:
        print("ARRIVATO -1")
        raise KeyboardInterrupt
        return
    data = self.rfile.read(byteNome+8)      # stringa + long
    nomeFile = data[:byteNome].decode("utf-8")
    somma = struct.unpack("!q", data[-8:])[0]
    print(f"{somma}\t{nomeFile}")
    self.wfile.write(struct.pack("!i",1))   # Invio segnale di ACK


# classe che specifica che voglio un TCP server gestito con i thread 
class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True
    
    
def main(host=HOST,port=PORT):
  with ThreadedTCPServer((host, port), stampaSomme) as server:
    print('In attesa dei client...')
    server.serve_forever()



main()
