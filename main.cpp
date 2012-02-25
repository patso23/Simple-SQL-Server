#include "parser.h"
#include "print.h"
#include "table.h"
#include "catalog.h"
#include "bManager.h"
#include "sManager.h"
#include "main.h"
#include "bIndex.h"
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <dirent.h>
#include <cstring>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>


//consider using the GNU Readline library instead of myreadline()
//it has a similar interface, and you'll get nice features like working
//cursor keys and (if initialized properly) history.
//run "man 3 readline" for more information
using namespace std;

char* myreadline(char* prompt);
bIndex ind;

int main(int argc, char *argv[]){
	
   char* input=NULL;
   statement_t* parsed=NULL;
   int stillrunning=1;
   int timer =0;
   int timerop=0;
   struct timeval start, end, startGlobal, endGlobal;

   
   
   		
     		
   //test for database directory, create if doesn't exist
   struct stat buffer;
   int test =stat("base", &buffer);
   if(test<0)
   {
	int status;
	status = mkdir("base", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if(status<0)
        {
		cout << "Failed to create database directory\n";
	 	
        }
	else
        {
		cout << "Directory created\n";

	}
   } 

   //initialize and read catalog
   Catalog myCatalog;
   myCatalog.ReadCatalog();


   //set up Storage Manager
   Block sMan;
 
   //pass buffersize to buffer manager
   //string s = strdup(argv[1]);
   
   bManager bMan;
	
   

   int buffsize;
   if(argc<2)
   {
   	bMan.bManagerSet(3);
	buffsize=3;
   }
   else
   {   
	int size = atoi(argv[1]);
	buffsize=size;
   	bMan.bManagerSet(size);
   }
	

   while (stillrunning &&  (input=myreadline(">> ")) != 0){
      //change to lowercase
      string s = string(input);
      for (int i=0;i<s.size();i++) 
      {
	 input[i] = tolower(s[i]);
      }

      
      parsed=parse_statement(input);
      if (parsed){
	 /*
	  * dispatch_print is an example of how to work with the AST to detect
	  * and run the various commands, as well as how to get at all of the
	  * information in the AST. Remove this call when you implement the
	  * logic to actually run the commands in the DBMS.
	  * (You can use the functions in print.c for debugging purposes)
	  */
	if(parsed->set)
	{	gettimeofday(&start, NULL);
		
		//cout << "value " << parsed->set->value << " variable " << parsed->set->variable << "\n";
		if((parsed->set->value == 1) && parsed->set->variable==0)
		{	 
     			//gettimeofday(&start, NULL);
			gettimeofday(&startGlobal, NULL);
			timer=1;
			timerop=1;
	 		//cout << "Timer on! \n";
		} 
		else if((parsed->set->value == 0) && parsed->set->variable==0)
		{
			//cout << "Timer off!\n";
			timer =0;
		}
		else if((parsed->set->value == 1) && parsed->set->variable==1)
		{
			ind.debugOn();
			//cout << "debug on \n";
		}
		else
		{
			//cout << "debug off\n";
			ind.debugOff();
		}
	}
	else if(parsed->parameterless == CMD_PRINT_CATALOG)
	{	
		gettimeofday(&start, NULL);
		
		myCatalog.PrintCatalog();

	}
	if(parsed->print_table)
	{	gettimeofday(&start, NULL);
		//find table id
		Table  tmp = myCatalog.GetTable(parsed->print_table);
		int id = tmp.GetTableID();
		string s = tmp.GetTableName();
		int blockid;
		
		int maxblockid = sMan.getmaxBlock(s);
		
		maxblockid=maxblockid+buffsize;
		
		int* mem;	
		select_star_unconditional(tmp, id, s, maxblockid, bMan);

			
	
	}
	else if(parsed->parameterless == CMD_PRINT_BUFFER)
	{	gettimeofday(&start, NULL);
		//implement buffer call here
		bMan.printBuffer();
		//cout << "Buffer!\n";
	}
	else if(parsed->parameterless == CMD_PRINT_HIT_RATE)
	{	gettimeofday(&start, NULL);
		//implement buffer call here
		bMan.printHitRate();		
		//cout << "Hit Rate!\n";
	}
	else if(parsed->parameterless == CMD_COMMIT)
	{
		gettimeofday(&start, NULL);
		//implement a buffer call and catalog write here
		bMan.commit();
		//cout << "Commit!\n";
		
		

	}	
	else if(parsed->create_table)
	{	gettimeofday(&start, NULL);

		//does table exist?  if: yes fail
		
		int success = 0;
		if(!myCatalog.IsTable(parsed->create_table->table))
		{

			//create table object
			Table tmp;
			tmp.SetName(parsed->create_table->table);

			//add/test attributes
			id_list_t* it = parsed->create_table->columns;
			int count=0;
			while(it)
			{
				if(!tmp.IsAttribute(it->id))
 				{
					tmp.AddAttribute(it->id);
				}
				else
				{
					cout << "Attribute: " << it->id 
						<< " already exists.  Each must be unique.  Create aborted.\n";
					success = 1; //flag for not adding to catalog
 					break;
				}
				it = it->next;
				count++;
			}
			
			tmp.SetNumAttributes(count);
			int maxID = myCatalog.GetMaxID();				
			if(maxID < 0)
			{
				tmp.SetTableID(1);	
			}
			else
			{
				tmp.SetTableID(myCatalog.GetMaxID()+1);
			}
			//query validated, send to buffer manager and on success, add to catalog with buffer info
			//buffermanager call goes here			
			sMan.builtfile(tmp.GetTableName());

			
			if(success==0)
			{
				myCatalog.AddTable(tmp);
			}


		}
		else
		{
			cout << "Table: " << parsed->create_table->table << " already exists.  Create aborted.\n";
		}
	

	}
	else if(parsed->drop_table)
	{	
		gettimeofday(&start, NULL);
		//check if table exists, if so drop it, otherwise display error
		
		if(myCatalog.IsTable(parsed->drop_table))
		{
			//table exists
			
			Table tmp = myCatalog.GetTable(parsed->drop_table);
			int id = tmp.GetTableID();
			bMan.flushTab(id);
			myCatalog.DropTable(parsed->drop_table);
			//delete file
			string t = parsed->drop_table;
			string s = "base/" + t + ".tbl";
			//cout << "file delete string " << s << "\n";
			if(remove(s.c_str()) != 0 )
			{
				 cout << "Error deleting file" << "\n";
			} 			
			else
			{
  				  cout << "File successfully deleted" << "\n";			
			}
			
 
		}
		else
		{
			//table does not exist
			cout << "Table: " << parsed->drop_table << " does not exist.  Drop aborted.\n";

		}



	}
	else if(parsed->insert)
	{	
		gettimeofday(&start, NULL);
		//make sure table exists, make sure it's the correct number of values
		if(myCatalog.IsTable(parsed->insert->table))
		{
			
			//right number of values??
			Table tmp = myCatalog.GetTable(parsed->insert->table);
			num_list_t* it = parsed->insert->values;
			int tmpint=tmp.Size();
			int id = tmp.GetTableID();
			string tablename = tmp.GetTableName();
			
			
			
			if(length_num_list(it) > tmpint || length_num_list(it) < tmpint)
			{
				cout << "Incorrect number of values.  Insert Aborted\n";
			} 
			else
			{
				//appropriate call to buffer manager, on success update block info
				//cout << "Insert!\n";
				//call buffer manager with table id, linked list of values and counter
				
				bool canInsert=true;
				writeLoaded(myCatalog);
				int count=0;
				int ptr=0;
				num_list_t* ite = parsed->insert->values;
				for(int i=0;i<tmpint;i++)
				{
					//cout<<"table name"<<parsed->insert->table<<"\n";
					//cout<<"test can read "<<ind.readIndex(parsed->insert->table, i)<<"\n";
					if(ind.readIndex(parsed->insert->table, i))
					{
						int key = ite->num;
						//cout<<"key: "<<key<<"\n";
						//cout<<"ptr: "<<ptr<<"\n";
						if(!ind.testInsert(key, ptr ))
						{
							canInsert=false;
							cout<<"Error: Attempted to add duplicate value to no duplicates index\n";
							break;
						}
						//cout<<"it is test insert\n";
						count++;
						if(count==2)
							break;
					}
					ite=ite->next;
				}
				if (canInsert)
				{
					//cout<<"can Insert\n";
					bMan.insert(id, tablename, it, tmpint);
					ptr=bMan.getMax(id, tablename);
					count=0;
					num_list_t* iti = parsed->insert->values;
					for(int i=0;i<tmpint;i++)
					{
						//cout<<"table name"<<parsed->insert->table<<"\n";
						//cout<<"can read "<<ind.readIndex(parsed->insert->table, i)<<"\n";
						if(ind.readIndex(parsed->insert->table, i))
						{
							int key = iti->num;
							//cout<<"key: "<<key<<"\n";
							//cout<<"ptr: "<<ptr<<"\n";
							ind.insert(key,ptr);
							writeLoaded(myCatalog);
							count++;
							if(count==2)
								break;
						}
						iti=iti->next;
					}
					
				}
			}
		}
		else
		{
			cout << "Table: " << parsed->insert->table << " does not exist.  Insert aborted.\n";

		} 

	}
	else if(parsed->select)
	{

	  gettimeofday(&start, NULL);
	  //first make sure table exists, then make sure columns exist, then validate conditions
	  //cout << "op " << parsed->select->conditions->op << "\n";


	  if(myCatalog.IsTable(parsed->select->table))	
	  {		

		Table tmp = myCatalog.GetTable(parsed->select->table);		
		int id = tmp.GetTableID();
		string s = tmp.GetTableName();	
		int maxblockid = sMan.getmaxBlock(s)+buffsize;
			
		int *mem;			

			
		
		//check for select * 
		if(!parsed->select->fields && !parsed->select->conditions)
		{
			select_star_unconditional(tmp, id, s, maxblockid, bMan);
			
		}//endif !parsedfields && not select cond 


		else  //everything but select ?  
		{
				
			//check for conditional select * 
			int count = 0;
			int colCount = length_id_list(parsed->select->fields);
				
				
			if((parsed->select->conditions) || (parsed->select->fields))
			{	
				//make sure columns exist		
				id_list_t* it = parsed->select->fields;
				string s;
					
				while(it)
				{
					if(tmp.IsAttribute(it->id))  //column exists
	 				{
						count++;					
					}			
					it = it->next;
				}
					
			}
			else		//unconditional select *
			{
				count == colCount;
			}




			if(count == colCount)
			{
				//valid number of columns....need to check conditions
				if(!parsed->select->conditions)  //specific select all no conditions
				{
					int id = tmp.GetTableID();
					string s = tmp.GetTableName();	
					int maxblockid = sMan.getmaxBlock(s)+buffsize;
					int* mem;
					id_list_t* c = parsed->select->fields;
					int num, num2;
					int format;
					int siz=tmp.Size();
						
					//find number of fields
					int count=0;								
					while(c)
					{								
						count++;
						c = c->next;
					}
					select_list_unconditional(tmp, id, s, maxblockid, bMan, parsed, num, siz, format, count);
		        	}
				

			else	//* conditions			 
			{
				//check conditions
				condition_t* c = parsed->select->conditions;
				int left = 0; //0 is valid, 1 is invalid
				int right = 0;

				while(c)
				{	
					//check left column
					if(tmp.IsAttribute(c->left_col))
					{
						left = 1;
					}
							
					//check right column
					if(!c->right_col)
					{
						right = 1;				
					}
					else if(tmp.IsAttribute(c->right_col))
					{
						right = 1;
					}
					c = c->next;
				}

				if(left == 1 && right == 1)
				{
					//cout << "Query is valid\n";
					//Make appropriate call to buffer manager 
					int id = tmp.GetTableID();
					string s = tmp.GetTableName();	
					int maxblockid = sMan.getmaxBlock(s)+buffsize;
					int* mem;
						
					//find number of conditions
					condition_t* c=parsed->select->conditions;
					int count=0;								
					while(c)
					{								
						count++;
						c = c->next;
					}
							

					//select * where!
					if(!parsed->select->fields)//all *'s
					{

								
						//cout << "select *! where\n";
					
						//cout << "Table:\t" << s << "\n------\n";
				

							
						c = parsed->select->conditions;
						//cout << "conditions! " << c->right_num << "\n";
						int num, num2;
						int format;
						int siz=tmp.Size();				
							   	
							
			  				//for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
			 				//{	
							//	cout << tmp.GetAttributeName(j) << "\t";

			 				//}
			 				//cout << "\n";
								
							//select * from x where x = num
							if(count==1 && !c->right_col) //one condition, value based
							{
								//cout << "* one condition, value\n";
							  	select_star_conditional_one_value(tmp, id, s, maxblockid,  bMan, c, parsed, num, siz,
												 format,myCatalog);

							}
							else if(count==1 && c->right_col)
							{  //select * , one cond, column
								select_star_conditional_one_column(tmp, id, s, maxblockid, bMan, c, parsed, num, siz,
													 format);
							}
							else if(count==2 && !c->right_col)
							{
								  
								select_star_conditional_two_values(tmp, id, s, maxblockid, bMan, c, parsed, num, siz,
												 format,myCatalog);
							}
							else if(count==2 && c->right_col)
							{
								select_star_conditional_two_columns(tmp, id, s, maxblockid, bMan, c, parsed, num, siz,
													 format);

                                                        }				
							

						
					}//!parsed->fields




					else if(parsed->select->conditions)
					{	
						c=parsed->select->conditions;
						Table tmp = myCatalog.GetTable(parsed->select->table);		
						int id = tmp.GetTableID();
						string tablename = tmp.GetTableName();
							
						//cout << "Table:\t" << tablename << "\n------\n";			
						id_list_t* cols = parsed->select->fields;
						string s = tmp.GetTableName();
						int counter=0;
						//while(cols)
						//{
						//	cout << cols->id << "\t";
						//	cols=cols->next;
						//	counter++;							
						//}

			 			//cout << "\n";
						int format;
						int siz=tmp.Size();
						int num,num2;

						if(count==1 && !c->right_col)
						{	
								
							select_list_conditional_one_value(tmp, id, s, maxblockid, bMan, c, parsed, num, siz, format,
											 counter,myCatalog);
								
					        } 
						else if(count==1 && c->right_col)
						{
								
									
							select_list_conditional_one_column(tmp, id, s, maxblockid, bMan, c, parsed, num, siz, format,
												 counter, num2);

						}
						else if(count==2 && !c->right_col)
						{	

							select_list_conditional_two_values(tmp, id, s, maxblockid, bMan, c, parsed, num, siz, format,
												 counter, num2, count,myCatalog);

						}
						else if(count==2 && c->right_col)
						{
							
							select_list_conditional_two_columns(tmp, id, s, maxblockid, bMan, c, parsed, num, siz, format,
												 counter, num2, count);

						}
						     
							

					}//conditions	

					}
					else
					{
							cout << "One or more conditional columns are incorrect. Select aborted.\n";

					}
				  }
					
				}
				else
				{
					cout << "One or more columns do not exist.  Select aborted\n";
				}
				
				
			}//end else (not select * no conditions)

	  }//end  if(myCatalog.IsTable(parsed->select->table))	
	  else
	  {
		cout << "Table: " << parsed->select->table << " does not exist.  Select aborted.\n";
	  }
	


	}//end if(parsed->select)
	else if(parsed->create_index)
	{
		gettimeofday(&start, NULL);
		
		//make sure table exists, make sure attribute exists, check for max indices, check for unique indices
		if(myCatalog.IsTable(parsed->create_index->table))
		{
			
			Table tmp = myCatalog.GetTable(parsed->create_index->table);		
			int id = tmp.GetTableID();
			string s = tmp.GetTableName();	
			if(tmp.IsAttribute(parsed->create_index->column))
			{			
				int attr = tmp.GetAttributeNum(parsed->create_index->column);
				//int dups = atoi(*parsed->create_index->duplicates);
				//cout << "dups " << dups << "\n";
				//dups = atoi(&parsed->create_index->duplicates);
				if(checkIndex(parsed->create_index->table, parsed->create_index->index, attr)==0)
				{
			
					//ind.writeIndex();
					//ind.~index();
					writeLoaded(myCatalog);
					if(!ind.createIndex(attr, parsed->create_index->duplicates, parsed->create_index->table, 
							parsed->create_index->index,bMan))
					{
						ind.clear();
					}

					//cout << "Create index goes here!\n";

				}
			}
			else
			{
				cout << "Error:  Column " << parsed->create_index->column << " does not exist\n";
				
			}
		}
		else
		{
			cout << "Error: Table " << parsed->create_index->table << " does not exist\n";
		}
		
		
		
		
	}
	else if(parsed->print_index)
	{
		//cout << "print index goes here!\n";

		gettimeofday(&start, NULL);
		
		if(ind.getLoaded())
		{
			string tablename=ind.getTableName();
	    		string indexname =ind.getIndexName();
			int k = ind.getAttr();
			Table tmp = myCatalog.GetTable(const_cast<char *>(tablename.c_str()));

			char* table = (char*)malloc( sizeof( char ) *(tablename.length() +1) );
			char* index = (char*)malloc( sizeof( char ) *(indexname.length() +1) );
			
			strcpy(table, tablename.c_str());
			strcpy(index, indexname.c_str());
			
			

        		
			cout << index << "\n ";
			cout << table << " \n";
			cout << "getting into else\n";
			
			if((strcmp(parsed->print_index->index, index) ==0) && strcmp(parsed->print_index->table, table)==0)
			{	//is index in memory?
				//cout << "in memory\n";
				ind.printIndex();
			}
			else
			{	Table tmp = myCatalog.GetTable(parsed->print_index->table);
				int test= ind.getAttrNum(parsed->print_index->index, parsed->print_index->table);
				if(ind.readIndex(table, test))
				{	
					ind.printIndex();
				}
				else
				{
					cout << "Error: Index " << index << " does not exist on table " << table << "\n";
				}
			}
		}
		else
		{	
			Table tmp = myCatalog.GetTable(parsed->print_index->table);
			int test= ind.getAttrNum(parsed->print_index->index, parsed->print_index->table);
		
			//need to find the key!//keynum//keyname
			if(ind.readIndex(parsed->print_index->table, test))
			{	
				ind.printIndex();
			}
			else
			{
					cout << "Error: Index " << parsed->print_index->index << " does not exist on table " << parsed->print_index->table << "\n";
			}
			
			

		}
	
	}
	if(timer==1 && timerop==0 && parsed->parameterless!=CMD_EXIT)
	{		gettimeofday(&end, NULL);
			cout << "Operation took " << (end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec) << " μs\n";
			
			
	}
	else if(timerop==1)
	{
		timerop=0;
	}
	 myCatalog.WriteCatalog();

	 //dispatch_print(parsed);
	 if(parsed->parameterless == CMD_EXIT){
	    if(timer==1)
	    {
	       gettimeofday(&endGlobal, NULL);
	       cout << "Global clock time " 
		   << (endGlobal.tv_sec * 1000000 + end.tv_usec) - (startGlobal.tv_sec * 1000000 + startGlobal.tv_usec) << " μs\n";
	    }
	    stillrunning=0;
	    //cout << "quit commit!\n";
	    bMan.commit();
	    bMan.~bManager();
	    //cout << "before writeloaded\n";
	    writeLoaded(myCatalog);
	
	 }
      }else{
	 /* There was a syntax error, and the parser has already 
	  * printed an error message, so nothing to do here.*/
      }
      free(input);            input=NULL;
      free_statement(parsed); parsed=NULL;
   }
}

char* myreadline(char* prompt){
   char* input=NULL;
   size_t inputlength=0;
   printf("%s",prompt);
   if (getline(&input,&inputlength,stdin)!=-1)
      return input;
   else
      return input;
}

void select_star_unconditional(Table tmp, int id, string s, int maxblockid,  bManager& bMan)
{
		

		int* mem;
		cout << "select uncond. function\n";
		cout << "Table:\t" << s << "\n------\n";
		        
		 if(maxblockid!=0)
		  {
			  for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
			  {	
				cout << tmp.GetAttributeName(j) << "\t";

			  }
			  cout << "\n\n";
			  for(int blockid=1; blockid<=maxblockid; ++blockid)
		          {	

				mem = bMan.getBlock(id, s, blockid);
				
				if(!mem)
				{	
					break;
				}
				
						
				int format=1;
				for(int i = 0; i < 12; i++)
				{
					if(mem[i] != -1)
					{		
			 			cout << mem[i] << "\t";
						format++;	
						if(format==tmp.Size()+1)
						{
							cout << "\n";
							format=1;							
						}
									
						//cout << "i " << i << "size " << tmp.Size() << " \n";
								
									
					}	
										
				}
				
			  }	
		  }	//end maxblock!=0
	

}


void select_star_conditional_one_value(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, Catalog& myCatalog)	
{

	
	//string s = tmp.GetTableName();
 	
	int* mem;

						

	//testing readindex		
	condition_t* cond = parsed->select->conditions;
	char* n = cond->left_col;
	cond = parsed->select->conditions;
	writeLoaded(myCatalog); 	 
	if(ind.readIndex(parsed->select->table, tmp.GetAttributeNum(n)))
	{

		if(!ind.getDups() && parsed->select->conditions->op==0)///no duplicates, ==
		{
			//lookup
			int blockid;

		 	blockid = ind.lookupNoDups(cond->right_num);

			//cout << "block id " << blockid << "\n";
			//cout << "no dups!\n";
			//cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
			id_list_t* cols = parsed->select->fields;
			c = parsed->select->conditions;
			num = tmp.GetAttributeNum(c->left_col); 	
			for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
			{	
				//cout << tmp.GetAttributeName(j) << "\t";

			}
			//cout << "\n\n";
			if(blockid == -1)
			{  return; }
			mem = bMan.getBlock(id, s, blockid);
										
			for(int i=0; i<12;  i=i+siz)
			{	
				format=1;
				if(mem[i+num]==c->right_num)
				{
											
					for(int j=i; j<i+siz; j++)
					{ 
						//cout <<  mem[j] << "\t";
						format++;	
						if(format==tmp.Size()+1)
						{
							//cout << "\n";
							format=1;							
						}
					}
				}
					

			}

		}
		else  ///duplicates or non-duplicate range queries!
		{

			//cout << "dups!\n";
			deque<int> blocks;
			int i = cond->right_num;

			

			if(parsed->select->conditions->op==0)
			{
				ind.lookupDups(i, blocks);
			}	
			else if(parsed->select->conditions->op==1)
			{
				//cout << "getting here <\n";
				ind.lookup(i, 3, blocks);
			}	
			else if(parsed->select->conditions->op==2)
			{
				//cout << "getting here <=\n";
				ind.lookup(i, 4, blocks);
	
			}
			else if(parsed->select->conditions->op==3)
			{
				//cout << "here! >\n";
				ind.lookup(i, 1, blocks);

			}
			else if(parsed->select->conditions->op==4)
			{
				//cout <<"here >=\n";
				ind.lookup(i, 2, blocks);
			}
	
			//cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
			id_list_t* cols = parsed->select->fields;
			for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
			{	
				//cout << tmp.GetAttributeName(j) << "\t";

			}
			//cout << "\n\n";
			
			if(blocks.size() == 1)
			{
				
				mem = bMan.getBlock(id, s, blocks.front());
						
				id_list_t* cols = parsed->select->fields;
				c = parsed->select->conditions;
				num = tmp.GetAttributeNum(c->left_col); 	
				if(parsed->select->conditions->op==0)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						if(mem[i+num]==c->right_num)
						{
											
							for(int j=i; j<i+siz; j++)
							{ 
								//cout <<  mem[j] << "\t";
								format++;	
								if(format==tmp.Size()+1)
								{
									//cout << "\n";
									format=1;							
								}
							}
						}
					

					}
				}
				else if(parsed->select->conditions->op==1)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						if(mem[i+num]<c->right_num)
						{
											
							for(int j=i; j<i+siz; j++)
							{ 
								//cout <<  mem[j] << "\t";
								format++;	
								if(format==tmp.Size()+1)
								{
									//cout << "\n";
									format=1;							
								}
							}
						}
					

					}


				}
				else if(parsed->select->conditions->op==2)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						if(mem[i+num]<=c->right_num)
						{
											
							for(int j=i; j<i+siz; j++)
							{ 
								//cout <<  mem[j] << "\t";
								format++;	
								if(format==tmp.Size()+1)
								{
									//cout << "\n";
									format=1;							
								}
							}
						}
					

					}


				}
				else if(parsed->select->conditions->op==3)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						if(mem[i+num]>c->right_num)
						{
											
							for(int j=i; j<i+siz; j++)
							{ 
								//cout <<  mem[j] << "\t";
								format++;	
								if(format==tmp.Size()+1)
								{
									//cout << "\n";
									format=1;							
								}
							}
						}
					

					}


				}
				else if(parsed->select->conditions->op==4)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						if(mem[i+num]>=c->right_num)
						{
											
							for(int j=i; j<i+siz; j++)
							{ 
								//cout <<  mem[j] << "\t";
								format++;	
								if(format==tmp.Size()+1)
								{
									//cout << "\n";
									format=1;							
								}
							}
						}
					

					}


				}
			}
			else
			{

				list<int> scanned;
							   
				while(blocks.size()>0)
				{
					int prev = blocks.front();
					int num = 0;
					
					//cout << "here!\n";
					//cout << "size of blocks " << blocks.size()<<"\n";
					//cout << "block " << blocks.front() << "\n";

					for(list<int>::iterator it = scanned.begin(); it != scanned.end(); it++)
					{
						//cout << "getting into while\n";
						//cout << "block in list " << *it <<"\n";
						if(*it == blocks.front())
						{
							blocks.pop_front();
						}
					}
					
					//while(blocks.front() == prev)
					//{
					
						//cout << "block " << blocks.front() << "\n";
					
					
						mem = bMan.getBlock(id, s, blocks.front());
				
						c = parsed->select->conditions;
						num = tmp.GetAttributeNum(c->left_col); 
						if(parsed->select->conditions->op==0)
						{								
							for(int i=0; i<12;  i=i+siz)
							{	
								format=1;
								if(mem[i+num]==c->right_num)
								{
											
									for(int j=i; j<i+siz; j++)
									{ 
										//cout <<  mem[j] << "\t";
										format++;	
										if(format==tmp.Size()+1)
										{
											//cout << "\n";
										format=1;							
										}
									}
								}
					

							}
						}
						else if(parsed->select->conditions->op==1)
						{			
							//cout << "getting into <\n";					
							for(int i=0; i<12;  i=i+siz)
							{	
								format=1;
								//cout << "mem[i+num] " << mem[i+num] <<"\n";
								if(mem[i+num]<c->right_num && mem[i+num]!=-1)
								{
											
									for(int j=i; j<i+siz; j++)
									{ 
										//cout <<  mem[j] << "\t";
										format++;	
										if(format==tmp.Size()+1)
										{
											//cout << "\n";
										format=1;							
										}
									}
								}
					

							}
						}
						else if(parsed->select->conditions->op==2)
						{								
							for(int i=0; i<12;  i=i+siz)
							{	
								format=1;
								if(mem[i+num]<=c->right_num  && mem[i+num]!=-1)
								{
											
									for(int j=i; j<i+siz; j++)
									{ 
										//cout <<  mem[j] << "\t";
										format++;	
										if(format==tmp.Size()+1)
										{
											//cout << "\n";
										format=1;							
										}
									}
								}
					

							}
						}
						else if(parsed->select->conditions->op==3)
						{								
							for(int i=0; i<12;  i=i+siz)
							{	
								format=1;
								if(mem[i+num]>c->right_num && mem[i+num]!=-1)
								{
											
									for(int j=i; j<i+siz; j++)
									{ 
										//cout <<  mem[j] << "\t";
										format++;	
										if(format==tmp.Size()+1)
										{
											//cout << "\n";
										format=1;							
										}
									}
								}
					

							}
						}
						else if(parsed->select->conditions->op==4)
						{		
							for(int i=0; i<12;  i=i+siz)
							{	
								format=1;
								if(mem[i+num]>=c->right_num && mem[i+num]!=-1)
								{
											
									for(int j=i; j<i+siz; j++)
									{ 
										//cout <<  mem[j] << "\t";
										format++;	
										if(format==tmp.Size()+1)
										{
											//cout << "\n";
										format=1;							
										}
									}
								}
					

							}
						}

						
						scanned.push_back(blocks.front());
						blocks.pop_front();
						
						//while(blocks.front()==prev && blocks.size() > 0){
						//	blocks.pop_front();
						//}
					//}
				
				
				}
			}
			//cout <<"\n";

		}



	}
	else  //non-indexed
	{	
	
		//cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
		id_list_t* cols = parsed->select->fields;
		for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
		{	
			//cout << tmp.GetAttributeName(j) << "\t";

		}
		//cout << "\n\n";	



		if(parsed->select->conditions->op==0)
		{				

			
			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{	
				mem = bMan.getBlock(id, s, blockid);
				if(!mem){ break; }
				c = parsed->select->conditions;
				num = tmp.GetAttributeNum(c->left_col); 								
				for(int i=0; i<12;  i=i+siz)
				{	
					format=1;
					if(mem[i+num]==c->right_num  && mem[i+num] !=-1 )
					{
											
						for(int j=i; j<i+siz; j++)
						{ 
							//cout <<  mem[j] << "\t";
							format++;	
							if(format==tmp.Size()+1)
							{
								//cout << "\n";
								format=1;							
							}
						}
					}
					
				}
										
			}
			//cout <<"\n";
		}//if op is =
		else if(parsed->select->conditions->op==1)
		{

			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{	
				mem = bMan.getBlock(id, s, blockid);
				if(!mem){ break; }
				c = parsed->select->conditions;
				num = tmp.GetAttributeNum(c->left_col); 								
				for(int i=0; i<12;  i=i+siz)
				{	
					format=1;
					if(mem[i+num]<c->right_num && mem[i+num] !=-1)
					{
											
						for(int j=i; j<i+siz; j++)
						{ 
							//cout <<  mem[j] << "\t";
							format++;	
							if(format==tmp.Size()+1)
							{
								//cout << "\n";
								format=1;							
							}
						}
					}
					
				}
										
			}
			//cout <<"\n";

		}//if op is <
		else if(parsed->select->conditions->op==2)
		{

			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{	
				mem = bMan.getBlock(id, s, blockid);
				if(!mem){ break; }
				c = parsed->select->conditions;
				num = tmp.GetAttributeNum(c->left_col); 								
				for(int i=0; i<12;  i=i+siz)
				{	
					format=1;
					if(mem[i+num]<=c->right_num && mem[i+num] !=-1)
					{
											
						for(int j=i; j<i+siz; j++)
						{ 
							//cout <<  mem[j] << "\t";
							format++;	
							if(format==tmp.Size()+1)
							{
								//cout << "\n";
								format=1;							
							}
						}
					}
					
				}
										
			}
			//cout <<"\n";

		}//if op is <=
		else if(parsed->select->conditions->op==3)
		{

			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{	
				mem = bMan.getBlock(id, s, blockid);
				if(!mem){ break; }
				c = parsed->select->conditions;
				num = tmp.GetAttributeNum(c->left_col); 								
				for(int i=0; i<12;  i=i+siz)
				{	
					format=1;
					if(mem[i+num]>c->right_num && mem[i+num] !=-1)
					{
											
						for(int j=i; j<i+siz; j++)
						{ 
							//cout <<  mem[j] << "\t";
							format++;	
							if(format==tmp.Size()+1)
							{
								//cout << "\n";
								format=1;							
							}
						}
					}
					
				}
										
			}
			//cout <<"\n";

		}//if op is >
		else if(parsed->select->conditions->op==4)
		{

			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{	
				mem = bMan.getBlock(id, s, blockid);
				if(!mem){ break; }
				c = parsed->select->conditions;
				num = tmp.GetAttributeNum(c->left_col); 								
				for(int i=0; i<12;  i=i+siz)
				{	
					format=1;
					if(mem[i+num]>=c->right_num && mem[i+num] !=-1)
					{
											
						for(int j=i; j<i+siz; j++)
						{ 
							//cout <<  mem[j] << "\t";
							format++;	
							if(format==tmp.Size()+1)
							{
								//cout << "\n";
								format=1;							
							}
						}
					}
					
				}
										
			}
			//cout <<"\n";

		}//if op is >=
		else 
		{

			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{	
				mem = bMan.getBlock(id, s, blockid);
				if(!mem){ break; }
				c = parsed->select->conditions;
				num = tmp.GetAttributeNum(c->left_col); 								
				for(int i=0; i<12;  i=i+siz)
				{	
					format=1;
					if(mem[i+num]!=c->right_num && mem[i+num] !=-1)
					{
											
						for(int j=i; j<i+siz; j++)
						{ 
							//cout <<  mem[j] << "\t";
							format++;	
							if(format==tmp.Size()+1)
							{
								//cout << "\n";
								format=1;							
							}
						}
					}
					
				}
										
			}
			//cout <<"\n";


		}//if op is !=
	}
}



