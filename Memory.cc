#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>

using namespace std;


//represents memory will eventually be of size 2000
static int memory[2000];
void memorySetup(const string filename);
int readMem(int address);
void writeMem(int address, int info);


int main(int argc, char *argv[])
{
	//if there are no arguments passed
	if(argc < 2)
	{
		cout << "No arguments found" << endl;
		return -1;
	}

	string fileName = argv[1];

	//tries to intialize and set up memory
	try
	{
		memorySetup(fileName);
	}
	catch(exception &ex)
	{
		cerr << "File not found" << endl;
		return -1;
	}

	//gets input from parent 
	string input;
	while (cin >> input)
	{
		char ch = input[0];
		int info, address;
		//read
		if(ch == 'r')
		{
			string s = input.substr(1);
			istringstream(s) >> address;
			cout << readMem(address) << endl;
		}

		//write
		else if(ch == 'w')
		{
			string delimiter = ",";
			string s = input.substr(1, input.find(delimiter));
			istringstream(s) >> address;
			s = input.substr(input.find(delimiter)+1);
			istringstream(s) >> info;
			writeMem(address, info);
		}

		//end process command
		else if(ch == 'e')
		{
			return 0;
		}
	}
}

//setup memory and read from the file
void memorySetup(const string fileName)
{

	string ln;
	int index =0;
	ifstream input;
	input.open(fileName.c_str());
	if(!input)
	{
		throw "File did not open";
		return;
	}

	while (getline(input, ln))
	{
		//skip empty lines
		if(ln.length() < 1)
			continue;
		//lines that start with zero meaning a change in location
		if(ln[0] == '.')
		{
			string s = ln.substr(1, ln.find("\\s+"));
			istringstream(s) >> index;
			continue;
		}

		//if line isnt number skip
		if(ln[0] < '0' || ln[0] > '9')
			continue;

		vector<string> token;
		boost::split(token, ln, boost::is_any_of("\\s+"));
		//skips empty lines


		if(token.size() < 1)
			continue;


		//read integer into memory
		else
		{
			int x;
			string s = token.at(0);
			istringstream(s) >> x;
			memory[index++] = x;
		}
	}
	input.close();
}


//gives data at passed address
int readMem(int address)
{
	return memory[address];
}

//writes info to given address
void writeMem(int address, int info)
{
	memory[address] = info;
}

