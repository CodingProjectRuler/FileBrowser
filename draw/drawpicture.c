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
/*Ϊÿ�����ܹ���һ������*/
int RegisterListp(void)
{
	DisplayRegister();		/*��ʾģ��*/
	FontsRegister();		/*��ȡfontλͼģ��*/
	EncodingRegister();	/*����txt�ı�ģ��*/
	InputRegister();		/*����ģ��*/
	PictureFileParserInit();	/*ͼƬ����ģ��*/
}

/*������ʾ�豸��������ʾģ��������ѡ���豸����ʼ��*/
/*��ν�ĳ�ʼ�����Ƕ�����ʾ������ز�����ӳ���Դ�*/
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

	/*Calculate the position of(x and y) in video memory(�Դ�)*/
	pucPen_8   = CopyOfVideoMemery + iPenY * g_ptDispOpr->iLineWidthBytes+ iPenX * g_ptDispOpr->iBpp/8;
	pwPen_16   = (unsigned short *)pucPen_8;		/*Just chage it's attribute(����)*/
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

/*ptFontBitMap����һ��Font��������Ϣ*/
int ShowOneFontP(PT_FontBitMap ptFontBitMap)
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
				FBShowPixelP(x, y, COLOR_FOREGROUND);
			if(bit == -1)
				bit = 7;
		}
	}
	return 0;
}

/*�����ļ������ļ�*/
/*��ȡ�ļ��������ϢȻ��ӳ���ļ�*/
/*�����ļ�����(��ʽ)���ļ�����������ѡ�������*/
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

/*�����ļ���Ϣ����ʾ�豸��Ϣȥȡ�ÿ���ʾ����������*/
/*������ת�����洢��һ����ʱ�ڴ���*/
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

/*�����Դ��ʽ����*/
/*�洢��һ�����������������������Դ��ֱ�Ӹ��Ƹ��Դ�*/
/*���Դ�ĸ��ƿռ����ͼƬ*/
/*�ɶ�η��ã�λ��:(x, y)  ��С:tPictureInfos.iTotalBytes*/
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