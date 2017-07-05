#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

	char name[500];
	FILE *ptr_myfile;
	int i;
	unsigned char tchar;

 	if (argc > 1)
  {
  	sprintf (name,argv[1]);
		printf ("%s\n",name);
	}

		ptr_myfile=fopen(name,"rb");
		if (!ptr_myfile)
		{
			printf("Unable to open file!");
			return 1;
		}
		i=0;
		while(1 == fread(&tchar,1,1,ptr_myfile))
		{
    	printf("%02x",tchar);
	    i++;
			if (tchar ==0xb4) printf ("\n");
		}	
		printf ("\n");
		fclose(ptr_myfile);
		return 0;
	}
