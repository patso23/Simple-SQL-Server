#include "print.h"
#include "stdio.h"

void print_id_list(id_list_t* i){
   while(i){
      printf("%s",i->id);
      if (i->next) printf(", ");
      i=i->next;
   }
}
void print_num_list(num_list_t* i){
   while(i){
      printf("%d",i->num);
      if (i->next) printf(", ");
      i=i->next;
   }
}
void print_condition(condition_t* i){
   while(i){
      printf("%s ", i->left_col);
      switch(i->op){
	 case OP_EQUAL: printf("="); break;
	 case OP_LESS: printf("<"); break;
	 case OP_LESS_EQUAL: printf("<="); break;
	 case OP_GREATER: printf(">"); break;
	 case OP_GREATER_EQUAL: printf(">="); break;
	 case OP_NOT_EQUAL: printf("!="); break;
      }
      if (i->right_col)
	 printf (" %s", i->right_col);
      else
	 printf (" %d", i->right_num);
      if (i->next) printf(" and ");
      i=i->next;
   }
}
void print_select_statement(select_statement_t* i){
   printf("select ");
   if(i->fields)
      print_id_list(i->fields);
   else
      printf("*");
   printf(" from %s",i->table);
   if (i->conditions){
      printf(" where ");
      print_condition(i->conditions);
   }
   printf(";\n");
}
void print_insert_statement(insert_statement_t* i){
   printf("insert into %s values (", i->table);
   print_num_list(i->values);
   printf(");\n");
   
}
void print_create_table_statement(create_table_statement_t* i){
   printf("create table %s (", i->table);
   print_id_list(i->columns);
   printf(");\n");
}
void print_set_statement(set_statement_t* i){
   printf("set ");
   switch (i->variable){
      case CONFIG_TIMER: printf("timer "); break;
      case CONFIG_INDEX_DEBUG: printf("index debug "); break;
   }
   printf("%s;\n", i->value ? "on" : "off" );
}
void dispatch_print(statement_t* parsed){
	 /* parse succeded. determine which command was run */
	 if(parsed->select){
	    print_select_statement(parsed->select);
	 }
	 if(parsed->insert){
	    print_insert_statement(parsed->insert);
	 }
	 if(parsed->create_table){
	    print_create_table_statement(parsed->create_table);
	 }
	 if(parsed->create_index){
	    printf("create index %s on %s (%s);\n",
		  parsed->create_index->index,
		  parsed->create_index->table,
		  parsed->create_index->column);
	 }
	 if(parsed->drop_table){
	    printf("drop table %s;\n",parsed->drop_table);
	 }
	 if(parsed->drop_index){
	    printf("drop index %s of table %s;\n",
		  parsed->drop_index->index,
		  parsed->drop_index->table);
	 }
	 if(parsed->print_table){
	    printf("print table %s;\n",parsed->print_table);
	 }
	 if(parsed->print_index){
	    printf("print index %s of table %s;\n",
		  parsed->print_index->index,
		  parsed->print_index->table);
	 }
	 if(parsed->set){
	    print_set_statement(parsed->set);
	 }
	 if(parsed->parameterless == CMD_PRINT_CATALOG)
	    printf("print catalog;\n");
	 if(parsed->parameterless == CMD_PRINT_BUFFER)
	    printf("print buffer;\n");
	 if(parsed->parameterless == CMD_PRINT_HIT_RATE)
	    printf("print hit rate;\n");
	 if(parsed->parameterless == CMD_COMMIT)
	    printf("commit;\n");
	 if(parsed->parameterless == CMD_EXIT){
	    printf("exit;\n");
	 }
}
