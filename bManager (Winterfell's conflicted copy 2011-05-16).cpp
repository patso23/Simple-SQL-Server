#include <iostream>
#include <iomanip>
#include <list>
#include <string>
#include "stdio.h"
#include "bPool.h"
#include "parser.h"
#include "bManager.h"
#include "sManager.h"

using namespace std;


	bManager::bManager()
	{
	}
	bManager::~bManager()
	{
		bpt.~bPool();	
	}
	void bManager::bManagerSet(int bPS)
	{
		bpt.bpResize(bPS);
		hit=0;
		total=0;
	}

	bool bManager::insert(int t, string tn, num_list_t* record, int colums)
	{
                list<bChunk>::iterator mrut=bpt.GetMru();
	        list<bChunk>::iterator lrut=bpt.GetLru();
		total++;
		Block sMan;
		int bI = sMan.getmaxBlock(tn);//get max block id belongs to t
		
		if (bI==0)//empty tab
		{
			int b=0;int bn=bpt.GetBegin()->bN;
			int test = 0;
			for (list<bChunk>::iterator mp=bpt.GetBegin(); mp!= bpt.GetEnd();mp++)
			{
                                if (bn==mp->bN && test==1) break;
				else 
                                {test++; 
				if(mp->tab==t)
				{
					if(bpt.insertRecord(tn,record,mp,colums))
					{

						bpt.lruBlock(mp);
						hit++;
						return 1;
					}
					else if(b<mp->blockID)
						{b=mp->blockID;
                                        list<bChunk>::iterator it=mp;
                                        it++; bpt.lruBlock(mp);
                                        
                                        mp=it;
                                        mp--; }
				}
			}
}
			for ( list<bChunk>::iterator emp=bpt.GetBegin(); emp!= bpt.GetEnd();emp++)//check if there is a empty buffer
			{
			   if(emp->isEmpty)
			   {
				   bpt.insertRecord(tn,record,emp,colums);
				   emp->tab = t;
				   emp->blockID = b+1;
				   hit++;
				   bpt.lruBlock(emp);
				   return 1;
			   }
			}

			flushBuffer(lrut);
			bpt.insertRecord(tn,record,lrut,colums);
			hit++;
			lrut->tab = t;
			lrut->blockID = b+1;
			bpt.lruBlock(lrut);
			return 1;

		}
		else//not empty table
		{
			int b=0;int bn=bpt.GetBegin()->bN;
			int test = 0;
			for (list<bChunk>::iterator mp=bpt.GetBegin(); mp!= bpt.GetEnd();mp++)
			{
                               if (bn==mp->bN && test==1) break;
				else {test++;                                
				if(mp->tab==t)
				{
					if(bpt.insertRecord(tn,record,mp,colums))
					{
						bpt.lruBlock(mp);
						hit++;
						return 1;
					}
					else
                                        { 
                                                 
                                                if(b<mp->blockID)
						{b=mp->blockID;
                                                list<bChunk>::iterator it=mp;
                                                it++;
                                                  bpt.lruBlock(mp);
                                        
                                       
                                                  mp=it;
                                             mp--; }
                                                 
                                                
                                        }
                                        
                                      }  

				}
			}
			if(b>=bI)
			{
				for ( list<bChunk>::iterator emp=bpt.GetBegin(); emp!= bpt.GetEnd();emp++)//check if there is a empty buffer
				{
				   if(emp->isEmpty)
				   {
					   bpt.insertRecord(tn,record,emp,colums);
					   emp->tab = t;
					   emp->blockID = b+1;
					   hit++;
					   bpt.lruBlock(emp);
					   return 1;
				   }
				}
				
				flushBuffer(lrut);
				bpt.insertRecord(tn,record,lrut,colums);
				hit++;
				lrut->tab = t;
				lrut->blockID = b+1;
				bpt.lruBlock(lrut);
				return 1;
			}
			else//b<bI
			{
				for ( list<bChunk>::iterator emp=bpt.GetBegin(); emp!= bpt.GetEnd();emp++)//check if there is a empty buffer
				{
				    if(emp->isEmpty)
				    {
				    	bpt.rBlock(lrut,tn,t,bI);;
				    	bpt.lruBlock(lrut);
					mrut=bpt.GetMru();
				    	if(bpt.insertRecord(tn,record,mrut,colums))//if can add record
				    	{
				    	   hit++;
				    	   return 1;
				    	}
				    	else//find a place to insert the record
				    	{
				    		for ( list<bChunk>::iterator emp=bpt.GetBegin(); emp!= bpt.GetEnd();emp++)//check if there is a empty buffer
				    		{

				    		    if(emp->isEmpty)
				    		    {
				    		    	bpt.insertRecord(tn,record,emp,colums);
				    				emp->tab = t;
				    				emp->blockID = bI+1;
				    				hit++;
				    				bpt.lruBlock(emp);
				    				return 1;
				    			}
				    		}
				    		flushBuffer(lrut);
				    		bpt.insertRecord(tn,record,lrut,colums);
				    		hit++;
				    		lrut->tab = t;
				    		lrut->blockID = bI+1;
				    		bpt.lruBlock(lrut);
				    		return 1;
				    	}
					}
				}
		        flushBuffer(lrut);
		        bpt.rBlock(lrut,tn,t,bI);//if not, read last block from sM
			bpt.lruBlock(lrut);
			mrut=bpt.GetMru();
		        if(bpt.insertRecord(tn,record,mrut,colums))//if can add record
		        {
		    	   hit++;
		    	   return 1;
		        }
		        else
		        {
		    	    for ( list<bChunk>::iterator emp=bpt.GetBegin(); emp!= bpt.GetEnd();emp++)//check if there is a empty buffer
			        {
				       if(emp->isEmpty)
				       {
					       bpt.insertRecord(tn,record,emp,colums);
					       emp->tab = t;
					       emp->blockID = bI+1;
					       hit++;
					       bpt.lruBlock(emp);
					       return 1;
				       }
				    }
		    	    flushBuffer(lrut);
		    	    bpt.insertRecord(tn,record,lrut,colums);
		    	    hit++;
		    	    lrut->tab = t;
		    	    lrut->blockID = bI+1;
		    	    bpt.lruBlock(lrut);
		    	    return 1;
			    }
		    }
		}
	}



	int * bManager::getBlock(int ta, string tn, int blID)
	{
		
		for (list<bChunk>::iterator it = bpt.GetBegin();it != bpt.GetEnd();it++)
		{
			if(it->tab==ta && it->blockID==blID)// if find it at buffer
			{
				total++;
				hit++;
				bpt.lruBlock(it);                 //adjust lru and mru
				return it->buffer;
			}
		}
		                                 //else read from sManager
		Block sMan;
	    int bI = sMan.getmaxBlock(tn);
	    if(blID<=bI)
	    {
	    	for (list<bChunk>::iterator it = bpt.GetBegin();it != bpt.GetEnd();it++)
	    	{
	    		if(it->isEmpty)
	    		{
	    			bpt.rBlock(it, tn, ta, blID);
	    			bpt.lruBlock(it);
	    			total++;
	    			return it->buffer;
	    		}
	    	}
		list<bChunk>::iterator lrut=bpt.GetLru();
	    	flushBuffer(lrut);
	    	bpt.rBlock(lrut, tn, ta, blID);
	    	list<bChunk>::iterator temp = lrut;
	    	bpt.lruBlock(lrut);
	    	total++;
	    	return temp->buffer;
	    
	    }
	    else
            {
		return NULL;
	    }
	}

    void bManager::flushBuffer(list<bChunk>::iterator b)
    {
    	//check if it is dirty
	    Block sMan;
    	if (b->isDirty)//if aim is dirty
    	{
    		sMan.writeBlock(b->tabName, b->blockID, b->buffer);
    		b->isDirty = 0;
    	}
	    for(int i = 0; i<12; i++)
	    {
		    b->buffer[i]=-1;}
		    b->blockID= 0;
		    b->isEmpty= 1;
		    b->isDirty= 0;
		    b->tab = 0;
	    
    }

    void bManager::flushTab(int ta)
    {
    	for( list<bChunk>::iterator it = bpt.GetBegin();it != bpt.GetEnd();it++)
    	{
    		if(it->tab==ta)
    			flushBuffer(it);
    	}
    }

    void bManager::printBuffer()
    {
    	cout <<"Buffer#  file  block#"<< endl;
        cout <<"least recently used"<<endl;
    	for(list<bChunk>::iterator it = bpt.GetBegin();it != bpt.GetEnd();it++)
    	{
    		if(it->isEmpty)
    		{
    			cout<<setw(7)<<it->bN<<setw(7)<<"Empty"<<endl;//ask for tab name instead of tab ID
    		}
    		else
    			cout<<setw(7)<<it->bN<<setw(7)<<it->tabName<<setw(7)<<it->blockID<<endl;//ask for tab name instead of tab ID
    	}
        cout<<"most recently used:"<<endl;
    }  

    void bManager::printHitRate()
    {

	if(total!=0)
	{
    		cout << "hit rate is "<<hit<<"/"<<total<<" = "<<(float)hit/total<<endl;
    	}

	else
	{
		cout << "hit rate is 0/0" <<endl;
	}
    }

    void bManager::commit()
    {
	    Block sMan;
    	    for(list<bChunk>::iterator it = bpt.GetBegin();it != bpt.GetEnd();it++)
	    {
    		if (it->isDirty)
            	{
    			sMan.writeBlock(it->tabName, it->blockID, it->buffer);
			    it->isDirty=0;
		}
	    }
    }
int bManager::getMax(int tid, string tn )
{
	int i=0;
	Block sMan;
	for(list<bChunk>::iterator it = bpt.GetBegin();it!=bpt.GetEnd();it++)
	{
		if(it->tab==tid)
		{
			if(it->blockID>i)
				i=it->blockID;
		}
	}
        int mb = sMan.getmaxBlock(tn);
	if (mb>i) i = mb;
	return i;
}
