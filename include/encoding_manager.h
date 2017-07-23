#ifndef _ENCODING_MANAGER_H
#define _ENCODING_MANAGER_H

#include <font_manager.h>

typedef struct EncodingOpr{
	char *name;
	int HeadLen;
	PT_FontOpr ptFontOprSupportedHead;
	int (*CodeType)(unsigned char *pucFileHead);
	int (*GetCodeFromFile)(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode);
	struct EncodingOpr *ptNext; 
}T_EncodingOpr, *PT_EncodingOpr;


int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr);
int AsciiEncodingRegister(void);
int UTF_8EncodingRegister(void);
int UTF_16leEncodingRegister(void);
int UTF_16beEncodingRegister(void);
int EncodingRegister(void);
PT_EncodingOpr SelectEncodingForFile(unsigned char *pucFileHead);
int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr);
int DelFontOprForEncoding(void);

#endif


