#! /usr/bin/env python3

# NOTA: Per identazione vengono usati 4 spazi
import sys, struct, socket, threading

HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

def main():
    # Creazione server socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:    # SOCK_STREAM == TCP
        try:  
            s.bind((HOST, PORT))
            s.listen()
            while True:
                print("In attesa di un client...")
                conn, addr = s.accept()
                print(f"Apparso un client\nconn: {conn}\taddr: {addr}")
                # t = threading.Thread(target=gestisci_connessione, args=(conn,addr))
                # t = ClientThread(conn,addr)
                # t.start()
        except KeyboardInterrupt:
            pass
    print('Va bene smetto...')
    s.shutdown(socket.SHUT_RDWR)

main()