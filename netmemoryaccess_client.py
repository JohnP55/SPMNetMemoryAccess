import socket
import json
import base64
import time

def sendData(cmd):
    cmd += '\n' * (4 - (len(cmd) % 4))
    fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    fd.connect(("<insert IP here>", 5555))
    fd.sendall(cmd.encode())

    data = fd.recv(1024)

    fd.close()
    return data

def read(addr, size):
    return sendData(f"read {addr:x} {size}")

def write(addr, data: bytes):
    cmd = f"write {addr:x} {base64.b64encode(data).decode()} {len(data)}"
    print(cmd)
    return sendData(cmd)

def msgbox(message: str):
    cmd = f"msgbox {base64.b64encode(message.encode()).decode()} {len(message)}"
    print(cmd)
    return sendData(cmd)
