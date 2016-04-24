/*******************************************************************************	*/
/*												Project : CSCI551 Multi-threading Warmup 2 Project							*/
/*												Filename: mm2.cpp (M/M/2 Queue Implementation)								*/
/*												Name: Atul Kulkarni																		*/
/*												E-mail: atulkulk@usc.edu																*/
/*****************************************************************************	**	*/
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fstream>
#include <pthread.h>
#include <sstream>
#include <math.h>
#include <signal.h>
#include <list>
#include <signal.h>
#include "parse.h"		//Include new header file for command line parsing

#define round(X) (((X)>= 0)?(int)((X)+0.5):(int)((X)-0.5))
#define FAIL -1

using namespace std;

//Declaration of Variables
typedef struct cust {
	long customerNumber;
	double custArrTime;
	double interArrTime;
	double qEntryTime;
	double timeInQ;
	double serviceTime;
	double interArrTimeFromFile;
	double serviceTimeFromFile;
} customer;

pthread_t arrivalThread, server1Thread, server2Thread;
pthread_attr_t attr;
pthread_mutex_t stdmutex;
pthread_mutex_t countmutex;
pthread_mutex_t queuemutex;
pthread_mutex_t statmutex;
pthread_cond_t queueReady;

list<customer*> Q1;
bool s1Flag = false, s2Flag = false, timeToQuit = false;

long int qCount = 0, qRemove, custCntS1 = 0, custCntS2 = 0;
struct timeval initTime;
double totalSimulationTime = 0, totalServiceTime = 0, totalTimeInQ = 0, totalServiceTimeAtS1 = 0, totalServiceTimeAtS2 = 0, totalTimeSpentInSystem = 0;
double totalCustDropped = 0;
double avgInterArrTime = 0, avgServiceTime = 0, avgCustQ1 = 0, avgCustS1 = 0, avgCustS2 = 0, timeInSystem = 0;
double totalSquareOfTimeSpent = 0, prevCustArrTime = 0, totalInterArrTime = 0;
double avgTimeSpentInSystem = 0, stdDeviationTimeSpent = 0, custDropProbability = 0;

ifstream tFile;
size_t startpos, endpos;

sigset_t newSignal;
//End of Declaration

//Description: This function generates pseudo random number.
//Start of Function InitRandom
void InitRandom(long l_seed)
{
	if (l_seed == 0L) {
		time_t localtime=(time_t)0;
		time(&localtime);
		srand48((long)localtime);
	} else {
		srand48(l_seed);
	}
}
//End of Function InitRandom

//Description: This function gets the exponential time interval in millisecond
//Start of Function ExponentialInterval
int ExponentialInterval(double dval, double rate)
{
	double w = (double) ((log(1.0 - dval) * 1000 / (-rate)));
	if (w < 1) {
		w = 1;
	}
	else if(w > 10000) {
		w = 10000;
	}
	return w;
}
//End of Function ExponentialInterval

//Description: This function gets the time interval in millisecond
//Start of Function GetInterval
int GetInterval(int exponential, double rate)
{
	if (exponential) {
		double dval=(double)drand48();
		return ExponentialInterval(dval, rate);
	} else {
		double millisecond=((double)1000)/rate;
		if(millisecond < 1) {
			return 1;
		}
		else if (millisecond > 10000) {
			return 10000;
		}
		else
			return round(millisecond);
	}
	return 0;
}
//End of Function GetInterval 

//Description: This function handles the interrupt signal if it is invoked.
//Start of Function intForThread
void intForThread(int sig) {
	pthread_mutex_lock(&queuemutex);
		Q1.pop_front();
		qRemove++;
	pthread_mutex_unlock(&queuemutex);
	timeToQuit = true;
	pthread_mutex_lock(&queuemutex);
		pthread_cond_broadcast(&queueReady);		//Sending Signal
	pthread_mutex_unlock(&queuemutex);	
	pthread_exit(NULL);
}
//End of Function intForThread

