/***************************************************************************************/
/*												Project : CSCI551 Multi-threading Warmup 2 Project										*/
/*												Filename: parse.h (Header File Implementation)												*/
/*												Name: Atul Kulkarni																					*/
/*												E-mail: atulkulk@usc.edu																			*/
/***************************************************************************************/
#ifndef PARSE_H
#define ADD_H

//Declaration of Global Variables and Functions
extern bool lFlag, muFlag, sFlag, sdFlag, szFlag, numFlag, dFlag, tFlag;
extern char *traceSpecFile;
extern long int sz, num;
extern int probDistri;
extern float lambda, mu;
extern unsigned long int seedval;

void parseCmdLine(int argc, char* argv[]);

//End of Declaration
#endif
