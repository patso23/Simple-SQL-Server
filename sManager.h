#ifndef SMANAGER_H
#define SMANAGER_H

#include <iostream>
#include <string>
#include <fstream>
#include "node.h"


class Block
{

	public:
		char* blocksize[48];
		int *readBlock(std::string Tname, int blockID);
		void builtfile(std::string Tablename);
		void writeBlock(std::string Tname, int blockid, int *Barray);
		int getmaxBlock(std::string tablename);
		void writeNode(std::string indexFileName, int blockid, int *Narray);
		int *readNode(std::string indexFileName, int blockid);
		int getmaxNode(std::string tablename);

	private:
	        int blockID;
		void setBID(int setid);
		int getBID();
		
		
};







#endif
