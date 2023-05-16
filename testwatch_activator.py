import socket
import sys
from os import system

from _socket import SO_REUSEADDR, SOL_SOCKET

HOST, PORT = '', 28222

print(f'TRY CREATE SERVER SOCKET...')
try:
    sock = socket.create_server((HOST, PORT), reuse_port=True)
except socket.error as msg:
    print(f'CREATE SERVER FAILED: {msg}')
    sys.exit()

print(f'CREATED SERVER SOCKET ON PORT: {PORT}')

sock.listen(128)

print(f'SOCKET ON PORT {PORT} LISTENING...')

while True:
    conn, addr = sock.accept()
    print(f'CONNECTION ACCEPTED ON ADDRESS: {addr[0]}:{addr[1]}')
    conn.send(b'ERROR' if system("sudo systemctl restart testwatch") != 0 else b'SUCCESS')
    conn.close()

sock.close()
