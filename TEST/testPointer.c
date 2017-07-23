#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *a;

void freeP(void)
{
	free(a);
}
int main(void)
{
	char *b;
	int i = 0;
	a = malloc(10);
	b = malloc(10);
	for (i =0; i < 9; i++) {
		a[i] = 'a';
	}
	a[i] = '\0';
	printf("before free: %s\n", a);
	freeP();
	memcpy(b, a, 10);
	printf("after free: %s\n", b);
	return 0;
}
