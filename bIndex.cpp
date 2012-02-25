//bindex.cpp

#include <iostream>
#include <list>
#include <vector>
#include <fstream>
#include <deque>
#include <vector>
#include <stack>
#include <dirent.h>
#include <boost/algorithm/string.hpp>
#include "bIndex.h"
#include "table.h"
#include "node.h"
#include "sManager.h"
#include "bManager.h"
#include "catalog.h"


using namespace std;


bIndex::bIndex()
{
	debug = false;
	loaded = false;
	attr = -1;
}


bIndex::~bIndex()
{
	indexVector.clear();
}

bool bIndex::getDups()
{
	return indexVector[0].getInfo(6);
}

bool bIndex::createIndex(int attr, int dups, char* tab, char* indexName, bManager& bMan)
{
	loaded = true;
	setTableName(tab);
	setIndexName(indexName);
	setAttr(attr);
	indexVector.clear();
	Catalog myCatalog;
  	myCatalog.ReadCatalog();
  	 //set up Storage Manager
   	//pass buffersize to buffer manager
   	//string s = strdup(argv[1]);
	Table tmp = myCatalog.GetTable(tab);
	int id = tmp.GetTableID();
	string s = tmp.GetTableName();			
	int arrSize = bMan.getMax(id, s)*12;
	node header(4);
	header.setDups(dups);
	//cout<<"array size "<<arrSize<<"\n";
	int array[arrSize][2];
	arrSize = select_create_index(tab, attr, array, bMan);
	
	indexVector.push_back(header);
    if(dups==1)
    {
      	for(int i=0;i<arrSize;i++)
       	{
		//cout<<"array "<<array[i][0]<<"\n";
		if (array[i][0]!=-1)     		
		insertDups(array[i][0],array[i][1]);
		else return true;     	    
       	}
	/*for(int j=0;j<=indexVector[0].getNum();j++)	
	{	for(int s=0;s<12;s++)	
		{cout<<indexVector[j].getInfo(s)<<" ";
		}
		cout<<"\n";
	}*/
       	return true;
    }
    else
    {
       	for(int i=0;i<arrSize;i++)
       	{
		//cout<<"array "<<array[i][0]<<"\n";
		if (array[i][0]!=-1)
       		{
			
			if(!insertNoDups(array[i][0],array[i][1]))
       	    			return false;
		}
		else break;
	}
        /*for(int j=0;j<=indexVector[0].getNum();j++)	
	{	for(int s=0;s<12;s++)	
		{cout<<indexVector[j].getInfo(s)<<" ";
		}
		cout<<"\n";
	}*/
       	return true;
    }
}

bool bIndex::insert(int key, int ptr)
{	
	/*for(int j=0;j<=indexVector[0].getNum();j++)	
	{	for(int s=0;s<12;s++)	
		{cout<<indexVector[j].getInfo(s)<<" ";}
		cout<<"\n";
	}*/
	//cout<<"tablename :"<<tablename<<" indexname: "<<indexname<<" attr: "<<attr<<" loaded: "<<loaded<<"\n";
	//cout<<"\n";
	if(indexVector[0].getDups()==0)
		insertNoDups(key,ptr);
	else
		insertDups(key,ptr);
	/*for(int j=0;j<=indexVector[0].getNum();j++)	
	{	for(int s=0;s<12;s++)	
		{cout<<indexVector[j].getInfo(s)<<" ";}
		cout<<"\n";
	}*/
}

bool bIndex::testInsert(int key, int ptr )
{
	//cout<<"it is test insert\n";
	if(indexVector[0].getDups()==1)
		return true;
	if(indexVector[0].getNum()==0)
		return true;
	int n = indexVector[0].getRoot();
	//cout<<"it is test insert 1\n";
	while(indexVector[n].getType() != 3) //type 2 is leaf node
	{
		n=indexVector[n].getPtr(key);
	}
	bool found = indexVector[n].search(key);
	//cout<<"it is test insert 2\n";
	if(found == true)
	{
		return false;
	}
	else return true;

}

