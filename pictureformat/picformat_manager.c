#include <picformat_manager.h>
#define SCREEN_HEIGHT 272
#define SCREEN_WIDTH 480
#define ONE_PAGE_ICON_NUMBERS 15
#define DIR_STATUS 0
#define TXT_STATUS 1
#define PIC_STATUS 2
#define ZOOM_STATUS 3
static PT_PictureFileParser g_ptPictureFileParserHeader;

int ConvertFormatForOneLine(int iWidth, int picBpp, int disBpp, unsigned char *pucDest, unsigned char *pucSrc, char *name)
{
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int Color;
	int i, pointer = 0;
	
	unsigned short *Bpp16 = (unsigned short *)pucDest;
	unsigned int *Bpp32 = (unsigned int *)pucDest;
	if (disBpp == 24) {
		memcpy(pucDest, pucSrc, iWidth*3);
	}
	else {
		/*handle every pixel one by one*/
		for (i = 0; i < iWidth; i++) {
			if (name == "bmp") {
				blue = pucSrc[pointer++];
				green = pucSrc[pointer++];				
				red = pucSrc[pointer++];
			}
			else if (name == "jpeg"){
				red = pucSrc[pointer++];
				green = pucSrc[pointer++];				
				blue = pucSrc[pointer++];				
			}
			if (disBpp == 16) {
				Color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >>3);
				*Bpp16 = Color;
				Bpp16++;
			}
			else if (disBpp == 32) {
				Color = (red << 16) | (green << 8) | (blue);
				*Bpp32 = Color;
				Bpp32++;
			}
		}
	}
	return 0;
}

/*根据用户需求决定压缩系数*/
/*把决定的系数写入pictureInfos指向的结构体中，供GetPixelDatas函数调用*/
int ZoonHandler(T_ZoonInfos tZoonInfos, PT_pictureInfos pictureInfos)
{
	int Number;
	static T_ZoonInfos sTZoonInfos;
	if (tZoonInfos.Staus == DIR_STATUS || tZoonInfos.Staus == ZOOM_STATUS) {
		pictureInfos->addNumber = tZoonInfos.addNumber;
		pictureInfos->declineNumber = tZoonInfos.declineNumber;
		if (pictureInfos->addNumber == pictureInfos->declineNumber) {
			/*不用缩放*/
			return 0;
		}
		else {
			/*需要缩放*/
			return 1;
		}
	}
	/*处理PIC_STATUS分支*/
	/*图片打开的一瞬间只会缩小，不会进行放大*/
	if (pictureInfos->iWidth < SCREEN_WIDTH && pictureInfos->iHeight < SCREEN_HEIGHT) {
		/*不用缩放*/
		pictureInfos->addNumber = pictureInfos->declineNumber = 1;
		return 0;
	}
	Number = pictureInfos->iWidth / SCREEN_WIDTH;
/*	if (pictureInfos->iWidth % SCREEN_WIDTH > 0) {
		Number++;
	}*/	
	if (Number < pictureInfos->iHeight / SCREEN_HEIGHT) {
		Number = pictureInfos->iHeight / SCREEN_HEIGHT;
	}
	tZoonInfos.addNumber = 1;
	tZoonInfos.declineNumber = Number;
	pictureInfos->addNumber = tZoonInfos.addNumber;
	pictureInfos->declineNumber = tZoonInfos.declineNumber;	
	if (pictureInfos->addNumber == pictureInfos->declineNumber) {
		/*不用缩放*/
		return 0;
	}
	else {
		/*需要缩放*/
		return 1;
	}
}

int RegisterPicFileParser(PT_PictureFileParser ptPictureFileParser)
{
	PT_PictureFileParser ptTemp;
	ptPictureFileParser->ptNext = NULL;
	if(g_ptPictureFileParserHeader == NULL)
		g_ptPictureFileParserHeader = ptPictureFileParser;
	else{
		ptTemp = g_ptPictureFileParserHeader;
		while (ptTemp->ptNext != NULL)
			ptTemp = ptTemp->ptNext;
		ptTemp->ptNext = ptPictureFileParser;	
	}
	return 0;	
}

void ShowSupportPicFormat(void)
{
	int i;
	PT_PictureFileParser ptTemp = g_ptPictureFileParserHeader;
	while (ptTemp) {
		printf("%02d %s", ++i, ptTemp->name);
		ptTemp = ptTemp->ptNext;
	}
}

PT_PictureFileParser GetParserByName(unsigned char *pucName)
{
	PT_PictureFileParser ptTemp = g_ptPictureFileParserHeader;
	while (ptTemp) {
		if (strcmp(pucName, ptTemp->name) == 0) {
			return ptTemp;
		}
		ptTemp = ptTemp->ptNext;
	}
	return NULL;
}

PT_PictureFileParser GetParserByFile(PT_FileInfos ptFileInfos)
{
	PT_PictureFileParser ptTemp = g_ptPictureFileParserHeader;
	while (ptTemp) {
		if (ptTemp->isSupport(ptFileInfos)) {
			return ptTemp;
		}
		ptTemp = ptTemp->ptNext;
	}
	return NULL;	
}

int PictureFileParserInit(void) 
{
	int Error;
	int iError;

	iError = BMPParserInit();
	iError |= JPEGParserInit();
		
	return iError;
}