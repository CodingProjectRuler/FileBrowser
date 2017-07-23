
#include<font_manager.h>
#include <ft2build.h>
#include <stdio.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

static FT_Library g_tLibrary;
static FT_Face g_tFace;
static FT_GlyphSlot g_tSlot;

/*一个传输作用*/
static unsigned int g_dwFontSize;

static int FreeTypeFontInit(char *pcFontFile, unsigned int dwFontSize);
static int FreeTypeGetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap);

static T_FontOpr g_tFreeTypeFontOpr = {
	.name = "freetype",
	.FontInit = FreeTypeFontInit,
	.GetFontBitmap = FreeTypeGetFontBitmap,
};


static int FreeTypeFontInit(char *pcFontFile, unsigned int dwFontSize)
{
	int iError;

	if(pcFontFile == NULL)
		return -1;
	
	iError = FT_Init_FreeType( &g_tLibrary );			   /* initialize library */
	/* error handling omitted */
	if(iError){
		printf("FT_Init_FreeType Failed\n");
		return -1;
	}
	
	iError = FT_New_Face( g_tLibrary, pcFontFile, 0, &g_tFace ); /* create face object */
	/* error handling omitted */	
	if(iError){
		printf("FT_New_Face Failed\n");
		return  -1;
	}

	g_tSlot = g_tFace->glyph;

	iError = FT_Set_Pixel_Sizes(g_tFace, dwFontSize, 0);
	if(iError){
		printf("FT_Set_Sizes Failed\n");
		return -1;
	}
	g_dwFontSize = dwFontSize;
	printf("FreeType\n");
	return 0;
}
static int FreeTypeGetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap)
{
	int iError;
	static int i = 0;
	/*FT_LOAD_MONOCHROME:一个像素用一个bit表示*/
       iError = FT_Load_Char( g_tFace, dwCode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
	if (iError){
		printf("FT_Load_Char error\n");
		return -1;
	}


/*根据码值得到字体数据并设置位图的显示属性*/
	ptFontBitMap->iLeft = ptFontBitMap->iX + g_tSlot->bitmap_left;
	ptFontBitMap->iTop = ptFontBitMap->iY + g_dwFontSize - g_tSlot->bitmap_top;
	ptFontBitMap->iXmax = ptFontBitMap->iLeft + g_tSlot->bitmap.width;
	ptFontBitMap->iYmax = ptFontBitMap->iTop + g_tSlot->bitmap.rows;
	ptFontBitMap->iNextX = ptFontBitMap->iX + g_tSlot->advance.x / 64;
	ptFontBitMap->iNextY = ptFontBitMap->iY;
	ptFontBitMap->iBpp = 1;
	ptFontBitMap->iPitch = g_tSlot->bitmap.pitch;
	ptFontBitMap->pucBuffer = g_tSlot->bitmap.buffer;	
	return 0;
}

int FreeTypeRegister(void)
{
	return RegisterFontOpr(&g_tFreeTypeFontOpr);
}


