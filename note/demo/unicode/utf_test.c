#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) 
{
	FILE *f;
	int n, t;
	char *s, line[6];

	if(argc != 2)
	{
		fprintf(stderr, "help: utf_test codepoint.txt");
		return EXIT_FAILURE;
	}
	f = fopen(argv[1], "rb");
	if(!f)
	{
		fprintf(stderr, "open file failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if(fseek(f, 0, SEEK_END) < 0)
	{
		fclose(f);
		fprintf(stderr, "seek file failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if((n = ftell(f)) < 0)
	{
		fclose(f);
		fprintf(stderr, "ftell failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if(fseek(f, 0, SEEK_SET) < 0)
	{
		fclose(f);
		fprintf(stderr, "seek set to start failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	s = malloc(sizeof(char) * n + 1);
	t = fread(s, 1, n, f);
	if(t != n)
	{
		fclose(f);
		fprintf(stderr, "read data from %s failed: %s", argv[1], strerror(errno));
		return EXIT_FAILURE;
	}
	s[n] = 0;
	fseek(f, 0, SEEK_SET);
	while(fgets(line, sizeof(line), f))
	{
		printf("%s: 0x%lx\n", line, strtol(line, NULL, 16));
	}
	fclose(f);
	return 0;
}