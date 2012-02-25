#ifndef BMANAGER_H
#define BMANAGER_H

#include "bPool.h"
using namespace std;

class bManager
{
private:
	int hit;
	int total;
	bPool bpt;
	//list<bChunk>::iterator mrut;
	//list<bChunk>::iterator lrut;
public:	bManager();
	~bManager();
	void bManagerSet(int bPS);

	bool insert(int t,string tn, num_list_t* record, int colums);

	int *getBlock(int ta, string tn, int blID);

    void flushBuffer(list<bChunk>::iterator b);

    void flushTab(int ta);

    void printBuffer();

    void printHitRate();

    void commit();
    
    int getMax(int tid, string tn);

};


#endif
