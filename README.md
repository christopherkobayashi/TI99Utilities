# TI99Utilities

This is a collection of command-line utilities that do various things with
TI-99/4A files.

hdx.py:		cross-platform HDX server.  Supports both FIAD and TIFILES
		file formats, automatically converts text files to DV80
		(and vice-versa).  Needs the pyserial python module.  See
		http://www.ti99-geek.nl/Projects/ti99hdx/ti99hdx_server.html
		for the Windows-only program that this replaces.

ti_bin_to_wav:	converts TI binary file to WAV file suitable for reading via
		cassette port.  Output is a 48kHz stereo WAV file constructed
		from sine waves.

		Usage: ti_bin_to_wav -i <input> -o <output> [-t/--tifiles] [-f/--fast]
				-t/--tifiles strips TIFILES header from input
				-f/--fast specifies the target console is
					running at 3.58MHz rather than 3MHz

		Why is "constructed from sine waves" important?  Well, the
		original TI output waveform as viewed on an oscilloscope
		looks like this:

		              *                   *           *
                 |                  * |          ||
                * |                 | |         * |
                | *                 | |       **  |
                |  |                | *       |   *
         * *** *   |               *   |     *     |
        * *   *    |         * *** |   |     |     |
    ---A-----------B---------C*---*----D-----E-----F----
                   |        *          |    *      |
                   * *   *  |          *   *       *
                    * *** | |           |  |
                          | |           |  |
                          * |           *  |
                           ||            |*
                           *             *

		... which suffers greatly from DC drift.  If the DC offset
		of the playback is off just a bit, then you'll have extra
		zero-crossings which the console will interpret as the end
		of a bit, thus corrupting the playback.

		.wav files generated with this script don't have that issue,
		as they're as close to perfectly sinusoidal as one can get
		with that sample rate and frequency.

		Why would someone want to do this?  TI binary files are
		readily available on the Internet.  TI floppy drives, however,
		are not.  It's trivial to build a TI cassette interface cable
		from a DB9 connector and a couple of mono jacks; the input
		jack is then be connected to a PC's sound card and fed the
		output from this script.

		Presto!  A cheap-and-easy way to run old TI software on that
		TI-99/4A that was collecting dust in your uncle's attic.

