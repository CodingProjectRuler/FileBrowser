
#include <encoding_manager.h>
#include <string.h>
#include <stdlib.h>

static PT_EncodingOpr g_ptEncodingOprHead;

int RegisterEncodingOpr(PT_EncodingOpr ptEncodingOpr)
{
	PT_EncodingOpr ptTemp;

	ptEncodingOpr->ptNext = NULL;
	
	if(g_ptEncodingOprHead == NULL)
		g_ptEncodingOprHead = ptEncodingOpr;
	else{
		ptTemp = g_ptEncodingOprHead;
		while(ptTemp->ptNext != NULL)
			ptTemp = ptTemp->ptNext;
		ptTemp->ptNext = ptEncodingOpr;
	}
	return 0;
}

PT_EncodingOpr SelectEncodingForFile(unsigned char *pucFileHead)
{
	PT_EncodingOpr ptTemp;
	
	ptTemp = g_ptEncodingOprHead;

	while(ptTemp != NULL){
		if(ptTemp->CodeType(pucFileHead) == 0)
			return ptTemp;
		else 
			ptTemp = ptTemp->ptNext;
	}
	return NULL;
}

int AddFontOprForEncoding(PT_EncodingOpr ptEncodingOpr, PT_FontOpr ptFontOpr)
{
	/*这个函数会破坏原来的FontOpr链表
	，所以需要重新复制一份一样的结构体*/

	PT_FontOpr CptFontOpr;

	CptFontOpr = malloc(sizeof(T_FontOpr));
	if(CptFontOpr == NULL)
		return -1;		/*Alloc Failed*/
	memcpy(CptFontOpr, ptFontOpr, sizeof(T_FontOpr));
	CptFontOpr->ptNext = ptEncodingOpr->ptFontOprSupportedHead;
	ptEncodingOpr->ptFontOprSupportedHead = CptFontOpr;
/*	
	PT_FontOpr ptTemp;

	if(ptEncodingOpr->ptFontOprSupportedHead == NULL)
		ptEncodingOpr->ptFontOprSupportedHead = CptFontOpr;
	else{
		ptTemp = ptEncodingOpr->ptFontOprSupportedHead;
		while(ptTemp->ptNext != NULL)
			ptTemp = ptTemp->ptNext;
		ptTemp->ptNext = CptFontOpr;
	}	
*/
}
/*Free all memeory malloced above, avoiding to leaking memory  */
/*
int DelFontOprForEncoding(PT_EncodingOpr ptEncodingOpr)
{
	PT_FontOpr ptFontOpr;
	ptFontOpr = ptEncodingOpr->ptFontOprSupportedHead;
	while(ptFontOpr != NULL){
		free(ptFontOpr);
		ptFontOpr = ptFontOpr->next;
	}
	return 0;
}
*/

int DelFontOprForEncoding()
{
	PT_EncodingOpr ptTemp;	
	PT_FontOpr ptFontOpr;
	ptTemp = g_ptEncodingOprHead;
	while(ptTemp != NULL){
		ptFontOpr = ptTemp->ptFontOprSupportedHead;
		while(ptFontOpr != NULL){
			free(ptFontOpr);
			ptFontOpr = ptFontOpr->ptNext;
		}
		ptTemp = ptTemp->ptNext;
	}
	return 0;
}

int EncodingRegister(void)
{
	int iError;
	iError = AsciiEncodingRegister();
	iError = UTF_8EncodingRegister();
	iError = UTF_16leEncodingRegister();
	iError = UTF_16beEncodingRegister();	

	return iError;
}

