/* Sydney Ward
 * 9/10/2019
 * CS 4348.001
 * Assignment 1
 */

#include <string>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

using namespace std;

int PC, SP, IR, AC, X, Y;
int pipeOut[2], pipeIn[2];



//this class contains the "CPU" and all its functions. It also defines the "language" that is read in from the files.
class CPU
{
	int PC, SP, IR, AC, X, Y, timer, timeout;
private: 
	bool kernel;
	 //CPU reads from memory and wrties to memory and timer, and there are registers
public: 
	CPU(int to)
	{
		timeout = to;
		kernel = false;
		PC = IR = AC = X = Y = timer = 0;
		SP = 1000;
	}


private:
	//saves registers needed to stack and switches them
	void kernelMode()
	{
		kernel = true;
		int tempSP = SP;
		SP = 2000;
		push(tempSP);
		push(PC);
		push(IR);
		push(AC);
		push(X);
		push(Y);
	}

	//writes to memory
	void writeMemory(int address, int data)
	{
		stringstream ss;
		ss << "w" << address << "," << data << endl;
		string send = ss.str();
		write(pipeIn[1], send.c_str(), send.length());
	}

	//reads from memory
	int readMemory(int address)
	{
		if(address >= 1000 && !kernel)
		{
			cerr << "Error: Cannot access memory in user mode" << endl;
			exit(1);
		}
		stringstream ss;
		ss << "r" << address << endl;
		string send = ss.str();
		write(pipeIn[1], send.c_str(), send.length());
		


		char readStuff[100];
		int readInt;
		read(pipeOut[0], readStuff, 100000);
		istringstream(readStuff) >> readInt;
		return readInt;
	}

	//end memory processing
	void endMemoryProcess()
	{
		write(pipeIn[1], "e\n", 2);
	}

	//fetches and executes
public: 
	void run()
	{
		bool run = true;
		
		//will run until the user program is done
		while(run)
		{
			fetch();
			run = execute();
			if(!kernel)
			  timer++;

			//interrupt to 1000 for timeout instructions
			if(timer >= timeout)
			{
				if(!kernel)
				{
					timer = 0;
					kernelMode();
					PC = 1000;
				}
			}
		}
	}


	//fetch instruction and add to PC
private:
	void fetch()
	{
		IR = readMemory(PC++);
	}

	 //push on stack
	void push(int info)
	{
		writeMemory(--SP, info);
	}

	 //pop from stack
	int pop()
	{
		return readMemory(SP++);
	}

	 //creates the instruction definitions
public: 
	bool execute()
	{
		switch(IR)
		{
			case 1: fetch(); AC = IR; break; //loads value into the AC register
		case 2: fetch(); AC = readMemory(IR); break; //loads the address of the value into the AC register
			case 3: fetch(); AC = readMemory(readMemory(IR)); break; //load value from the address given into the AC
			case 4: fetch(); AC = readMemory(IR + X); break; //load value at address+X into AC
			case 5: fetch(); AC = readMemory(IR + Y); break; //load value at address+Y into AC
			case 6: AC = readMemory(SP + X); break; //load from SP+X into AC
			case 7: fetch(); writeMemory(IR, AC); break;//store value in AC into the address
			case 8: AC = rand() % 100 +1; break; //gets random int from o to 100 in the AC
			case 9: fetch(); 
					if(IR == 1)
						cout << AC;
					else if(IR == 2)
					  cout << (char)AC;
					break; //if port=1, writes AC as an int to the screen... if port=2, writes AC as a char to the screen
			case 10: AC += X; break; //add value in X to AC
			case 11: AC += Y; break; //add value in Y to AC
			case 12: AC -= X; break; //subtract X from AC
			case 13: AC -= Y;break; //subtract Y from AC
			case 14: X = AC; break; //copy value from AC to X
			case 15: AC = X; break; //copy value from X to AC
			case 16: Y = AC; break; //copy value from AC to Y
			case 17: AC = Y; break; //copy value from Y to AC
			case 18: SP = AC; break; //copy value from AC to SP
			case 19: AC = SP; break; //copy value from SP to AC
			case 20: fetch(); PC = IR; break; //jump to the address
			case 21: fetch(); if(AC == 0) PC = IR; break; //jump to the address only if the value in the AC is zero
			case 22: fetch(); if(AC != 0) PC = IR; break; //jump to the address if the value in the AC is not zero
			case 23: fetch(); push(PC); PC = IR;break; //push return address from the stack, jump the the address
			case 24: PC = pop(); break; //pop return address from the stack, jump to the address
			case 25: X++; break; //increment the value in X
			case 26: X--; break; //decrement the value in X
			case 27: push(AC); break; //push AC onto the stack
			case 28: AC = pop(); break; //pop from the stack into AC
			case 29: if(!kernel) {kernelMode(); PC = 1500;} break; //perform the system call
			case 30: 
						 Y = pop();
						 X = pop();
						 AC = pop();
						 IR = pop();
						 PC = pop(); 
						 SP = pop(); 
						 kernel = false; 
						 break; //return from the system call
			case 50: endMemoryProcess();
					 return false;  //end execution
			default:
					 cerr << "Error: Invalid instruction." << endl;
					 endMemoryProcess();
					 
					 return false;
		}

		return true;

	}
};



int main(int argc, char *argv[])
{
	//if there are not enough arguments
	if(argc < 3)
	{
		cerr << "Not enough arguments" << endl << "Required arguments: input filename and timer interrupt value" << endl;
		return -1;
	}

	//gets the arguments
	string filename = argv[1];
	int timer; 
	try{
		istringstream(argv[2]) >> timer;
	}
	catch(exception& ex){
		cerr << "Second argument must be an integer" << endl;
	}

	//creates the pipe and forks the process. The child runs the memory class
	if(pipe(pipeOut) < 0 || pipe(pipeIn) < 0)
	{
		write(STDERR_FILENO, "Pipe failed\n", 12);
		exit(1);
	}

	switch(fork())
	{
		case -1:
			write(STDERR_FILENO, "Fork failed\n", 12);
			exit(2);

		case 0: 
			dup2(pipeIn[0], STDIN_FILENO);
			dup2(pipeOut[1], STDOUT_FILENO);
			execlp("./mem.exe", "./mem.exe", filename.c_str(), NULL);

		default:
			sleep(1);
			CPU cpu(timer);
			cpu.run();
	}
	cerr << endl;

}

