CC=gcc
CFLAGS=-I. -Wall -Werror
DEPS=
OBJ=http.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: main

main: $(OBJ)
	$(CC) -o main $^ $(CFLAGS)

clean:
	rm -rf *.o main


