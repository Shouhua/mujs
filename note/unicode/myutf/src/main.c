#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include "utf.h"

int main(int argc, char *argv[]) 
{
	FILE *f;
	char line[8];
	uint16_t *utf16;
	uint8_t *utf8;
	int len;
	long cp;

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
	utf16 = malloc(sizeof(uint16_t) * 2);
	utf8 = malloc(sizeof(uint8_t) * 4);
	while(fgets(line, sizeof(line), f))
	{
		// printf("%s: 0x%lx\n", line, strtol(line, NULL, 16));
		// line[strcspn(line, "\n")] = '\0';
		cp = strtol(line, NULL, 16);
		fprintf(stdout, "0x%08lx\t", cp);
		len = unicodetoutf16(utf16, (uint32_t)cp);
		fprintf(stdout, "utf16 len: %d, 0x%04x 0x%04x\t", len, utf16[0], utf16[1]);
		len = unicodetoutf8(utf8, (uint32_t)cp);
		fprintf(stdout, "utf8 len: %d, 0x%x 0x%x 0x%x 0x%x\n", len, utf8[0], utf8[1], utf8[2], utf8[3]);
	}
	fclose(f);
	utf16[0] = 0xD801;
	utf16[1] = 0xDC37;
	len = utf16toutf8(utf8, utf16);
	printf("utf16toutf8: 0xD801 0XDC37 > len: %d, 0x%x 0x%x 0x%x 0x%x\n", len, utf8[0], utf8[1], utf8[2], utf8[3]);
	uint8_t utf88[] = {0xf3, 0xa0, 0x87, 0xad};
	len = utf8toutf16(utf16, utf88);
	printf("utf8toutf16: 0xf3 0xa0 0x87 0xad > len: %d, 0x%x 0x%x\n", len, utf16[0], utf16[1]);
	uint8_t utf888[] = {0xc0, 0x80};
	uint32_t code_point;
	len = utf8tounicode(&code_point, utf888);
	printf("0xc0 0x80 unicode: 0x%x", code_point);
	return 0;
}