#include <picformat_manager.h>
#include <file.h>
#include <stdlib.h>
#include <string.h>

/*加入这个属性会让结构体以字节对齐*/
/*默认会以最大的字节数对齐*/
/*bmp文件头*/
struct tagBITMAPFILEHEADER { /* bmfh */
	unsigned short bfType; 
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned long  bfOffBits;
}__attribute__((packed));	
typedef struct tagBITMAPFILEHEADER BITMAPFILEHEADER;
/*bmp信息头*/
struct tagBITMAPINFOHEADER { /* bmih */
	unsigned long  biSize;
	unsigned long  biWidth;
	unsigned long  biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long  biCompression;
	unsigned long  biSizeImage;
	unsigned long  biXPelsPerMeter;
	unsigned long  biYPelsPerMeter;
	unsigned long  biClrUsed;
	unsigned long  biClrImportant;
}__attribute__((packed));
typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;

static int isBMPFormat(PT_FileInfos ptFileInfos);
static int GetPixelDatasFrmBMPFile(PT_FileInfos ptFileInfos, PT_pictureInfos pictureInfos, T_ZoonInfos tZoonInfos);
static int FreePixelDatasForBMP(PT_pictureInfos pictureInfos);

/*Every picture file has a parser*/
static T_PictureFileParser g_tBMPParser = {
	.name 		  = "bmp",
	.isSupport	  = isBMPFormat,
	.GetPixelDatas  = GetPixelDatasFrmBMPFile,
	.FreePixelDatas = FreePixelDatasForBMP,
};

/*Is it a bmp file? judgement the header of file*/
static int isBMPFormat(PT_FileInfos ptFileInfos)
{
	unsigned char *pucFileHead = ptFileInfos->pucFileMapMemery;
	if (pucFileHead[0] != 0x42 || pucFileHead[1] != 0x4d) { 
		return 0;
	}
	else {
		return 1;
	}
}

//
static int ZoonBmpPicture(PT_pictureInfos pictureInfos)
{
	int x, y, PosX, PosY;
	int SourceWidth, BytesPerPixel;
	unsigned char *NewDataAddress, *pNew;
	unsigned char *RecordIndex;
	float ZoonCoefficient = (float)pictureInfos->addNumber / (float)pictureInfos->declineNumber;
	int NewWidth = pictureInfos->iWidth * ZoonCoefficient;
	int NewHeight = pictureInfos->iHeight * ZoonCoefficient;
	SourceWidth = pictureInfos->iWidth;
	BytesPerPixel = pictureInfos->iBpp/8;
	RecordIndex = malloc(NewWidth);
	NewDataAddress = malloc(NewWidth * NewHeight * BytesPerPixel);
	pNew = NewDataAddress;
	if (NewDataAddress == NULL || RecordIndex == NULL) {
		printf("Malloc fail!\n");
		return -1;
	}
	for (x = 0; x < NewWidth; x++) {
		RecordIndex[x] = x * SourceWidth / NewWidth;
	}
	for (y = 0; y < NewHeight; y++) {
		PosY = y * pictureInfos->iHeight / NewHeight;
		for (x = 0; x < NewWidth; x++) {
			PosX = RecordIndex[x];
			memcpy(pNew, pictureInfos->pucDataAddress + PosY * SourceWidth * BytesPerPixel + PosX * BytesPerPixel, BytesPerPixel);
			pNew += BytesPerPixel;
		}
	}
	free(RecordIndex);
	pictureInfos->iWidth = NewWidth;
	pictureInfos->iHeight = NewHeight;
	pictureInfos->iLineBytes = NewWidth * BytesPerPixel;
	pictureInfos->iTotalBytes = NewWidth * NewHeight * BytesPerPixel;
	free(pictureInfos->pucDataAddress);
	pictureInfos->pucDataAddress = NewDataAddress;
	return 0;
}

/*Get the file detail infomation and the available datas to display*/
static int GetPixelDatasFrmBMPFile(PT_FileInfos ptFileInfos, PT_pictureInfos pictureInfos, T_ZoonInfos tZoonInfos)
{
	int x, y;	
	int SourcePictureBpp;
	int iLineAlignBytes;
	unsigned char *pucFileHead;
	unsigned char *pucSrc;
	unsigned char *pucDest;	
	
	BITMAPFILEHEADER *ptBITMAPFILEHEADER;
	BITMAPINFOHEADER *ptBITMAPINFOHEADER;

	pucFileHead = ptFileInfos->pucFileMapMemery;

	ptBITMAPFILEHEADER  = (BITMAPFILEHEADER *)pucFileHead;
	ptBITMAPINFOHEADER = (BITMAPINFOHEADER *)(pucFileHead + sizeof(BITMAPFILEHEADER));	
	SourcePictureBpp = ptBITMAPINFOHEADER->biBitCount;
	if (SourcePictureBpp != 24) {
		printf("Can not support this format\n");
		return -1;
	}

	pictureInfos->iWidth  = ptBITMAPINFOHEADER->biWidth;
	pictureInfos->iHeight = ptBITMAPINFOHEADER->biHeight;
	pictureInfos->iLineBytes  = pictureInfos->iWidth * pictureInfos->iBpp / 8;
	pictureInfos->iTotalBytes = pictureInfos->iHeight * pictureInfos->iLineBytes;
	/*Malloc a memery to storage the handled datas*/
	pictureInfos->pucDataAddress = (unsigned char *)malloc(pictureInfos->iTotalBytes);
	if (NULL == pictureInfos->pucDataAddress) {
		printf("Malloc fail test\n");
		return -1;
	}	

	/*向4取整,bmp 文件是以4字节对齐的，可能有无用的冗余数据*/
	iLineAlignBytes = (pictureInfos->iWidth * SourcePictureBpp/8 + 3) & ~0x03;	
	pucSrc   = (pucFileHead + ptBITMAPFILEHEADER->bfOffBits) + (pictureInfos->iHeight - 1)*iLineAlignBytes;
	pucDest = pictureInfos->pucDataAddress;

	for (y = 0; y < pictureInfos->iHeight; y++) {
		/*iWidth:原 图片的Bpp  | pictureInfos->iBpp:显存支持的Bpp*/
		ConvertFormatForOneLine(pictureInfos->iWidth, SourcePictureBpp, pictureInfos->iBpp, pucDest, pucSrc, "bmp");
		pucSrc   -= iLineAlignBytes;
		pucDest += pictureInfos->iLineBytes;
	}
	if (ZoonHandler(tZoonInfos, pictureInfos)) {
		ZoonBmpPicture(pictureInfos);
	}

	return 0;
}

static int FreePixelDatasForBMP(PT_pictureInfos pictureInfos)
{
	free(pictureInfos->pucDataAddress);
	return 0;	
}

int BMPParserInit(void)
{
	return RegisterPicFileParser(&g_tBMPParser);
}	