void select_star_conditional_one_column(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format)
{





	cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
	id_list_t* cols = parsed->select->fields;
	//string s = tmp.GetTableName();
 	for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
	{	
		cout << tmp.GetAttributeName(j) << "\t";

	}
	cout << "\n\n";
	int* mem;
	cout << "select * condit one column function!\n";
	int num2;


	//this here
	for(int blockid=1; blockid<=maxblockid; ++blockid)
	{	
		mem = bMan.getBlock(id, s, blockid);
									
		if(!mem){ break; }
		c = parsed->select->conditions;
		num = tmp.GetAttributeNum(c->left_col);
		num2 = tmp.GetAttributeNum(c->right_col);
							
		for(int i=0; i<12;  i=i+siz)
		{	
			format=1;
			if(mem[i+num]==mem[i+num2] && mem[i+num]!=-1 && mem[i+num2]!=-1)
			{
				for(int j=0; j<siz; j++)
				{
					cout <<  mem[i+j] << "\t";
					format++;	
					if(format==tmp.Size()+1)
					{
						cout << "\n";
						format=1;
					}
				}
												
			}
										

		}
										

									

	}



	cout <<"\n";
	
	
}


void select_star_conditional_two_values(Table tmp, int id, string s, int maxblockid, bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, Catalog& myCatalog)
{




	int* mem;
	//cout << "select * cond two values function!\n";


				
	id_list_t* cols = parsed->select->fields;

	//string s = tmp.GetTableName();
	bool index1 = false;
	bool index2 = false;
	int counter2=0;
	char* col1;	
	char* col2;
	int which;
	writeLoaded(myCatalog);
	condition_t* cond = parsed->select->conditions;
	bool indexed=false;
	for(int i=0; i <2; i++)
	{
		if(ind.indexExist(tmp.GetAttributeNum(cond->left_col), parsed->select->table) && i==0)
		{	//cout << "getting into here\n";
			index1 = true;
			
		}
		else if(ind.indexExist(tmp.GetAttributeNum(cond->left_col), parsed->select->table) && i==1)
		{	
			
			index2 = true;	
			
		}
		if(i==0)
		{
			col1 = cond->left_col;
		}
		else
		{
			col2 = cond->left_col;
		}
		cond = cond->next;
	}
	
	if(index1 && index2)
	{
		
		indexed = ind.readIndex(parsed->select->table, tmp.GetAttributeNum(col1));
		which=1;

	}
	else if(index1 && !index2)
	{
		
		indexed = ind.readIndex(parsed->select->table, tmp.GetAttributeNum(col1));
		which=1;
	}
	else if(!index1 && index2)
	{
		cout << "gett " << tmp.GetAttributeNum(col2)<<"\n";
		indexed = ind.readIndex(parsed->select->table, tmp.GetAttributeNum(col2));
		which=2;
	}

	if(indexed)
	{		

		if(!ind.getDups())///no duplicates
		{

			
			//lookup
			
			cond = parsed->select->conditions;
			int blockid;
			if(which==1)
			{
				
				blockid = ind.lookupNoDups(cond->right_num);
			}
			else
			{
				
				cond = cond->next;
				blockid = ind.lookupNoDups(cond->right_num);
			}
			cond = parsed->select->conditions;
			

			cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
			cols = parsed->select->fields;
			//string s = tmp.GetTableName();
			int counter=0;
			for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
			{	
				cout << tmp.GetAttributeName(j) << "\t";
	
			}
			cout << "\n\n";
			int num2[2];
			int col[2];
			int b = 0;
			while(c)
			{
				num2[b] = c->right_num; 	
				col[b] = tmp.GetAttributeNum(c->left_col);
				b++;
				c = c->next;
			}
			if(blockid==-1)
			{  return; }
			mem = bMan.getBlock(id, s, blockid);
			
	
			for(int i=0; i<12;  i=i+siz)
			{	
				format=1;
				if(mem[i+col[0]]==num2[0] && mem[i+col[1]]==num2[1])
				{
											
					for(int j=i; j<i+siz; j++)
					{ 
						cout <<  mem[j] << "\t";
						format++;	
						if(format==tmp.Size()+1)
						{
							cout << "\n";
							format=1;							
						}
					}
				}
										

			}

		

		}
		else //duplicates
		{
			//cout << "getting into duplicates\n";
			cond = parsed->select->conditions;
			deque<int> blocks;
			int blockid;
			if(which==1)
			{
				
				ind.lookupDups(cond->right_num, blocks);
				
			}
			else
			{
				
				cond = cond->next;
				ind.lookupDups(cond->right_num, blocks);
			}

			cond = parsed->select->conditions;
			cout << "Table:\t" << tmp.GetTableName() << "\n------\n";

			cols = parsed->select->fields;
			string s = tmp.GetTableName();
			int counter=0;
			for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
			{	
				cout << tmp.GetAttributeName(j) << "\t";
	
			}
			cout << "\n\n";
			int num2[2];
			int col[2];
			int b = 0;
			while(c)
			{
				num2[b] = c->right_num; 	
				col[b] = tmp.GetAttributeNum(c->left_col);
				b++;
				c = c->next;
			}

			int prev;
			while(blocks.size()>0)
			{
				prev = blocks.front();
				//cout << blocks.front() << " "<< "\n";
				int num = 0;
				while(blocks.front() == prev)
				{


					mem = bMan.getBlock(id, s, blocks.front());
					if(!mem){ break;  }
	
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						//cout << "mem " << mem[i+col[0]] << "num2 " << num2[0] << " num2 " << num2[1] << " mem " << mem[i+col[1]] << "\n";
						if(mem[i+col[0]]==num2[0] && mem[i+col[1]]==num2[1])
						{
											
							for(int j=i; j<i+siz; j++)
							{ 
								cout <<  mem[j] << "\t";
								format++;	
								if(format==tmp.Size()+1)
								{
									cout << "\n";
									format=1;							
								}
							}
						}
										
	
					}


				blocks.pop_front();
				while(blocks.front()==prev && blocks.size()>0){
					//cout << "popping, size " << blockcount << " block " << blocks.front() << " prev " << prev <<"\n";
					blocks.pop_front();
				}
			}
				

		}
	}

	}
	else  //non-indexed
	{


		cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
		cols = parsed->select->fields;
		//string s = tmp.GetTableName();
		int counter=0;
		for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
		{	
			cout << tmp.GetAttributeName(j) << "\t";
	
		}
		cout << "\n\n";
		int num2[2];
		int col[2];
		int b = 0;
		while(c)
		{
			num2[b] = c->right_num; 	
			col[b] = tmp.GetAttributeNum(c->left_col);
			b++;
			c = c->next;
		}


		
		for(int blockid=1; blockid<=maxblockid; ++blockid)
		{	
			mem = bMan.getBlock(id, s, blockid);
			if(!mem){ break;  }
			for(int i=0; i<12;  i=i+siz)
			{	
				format=1;
				if(mem[i+col[0]]==num2[0] && mem[i+col[1]]==num2[1])
				{
										
					for(int j=i; j<i+siz; j++)
					{ 
						cout <<  mem[j] << "\t";
						format++;	
						if(format==tmp.Size()+1)
						{
							cout << "\n";
							format=1;							
						}
					}
				}
										

			}
								
		}
		
	


	}
	cout <<"\n";
	
}




