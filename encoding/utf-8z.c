
#include <encoding_manager.h>
#include <string.h>

static int UTF_8CodeType(unsigned char *pucFileHead);
static int UTF_8GetCodeFromFile(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode);

static T_EncodingOpr g_tUTF_8EncodingOpr = {
	.name = "utf-8",
	.HeadLen = 3,
	.CodeType = UTF_8CodeType,
	.GetCodeFromFile = UTF_8GetCodeFromFile,
};

static int UTF_8CodeType(unsigned char *pucFileHead)
{
	const unsigned char StrUTF_8[]    = {0xEF, 0xBB, 0xBF};
	if(strncmp(pucFileHead, StrUTF_8, 3) == 0)
		return 0;	/*UTF-8ÀàÐÍ*/
	else
		return -1;
}

static int GetPreOneBits(unsigned char ucVal)
{
	int i;
	int num = 0;
	
	for(i = 7; i >= 0; i--){
		if(!(ucVal & (1<< i)))
			break;
		else num++;
	}
	return num;
}

static int UTF_8GetCodeFromFile(unsigned char *pucStart, unsigned char *pucFileEnd, unsigned int *pdwCode)
{
	int i;	
	int iNum;
	unsigned char ucVal;
	unsigned int dwSum = 0;
	
	if(pucStart < pucFileEnd){
		ucVal = *pucStart;
		iNum = GetPreOneBits(*pucStart);
		if((pucStart + iNum) > pucFileEnd)
			return 0;
		if(iNum == 0){
			*pdwCode = *pucStart;
			return 1;
		}else{
			ucVal = ucVal << iNum;
			ucVal = ucVal >> iNum;
			dwSum += ucVal;
			for (i = 1; i < iNum; i++){
				ucVal = (*(pucStart+i)) & 0x3f;
				dwSum = dwSum << 6;
				dwSum += ucVal;			
			}
			*pdwCode = dwSum;
			return iNum;
		}	
	}else return 0;
}

int UTF_8EncodingRegister(void)
{
	AddFontOprForEncoding(&g_tUTF_8EncodingOpr, GetFontOpr("freetype"));
	AddFontOprForEncoding(&g_tUTF_8EncodingOpr, GetFontOpr("ascii"));	
	return RegisterEncodingOpr(&g_tUTF_8EncodingOpr);
}

