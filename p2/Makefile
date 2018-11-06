.phony all:
all: ACS 

ACS: ACS.c QueueNode.c
	gcc -Wall ACS.c QueueNode.c -lpthread  -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe