//Catalog.cpp

#include <iostream>
#include <list>
#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "catalog.h"
#include "table.h"




using namespace std;


Catalog::Catalog()
{//constructor
	

}


Catalog::~Catalog()
{//Destructor

}


void Catalog::AddTable(Table add)
{

	Tables.push_back(add);
	
}

int Catalog::Size()
{
	return Tables.size();
}

int Catalog::WriteCatalog()
{
	ofstream writeCat("base/cat", ios::out);
	if(writeCat.is_open())
	{
		//write the file: table, attributes ; etc.
		list<Table>::iterator i;
		i = Tables.begin();
		list<string>::iterator j;
		Table tmp;
		
		while(i != Tables.end())	//write tables
		{	
			tmp = *i;				
			writeCat << tmp.GetTableName() << " " << tmp.GetTableID() << " " << tmp.Size() << " ";
			j = tmp.AttributesBegin();
			while( j != tmp.AttributesEnd())
			{
				writeCat << tmp.GetAttributeName(j) << " ";
				j++;
			}
			i++;
			
			writeCat << "; ";
		}
		
		writeCat.close();
		return 0;


	} 
	else
	{
		cout << "Error opening catalog file\n";
		return 1;
	}
	
}

int Catalog::ReadCatalog()
{
	char buff;
	string tableString;
	ifstream readCat("base/cat", ios::in);
	if(readCat.is_open())
	{
		 
		while(!readCat.eof())
		{
			
			buff = readCat.get();		
			while(!readCat.eof() && buff !=  ';')
			{	
				tableString.push_back(buff);  	
				buff = readCat.get();
				
			}
			if(!tableString.empty())
			{
				tableString.erase(tableString.end()-1);	//removes single trailing whitespace		
			}			
			readCat.get();		//advances file pointer
			
			//tokenize
			if(!tableString.empty()) 
			{
				
				string tmpTableName, tmpId, tmpRecords, tmpAttr;
				int id, records, numattr;				
				vector<string> tableTokens;
				boost::split(tableTokens, tableString, boost::is_any_of("\t "));
				vector<string>::iterator i;
				i = tableTokens.begin();
				tmpTableName = *i;
				i++; //move to tableid
				tmpId = *i;
				id = atoi(tmpId.c_str());
				i++; //move to number of records
				tmpAttr = *i;
				numattr = atoi(tmpAttr.c_str());
				i++;	//move to attributes

				//Create the table
				Table tmpTable;
				tmpTable.SetName(tmpTableName);
				tmpTable.SetTableID(id);
				
				int count=0;
				while(i != tableTokens.end())
				{
					
					tmpTable.AddAttribute(*i);
					i++;
					count++;
				}
							
				tmpTable.SetNumAttributes(count);

	

				
				//Put table in Catalog
				Tables.push_back(tmpTable);
				tableString = "";	//reset table string
					
			}
			
		}
			
		return 0;


	}
	else
	{
		cout << "Error opening catalog file.  A new catalog will be created.\n";
		return 1;
	}	

}

int Catalog::PrintCatalog()
{
	list<Table>::iterator i;
	i = Tables.begin();
	
	cout << "Catalog rsize: " << Tables.size() << "\n";

	Table tmp;
	list<string>::iterator j;
	int count = 0;
	
	

	while(count < Tables.size())
	{
		tmp = *i;
		cout << "\n\tTable: " << tmp.GetTableName() << "\t";  
		cout << "TableID: " << tmp.GetTableID() << "\t";
		cout << "# of columns: " << tmp.Size() << "\t\n\t-----\n";
		cout << "\tColumns:\t\n\t-----\n\t";		
		j = tmp.AttributesBegin();
		
		while(j != tmp.AttributesEnd())
		{	
			 cout << tmp.GetAttributeName(j) << "\t";
			j++;
		}
		cout << "\n";
		i++;
		count++;
	
	}

		

}

bool Catalog::IsTable(char* test)
{

	list<Table>::iterator i;
	i = Tables.begin();
	Table tmp;
	string s = string(test);

	while(i != Tables.end())
	{
		tmp = *i;	
		if(s.compare(tmp.GetTableName()) == 0)
		{
			return true;
		}
		i++;
	}
	return false;
	
}

bool Catalog::DropTable(char* drop)
{
	list<Table>::iterator i;
	i = Tables.begin();
	string s = string(drop);
	Table tmp;

	cout << "Size before drop " << Tables.size() << "\n";
	

	while (i != Tables.end())
	{	
		tmp = *i;
		if(s.compare(tmp.GetTableName())==0)
		{
			Tables.erase(i);
			break;
		}
		i++;
	}
	
}

Table Catalog::GetTable(char* tab)
{
	list<Table>::iterator i;
	i = Tables.begin();
	string s = string(tab);
	Table tmp;
	Table *ret;	

	while(i != Tables.end())
	{

		tmp = *i;
		if(s.compare(tmp.GetTableName())==0)
		{
			//ret = &tmp;
			//return ret;
			return *i;
		}
		i++;
	}

}

int Catalog::GetMaxID()
{
list<Table>::iterator i;
i = Tables.begin();
int tmp, max = 0;
Table tmpTable;


	while(i != Tables.end())
	{
		tmpTable = *i;	
		tmp = tmpTable.GetTableID();
		if(tmp > max)
		{
			max = tmp;
		}
		i++;
	}
	return tmp;

}

void Catalog::UpdateTable()
{ 


}
 


void Catalog::UpdateAttribute()
{

}


void Catalog::UpdateNumAttributes()
{

}




void Catalog::NumAttributes()
{

}




