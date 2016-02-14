#! /usr/bin/python2

import time
from time import sleep
import serial
import sys
import os

fiad_dir = "/home/wileyc/ti/hdx/fiad"
foad_dir = "/home/wileyc/ti/hdx/fiad"
reading_fiad = 0
reading_foad = 0

foad_list = os.listdir(foad_dir)
fiad_list = os.listdir(fiad_dir)
fiad_counter = 0

debug = 1

# record 0 (which we return as record +1) contains null filename, "0" as
# filetype, BIGNUM as blocks used, same BIGNUM as filesize, then 00 and "524288"
# (prefixed by 0) as timestamp?
#
# final record is "000000" for every field, except for "03 07 14" at the end.
# it's copied from the previous file -- is Fred not initializing?


def build_directory_record(file, ti_fd):
	global fiad_counter
	buffer = ""
	fullpath = fiad_dir+"/"+file
	ti_filename = ""
	file_blocks_radix = list()
	file_size_radix = list()

	if os.path.isdir(fullpath):
	  ti_filename = file
	  file_size_str = "0"
	  file_size = 0
	  file_size_radix.append(chr(0x00))
	  file_blocks_str = "0"
	  file_blocks = 0
	  file_blocks_radix.append(chr(0x00))
	  
	elif os.path.isfile(fullpath) and file[0] != '.':
	  f = open(fullpath, "rb")
	  ti_filename = f.read(10)
	  ti_filename = ti_filename.rstrip(' \t\r\n\0')
	  f.seek(0x0c)
	  ti_filetype = f.read(1)
	  ti_filetype = 0x05 # ord ti_filetype
	  file_size = os.path.getsize(fiad_dir+"/"+file) - 128

	  file_blocks = file_size / 256
	  file_blocks_str = str(file_blocks)
	  if len(file_blocks_str) % 2 != 0:
	 	file_blocks_str = "0"+file_blocks_str

	  if len(file_blocks_str) > 2:
	    for i in range (0, len(file_blocks_str), 2):
		c = file_blocks_str[i:i+2]
		file_blocks_radix.append(chr(int(c)))
	  else:
		file_blocks_radix.append(file_blocks_str)

	  file_size_str = str(file_size)
	  if len(file_size_str) % 2 != 0:
	 	file_size_str = "0"+file_size_str

	  for i in range (0, len(file_size_str), 2):
		c = file_size_str[i:i+2]
		file_size_radix.append(chr(int(c)))
	else:
	  print "filename is weird, noop"
	  return ""

	if debug > 0 and ti_filename:
	  print ti_filename + " ", os.path.getsize(fiad_dir+"/"+file), "blocks: " + file_blocks_str + " size: " + file_size_str
	  for i in file_blocks_radix:
	    print " - blocks radix - " + i.encode("hex")
	  for i in file_size_radix:
	    print " - size radix - " + i.encode("hex")

	 # can easily add the timestamp, as documented but not implemented
	 # by Fred's application.

	 # okay, build buffer

	fiad_counter += 1
	buffer = chr(0x40) + chr(0x00) + chr(0x30 + ti_fd) + chr(0x00)
	buffer += chr(fiad_counter)
	buffer += chr(0x92) # fixed length
	buffer += chr(len(ti_filename))
	buffer += ti_filename
	buffer += chr(0x08) + chr(0x40)
	buffer += chr(0x05)  # hardcoding to program for now -- fixme!
	for i in range(0, 7):
	  buffer += chr(0x00)
	buffer += chr(0x08) + chr(0x40 + len(file_blocks_str)-1)
	for i in file_blocks_radix:
	  buffer += i
	for i in range(0, 8 - len(file_blocks_radix)):
	  buffer += chr(0x00)
	buffer += chr(0x08) + chr(0x40 + len(file_size_str)-1)
	for i in file_size_radix:
	  buffer += i
	for i in range(0, 8 - len(file_size_radix)):
	  buffer += chr(0x00)

	for i in range (len(buffer), 0x92):
	  buffer += chr(0x00)
	if debug > 0:

	  for i in buffer:
		print "Hex: " + i.encode("hex") + " Asc: " + i
	  print hex(len(buffer))
	  print
	  print
	return buffer

def serial_write(string):
	checksum = 0
	for i in string:
	  ser.write(i)
	  checksum += ord(i)
	checksum &= 0xff
	ser.write(chr(checksum))
	sleep(.2)
	ser.flushOutput()

ser = serial.Serial(
    port='/dev/ttyS1',
    baudrate=38400,
    parity=serial.PARITY_EVEN,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    dsrdtr=False
)

