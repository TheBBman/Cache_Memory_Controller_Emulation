#include "cache.h"

cache::cache()
{
	for (int i = 0; i < L1_CACHE_SETS; i++)
		L1[i].valid = false; 
	for (int i = 0; i < VICTIM_SIZE; i++)
		victim[i].valid = false;
	for (int i = 0; i < L2_CACHE_SETS; i++)
		for (int j = 0; j < L2_CACHE_WAYS; j++)
			L2[i][j].valid = false; 

	this->myStat.missL1 = 0;
	this->myStat.missVic = 0;
	this->myStat.missL2 = 0;
	this->myStat.accL1 = 0;
	this->myStat.accVic = 0;
	this->myStat.accL2 = 0;
	
}
void cache::controller(bool MemR, bool MemW, int data, int addr, int* myMem)
{

}

bool cache::search_L1(int addr)	//Done
{
    int tag = addr/64;
	int index = (addr-64*tag)/4;
	this->myStat.accL1++;
	// Cache Hit
	if (this->L1[index].tag == tag && this->L1[index].valid) 
		return true;
	// Cache Miss
	this->myStat.missL1++;
	return false;
}

bool cache::search_victim(int addr, int MemR) //Should be done
{
	int tag = addr/4;
	int hit = -2;
	if (MemR) 
		this->myStat.accVic++;
    for (int i = 0; i < VICTIM_SIZE; i++) {
		if (this->victim[i].tag == tag && this->victim[i].valid) {
			hit = i;
			break;
		}
	}
	// Cache hit, update LRU
	if (hit+1) {
		for (int i = 0; i < VICTIM_SIZE; i++) {
			if (this->victim[i].valid) 
				this->victim[i].lru_position++;
		}
		this->victim[hit].lru_position = 0;
		return true;
	}
	// Cache Miss
	if (MemR) 
		this->myStat.missVic++;
	return false;
}

bool cache::search_L2(int addr, int MemR)
{
    return 0;
}
