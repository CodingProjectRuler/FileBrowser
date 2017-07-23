#include <stdio.h>
#include <string.h>

/*sizeof()函数计算长度仅仅在用于数组的时候不是计算本身的长度，计算数组的长度，其他包括与二位数组类似的二级指针也不能计算数组的长度，而且计算的长度是以字节计算的而不是条目个数*/

int main(int argc, char **argv)
{
	int ab[4][2] = {
		{1, 1},
		{1, 1},
		{1, 1},
		{1, 1},
	};
	printf("%d\n", sizeof(ab));
	return 0;
	char *a = ".txt";
	char *b[] = {"a", "b", "c"};
	char **c = a;
	if (strcmp(argv[1]+1, a) == 0) {
		printf("right:%s   %d %d\n", argv[1]+1, sizeof(b), sizeof(c));
	}
	return 0;
}
