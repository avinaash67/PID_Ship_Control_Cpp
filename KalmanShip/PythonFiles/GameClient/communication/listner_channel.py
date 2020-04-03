import socket

class listner_channel:
    def __init__(self,ipaddress,portnumber):
        self.address = (ipaddress,portnumber)
        self.bufferSize = 1024
        self.cb = None
        self.UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
        self.UDPServerSocket.bind(self.address)

    def recv(self):
        bytesAddressPair = self.UDPServerSocket.recvfrom(self.bufferSize)
        message = bytesAddressPair[0].decode()
        print(message)
        return (str(message).__len__(),message)

    def registerRecvCB(self, cb):
        self.cb = cb