//Description: This function calculates the customer's inter-arrival time.
//Start of Function arrThread
void *arrThread(void *null) {
	struct timeval arrTime;
	double initialTime = 0, bookkeepingTimeTotal = 0, w, currInterArrTime = 0;

	//Handling Signal
	struct sigaction act;
	act.sa_handler = intForThread;
	sigaction(SIGINT, &act, NULL);
	pthread_sigmask(SIG_UNBLOCK, &newSignal, NULL);

	pthread_mutex_lock(&stdmutex);
		printf("%012.3lfms: emulation begins\n", initialTime);
	pthread_mutex_unlock(&stdmutex);

	gettimeofday(&initTime,NULL);

	ifstream tFile(traceSpecFile, ios::in);		//Reads the trace specification file

	if(tFlag) {
		if (tFile.is_open())
		{
			string str;
			getline(tFile, str);
			num = atoi(str.c_str());
		} else {
			cout << "Error: Trace specification file reading failed!!!" << endl;
			exit(1);
		}
	}
	string numCustStr, interServStr;
	for(long i = 0; i < num; i++) {
		customer *newCust = new customer;
		if(!tFlag) {
			w = GetInterval(probDistri, lambda);
		}
		else {
			getline(tFile, interServStr);
			
			size_t pPos = interServStr.find(' ');
			if (pPos!=string::npos) {
				string inter = interServStr.substr(0, pPos);
				startpos = inter.find_first_not_of(" \t");			//Credit: http://codereflect.com/2007/01/31/how-to-trim-leading-or-trailing-spaces-of-string-in-c/
				endpos = inter.find_last_not_of(" \t");
				inter = inter.substr(startpos, endpos-startpos+1);
			
				string serv = interServStr.substr(pPos+1);
				startpos = serv.find_first_not_of(" \t");
				endpos = serv.find_last_not_of(" \t");
				serv = serv.substr(startpos, endpos-startpos+1);

				newCust->interArrTimeFromFile = atof(inter.c_str());
				newCust->serviceTimeFromFile = atof(serv.c_str());
				w = newCust->interArrTimeFromFile;
			}
		}
		
		if((w - bookkeepingTimeTotal) > 0)
			usleep((w - bookkeepingTimeTotal) * 1000);

		gettimeofday(&arrTime,NULL);
		double bookkeepingTimeStart = ((arrTime.tv_sec - initTime.tv_sec)* 1000) + ((double)(arrTime.tv_usec - initTime.tv_usec  )/1000);

		if((int)Q1.size() < 5 ) {

			gettimeofday(&arrTime,NULL);
			currInterArrTime = ((arrTime.tv_sec - initTime.tv_sec)* 1000) + ((double)(arrTime.tv_usec - initTime.tv_usec  )/1000);		
			newCust->customerNumber = i+1;
			newCust->interArrTime = currInterArrTime - prevCustArrTime;
			newCust->custArrTime = prevCustArrTime = currInterArrTime;

			pthread_mutex_lock(&stdmutex);
				printf("%012.3lfms: c%ld arrives, inter-arrival time = %.3lfms\n", currInterArrTime, newCust->customerNumber, newCust->interArrTime);
			pthread_mutex_unlock(&stdmutex);

			pthread_mutex_lock(&queuemutex);
				gettimeofday(&arrTime,NULL);	
				newCust->qEntryTime = ((arrTime.tv_sec - initTime.tv_sec)* 1000) + ((double)(arrTime.tv_usec - initTime.tv_usec  )/1000);

				Q1.push_back(newCust);							//PUSH

				pthread_mutex_lock(&stdmutex);
					printf("%012.3lfms: c%ld enters Q1\n", newCust->qEntryTime, i+1);
				pthread_mutex_unlock(&stdmutex);


				pthread_cond_broadcast(&queueReady);		//Sending Signal
			pthread_mutex_unlock(&queuemutex);
	


			gettimeofday(&arrTime,NULL);		
			double bookkeepingTimeEnd = ((arrTime.tv_sec - initTime.tv_sec)* 1000) + ((double)(arrTime.tv_usec - initTime.tv_usec)/1000);
			
			bookkeepingTimeTotal = bookkeepingTimeEnd - bookkeepingTimeStart;
		}
		else {
			gettimeofday(&arrTime,NULL);
			double dropTime = ((arrTime.tv_sec - initTime.tv_sec) * 1000) + ((double)(arrTime.tv_usec - initTime.tv_usec)/1000);
			pthread_mutex_lock(&countmutex);	
				qCount++;
			pthread_mutex_unlock(&countmutex);
			
			pthread_mutex_lock(&stdmutex);
				totalCustDropped++;
				printf("%012.3lfms: c%ld dropped\n", dropTime, i+1);
			pthread_mutex_unlock(&stdmutex);
		}
	}
	pthread_exit(NULL);
}
//End of arrThread function

