#ifndef _DRAWTXT_H
#define _DRAWTXT_H
#include <font_manager.h>
#include <display_manager.h>
#define COLOR_BACKGROUND   0xE7DBB5  /* ·º»ÆµÄÖ½ */
#define COLOR_FOREGROUND   0x514438  /* ºÖÉ«×ÖÌå */
int RegisterList(void);
int ExitWork(void);
int OpenTextFile(char *pcFileName);
int InitEncoding_Fonts(char *pucHZKFile, char *pucFreeTypeFile, unsigned int dwFontSize);
int SelectAndInitDisplay(char *pcName);
int ShowOneFont(PT_FontBitMap ptFontBitMap);
int ShowNextPage(void);
int ShowPrePage(void);
int CopyPointer(PT_DispOpr ptDisOpr);
int MunmapAndCloseFile(void);

#endif




