#include <stdio.h>
#include <setjmp.h>

jmp_buf buf;

void try()
{
	int err_id = setjmp(buf);
	if (err_id == 1)
	{
		printf("throw error #%d\n", err_id);
	}
	else if (err_id == 2)
	{
		printf("throw error #%d\n", err_id);
	}
	else
	{
	}
	printf("finally");
}

void func()
{
	printf("hello before\n");
	// throw exception #1
	longjmp(buf, 1);
	printf("hello after\n");
}

int main()
{
	try();
	printf("out setjmp\n");
	func();
	return 0;
}