# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
# compilatore
CC=gcc
# flag di compilazione
CFLAGS=-g -Wall -O -std=c99
LDLIBS=-lm -lrt -pthread

# singolo eseguilbile da compilare
MAIN=farm

# chi compilare
all: $(MAIN)

# come compilare main
$(MAIN): $(MAIN).o libreria.o

clean:
	rm -f $(MAIN) *.o  

zip:
	zip $(MAIN).zip makefile *.c *.h *.py *.md *.dat