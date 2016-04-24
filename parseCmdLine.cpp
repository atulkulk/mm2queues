/*********************************************************************************	*/
/*												Project : CSCI551 Multi-threading Warmup 2 Project 								*/
/*												Filename: parseCommandLine.cpp (Parser Implementation)						*/
/*												Name: Atul Kulkarni																			*/
/*												E-mail: atulkulk@usc.edu																	*/
/*********************************************************************************	*/
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <cctype>

using namespace std;
//Declaration of Variables
bool lFlag = false, muFlag = false, sFlag = false, sdFlag = false, szFlag = false, numFlag = false, dFlag = false, tFlag = false;
char *traceSpecFile;
long int sz = 5, num = 20;
int probDistri = 1;
float lambda = 0.5, mu = 0.35;
unsigned long int seedval = 0;
//End of Declaration

//Description: Prints out usage information, i.e. commandline syntax.
//mm2 [-lambda lambda] [-mu mu] [-s] [-seed seedval] [-size sz] [-n num] [-d {exp|det}] [-t tsfile]
//Start of Function Usage
void Usage()
{
 	cout << "usage: mm2 [-lambda lambda] [-mu mu] [-s] [-seed seedval] [-size sz] [-n num] [-d {exp|det}] [-t tsfile]" << endl;
	cout << "       -lambda : Single stream of customers arriving to the system according to the Poisson process." << endl;
	cout << "       -mu : Service time of a customer." << endl;
	cout << "       -s : A single server should be present (server S2 is considered turned off)." << endl;
	cout << "       -seed : The value of the l_seed parameter to be used for the InitRandom() function." << endl;
	cout << "       -size : The size of Q1." << endl;
	cout << "       -n : The total number of customers to arrive." << endl;
	cout << "       -d {exp|det} : The probability distribution to use." << endl;
	cout << "       -t : tsfile is a trace specification file that is use to drive your simulation." << endl;
	exit(1);
}
//End of Usage()

//Description: This function checks whether the string given is an number
//Start of Function checkDigit()
int checkDigit(string str) {
	size_t pPos = str.find(".");
	if (pPos!=string::npos) {
		string beforeDecimal = (char*)malloc(strlen((str.substr(0, pPos)).c_str()));
		for (unsigned int i = 0; i < beforeDecimal.length(); i++) {
			if (!isdigit(beforeDecimal[i]))
			return false;
		}
		string afterDecimal = str.substr(pPos+1);
		for (unsigned int i = 0; i < afterDecimal.length(); i++) {
			if (!isdigit(afterDecimal[i]))
			return false;
		}
	} else {
		for (unsigned int i = 0; i < str.length(); i++) {
			if (!isdigit(str[i]))
			return false;
		}
	}
	return true;
}
//End of checkDigit()

//Description: This function parses the command line arguments.
//Start of Function parseCmdLine()
void parseCmdLine(int argc, char* argv[]) {
	argc--, argv++;
	for (argc--; argc >= 0; argc--, argv++) {
		if (*argv[0] == '-') {		//If 1
			if (!strcmp(*argv, "-lambda")) {						//Interarrival Time
				lFlag = true;
				if (argc <= 0) {
					cout << "malformed command" << endl;
					Usage();
				} else {
					// Reading lamda value from *argv
					argc--, argv++;
					string pstr = *argv;
					if(checkDigit(pstr)) {
						lambda = atof(pstr.c_str());
					}
					else {
						cout << "Invalid lambda value" << endl;
						Usage();	
					}	
				}
			}	//Endif 2
			else if(!strcmp(*argv, "-mu") && !muFlag)			//Service Time
			{	//Else if 1
				muFlag = true;
				if (argc <= 0) {
					cout << "malformed command" << endl;
					Usage();
				} else {
					// Reading mu value from *argv
					argc--, argv++;
					string pstr = *argv;
					if(checkDigit(pstr))
						mu = atof(pstr.c_str());
					else {
						cout << "Invalid mu value" << endl;
						Usage();	
					}	
				}
			}	//End Elseif 1
			else if(!strcmp(*argv, "-s") && !sFlag)				//If sFlag is set server 2 is off 
			{	//Elseif 2
				sFlag = true;
			}	//End Elseif 2
			else if(!strcmp(*argv, "-seed") && !sdFlag)
			{	//Elseif 3
				sdFlag = true;
				if (argc <= 0) {
					cout << "malformed command" << endl;
					Usage();
				} else {
					// Reading seed value from *argv
					argc--, argv++;
					string pstr = *argv;
					if(checkDigit(pstr))
						seedval = atol(pstr.c_str());
					else {
						cout << "Invalid seed value" << endl;
						Usage();	
					}	
				}
			}	//End Elseif 3
			else if (!strcmp(*argv, "-size") && !szFlag)		//Size of the Queue
			{	//Elseif 4
				szFlag = true;
				if (argc <= 0) {
					cout << "malformed command" << endl;
					Usage();
				} else {
					// Reading size value from *argv
					argc--, argv++;
					string pstr = *argv;
					if(checkDigit(pstr))
						sz = atoi(pstr.c_str());
					else {
						cout << "Invalid size value" << endl;
						Usage();	
					}
				}
			}	//End Elseif 4
			else if (!strcmp(*argv, "-n") && !numFlag)		//Number of Customers
			{	//Elseif 5
				numFlag = true;
				if (argc <= 0) {
					cout << "malformed command" << endl;
					Usage();
				} else {
					// Reading n value from *argv
					argc--, argv++;
					string pstr = *argv;
					if(checkDigit(pstr))
						num = atoi(pstr.c_str());
					else {
						cout << "Invalid n value" << endl;
						Usage();	
					}
				}
			}	//End Elseif 5
			else if (!strcmp(*argv, "-d") && !dFlag)			//Probability Distribution
			{	//Elseif 6
				dFlag = true;
				if (argc <= 0) {
					cout << "malformed command" << endl;
					Usage();
				} else {
					// Reading d value from *argv
					argc--, argv++;
					string pstr = *argv;
					if(!strcmp(*argv, "exp") || !strcmp(*argv, "det")) {
						if(!strcmp(*argv, "exp")){					
							probDistri = 1;
						} 
						else if (!strcmp(*argv, "det"))
						{
							probDistri = 0;
						}
					} 
					else 
					{
						cout << "Invalid d value" << endl;
						Usage();	
					}
				}
			}	//End Elseif 6
			else if (!strcmp(*argv, "-t") && !tFlag)				//Trace Specification File
			{	//Elseif 7
				tFlag = true;
				if (argc <= 0) {
					cout << "malformed command" << endl;
					Usage();
				} else {
					// Reading t value from *argv
					argc--, argv++;
					traceSpecFile = *argv;
				}				
			}	//End Elseif 7
		}	//Endif 1
	}	//End For
}
//End of Function parseCmdLine()
