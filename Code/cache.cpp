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

void cache::controller(bool MemR, int addr)
{
	if (search_L1(addr, MemR)) {
		return;
	} else {
		search_victim(addr, MemR);
		search_L2(addr, MemR);
	}
	if (MemR)
		insert_L1(addr);
}

// Only search L1 for load commands, store doesn't change anything
bool cache::search_L1(int addr, int MemR)	//Done
{
    int tag = addr/64;
	int index = (addr-64*tag)/4;
	if (MemR)
		this->myStat.accL1++;
	// Cache Hit
	if (this->L1[index].tag == tag && this->L1[index].valid) 
		return true;
	// Cache Miss
	if (MemR)
		this->myStat.missL1++;
	return false;
}

bool cache::search_victim(int addr, int MemR) //Done?
{
	int tag = addr/4;
	int hit = -1;
	if (MemR) 
		this->myStat.accVic++;
    for (int i = 0; i < VICTIM_SIZE; i++) {
		if (this->victim[i].tag == tag && this->victim[i].valid) {
			if (MemR)	// Remove current entry, insert L1 for load
				this->victim[i].valid = false;
			hit = i;
			break;
		}
	}
	// Cache hit, update LRU
	if (hit > -1) {
		for (int i = 0; i < VICTIM_SIZE; i++) {
			if (this->victim[i].valid) 
				this->victim[i].lru_position++;
		}
		// Update LRU for store
		if (!MemR)
			this->victim[hit].lru_position = 0;
		return true;
	}
	// Cache Miss
	if (MemR) 
		this->myStat.missVic++;
	return false;
}

bool cache::search_L2(int addr, int MemR)	//Done?
{
    int tag = addr/64;
	int index = (addr-64*tag)/4;
	int hit = -1;
	if (MemR)
		this->myStat.accL2++;
	for (int i = 0; i < 8; i++) {
		if (this->L2[index][i].tag == tag && this->L2[index][i].valid) {
			if (MemR)	// Remove current entry, insert into L1 on load
				this->L2[index][i].valid = false;
			hit = i;
			break;
		}
	}
	// Cache hit, update LRU
	if (hit > -1) {
		for (int i = 0; i < VICTIM_SIZE; i++) {
			if (this->L2[index][i].valid)
				this->L2[index][i].lru_position++;
		}
		// Update LRU for store
		if (!MemR)
			this->L2[index][hit].lru_position = 0;
		return true;
	}
	// Cache Miss
	if (MemR)
		this->myStat.missL2++;
	return false;
}

void cache::insert_L1(int addr)
{
	int tag = addr/64;
	int index = (addr-64*tag)/4;
	// Kick from L1 ?
	int kicked_L1_tag = this->L1[index].valid ? (this->L1[index].tag*16 + index) : -1;
	this->L1[index].tag = tag;
	this->L1[index].valid = true;
	// If someone was kicked from L1, insert them to victim
	if (kicked_L1_tag > -1) {
		int kicked_victim_tag = -1;
		int hit = -1;
		int LR_victim = 0;
		for (int i = 0; i < 4; i++) {
			if (this->victim[i].valid)
				this->victim[i].lru_position++;
			else
				hit = i;
			// Keep track of least recently used entry
			if (this->victim[i].lru_position > this->victim[LR_victim].lru_position)
				LR_victim = i;
		}
		// Empty spot available
		if (hit > -1) {
			this->victim[hit].tag = kicked_L1_tag;
			this->victim[hit].lru_position = 0;
			this->victim[hit].valid = true;
			return;
		} 
		// Kick least recent
		else {
			kicked_victim_tag = this->victim[LR_victim].tag;
			this->victim[LR_victim].tag = kicked_L1_tag;
			this->victim[LR_victim].lru_position = 0;
		}
		// If someone was kicked from victim, insert to L2
		if (kicked_victim_tag > -1) {
			int tag = kicked_victim_tag/16;
			int index = kicked_victim_tag - 16*tag;
			int hit = -2;
			int LR_L2 = 0;
			for (int i = 0; i < 8; i++) {
				if (this->L2[index][i].valid)
					this->L2[index][i].lru_position++;
				else 
					hit = i;
				if (this->L2[index][i].lru_position > this->L2[index][LR_L2].lru_position)
					LR_L2 = i;
			}
			int target_index = (hit+1) ? hit : LR_L2;

			this->L2[index][target_index].tag = tag;
			this->L2[index][target_index].lru_position = 0;
			this->L2[index][target_index].valid = true;
		}
	}
}

std::tuple<double, double, double> cache::get_Stats() {
	return {this->myStat.missL1/this->myStat.accL1, this->myStat.missVic/this->myStat.accVic, this->myStat.missL2/this->myStat.accL2};
}
