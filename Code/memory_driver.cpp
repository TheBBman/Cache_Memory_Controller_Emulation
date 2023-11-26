#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <tuple>
#include <iomanip>
#include "cache.h"
using namespace std;

struct trace
{
	bool MemR; 
	bool MemW; 
	int adr; 
	int data; 
};

int main (int argc, char* argv[]) // the program runs like this: ./program <filename> <mode>
{
	// input file (i.e., test.txt)
	string filename = argv[1];
	
	ifstream fin;

	// opening file
	fin.open(filename.c_str());
	if (!fin){ // making sure the file is correctly opened
		cout << "Error opening " << filename << endl;
		exit(1);
	}
	
	// reading the text file
	string line;
	vector<trace> myTrace;
	int TraceSize = 0;
	string s1,s2,s3,s4;
    while( getline(fin,line) )
      	{
            stringstream ss(line);
            getline(ss,s1,','); 
            getline(ss,s2,','); 
            getline(ss,s3,','); 
            getline(ss,s4,',');
            myTrace.push_back(trace()); 
            myTrace[TraceSize].MemR = stoi(s1);
            myTrace[TraceSize].MemW = stoi(s2);
            myTrace[TraceSize].adr = stoi(s3);
            myTrace[TraceSize].data = stoi(s4);
            //cout<<myTrace[TraceSize].MemW << endl;
            TraceSize+=1;
        }


	// Defining cache and stat
    cache myCache;

	int traceCounter = 0;
	bool cur_MemR; 
	int cur_adr;

	// Removed all the unneeded bloat for this coding assignment
	while(traceCounter < TraceSize){
		
		cur_MemR = myTrace[traceCounter].MemR;
		cur_adr = myTrace[traceCounter].adr;
		traceCounter += 1;
		myCache.controller(cur_MemR, cur_adr);
	}
	
	double L1_miss_rate, victim_miss_rate, L2_miss_rate, AAT; 	
	
	// Get stats from cache object using stats function
	tie(L1_miss_rate, victim_miss_rate, L2_miss_rate) = myCache.get_Stats();

	// AAT Formula
	AAT = 1 + L1_miss_rate*( 1 + victim_miss_rate*( 8 + L2_miss_rate*100) );

	cout << setprecision(10) << "(" << L1_miss_rate << "," << L2_miss_rate << "," << AAT << ")" << endl;

	// closing the file
	fin.close();

	return 0;
}
