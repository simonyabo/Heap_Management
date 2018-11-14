CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g -lm
OBJECTS = quickfit.o firstfitmodule.o HeapTestEngine.o
EXES = HeapTest

HeapTest:	$(OBJECTS)
	$(CC) -o HeapTest $(LDFLAGS) $(OBJECTS)


all:
	$(MAKE) $(EXES)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f firstfit.o
