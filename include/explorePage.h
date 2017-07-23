#ifndef _EXPLOREPAGE_H
#define _EXPLOREPAGE_H
#include <font_manager.h>
typedef struct ItemIconAndNameInfos {
	int pages;
	int leftIems;
	int dirNumbers;		/*图标在最前面并且图标都是一样的，所以不需要一个一个判断文件类型*/
	PT_FontOpr ptFileNameFontOpr;	
}T_ItemIconAndNameInfos, *PT_ItemIconAndNameInfos;

int ShowNextOnePageForItems(void);
int ShowPreOnePageForItems(void);	
int RunRefreshPage(void) ;

#endif
