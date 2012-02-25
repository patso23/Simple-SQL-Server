//bindex.h
#ifndef BINDEX_H
#define BINDEX_H


#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include "node.h"
#include "bManager.h"

using namespace std;



class bIndex {

	public:
		bIndex();
		~bIndex();
		bool createIndex(int attr, int dups, char* tab, char* indexName,bManager& bMan);  //also create index header block
		bool insert(int key, int ptr);
		bool testInsert(int key, int ptr );
		bool insertDups(int key, int ptr);
		bool insertNoDups(int key, int ptr);
		void debugOn();
		void debugOff();		
		int lookupNoDups(int key);
		void lookupDups(int key,deque<int>& pointers);
		void printIndex();
		bool readIndex(char* tableName, int attrid);
		void writeIndex(char* tableName,char* indexName);
		void setTableName(char* table);		
		std::string getTableName();
		std::string getIndexName();
		int select_create_index(char* tableName, int attrid, int (*array)[2], bManager& bMan);
		void setIndexName(char* s);
		void setKey(char* key);
		int getAttr();
		void setAttr(int attrid);
		void setLoaded(bool load);
		bool getLoaded();
		void clear();
		int getAttrNum(char* index, char* tableName);
		bool getDups();
		bool indexExist(int column, char* tableName);
		void lookup(int key, int type, deque<int>& pointers);

	protected:
		string tablename;
		bool debug;
		vector<node> indexVector;
		string indexname;
		int attr;
		bool loaded;


};





#endif
