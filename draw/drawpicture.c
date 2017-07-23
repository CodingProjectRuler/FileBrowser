#include <file.h>
#include <drawpicture.h>
#include <drawtxt.h>
#include <font_manager.h>
#include <display_manager.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdbool.h>

static PT_DispOpr g_ptDispOpr;
static PT_PictureFileParser ptPictureFileParser;
static T_pictureInfos tPictureInfos;
static T_FileInfos tFileInfos;
unsigned char *CopyOfVideoMemery;
/*为每个功能构建一个链表*/
int RegisterListp(void)
{
	DisplayRegister();		/*显示模块*/
	FontsRegister();		/*获取font位图模块*/
	EncodingRegister();	/*解码txt文本模块*/
	InputRegister();		/*输入模块*/
	PictureFileParserInit();	/*图片解析模块*/
}

/*根据显示设备名字在显示模块链表中选择设备并初始化*/
/*所谓的初始化就是读出显示器的相关参数和映射显存*/
int SelectAndInitDisplayp(char *pcName)
{
	int iError;
	g_ptDispOpr = GetDispOpr(pcName);
	if(g_ptDispOpr == NULL){
		printf("Failed to GetDispOpr\n");
		return -1;
	}
	iError = g_ptDispOpr->DeviceInit();
	CopyPointer(g_ptDispOpr);
	return iError;
}

static int FBShowPixelP(int iPenX, int iPenY, unsigned int dwColor)
{
	unsigned char  *pucPen_8;	    /*One byte*/
	unsigned short *pwPen_16;	    /*Two bytes*/
	unsigned int     *pdwPen_32;	    /*Four bytes*/

	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;

	/*Calculate the position of(x and y) in video memory(显存)*/
	pucPen_8   = CopyOfVideoMemery + iPenY * g_ptDispOpr->iLineWidthBytes+ iPenX * g_ptDispOpr->iBpp/8;
	pwPen_16   = (unsigned short *)pucPen_8;		/*Just chage it's attribute(属性)*/
	pdwPen_32  = (unsigned int *)pucPen_8;
	/*Write the color date to the calculated poation according the "iBpp"*/
	switch(g_ptDispOpr->iBpp){
		case 8:
			*pucPen_8 = dwColor;
			break;
		case 16:
			/*565*/
			dwRed = (dwColor >> 19) & 0x1f;
			dwGreen = (dwColor >>10) & 0x3f;
			dwBlue = (dwColor >> 3) & 0x1f;
			dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue << 0);
			*pwPen_16 = dwColor;
			break;
		case 32:
			*pdwPen_32 = dwColor;
			break;
		default:
			printf("Can't suppot this Pixelwidth\n ");
			return -1;
			break;
	}
	return 0;
}

/*ptFontBitMap包含一个Font的所有信息*/
int ShowOneFontP(PT_FontBitMap ptFontBitMap)
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
				FBShowPixelP(x, y, COLOR_FOREGROUND);
			if(bit == -1)
				bit = 7;
		}
	}
	return 0;
}

/*根据文件名打开文件*/
/*获取文件的相关信息然后映射文件*/
/*根据文件内容(格式)在文件解析链表中选择解析器*/
int SelectPicformatParserForFile(char *strFileName)	
{
	int iError;
	
	strncpy(tFileInfos.FileName, strFileName, 256);
	tFileInfos.FileName[255] = '\0';
	iError = OpenAndMapFile(&tFileInfos);
	if (iError) {
		printf("Open or map file fail");
		return -1;
	}

	ptPictureFileParser = GetParserByFile(&tFileInfos);
	if (ptPictureFileParser == NULL) {
		UnmapAndCloseFile(&tFileInfos);
		return -1;
	}
	return 0;
}



int RealseFile(void)
{
	UnmapAndCloseFile(&tFileInfos);
	return 0;
}

/*根据文件信息和显示设备信息去取得可显示的像素数据*/
/*将数据转换并存储在一块临时内存中*/
int GetPixelDatasFrmFileInfos(PT_pictureInfos *ptPictureInfos, T_ZoonInfos tZoonInfos)
{
	int iError;
	*ptPictureInfos = &tPictureInfos;
	tPictureInfos.iBpp = g_ptDispOpr->iBpp;			
	iError = ptPictureFileParser->GetPixelDatas(&tFileInfos, &tPictureInfos, tZoonInfos);
	if (iError) {
		printf("Get pixel datas fail\n");
		return -1;
	}
	return 0;
}

