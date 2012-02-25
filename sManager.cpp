#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include "sManager.h"
#include "stdio.h"
#include "stdlib.h"

using namespace std;

int Block::getBID()
{
	int getid=blockID;
        return getid;
}


void Block::setBID(int setid)
{
	blockID = setid;
}


void Block::builtfile(string Tablename)
{

       string k="base/"+string(Tablename)+".tbl";
	const char* c = k.c_str();
       ofstream myfile;
       myfile.open(c);
       myfile.close();
}
;



void Block::writeBlock(string Tname, int blockid, int* Barray)
{
	
	//cout << "getting into write \n";
      	int* array = Barray;

	int maxblockid = getmaxBlock(Tname);
	//cout << "block id " << blockid << " max blockid " << maxblockid<< "\n";
	
	if(blockid <= maxblockid)
	{	 string f = "base/"+Tname +".tbl";
		const char* c = f.c_str();
		ofstream myfile(c, ios::in | ios::out | ios::binary);
		
		myfile.seekp((48*(blockid-1)), ios::beg);
		myfile.write((char *)array, 48);
		myfile.close();

	}
	else
	{
		 string f = "base/"+Tname +".tbl";
		const char* c = f.c_str();
		ofstream myfile(c, ios::app | ios::binary);
     		myfile.write((char *)array, 48);
      		myfile.close();
		//cout << "end else\n";

	}
}



int* Block::readBlock (string Tname, int blockID)
{
	string f = "base/"+Tname +".tbl";
	const char* c = f.c_str();
        ifstream myfile (c, ios::in | ios::binary);  
	myfile.seekg((48*blockID)-48,ios::beg);
        int* array=(int *)malloc(48);
        // for (int j=0;j<12;j++)
        //{array[j]=0;} 
        myfile.read((char *)array,48);
	myfile.clear();  
        myfile.close();

	
	


       return array;
//use seek to change the get pointer
//the position is 4*blockID

}



void Block::writeNode(string indexFileName, int blockid, int* Narray)
{
	
	int* array = Narray;
	int maxblockid = getmaxNode(indexFileName);
	


	if(blockid == 0)
	{
		 string f = "base/"+indexFileName;
		const char* c = f.c_str();
		ofstream myfile(c, ios::out | ios::binary);
		//myfile.seekp((48*(blockid-1)), ios::beg);
		myfile.write((char *)array, 48);
		myfile.close();
	}
	else if(blockid < maxblockid)
	{	 string f = "base/"+indexFileName;
		const char* c = f.c_str();
		ofstream myfile(c, ios::in | ios::out | ios::binary);
		myfile.seekp((48*(blockid-1)), ios::beg);
		myfile.write((char *)array, 48);
		myfile.close();

	}
	else
	{
		 string f = "base/"+indexFileName;
		const char* c = f.c_str();
		ofstream myfile(c, ios::app | ios::binary);
     		myfile.write((char *)array, 48);
      		myfile.close();
		//cout << "end else\n";

	}
}




int* Block::readNode (string indexFileName, int blockID)
{
	
	string f = "base/"+indexFileName;
	//cout << "indexFileName " << indexFileName << " blockid " << blockID <<"\n";

	const char* c = f.c_str();
        ifstream myfile (c, ios::in | ios::binary);  
	myfile.seekg((48*blockID)-48,ios::beg);
        int* array=(int *)malloc(48); 
        myfile.read((char *)array,48);
	myfile.clear();  
        myfile.close();
	
	
	

       return array;
//use seek to change the get pointer
//the position is 4*blockID

}


int Block::getmaxBlock(string tablename)
{
int k;
string f = "base/"+tablename+".tbl";
const char* c = f.c_str();
ifstream myfile (c, ios::in | ios::binary);
myfile.seekg(0,ios::end);                        //putthe get pointer to the end of the file
k=myfile.tellg();         
                             //let k = the length between the begin and the end...
myfile.clear();
myfile.close();                         //because the last block may be not full, so the maxnumber=((k-1)/48+1)

if(k==0)
{
	return 0;
}
else
{	
	return (k/48);

}
}


int Block::getmaxNode(string tablename)
{
int k;
string f = "base/"+tablename;
const char* c = f.c_str();
ifstream myfile (c, ios::in | ios::binary);
myfile.seekg(0,ios::end);                        //putthe get pointer to the end of the file
k=myfile.tellg();         
                             //let k = the length between the begin and the end...
myfile.clear();
myfile.close();                         //because the last block may be not full, so the maxnumber=((k-1)/48+1)

if(k==0)
{
	return 0;
}
else
{	
	return (k/48);

}
}


