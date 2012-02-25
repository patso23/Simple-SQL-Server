#ifndef BPOOL_H
#define BPOOL_H

#include <string>
#include <list>
#include "parser.h"

using namespace std;

class bChunk
{
public:
	int buffer[12] ;
	int bN;
        int tab;
        int blockID ;
        char tabName[3] ;
        bool isEmpty ;
        bool isDirty ;
};

class bPool
{
private:
	list<bChunk> bPool1;
public:	bPool();
	~bPool();
	void bpResize(int bufferPoolSize);
	void lruBlock(list<bChunk>::iterator mr);
	bool insertRecord(string tablename, num_list_t* record,list<bChunk>::iterator bom, int col );
		//write the record to the bom start
	void rBlock(list<bChunk>::iterator aim, string t, int ta, int bID);//read block from sM
	list<bChunk>::iterator GetMru();
	list<bChunk>::iterator GetLru();
	list<bChunk>::iterator GetBegin();
	list<bChunk>::iterator GetEnd();
	int GetTab(list<bChunk>::iterator i);
	int GetBlockId(list<bChunk>::iterator i);
	string GetTabName(list<bChunk>::iterator i);
	bool isEmpty(list<bChunk>::iterator i);
	bool isDirty(list<bChunk>::iterator i);
};
  


#endif
