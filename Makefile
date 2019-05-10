CC=gcc
CFLAGS=-I.
DEPS = parse.h jobcontrol.h debug.h
OBJ = yash.o parse.o jobcontrol.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

yash: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	