int MallocCopyOfVideoMemery(void)
{
	CopyOfVideoMemery = malloc(g_ptDispOpr->iLineWidthBytes * g_ptDispOpr->iYres);
	if (CopyOfVideoMemery == NULL) {
		printf("Malloc fail!\n");
		return -1;
	}
	return 0;
}

int TestPosition(PT_PosXY ptPosXY)
{
	bool Flag = false;
	if (ptPosXY->PosX >= g_ptDispOpr->iXres) {
		ptPosXY->PosX = g_ptDispOpr->iXres;
		Flag = true;
	}
	if (ptPosXY->PosY >= g_ptDispOpr->iYres) {
		ptPosXY->PosY = g_ptDispOpr->iYres;
		Flag = true;
	}
	if (-ptPosXY->PosX >= g_ptDispOpr->iXres) {
		ptPosXY->PosX = -g_ptDispOpr->iXres;
		Flag = true;
	}
	if (-ptPosXY->PosY >= g_ptDispOpr->iYres) {
		ptPosXY->PosY = -g_ptDispOpr->iYres;
		Flag = true;
	}		 	
	if (Flag) {
		return -1;
	}
	return 0;
}

/*构建显存格式数据*/
/*存储在一个缓冲区，构建好完整的显存后直接复制给显存*/
/*在显存的复制空间放置图片*/
/*可多次放置，位置:(x, y)  大小:tPictureInfos.iTotalBytes*/
int CreateAndShowDisplayDatas(T_PosXY tPosXY)
{
	unsigned char *pucSrc;
	unsigned char *pucDest;
//	unsigned char *pucCDest;
	int i, width, height;
	if (tPictureInfos.iBpp != g_ptDispOpr->iBpp ||
		tPosXY.PosX >= g_ptDispOpr->iXres || tPosXY.PosY >= g_ptDispOpr->iYres ||
		-tPosXY.PosX >= g_ptDispOpr->iXres || -tPosXY.PosY >= g_ptDispOpr->iYres) {
		return -1;
	}
	pucSrc = tPictureInfos.pucDataAddress;
	
	if (tPosXY.PosX < 0) {
		tPosXY.PosX = -tPosXY.PosX;
		pucSrc +=  tPosXY.PosX*g_ptDispOpr->iBpp/8;
		width = g_ptDispOpr->iXres < (tPictureInfos.iWidth - tPosXY.PosX)? g_ptDispOpr->iXres : tPictureInfos.iWidth - tPosXY.PosX;		
		tPosXY.PosX = 0;
	}
	else {
		width = g_ptDispOpr->iXres < (tPosXY.PosX + tPictureInfos.iWidth)? (g_ptDispOpr->iXres - tPosXY.PosX) : tPictureInfos.iWidth;
	}
	
	if (tPosXY.PosY < 0) {
		tPosXY.PosY = -tPosXY.PosY;
		pucSrc += tPosXY.PosY*tPictureInfos.iLineBytes;
		height = g_ptDispOpr->iYres < (tPictureInfos.iHeight - tPosXY.PosY)? g_ptDispOpr->iYres : tPictureInfos.iHeight - tPosXY.PosY;	
		tPosXY.PosY = 0;
	}
	else {
		height = g_ptDispOpr->iYres < (tPosXY.PosY + tPictureInfos.iHeight)? (g_ptDispOpr->iYres - tPosXY.PosY) : tPictureInfos.iHeight;	
	}
	pucDest = CopyOfVideoMemery + tPosXY.PosY*g_ptDispOpr->iLineWidthBytes + tPosXY.PosX*g_ptDispOpr->iBpp/8;

	for (i = 0; i < height; i++) {
		memcpy(pucDest, pucSrc, width*g_ptDispOpr->iBpp/8);
		pucSrc += tPictureInfos.iLineBytes;
		pucDest += g_ptDispOpr->iLineWidthBytes;
	}
	return 0;
}

void ShowOnePicture(void)
{
	g_ptDispOpr->ShowCompleteOnePage(CopyOfVideoMemery);
	//free(CopyOfVideoMemery);
}

void CleanScreen(unsigned int dwBackColor)
{
	memset(CopyOfVideoMemery, dwBackColor, g_ptDispOpr->iLineWidthBytes * g_ptDispOpr->iYres);
}

void FreePixelDataMemery(void)
{
	free(tPictureInfos.pucDataAddress);
}