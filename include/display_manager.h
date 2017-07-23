#ifndef _DISPLAY_MANAGER_H
#define _DISPLAY_MANAGER_H
#include <drawpicture.h>

typedef struct DispOpr{
	char *name;
	int iXres;
	int iYres;
	int iBpp;
	int iLineWidthBytes;
	unsigned char *pucDispMem;	/*ÏÔ´æµÄµØÖ·*/
	int (*DeviceInit)(void);
	int (*ShowOnePixel)(int iPenX, int iPenY, unsigned int dwColor);
	int (*ShowCompleteOnePage)(unsigned char *OnePagePixelDatasAddr);
	int (*CleanScreen)(unsigned int dwBackColor);
	struct DispOpr *ptNext;
}T_DispOpr, *PT_DispOpr;

int RegisterDispOpr(PT_DispOpr ptDispOpr);
PT_DispOpr GetDispOpr(char *pcName);
int FBRegister(void);
int DisplayRegister(void);

#endif