bool bIndex::insertNoDups(int key,int ptr)//nodups
{
	/*if(debug)
	{
		cout<<"Inserting value "<<key<<" into the index "<<indexname<<" of the table "<<tablename<<"\n";
	}*/
	//inserting a record with key value K in a b+ tree of order p
	if(indexVector[0].getNum()==0)//empty update numblock
	{
		node root(3);
		root.setBlockNum(1);
		root.insert(key,ptr);
		indexVector.push_back(root);
		indexVector[0].setRoot(root);
		indexVector[0].addNum();
		/*if(debug)
		{
			cout<<"Node: 1 Type: Leaf Function: insertNoDups Value: "<<key<<"\n";
		}*/
		return true;
	}
	else
	{
		int n=indexVector[0].getRoot();//n <- block containing root node of tree
	    stack<node> S;//set stack S to empty;
	    while (indexVector[n].getType()!=3)//n is not a leaf node of the b+ tree
	    {
                
	    	S.push(indexVector[n]);
			//stack S holds parent nodes that are needed in case of split
		    //q <- number of tree pointers in node n;
		    n=indexVector[n].getPtr(key);/*if key <= n.K1 //n.Ki refers to the ith search vield value in node n
			then n <- n.P1  //n.Pi refers to the ith tree pointer in node n
		else if K > n.Kq-1
			then n <-n.Pq
		else begin
			search node n for an entry i such that n.Ki-1 < K <= n.Ki;
			n <- n.Pi
		        end;
		read block n*/
	    }
	    bool found = indexVector[n].search(key);//search block n for entry (Ki, Pri) with K = Ki; //search leaf node n
		//cout<<"found?"<<found<<"\n";
	    if (found)
	    {
	            cout<<"Error: Attempted to add duplicate value to no duplicates index\n";
		    return false;//then record already in file cannot insert   //if disallowing duplicates
	    }
	    else //insert entry in B+ tree to point to record
	    {
	    	//create entry (K, Pr) where Pr points to the new record;
			if(indexVector[n].getNumKey()<4) //leaf node n is not full
			{
				indexVector[n].insert(key,ptr);// entry (K, Pr) in correct position in leaf node n
				/*if(debug)
				{
					cout<<"Node: "<<n<<" Type: Leaf Function: insertNoDups Value: "<<key<<"\n";
				}*/
			}
			else//leaf node n is full with Pleaf record pointers; is split
			{
				node temp(6);	//copy n to temp temp is an oversize leaf node to hold extra entries
				temp.oversizeCopy(indexVector[n]);
				temp.oversizeInsert(key,ptr);
				//insert entry (K, Pr) in temp in correct position;
				//temp now hold pleaf+1 entries of the form (Ki, Pri)
				node newNode(3);	//new <- a new empty leaf node for the tree;  new.Pnext <- n.Pnext;
				newNode.setNext(indexVector[n].getNext());
				//j <- ceil((pleaf + 1)/2);3
				indexVector[n].clear();
				indexVector[n].copy(temp);			//	n <- first j entries in temp (up to entry (Kj, Prj));
				newNode.tail(temp);//				new <- remaining entries in temp;  K <- Kj;
				/*if(debug)
				{
					cout<<"Node: "<<n<<" Type: Leaf Function: insertNoDups Value: "<<key<<"\n";
					cout<<"Node: "<<n<<" Type: Leaf split"<<"\n";
				}*/
				key = newNode.getKey(1);
				newNode.setBlockNum(indexVector.size());
				indexVector.push_back(newNode);
				indexVector[n].setNext(newNode.getBlockNum());
				indexVector[0].addNum();
				node t = indexVector[n];
				
				//now we must move (K, new) and insert in parent internal node, however if parent is full, split may propagate
				bool finished = false;
				while(finished==false)
				{
					if (S.empty())
					{ //no parent node; new root node is created for the tree
						node parent(1);
								//root <- a new empty internal node for the tree;
						parent.insert(indexVector[n],key,newNode);		//root <- <n, K, new>;
						parent.setBlockNum(indexVector.size());						
						indexVector.push_back(parent);
						indexVector[0].addNum();
						indexVector[0].setRoot(parent);
						/*if(debug)
						{
							cout<<"Node: "<<parent.getBlockNum()<<" Type: Root Function: insertNoDups Value: "<<key<<"\n";
						}*/
						finished = true;

					}
					else
					{
                                                t = S.top();
						S.pop();
						n = t.getInfo(0);
						if (t.getNumKey()<4)//internal node n is not full
						{
							//parent node not full, no split
							indexVector[n].insert(key,newNode.getInfo(0));//insert (K, new) in correct position in internal node n;
							/*if(debug)
							{	
								string ty=(t.getInfo(1)==1)?"Root":"Internal";
								cout<<"Node: "<<n<<" Type: "<<ty<<" Function: insertNoDups Value: "<<key<<"\n";
							}*/
							finished = true;
						}
						else    //internal node n is full with p tree pointers; overflow condition, node is split
						{
							node tem(6);//copy n to temp //temp is oversize internal node
							tem.oversizeCopy(indexVector[n]);
							tem.oversizeInsert(key,newNode.getInfo(0));
							//insert (K, new) in temp in correct position;
							//temp now has p+1 tree pointers
							//newNode = new leafBlockNoDups();//new <- a new empty internal node for the tree;
							//j <- floor((p + 1)/2);
							indexVector[n].clear();//n <- entries up to tree pointer Pj in temp;
							indexVector[n].copy(tem);
							indexVector[n].setType(2);
							node ne(2);
							//n contains <P1, K1, P2, K2, ... , Pj-1, Kj-1, Pj>
							ne.tail(tem);//new <- entries from tree pointer Pj+1 in temp;
							//new contains <Pj+1, Kj+1, ... , Kp-1, Pp, Kp, Pp+1>
							/*if(debug)
							{
								string ty = (t.getInfo(1)==1)?"Root":"Internal";
								cout<<"Node: "<<n<<" Type: "<<ty<<" Function: insertNoDups Value: "<<key<<"\n";
								cout<<"Node: "<<n<<" Type: "<<ty<<"split"<<"\n";
							}*/
							key = tem.oversizeGetKey(3);//K < Kj
							//now we must move (K, new) and insert in parent internal node
							ne.setBlockNum(indexVector.size());							
							indexVector.push_back(ne);
							indexVector[0].addNum();
							newNode = ne;
							
						}
					}
				}
			}
			return true;			
	    }
	}
}

