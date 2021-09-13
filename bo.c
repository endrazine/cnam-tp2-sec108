/**
* bo.c : v0.1 : Mon 06 Sep 2021 03:19:31 AM CEST
* Jonathan Brossard // endrazine@gmail.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

int vulnerable(char *src){

	char buffer[512];
	
	memcpy(buffer, src, strlen(src));

	printf("Welcome: %s !\n", buffer);

	return 0;
}

int main (int argc, char **argv, char **envp)
{

	vulnerable(argv[1]);
	return 0;
}

