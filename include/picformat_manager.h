#ifndef _PICFORMAT_MANAGER_H
#define _PICFORMAT_MANAGER_H

#include <file.h>
/*��ʱ���ı��ݺ�ȵ�����*/
typedef struct ZoonInfos {
	int addNumber;		/*�Ŵ�ϵ��*/
	int declineNumber;		/*��Сϵ��*/
	int Staus;				/*����������״̬*/
}T_ZoonInfos, *PT_ZoonInfos;

/* ͼƬ����ϸ��Ϣ */
typedef struct PictureInfos {
	int iBpp;
	int iWidth;
	int iHeight;
	int iLineBytes;
	int iTotalBytes;
	int addNumber;
	int declineNumber;
	unsigned char *pucDataAddress;
}T_pictureInfos, *PT_pictureInfos;

/*ͼƬ�ļ�������*/
typedef struct PictureFileParser {
	char *name;
	int (*isSupport)(PT_FileInfos ptFileInfos);
	int (*GetPixelDatas)(PT_FileInfos ptFileInfos, PT_pictureInfos pictureInfos, T_ZoonInfos tZoonInfos);
	int (*FreePixelDatas)(PT_pictureInfos pictureInfos);
	struct PictureFileParser *ptNext;
}T_PictureFileParser, *PT_PictureFileParser;

int ConvertFormatForOneLine(int iWidth, int picBpp, int disBpp, unsigned char *pucDest, unsigned char *pucSrc, char *name);
int ZoonHandler(T_ZoonInfos tZoonInfos, PT_pictureInfos pictureInfos);
int RegisterPicFileParser(PT_PictureFileParser ptPictureFileParser);
int BMPParserInit(void);
int JPEGParserInit(void);

#endif
