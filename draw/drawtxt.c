/*�ļ�ͷ��->EncodingOpr
  *�������֣���ʼ��EncodingOpr��Fonts��
  */
#include <display_manager.h>
#include <font_manager.h>
#include <encoding_manager.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <drawtxt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <input_manager.h>


typedef struct PageDesc {
	int iPage;		//ҳ��
	unsigned char *pucThisPageStartAtFile;
	unsigned char *pucNextPageStartAtFile;
	struct PageDesc *ptPrePage;
	struct PageDesc *ptNextPage;
}T_PageDesc, *PT_PageDesc;

typedef struct CodeDesc{
	unsigned char *pucThisCodeStart;
	unsigned int dwCode;
	int CodeLen;
	int iError;

}T_CodeDesc, *PT_CodeDesc;

static unsigned char ucFlag;
static unsigned int g_dwFontSize;
static PT_DispOpr g_ptDispOpr;
static PT_EncodingOpr g_ptEncodingOpr;

static unsigned char *g_pucTEXTMST;
static unsigned char *g_pucTEXTMEND;

static unsigned char *g_pucCodeStartAtFile;
static unsigned char *g_pucNextPageStartAtFile;

static PT_PageDesc g_ptPageListHead = NULL;

static PT_PageDesc g_ptCurPage = NULL;

FILE *g_Fd;

/*Create List for three parts(Dispay, FreeType, Encoding)*/
int RegisterList(void)
{
	DisplayRegister();
	FontsRegister();
	EncodingRegister();
	InputRegister();
}
int ExitWork(void)
{
	PT_PageDesc ptTemp; 
	DelFontOprForEncoding();
	ptTemp = g_ptPageListHead;
	while(ptTemp != NULL){
		free(ptTemp);
		ptTemp = ptTemp->ptNextPage;
	}
	AllInputDeviceExit();
	return 0;
}

/*���ı��ļ��������ļ�ͷ��ѡ��EncodingOpr*/
int OpenTextFile(char *pcFileName)
{
	int Textfd;
	struct stat tStat;	//�ļ����Խṹ
#if 0
	Textfd = open(pcFileName, O_RDONLY);
	if(Textfd < 0){
		printf("Can't open %s\n", pcFileName);
		return -1;
	}
#endif 	
	g_Fd = fopen(pcFileName, "r+");
	if(g_Fd == NULL){
		printf("Can't open %s\n", pcFileName);
		return -1;
	}
	Textfd = fileno(g_Fd);

	if(fstat(Textfd, &tStat)){
		printf("Can't get fstat\n");
		return -1;
	}
	g_pucTEXTMST = (unsigned char *)mmap(NULL, tStat.st_size, PROT_READ, MAP_SHARED, Textfd, 0);
	if(g_pucTEXTMST == (unsigned char *)-1){
		printf("Can't mmap for textfile\n");
		return -1;
	}

	g_pucTEXTMEND = g_pucTEXTMST + tStat.st_size;

	/*ѡ����EncodingOpr*/
	g_ptEncodingOpr = SelectEncodingForFile(g_pucTEXTMST);
	if(g_ptEncodingOpr != NULL){
		g_pucCodeStartAtFile = g_pucTEXTMST + g_ptEncodingOpr->HeadLen;
		return 0;
	}
	else {
		printf("Select encoding fail\n");
		return -1;
	}
}

int MunmapAndCloseFile(void)
{
	PT_PageDesc ptPageDescOne, ptPageDescTwo;
	ptPageDescOne = g_ptPageListHead;
	while (ptPageDescOne) {
		/*�ͷ��ı��ļ�ÿҳ����Ϣ*/
		ptPageDescTwo = ptPageDescOne;
		ptPageDescOne = ptPageDescOne->ptNextPage;
		free(ptPageDescTwo);
	}
	g_ptPageListHead = NULL;
	g_ptCurPage = NULL;
	munmap(g_pucTEXTMST, g_pucTEXTMEND - g_pucTEXTMST);
	fclose(g_Fd);	
	//DelFontOprForEncoding();
}
	
