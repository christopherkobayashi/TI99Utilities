#! /usr/bin/env python2.7

# $Id$

# Post-process with "sox ${INFILE} -c 2 ${OUTFILE}" for 44100 stereo output

# generate wav file containing sine waves
# FB36 - 20120617
# https://code.activestate.com/recipes/578168-sound-generator-using-wav-file/
# License: MIT

import math, wave, array, sys, getopt

debug = 0

duration = 1		# seconds
volume = 100		# percent
data = array.array('h') # signed short integer (-32768 to 32767) data
sampleRate = 48000	# of samples per second (standard)
numChan = 2		# of channels (1: mono, 2: stereo)
dataSize = 2		# 2 bytes for signed short ints (bit depth 16)
fast_console = 0	# 0 for 3MHz TI, 1 for 3.58MHz TI
previous_polarity = 0

bit_1 = 1400		# bit "1" is 1378 Hz @3MHz
bit_0 = 700		# bit "0" is 689 Hz @3MHz, we're being nominal here
numSamples = 34	 	# sampleRate * duration

def write_byte(byte):
	for bit in range(7, -1, -1):
		write_bit( (byte & (1 << bit) ) >> bit)

def write_bit(bit):
	global previous_polarity
	global numSamples
	global bit_1
	global bit_0
	global debug
	global numChan

	adder = 0

	if bit == 1:
		frequency = bit_1	# data "1"
	elif bit == 0:
		frequency = bit_0	# data "0"
	period = int(sampleRate / frequency)

	for i in range(numSamples):
	    sample = 32767
	    if i == 0:
		if previous_polarity == 1: # switch to negative
	    		adder = period / 2
	    sample *= round(math.sin(math.pi * 2 * ( (i + adder) % period) / period), 6)
	    if debug:
	    	print i, int(sample)
	    data.append(int(sample))
	    if numChan == 2:
	    	data.append(int(sample)) # for stereo
	    if sample < 0:
		previous_polarity = -1
	    elif sample > 0:
		previous_polarity = 1

def main(argv=None):
	tifiles = 0
	outfile = sys.stdout
	infile = sys.stdin

	global debug
	global numSamples
	global bit_1
	global bit_0

	try:
		opts, args = getopt.getopt(sys.argv[1:], "hi:o:vtfd", ["help", "input=", "output=", "tifiles", "fast", "debug"])
	except getopt.GetoptError as err:
		# print help information and exit:
#		print str(err) # will print something like "option -a not recognized"
		usage()
		sys.exit(2)
	output = None
	verbose = False
	for o, a in opts:
		if o == "-v":
			verbose = True
		elif o in ("-h", "--help"):
			usage()
			sys.exit()
		elif o in ("-i", "--input"):
			infile = a
		elif o in ("-o", "--output"):
			outfile = a
		elif o in ("-t", "--tifiles"):
			tifiles = 128
		elif o in ("-d", "--debug"):
			debug = 1
		elif o in ("-f", "--fast"):
			fast_console = 1
			bit_1 = 1600		# bit "1" is 1638 Hz @ 3.58MHz
			bit_0 = 800		# bit "0" is 806 Hz @ 3.58MHz
			numSamples = 30		# divisable by above
		else:
			assert False, "unhandled option"

	f = wave.open(outfile, 'w')
	f.setparams((numChan, dataSize, sampleRate, numSamples, "NONE", "Uncompressed"))

	binfd = open(infile, 'rb')
	binblock = 0
	binbuf = ""

	if debug:
		print "Converting", infile, "into", outfile

	if tifiles > 0:
		binfd.seek(tifiles) # skip TIFILES header
		if debug:
			print "Skipping TIFILES header."

	while(1):
		bytes = binfd.read(64)
		if bytes:
			binbuf += bytes
			binblock += 1
		else:
			break

	if binblock > 0xff:
		print "Input file size greater than 16k, aborting."
		binfd.close()
		sys.exit(5)		

	if debug:
		print binblock

	binfd.seek(tifiles) # skip TIFILES header if present

	binpointer = 0

	for i in range(768): # tape header
		write_byte(0x00)

	# control byte
	write_byte(0xff)

	for i in range(2): # block count (little-endian) (twice)
		write_byte(binblock)

	for b in range(binblock): # this is for each block

		if debug:
			print "block: ", b
		bytes_read = binfd.read(64)

		for a in range(2):
			if debug:
				print "blockcopy: ", a
			for j in range(8):
				write_byte(0x00)

			write_byte(0xff)

		# this is where the payload starts
			chksum = 0
			count = 0
			for now_byte in bytes_read:
				chksum += ord(now_byte)
				write_byte(ord(now_byte))
				count += 1

			# handle padding up to 64-byte boundary
			# 0x00 doesn't affect checksum
			while count < 64:
				write_byte(0x00)
				if debug:
					print count
				count +=1

			# checksum
			write_byte( (chksum & 0xff) )
			if debug:
				print "checksum: ", (chksum & 0xff)

	f.writeframes(data.tostring())
	f.close()
	binfd.close()

if __name__ == "__main__":
	sys.exit(main())