bool bIndex::insertDups(int key,int ptr)//dups
{
	
	/*if(debug)
	{
		cout<<"Inserting value "<<key<<" into the index "<<indexname<<" of the table "<<tablename<<"\n";
	}*/
	//inserting a record with key value K in a b+ tree of order p
	if(indexVector[0].getNum()==0)//empty update numblock
	{
		node root(3);
		root.setBlockNum(1);
		indexVector.push_back(root);
		indexVector[0].addNum();
		indexVector[0].setRoot(root);
		node pb(5);
		pb.addPtr(ptr);
		pb.setBlockNum(indexVector.size());
		indexVector.push_back(pb);
		indexVector[0].addNum();		
		indexVector[1].insert(key,pb.getInfo(0));
		/*if(debug)
		{
			cout<<"Node: 1 Type: Root Function: insertDups Value: "<<key<<"\n";
			cout<<"Node: 2 Type: pointer bucket insert 1\n";		
		}*/
		return true;
	}
	else
	{
		int n=indexVector[0].getRoot();//n <- block containing root node of tree
	    stack<node> S;//set stack S to empty;
	    while (indexVector[n].getType()!=3)//n is not a leaf node of the b+ tree
	    {
	    	S.push(indexVector[n]);
			//stack S holds parent nodes that are needed in case of split
		    //q <- number of tree pointers in node n;
		    n=indexVector[n].getPtr(key);/*if key <= n.K1 //n.Ki refers to the ith search vield value in node n
			then n <- n.P1  //n.Pi refers to the ith tree pointer in node n
		else if K > n.Kq-1
			then n <-n.Pq
		else begin
			search node n for an entry i such that n.Ki-1 < K <= n.Ki;
			n <- n.Pi
		        end;
		read block n*/
	    }
	    bool found = indexVector[n].search(key);//search block n for entry (Ki, Pri) with K = Ki; //search leaf node n
	    if (found)//insert into pointer bucks
	    {
		    int te = indexVector[n].getPtr(key);
		    while (indexVector[te].getNext()!=-1)
		    {
		    	for(int i=2;i<10;i++)
		    	{
		    		if(indexVector[te].getInfo(i)==ptr)
		    			return true;
		    	}
		    	te = indexVector[te].getNext();
		    }
		    if(indexVector[te].getNumPtr()!=9)
		    {
			for(int i=2;i<indexVector[te].getNumPtr()+2;i++)
			{
				if ( indexVector[te].getInfo(i)==ptr)
					return true;
			}
		    	indexVector[te].addPtr(ptr);
			/*if(debug)
		    	{
				cout<<"Node: "<<n<<" Type: Leaf Function: insertDups Value: "<<key<<"\n";
				cout<<"Node: "<<te<<" Type: pointer bucket insert "<<ptr<<"\n";		
		    	}*/
		    }
		    else
		    {
		    	node pb(5);
		    	pb.addPtr(ptr);
			pb.setBlockNum(indexVector.size());
		    	indexVector.push_back(pb);
		    	indexVector[0].addNum();
		    	indexVector[te].setNext(pb.getBlockNum());
			/*if(debug)
		    	{
				cout<<"Node: "<<n<<" Type: Leaf Function: insertDups Value: "<<key<<"\n";
				cout<<"Node: "<<pb.getBlockNum()<<" Type: pointer bucket insert "<<ptr<<"\n";		
		    	}*/
		    }
		    
		    return true;   //if disallowing duplicates
	    }
	    else //insert entry in B+ tree to point to record
	    {
	    	//create entry (K, Pr) where Pr points to the new record;
			if(indexVector[n].getNumKey()<4) //leaf node n is not full
			{
				node pb(5);
				pb.addPtr(ptr);
				pb.setBlockNum(indexVector.size());
				indexVector.push_back(pb);
				indexVector[0].addNum();
				/*if(debug)
		    		{
					cout<<"Node: "<<n<<" Type: Leaf Function: insertDups Value: "<<key<<"\n";
					cout<<"Node: "<<pb.getBlockNum()<<" Type: pointer bucket insert "<<ptr<<"\n";		
		    		}*/
				
                		indexVector[n].insert(key,pb.getInfo(0));
				// entry (K, Pr) in correct position in leaf node n
			}
			else//leaf node n is full with Pleaf record pointers; is split
			{
				node pb(5);
				pb.addPtr(ptr);
				pb.setBlockNum(indexVector.size());
				indexVector.push_back(pb);
				indexVector[0].addNum();
				node temp(6);	//copy n to temp temp is an oversize leaf node to hold extra entries
				temp.oversizeCopy(indexVector[n]);
				temp.oversizeInsert(key,pb.getInfo(0));
				//insert entry (K, Pr) in temp in correct position;
				//temp now hold pleaf+1 entries of the form (Ki, Pri)
				node newNode(3);	//new <- a new empty leaf node for the tree;  new.Pnext <- n.Pnext;
				newNode.setNext(indexVector[n].getNext());
				//j <- ceil((pleaf + 1)/2);3
				indexVector[n].clear();
				indexVector[n].copy(temp);			//	n <- first j entries in temp (up to entry (Kj, Prj));
				newNode.tail(temp);//				new <- remaining entries in temp;  K <- Kj;
				/*if(debug)
		    		{
					cout<<"Node: "<<n<<" Type: Leaf Function: insertDups Value: "<<key<<"\n";
					cout<<"Node: "<<n<<" Type: Leaf split\n";
					cout<<"Node: "<<pb.getBlockNum()<<" Type: pointer bucket insert "<<ptr<<"\n";		
		    		}*/
				key = newNode.getKey(1);
				newNode.setBlockNum(indexVector.size());
				indexVector.push_back(newNode);
				indexVector[n].setNext(newNode.getBlockNum());
				indexVector[0].addNum();
				node t = indexVector[n];
				
				//now we must move (K, new) and insert in parent internal node, however if parent is full, split may propagate
				bool finished = false;
				while(finished==false)
				{
					if (S.empty())
					{ //no parent node; new root node is created for the tree
						node parent(1);
								//root <- a new empty internal node for the tree;
						parent.insert(indexVector[n],key,newNode);		//root <- <n, K, new>;
						parent.setBlockNum(indexVector.size());
						indexVector.push_back(parent);
						indexVector[0].addNum();
						indexVector[0].setRoot(parent);
						/*if(debug)
		    				{
							cout<<"Node: "<<parent.getBlockNum()<<" Type: Root Function: insertDups Value: "<<key<<"\n";
		    				}*/
						finished = true;

					}
					else
					{
						t = S.top();
                                                S.pop();
						n = t.getInfo(0);
						if (t.getNumKey()<4)//internal node n is not full
						{
							//parent node not full, no split
							indexVector[n].insert(key,newNode.getInfo(0));//insert (K, new) in correct position in internal node n;
							/*if(debug)
		    					{
								string ty = (t.getInfo(1)==1)?"Root":"Internal";
								cout<<"Node: "<<n<<" Type: "<<ty<<" Function: insertDups Value: "<<key<<"\n";	
		    					}*/
							finished = true;
						}
						else    //internal node n is full with p tree pointers; overflow condition, node is split
						{
							node tem(6);//copy n to temp //temp is oversize internal node
							tem.oversizeCopy(indexVector[n]);
							tem.oversizeInsert(key,newNode.getInfo(0));
							//insert (K, new) in temp in correct position;
							//temp now has p+1 tree pointers
							//newNode = new leafBlockNoDups();//new <- a new empty internal node for the tree;
							//j <- floor((p + 1)/2);
							indexVector[n].clear();//n <- entries up to tree pointer Pj in temp;
							indexVector[n].copy(tem);
							indexVector[n].setType(2);
							node ne(2);
							//n contains <P1, K1, P2, K2, ... , Pj-1, Kj-1, Pj>
							ne.tail(tem);//new <- entries from tree pointer Pj+1 in temp;
							//new contains <Pj+1, Kj+1, ... , Kp-1, Pp, Kp, Pp+1>
							/*if(debug)
		    					{
								string ty = (t.getInfo(1)==1)?"Root":"Internal";
								cout<<"Node: "<<n<<" Type: "<<ty<<" Function: insertDups Value: "<<key<<"\n";
								cout<<"Node: "<<n<<" Type: "<<ty<<" split"<<"\n";
		    					}*/
							key = tem.oversizeGetKey(3);//K < Kj
							//now we must move (K, new) and insert in parent internal node
							ne.setBlockNum(indexVector.size());							
							indexVector.push_back(ne);
							indexVector[0].addNum();
							newNode = ne;
							
						}
					}
				}
			}
			return true;			
	    }
	}
}

