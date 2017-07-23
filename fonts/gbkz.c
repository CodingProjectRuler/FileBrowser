
#include <font_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

static unsigned char *g_pucHZKMST;
static unsigned char *g_pucHZKMEnd;

static int GBKFontInit(char *pcFontFile, unsigned int dwFontSize);
static int GBKGetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap);

static T_FontOpr g_tGBKFontOpr = {
	.name = "gbk",
	.FontInit = GBKFontInit,
	.GetFontBitmap = GBKGetFontBitmap,
};

static int GBKFontInit(char *pcFontFile, unsigned int dwFontSize)
{
	int GBKfd;
	struct stat tStat;	//�ļ����Խṹ

	if(dwFontSize != 16){
		printf("GBK can't suppot %d font size\n", dwFontSize);
		return -1;
	}
	
	GBKfd = open(pcFontFile, O_RDONLY);
	if(GBKfd < 0){
		printf("Can't open %s\n",pcFontFile);
		return -1;
	}
	if(fstat(GBKfd, &tStat)){		//��ȡ�ļ�����
		printf("Can't get stat\n");
		return -1;
	}

	g_pucHZKMST = (unsigned char *)mmap(NULL, tStat.st_size, PROT_READ, MAP_SHARED, GBKfd, 0);
	if(g_pucHZKMST == (unsigned char *)(-1)){
		printf("Can't mmap for hzkfile\n");
		return -1;
	}
	g_pucHZKMEnd = g_pucHZKMST + tStat.st_size;
	return 0;	
}

/*�����õ�λͼ�ĸ�������*/
static int GBKGetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap)
{
	int iArea;
	int iWhere;
	if((dwCode & 0xffff0000)){
//		printf("Can't support this code\n");
		return -1;
	}
	iArea = (int)((dwCode >> 8) & 0xff) - 0xA1;
	iWhere =  (int)(dwCode & 0xff) - 0xA1;

	if((iArea < 0) || (iWhere < 0))
		return -1;

	/*������ֵ�õ��������ݲ�����λͼ����ʾ����*/
	ptFontBitMap->iLeft = ptFontBitMap->iX;
	ptFontBitMap->iTop = ptFontBitMap->iY;
	ptFontBitMap->iXmax = ptFontBitMap->iX + 16;
	ptFontBitMap->iYmax = ptFontBitMap->iY + 16;
	ptFontBitMap->iNextX = ptFontBitMap->iX + 16;
	ptFontBitMap->iNextY = ptFontBitMap->iY;
	ptFontBitMap->iBpp = 1;
	ptFontBitMap->iPitch= 2;	/*ÿ����Ҫ�����ֽڱ�ʾ*/
	ptFontBitMap->pucBuffer = g_pucHZKMST+ (iArea * 94 + iWhere) * 32;
	if(ptFontBitMap->pucBuffer >= g_pucHZKMEnd)
		return -1;
	return 0;
}

int GBKRegister(void)
{
	return RegisterFontOpr(&g_tGBKFontOpr);
}




