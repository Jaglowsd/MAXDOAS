/***********************************************************************//**
 * @file    data-collection.cpp
 * @date    Feb 3, 2014
 * @original-author Ocean Optics, Inc
 * @modified-author David Jaglowski
 *
 * This is a command-line utility to perform bulk data-collection via SeaBreeze.
 *
 * Invocation and arguments: see usage()
 *
 * LICENSE:
 *
 * SeaBreeze Copyright (C) 2014, Ocean Optics Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "api/SeaBreezeWrapper.h"
#include "api/seabreezeapi/SeaBreezeAPIConstants.h"
#include "common/Log.h"
#include "util.h"
#include "api/DllDecl.h"

#include <string>
#include <vector>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_EEPROM_LENGTH   15
#define MAX_DEVICES         32
#define MAX_PIXELS        2048

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// constants
////////////////////////////////////////////////////////////////////////////////

static const char *rcs_id __attribute__ ((unused)) =
    "$Header: http://gforge.oceanoptics.com/svn/seabreeze/releases/Release_2014_10_01-1730-3.0/sample-code/cpp/data-collection.cpp 1215 2014-08-07 21:52:36Z mzieg $";

////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////

int specIndex = 0;
int error = ERROR_SUCCESS;

double total_Time = -1;
double angle_Time = -1;
double upper_saturation = -1;
double lower_saturation = -1;
double MAX_SATURATION = -1;
string basefile = "data";
string tossed = "/root/scripts/seabreeze-3.0.10/sample-code/output/garbagecollector.txt";
long intgrTime = -1;

time_t begin;
time_t now;
struct tm * later;

std::vector<int> angles;
std::vector<int>::iterator itr;

static const struct option opts[] =
{
    { "basefile",   required_argument, NULL, 0 },
    { "index",      required_argument, NULL, 0 },
    { "log-level",  required_argument, NULL, 0 },
    { "integration",required_argument, NULL, 0 },
    { "angles",     required_argument, NULL, 0 },
    { "angle-time", required_argument, NULL, 0 },
    { "total-time", required_argument, NULL, 0 },
    { "MAX-SATURATION",
                    required_argument, NULL, 0 },
    { "upper-saturation-range",
                    required_argument, NULL, 0 },
    { "lower-saturation-range",
                    required_argument, NULL, 0 },
    { NULL,         no_argument,       NULL, 0 }
};

void usage() {
     puts("\ndata-collection (C) 2014, Ocean Optics Inc\n"
         "\n"
         "Usage:\n"
         "    $ ./data-collection [--index specIndex]  [--integration intgrTime]  [--basefile foo]\n"
         "                        [--log-level foo]    [--angles ang1,ang2,ang3,...,angN] [--angle-time aTime]\n"
	 "                        [--total-time tTime] [--MAX-SATURATION maxSat]  [--upper-saturation-range uppSat] \n"
	 "                        [--lower-saturation-range lowSat] \n"
         "\n"
         "Where:\n"
         "\n"
         "--index      takes the integral index of an enumerated Ocean Optics spectrometer (0-127)\n"
         "             (default: 0, the first or only connected spectrometer found)\n"
         "--integration\n"
         "             integration time read as microseconds. Sample data is taken to check saturation level of spectra. Intgr time will vary \n"
	 "             by (+ or -) 5% depending on the max saturation of the ocean optics spectrometer \n"
         "--basefile   output filename (i.e. '/path/to/data').  Will be automatically be\n"
         "             suffixed with 'filename-iter-00-round-00-angle-00.csv' for each summed set of acquisitions. Iterations how many times the whole process occurs,\n"
	 "             (i.e. when the total time expires, iterations will increment if there is time remaining) while rounds is the number of times each angle is visited. \n"
         "--log-level  Developer use: one of TRACE, DEBUG, INFO, WARN, ERROR. Allow INFO to be default.\n"
	 "--angles     set of angles that are visited. delimited by a comma and no white space. Any number of angles are allowed. Interpreted as degrees\n"
	 "--angle-time time spent at each angle defined as MINUTES (i.e. 1 minute = 1, 1/2 minute = 0.5, etc.)\n"
	 "             Conversion from seconds to minutes: (20 seconds * (1 min / 60 sec) = 0.333 mins)\n"
	 "--total-time time specified for entire data collection  defined as MINUTES, similar to angle-time\n"
	 "--MAX-SATURATION\n"
	 "             maximum saturation allowed by a spectrometer\n"
	 "--upper-saturation range\n"
	 "             upper bound for the maximum saturation allowed in a given set of data. Defined as a PERCENTAGE, no decimals required\n"
	 "--lower-saturation range\n"
	 "             lower bound for the minimum saturation allowed in a given set of data. Defined as a PERCENTAGE, no decimals required\n\n"
	 "Exit instructions: \n"
	 "             1) command: ctrl+z           --forcibly stop program\n"
	 "             2) command: ps               --retrieve active processes\n"
	 "             3) command: kill -KILL <PID> --find the proccess for 'data-collection', find the PID column and the associated value to the program name\n"
	 "                                            EXAMPLE: kill -KILL 3421 \n"
	 );
}

bool hasError(const char *label)
{
    if (error != ERROR_SUCCESS)
    {
        char msg[80];
        seabreeze_get_error_string(error, msg, sizeof(msg));
        seabreeze_log_error("error calling %s: %s\n", label, msg);
    }
    return error != 0;
} // Returns error messages logged by seabreeze functions

int parseArgs(int argc, char **n_argv)
{
    int longIndex = 0;

    // iterate over cmd-line arguments
    while (1)
    {
        // read next option
        int opt = getopt_long(argc, n_argv, "", opts, &longIndex);

        // no more options
        if (opt == -1)
            break;

        // was this a valid argument?
        if (opt == 0)
        {
            string option(opts[longIndex].name);

            if (option == "index")
                specIndex = atoi(optarg);
            else if (option == "basefile")
                basefile = optarg;
            else if (option == "log-level")
                seabreeze_log_set_level_string(optarg);
	    else if (option == "angle-time")
	        angle_Time = atof(optarg) * 60;
	    else if (option == "total-time")
	        total_Time = atof(optarg) * 60;
	    else if(option == "MAX-SATURATION")
	        MAX_SATURATION = atof(optarg);
	    else if (option == "upper-saturation-range")
	        upper_saturation = (atoi(optarg)/100.0) * MAX_SATURATION;
	    else if (option == "lower-saturation-range")
	        lower_saturation = (atoi(optarg)/100.0) * MAX_SATURATION;
	    else if (option == "integration")
     	        intgrTime = atol(optarg);
	    else if (option == "angles")
	      {
		std::vector<string> temp = OOI::Util::split(string(optarg), ',');
		
		for(std::vector<string>::iterator s_itr = temp.begin(); s_itr != temp.end(); s_itr++)
		  {
		    angles.push_back(atoi((*s_itr).c_str()));
		  }

	      }
            else
	      {
                usage();
		return -1;
	      }
        }
        else
            {
             usage();
	     return -1;
	    }
    }
    return 0;
} // Parses arguments from the input file

double findMax(double spectrum[], unsigned pixels)
{
  double max = spectrum[0];
  double temp;
  unsigned begin, end;
  end = pixels;

  for(begin = 1; begin < end; begin++)
  {
    temp = spectrum[begin];
    if(temp > max)
      {
	max = temp;
      }
  }
  return  max;
} // max value in given array

int main(int argc, char *argv[])
{
    seabreeze_log_set_level_string("info");

    ////////////////////////////////////////////////////////////////////////////
    // parse & validate arguments
    ////////////////////////////////////////////////////////////////////////////

    std::ifstream input_file;
    std::string input_name = "parameters";

    // Open the parameters file
    input_file.open(input_name.c_str(), ios::in);
    
    if (input_file.fail())
      {
        //open returns 0, the NULL pointer, on failure
	std::cout << "ERROR: File name " << input_name << " was not opened successfully." << std::endl;
	std::cout << "Check that the file is named 'parameters'." << std::endl;
	return EXIT_FAILURE;
      } // Check whether the file opend properly
    else
      {
	seabreeze_log_debug("processing cmd-line args");
	printf("INFO: Processing parameter arguments\n");

      	unsigned int i = 1;
	unsigned int j = 1;
	string str;
	string program_name;
	
	// Count the number of elements in the file to dyncamically allocate space for char**
	while (input_file >> str)	  
      	    i++;

	// Reset the file position marker to the beginning
	input_file.clear();
	input_file.seekg(input_file.beg);

	// Create a char** to store the file's parameters due to the fact that the original file would expect argv[]
	// from main to be passed as a parameter to parseArgs()
	char** n_argv = new char* [i+1];
	
	// First piece of data in argv is the filename, although it is not necessary to give n_argv this information
	// It is done for the sake of completeness
	program_name = argv[0];
	n_argv[0] = new char[program_name.length()+1];
	strcpy(n_argv[0], program_name.c_str());

	// Fill n_argv with the contents of the file
	while (input_file >> str)
	  {
	    n_argv[j] = new char[str.length()+1]; // For each piece of data we create a new char that stores the length of the string plus the null terminator
	    strcpy(n_argv[j],str.c_str()); // Copy the contents of str into n_argv as a set of characters
	    j++; // Increment the number of items in the file
	  } 
	input_file.close(); // Close the file once the contents are fully read

	int errorCode = parseArgs(j, n_argv); 
	if (errorCode == (-1)) // Check the return value of parseArgs 
	  return EXIT_FAILURE; // If something goes wrong in the function, exit the program

	// Delete all allocated space
	for(unsigned int k = 0; k < i; k++)
	  delete[] n_argv[k];
	delete[] n_argv;

      } // If the file is read properly, read and process data
      
    ///////////////////////////////////////////////////////////////////////////
    // Parameter checking
    ///////////////////////////////////////////////////////////////////////////
    
    printf("INFO: Post processing parameter check\n");

    if (angles.size() == 0)
    {
        printf("ERROR: no angles provided\n");
        usage();
	return EXIT_FAILURE;
    } 

    if (angle_Time == -1 || total_Time == -1)
    {
        printf("ERROR: times may not be formatted correctly\n");
        usage();
	return EXIT_FAILURE;
    }

    if (upper_saturation == -1 || lower_saturation == -1 || MAX_SATURATION == -1)
    {
        printf("ERROR: check inputted saturation ranges and MAX saturation\n");
        usage();
	return EXIT_FAILURE;
    }

    if (intgrTime == -1)
      {
	printf("Error: check inputted integration time\n");
	usage();
	return EXIT_FAILURE;
      }

    ////////////////////////////////////////////////////////////////////////////
    // initialize SeaBreeze
    ////////////////////////////////////////////////////////////////////////////

    // give SeaBreeze time to fully instantiate
    printf("INFO: Initialzing spectrometer\n");

    seabreeze_log_debug("initializing SeaBreeze");
    sleep(1);

    seabreeze_open_spectrometer(specIndex, &error);
    if (hasError("open_spectrometer"))
        return EXIT_FAILURE;

    unsigned pixels = seabreeze_get_formatted_spectrum_length(specIndex, &error);
    if (hasError("get_formatted_spectrum_length"))
        return EXIT_FAILURE;
    
    double wavelengths[pixels];
    seabreeze_get_wavelengths(specIndex, &error, wavelengths, sizeof(wavelengths));
    if (hasError("get_wavelengths"))
         return EXIT_FAILURE;
   
    char buffer[126];
    seabreeze_get_model(specIndex, &error, buffer, sizeof(buffer));

    long minIntgrTime = seabreeze_get_min_integration_time_microsec(specIndex, &error);
    if(intgrTime < minIntgrTime)
      {
	std::cerr << std::endl << "ERROR: Minimum integration time is: " << minIntgrTime  << " microseconds" << std::endl; 
	return EXIT_FAILURE;
      }

    ////////////////////////////////////////////////////////////////////////////
    // perform data collection
    ////////////////////////////////////////////////////////////////////////////
    
    // Initializations
    double max_sat;
    int acquisitions = 0;
    int rounds = 1;
    int iterations = 1;
    int totalSpectra;
    long totalIntegrationTime;
    bool isOverSaturated;
    bool incIntgr;
    bool decIntgr;
    bool done = false;
    bool test = false;
    
    seabreeze_log_debug("performing data collection");
    printf("INFO: Performing data collection\n");
    std::ofstream f;
    f.open(tossed.c_str());
    while(!test)
      {
	time(&begin); //Start timer by getting current time
	later = localtime(&begin); // Store the current time in a tm struct to manipulate the individual values
	later->tm_sec = later->tm_sec + total_Time; // Add the total time in seconds to the seconds member of tm struct
	
	while(!done)
	  {
	    for(itr = angles.begin(); itr != angles.end(); itr++)
	      {
		isOverSaturated = false;
		totalSpectra = 1;
		totalIntegrationTime = 0;
		// apply settings for this angle
		seabreeze_set_integration_time_microsec(specIndex, &error, intgrTime);
	        if (hasError("set_integration_time_microsec"))
		   return EXIT_FAILURE;;
		do
		  {
   		    // Take sample data		   
		    double spectra[pixels];
		    seabreeze_get_formatted_spectrum(specIndex, &error, spectra, sizeof(spectra));
		    if (hasError("get_formatted_spectrum"))
		       return EXIT_FAILURE;

		    // Find the max value in the sample and check if max is in the correct range
		    max_sat = findMax(spectra, pixels);	
		    decIntgr = (max_sat > upper_saturation) ? true : false;
		    incIntgr = (max_sat < lower_saturation) ? true : false;

		    // Check if the saturation is in the correct range, if not increase/decrease integration time
		    if (decIntgr)
		      {
			intgrTime = intgrTime * .95; // -5% of intgr time
			seabreeze_set_integration_time_microsec(specIndex, &error, intgrTime);
			if (hasError("set_integration_time_microsec"))
			  return EXIT_FAILURE;
		      } // Decrease integration time
		    else if (incIntgr)
		      {
			intgrTime = intgrTime * 1.05 ; // +5% of intgr time		        			
			seabreeze_set_integration_time_microsec(specIndex, &error, intgrTime);
			if (hasError("set_integration_time_microsec"))
			   return EXIT_FAILURE;
		      } // Increase integration time
		    
       		    } while(decIntgr || incIntgr); // Continue sampling data until the max saturation is in the correct range

		if (!incIntgr && !decIntgr)
		  {
		    acquisitions = nearbyint((angle_Time * 1000000.0) / intgrTime);
		  } // If the max_sat is in the correct range then we calculate the number of acquisitions and round to the nearest integer
		else if (acquisitions <= 0)
		  {
   		    std::cerr << "ERROR: Acquisitions were unable to be properly calculated." << std::endl
			      << "Check the angle-Time that was inputted in the parameters file." << std::endl;
		    std::cerr << "Angle-time should be written in minutes" << std::endl;
		    return EXIT_FAILURE;
		  } // If not, show error and exit program
		    
		// Gather data and sum data at current angle
		double spectrum[pixels] = {}; // Reset each index to 0
		double tmp[pixels];
		max_sat = -1; // Re-use variable for finding over saturated data during actual data collection
                for (signed acqCount = 1; acqCount <= acquisitions; acqCount++)
                {
		    seabreeze_get_formatted_spectrum(specIndex, &error, tmp, sizeof(tmp));
                    if (hasError("get_formatted_spectrum"))
		      return EXIT_FAILURE;
		    totalSpectra++; // Keeping track of total signals sent to the spectrometer to take spectra
		    totalIntegrationTime += intgrTime;		    

		    max_sat = findMax(tmp, pixels);
		    if (max_sat == MAX_SATURATION)
		      isOverSaturated = true;

		    for (unsigned pixel = 0; pixel < pixels; pixel++)
		      spectrum[pixel] += tmp[pixel];
     		} // Number of acquisitions to take
		  // Also checking if any data is over saturated
                    	
		// save summed acquisition to file
		// save total integration time and number of recorded spectra
		if(!isOverSaturated) 
		  {
		    char filename[256];
		    snprintf(filename, sizeof(filename), "%s-iter-%02u-round-%02u-angle-%02u.csv",
			     basefile.c_str(), iterations, rounds, *itr); // Name file
		    seabreeze_log_info("saving %s", filename);
		    FILE *f = fopen(filename, "w");
		    if (f != NULL)
		      {
			fprintf(f, "%s", "Total formatted spectra acquired: ");
			fprintf(f,"%d\n",totalSpectra);
			fprintf(f, "%s", "Sum of integration times: ");
			fprintf(f,"%ld\n\n",totalIntegrationTime);
			for (unsigned pixel = 0; pixel < pixels; pixel++)
			  fprintf(f, "%.2lf,%.2lf\n", wavelengths[pixel], spectrum[pixel]);
			fclose(f);
		       } // Write wavelength, spectra to file
		    else
		      {
			printf("ERROR: can't write %s\n", filename);
			return EXIT_FAILURE;
		      }
		  }
		else 
		  {
		    char filename[256];
		    snprintf(filename, sizeof(filename), "data-iter-%02u-round-%02u-angle-%02u.csv",
			     iterations, rounds, *itr); // Name file
		    std::string str(filename); // Stringify file name
		    printf("INFO: Dumping data\n");
		    
		    if(!f.fail())
		      {	
			f << str.c_str();
			f << std::endl;			
		      }
		    else
		      {
			printf("ERROR: could not open garbage file\n");
			return EXIT_FAILURE;
		      }
		  }
	      }
	    // Increase round counter, indicating how many times the set of angles has been completed
	    rounds++;
		
    	    // Retrieve the current time in a new time_t object
	    // Take the difference of end point time and the current time
	    // Negative/0 indicates that the program has reached or exceeded the time limit and must end
	    // Positive indicates there is time remaining
	    time(&now);
	    double seconds = difftime(mktime(later),now);
	    if (seconds <= 0)
	      done = true;

	  } // Runs at LEAST as long as the desired time

	// Kill program when needed
	iterations++;
	test = true;
      } // Runs indefinitely until the program is forcibly stopped
        
      
    ////////////////////////////////////////////////////////////////////////////
    // shutdown
    ////////////////////////////////////////////////////////////////////////////

    seabreeze_close_spectrometer(specIndex, &error);
    f.close();
    
    return 0;
}
