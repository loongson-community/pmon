#include <stdio.h>
union data {
	unsigned long data_long;
	struct {
		unsigned char d1, d2, d3, d4;
	}data_char;
}indata;
int main(int argc, char **argv)
{
	FILE *infile;
	FILE *outfile;
	if(argc < 3)return 1;
	infile = fopen(argv[1],"rb");
	if(!infile){
		printf("Cannot open input file \n");
		return 2;
	}
	outfile = fopen(argv[2],"w");
	if(!outfile){
		printf("Cannot open output file \n");
		return 3;
	}
	while(fread(&indata, sizeof(indata),1,infile)){
		fprintf(outfile, "%02x %02x %02x %02x\n",
			indata.data_char.d1,
			indata.data_char.d2,
			indata.data_char.d3,
			indata.data_char.d4);
	}	
	fclose(outfile);
	fclose(infile);
}
