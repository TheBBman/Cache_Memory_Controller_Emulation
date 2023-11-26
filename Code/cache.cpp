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

	// Initialization
	this->myStat.missL1 = 0;
	this->myStat.missVic = 0;
	this->myStat.missL2 = 0;
	this->myStat.accL1 = 0;
	this->myStat.accVic = 0;
	this->myStat.accL2 = 0;
}

void cache::controller(bool MemR, int addr)	
{
	if (search_L1(addr, MemR)) {	// If addr found in L1, just update LRU & stats and return
		return;
	} else {						// Otherwise, will also need to insert addr into L1 for loads
		search_victim(addr, MemR);
		search_L2(addr, MemR);
		if (MemR)
			insert_L1(addr);
	}	
}

// Search for addr in L1, MemR determines whether or not to update L1 stats
bool cache::search_L1(int addr, int MemR)	
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

// Search for addr in victim, MemR determines whether to update stats
// Delete addr if found, insert into L1 later
bool cache::search_victim(int addr, int MemR) 
{
	// Fully associative tag
	int tag = addr/4;
	int hit = -1;
	if (MemR) 
		this->myStat.accVic++;
    for (int i = 0; i < VICTIM_SIZE; i++) {
		if (this->victim[i].tag == tag && this->victim[i].valid) {
			if (MemR)	
				this->victim[i].valid = false;	// Delete entry for addr if found (load only)
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
		// Update LRU for addr (store only)
		if (!MemR)
			this->victim[hit].lru_position = 0;
		return true;
	}
	// Cache Miss
	if (MemR) 
		this->myStat.missVic++;
	return false;
}

// Search for addr in L2, MemR determines whether to update stats
// Delete addr if found, insert into L1 later
bool cache::search_L2(int addr, int MemR)	//Done?
{
	// Set associative cache
    int tag = addr/64;
	int index = (addr-64*tag)/4;
	int hit = -1;
	if (MemR)
		this->myStat.accL2++;
	for (int i = 0; i < 8; i++) {
		if (this->L2[index][i].tag == tag && this->L2[index][i].valid) {
			if (MemR)	
				this->L2[index][i].valid = false;	// Delete entry for addr if found (load only)
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
		// Update LRU for addr (store only)
		if (!MemR)
			this->L2[index][hit].lru_position = 0;
		return true;
	}
	// Cache Miss
	if (MemR)
		this->myStat.missL2++;
	return false;
}

// Insert addr into L1 and evict downwards
void cache::insert_L1(int addr)
{
	int tag = addr/64;
	int index = (addr-64*tag)/4;
	// If there is already valid entry in index then it needs to be kicked from L1
	// Reconstruct fully associative tag using stored tag and current index for kicked entry
	int kicked_L1_tag = this->L1[index].valid ? (this->L1[index].tag*16 + index) : -1;
	this->L1[index].tag = tag;
	this->L1[index].valid = true;
	// If someone was kicked from L1, insert them to victim
	if (kicked_L1_tag > -1) {
		int kicked_victim_tag = -1;
		int hit = -1;
		int LR_victim = 0;
		for (int i = 0; i < 4; i++) {
			// Update LRU for valid entries
			if (this->victim[i].valid)
				this->victim[i].lru_position++;
			else
				hit = i;
			// Keep track of least recently used entry
			if (this->victim[i].lru_position > this->victim[LR_victim].lru_position)
				LR_victim = i;
		}
		// If empty spot found, insert kicked_L1 and return
		if (hit > -1) {
			this->victim[hit].tag = kicked_L1_tag;
			this->victim[hit].lru_position = 0;
			this->victim[hit].valid = true;
			return;
		} 
		// No empty spot found, kick least recent to L2
		else {
			kicked_victim_tag = this->victim[LR_victim].tag;
			this->victim[LR_victim].tag = kicked_L1_tag;
			this->victim[LR_victim].lru_position = 0;
		}
		// If someone was kicked from victim, insert to L2
		if (kicked_victim_tag > -1) {
			int tag = kicked_victim_tag/16;
			int index = kicked_victim_tag - 16*tag;
			int hit = -1;
			int LR_L2 = 0;
			for (int i = 0; i < 8; i++) {
				// Update LRU for valid entries
				if (this->L2[index][i].valid)
					this->L2[index][i].lru_position++;
				else 
					hit = i;
				// Keep track of least recently used entry
				if (this->L2[index][i].lru_position > this->L2[index][LR_L2].lru_position)
					LR_L2 = i;
			}
			// Insert location is either empty spot or least recently used entry
			int target_index = (hit > -1) ? hit : LR_L2;

			// Insert and return, simply discard kicked entry
			this->L2[index][target_index].tag = tag;
			this->L2[index][target_index].lru_position = 0;
			this->L2[index][target_index].valid = true;
		}
	}
}

// Return stats for computing output
std::tuple<double, double, double> cache::get_Stats() {
	return {this->myStat.missL1/this->myStat.accL1, this->myStat.missVic/this->myStat.accVic, this->myStat.missL2/this->myStat.accL2};
}
