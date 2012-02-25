CFLAGS=-g -std=c99
dbms:	sql.tab.o lex.yy.o main.o parser.o print.o table.o catalog.o bManager.o bPool.o sManager.o node.o bIndex.o
	$(CXX) $(CFLAGS) $^ -o dbms

sql.tab.c sql.tab.h: sql.y
	bison --defines sql.y
lex.yy.c: sql.lex
	flex --header-file=sql.lex.h sql.lex
lex.yy.o: lex.yy.c sql.tab.h
	$(CC) $(CFLAGS) -D_POSIX_SOURCE -c $<
sql.tab.o: sql.tab.c sql.tab.h
main.o: main.cpp main.h sql.tab.h
parser.o: parser.c parser.h
table.o: table.cpp table.h
catalog.o: catalog.cpp catalog.h
bManager.o: bManager.cpp bManager.h
bPool.o: bPool.cpp bPool.h
sManager.o: sManager.cpp sManager.h
node.o: node.cpp node.h
bIndex.o: bIndex.cpp bIndex.h