/*ÿ��Encoding��һ��Font���������е�ÿ��ģ����Ի�ȡ�����Fontλͼ*/
/*���ڸ������������������е�ÿһ��ģ����м�⣬����Ľ��г�ʼ��*/
int InitEncoding_Fonts(char *pucHZKFile, char *pucFreeTypeFile, unsigned int dwFontSize)
{
	int iError =0;
	PT_FontOpr ptTemp;
	ptTemp = g_ptEncodingOpr->ptFontOprSupportedHead;
	while(ptTemp != NULL){
		if(strcmp(ptTemp->name, "ascii") == 0){
			if (g_ptEncodingOpr->name == "ascii") {
				/*����ΪANSI_GBK��֧�ֵ�FontΪascii��gbk*/
				dwFontSize = 16;
			}
			iError = ptTemp->FontInit(NULL, dwFontSize);
			g_dwFontSize = dwFontSize;	/*�����С��Ϊ�������룬�������¼����*/
		}
		else if(strcmp(ptTemp->name, "gbk") == 0){
			if (g_ptEncodingOpr->name == "ascii") {
				dwFontSize = 16;
			}	
			printf("FontSize:%d\n", dwFontSize);
			iError = ptTemp->FontInit(pucHZKFile, dwFontSize);
			g_dwFontSize = dwFontSize;	/*�����С��Ϊ�������룬�������¼����*/
		}
		else {
			/*freetype*/
			iError = ptTemp->FontInit(pucFreeTypeFile, dwFontSize);	
			g_dwFontSize = dwFontSize;	/*�����С��Ϊ�������룬�������¼����*/
			
		}
		ptTemp = ptTemp->ptNext;	
	}
	return 0;
} 

int CopyPointer(PT_DispOpr ptDisOpr)
{
	g_ptDispOpr = ptDisOpr;
	return 0;
}

/*ѡ����ʾ������ʼ��*/
int SelectAndInitDisplay(char *pcName)
{
	int iError;
	g_ptDispOpr = GetDispOpr(pcName);
	if(g_ptDispOpr == NULL){
		printf("Failed to GetDispOpr\n");
		return -1;
	}
	
	iError = g_ptDispOpr->DeviceInit();
	return iError;
}
/*������׼��������������������LCD����ʾ������*/
/*ptFontBitMap����һ��Font��������Ϣ*/
int ShowOneFont(PT_FontBitMap ptFontBitMap)
{
	int i = 0;
	int x, y;
	int bit;
	unsigned char ucByte = 0;
	
	for(y = ptFontBitMap->iTop; y < ptFontBitMap->iYmax; y++){
		i = (y -ptFontBitMap->iTop) * ptFontBitMap->iPitch;		/*ÿһ�е���ʼ��ַ*/
		for(x = ptFontBitMap->iLeft, bit =7; x < ptFontBitMap->iXmax; x++){
			if(bit == 7)	/*ÿ��ʾ��8λ���ؾʹ�buffer��ȡһ���ֽ�*/
				ucByte = ptFontBitMap->pucBuffer[i++];
			if(ucByte & (1<<(bit--)))	/*���������*/
				g_ptDispOpr->ShowOnePixel(x, y, COLOR_FOREGROUND);
			if(bit == -1)
				bit = 7;
		}
	}
	return 0;
}

int CheckYRange(int iY)
{
	if((iY + g_dwFontSize) < (g_ptDispOpr->iYres - g_dwFontSize))
		return (iY + g_dwFontSize);
	else
		return 0;
}

