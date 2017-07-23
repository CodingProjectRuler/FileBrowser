/*获取编码*/
#include <encoding_manager.h>
#include <string.h>

static int AsciiCodeType(unsigned char *pucFileHead);
static int AsciiGetCodeFrmBuf(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode);
	
static T_EncodingOpr g_tASCIIEncodingOpr = {
	.name = "ascii",
	.HeadLen = 0,
	.CodeType = AsciiCodeType,
	.GetCodeFromFile = AsciiGetCodeFrmBuf,
};

static int AsciiCodeType(unsigned char *pucFileHead)
{
	const unsigned char StrUTF_8[]    = {0xEF, 0xBB, 0xBF};
	const unsigned char StrUTF_16le[] = {0xFF, 0xFE};
	const unsigned char StrUTF_16be[] = {0xFE, 0xFF};

	if(strncmp(pucFileHead, StrUTF_8, 3) == 0)
		return -1;	/*UTF-8*/
	if(strncmp(pucFileHead, StrUTF_16le, 2) == 0)
		return -1; /*UTF-16le*/
	if(strncmp(pucFileHead, StrUTF_16be, 2) == 0)
		return -1; /*UTF-16be*/
	return 0;		/*return the length of head of this type code file*/
}

/*在指定的位置获取编码值*/
static int AsciiGetCodeFrmBuf(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode)
{
	if(pucStart < pucFileEnd){
		if((*pucStart) < (unsigned char)0x80){
			*pdwCode = (unsigned int)(*pucStart);
			return 1;	/*ASCII*/
		}
		else if((pucStart + 1) < pucFileEnd){
			*pdwCode = ((unsigned int)(*pucStart) << 8) | *(pucStart + 1);
			return 2;
		}
		else return 0;		
	}else return 0;
}

int AsciiEncodingRegister(void)
{
	AddFontOprForEncoding(&g_tASCIIEncodingOpr, GetFontOpr("ascii"));
	AddFontOprForEncoding(&g_tASCIIEncodingOpr, GetFontOpr("gbk"));	
	return RegisterEncodingOpr(&g_tASCIIEncodingOpr);
}

