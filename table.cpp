//Table.cpp
#include <iostream>
#include <list>
#include "table.h"

using namespace std;

Table::Table()
{//constructor
	
}


Table::~Table()
{//destructor

}

string Table::GetTableName()
{
	return TableName;
}


string Table::GetAttributeName(list<string>::iterator j)
{
	return *j;

}



int Table::AddAttribute(string attr)
{
	if(attr.empty())
	{
		cout << "NULL value!" << "\n";
	}	
	else if(attr.length()<=3)
	{
		AttributeNames.push_back(attr);
	
	}
	else
	{
		//name too long
		cout << "Attribute name must be three characters or less\n";
	}
}


int Table::Size()
{
	return NumAttributes;
		
}

int Table::SetNumAttributes(int n)
{
	NumAttributes = n;
	return NumAttributes;
}

list<string>::iterator Table::AttributesBegin()
{
	return AttributeNames.begin();
}

list<string>::iterator Table::AttributesEnd()
{
	return AttributeNames.end();
}

bool Table::SetName(string name)
{
	if(name.length() <= 3)
	{
		TableName = name;	
		return true;
	}
	else
	{
		cout << "Table name must be three characters or less\n";
		return false;
	}

}



void Table::SetTableID(int id)
{
	TableID = id;
}


int Table::GetTableID()
{
	return TableID;
}

bool Table::IsAttribute(char* attr)
{
	list<string>::iterator i;
	i = AttributeNames.begin();
	string s = string(attr);

	
	while(i != AttributeNames.end())
	{
		if(s.compare(*i) == 0)
		{
			return true;
		}
		i++;
	}
	
	return false;
}

void Table::PrintTable()
{
	list<string>::iterator i;
	
	cout << "Table Name : " << GetTableName() << "\n-----\n";
	for(i = AttributeNames.begin(); i != AttributeNames.end(); ++i)
	{		
		cout << *i << " ";
	}
	cout << "\n\n\n";

}

int Table::GetAttributeNum(char* name)
{
	int x;
	int count = 0;
	list<string>::iterator i;
	string s = string(name);
	
	for(i = AttributeNames.begin(); i!= AttributeNames.end(); ++i)
	{
		if(s.compare(*i)==0)
		{
			x = count;
		}	
		count++;
	}

	return x;
}

bool Table::UpdateAttribute(string attr, string newAttr)
{
	list<string>::iterator i;
	i = AttributeNames.begin();
	string tmp = *i;	

	//find desired attribute
	while(i != AttributeNames.end() && tmp.compare(attr) != 0)
	{
		i++;
		tmp = *i;

	}
	
	i++;	
	AttributeNames.remove(attr);
	AttributeNames.insert(i, newAttr);
		
	
}




