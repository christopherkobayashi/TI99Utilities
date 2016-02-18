#! /usr/bin/python2

import time
from time import sleep
import serial
import sys
import os

fiad_dir = "/home/wileyc/ti/hdx/fiad"
foad_dir = "/home/wileyc/ti/hdx/foad"
reading_fiad = 0
reading_foad = 0

foad_list = os.listdir(foad_dir)
fiad_list = os.listdir(fiad_dir)
fiad_counter = 0

# Array should be "TI file descriptor", "local FD", "local filename", "ti filename"

filename_list = []
fd_list = []
fds_open = 0

last_command = ""

debug = 1

serial_port = '/dev/ttyS1'

ser = serial.Serial(
    port=serial_port,
    baudrate=38400,
    parity=serial.PARITY_EVEN,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    rtscts=True
#    timeout=0
)

command_start = '@'
command_zero = chr(0x00)
command_one = chr(0x01)
command_open_byte = '0'
command_close_byte = '1'
command_readfile_byte = chr(0x32)
command_read_byte = chr(0x52)
command_write_byte = chr(0x57)
command_setpath_byte = 'c'
command_reqfileinfo_byte = '5'

# record 0 (which we return as record +1) contains null filename, "0" as
# filetype, BIGNUM as blocks used, same BIGNUM as filesize, then 00 and "524288"
# (prefixed by 0) as timestamp?
#
# final record is "000000" for every field, except for "03 07 14" at the end.
# it's copied from the previous file -- is Fred not initializing?

# This routine parses everything in the current directory looking for a match
def find_file_in_path(filename):
	files = os.listdir(fiad_dir)

	for candidate in files:
	  if os.path.isdir(fiad_dir + "/" + candidate):
	    return ([ filename, fiad_dir + "/" + candidate, 6 ])
	  elif os.path.isfile(fiad_dir + "/" + candidate):
	    temp = detect_file(candidate)
	    if temp[0]:
	    return temp

	return [ None, None, None ]

def detect_file(candidate):
	  # check TIFILES first, then FIAD
	  f = open(fiad_dir + "/" + candidate, "rb")
	  string = f.read(8)
	  print "checking TIFILES " + string[1:8] + " ", len(string[1:8])
	  if string[1:8] == "TIFILES":
	    f.seek(0x10)
	    filename = f.read(10).rstrip(' \t\r\n\0')
	    f.seek(0x0a)
	    type = ord(f.read(1))
	    f.close()
	    return ([ filename, fiad_dir + "/" + candidate, type ])
	  
	  # rewind, check for FIAD
	  print "checking FIAD"
	  f.seek(0)
	  filename = f.read(10).rstrip(' \t\r\n\0')
	  f.seek(0x0c)
	  type = ord(f.read(1))
	  f.close()
	  return ([ filename, fiad_dir + "/" + candidate, type ])		