void select_star_conditional_two_columns(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format)
{

	cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
	id_list_t* cols = parsed->select->fields;
	//string s = tmp.GetTableName();
	int counter=0;
  	for(list<string>::iterator j = tmp.AttributesBegin(); j != tmp.AttributesEnd(); j++)
	{	
		cout << tmp.GetAttributeName(j) << "\t";

	}
	cout << "\n\n";
	
	int* mem;
	//cout << "* 2 cond, columns function!\n";
	for(int blockid=1; blockid<=maxblockid; ++blockid)
	{	
		mem = bMan.getBlock(id, s, blockid);
		if(!mem) { break; }
		c = parsed->select->conditions;
		int num = tmp.GetAttributeNum(c->left_col);
		int num2 = tmp.GetAttributeNum(c->right_col);
		for(int i=0; i<12;  i=i+siz)
		{	
			format=1;
			if(mem[i+num]==mem[i+num2] && mem[i+num]!=-1 && mem[i+num2]!=-1)
			{
				for(int j=i; j<i+siz; j++)
				{ 
					cout <<  mem[j] << "\t";
					format++;	
					if(format==tmp.Size()+1)
					{
						cout << "\n";
						format=1;			
					}
				}
			}
										

		}
	
	}
	cout <<"\n";
	
}

