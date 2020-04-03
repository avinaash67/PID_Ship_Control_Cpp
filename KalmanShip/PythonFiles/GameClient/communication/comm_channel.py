from communication.listner_channel import *
from communication.speaker_channel import *

class comm_channel:
    def __init__(self, listner_ip_address, listner_port_num, speaker_ip_address,speaker_port_num):
        self.listner = listner_channel(listner_ip_address,listner_port_num)
        self.speaker = speaker_channel(speaker_ip_address,speaker_port_num)

    def recvThread(self):
        self.process()

    def send(self,message):
        self.speaker.send(message)

    def registerRecvCB(self, cb):
        self.listner.registerRecvCB(cb)

    def process(self):
        while (True):
            (byteCnt,msg) = self.listner.recv()
            if byteCnt > 0:
                (sendFlag,message) = self.listner.cb(msg)
                if sendFlag == True:
                    self.speaker.send(message)