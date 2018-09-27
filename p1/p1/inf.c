#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main(int argc, char* argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: inf tag interval\n");
	} else {
	    FILE *fp;
	    fp = fopen("infprint.txt", "w+");
		const char* tag = argv[1];
		int interval = atoi(argv[2]);
		while(1) {
			fprintf(fp, "%s\n", tag);
			sleep(interval);
		}
//		fclose(fp);
	}
}