void select_list_unconditional(Table tmp, int id, string s, int maxblockid, bManager& bMan, statement_t* parsed, int num, int siz, int format, int counter)
{

	cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
	id_list_t* cols = parsed->select->fields;
	//string s = tmp.GetTableName();
	int counter2=0;
	while(cols)
	{
		cout << cols->id << "\t";
		cols=cols->next;
		counter2++;							
	}
	cout <<"\n\n";



	int* mem;
	//cout << "select x,x unconditional function!\n";				
	for(int blockid=1; blockid<=maxblockid; ++blockid)
	{
		mem = bMan.getBlock(id, s, blockid);
		if(!mem) { break; }
 		id_list_t* attr = parsed->select->fields;
		
	

		
		for(int i=0; i<12;  i=i+siz)
		{	
			format=1;
			num = tmp.GetAttributeNum(attr->id);
			if(mem[i]!=-1)
			{
				
				while(attr)
		     		{		
					num = tmp.GetAttributeNum(attr->id);
								
					format++;				
					cout << mem[i+num] << "\t";
					if(format==counter+1)
					{
						cout << "\n";
						format=1;				
					}
				
					attr=attr->next;	
		       		}
				attr = parsed->select->fields;
				
		       }
			      		
		}
											
		
		
									
	}
 	cout <<"\n";
	
}


void select_list_conditional_one_value(Table tmp, int id, string s, int maxblockid, bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, Catalog& myCatalog)
{




	int* mem;
	//cout << "select x,x one cond, value function!\n";	

	
	

	//testing readindex		
	condition_t* cond = parsed->select->conditions;
	char* n = cond->left_col;
	cond = parsed->select->conditions;
	writeLoaded(myCatalog);
	if(ind.readIndex(parsed->select->table, tmp.GetAttributeNum(n)))
	{

		if(!ind.getDups())///no duplicates
		{
			//lookup
			int blockid = ind.lookupNoDups(cond->right_num);
			c = parsed->select->conditions;
 			id_list_t* attr = parsed->select->fields;
			int num2 =  tmp.GetAttributeNum(c->left_col);
			cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
			id_list_t* cols = parsed->select->fields;
			//string s = tmp.GetTableName();
			int counter2=0;
			while(cols)
			{
				cout << cols->id << "\t";
				cols=cols->next;
				counter2++;							
			}
			cout <<"\n\n";
			if(blockid==-1)
			{ return; } 
			mem = bMan.getBlock(id, s, blockid);
			
		

			for(int i=0; i<12;  i=i+siz)
			{	
				format=1;
				num = tmp.GetAttributeNum(attr->id);
				if(mem[i+num]==c->right_num || (num!=num2 && mem[i+num2]==c->right_num))
				{
				
					while(attr)
					{		
						num = tmp.GetAttributeNum(attr->id);		
						format++;				
						cout << mem[i+num] << "\t";
						if(format==counter+1)
						{
							cout << "\n";
							format=1;				
						}
				
						attr=attr->next;	
					}
					attr = parsed->select->fields;
				
			      	}
			      		
			}
											

		}
		else	//duplicates
		{

			c = parsed->select->conditions;
 			
			int num2 =  tmp.GetAttributeNum(c->left_col);
	
			deque<int> blocks;
			int i = cond->right_num;
			ind.lookupDups(i, blocks);
			
			
			cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
			id_list_t* cols = parsed->select->fields;
			//string s = tmp.GetTableName();
			int counter2=0;
			while(cols)
			{
				cout << cols->id << "\t";
				cols=cols->next;
				counter2++;							
			}
			cout <<"\n\n";
			id_list_t* attr = parsed->select->fields;
			while(blocks.size()>0)
			{
				int prev = blocks.front();
				//cout << blocks.front() << " " << curr << "\n";
				int num = 0;
				while(blocks.front() == prev)
				{
					
					mem = bMan.getBlock(id, s, blocks.front());
					//cout << "block " << blocks.front() << "\n";		
			
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						num = tmp.GetAttributeNum(attr->id);
						if(mem[i+num2]==c->right_num || (num!=num2 && mem[i+num2]==c->right_num) )
						{	
							while(attr)
					     		{	
								
								num = tmp.GetAttributeNum(attr->id);
								//cout << "mem[i+num] " << mem[i+num] << " mem[i+num2] " << mem[i+num2] << "\n";	
								//cout << "attr id " << num << "\n";
								format++;				
								cout << mem[i+num] << "\t";
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
						
								attr=attr->next;	
					       		}
							attr = parsed->select->fields;
					
			      			}
			      		
					}
					blocks.pop_front();
					while(blocks.front()==prev && blocks.size()>0){
						blocks.pop_front();
						cout << "front " << blocks.front()<<"\n";
					}
				}
				
				
			}						
			






		}


	}
	else //non-indexed
	{

		cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
		id_list_t* cols = parsed->select->fields;
		//string s = tmp.GetTableName();
		int counter2=0;
		while(cols)
		{
			cout << cols->id << "\t";
			cols=cols->next;
			counter2++;							
		}
		cout <<"\n\n";		

		if(parsed->select->conditions->op==0)
		{
			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{ 
				mem = bMan.getBlock(id, s, blockid);
				if(!mem) { break; }
				c = parsed->select->conditions;
 				id_list_t* attr = parsed->select->fields;
				int num2 =  tmp.GetAttributeNum(c->left_col);
	
				//cout << "counter " << counter << "\n";
							  
				if(!c->right_col)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						num = tmp.GetAttributeNum(attr->id);
						if((mem[i+num2]==c->right_num || (num!=num2 && mem[i+num2]==c->right_num)) && mem[i+num2] !=-1)
						{
					
							while(attr)
					     		{		
								num = tmp.GetAttributeNum(attr->id);
									
								format++;				
								cout << mem[i+num] << "\t";
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
					
								attr=attr->next;	
					       		}
							attr = parsed->select->fields;
				
				      		}
			      		
					}
											
				}
		
			}
		}//if op is =
		else if(parsed->select->conditions->op==1)
		{
			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{ 
				mem = bMan.getBlock(id, s, blockid);
				if(!mem) { break; }
				c = parsed->select->conditions;
 				id_list_t* attr = parsed->select->fields;
				int num2 =  tmp.GetAttributeNum(c->left_col);
	
				//cout << "counter " << counter << "\n";
							  
				if(!c->right_col)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						num = tmp.GetAttributeNum(attr->id);
						if((mem[i+num2]<c->right_num || (num!=num2 && mem[i+num2]==c->right_num)) && mem[i+num2] !=-1)
						{
					
							while(attr)
					     		{		
								num = tmp.GetAttributeNum(attr->id);
									
								format++;				
								cout << mem[i+num] << "\t";
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
					
								attr=attr->next;	
					       		}
							attr = parsed->select->fields;
				
				      		}
			      		
					}
											
				}
		
			}
		}//if op is <
		else if(parsed->select->conditions->op==2)
		{
			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{ 
				mem = bMan.getBlock(id, s, blockid);
				if(!mem) { break; }
				c = parsed->select->conditions;
 				id_list_t* attr = parsed->select->fields;
				int num2 =  tmp.GetAttributeNum(c->left_col);
	
				//cout << "counter " << counter << "\n";
							  
				if(!c->right_col)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						num = tmp.GetAttributeNum(attr->id);
						if((mem[i+num2]<=c->right_num || (num!=num2 && mem[i+num2]==c->right_num)) && mem[i+num2] !=-1)
						{
					
							while(attr)
					     		{		
								num = tmp.GetAttributeNum(attr->id);
									
								format++;				
								cout << mem[i+num] << "\t";
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
					
								attr=attr->next;	
					       		}
							attr = parsed->select->fields;
				
				      		}
			      		
					}
											
				}
		
			}
		}//if op is <=
		else if(parsed->select->conditions->op==3)
		{
			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{ 
				mem = bMan.getBlock(id, s, blockid);
				if(!mem) { break; }
				c = parsed->select->conditions;
 				id_list_t* attr = parsed->select->fields;
				int num2 =  tmp.GetAttributeNum(c->left_col);
	
				//cout << "counter " << counter << "\n";
							  
				if(!c->right_col)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						num = tmp.GetAttributeNum(attr->id);
						if((mem[i+num2]>c->right_num || (num!=num2 && mem[i+num2]==c->right_num)) && mem[i+num2] !=-1)
						{
					
							while(attr)
					     		{		
								num = tmp.GetAttributeNum(attr->id);
									
								format++;				
								cout << mem[i+num] << "\t";
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
					
								attr=attr->next;	
					       		}
							attr = parsed->select->fields;
				
				      		}
			      		
					}
											
				}
		
			}
		}//if op is >
		else if(parsed->select->conditions->op==4)
		{
			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{ 
				mem = bMan.getBlock(id, s, blockid);
				if(!mem) { break; }
				c = parsed->select->conditions;
 				id_list_t* attr = parsed->select->fields;
				int num2 =  tmp.GetAttributeNum(c->left_col);
	
				//cout << "counter " << counter << "\n";
							  
				if(!c->right_col)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						num = tmp.GetAttributeNum(attr->id);
						if((mem[i+num2]>=c->right_num || (num!=num2 && mem[i+num2]==c->right_num)) && mem[i+num2] !=-1)
						{
					
							while(attr)
					     		{		
								num = tmp.GetAttributeNum(attr->id);
									
								format++;				
								cout << mem[i+num] << "\t";
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
					
								attr=attr->next;	
					       		}
							attr = parsed->select->fields;
				
				      		}
			      		
					}
											
				}
		
			}
		}//if op is >=
		else
		{
			for(int blockid=1; blockid<=maxblockid; ++blockid)
			{ 
				mem = bMan.getBlock(id, s, blockid);
				if(!mem) { break; }
				c = parsed->select->conditions;
 				id_list_t* attr = parsed->select->fields;
				int num2 =  tmp.GetAttributeNum(c->left_col);
	
				//cout << "counter " << counter << "\n";
							  
				if(!c->right_col)
				{
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						num = tmp.GetAttributeNum(attr->id);
						if((mem[i+num2]!=c->right_num || (num!=num2 && mem[i+num2]==c->right_num)) && mem[i+num2] !=-1)
						{
					
							while(attr)
					     		{		
								num = tmp.GetAttributeNum(attr->id);
									
								format++;				
								cout << mem[i+num] << "\t";
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
					
								attr=attr->next;	
					       		}
							attr = parsed->select->fields;
				
				      		}
			      		
					}
											
				}
		
			}
		}//if op is !=

						
	}
 	cout <<"\n";
	
}

