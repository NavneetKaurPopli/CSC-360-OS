.phony all:
all: sfs

sfs:
	gcc -Wall SFSUtils.c diskinfo.c -o diskinfo
	gcc -Wall SFSUtils.c disklist.c -o disklist
	gcc -Wall SFSUtils.c diskget.c -o diskget
	gcc -Wall SFSUtils.c diskput.c -o diskput

.PHONY clean:
clean:
	-rm -rf diskinfo
	-rm -rf disklist
	-rm -rf diskget
	-rm -rf diskput