//Description: This is a thread function in which the customer is served and statistics are being calculated
//Start of Function sOneThread
void *sOneThread(void *sNum) {
	customer *custNum;	
	int serNum = (int)sNum;
	double serviceEndTime, currQDepTime, w;
	struct timeval ser1Time, ser1InitTime;

	gettimeofday(&ser1InitTime,NULL);
	while(timeToQuit == false) {
		pthread_mutex_lock(&countmutex);	
		if(qCount == num) {
			pthread_mutex_unlock(&countmutex);	
			break;
		}
		pthread_mutex_unlock(&countmutex);	

		pthread_mutex_lock(&queuemutex);
		if(!Q1.empty()) {			
				custNum = Q1.front();
				Q1.pop_front();												//POP
				pthread_mutex_lock(&countmutex);	
					qCount++;
				pthread_mutex_unlock(&countmutex);
		}
		else {
			pthread_cond_wait(&queueReady, &queuemutex);		//Waiting for Customer
			pthread_mutex_unlock(&queuemutex);
			continue;
		}
		gettimeofday(&ser1Time,NULL);
		currQDepTime = (((ser1Time.tv_sec - ser1InitTime.tv_sec) * 1000) + ((double)(ser1Time.tv_usec - ser1InitTime.tv_usec)/1000));
		custNum->timeInQ = currQDepTime - custNum->qEntryTime;
		pthread_mutex_lock(&stdmutex);
			printf("%012.3lfms: c%ld leaves Q1, time in Q1 = %.3lfms\n", currQDepTime, custNum->customerNumber, custNum->timeInQ);
		pthread_mutex_unlock(&stdmutex);

		pthread_mutex_unlock(&queuemutex);

		//Customer Service Time Started
		gettimeofday(&ser1Time,NULL);
		double serviceStartTime = (((ser1Time.tv_sec - ser1InitTime.tv_sec) * 1000) + ((double)(ser1Time.tv_usec - ser1InitTime.tv_usec)/1000));
		
		pthread_mutex_lock(&stdmutex);
			printf("%012.3lfms: c%ld begin service at S%d\n", serviceStartTime, custNum->customerNumber, serNum);
		pthread_mutex_unlock(&stdmutex);

		if(!tFlag)
			w = GetInterval(probDistri, mu);
		else
			w = custNum->serviceTimeFromFile;

		usleep(w * 1000);

		//Customer Service Time Ended
		gettimeofday(&ser1Time,NULL);
		serviceEndTime = (((ser1Time.tv_sec - ser1InitTime.tv_sec) * 1000) + ((double)(ser1Time.tv_usec - ser1InitTime.tv_usec)/1000));
		//Calculating total service time for the current customer
		double serviceTotalTime = serviceEndTime - serviceStartTime;
		//Calculating total time in system for the current customer
		double timeInSystem = serviceEndTime - custNum->custArrTime;
		
		pthread_mutex_lock(&stdmutex);
			printf("%012.3lfms: c%ld departs from S%d, service time = %.3lfms, time in system = %.3lfms\n", serviceEndTime, custNum->customerNumber, serNum, serviceTotalTime, timeInSystem);
		pthread_mutex_unlock(&stdmutex);

		//Summation of all the times
		pthread_mutex_lock(&statmutex);
			totalInterArrTime = totalInterArrTime + custNum->interArrTime;
			totalTimeInQ = totalTimeInQ + custNum->timeInQ;
			totalTimeSpentInSystem = totalTimeSpentInSystem + timeInSystem;
		
			totalSquareOfTimeSpent = totalSquareOfTimeSpent + ((timeInSystem/1000) * (timeInSystem/1000));
			if(serNum == 1) {
				custCntS1++;
				totalServiceTimeAtS1 = totalServiceTimeAtS1 + serviceTotalTime;
			} else {
				custCntS2++;
				totalServiceTimeAtS2 = totalServiceTimeAtS2 + serviceTotalTime;
			}
			totalServiceTime = totalServiceTime + serviceTotalTime;
			if(totalSimulationTime <= serviceEndTime) {			//To obtain complete simulation time
				totalSimulationTime = serviceEndTime;
			}
		pthread_mutex_unlock(&statmutex);
	}
	delete custNum;
	pthread_exit(NULL);
}
//End of sOneThread

