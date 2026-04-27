/************************************************
 * Title:		chamal		        *
 * Creator:		John J. Coleman		*
 * Creation Date:	2026.04.27		*
 * Modified Date:	2026.04.27		*
 * 						*
 * Description:					*
 * Test for chamal module.			*
 ************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MEMSIZE 64

int main(int argc, char *argv[])
{
	int fd;
	char *text;

	text = malloc(MEMSIZE);
	if(!text)
	{
		fprintf(stderr, "NULL memory pointer.\n");
		return -1;
	}

	fd = open("/dev/chamal0", O_RDWR);
	if(fd < 0)
	{
		fprintf(stderr, "open failure.\n");
		free(text);
		return -1;
	}

	sprintf(text, "Testing 1, 2, 3...");
	write(fd, text, strlen(text));

	lseek(fd, 0, SEEK_SET);

	memset(text, 0, MEMSIZE);

	read(fd, text, MEMSIZE);

	fprintf(stdout, "%s\n", text);

	free(text);

	return 0;
}
