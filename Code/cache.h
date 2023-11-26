#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <tuple>
#include <iomanip>
using namespace std;

#define L1_CACHE_SETS 16
#define L2_CACHE_SETS 16
#define VICTIM_SIZE 4
#define L2_CACHE_WAYS 8
#define MEM_SIZE 4096
#define BLOCK_SIZE 4 // bytes per block
#define DM 0
#define SA 1

struct cacheBlock
{
	int tag; 			// you need to compute offset and index to find the tag.
	int lru_position; 	// for SA only
	bool valid;
};

struct Stat
{
	double missL1; 
	double missVic;
	double missL2;
	double accL1;
	double accVic;
	double accL2;
	// add more stat if needed. Don't forget to initialize!
};

class cache 
{
private:
	cacheBlock L1[L1_CACHE_SETS]; 
	cacheBlock victim[VICTIM_SIZE];
	cacheBlock L2[L2_CACHE_SETS][L2_CACHE_WAYS]; 

	Stat myStat;

public:
	cache();
	void controller(bool MemR, bool MemW, int data, int addr, int* myMem);
	bool search_L1(int addr, int MemR);
	bool search_victim(int addr, int MemR);
	bool search_L2(int addr, int MemR);
	void insert_L1(int addr);
	std::tuple<double, double, double> get_Stats();
};