//Start of Main()
int main(int argc, char* argv[]) {
	
	sigemptyset(&newSignal);
	sigaddset(&newSignal, SIGINT);
	pthread_sigmask(SIG_BLOCK, &newSignal, NULL);

	// Initialize mutex and condition variable objects 
	pthread_mutex_init(&countmutex, NULL);
	pthread_mutex_init(&stdmutex, NULL);
	pthread_mutex_init(&queuemutex, NULL);
	pthread_mutex_init(&statmutex, NULL);
	pthread_cond_init (&queueReady, NULL);
	
	//Parsing Command Line
	parseCmdLine(argc, argv);

	if(tFlag) {
		ifstream tFile(traceSpecFile, ios::in);		//Reads the trace specification file
		if (tFile.is_open())
		{
			string str;
			getline(tFile, str);
			num = atoi(str.c_str());
		} else {
			cout << "Error: Trace specification file reading failed!!!" << endl;
			exit(1);
		}
	}
	cout << "Parameters: " << endl;
	if(!tFlag) {
		printf("\tlambda = %f\n", lambda);
		printf("\tmu = %f\n", mu);
		if(!sFlag)
			printf("\tsystem = M/M/2\n");
		else
			printf("\tsystem = M/M/1\n");
		printf("\tseed = %ld\n", seedval);
		printf("\tsize = %ld\n", sz);
		printf("\tnum = %ld\n", num);
		if(probDistri == 1)
			printf("\tdistribution = exp\n");
		else
			printf("\tdistribution = det\n");
	}
	else {
		if(!sFlag)
			printf("\tsystem = M/M/2\n");
		else
			printf("\tsystem = M/M/1\n");
		printf("\tsize = %ld\n", sz);
		printf("\tnum = %ld\n", num);
		if(probDistri == 1)
			printf("\tdistribution = exp\n");
		else
			printf("\tdistribution = det\n");
		printf("\ttsfile = %s\n", traceSpecFile);
	}

	InitRandom(seedval);																				//Random Number Generator

	if (pthread_create(&arrivalThread, NULL, arrThread, NULL) == FAIL) {				//Starting Arrival Thread
		cout << "ERROR: pthread_create() failed for arrival thread." << endl;
	}

	if (pthread_create(&server1Thread, NULL, sOneThread, (void*)1) == FAIL) {		//Starting Server 1 Thread
		cout << "ERROR: pthread_create() failed for server 1." << endl;
	}

	if(!sFlag) {
		if (pthread_create(&server2Thread, NULL, sOneThread, (void*)2) == FAIL) {	//Starting Server 2 Thread
			cout << "ERROR: pthread_create() failed for server 2." << endl;
		}
	}

	pthread_join(arrivalThread, NULL);
	pthread_join(server1Thread, NULL);
	if(!sFlag)
		pthread_join(server2Thread, NULL);

	avgInterArrTime = totalInterArrTime / ((custCntS1+custCntS2) * 1000);

	//Calculating Average Service Time
	avgServiceTime = totalServiceTime/((custCntS1+custCntS2) * 1000);

	//Calculating Average # of Customers in Q1
	avgCustQ1 = totalTimeInQ/totalSimulationTime ;

	//Calculating Average # of Customers at S1
	avgCustS1 = totalServiceTimeAtS1/totalSimulationTime ;

	//Calculating Average # of Customers at S2
	avgCustS2 = totalServiceTimeAtS2/totalSimulationTime ;

	//Calculating Average Time Spent in System
	avgTimeSpentInSystem = totalTimeSpentInSystem/((custCntS1+custCntS2) * 1000);
	

	//Calculating Standard Deviation for Time Spent in System
	double avgSquareOfTimeSpent = totalSquareOfTimeSpent/((custCntS1+custCntS2));
	double varOfTimeSpent = (avgSquareOfTimeSpent - (avgTimeSpentInSystem * avgTimeSpentInSystem));
	stdDeviationTimeSpent = sqrt(varOfTimeSpent);

	//Calculating customer drop probability
	custDropProbability = totalCustDropped/(num - qRemove);

	pthread_mutex_lock(&stdmutex);
		printf("Statistics:\n");
		printf("	average inter-arrival time = %.6gsec\n", avgInterArrTime);
		printf("	average service time = %.6gsec\n", avgServiceTime);
		printf("	average number of customers in Q1 = %.6g\n", avgCustQ1);
		printf("	average number of customers at S1 = %.6g\n", avgCustS1);
		if(!sFlag)
			printf("	average number of customers at S2 = %.6g\n", avgCustS2);
		printf("	average time spent in system = %.6gsec\n", avgTimeSpentInSystem);
		printf("	standard deviation for time spent in system = %.6gsec\n", stdDeviationTimeSpent);
		printf("	customer drop probability = %.6g\n", custDropProbability);
	pthread_mutex_unlock(&stdmutex);

	//Clean up and exit....
	fflush(stdout);
	pthread_mutex_destroy(&stdmutex);
	pthread_mutex_destroy(&countmutex);
	pthread_mutex_destroy(&queuemutex);
	pthread_mutex_destroy(&statmutex);
	pthread_cond_destroy(&queueReady);
	
	pthread_exit(NULL);
	return 0;
}
//End of Main()
