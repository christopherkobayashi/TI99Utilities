#! /usr/bin/env python2.7

import datetime
import time
from time import sleep
import signal
import serial
import sys
import os
import getopt

ser = serial.Serial()
f = ""

def control_c(signum, frame):
    print "control-c hit, closing up shop"
    if ser:
      ser.close()
    if f:
      f.close()
    sys.exit(1)

def usage():
    print "usage: printer_listener.py -d <serial device> -s <speed>"
    sys.exit(1)

def main(argv=None):
    line = ""
    speed = 0
    device = ""
    directory = "./"
    discard_junk = 0

    try:
        opts, args = getopt.getopt(sys.argv[1:], "hjd:s:f:n:j", ["help", "device=", "speed=", "filter=", "directory=", "discard_junk"]) 
    except getopt.GetoptError as err:
	usage()
    for o, a in opts:
	if o in ("-d", "--device"):
	    device = a
	if o in ("-s", "--speed"):
	    speed = a
	if o in ("-f", "--filter"):
	    filter = a
	if o in ("-n", "--directory"):
	    directory = a
	if o in ("-j", "--discard_junk"):
	    discard_junk = 1

    if speed == 0 or device == "":
	usage()

    signal.signal(signal.SIGINT, control_c)

    ser.port=device
    ser.baudrate=speed
    ser.parity=serial.PARITY_NONE
    ser.stopbits=serial.STOPBITS_ONE
    ser.bytesize=serial.EIGHTBITS
    ser.dsrdtr=True
    ser.timeout=15

    ser.open()
    ser.flush()

    while 1:

        print "printer listener listening"
        while not line:
	    sleep(5)
	    line = ser.read(1)
	    if line == chr(0xff) and discard_junk:
	        line = ""

        print "printer active"
        now = datetime.datetime.now()
        nowstring = now.strftime("%Y%m%d%H%M")
	received_file = directory+"printer_buffer-"+nowstring+".txt"
        f = open(received_file, "wb")

        while line:
  	    print line
	    f.write(line)
	    line = ser.read(10)

        print "printer inactive for 15 seconds, closing up shop."
        f.close()
        line = ""

	# we have a file, now feed it to the filter
	# my filter is a shell script that contains this:
	# /path/to/epsonps -o <temp file> ${1} 
	# gs -q -dBATCH -dNOPAUSE -sDEVICE=ljet3 -sOutputFile=- test.ps | rlpr -Plp -Hprinter

	if filter:
	    os.system("cat "+received_file+" | "+filter)

    ser.close()

if __name__ == "__main__":
    sys.exit(main())
