#ifndef _FONT_MANAGER_H
#define _FONT_MANAGER_H

typedef struct FontBitMap{
	int iLeft;
	int iTop;
	int iXmax;
	int iYmax;
	int iBpp;
	int iPitch;
	int iX;	/*x,yµÄ»ù×¼|-*/
	int iY;
	int iNextX;
	int iNextY;
	unsigned char *pucBuffer;
}T_FontBitMap, *PT_FontBitMap;

typedef struct FontOpr{
	char *name;
	int (*FontInit)(char *pcFontFile, unsigned int dwFontSize);
	int (*GetFontBitmap)(unsigned int dwCode, PT_FontBitMap ptFontBitMap);
	struct FontOpr * ptNext;
}T_FontOpr, *PT_FontOpr;

int RegisterFontOpr(PT_FontOpr ptFontOpr);
int ASCIIRegister(void);
int GBKRegister(void);
int FreeTypeRegister(void);
int FontsRegister(void);
PT_FontOpr GetFontOpr(char *pcName);


#endif


