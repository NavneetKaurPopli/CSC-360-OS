.phony all:
all: PMan

PMan: PMan.c
	gcc -Wall PMan.c -lreadline -lhistory -o PMan
	gcc inf.c -o inf

.PHONY clean:
clean:
	-rm -rf *.o *.exe