void select_list_conditional_one_column(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, int num2)
{
	
	int* mem;
	cout << "select x,x cond one column function!\n";


	cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
	id_list_t* cols = parsed->select->fields;
	//string s = tmp.GetTableName();
	int counter2=0;
	while(cols)
	{
		cout << cols->id << "\t";
		cols=cols->next;
		counter2++;							
	}
	cout <<"\n\n";


	for(int blockid=1; blockid<=maxblockid; ++blockid)
	{	
		mem = bMan.getBlock(id, s, blockid);
																	
		if(!mem){ break; }
		c = parsed->select->conditions;
		id_list_t* attr = parsed->select->fields;
		num = tmp.GetAttributeNum(c->left_col);
		num2 = tmp.GetAttributeNum(c->right_col);
		int num3;
				
		for(int i=0; i<12;  i=i+siz)
		 {
						
		
			format=1;
			if(mem[i+num]==mem[i+num2] && mem[i+num]!=-1 && mem[i+num2]!=-1)
			{
					while(attr)
			     		{		
						num3 = tmp.GetAttributeNum(attr->id);
									
						format++;				
						cout << mem[i+num] << "\t";
						if(format==counter+1)
						{
							cout << "\n";
							format=1;				
						}
					
						attr=attr->next;	
			       		}
					attr = parsed->select->fields;		     	
											
			}
										

		  }
										

									

	}
	cout <<"\n";


}


