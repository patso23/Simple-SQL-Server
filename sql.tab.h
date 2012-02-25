
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SELECT = 258,
     FROM = 259,
     WHERE = 260,
     AND = 261,
     INSERT = 262,
     INTO = 263,
     VALUES = 264,
     CREATE = 265,
     TABLE = 266,
     PRINT = 267,
     CATALOG = 268,
     BUFFER = 269,
     HIT = 270,
     RATE = 271,
     SET = 272,
     DROP = 273,
     TIMER = 274,
     INDEX = 275,
     NO = 276,
     DUPLICATES = 277,
     OF = 278,
     ON = 279,
     OFF = 280,
     DEBUG = 281,
     COMMIT = 282,
     EXIT = 283,
     OPERATOR = 284,
     IDENTIFIER = 285,
     NUMBER = 286
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 43 "sql.y"

   char* string_v;
   int int_v;
   struct id_list_t* id_list;
   struct num_list_t* num_list;
   enum operator_t operator_v;
   struct condition_t* condition;
   struct select_statement_t* select_statement;
   struct create_table_statement_t* create_table_statement;
   struct create_index_statement_t* create_index_statement;
   struct index_ref_t* index_ref;
   struct insert_statement_t* insert_statement;
   enum variable_t variable;
   struct set_statement_t* set_statement;
   enum parameterless_statement_t parameterless_statement;
   struct statement_t* statement;



/* Line 1676 of yacc.c  */
#line 103 "sql.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


