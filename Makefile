default: program

program.o: con.c
	gcc -o mt19937ar.o -c mt19937ar.c

program: program.o
	gcc -g con.c mt19937ar.o -o con -lpthread
	rm -f mt19937ar.o