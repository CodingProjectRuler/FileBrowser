#ifndef _DRAWPICTURE_H
#define _DRAWPICTURE_H
#include <font_manager.h>
#include <picformat_manager.h>

typedef struct PosXY {
	int PosX;
	int PosY;
}T_PosXY, *PT_PosXY;

int RegisterListp(void);
int SelectPicformatParserForFile(char *strFileName);
int GetPixelDatasFrmFileInfos(PT_pictureInfos *ptPictureInfos, T_ZoonInfos tZoonInfos);
void FreePixelDataMemery(void);
int SelectAndInitDisplayp(char *pcName);
int RealseFile(void);
int TestPosition(PT_PosXY ptPosXY);
int CreateAndShowDisplayDatas(T_PosXY tPosXY);
int MallocCopyOfVideoMemery(void);
void ShowOnePicture(void);
int ShowOneFontP(PT_FontBitMap ptFontBitMap);
void CleanScreen(unsigned int dwBackColor);
#endif /* _DRAWPICTURE_H */


