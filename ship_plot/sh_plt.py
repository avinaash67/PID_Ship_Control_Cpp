#!/usr/bin/env python
# -*- coding: utf-8 -*-

import socket
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Socket details
print("Setting Socket details")
UDP_IP = "169.254.102.146"
UDP_PORT = 4245
sock = socket.socket(socket.AF_INET,  # Internet
                     socket.SOCK_DGRAM)  # UDP
sock.bind((UDP_IP, UDP_PORT))

print("Sever Created")

# Setting lists
Xmeas_l = []
Ymeas_l = []
CPU_ticks_l = []

# Setting Empty plot fig figure with axes
fig = plt.figure()
ax1 = fig.add_subplot(211)
ax2 = fig.add_subplot(212)

# Formatting plot
ax1.set_ylabel('Xmeas (m)')
ax2.set_ylabel('Ymeas (m)')
ax2.set_xlabel('CPU_Time (ms)')



def animate(i, xmeas_l, ymeas_l, cpu_ticks_l):

    # Getting Data from Socket
    data, addr = sock.recvfrom(50)  # buffer size is 1024 bytes
    meas = data.decode('UTF-8')
    meas = meas.split(':')

    # Add x and y to lists
    xmeas_l.append(round(float(meas[1]), 0))
    ymeas_l.append(round(float(meas[2]), 0))
    cpu_ticks_l.append(round(float(meas[3]), 0))

    # Limit x and y lists to 100 items
    xmeas_l = xmeas_l[-100:]
    ymeas_l = ymeas_l[-100:]
    cpu_ticks_l = cpu_ticks_l[-100:]

    # Clearing Axes and Plotting
    ax1.cla()
    ax2.cla()
    ax1.plot(cpu_ticks_l, xmeas_l)
    ax2.plot(cpu_ticks_l, ymeas_l)


print("Calling Func animation")
ani = FuncAnimation(plt.gcf(), animate, fargs=(Xmeas_l, Ymeas_l, CPU_ticks_l), interval=100)

plt.tight_layout()
plt.show()
print("showing plot")

