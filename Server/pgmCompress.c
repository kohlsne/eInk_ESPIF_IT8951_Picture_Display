#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(){
	printf("Running C Code\n");
	printf("test\n");
	int x=0;
	int y=0;
	printf("test1\n");
	FILE *r_ptr;
	FILE *w_ptr;
	r_ptr = fopen("pic.pgm","r");
	w_ptr = fopen("pic.bin","wb");
	if(r_ptr == NULL || w_ptr == NULL){
	  printf("Error!\n");   
	  exit(1);             
	}
	//Parse Header
	char line[256];
	char *token;
	int temp;
	for (uint8_t i = 0; i<3; i++){
	  fgets(line, sizeof(line), r_ptr);
	  printf("%s\n", line);
	  switch(i){
	    case 0:
		    if (line[0] != 'P'  && line[1] != '5'){
			printf("Wrong File Format\n");
			exit(1);
		    }
		    continue;
	    case 1:
		token = strtok(line," ");
	      	x = atoi(token);
	 	printf("%d\n", x);
		token = strtok(NULL, " ");
		y = atoi(token);
		printf("%d\n", y);
	   	continue;
	   case 2:
		temp = atoi(line);
		if (temp != 15){
			printf("Wrong Number of bit depth\n");
			exit(2);
		}
		continue;
	   default:
	   	continue;

	  }
	}
printf("%d\n", y);
printf("%d\n", x);
uint32_t pixelsTotal = y*x;
union Picture {
	//use union
	uint8_t pgm[pixelsTotal];
	uint8_t nibbles[pixelsTotal/2];

} pic;

fread(pic.pgm, sizeof(pic.pgm),1,r_ptr);


for (uint8_t i = 0; i< 20; i++){
	printf("%x\n",pic.pgm[i]);
}
	//Parse Data

	for (uint32_t i = 0; i<pixelsTotal/2; i++){
		pic.pgm[i] = (pic.pgm[i*2] << 4) | pic.pgm[i*2+1];
	}

printf("Process Complete\n");
for (uint8_t i = 0; i< 10; i++){
	printf("%x\n",pic.pgm[i]);
}


fwrite(pic.nibbles,sizeof(pic.nibbles),1, w_ptr);


	fclose(r_ptr);
	fclose(w_ptr);
	printf("eInk Image Format Processing Complete\n");
	return 0;
}
