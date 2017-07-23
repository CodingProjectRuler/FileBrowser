
#include <encoding_manager.h>
#include <string.h>

static int UTF_16beCodeType(unsigned char *pucFileHead);
static int UTF_16beGetCodeFrmBuf(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode);


static T_EncodingOpr g_tUTF_16beEncodingOpr = {
	.name          = "utf-16be",
	.HeadLen	   = 2,
	.CodeType     = UTF_16beCodeType,
	.GetCodeFromFile = UTF_16beGetCodeFrmBuf,
};

static int UTF_16beCodeType(unsigned char *pucFileHead)
{
	const unsigned char StrUTF_16be[] = {0xFE, 0xFF};
	if(strncmp(pucFileHead, StrUTF_16be, 2) == 0)
		return 0;	/*UTF-16be¿‡–Õ*/
	else
		return -1;
	
}
static int UTF_16beGetCodeFrmBuf(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode)
{
	if((pucStart + 1) < pucFileEnd){
		*pdwCode = ((unsigned int)(*pucStart) << 8) | *(pucStart + 1);
		return 2;
	}else return 0;
}
int UTF_16beEncodingRegister(void)
{
	AddFontOprForEncoding(&g_tUTF_16beEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUTF_16beEncodingOpr, GetFontOpr("ascii"));	
	return RegisterEncodingOpr(&g_tUTF_16beEncodingOpr);
}

