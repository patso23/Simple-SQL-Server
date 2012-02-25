//Table.h
#ifndef TABLE_H
#define TABLE_H


#include <iostream>
#include <list>



class Table
{

	public:
		Table();
		~Table();
		bool SetName(std::string name);
		int AddAttribute(std::string attr);
		int Size();
		std::list<std::string>::iterator AttributesBegin();
		std::list<std::string>::iterator AttributesEnd();
		std::string GetAttributeName(std::list<std::string>::iterator j);
		std::string GetTableName();
		bool IsAttribute(char* attr);
		void PrintTable();
		bool UpdateAttribute(std::string attr, std::string newattr);
		void SetTableID(int id);
		int GetTableID();
		int SetNumAttributes(int n);
		int GetAttributeNum(char* name);
		

	protected:
		std::string TableName;
		int TableID; //used for storage manager
		int NumAttributes;
		std::list<std::string> AttributeNames;

	

};

#endif
