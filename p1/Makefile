.phony all:
all: PMan inf

PMan: PMan.c ProcNode.c PManUtils.c
	gcc -Wall PMan.c ProcNode.c PManUtils.c -lreadline -lhistory -o PMan
inf: inf.c
	gcc inf.c -o inf

.PHONY clean:
clean:
	-rm -rf *.o *.exe