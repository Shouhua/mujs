#include <stdio.h>
#include <setjmp.h>

jmp_buf buf;

void func()
{
	printf("hello before\n");

	longjmp(buf, 1);
	printf("hello after\n");
}

int main (int argc, char *argv[]) {
	if(setjmp(buf))
	{
		printf("in setjmp\n");
	}
	else
	{
		printf("out setjmp\n");
		func();
	}
	return 0;
}