def build_directory_record(file, ti_fd):
	global fiad_counter
	buffer = ""
	fullpath = fiad_dir+"/"+file
	ti_filename = ""
	file_blocks_radix = list()
	file_size_radix = list()

	if os.path.isdir(fullpath):
	  ti_filename = file
	  file_size_radix.append(chr(0x00))
	  file_blocks_radix.append(chr(0x00))
	  file_blocks = 0
	  file_size = 0
	  ti_filetype = 6
	  
	elif os.path.isfile(fullpath) and file[0] != '.':
	  file_temp = detect_file(file)
	  ti_filename = file_temp[0]
	  ti_filetype = file_temp[2]
	  if ti_filetype & 0x01:
	  	ti_filetype = 0x05 # ord ti_filetype
	  file_size = os.path.getsize(fiad_dir+"/"+file) - 128

	  file_blocks = file_size / 256
	  file_blocks_radix = int_to_radix100(file_blocks)
	  file_size_radix = int_to_radix100(file_size)

	else:
	  print "filename is weird, noop"
	  return ""

	if debug > 0 and ti_filename:
	  print ti_filename + " ", os.path.getsize(fiad_dir+"/"+file), "blocks: ", file_blocks, " size: ", file_size

	# can easily add the timestamp, as documented but not implemented
	# by Fred's application.

	# okay, build buffer

	fiad_counter += 1
	buffer = chr(0x40) + chr(0x00) + chr(0x31) + chr(0x00)
	buffer += chr(fiad_counter+1)
	buffer += chr(0x92) # fixed length
	buffer += chr(len(ti_filename))
	buffer += ti_filename
	buffer += chr(0x08) + chr(0x40)
	buffer += chr(ti_filetype)
	for i in range(0, 6):
	  buffer += chr(0x00)
	buffer += chr(0x08) + chr(0x40 + (len(file_blocks_radix))-1)
	for i in file_blocks_radix:
	  buffer += i
	for i in range(0, 7 - len(file_blocks_radix)):
	  buffer += chr(0x00)
	buffer += chr(0x08) + chr(0x40 + (len(file_size_radix))-1)
	for i in file_size_radix:
	  buffer += i
	for i in range(0, 7 - len(file_size_radix)):
	  buffer += chr(0x00)

	for i in range (len(buffer), 0x98):
	  buffer += chr(0x00)
#	if debug > 0:
#
#	  for i in buffer:
#		print "Hex: " + i.encode("hex") + " Asc: " + i
#	  print hex(len(buffer))
#	  print
#	  print
	filename_list.append([ ti_filename, fullpath] )
	return buffer

def int_to_radix100(orig_number):
	  radix = list()
	  orig_string = str(orig_number)
	  if len(orig_string) % 2 != 0:
	 	orig_string = "0"+orig_string

	  if len(orig_string) > 2:
	    for i in range (0, len(orig_string), 2):
		c = orig_string[i:i+2]
		radix.append(chr(int(c)))
	  else:
		x = int(orig_string)
		radix.append(chr(x))
	  return radix

def calc_checksum(buffer):
	checksum = 0
	for i in range(0, len(buffer)-1):
	  checksum += ord(buffer[i])
	checksum &= 0xff
	ti_checksum = ord(buffer[len(buffer)-1])
	if checksum == ti_checksum:
	  return 1
	else:
	  print "calc_checksum: MISMATCH", hex(checksum), hex(ti_checksum)
	  return 0

def serial_read(command, count, header=0):
	buffer = ""
	if header != 0:
	  buffer = chr(0x40)
	  buffer += command
	for i in range(0, count):
	  byte = ser.read(1)
	  if not byte:
	    print "hdx.py: serial_read stopped at ", i
	    continue
	  else:
#	    print "count: ", i, "value: " + byte.encode("hex")
            buffer += byte
	return buffer

def serial_write(string):
	global last_command

	checksum = 0
        count = 0

	for i in string:
	  ser.write(i)
#	  print hex(count), " - ", i.encode("hex")
	  checksum += ord(i)
	  count += 1
	checksum &= 0xff
	ser.write(chr(checksum))
#	print " - ", hex(checksum)
#	print
#	print
	sleep(.2)
	ser.flushOutput()
	last_command = string

def serial_retrans():
	print "sending retransmit"
	ser.write("#")
	ser.flushOutput

def command_close():
	global fd_list
	global fds_open

	print "hdx.py: TI request close (0x01)"

	buffer = serial_read(command_close_byte, 2, 1)
	if calc_checksum(buffer) == 0:
	  sys.exit()

	serial_write(command_start + command_zero + chr(0x31))

#	fds_open -= 1
#	print fd_close
#	del fd_list[fd_close]

def command_setpath(path):
	global fiad_dir
	global foad_dir

	print "hdx.py: TI request command_setpath"
	buffer = serial_read(command_readfile_byte, 4, 1)
	length = ( ord(buffer[3]) * 256) + ord(buffer[4])
	buffer += serial_read(command_readfile_byte, length+1)

	if calc_checksum(buffer) == 0:
	    print "hdx.py: TI request command_setpath checksum failure"
	    serial_retrans()
	    return

	dir = buffer[5:length+5].rstrip(' \t\r\n\0')
	fiad_dir = dir
