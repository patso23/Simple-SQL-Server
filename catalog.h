//catalog.h
#ifndef CATALOG_H
#define CATALOG_H


#include <iostream>
#include <list>
#include <string>
#include "table.h"


class Catalog
{
	public:
		//implemented
		Catalog();
		~Catalog();
		void AddTable(Table add);  
		int Size();
		void InsertAttribute();  
		int WriteCatalog();
		int ReadCatalog();
		int PrintCatalog();
		bool IsTable(char* test);		 
		bool DropTable(char* drop);
		Table GetTable(char* tab);
		int GetMaxID();
		
		//not implemented yet
		void UpdateNumAttributes();
		void NumAttributes();
		void UpdateTable();
		void UpdateAttribute();
		


	protected:
	std::list<Table> Tables;
};

#endif
