#include "../../../utf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char *argv[]) {
	Rune *r = malloc(sizeof(Rune));
	memset(r, 0, sizeof(Rune));
	char *c = "\U00010400";
	int len = chartorune(r, c);
	printf("\\U00010400 len: %d, Rune: 0x%x\n", len, *r);
	// c = "\u0205";
	// len = chartorune(r, c);
	// printf("\\u0205 len: %d, Rune: 0x%x, utflen: %d, runelen: %d\n", len, *r, utflen(c), runelen(r));
	c = "\U0001f525";
	len = chartorune(r, c);
	printf("\U0001f525 len: %d, Rune: 0x%x, utflen: %d, runelen: %d\n", len, *r, utflen(c), runelen(r));
	// TODO 为什么不行
	// c = "\u0045";
	// len = chartorune(r, c);
	// printf("\u0045 len: %d, Rune: 0x%x\n", len, *r);
	// printf("\u0048\u0065\u006C\u006C\u006F\n");
}