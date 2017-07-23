#include <stdio.h>
/*
	1.对ASCII编码的英文字母和一般常用的英文标点符号
	内存中存储他们的ASCII码值，也就是他们的编码值,根据编码值
	在一个数组(个数比较少，用一个数组就可以搞定)中找到位图数据
	2.对于GBK来说，内存中存储的也是编码值，然后对应一个字库文件，
	使用编码在相应的文件中找到位图数据
	3.对于utf编码的文本，内存中存储的依然是编码，借助一个字体轮廓文件，
	从这里开始freetype库帮我们做，从字体轮廓库中找到那个字的轮廓，
	然后生成用于显示的位图数据，并且可以放大和缩小
*/

int main(int argc, char **argv)
{
	char *a = argv[1];
	int i = 0;
	int *b = &i;
	printf("%d   %d\n", sizeof(a), sizeof(b));
	while(a[i++] != '\0');
	printf("%d\n", i);
	return 0;
}