void bIndex::debugOn()
{
	debug = true;
}


void bIndex::debugOff()
{
	debug = false;
}

int bIndex::lookupNoDups(int key)
{

	//all lowercase i's and q's are subscripts
	/*if(debug)
	{
		cout << "Starting find " << key << " on index " << getIndexName() << "of table " <<getTableName() <<"\n";
	}*/
	
	int n = indexVector[0].getRoot();
	//int q=0;
	//int i=0;

	//read block n;
	if(indexVector[0].getNum()==0)
		return -1;
	while(indexVector[n].getType() != 3) //type 2 is leaf node
	{
		n=indexVector[n].getPtr(key);
	}
	//n is leaf block node

	bool found = indexVector[n].search(key);

	if(found == true)
	{
		return indexVector[n].getPtr(key);
	}
	else	
	{
		return -1;
	}

	
}


void bIndex::lookupDups(int key, deque<int>& pointers)
{
	/*if(debug)
	{
		cout << "Starting find " << key << " on index " << getIndexName() << " of table " <<getTableName() <<"\n";
	}*/
		

	int n = indexVector[0].getRoot();
	if(indexVector[0].getNum()==0)
	{}
	else{	
	//read block n;
	while(indexVector[n].getType() != 3) //type 3 is leaf node
	{
		n=indexVector[n].getPtr(key);
	}
	//n is leaf block node

	bool found = indexVector[n].search(key);
	//cout<<"leaf: "<<n<<"\n";
	if(found == true)
	{

		int te = indexVector[n].getPtr(key);
		while (indexVector[te].getNext()!=-1)
		{
		    	for(int i=2;i<11;i++)
		    	{	//cout<<"indexVector[te].getInfo(i)"<<indexVector[te].getInfo(i)<<"\n";
		    		pointers.push_back(indexVector[te].getInfo(i));
		    	}
		    	te = indexVector[te].getNext();
		}
		int i=2;
		while(indexVector[te].getInfo(i)!=-1)
		{	//cout<<"indexVector[te].getInfo(i)"<<indexVector[te].getInfo(i)<<"\n";
			pointers.push_back(indexVector[te].getInfo(i));
			i++;
		}		
			
	}
	}
	
}

void bIndex::printIndex()
{
		
	int blocknum = indexVector[0].getRoot();
	bool dups = indexVector[0].getDups();

	deque<node> q;
	node curr = indexVector[blocknum];
	q.push_back(curr);  //load the deque with pointer to root
	if(indexVector[0].getNum()==0)
	{
		cout<<"Empty Index!\n";	
	}
	else{
	
	cout << "Header->" << blocknum << "\n";
 
	
	while(q.size() > 0)
	{	
		curr = q.front();  //pop next node off the front of the queue
		q.pop_front();

		
		cout << "Block " << curr.getBlockNum() << ": ";
		if(curr.getType()==1)
		{
			cout << "root\n";
		}
		else if (curr.getType()==2)
		{
			cout << "internal\n";
		}
		else if(curr.getType()==3)
		{
			cout << "leaf\n";
		}
		else if(curr.getType()==5)
		{
			cout << "pointer bucket\n";
		}
		
		
		if(curr.getType()!=5)
		{	
			cout << "keys: ";
			for(int i=1;  i <= curr.getNumKey(); i++)
			{
				cout << curr.getKey(i);
				//keys.push_back(curr->getKey(i+1));
				if(i < curr.getNumKey())
				{
					cout << ", ";
				}
			}
			cout << "\n";
		}

		//output root's pointers
		cout << "pointers: ";
		if(!dups)
		{
			for(int i =3; i < 11; i+=2)
			{	
				//cout << "# of pointer " << curr->getNumPtr() << "\n";		
				if(curr.getInfo(i) !=-1)
	   			{		
					cout << curr.getInfo(i) << " ";
					//cout<<"curr.getType: "<<curr.getType()<<"\n";
					if(curr.getType()!=3)
					{
						q.push_back(indexVector[curr.getInfo(i)]);
					}
	  			}
			}
			if(curr.getInfo(11)!=-1 && curr.getType()!=3 )
			{
				cout << curr.getInfo(11)<<" ";
				q.push_back(indexVector[curr.getInfo(11)]);
			}
		}
		else
		{
			if(curr.getType()!=5)
			{
				for(int i =3; i < 11; i+=2)
				{	
					//cout << "# of pointer " << curr->getNumPtr() << "\n";		
					if(curr.getInfo(i) !=-1)
	   				{		
						cout << curr.getInfo(i) << " ";
						//cout<<"curr.getType: "<<curr.getType()<<"\n";
						q.push_back(indexVector[curr.getInfo(i)]);
						
	  				}
				}
				if(curr.getInfo(11)!=-1 && curr.getType()!=3 )
				{
					cout << curr.getInfo(11)<<" ";
					q.push_back(indexVector[curr.getInfo(11)]);
				}
			}
			else
			{
				int te = curr.getBlockNum();
				while (indexVector[te].getNext()!=-1)
				{
			    		for(int i=2;i<11;i++)
			    		{
			    			cout << curr.getInfo(i) << " ";
			    		}
			    		te = indexVector[te].getNext();
				}
				int i=2;
				while(indexVector[te].getInfo(i)!=-1)
				{
					cout << curr.getInfo(i) << " ";
					i++;
				}
			}

		}
		cout << "\n\n";
	}
	}
}

