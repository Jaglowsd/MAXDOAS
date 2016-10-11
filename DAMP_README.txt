/*!
\file README.txt

This file is formatted with Doxygen markup so that it can be parsed and
rendered directly by "make doc" into our Word-formatted user manuals
and HTML online documentation.

\tableofcontents

\section overview Overview

The data-collection.cpp file has been modified from its original distribution version
which can be found here: https://sourceforge.net/projects/seabreeze/
This program's orignal file had a simple iterative data collection process to showcase
how to use the API. Many modifcaitons were made to specialize the program for the particular use 
of designing a MAXDOAS. Go to \section details for more information about the program. The program is written
entirely in C++ using SeabreezeWrapperAPI.

\section license License

SeaBreeze is licensed under the MIT License.  Additional information may be found 
in the "LICENSE" file which should accompany this source distribution.

(Also available at http://opensource.org/licenses/MIT)

\section maxdoas MAXDOAS

A MAXDOAS is an instrument that detects halogens in the atmosphere by using a spectrometer.
Incomplete

\section details Program Details

To be completed...

\section guide How-to for program

Interfacing with UDOO
1) Ensure that PuTTY is installed on the computer that is being used to link with the UDOO.
   Additionally, make sure the UDOO is plugged in, indicated by a green LED on the board 
   and that a spectrometer is properly plugged into the UDOO, indicatded by lit LEDs.
   Finally, a sufficient light source to test the program must be provided otherwise
   the program will exit.

2) Two ways to connect using PuTTY:
   a) SSH login
      i) Enter host IP address 
      ii) Input username in username field follwed by pressing enter, and the same for password.
   b) Serial Port
      i) Change speed field from default to 115200
     ii) The number follwing COM in serial line field is not the same on every computer but the correct COM # can be found.
    	 When a physical connection between the UDOO and the current computer is made a driver for the 
	 usb should automatically be installed. Search "device manager" in windows search bar.
         Open device manager. There should be a category named "Ports" and in the drop-down menu
	 the COMs installed shoud be present. Input "COM#" in serial line where # is a the number
	 found in the device manager.
    iii) The proper COM # is selected when a black screen appears with no fields for username and password
    	 but characters can be seen on the interface when typing. That is when the username is inputted
	 followed by pressing enter, then the password follwed by pressing enter.
   A) SSH Login:
      i) Type "ldconfig" into the console and press enter in order to export the libraries needed to run the program.
   B) Serial Port:
     ii) Same instructions as SSH login

3) Directory of project:
   Provided location of project is not changed it can be found in
   ~/scripts/seabreeze-3.0.10/sample-code/cpp and the name of the program is "data-collection.cpp"

4) Before running program:
   If not already done, "ldconfig" must be inputted into the console in order to export the required libraries.
   Also, there is a parameters file located in the directory previously specified named "parameters.txt"
   that can be modified to change the behavior of the program. 

5) Text Editor
   When modifying files on the UDOO, you will need a text editor. Both emacs and vi are commonly used editors on linux
   and both are on the UDOO. Emacs is the only one I have experience with so I will refer to it when editing files. 
   Open emacs by typing "emacs -nw". There is a tutorial for emacs usage which can be accessed by 
   pressing ctrl-h t (hold down ctrl press h, then t)

6) Modifying parameters:
   i) Open emacs
      a) Type "emacs -nw" into the console
  ii) Open file
      a) Keyboard key combination ctrl-x ctrl-f
      b) Specify the location and then the name of the file 
         being opened (i.e. ~/scripts/seabreeze-3.0.10/sample-code/cpp/parameters.txt)
 iii) Modify file
      a) Information about the parameters can be found in the USAGE section
  iv) Saving
      a) Keyboard combination ctrl-x ctrl-s
   v) Exiting
      a) KILL (exit) emacs with ctrl-x ctrl-c
	OR
      b) SUSPEND (temporarily close) emacs ctrl-z
      	 suspending allows you to come back to the previous buffer, to do this type "fg" into the console

   NOTE: Hyphens indicate that ctrl must be held down while pressing characters.
 
7) Running program:
   From the specified directory above, run the program by typing 
   "./data-collection"

8) Stopping program:
   By pressing ctrl-z, the program is forcibly stopped. After doing so the command must be terminated.
   Type "ps" into the console. A list of active processes with a PID, TTY, TIME, and CMD will appear. To terminate a desired
   command, type "kill -KILL <PID>" where the PID of the command is placed in the slot for <PID>.

9) Output:
   In ~/scripts/seabreeze-3.0.10/sample-code there is a folder named "output" where the output of the program is placed.
   The location of the output can be modified from the parameters file
   

\section USAGE Parameter file usage

$ ./data-collection [--index specIndex]  [--integration intgrTime]  [--basefile foo]
	 	    [--log-level foo]    [--angles ang1,ang2,ang3,...,angN] [--angle-time aTime]
		    [--total-time tTime] [--MAX-SATURATION maxSat]  [--upper-saturation-range uppSat] 
		    [--lower-saturation-range lowSat]

Where:

--index      	takes the integral index of an enumerated Ocean Optics spectrometer (0-127) (default: 0, the first or only connected spectrometer found)
	
--integration   integration time read as microseconds. Sample data is taken to check saturation level of spectra. 
		Intgr time will vary by (+ or -) 5% depending on the max saturation of the ocean optics spectrometer 

--basefile 	output filename (i.e. '/path/to/data').  Will be automatically be suffixed with 'filename-iter-00-round-00-angle-00.csv' for each summed set of acquisitions. 				Iterations how many times the whole process occurs. (i.e. when the total time expires, iterations will increment if there is time remaining) while rounds is the 			number of times each angle is visited.

--log-level  	Developer use: one of TRACE, DEBUG, INFO, WARN, ERROR. Allow INFO to be default.

--angles     	set of angles that are visited. delimited by a comma and no white space. Any number of angles are allowed. Interpreted as degrees

--total-time 	time specified for entire data collection  defined as MINUTES, similar to angle-time

--MAX-SATURATION
		maximum saturation allowed by a spectrometer

--upper-saturation range
		upper bound for the maximum saturation allowed in a given set of data. Defined as a PERCENTAGE, no decimals required

--lower-saturation range
		lower bound for the minimum saturation allowed in a given set of data. Defined as a PERCENTAGE, no decimals required

*/
