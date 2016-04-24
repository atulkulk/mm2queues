###Name: Atul Kulkarni
###CSCI551 Client/Server Warmup 2 Project 
###Filename: Makefile

all: mm2

mm2: parseCmdLine.o mm2.o
	g++ parseCmdLine.o mm2.o -o mm2 -lpthread -Wall

mm2.o: mm2.cpp parse.h
	g++ -c mm2.cpp -Wall

parseCmdLine.o: parseCmdLine.cpp parse.h
	g++ -c parseCmdLine.cpp -Wall 

clean:
	rm -rf mm2 mm2.o parseCmdLine.o
	