bool bIndex::readIndex(char* tableName, int attrid)
{	
	//is index already in memory?
	char buffer[50];	
	char* t=buffer;
	strcpy(t, tablename.c_str());
	//cout<< "tablename " << t << " attrid " << attrid << "\n";
	if(strcmp(tableName, t)==0 && attr == attrid)
	{
		//cout << "already in memory!\n";
		return true;
	}
	else
	{
		bool found=false;
		char* indexFileName;
		char* split;
		//cout << " getting into readindex \n";	
		//does file exist?
		DIR *dp;
  		struct dirent *ep;     
  		dp = opendir ("./base/");
		
		
  		if (dp != NULL)
  		{
			//ep = readdir(dp); //skip . and ..
			//ep = readdir(dp);
			//char* tablePrefix;
			char buffer3[50];
			char* tablePrefix = buffer3;
			strcpy(buffer3,tableName);
			tablePrefix = strcat(tablePrefix, "_");
			int length = strlen(tablePrefix);
			char buffer4[50];
			char *filePrefix = buffer4;
			
			for (int i=0; i<length; i++) 
			{
		    		filePrefix[i] = 0;    // Initialize all elements to zero.
			}
    			while (ep = readdir (dp))
      			{
				strncpy(filePrefix, ep->d_name, length);
				filePrefix[length]='\0';

				//cout << "tablepre " << tablePrefix << " filepre "<<filePrefix << "\n";
				if(strcmp(tablePrefix, filePrefix)==0)
				{
					
					char buffer2[50];
					int tmp = atoi(&ep->d_name[strlen(ep->d_name)-1]);
					//cout << "tmp " << tmp << " attrid " << attrid << "\n";
					sprintf(buffer2, "%s", ep->d_name);
					split = strtok(ep->d_name, "_");
					int indexCount=0;
					//cout<<"just befor while:\n";
					while(indexCount < 1)
					{
						split = strtok(NULL, "_");
						indexCount++;
					}
					if(tmp == attrid)
					{	//cout << "final tmp " << tmp << " attrid " << attrid << "\n";
						//tableName[strlen(tableName) - 1] = '\0';
						//cout << "Index exists!\n";  	
						found = true;
						//read file												
						indexFileName = buffer2;
						setIndexName(split);
						//cout << "split " << split <<"\n";
						break;
					}
				}
			}
			//cout << "count " << count << "\n";
    			(void) closedir (dp);
  		}
  		else
   		{
			//tableName[strlen(tableName) - 1] = '\0';
			cout << "Couldn't open the /base directory\n";
		}
		if(found == false)
		{
			return false;  // no index for that attr
		}
		else
		{
			//read index file
			indexVector.clear();
			Block sMan;	
			string s = string(indexFileName);
			//read header
			int* array = sMan.readNode(s,1);
			node header(4);
			for(int i=0; i < 12; i++)
			{
				
				 header.setInfo(i, array[i]);
				 //cout << header.getInfo(i) << " ";
			}	
			indexVector.push_back(header);
			int blocks = indexVector[0].getNum();
			node curr(0);
			for(int i = 2; i <= blocks+1; i++)
			{
				array = sMan.readNode(s, i);
				for(int j=0; j<12; j++)
				{
					curr.setInfo(j, array[j]);
					//cout << curr.getInfo(j) << " ";
				}
				indexVector.push_back(curr);
			}
			//set up values of index;
			//debugOff();
			setTableName(tableName);
			setAttr(attrid);
			//cout << "here!\n";
			//printIndex();
			loaded = true;
			return true;	
		}
	}
}


void bIndex::writeIndex(char* tableName, char* indexName)
{
	int n = indexVector[0].getRoot();
	int array[12];	
        //load the deque with root
	Block sMan;
	char buffer[50];
	sprintf(buffer, "%s_%s_%d", tableName, indexName, attr);
	char* indexFileName = buffer; 
	string s = string(indexFileName);

	//cout << "numblocks " << indexVector[0].getNum() << "\n";
	for(int i=0; i<=indexVector[0].getNum(); i++)
	{	//
		//build array copy
      		for(int j=0; j<12; j++)
		{
			array[j] = indexVector[i].getInfo(j);
		}
		sMan.writeNode(s, i, array);  //not complete, need new writeNode function
	}
}

void bIndex::setTableName(char* table)
{
	tablename = string(table);
}

string bIndex::getTableName()
{

	return tablename;
}

void bIndex::setIndexName(char* s)
{
	indexname = string(s);
}

string bIndex::getIndexName()
{
	return indexname;
}

void bIndex::setAttr(int attrid)
{
	attr= attrid;
}

int bIndex::getAttr()
{
	return attr;
}

void bIndex::setLoaded(bool load)
{
	loaded = load;
}

bool bIndex::getLoaded()
{
	return loaded;
}

void bIndex::clear()
{
	loaded = false;
	tablename.clear();
	indexname.clear();
	attr = -1;
	indexVector.clear();
}

int bIndex::select_create_index(char* tableName, int attrid, int (*array)[2], bManager& bMan)
{			
   	//initialize and read catalog
  	 Catalog myCatalog;
  	 myCatalog.ReadCatalog();
  	 //set up Storage Manager
   	//pass buffersize to buffer manager
   	//string s = strdup(argv[1]);
   	Table tmp = myCatalog.GetTable(tableName);		
	int id = tmp.GetTableID();
	string s = tmp.GetTableName();	
	int *mem;
	int num =attrid;
	int siz = tmp.Size();
	int size = bMan.getMax(id,s);

	//number of fields is 1
	int j =0;

	//cout << "select for create index function!\n";				
	for(int blockid=1; blockid<=size; blockid++)
	{
		
		mem = bMan.getBlock(id, s, blockid);
		if(!mem) { break; }
		
		for(int i=0; i<12;  i=i+siz)
		{	
			if(mem[i+num]!=-1)
			{
				array[j][0] = mem[i+num];
				array[j][1] = blockid;
				j++;
				
		        }			
		}
										
	}
	return j;
 	

}

