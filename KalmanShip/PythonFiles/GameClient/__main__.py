from communication.comm_channel import *
#from simplethreads.ThreadPool import ThreadPool
import threading

def setup_comm():
    comm = comm_channel("127.0.0.1",4243,"127.0.0.1",4244)
    comm.registerRecvCB(App)
    t1 = threading.Thread(target=comm.recvThread, args=())
    t1.start()

def App(message):
    #extract information from message string
    textArray = str(message).split(":")
    if textArray[0] == "$MEAS":
        pos_x = float(textArray[1])
        pos_y = float(textArray[2])
        angle = float(textArray[3])

    ## solve the problem and store in a format like below "$KALMAN:0:-0.2"
    result = str("$KALMAN1:0:-0.2")

    return (True,result)

if __name__ == '__main__':
    setup_comm()