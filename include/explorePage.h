#ifndef _EXPLOREPAGE_H
#define _EXPLOREPAGE_H
#include <font_manager.h>
typedef struct ItemIconAndNameInfos {
	int pages;
	int leftIems;
	int dirNumbers;		/*ͼ������ǰ�沢��ͼ�궼��һ���ģ����Բ���Ҫһ��һ���ж��ļ�����*/
	PT_FontOpr ptFileNameFontOpr;	
}T_ItemIconAndNameInfos, *PT_ItemIconAndNameInfos;

int ShowNextOnePageForItems(void);
int ShowPreOnePageForItems(void);	
int RunRefreshPage(void) ;

#endif
