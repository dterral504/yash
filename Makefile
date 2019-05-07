CC=gcc
CFLAGS=-I.
DEPS = parse.h
OBJ = yash.o parse.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

yash: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	