#	foad_dir = dir

	serial_write(command_start + command_zero)
	print "hdx.py: TI request command_setpath " + dir


def command_readfile():
	global fd_list 
	global filename_list
	global fiad_counter

	print "hdx.py: TI request command_readfile"
	buffer = serial_read(command_readfile_byte, 4, 1)
	fd_read = ord(buffer[2])
	record = ( ord(buffer[3]) * 256) + ord(buffer[4])
	print record

	if calc_checksum(buffer) == 0:
	  print "hdx.py: TI request command_readfile checksum failure"
	  serial_retrans()
	  return

	print "reading record: ", record

	filename = "." # override!
	if filename[len(filename)-1] == "." and record == 0:
	  print "start of directory"
	  filename_list = []
	  fd_list = []
	  sector = command_start + command_zero + chr(fd_read)
	  sector += command_zero + chr(record+1) + chr(0x92)
	  sector += command_zero
	  sector += chr(0x08)
	  for i in range(0, 8):
	    sector += chr(0x00)

	  temp = int_to_radix100( 8388480 )

	  sector += chr(0x08) + chr(0x40 + (len(temp)-1))
	  for i in temp:
	    sector += i
	  sector += chr(0) + chr(0) + chr(0)
	  sector += chr(0x08)
	  for i in range(0, 8):
	    sector += chr(0x00)

	  sector += chr(0x08) + chr(0x40 + (len(temp)-1))
	  for i in temp:
	    sector += i
	  sector += chr(0) + chr(0) + chr(0)

	  for i in range (len(sector), 0x98):
	    sector += chr(0x00)
	  for i in sector:
		print " - ", i.encode("hex")
	  serial_write(sector)

	elif filename[len(filename)-1] == "." and record > 0 and record <= len(fiad_list):
	  sector = build_directory_record(fiad_list[record-1], record)
	  serial_write(sector)
	elif filename[len(filename)-1] == "." and record > len(fiad_list):
	  print "sending end of directory"
	  sector = command_start + command_zero + chr(fd_read)
	  sector += command_zero + chr(record+1) + chr(0x92)
	  sector += command_zero
	  sector += chr(0x08)
	  for i in range(0, 8):
	    sector += chr(0x00)
	  sector += chr(0x08)
	  for i in range(0, 8):
	    sector += chr(0x00)
	  sector += chr(0x08)
	  for i in range(0, 8):
	    sector += chr(0x00)
	  for i in range (len(sector), 0x98):
	    sector += chr(0x00)
	  serial_write(sector)
	  fiad_counter = 0	# reset directory pointer

	  if debug > 0:
	    for file in filename_list:
	 	print file

def command_readwrite(byte):
	command = byte
	filename = ""
	sector = ""
	file = ""

	if command == command_read_byte:
	  readin = 16
	else:
	  readin = 15

	sector = serial_read(command, readin, 1)
	byte_count = ( ord(sector[3]) * 256) + ord(sector[4])
	filename = sector[5:14].rstrip(' \t\r\n\0')
	sec_number = (ord(sector[15]) * 256) + ord(sector[16])

	print "hdx.py: filename: " + filename
	print "hdx.py: sector number ", sec_number

	if command == command_write_byte:
	  sector += serial_read(command, 256+1) # snarf checksum too
	  if calc_checksum(sector) == 0:
	    print "hdx.py: TI request "+ command + "sector", sector, "checksum failure"
	    serial_retrans()
	    f.close()
	    return
	  if sec_number == 0:
	    f = open(foad_dir + "/" + filename + ".dsk", "wb")
	  else:
	    f = open(foad_dir + "/" + filename + ".dsk", "ab")

	  f.write(sector[17:256+17])
	  f.flush()
	  serial_write(command_start + command_zero)
	elif command == command_read_byte:
	  if calc_checksum(sector) == 0:
	    print "hdx.py: TI request "+ command + "sector", sector, "checksum failure"
	    serial_retrans()
	    return
	  sector = command_start + command_one + command_zero
	  f = open(foad_dir + "/" + filename + ".dsk", "rb")
	  f.seek(sec_number * 256)
	  for i in range (0, 256):
	    byte = f.read(1)
	    if not byte:
		print filename + ".dsk is truncated!"
		f.close()
		break
	    sector += byte
	  serial_write(sector)
	    
	if f:		
	  f.close()

