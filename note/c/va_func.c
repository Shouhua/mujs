#include <stdio.h>
#include <stdarg.h>

int mysqrt(int n1, ...)
{
	va_list vl;
	va_start(vl, n1);
	int sum = 0, n = n1;
	while (n > 0)
	{
		sum += (n * n);
		n = va_arg(vl, int);
	}
	va_end(vl);
	return sum;
}

int main()
{
	int sum = mysqrt(1, 2, 3);
	printf("sum: %d\n", sum);
}