.phony all:
all: mts

mts: mts.c
	gcc -o mts -pthread mts.c

.PHONY clean:
clean:
	-rm -rf *.o *.exe
