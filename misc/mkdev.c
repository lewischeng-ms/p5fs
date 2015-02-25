#include <stdio.h>
#include "p5.h"

char buf[BLOCKSIZE];

int main()
{
	FILE * fp = fopen("simulated_device", "wb");
	if (!fp)
	{
		printf("Error creating simulated_device file.\n");
		return 1;
	}
	int i;
	for (i = 0; i < 250000; i++)
		fwrite(buf, BLOCKSIZE, 1, fp);
	fclose(fp);
	return 0;
}
