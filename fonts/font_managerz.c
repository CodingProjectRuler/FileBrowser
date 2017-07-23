
#include <font_manager.h>
#include <string.h>

static PT_FontOpr g_ptFontOprHead;

int RegisterFontOpr(PT_FontOpr ptFontOpr)
{
	PT_FontOpr ptTemp;
	ptFontOpr->ptNext = NULL;
	
	if(g_ptFontOprHead == NULL)
		g_ptFontOprHead = ptFontOpr;
	else{
		ptTemp = g_ptFontOprHead;
		while(ptTemp->ptNext != NULL)
			ptTemp = ptTemp->ptNext;
		ptTemp->ptNext = ptFontOpr;
	}
	return 0;
}

PT_FontOpr GetFontOpr(char *pcName)
{
	PT_FontOpr ptTemp;
	ptTemp = g_ptFontOprHead;
	
	while(ptTemp != NULL){
		if(strcmp(ptTemp->name, pcName) == 0)
			return ptTemp;
		ptTemp = ptTemp->ptNext;
	}
	return NULL;
}

int FontsRegister(void)
{
	ASCIIRegister();

	GBKRegister();

	FreeTypeRegister();

	return 0;
}


