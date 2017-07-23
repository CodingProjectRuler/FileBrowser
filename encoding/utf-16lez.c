
#include <encoding_manager.h>
#include <string.h>

static int UTF_16leCodeType(unsigned char *pucFileHead);
static int UTF_16leGetCodeFrmBuf(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode);


static T_EncodingOpr g_tUTF_16leEncodingOpr = {
	.name          = "utf-16le",
	.HeadLen	   = 2,
	.CodeType     = UTF_16leCodeType,
	.GetCodeFromFile = UTF_16leGetCodeFrmBuf,
};

static int UTF_16leCodeType(unsigned char *pucFileHead)
{
	const unsigned char StrUTF_16le[] = {0xFF, 0xFE};
	if(strncmp(pucFileHead, StrUTF_16le, 2) == 0)
		return 0;	/*UTF-16le¿‡–Õ*/ 
	else
		return -1;
	
}
static int UTF_16leGetCodeFrmBuf(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode)
{
	if((pucStart + 1) < pucFileEnd){
		*pdwCode = ((*pucStart)) | ((unsigned int)(*(pucStart + 1)) << 8);
		return 2;
	}else return 0;
}
int UTF_16leEncodingRegister(void)
{
	AddFontOprForEncoding(&g_tUTF_16leEncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUTF_16leEncodingOpr, GetFontOpr("ascii"));	
	return RegisterEncodingOpr(&g_tUTF_16leEncodingOpr);
}