void select_list_conditional_two_values(Table tmp, int id, string s, int maxblockid, bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, int num2, int count, Catalog& myCatalog)
{





	int* mem;
	cout << "select x,x cond two values function!\n";


	writeLoaded(myCatalog);			
	id_list_t* cols = parsed->select->fields;
	//string s = tmp.GetTableName();
	bool index1 = false;
	bool index2 = false;
	int counter2=0;
	char* col1;	
	char* col2;
	int which;
	condition_t* cond = parsed->select->conditions;
	bool indexed=false;
	for(int i=0; i <2; i++)
	{
		if(ind.indexExist(tmp.GetAttributeNum(cond->left_col), parsed->select->table) && i==0)
		{	//cout << "getting into here\n";
			index1 = true;
			
		}
		else if(ind.indexExist(tmp.GetAttributeNum(cond->left_col), parsed->select->table) && i==1)
		{	
			
			index2 = true;	
			
		}
		if(i==0)
		{
			col1 = cond->left_col;
		}
		else
		{
			col2 = cond->left_col;
		}
		cond = cond->next;
	}
	//cout << "index1 " << index1 << " index2 " << index2 << "\n";
	
	if(index1 && index2)
	{
		
		indexed = ind.readIndex(parsed->select->table, tmp.GetAttributeNum(col1));
		which=1;

	}
	else if(index1 && !index2)
	{
		//cout << "in here " << " table " << parsed->select->table << " " <<tmp.GetAttributeNum(col1) << "\n";
		indexed = ind.readIndex(parsed->select->table, tmp.GetAttributeNum(col1));
		which=1;
	}
	else //if(!index1 && index2)
	{
		indexed = ind.readIndex(parsed->select->table, tmp.GetAttributeNum(col2));
		
		which=2;
	}
	//cout << "index1 " << index1 << " index2 " << index2  << " indexed " << indexed <<"\n";
	

	if(indexed)
	{		

		if(!ind.getDups())///no duplicates
		{

			
			//lookup
			
			cond = parsed->select->conditions;
			int blockid;
			if(which==1)
			{
				
				blockid = ind.lookupNoDups(cond->right_num);
			}
			else
			{
				
				cond = cond->next;
				blockid = ind.lookupNoDups(cond->right_num);
			}
			cond = parsed->select->conditions;
			cout << "Table:\t" << tmp.GetTableName() << "\n------\n";
			while(cols)
			{
				cout << cols->id << "\t";
				cols=cols->next;
				counter2++;							
			}
			cout <<"\n\n";
			
			id_list_t* attr = parsed->select->fields;
			int num3;

			if(blockid==-1)
			{ return; }
			mem = bMan.getBlock(id, s, blockid);
			
			c = parsed->select->conditions;
			int num2[2];
			int col[2];
			int b = 0;
			while(c)
			{
				num2[b] = c->right_num; 	
				col[b] = tmp.GetAttributeNum(c->left_col);
				b++;
				c = c->next;
			}
			for(int i=0; i<12;  i=i+siz)
			{	
				format=1;
			
				if(mem[i+col[0]]==num2[0] && mem[i+col[1]]==num2[1])
				{

												
					while(attr)
					{
						num3 = tmp.GetAttributeNum(attr->id);
										
						cout <<  mem[i+num3] << "\t";
						format++;
						if(format==count+1)
						{
							cout << "\n";
							format=1;				
						}
					attr=attr->next;	
					}
				attr = parsed->select->fields;
						
				}
										
			}
				




		}
		else   //duplicates
		{

			
			cond = parsed->select->conditions;
			deque<int> blocks;
			int blockid;
			if(which==1)
			{
				
				ind.lookupDups(cond->right_num, blocks);
			}
			else
			{
				
				cond = cond->next;
				ind.lookupDups(cond->right_num, blocks);
			}

			cond = parsed->select->conditions;
			cout << "Table:\t" << tmp.GetTableName() << "\n------\n";

			while(cols)
			{
				cout << cols->id << "\t";
				cols=cols->next;
				counter2++;							
			}
			cout <<"\n\n";


			/*cout << "size " << blocks.size() << "\n";
			while(blocks.size()>0)
			{
				cout << "block " << blocks.front()<<"\n";
				blocks.pop_front();
			}*/
			
		
			//cout << "back " << blocks.back() << " size " << blocks.size() << "\n";
			int blockcount = blocks.size();
			int prev;
			while(blocks.size()>0)
			{
				prev = blocks.front();
				//cout << "block" <<blocks.front() << " size " <<blocks.size() << " back " << blocks.back() <<"\n";
				int num = 0;
				
				while(blocks.front() == prev)
				{
					id_list_t* attr = parsed->select->fields;
					int num3;
					//cout << "bloc " << blocks.front() << " "<< "\n";

					mem = bMan.getBlock(id, s, blocks.front());
					if(!mem) { break; }
					c = parsed->select->conditions;
					int num2[2];
					int col[2];
					int b = 0;
					while(c)
					{
						num2[b] = c->right_num; 	
						col[b] = tmp.GetAttributeNum(c->left_col);
						b++;
						c = c->next;
					}
					for(int i=0; i<12;  i=i+siz)
					{	
						format=1;
						if(mem[i+col[0]]==num2[0] && mem[i+col[1]]==num2[1])
						{

												
							while(attr)
							{
								num3 = tmp.GetAttributeNum(attr->id);
										
								cout <<  mem[i+num3] << "\t";
								format++;
								if(format==counter2+1)
								{
									cout << "\n";
									format=1;				
								}
							attr=attr->next;	
							}
						attr = parsed->select->fields;
							
						}
										
					}	 				

				

					blocks.pop_front();
					blockcount--;
					while(blocks.front()==prev && blocks.size()>0){
						cout << "popping, size " << blockcount << " block " << blocks.front() << " prev " << prev <<"\n";
						blocks.pop_front();
						blockcount--;
						
					}
				}
				
				
			}



		}
	}
	else
	{

		cout << "Table:\t" << tmp.GetTableName() << "\n------\n";

		while(cols)
		{
			cout << cols->id << "\t";
			cols=cols->next;
			counter2++;							
		}
		cout <<"\n\n";
		
		id_list_t* attr = parsed->select->fields;
		int num3;
	 	for(int blockid=1; blockid<=maxblockid; ++blockid)
		{	
			mem = bMan.getBlock(id, s, blockid);
			if(!mem) { break; }
			c = parsed->select->conditions;
			int num2[2];
			int col[2];
			int b = 0;
			while(c)
			{
				num2[b] = c->right_num; 	
				col[b] = tmp.GetAttributeNum(c->left_col);
				b++;
				c = c->next;
			}
			for(int i=0; i<12;  i=i+siz)
			{	
				format=1;
				if(mem[i+col[0]]==num2[0] && mem[i+col[1]]==num2[1])
				{

												
					while(attr)
					{
						num3 = tmp.GetAttributeNum(attr->id);
										
						cout <<  mem[i+num3] << "\t";
						format++;
						if(format==counter2+1)
						{
							cout << "\n";
							format=1;				
						}
					attr=attr->next;	
					}
				attr = parsed->select->fields;
						
				}
										
			}
								 	
					
		}
	}
	cout << "\n";					
}



