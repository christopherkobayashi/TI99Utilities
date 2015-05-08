# TI99Utilities

This is a collection of command-line utilities that do various things with
TI-99/4A files.

ti_bin_to_wav:	converts TI binary file to WAV file suitable for reading via
		cassette port.  Output is a 48kHz stereo WAV file constructed
		from sine waves.

		Usage: ti_bin_to_wav -i <input> -o <output> [-t/--tifiles] [-f/--fast]
				-t/--tifiles strips TIFILES header from input
				-f/--fast specifies the target console is
					running at 3.58MHz rather than 3MHz
