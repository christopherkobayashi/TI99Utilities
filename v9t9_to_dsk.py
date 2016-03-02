#! /usr/bin/python2.7

import os, sys, getopt

#Name 		Bytes 	Value 	Description
#DSSD
#Index gap 	12 	>FF 	Index mark filler
#ID sync 	6 	>00 	Synchronizes the controller for ID mark
#ID mark 	1 	>FE 	Recorded in a special way (clock >C7)
#Track 		1 	>00-FF 	Track number (normally 0-39)
#Side 		1 	>00-01 	>00=side A, >01=side B
#Sector 	1 	>00-FF 	Sector number (normally 0-8)
#Length 	1 	>01-04 	Sector size (normally >01: 256 bytes)
#CRC 		2 	crc 	Cyclic redundance check
#Separator 	~11 	>FF 	Prevents "Write sector" from erasing the ID block
#Data sync 	6 	>00 	Synchronizes the controller for data mark
#Data mark 	1 	>FB 	Recorded in a special way (clock >C7)
#Data 		256 	data 	This is the data contained in the sector
#CRC 		2 	crc 	Cyclic redundancy check
#Separator 	~36 	>FF 	Prevents "Write sector" from erasing next ID block
#End filler 	~240 	>FF 	Varies with motor speed (+/- 32 bytes)

# DSDD
#Index gap 	32 	>4E 	Index mark filler
#ID sync 	12 	>00 	Synchronizes the controller for ID mark
#MFM mark 	3 	>A1 	Recorded in a special way (missing clock 4-5)
#ID mark 	1 	>FE 	Recorded in normal MFM
#Track	 	1 	>00-FF 	Track number (normally 0-39)
#Side	 	1 	>00-01 	>00=side A, >01=side B
#Sector 	1 	>00-FF 	Sector number (normally 0-17)
#Length 	1 	>01 	Sector size (normally >01: 256 bytes)
#CRC	 	2 	crc 	Cyclic redundance check
#Separator 	~22 	>4E 	Prevents "Write sector" from erasing the ID block
#Data sync 	12 	>00 	Synchronizes the controller for data mark
#MFM mark 	3 	>A1 	Recorded in a special way (missing clock 4-5)
#Data mark 	1 	>FB 	Recorded in normal MFM
#Data 		256 	data 	This is the data contained in the sector
#CRC	 	2 	crc 	Cyclic redundancy check
#Separator 	~28 	>4E 	Prevents "Write sector" from erasing next ID block

#End filler 	~190 	>FF 	Varies with motor speed: (+/- 32 bytes)

def valid_image(track):
  i = 0
  while i < 5:
	if track[16 + 6 + (i * 334)] != chr(0xfe):
	  break
	elif track[16 + 30 + (i * 334)] != chr(0xfb):
	  break
	i = i + 1
  if i == 5:
	print "Verified DSSD"
	return 1

  while i < 5:
	if track[40 + 13 + (i * 340)] != chr(0xfe):
	  break
        elif track[40 + 57 + (i * 340)] != chr(0xfb):
	  break
	i = i + 1
  if i == 5:
	print "Verified DSDD"
	return 2
  else:
    	return 0

def usage():
  print "usage: v9t9_to_dsk.py -i <input file> -o <output file>"
  sys.exit(0)

def main(argv=None):
  buffer = ""
  inputfile = ""
  outputfile = ""

  try:
    opts, args = getopt.getopt(sys.argv[1:], "hi:o:", ["help", "input=", "output="])
  except getopt.GetoptError as err:
    usage()
  for o, a in opts:
    if o in ("-i", "--input"):
	inputfile = a
    if o in ("-o", "--output"):
	outputfile = a
    if o in ("-h", "--help"):
	usage()

  if len(inputfile) == 0 or len(outputfile) == 0:
    usage()

  with open(inputfile, "rb") as f:
    byte = f.read(1)
    while byte != "":
	buffer += byte
        byte = f.read(1)
    f.close()

  result = valid_image(buffer)
  if result == 0:
	print "invalid image, bombing out: ", result
	sys.exit(1)

  input_size = len(buffer)

# 260240 is DSSD
# 549760 is DSDD
# We don't really care about sizes, as the sector offsets in the file are
# calculated from the embedded data.  We should, however, do some sanity
# checks regarding file size and highest sector number.

  if input_size == 260240:
	print "Source is DSSD (260240)"
	density = 1
  elif input_size == 549760:
	print "Source is DSDD (549760)"
	density = 2
  else:
	print "Source has a bad size:", len(buffer)
	sys.exit(1)

  index = 0
  state = 0

  with open(outputfile, "wb") as w:

	while index < len(buffer):
	  if buffer[index] == chr(0xfe):
		state = 1
		track = ord(buffer[index+1])
		if track > 39:
		  print "Error: found track", track, "which is clearly wrong."
		  w.close()
		  sys.exit(4)
		side = ord(buffer[index+2])
		if side == 1:
		  track += 40
		sector = ord(buffer[index+3])
		if sector > 8 and density < 2:
		  print "Error: found sector", sector, "on a DSSD image."
		  w.close()
		  sys.exit(3)
		length = ord(buffer[index+4])
		if length != 1:
		  print "Error: sector length is not 1 as expected."
		  w.close()
		  sys.exit(5)
		index += 1
	  elif buffer[index] == chr(0xfb) and state == 1:
		print "track:", track, "side:", side, "sector:", sector
		state = 0
		seekto = (sector * 256) + (track * 256 * (9 * density))
		w.seek( seekto )
		for j in range(0, 256):
		  index += 1
		  w.write(buffer[index])
		index += 2	# avoid checksum causing problems
	  else:
		index += 1
  if w:	
    w.close()

if __name__ == "__main__":
        sys.exit(main())

