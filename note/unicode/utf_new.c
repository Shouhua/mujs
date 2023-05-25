#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/utf.h"

int main (int argc, char *argv[]) {
	Rune r;
	int len = chartorune(&r, "😊");
	printf("\\u05D0 len: %d, 0x%x\n", len, r);
	char *c = malloc(sizeof(char)*4);
	memset(c, 0, 4);
	runetochar(c, &r);
	printf("0x%02x 0x%x 0x%x 0x%x\n", (unsigned char)c[0], (unsigned char)c[1], (unsigned char)c[2], (unsigned char)c[3]);
	printf("%s\n", c);

	len = chartorune(&r, "hello");
	printf("hello len: %d, 0x%x\n", len, r);
}