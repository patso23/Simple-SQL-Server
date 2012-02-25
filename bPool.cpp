#include <iostream>
#include <list>
#include <string>
#include "stdio.h"
#include "string.h"
#include "bPool.h"
#include "sManager.h"

using namespace std;

	Block sMan;

	bPool::bPool()//generate buffer pool
	{
	}
	bPool::~bPool()
	{
		bPool1.clear();
	}
	void bPool::bpResize(int bufferPoolSize)
	{
		bPool1.resize(bufferPoolSize);
		int i=0;
		for (list<bChunk>::iterator it = bPool1.begin(); it!= bPool1.end();it++)
		{
			it->bN = i;
			i++;
			it->isDirty = 0;
			it->isEmpty = 1;
			it->blockID = 0;
			it->tab = 0;
			for(int i = 0; i<12; i++)
			{
			    it->buffer[i]=-1;
			}
		}
	}

    bool bPool::insertRecord(string tn, num_list_t* record,list<bChunk>::iterator bom, int col )
    {
		int i;
   		for ( i = 0;i<12;i++)
   		{
   			if((bom->buffer[i])==-1)
   				break;
   		}
   		if (col>(12-i))//not enough space
   			return false;
   		else
   		{
   			for(int j=i;j<i+col;j++)
   			{
   				bom->buffer[j]=record->num;
   				record = record->next;
   			}
   			bom->isDirty = 1;
			bom->isEmpty = 0;
			memset(bom->tabName, '\0', 3 );
			tn.copy(bom->tabName, 3);			
  			return true;
   		}
   	}

    void bPool::rBlock(list<bChunk>::iterator aim, string tn, int ta, int bID)//read block from sM
   	{
   		int* buf=sMan.readBlock(tn, bID);
		  		
		for(int i= 0;i<12;i++)
   		{
			aim->buffer[i]=*buf;
			
   			buf++;
   		}
		
   	    aim->blockID = bID;
   		aim->tab = ta;
   		aim->isEmpty = 0;
   		memset(aim->tabName, '\0', 3 );
   		tn.copy(aim->tabName, 3);
   	}

    void bPool::lruBlock(list<bChunk>::iterator mr)
    {
	list<bChunk>::iterator mru=bPool1.end();
	bPool1.splice(mru, bPool1, mr);
    }

    list<bChunk>::iterator bPool::GetMru()
    {
	list<bChunk>::iterator i=bPool1.end();
	i--;
	return i;
    }

    list<bChunk>::iterator bPool::GetLru()
    {
	return bPool1.begin();
    }

    list<bChunk>::iterator bPool::GetBegin()
    {
	return bPool1.begin();
    }

    list<bChunk>::iterator bPool::GetEnd()
    {
	return bPool1.end();
    }

    int bPool::GetTab(list<bChunk>::iterator i)
    {
	return i->tab;
    }

    int bPool::GetBlockId(list<bChunk>::iterator i)
    {
	return i->blockID;
    }

    string bPool::GetTabName(list<bChunk>::iterator i)
    {
	return i->tabName;
    }

    bool bPool::isEmpty(list<bChunk>::iterator i)
    {
	return i->isEmpty;
    }
    
    bool bPool::isDirty(list<bChunk>::iterator i)
    {
	return i->isDirty;
    }