int CheckXYRange(PT_FontBitMap ptFontBitMap)
{
	/*�����˱���*/
	if(ptFontBitMap->iXmax > g_ptDispOpr->iXres){
	       /*����Ҫ���¼���Left, Top,NextX,NextY��ֵ*/
	       ptFontBitMap->iY = CheckYRange(ptFontBitMap->iY);
		if(ptFontBitMap->iY == 0)
			return -1;				/*��ʾ��һҳ*/
		else{
			ptFontBitMap->iLeft  -= ptFontBitMap->iX;
			ptFontBitMap->iXmax -= ptFontBitMap->iX;			

			ptFontBitMap->iTop += g_dwFontSize;
			ptFontBitMap->iYmax += g_dwFontSize;

			ptFontBitMap->iNextX -=  ptFontBitMap->iX;
			ptFontBitMap->iNextY += g_dwFontSize;

			ptFontBitMap->iX = 0;
		}
	}else return 0;
	/*û�г�������,����������ʾ*/
}

int RecordOnePageDesc(PT_PageDesc ptOnePageDesc)
{
	PT_PageDesc ptTemp;
	ptOnePageDesc->ptNextPage = NULL;
	ptOnePageDesc->ptPrePage = NULL;	
	if(g_ptPageListHead == NULL){
		g_ptPageListHead = ptOnePageDesc;
	}else{
		ptTemp = g_ptPageListHead;
		while(ptTemp->ptNextPage != NULL)
			ptTemp = ptTemp->ptNextPage;
		ptTemp->ptNextPage = ptOnePageDesc;
		ptOnePageDesc->ptPrePage = ptTemp;
	}
}

/*��g_ptPageListHeadΪͷ������ṹ��˫����洢ÿҳ����Ϣ*/
int RecordListPage(unsigned char *pucThisPageStartAtFile, PT_CodeDesc ptCodeDesc)
{
	if((g_ptCurPage == NULL) || (g_ptCurPage->ptNextPage == NULL)){
		g_ptCurPage = malloc(sizeof(T_PageDesc));
		if(g_ptCurPage != NULL){
			g_ptCurPage->pucThisPageStartAtFile = pucThisPageStartAtFile;
			g_ptCurPage->pucNextPageStartAtFile = ptCodeDesc->pucThisCodeStart ;
			RecordOnePageDesc(g_ptCurPage);
			if(g_ptCurPage->ptPrePage == NULL)
				g_ptCurPage->iPage = 1;			/* The first page */
			else
				g_ptCurPage->iPage = g_ptCurPage->ptPrePage->iPage + 1;
		}else {
			printf("Malloc fail\n");
			return -1;	/*�����ڴ�ʧ��*/
		}
	}
	else {
		return 0;	
	}
}

/*һҳ�Ŀ�ʼ
  *��������->���ݴ���ı�ҳ���ļ��е���ʼλ�û�ȡ����
  *���ѭ����ȡ���봦�����
  *������������¼��ҵ����Ϣ
  */
