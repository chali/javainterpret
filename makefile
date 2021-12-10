CC = g++
CFLAGS = -Wall

jaint: main.o program.o command.o expression.o node.o lexan.o
	$(CC) $(CFLAGS) -o $@ main.o program.o command.o expression.o node.o lexan.o

main.o: main.cpp program.h exception.h
program.o: program.cpp program.h exception.h
command.o: command.cpp program.h exception.h
expression.o: expression.cpp program.h exception.h
node.o: node.cpp program.h exception.h
lexan.o: lexan.cpp lexan.h exception.h

clean: 
	rm -f jaint main.o program.o lexan.o command.o expression.o node.o