def command_open():
	global fd_list
	global fds_open

	filename = ""
	print "hdx.py: TI request 0x30 '0' (open file)"

	buffer = serial_read(command_open_byte, 3, 1)

	flags = ord(buffer[2])
	record_length = ord(buffer[3])
	filename_length = ord(buffer[4])

	if filename_length > 0:
	  buffer += serial_read(command_open_byte, filename_length+1) #chksum
	  filename = buffer[5:filename_length+5]
	  print "opening filename: " + filename

	if calc_checksum(buffer) == 0:
	  sys.exit()

	print "checksums match, sending ack"
	if len(fd_list) >= 8:
	  print "hdx,py: too many fds open, rejecting command"
	  # reject command here
	else:
	  fds_open += 1
	  serial_write(command_start + command_zero + chr(0x30 + fds_open) + chr(0x92))
	  print fds_open
	  if filename_length > 0 and filename[len(filename) - 1] == ".":
	    print "open: doing directory, not really opening fd, building fiad_list"
	    fiad_list = os.listdir(fiad_dir)
	    fiad_counter = 0
	    fd = None
	  elif filename_length > 0:
	    fd = open(fiad_dir + "/" + filename, "rb")
	    fd_list.append([ 0x30 + fds_open, fd, "", filename ])
	  else:
	    print "command_open: no filelength?!?"
	    sys.exit()


ser.isOpen()
ser.setDTR(True)
ser.flushInput()
ser.flushOutput()

byte = 0
command = ""

print
print "hdx.py listening for commands on " + serial_port
print

while 1:
	byte = ser.read(1)

	if byte == command_start:
	  print "hdx.py: got command start"
	  checksum = ord(byte)

	  byte = ser.read(1)
	  if byte == chr(0x56):
		print "hdx.py: DSR probed us for init"
		buffer = serial_read(chr(0x56), 3)
		if buffer[len(buffer)-1] != chr(0x99):
		  print "probe was garbage, ignoring"
		  continue
		else:
	          serial_write(command_start + command_zero + chr(0x30))
 	  	  print "init: ack sent"

	  # Opcode "0" - open file for subsequent reading (set up list, prolly)
	  elif byte == command_open_byte:
	        print "hdx.py: TI request "+ byte.encode("hex")
		command_open()

	  # Opcode "1" - close file (should tidy fds if possible)
	  elif byte == command_close_byte: # close file
	        print "hdx.py: TI request "+ byte.encode("hex")
		command_close()

	  # Opcode "2" - read up to 256 bytes from file
	  elif byte == command_readfile_byte: # read record
	        print "hdx.py: TI request "+ byte.encode("hex")
		command_readfile()

	  # Opcodes R and W -- used by DSK2PC
	  elif byte == command_read_byte or byte == command_write_byte:
	        print "hdx.py: TI request "+ byte.encode("hex")
		command_readwrite(byte)

	  # Opcode c -- set current path name
	  elif byte == command_setpath_byte:
		print "hdx.py: TI request command_setpath"
		command_setpath()

	  # Opcode 5 -- request file information (for "load program")
	  elif byte == command_req_fileinfo_byte:
		command_reqfileinfo()

	  # Retransmit your last
	  elif byte == "#":
		print "hdx.py: TI request retransmit"
		ser.write(last_command)

	  else:
		print "hdx.py: unsupported command " + byte.encode("hex")
