#include <stdio.h>
#include "sort.h"

int main()
{
	int a[] = { 3, 1, 5, 0, 2};
	int len = sizeof(a) / sizeof(a[0]);
	insert(a, len);
	printf("insert sort: \n");
	for(int i = 0; i < len; i++)
	{
		printf("%d\n", a[i]);
	}
	select(a, len);
	printf("select sort: \n");
	for(int i = 0; i < len; i++)
	{
		printf("%d\n", a[i]);
	}

	return 0;
}