int bIndex::getAttrNum(char* index, char* tableName)
{
	DIR *dp;
  	struct dirent *ep;     
  	dp = opendir ("./base/");
	int count=0;
  	if (dp != NULL)
  	{
		//ep = readdir(dp); //skip . and ..
		//ep = readdir(dp);
		char buffer3[50];
		char* tablePrefix = buffer3;
		strcpy(buffer3,tableName);
		tablePrefix = strcat(tablePrefix, "_");
		unsigned int length = (unsigned int)strlen(tablePrefix);
		char *filePrefix = NULL;
		filePrefix = new char[length];
		for (int i=0; i<length; i++) 
		{
		    filePrefix[i] = 0;    // Initialize all elements to zero.
		}
     		while (ep = readdir (dp))
      		{
			strncpy(filePrefix, ep->d_name, length);
			filePrefix[length]='\0';
			//cout << "fileprefix " << filePrefix << " table " << tablePrefix << "\n";
			if(strcmp(tablePrefix, filePrefix)==0)
			{
				count++;
				int tmp = atoi(&ep->d_name[strlen(ep->d_name)-1]);
				char* split = strtok(ep->d_name, "_");
				int indexCount=0;
				while(indexCount < 1)
				{
					
					split = strtok(NULL, "_");
					indexCount++;
				}
				//cout << "split " << split << "\n";
				//cout << "split " << split << "\n";
				if(strcmp(split,index)==0)
				{
					//tableName[strlen(tableName) - 1] = '\0';
					return tmp;
				}
				//cout << "tmp " << tmp << "\n";
				
				
			}
			
		}
		//cout << "count " << count << "\n";
    		(void) closedir (dp);
  	}
  	else
   	{
		//tableName[strlen(tableName) - 1] = '\0';
		cout << "Couldn't open the /base directory\n";
	}
	if(count == 2)
	{
		//tableName[strlen(tableName) - 1] = '\0';
		cout << "Error:  max limit of indices (2) reached on this table\n";
		return 1;
	}
	//tableName[strlen(tableName) - 1] = '\0';
}

bool bIndex::indexExist(int column, char* tableName)
{


	string s= getIndexName();
	char* test = (char*)malloc( sizeof( char ) *(s.length() +1) );
	strcpy(test, s.c_str());
	
	string tab = getTableName();
	char* test3 = (char*)malloc( sizeof( char ) *(tab.length() +1) );
	strcpy(test3, tab.c_str());
	
	//Table temp = myCatalog.GetTable(tableName);
	int col;

	DIR *dp;
  	struct dirent *ep;     
  	dp = opendir ("./base/");
	int count=0;
  	if (dp != NULL)
  	{
		bool found=false;
		//ep = readdir(dp); //skip . and ..
		//ep = readdir(dp);
		char buffer3[50];
		char* tablePrefix = buffer3;
		strcpy(buffer3,tableName);
		tablePrefix = strcat(tablePrefix, "_");
		unsigned int length = (unsigned int)strlen(tablePrefix);
		char *filePrefix = NULL;
		filePrefix = new char[length];
		for (int i=0; i<length; i++) 
		{
		    filePrefix[i] = 0;    // Initialize all elements to zero.
		}
     		while (ep = readdir (dp))
      		{
			strncpy(filePrefix, ep->d_name, length);
			filePrefix[length]='\0';
			//cout << "fileprefix " << filePrefix << " table " << tablePrefix << "\n";
			if(strcmp(tablePrefix, filePrefix)==0)
			{
				count++;
				int col = atoi(&ep->d_name[strlen(ep->d_name)-1]);
				char* split = strtok(ep->d_name, "_");
				int indexCount=0;
				while(indexCount < 1)
				{
					
					split = strtok(NULL, "_");
					indexCount++;
				}
				//cout << "split " << split << "\n";
				//cout << "split " << split << "\n";
				//cout << "col " << col << " " << column <<" column " << "\n";
				if(col == column)
				{
					//tableName[strlen(tableName) - 1] = '\0';	
					return true;				
				}
				
			}
			
		}
		if(found==false)
		{	
			//tableName[strlen(tableName) - 1] = '\0';
			return false;
		}
		//cout << "count " << count << "\n";
    		(void) closedir (dp);
  	}
  	else
   	{
		//tableName[strlen(tableName) - 1] = '\0';
		//cout << "Couldn't open the /base directory\n";
	}
	
	//tableName[strlen(tableName) - 1] = '\0';
}




