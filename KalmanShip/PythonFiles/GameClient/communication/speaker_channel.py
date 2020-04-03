import socket

class speaker_channel:
    def __init__(self,ipaddress,portnumber):
        self.address=(ipaddress,portnumber)
        self.UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

    def send(self,message):
        bytesToSend = str.encode(message)
        self.UDPClientSocket.sendto(bytesToSend, self.address)