command_start = '@'
command_zero = chr(0x00)
command_one = chr(0x01)
command_read = chr(0x52)
command_write = chr(0x57)

ser.isOpen()

ser.setDTR(True)
ser.flushInput()
ser.flushOutput()

sleep(1)


input=1
last_milli_time = 0
byte = 0
command = ""

while 1:
	byte = ser.read(1)

	if byte == command_start:
	  print "hdx.py: got command start"
	  checksum = ord(byte)

	  byte = ser.read(1)
	  if byte == chr(0x56):
		print "hdx.py: DSR probed us for init"
		while byte != chr(0x99): # hardcode checksum, won't change
	  	  byte = ser.read(1)
	  	  command = command + byte
		serial_write(command_start + command_zero + chr(0x30))
	  	print "init: ack sent"

	  # This works but isn't relevant to DSK2PC

	  elif byte == chr(0x30):
		print "hdx.py: TI request 0x30 '0' (open file)"
		checksum += ord(byte)
		byte = ser.read(1)
		checksum += ord(byte)
	  	print byte.encode("hex")
		flags = byte
		byte = ser.read(1)
		checksum += ord(byte)
		print byte.encode("hex")
		record_length = ord(byte)
		byte = ser.read(1)
		checksum += ord(byte)
		print byte.encode("hex")
		filename_length = ord(byte)
		for i in range(0, filename_length):
		  byte = ser.read(1)
		  checksum += ord(byte)
		  print byte.encode("hex")
		byte = ser.read(1)
		send_chksum = ord(byte)
		print "checksum: " + byte.encode("hex")
		checksum = checksum & 0xff
		print "calculated checksum", hex(checksum)

		ser.flushInput()
		ser.flushOutput()

		if send_chksum == checksum:

		  print "checksums match, sending ack"
		  checksum = 0
	  	  ser.write(chr(0x40))
		  checksum += 0x40
	  	  ser.write(chr(0x00))
		  checksum += 0x00
	  	  ser.write(chr(0x31))
		  checksum += 0x31
		  ser.write(chr(0x92))
		  checksum += 0x92
		  checksum = checksum & 0xff
		  print "calculated checksum", hex(checksum)
	  	  ser.write(chr(checksum))
		  sleep(.2)
	  	  ser.flushOutput()
		  for i in range (3, 100+1):
	  	    byte = ser.read(1)
		    print str(i) + "- " + byte.encode("hex") + " " + byte

	  # End irrelevant code

	  # Opcode "W" -- sector write (one of two routines DSK2PC uses)

	  elif byte == command_read or byte == command_write:
		command = byte
		filename = ""
		sector = ""

		checksum = 0x40
		checksum += ord(byte)
		print "hdx.py: TI request "+ command
		byte = ser.read(1)
		checksum += ord(byte)
		byte_count_hi = ser.read(1)
		checksum += ord(byte_count_hi)
		byte_count_lo = ser.read(1)
		checksum += ord(byte_count_lo)
		byte_count = (ord(byte_count_hi) * 256) + ord(byte_count_lo)
		for i in range(5, 15):
		  byte = ser.read(1)
		  checksum += ord(byte)
		  filename += byte
		filename = filename.rstrip(' \t\r\n\0')
		print "hdx.py: filename: " + filename
		sec_number_hi = ser.read(1)
		checksum += ord(sec_number_hi)
		sec_number_lo = ser.read(1)
		checksum += ord(sec_number_lo)
		sec_number = (ord(sec_number_hi) * 256) + ord(sec_number_lo)
		print "hdx.py: sector number ", sec_number

		if command == command_write:
		  if sec_number == 0:
		    f = open(filename + ".dsk", "wb")
		  else:
		    f = open(filename + ".dsk", "ab")

		  for i in range (0, 256):
		    byte = ser.read(1)
		    checksum += ord(byte)
		    sector += byte

		byte = ser.read(1)
		ti_checksum = ord(byte)
		checksum &= 0xff

		if checksum == ti_checksum:
		  if command == command_write:
		    serial_write(command_start + command_zero)
		    f.write(sector)
		  elif command == command_read:
		    sector = command_start + command_one + command_zero
		    f = open(filename + ".dsk", "rb")
		    f.seek(sec_number * 256)
		    for i in range (0, 256):
		      byte = f.read(1)
		      if not byte:
			print filename + ".dsk is truncated!"
			f.close()
			break
		      sector += byte
		    serial_write(sector)
		
		  f.close()

	  else:
		if byte != chr(00):
		  print "hdx.py: unsupported command " + byte.encode("hex")
