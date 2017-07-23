/*文件头部->EncodingOpr
  *根据名字，初始化EncodingOpr的Fonts表
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
	int iPage;		//页码
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

/*打开文本文件，根据文件头部选择EncodingOpr*/
int OpenTextFile(char *pcFileName)
{
	int Textfd;
	struct stat tStat;	//文件属性结构
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

	/*选中了EncodingOpr*/
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
		/*释放文本文件每页的信息*/
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
	
/*每个Encoding有一个Font链表，链表中的每个模块可以获取编码的Font位图*/
/*现在根据条件，来对链表中的每一个模块进行检测，满足的进行初始化*/
int InitEncoding_Fonts(char *pucHZKFile, char *pucFreeTypeFile, unsigned int dwFontSize)
{
	int iError =0;
	PT_FontOpr ptTemp;
	ptTemp = g_ptEncodingOpr->ptFontOprSupportedHead;
	while(ptTemp != NULL){
		if(strcmp(ptTemp->name, "ascii") == 0){
			if (g_ptEncodingOpr->name == "ascii") {
				/*编码为ANSI_GBK，支持的Font为ascii和gbk*/
				dwFontSize = 16;
			}
			iError = ptTemp->FontInit(NULL, dwFontSize);
			g_dwFontSize = dwFontSize;	/*字体大小作为参数传入，在这里记录下来*/
		}
		else if(strcmp(ptTemp->name, "gbk") == 0){
			if (g_ptEncodingOpr->name == "ascii") {
				dwFontSize = 16;
			}	
			printf("FontSize:%d\n", dwFontSize);
			iError = ptTemp->FontInit(pucHZKFile, dwFontSize);
			g_dwFontSize = dwFontSize;	/*字体大小作为参数传入，在这里记录下来*/
		}
		else {
			/*freetype*/
			iError = ptTemp->FontInit(pucFreeTypeFile, dwFontSize);	
			g_dwFontSize = dwFontSize;	/*字体大小作为参数传入，在这里记录下来*/
			
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

/*选择显示器并初始化*/
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
/*以上是准备工作，接下来可以在LCD上显示文字了*/
/*ptFontBitMap包含一个Font的所有信息*/
int ShowOneFont(PT_FontBitMap ptFontBitMap)
{
	int i = 0;
	int x, y;
	int bit;
	unsigned char ucByte = 0;
	
	for(y = ptFontBitMap->iTop; y < ptFontBitMap->iYmax; y++){
		i = (y -ptFontBitMap->iTop) * ptFontBitMap->iPitch;		/*每一行的起始地址*/
		for(x = ptFontBitMap->iLeft, bit =7; x < ptFontBitMap->iXmax; x++){
			if(bit == 7)	/*每显示完8位像素就从buffer中取一个字节*/
				ucByte = ptFontBitMap->pucBuffer[i++];
			if(ucByte & (1<<(bit--)))	/*点亮这个点*/
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
	/*超出了本行*/
	if(ptFontBitMap->iXmax > g_ptDispOpr->iXres){
	       /*这里要重新计算Left, Top,NextX,NextY的值*/
	       ptFontBitMap->iY = CheckYRange(ptFontBitMap->iY);
		if(ptFontBitMap->iY == 0)
			return -1;				/*显示完一页*/
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
	/*没有超出本行,继续正常显示*/
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

/*以g_ptPageListHead为头部构造结构体双链表存储每页的信息*/
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
			return -1;	/*分配内存失败*/
		}
	}
	else {
		return 0;	
	}
}

/*一页的开始
  *首先清屏->根据传入的本页在文件中的起始位置获取编码
  *其次循环获取编码处理编码
  *最后根据条件记录本业的信息
  */
int ShowOnePage(unsigned char *pucThisPageStartAtFile)
{
	T_CodeDesc tCodeDesc;
	T_FontBitMap tFontBitMap;	/*每个Code对应的位置信息和字体数据*/
	PT_FontOpr ptFontOpr;
	tFontBitMap.iX = 0;	/*每一页的起始位置*/
	tFontBitMap.iY = 0;
	tCodeDesc.pucThisCodeStart = pucThisPageStartAtFile;	

	/*首先清屏*/
	g_ptDispOpr->CleanScreen(COLOR_BACKGROUND);
	
	/*循环文件中本页的起始位置一次取码显示*/
	/*知道LCD显示页满或者文件读取结束*/
	while(1){
		tCodeDesc.CodeLen = g_ptEncodingOpr->GetCodeFromFile(tCodeDesc.pucThisCodeStart, g_pucTEXTMEND, &(tCodeDesc.dwCode));
		if(tCodeDesc.CodeLen == 0){
			printf("End of File!\n");
			return -1;				/*The end of file*/
		}
		tCodeDesc.pucThisCodeStart += tCodeDesc.CodeLen;		/*指针本身的值增加*/	
		/*得到的Code有三种特殊情况
		  *控制符:只有效果的编码，没有实际的意义
		  *\n:换行
		  *\r:回车
		  *\t:制表符:tab键
		  */
		/*换行符:横坐标不变，纵坐标加FontSize*/
		if(tCodeDesc.dwCode == '\n'){
			tFontBitMap.iX = 0;			/*Go to the Start of Line*/
			tFontBitMap.iY = CheckYRange(tFontBitMap.iY);
			/*超出了LCD的边界*/
			if(tFontBitMap.iY == 0){
				/*记录这页的最后一个Code(超出的\n算在This Page)的位置*/
				/*也就是下一页的其实位置*/
				if(ucFlag)
					RecordListPage(pucThisPageStartAtFile, &tCodeDesc);
				 return 0;					/*显示完一页*/
			}
			else
				continue;					/*重新从取下一个码开始*/
		}
		else if(tCodeDesc.dwCode == '\r')
			continue;						/*直接忽略*/
		else if(tCodeDesc.dwCode == '\t')
			tCodeDesc.dwCode = ' ';		/*遇到制表符，用一个空格代替*/
		/*处理完特殊Code后，就可以根据Code获取位图了*/
		/*在选中的Encoding 所支持的*/
		ptFontOpr = g_ptEncodingOpr->ptFontOprSupportedHead;
		while(ptFontOpr != NULL){
			tCodeDesc.iError = ptFontOpr->GetFontBitmap(tCodeDesc.dwCode, &tFontBitMap);

			/*在这个Font中获取位图成功*/
			if(tCodeDesc.iError == 0){
				/*检测边界*/
				tCodeDesc.iError = CheckXYRange(&tFontBitMap);
				/*显示完一页*/
				if(tCodeDesc.iError == -1){
					if(ucFlag){
						tCodeDesc.pucThisCodeStart  -=  tCodeDesc.CodeLen;
						RecordListPage(pucThisPageStartAtFile, &tCodeDesc);
					}
					return 0;			/*显示完一页*/	
				}
				/*显示位图*/
				ShowOneFont(&tFontBitMap);
				/*更新显示坐标*/
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
	else return -1;	/*文件显示完毕*/
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

	 
