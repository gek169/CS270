#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char** argv){
	if(argc < 3) printf("Usage: t2a Infile.txt Outfile.txt\n");
	FILE* ifl, * ofl; char buffer[2048]; int i = 0, charcount = 0;
	ifl = fopen(argv[1],"r");
	if(ifl == NULL) exit(1);
	ofl = fopen(argv[2],"w");
	if(ofl == NULL) exit(1);
	while(NULL != fgets(buffer, 2048, ifl)) {
		int len = strlen(buffer);
		for(i = 0; i < len && i < 2048; i++){
			fprintf(ofl, "%d\t",buffer[i]); charcount++;
			if(charcount%4 == 0) fprintf(ofl, "\n");
		}
		/*fprintf(ofl, "\n0x%x\n",'\n');*/      /* make sure to put in newlines.*/
	}
	fclose(ifl);fclose(ofl); return 0;
}