#include <stdio.h>

int a[] = {1, 2, 3, 4, 5, 6, 7, 8};

int binary_search(int *a, int size, int i)
{
	int l = 0;
	int r = size - 1;
	while(l <= r)
	{
		int m = (l + r) >> 1;
		if(*(a+m) < i)
		{
			l = m + 1;
		}
		else if(*(a+m) > i) 
		{
			r = m - 1;
		}
		else
		{
			return m;
		}
	}
	return -1;
}

int main (int argc, char *argv[]) {
	int key;
	while(scanf("%d", &key))
	{
		int index = binary_search(a, sizeof(a)/sizeof(a[0]), key);
		printf("index: %d, value: %d\n", index, a[index]);
	}
	return 0;
}