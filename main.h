//main.h
#ifndef MAIN_H
#define MAIN_H
#include "bManager.h"
#include "parser.h"
#include "bIndex.h"


//select functions
void select_star_unconditional(Table tmp, int id, string s, int maxblockid, bManager& bMan);
void select_star_conditional_one_value(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, Catalog& myCatalog);
void select_star_conditional_one_column(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format);
void select_star_conditional_two_values(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, Catalog& myCatalog);
void select_star_conditional_two_columns(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format);
void select_list_unconditional(Table tmp, int id, string s, int maxblockid,  bManager& bMan, statement_t* parsed, int num, int siz, int format, int counter);
void select_list_conditional_one_value(Table tmp, int id, string s, int maxblockid, bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, Catalog& myCatalog);
void select_list_conditional_one_column(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, int num2);
void select_list_conditional_two_values(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, int num2, int count, Catalog& myCatalog);
void select_list_conditional_two_columns(Table tmp, int id, string s, int maxblockid,  bManager& bMan, condition_t* c, statement_t* parsed, int num, int siz, int format, int counter, int num2, int count);
void select_create_index(char* tablename, int attr, int (*array)[2], int size);
int checkIndex(char* tableName, char* index, int column);
void writeLoaded(Catalog& mycatalog);







#endif