void select_list_conditional_two_columns(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, int num2, int count)
{
	int* mem;
	cout << "select x,x 2 cond, columns function!\n";


	cout << "Table:\t" << tmp.GetTableName() << "\n------\n";			
	id_list_t* cols = parsed->select->fields;
	//string s = tmp.GetTableName();
	int counter2=0;
	while(cols)
	{
		cout << cols->id << "\t";
		cols=cols->next;
		counter2++;							
	}
	cout <<"\n\n";

	id_list_t* attr = parsed->select->fields;
   	for(int blockid=1; blockid<=maxblockid; ++blockid)
	{	
		mem = bMan.getBlock(id, s, blockid);
		if(!mem) { break; }
		c = parsed->select->conditions;
		int col2[2];
		int col[2];
		int b = 0;
		while(c)
		{
			col2[b] = tmp.GetAttributeNum(c->right_col); 	
			col[b] = tmp.GetAttributeNum(c->left_col);
			b++;
			c = c->next;
			
		}

		for(int i=0; i<12;  i=i+siz)
		{	
			format=1;
			
		
			if(mem[i+col[0]]==mem[i+col2[0]] && mem[i+col[1]]==mem[i+col2[1]] && mem[i+col[0]]!=-1 && mem[i+col[1]]!=-1)
			{

			
				while(attr)
				{			
					num=tmp.GetAttributeNum(attr->id);				
					cout <<  mem[i+num] << "\t";
					format++;	
					if(format==count+1)
						{
							cout << "\n";
							format=1;				
						}
					attr=attr->next;	
				}
				attr = parsed->select->fields;			
			}
										

		}					 
	}
	cout <<"\n";
	delete [] mem;

}





int checkIndex(char* tableName, char* index, int column)
{

	//check if two indices already exist X
	//then check if there is already index on that attribute X
	//check if attribute exists?
	//then check if there are dups


	//make sure index name is unique
	//you can use strchr() to find the first _, then get index before the next _
	//can use char* tokenizer to break it up at the _s
	

	string s= ind.getIndexName();
	char* test = (char*)malloc( sizeof( char ) *(s.length() +1) );
	strcpy(test, s.c_str());
	
	string tab = ind.getTableName();
	char* test3 = (char*)malloc( sizeof( char ) *(tab.length() +1) );
	strcpy(test3, tab.c_str());
	int col = ind.getAttr();
	if(!ind.getLoaded())
	{
		col=-1;	
	}
	//cout << "col " << col << " column " << column << "\n";
	if(strcmp(tableName,test3)!=0)
	{

	}
	else if (strcmp(test, index)==0)
	{
		cout << "Error: There is already an index " << index << " on table " << tableName << "\n";
		return 1;
	} 
	else if (col == column)
	{

		cout << "Error: There is already an index on that column on table " << tableName << "\n";
		return 1;
	}
	DIR *dp;
  	struct dirent *ep;     
  	dp = opendir ("./base/");
	int count=0;
  	if (dp != NULL)
  	{
		ep = readdir(dp); //skip . and ..
		ep = readdir(dp);
		char* tablePrefix;
		tablePrefix = strcat(tableName, "_");
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
			//cout << "fileprefix " << filePrefix << " length " << strlen(filePrefix) << "\n";
			if(strcmp(tablePrefix, filePrefix)==0)
			{
				count++;
				char* split = strtok(ep->d_name, "_");
				int indexCount=0;
				while(indexCount < 1)
				{
					
					split = strtok(NULL, "_");
					indexCount++;
				}
				//cout << "split " << split << "\n";
				if(strcmp(split,index)==0)
				{
					tableName[strlen(tableName) - 1] = '\0';
					cout << "Error: There is already an index " << index << " on table " << tableName << "\n";
					return 1;
				}
				if(col == column)
				{
					tableName[strlen(tableName) - 1] = '\0';
					cout << "Error:  There is already an index for that column on table " << tableName << "\n";  		
					return 1;				
				}
				
			}
			
		}
		//cout << "count " << count << "\n";
    		(void) closedir (dp);
  	}
  	else
   	{
		tableName[strlen(tableName) - 1] = '\0';
		cout << "Couldn't open the /base directory\n";
	}
	if(count == 2)
	{
		tableName[strlen(tableName) - 1] = '\0';
		cout << "Error:  max limit of indices (2) reached on this table\n";
		return 1;
	}
	tableName[strlen(tableName) - 1] = '\0';

  return 0;

}


void writeLoaded(Catalog& myCatalog)
{

	    //check if index is in memory
	    if(ind.getLoaded())
	    {
	    	string tablename=ind.getTableName();
	    	string indexname =ind.getIndexName();
		
		Table tmp = myCatalog.GetTable(const_cast<char *>(tablename.c_str()));

		char* table = (char*)malloc( sizeof( char ) *(tablename.length() +1) );
		char* index = (char*)malloc( sizeof( char ) *(indexname.length() +1) );
		
	
		strcpy(table, tablename.c_str());
		strcpy(index, indexname.c_str());
		
        	int keyint = ind.getAttr();
		//cout << "writeIndex key: " << keyint << "\n";
		//cout << "writeIndex table: "<< table<<"\n";
	    	ind.writeIndex(table, index);
		
	    }	


}