void bIndex::lookup(int key, int type, deque<int>& pointers)
{
	
	if(indexVector[0].getNum()==0)
	{}
	else
	{
		int n = indexVector[0].getRoot();	
		//read block n;		
		switch(type)
		{
			case 1://a>key return a
				{
					while(indexVector[n].getType() != 3) //type 3 is leaf node
					{
						n=indexVector[n].getPtr(key);
					}
					//n is leaf block node
					int j;
					
					for(j=4;j<=10;j=j+2)
					{
						if(indexVector[n].getInfo(j) > key)
						{
							break;
						}
					}
					if(indexVector[0].getDups()==1)
					{
						if(j>10 && indexVector[n].getNext()!=-1)
						{
							n=indexVector[n].getNext();
							j=4;
						}
						if(j!=12)
						{
							while(indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)
							{
								if(indexVector[n].getInfo(j)==-1)
								{
									n=indexVector[n].getNext();
									j=4;
								}else if(j==10 && indexVector[n].getNext()!=-1)
								{	
									int te = indexVector[n].getInfo(j-1);
									while (indexVector[te].getNext()!=-1)
									{
								    		for(int i=2;i<11;i++)
								    		{
								    			pointers.push_back(indexVector[te].getInfo(i));
								    		}
								    		te = indexVector[te].getNext();
									}
									int i=2;
									while(indexVector[te].getInfo(i)!=-1)
									{
										pointers.push_back(indexVector[te].getInfo(i));
										i++;
									}		
									n=indexVector[n].getNext();
									j=4;
								}
								else if (j==10 && indexVector[n].getNext()==-1)
								{
									int te = indexVector[n].getInfo(j-1);
									while (indexVector[te].getNext()!=-1)
									{
								    		for(int i=2;i<11;i++)
								    		{
								    			pointers.push_back(indexVector[te].getInfo(i));
								    		}
								    		te = indexVector[te].getNext();
									}
									int i=2;
									while(indexVector[te].getInfo(i)!=-1)
									{
										pointers.push_back(indexVector[te].getInfo(i));
										i++;
									}
									break;
								}else
								{
									int te = indexVector[n].getInfo(j-1);
									while (indexVector[te].getNext()!=-1)
									{
								    		for(int i=2;i<11;i++)
								    		{
								    			pointers.push_back(indexVector[te].getInfo(i));
								    		}
								    		te = indexVector[te].getNext();
									}
									int i=2;
									while(indexVector[te].getInfo(i)!=-1)
									{
										pointers.push_back(indexVector[te].getInfo(i));
										i++;
									}
									j=j+2;
								}
							}
						}					
						
					}
					else
					{
						if(j>10 && indexVector[n].getNext()!=-1)
						{
							n=indexVector[n].getNext();
							j=4;
						}
						if(j!=12)
						{
							while(indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)
							{
								if(indexVector[n].getInfo(j)==-1)
								{
									n=indexVector[n].getNext();
									j=4;
								}else if(j==10 && indexVector[n].getNext()!=-1)
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[n].getInfo(j-1));
									n=indexVector[n].getNext();
									j=4;
								}
								else if (j==10 && indexVector[n].getNext()==-1)
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[n].getInfo(j-1));
									break;
								}else
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[n].getInfo(j-1));
									j=j+2;
								}
							}
						}
					}
							
				}			
				break;
			case 2://a>=key return a
				{
					while(indexVector[n].getType() != 3) //type 3 is leaf node
					{
						n=indexVector[n].getPtr(key);
					}
					//n is leaf block node
					int j;
					for(j=4;j<=10;j=j+2)
					{
						if(indexVector[n].getInfo(j) >= key)
						{	//cout<<indexVector[n].getInfo(j)<<"\n";
							break;
						}
					}
					if(indexVector[0].getDups()==1)
					{
						if(j>10 && indexVector[n].getNext()!=-1)
						{
							n=indexVector[n].getNext();
							j=4;
						}
						if(j!=12)
						{
							while(indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)
							{
								if(indexVector[n].getInfo(j)==-1)
								{
									n=indexVector[n].getNext();
									j=4;
								}else if(j==10 && indexVector[n].getNext()!=-1)
								{
									int te = indexVector[n].getInfo(j-1);
									while (indexVector[te].getNext()!=-1)
									{
								    		for(int i=2;i<11;i++)
								    		{
								    			pointers.push_back(indexVector[te].getInfo(i));
								    		}
								    		te = indexVector[te].getNext();
									}
									int i=2;
									while(indexVector[te].getInfo(i)!=-1)
									{
										pointers.push_back(indexVector[te].getInfo(i));
										i++;
									}		
									n=indexVector[n].getNext();
									j=4;
								}
								else if (j==10 && indexVector[n].getNext()==-1)
								{
									int te = indexVector[n].getInfo(j-1);
									while (indexVector[te].getNext()!=-1)
									{
								    		for(int i=2;i<11;i++)
								    		{
								    			pointers.push_back(indexVector[te].getInfo(i));
								    		}
								    		te = indexVector[te].getNext();
									}
									int i=2;
									while(indexVector[te].getInfo(i)!=-1)
									{
										pointers.push_back(indexVector[te].getInfo(i));
										i++;
									}
									break;
								}else
								{
									int te = indexVector[n].getInfo(j-1);
									while (indexVector[te].getNext()!=-1)
									{
								    		for(int i=2;i<11;i++)
								    		{
								    			pointers.push_back(indexVector[te].getInfo(i));
								    		}
								    		te = indexVector[te].getNext();
									}
									int i=2;
									while(indexVector[te].getInfo(i)!=-1)
									{
										pointers.push_back(indexVector[te].getInfo(i));
										i++;
									}
									j=j+2;
								}
							}
						}						
						
					}
					else
					{	/*cout<<"no duplicates >=\n";
						for(int mba=0;mba<=indexVector[0].getNum();mba++)	
						{	for(int s=0;s<12;s++)	
							{cout<<indexVector[mba].getInfo(s)<<" ";}
							cout<<"\n";
						}*/
						if(j>10 && indexVector[n].getNext()!=-1)
						{	//cout<<indexVector[n].getInfo(j)<<"\n";
							n=indexVector[n].getNext();
							j=4;
						}//cout<<"j: "<<j<<"\n";
						if(j!=12)
						{
							while(indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)
							{	
								if(indexVector[n].getInfo(j)==-1)
								{
									n=indexVector[n].getNext();
									j=4;
								}else if(j==10 && indexVector[n].getNext()!=-1)
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[n].getInfo(j-1));
									n=indexVector[n].getNext();
									j=4;
								}
								else if (j==10 && indexVector[n].getNext()==-1)
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[n].getInfo(j-1));
									break;
								}else
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[n].getInfo(j-1));
									j=j+2;
								}
							}
						}
					}
				}
				break;
			case 3://a<key return a
				{
					/*for(int mba=0;mba<=indexVector[0].getNum();mba++)	
					{	if(indexVector[mba].getType()!=5)
						{for(int s=0;s<12;s++)	
						{cout<<indexVector[mba].getInfo(s)<<" ";}
						cout<<"\n";}
					}*/				
					while(indexVector[n].getType() != 3) //type 3 is leaf node
					{
						n=indexVector[n].getInfo(3);
					}
					/*for(int s=0;s<12;s++)	
					{cout<<indexVector[n].getInfo(s)<<" ";}
						cout<<"\n";
					for(int s=0;s<12;s++)	
					{cout<<indexVector[6856].getInfo(s)<<" ";}
						cout<<"\n";*/
					
					int j=4;
					//n is leaf block node
					int count=0;
					if(indexVector[0].getDups()==1)
					{
						while((indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)&& indexVector[n].getInfo(j) < key)
						{
							if(indexVector[n].getInfo(j)==-1)
							{
								n=indexVector[n].getNext();
								j=4;
								
							}else if(j==10 && indexVector[n].getNext()!=-1)
							{
								int te = indexVector[n].getInfo(j-1);
								while (indexVector[te].getNext()!=-1)
								{
							    		for(int i=2;i<11;i++)
							    		{	//cout<<indexVector[n].getInfo(j)<<"\n";
							    			pointers.push_back(indexVector[te].getInfo(i));
										count++;
							    		}
							    		te = indexVector[te].getNext();
								}
								int i=2;
								while(indexVector[te].getInfo(i)!=-1)
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[te].getInfo(i));
									i++;
								}		
								n=indexVector[n].getNext();
								j=4;
							}
							else if (j==10 && indexVector[n].getNext()==-1)
							{
								int te = indexVector[n].getInfo(j-1);
								while (indexVector[te].getNext()!=-1)
								{
							    		for(int i=2;i<11;i++)
							    		{	//cout<<indexVector[n].getInfo(j)<<"\n";
							    			pointers.push_back(indexVector[te].getInfo(i));
							    		}
							    		te = indexVector[te].getNext();
								}
								int i=2;
								while(indexVector[te].getInfo(i)!=-1)
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[te].getInfo(i));
									i++;
								}
								break;
							}else
							{
								int te = indexVector[n].getInfo(j-1);
								while (indexVector[te].getNext()!=-1)
								{
							    		for(int i=2;i<11;i++)
							    		{	//cout<<indexVector[n].getInfo(j)<<"\n";
							    			pointers.push_back(indexVector[te].getInfo(i));
							    		}
							    		te = indexVector[te].getNext();
								}
								int i=2;
								while(indexVector[te].getInfo(i)!=-1)
								{	//cout<<indexVector[n].getInfo(j)<<"\n";
									pointers.push_back(indexVector[te].getInfo(i));
									i++;
								}
								j=j+2;
							}
						} 					
						
					}
					else
					{	/*for(int mba=0;mba<=indexVector[0].getNum();mba++)	
						{	for(int s=0;s<12;s++)	
							{cout<<indexVector[mba].getInfo(s)<<" ";}
							cout<<"\n";
						}*/
						while((indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)&& indexVector[n].getInfo(j) < key)
						{
							if(indexVector[n].getInfo(j)==-1)
							{
								n=indexVector[n].getNext();
								j=4;
							}else if(j==10 && indexVector[n].getNext()!=-1)
							{	//cout<<indexVector[n].getInfo(j)<<"\n";
								pointers.push_back(indexVector[n].getInfo(j-1));
								n=indexVector[n].getNext();
								j=4;
							}
							else if (j==10 && indexVector[n].getNext()==-1)
							{	//cout<<indexVector[n].getInfo(j)<<"\n";
								pointers.push_back(indexVector[n].getInfo(j-1));
								break;
							}else
							{	//cout<<indexVector[n].getInfo(j)<<"\n";
								pointers.push_back(indexVector[n].getInfo(j-1));
								j=j+2;
							}
						}
					}
				}
				break;
			case 4://a<=key return a
				{
					while(indexVector[n].getType() != 3) //type 3 is leaf node
					{
						n=indexVector[n].getInfo(3);
					}
					int j=4;
					//n is leaf block node
					if(indexVector[0].getDups()==1)
					{
						while((indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)&& indexVector[n].getInfo(j) <= key)
						{
							if(indexVector[n].getInfo(j)==-1)
							{
								n=indexVector[n].getNext();
								j=4;
							}else if(j==10 && indexVector[n].getNext()!=-1)
							{
								int te = indexVector[n].getInfo(j-1);
								while (indexVector[te].getNext()!=-1)
								{
							    		for(int i=2;i<11;i++)
							    		{
							    			pointers.push_back(indexVector[te].getInfo(i));
							    		}
							    		te = indexVector[te].getNext();
								}
								int i=2;
								while(indexVector[te].getInfo(i)!=-1)
								{
									pointers.push_back(indexVector[te].getInfo(i));
									i++;
								}		
								n=indexVector[n].getNext();
								j=4;
							}
							else if (j==10 && indexVector[n].getNext()==-1)
							{
								int te = indexVector[n].getInfo(j-1);
								while (indexVector[te].getNext()!=-1)
								{
							    		for(int i=2;i<11;i++)
							    		{
							    			pointers.push_back(indexVector[te].getInfo(i));
							    		}
							    		te = indexVector[te].getNext();
								}
								int i=2;
								while(indexVector[te].getInfo(i)!=-1)
								{
									pointers.push_back(indexVector[te].getInfo(i));
									i++;
								}
								break;
							}else
							{
								int te = indexVector[n].getInfo(j-1);
								while (indexVector[te].getNext()!=-1)
								{
							    		for(int i=2;i<11;i++)
							    		{
							    			pointers.push_back(indexVector[te].getInfo(i));
							    		}
							    		te = indexVector[te].getNext();
								}
								int i=2;
								while(indexVector[te].getInfo(i)!=-1)
								{
									pointers.push_back(indexVector[te].getInfo(i));
									i++;
								}
								j=j+2;
							}
						}						
						
					}
					else
					{	//cout<<"no duplicates <=\n";
						/*for(int mba=0;mba<=indexVector[0].getNum();mba++)	
						{	for(int s=0;s<12;s++)	
							{cout<<indexVector[mba].getInfo(s)<<" ";}
							cout<<"\n";
						}*/
						while((indexVector[n].getInfo(j) != -1 | indexVector[n].getNext() != -1)&& indexVector[n].getInfo(j) <= key)
						{
							if(indexVector[n].getInfo(j)==-1)
							{
								n=indexVector[n].getNext();
								j=4;
							}else if(j==10 && indexVector[n].getNext()!=-1)
							{	//cout<<indexVector[n].getInfo(j)<<"\n";
								pointers.push_back(indexVector[n].getInfo(j-1));
								n=indexVector[n].getNext();
								j=4;
							}
							else if (j==10 && indexVector[n].getNext()==-1)
							{	//cout<<indexVector[n].getInfo(j)<<"\n";
								pointers.push_back(indexVector[n].getInfo(j-1));
								break;
							}else
							{	//cout<<indexVector[n].getInfo(j)<<"\n";
								pointers.push_back(indexVector[n].getInfo(j-1));
								j=j+2;
							}
						}
					}
				}
				break;
			default:
				break;
		}
	}	
}
