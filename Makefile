# this is a makefile for project 1

CXX = g++

.PHONY: clean

proj1.exe: assignment1.o mem.exe
	$(CXX) -o proj1.exe assignment1.o

mem.exe: Memory.o
	$(CXX) -o mem.exe Memory.o

assignment1.o: assignment1.cc
	$(CXX) -c assignment1.cc

Memory.o: Memory.cc
	$(CXX) -c Memory.cc

clean:
	rm *.o *~ *.exe