int ShowOnePage(unsigned char *pucThisPageStartAtFile)
{
	T_CodeDesc tCodeDesc;
	T_FontBitMap tFontBitMap;	/*ÿ��Code��Ӧ��λ����Ϣ����������*/
	PT_FontOpr ptFontOpr;
	tFontBitMap.iX = 0;	/*ÿһҳ����ʼλ��*/
	tFontBitMap.iY = 0;
	tCodeDesc.pucThisCodeStart = pucThisPageStartAtFile;	

	/*��������*/
	g_ptDispOpr->CleanScreen(COLOR_BACKGROUND);
	
	/*ѭ���ļ��б�ҳ����ʼλ��һ��ȡ����ʾ*/
	/*֪��LCD��ʾҳ�������ļ���ȡ����*/
	while(1){
		tCodeDesc.CodeLen = g_ptEncodingOpr->GetCodeFromFile(tCodeDesc.pucThisCodeStart, g_pucTEXTMEND, &(tCodeDesc.dwCode));
		if(tCodeDesc.CodeLen == 0){
			printf("End of File!\n");
			return -1;				/*The end of file*/
		}
		tCodeDesc.pucThisCodeStart += tCodeDesc.CodeLen;		/*ָ�뱾���ֵ����*/	
		/*�õ���Code�������������
		  *���Ʒ�:ֻ��Ч���ı��룬û��ʵ�ʵ�����
		  *\n:����
		  *\r:�س�
		  *\t:�Ʊ��:tab��
		  */
		/*���з�:�����겻�䣬�������FontSize*/
		if(tCodeDesc.dwCode == '\n'){
			tFontBitMap.iX = 0;			/*Go to the Start of Line*/
			tFontBitMap.iY = CheckYRange(tFontBitMap.iY);
			/*������LCD�ı߽�*/
			if(tFontBitMap.iY == 0){
				/*��¼��ҳ�����һ��Code(������\n����This Page)��λ��*/
				/*Ҳ������һҳ����ʵλ��*/
				if(ucFlag)
					RecordListPage(pucThisPageStartAtFile, &tCodeDesc);
				 return 0;					/*��ʾ��һҳ*/
			}
			else
				continue;					/*���´�ȡ��һ���뿪ʼ*/
		}
		else if(tCodeDesc.dwCode == '\r')
			continue;						/*ֱ�Ӻ���*/
		else if(tCodeDesc.dwCode == '\t')
			tCodeDesc.dwCode = ' ';		/*�����Ʊ������һ���ո����*/
		/*����������Code�󣬾Ϳ��Ը���Code��ȡλͼ��*/
		/*��ѡ�е�Encoding ��֧�ֵ�*/
		ptFontOpr = g_ptEncodingOpr->ptFontOprSupportedHead;
		while(ptFontOpr != NULL){
			tCodeDesc.iError = ptFontOpr->GetFontBitmap(tCodeDesc.dwCode, &tFontBitMap);

			/*�����Font�л�ȡλͼ�ɹ�*/
			if(tCodeDesc.iError == 0){
				/*���߽�*/
				tCodeDesc.iError = CheckXYRange(&tFontBitMap);
				/*��ʾ��һҳ*/
				if(tCodeDesc.iError == -1){
					if(ucFlag){
						tCodeDesc.pucThisCodeStart  -=  tCodeDesc.CodeLen;
						RecordListPage(pucThisPageStartAtFile, &tCodeDesc);
					}
					return 0;			/*��ʾ��һҳ*/	
				}
				/*��ʾλͼ*/
				ShowOneFont(&tFontBitMap);
				/*������ʾ����*/
				tFontBitMap.iX = tFontBitMap.iNextX;
				tFontBitMap.iY = tFontBitMap.iNextY;	
				break;	
			}else ptFontOpr = ptFontOpr->ptNext;
		}
	}
	return 0;
}

int ShowNextPage(void)
{
	int iError;
	unsigned char *pucThisPageStart;

	ucFlag = 1;
	
	if(g_ptCurPage == NULL)
		pucThisPageStart = g_pucCodeStartAtFile;
	else 
		pucThisPageStart = g_ptCurPage->pucNextPageStartAtFile;
	iError = ShowOnePage(pucThisPageStart);
	if(iError == 0){
		if(g_ptCurPage && g_ptCurPage->ptNextPage)
			g_ptCurPage = g_ptCurPage->ptNextPage;
	}
	else return -1;	/*�ļ���ʾ���*/
}

int ShowPrePage(void)
{
	int iError;
	
	ucFlag = 0;
	
	if(g_ptCurPage == NULL)
		return -1;
	else{
		if(g_ptCurPage->ptPrePage == NULL)
			iError = ShowOnePage(g_ptCurPage->pucThisPageStartAtFile);
		else{
			iError = ShowOnePage(g_ptCurPage->ptPrePage->pucThisPageStartAtFile);
			if(iError == 0){
				g_ptCurPage = g_ptCurPage->ptPrePage;
			}
			else return -1;
		}
	}	
}

	 
