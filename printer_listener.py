#! /usr/bin/env python2.7

import datetime
import time
from time import sleep
import signal
import serial
import sys
import os

def control_c(signum, frame):
    print "control-c hit, closing up shop"
    if ser:
      ser.close()
    if f:
      f.close()
    sys.exit(1)

ser = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=38400,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    dsrdtr=True,
    timeout=15
)

line = ""
f = ""

signal.signal(signal.SIGINT, control_c)

ser.flush()

while 1:

    print "printer listener listening"
    while not line:
	sleep(5)
	line = ser.read(1)

    print "printer active"
    now = datetime.datetime.now()
    nowstring = now.strftime("%Y%m%d%H%M")
    f = open("printer_buffer-"+nowstring+".txt", "wb")

    while line:
	print line
	f.write(line)
	line = ser.read(10)

    print "printer inactive for 15 seconds, closing up shop."
    f.close()
    line = ""

ser.close()

