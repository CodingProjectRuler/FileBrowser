/*��ʾ��ǰ����Ŀ¼ҳ��*/
#include <explorePage.h>
#include <file.h>
#include <drawpicture.h>
#include <drawtxt.h>
#include <font_manager.h>
#include <input_manager.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define SCREEN_HEIGHT 272
#define SCREEN_WIDTH 480
#define ONE_PAGE_ICON_NUMBERS 15
#define DIR_STATUS 0
#define TXT_STATUS 1
#define PIC_STATUS 2
#define ZOOM_STATUS 3
#define MOVE_PICTURE_POSITION 31
#define TWICE_TOUCH 32
#define SLIP_MIN_DISTANCE (2*2)

static PT_DirContent *g_aptDirContents;	/*ָ���ָ����ָ�򶥲�Ŀ¼��������ֺ�����*/
static int g_iDirContentsNumber;			/*����Ŀ¼�е���Ŀ��*/

static T_ItemIconAndNameInfos g_tItemIconAndNameInfos;
static int g_PageIndex = 0;

char DefaultDir[256] = "/";
char CopyDefaultDir[256];
const char *PathOfIcons[] = {
	"/IconsForFileBrowser/fold_closed.bmp",	
	"/IconsForFileBrowser/txticon.bmp",
	"/IconsForFileBrowser/bmpicon.bmp",	
	"/IconsForFileBrowser/jpeginco.bmp",
	"/IconsForFileBrowser/jpeginco.bmp",	
	"/IconsForFileBrowser/unknowicon.bmp",	
};

static char *MenuIconsName[] = {
	"/IconsForFileBrowser/up.bmp",
	"/IconsForFileBrowser/select.bmp",	
	"/IconsForFileBrowser/pre_page.bmp",
	"/IconsForFileBrowser/next_page.bmp",
};

static const int startPosForMenuOneList[4][2] = {
		{0, 0},
		{0, 68},
		{0, 136},
		{0, 204},
};
/*һҳ��15����Ŀ*/
/*ÿ����Ŀ��ʼ�����*/
/*ÿ����Ŀ�Ĵ�СΪ42x42*/
/*ÿ����Ŀ���������Ӧ����㿪ʼ����СΪ80x85�ľ�������ʾ*/
static const int startPosForItemsList[15][2] = {
		{90, 20},   {170, 20} ,  {250, 20},   {330, 20} ,  {410, 20}, 
		{90, 105}, {170, 105} ,{250, 105}, {330, 105} ,{410, 105}, 
		{90, 190}, {170, 190} ,{250, 190}, {330, 190} ,{410, 190}, 	
};

PT_pictureInfos ptPictureInfos;
T_PosXY g_tPosXY;

/*��ʾһ����Ŀ������*/
static int GetFontAndShowFileName(const int x, const int y, char *FileName, PT_FontOpr FileNameFontOpr)
{
	int i, FileNameLength;
	int rowNumbers;
	int leftCharNumbers;
	T_FontBitMap FileNameFontBitMap;
	FileNameFontBitMap.iY = y + 43;
	/*�ж������Ƿ���16*8*/
	FileNameFontOpr->FontInit(NULL, 16);
	
	FileNameLength = 0;
	////////////////////////
	while(FileName[FileNameLength] != '\0') {
		FileNameLength++;
	}
	
	if (FileName[0] == '/') {
		printf("ShowfileName:%s\n", FileName);
		FileNameFontBitMap.iX = x;
		FileNameFontBitMap.iY = y;
		for (i =0; i < FileNameLength; i++) {
			FileNameFontOpr->GetFontBitmap((unsigned int)FileName[i], &FileNameFontBitMap);
			ShowOneFontP(&FileNameFontBitMap);
			FileNameFontBitMap.iX += 8;
		}
		return 0;
	}
	/*�ļ�����icon���������ʾ*/
	/*һ�еĳ�����64��������8���ַ�*/
	/*ֻ����ʾ������*/
	rowNumbers = FileNameLength / 8;    	/*����*/
	leftCharNumbers = FileNameLength % 8;	/*δ��һ�е��ַ�����*/
	rowNumbers = rowNumbers > 2? 2: rowNumbers;
	/*����ʾ���������ַ�*/
	FileNameFontBitMap.iX = x -11;
	for (i =0; i < rowNumbers * 8; i++) {
		FileNameFontOpr->GetFontBitmap((unsigned int)FileName[i], &FileNameFontBitMap);
		ShowOneFontP(&FileNameFontBitMap);
		FileNameFontBitMap.iX += 8;
		if ((i + 1)% 8 == 0) {
			FileNameFontBitMap.iX = x - 11;
			FileNameFontBitMap.iY += 16;
		}	
	}
	if (rowNumbers < 2) {
		FileNameFontBitMap.iX = x -11 + (64 -(leftCharNumbers * 8))/2;
		for (i = 0; i < leftCharNumbers; i++) {	
			FileNameFontOpr->GetFontBitmap((unsigned int)FileName[i+rowNumbers * 8], &FileNameFontBitMap);
			ShowOneFontP(&FileNameFontBitMap);
			FileNameFontBitMap.iX += 8;
		}
	}
	return 0;
}


static int InitForShowIconsAndFileName(char *DirName)
{
	int i;
		
	/*��ȡ��Ŀ¼�����µ���Ŀ��Ϣ*/
	GetDirContents(DirName, &g_aptDirContents, &g_iDirContentsNumber);
	g_tItemIconAndNameInfos.pages = g_iDirContentsNumber / 15;
	g_tItemIconAndNameInfos.leftIems = g_iDirContentsNumber % 15;
	if (g_tItemIconAndNameInfos.leftIems > 0) {
		g_tItemIconAndNameInfos.pages++;
	}
	g_tItemIconAndNameInfos.ptFileNameFontOpr = GetFontOpr("ascii");			/*�õ��ɱ����ȡ����ģ��*/
	for (i = 0; i < g_iDirContentsNumber; i++) {
		if (g_aptDirContents[i]->eFileType == FILETYPE_FILE) {
			g_tItemIconAndNameInfos.dirNumbers = i;
			break;
		}
		if (i == g_iDirContentsNumber-1) {
			g_tItemIconAndNameInfos.dirNumbers = g_iDirContentsNumber;
		}
	}
	/*��ʾ����·��Show complete path*/
	GetFontAndShowFileName(80, 3 , DirName, g_tItemIconAndNameInfos.ptFileNameFontOpr);	
	return 0;
}

/*�ȼ򵥵�ͨ���ļ�����ʶ���ļ�����*/
static int identifyFileType(char *fileName)
{
	int i = 0, j;
	char *Filesuffix;
	char *supportFileTypeList[] = {".txt", ".bmp", ".jpg", ".jpeg"};
	while (*(fileName + i) != '\0' && *(fileName + i) != '.') {
		i++;
	}
	if (fileName[i] == '\0') {
		return 5;
	}
	Filesuffix = fileName + i;		/*ָ���.��ʼ���ļ���׺�ַ���ָ��*/
	for (j = 0; j < sizeof(supportFileTypeList) /sizeof(supportFileTypeList[0]); j++) {
		if (strcmp(Filesuffix, supportFileTypeList[j]) == 0) {
			return j+1;
		} 
	}
	return j+1;		/*δʶ����ļ�*/
}

/*��һҳ����Ŀ*/
int ShowNextOnePageForItems(void)
{
	g_PageIndex++;
	if (g_PageIndex >= g_tItemIconAndNameInfos.pages) {
		if (g_tItemIconAndNameInfos.pages == 0) {
			g_PageIndex = 0;
		}
		else {
			g_PageIndex = g_tItemIconAndNameInfos.pages - 1;
		}
	}
	printf("NextIndex: %d\n", g_PageIndex);
	CreatePageForDir(g_PageIndex);

}

/*��һҳ����Ŀ*/
int ShowPreOnePageForItems(void)
{
	g_PageIndex--;
	if (g_PageIndex < 0) {
		g_PageIndex = 0;
	}
	printf("PreIndex: %d\n", g_PageIndex);
	CreatePageForDir(g_PageIndex);
}

/*��һ����Ŀ��Ӧ��ͼ����ڸ����Դ���*/
/*ֻ������bmp/jpeg/jpg�ļ�*/
void PutItemOnCopyMem(char *itemName, T_ZoonInfos tZoonInfos, T_PosXY tPosXY)
{	
	SelectPicformatParserForFile(itemName);			/*���ļ�*/
	GetPixelDatasFrmFileInfos(&ptPictureInfos, tZoonInfos);		/*����ռ䣬����ѹ���Ȼ�ȡͼƬ����������*/
	if (tZoonInfos.Staus == PIC_STATUS || tZoonInfos.Staus == ZOOM_STATUS) {
		if (ptPictureInfos->iWidth < SCREEN_WIDTH) {
			tPosXY.PosX= (SCREEN_WIDTH - ptPictureInfos->iWidth) / 2;
		}
		if (ptPictureInfos->iHeight < SCREEN_HEIGHT) {
			tPosXY.PosY = (SCREEN_HEIGHT - ptPictureInfos->iHeight) / 2;
		}
	}
	/*�ŵ���Ӧ��λ��*/
	CreateAndShowDisplayDatas(tPosXY);
	if (tZoonInfos.Staus == PIC_STATUS || tZoonInfos.Staus == ZOOM_STATUS) {
		/*PIC_STATUS������Ҫ����Ƭ�������ź��ƶ�*/
		/*�����ڻص�DIR_STATUS���������*/
		/*��¼ǰһ����Ƭ���Դ��Ͽ�ʼ��λ��*/
		g_tPosXY.PosX = tPosXY.PosX;
		g_tPosXY.PosY = tPosXY.PosY;
	}
	else {
		FreePixelDataMemery();						/*�ͷſռ�*/
		RealseFile();	
	}
}

static int ShowMenu(unsigned int dwBackColor)
{
	int i;
	T_ZoonInfos tZoonInfos;
	T_PosXY tPosXY;	
	tZoonInfos.addNumber = 1;
	tZoonInfos.declineNumber = 2;
	tZoonInfos.Staus = DIR_STATUS;
	tPosXY.PosX = 0;	
	CleanScreen(dwBackColor);
	for (i = 0; i < sizeof(MenuIconsName)/sizeof(MenuIconsName[0]); i++) {
		tPosXY.PosY = i*68;
		PutItemOnCopyMem(MenuIconsName[i], tZoonInfos, tPosXY);
	}	
}

/*��ʾһҳ��Ŀ��ͼ��Ͷ�Ӧ������*/
/*��ʾ���˵������ͼ��Ͷ�Ӧ���ļ�������*/
/*Icon's Size (Width*Height) : 42x42*/
/*�������Ϊҳ���*/
static int CreatePageForDir(int PageIndex)
{
	int i;
	int indexInItemList;
	int typeId;
	int currentPageItems;	/*��ҳҪ��ʾ����Ŀ����*/
	T_ZoonInfos tZoonInfos;
	T_PosXY tPosXY;	
	tZoonInfos.addNumber = 1;
	tZoonInfos.declineNumber = 3;
	tZoonInfos.Staus = DIR_STATUS;		
	if (g_tItemIconAndNameInfos.pages == 0) {
		/*��Ŀ¼*/
		return 0;
	}
	
	/*�ȿ���ҳ��û��Ŀ¼��Ҫ��ʾ*/
	if (ONE_PAGE_ICON_NUMBERS * PageIndex < g_tItemIconAndNameInfos.dirNumbers) {
		SelectPicformatParserForFile(PathOfIcons[0]);	       /*��Ŀ¼icon�ļ�*/
		GetPixelDatasFrmFileInfos(ptPictureInfos, tZoonInfos);					/*����ռ䣬����ѹ���Ȼ�ȡͼƬ����������*/
	}

	if ((PageIndex == g_tItemIconAndNameInfos.pages - 1) && g_tItemIconAndNameInfos.leftIems > 0) {
		currentPageItems = g_tItemIconAndNameInfos.leftIems;
	}
	else {
		currentPageItems = ONE_PAGE_ICON_NUMBERS;
	}
	for (i = 0; i < currentPageItems; i++) {
		indexInItemList = i + ONE_PAGE_ICON_NUMBERS * PageIndex;
		/*Ŀ¼�Ѿ���ʾ��ϣ������������ļ����ʹ��µ�icons*/
		if (indexInItemList >= g_tItemIconAndNameInfos.dirNumbers) {
			/*�ж��ļ�����*/
			typeId = identifyFileType(g_aptDirContents[indexInItemList]->strName);
			if (i > 0) {
				FreePixelDataMemery();	/*�ͷſռ䣬ֻ�з�����Ż��ͷ�*/
				RealseFile();	/*unmap֮ǰ���ļ���ֻ��֮ǰ�򿪹��ļ��Ż�����Ϊ*/
			}
			/*�����ļ����ʹ򿪶�Ӧ��ͼ���ǰ���ͷ�֮ǰ���ļ�*/
			SelectPicformatParserForFile(PathOfIcons[typeId]);	       /*�򿪶�Ӧ�ļ�ͼ��icons*/
			GetPixelDatasFrmFileInfos(ptPictureInfos, tZoonInfos);					/*����ռ䣬����ѹ���Ȼ�ȡͼƬ����������*/
		}
		/*�ŵ���Ӧ��λ��,ֻ������һ��ԭͼ�����Դ治ͬ��λ��*/	
		tPosXY.PosX = startPosForItemsList[i][0];
		tPosXY.PosY = startPosForItemsList[i][1];
		CreateAndShowDisplayDatas(tPosXY);

		/*��ʾ��ͼ�����Ҫ��Ӧ����Ŀ����*/
		GetFontAndShowFileName(startPosForItemsList[i][0], startPosForItemsList[i][1], g_aptDirContents[indexInItemList]->strName, g_tItemIconAndNameInfos.ptFileNameFontOpr);
		
	}
	FreePixelDataMemery();	/*�ͷſռ䣬ֻ�з�����Ż��ͷ�*/
	RealseFile();	/*unmap֮ǰ���ļ���ֻ��֮ǰ�򿪹��ļ��Ż�����Ϊ*/
	
	return 0;
}

static int DistanceBetweenTwoTouchP(PT_InputEvent ptInputEvent)
{
	return ((ptInputEvent->touchStartX - ptInputEvent->touchExitX) * (ptInputEvent->touchStartX - ptInputEvent->touchExitX) +
	(ptInputEvent->touchStartY - ptInputEvent->touchExitY) * (ptInputEvent->touchStartY - ptInputEvent->touchExitY));
}

static int EventArea(int Status, PT_InputEvent ptInputEvent)
{
	int i, j;
	static bool flag = true;
	switch (Status) {
		case DIR_STATUS:
			if (ptInputEvent->Pressure != 0) {
				return -1;
			}
			if (ptInputEvent->touchExitX < 64) {
				/*X���ڲ˵�����*/
				for (i = 0; i < sizeof(startPosForMenuOneList)/sizeof(startPosForMenuOneList[0]); i++) {
					if (ptInputEvent->touchExitY > startPosForMenuOneList[i][1]  && ptInputEvent->touchExitY < startPosForMenuOneList[i][1] + 64) {
						return i+20;
					}
				}
			}
			else if (ptInputEvent->touchExitX < 90) {
				/*X������Ч����*/
				return 30;
			}
			else if (ptInputEvent->touchExitX < 452)
			{
				/*X������Ŀ����*/
				if (ptInputEvent->touchExitY < startPosForItemsList[0][1] || ptInputEvent->touchExitY >  startPosForItemsList[10][1] + 42 + 16) {
					/*Y������Ч����*/
					return 30;
				}
				/*Y������Ŀ����*/
				for (i = 0; i < 3; i++) {
					if (ptInputEvent->touchExitY > startPosForItemsList[i*5][1] && ptInputEvent->touchExitY < startPosForItemsList[i*5][1] + 58) {
						for (j = 0; j < 5; j++) {
							if (ptInputEvent->touchExitX > startPosForItemsList[i*5 + j][0] && ptInputEvent->touchExitX < startPosForItemsList[i*5 + j][0] + 42) {
								return  i*5 + j;						
							}
						}
						break;
					}
				}
			}
			return 30;		/*��Ч������*/
			break;
		case TXT_STATUS:
			if (ptInputEvent->Pressure != 0) {
				return -1;
			}
			if (ptInputEvent->touchExitX < 64 && ptInputEvent->touchExitY < 64) {
				/*������ϽǷ���*/
				return 20;
			}
			else if (ptInputEvent->touchExitY < SCREEN_HEIGHT / 3) {
				return 1;
			}
			else if (ptInputEvent->touchExitY > 2*SCREEN_HEIGHT / 3) {
				return 2;
			}
			else {
				return 0;
			}
			break;
		case PIC_STATUS:
			if (ptInputEvent->Pressure == 0) {
				/*�ɿ�*/
				flag = true;
				if (ptInputEvent->touchExitX < 64 && ptInputEvent->touchExitY < 64 ) {
					/*������ϽǷ���*/
					return 20;
				}				
				if (ptInputEvent->Falg && DistanceBetweenTwoTouchP(ptInputEvent) < SLIP_MIN_DISTANCE+2) {
					/*˫���¼�*/
					return TWICE_TOUCH;
				}
			}
			else {
				if (flag) {
					ptInputEvent->touchStartX = ptInputEvent->touchExitX;
					ptInputEvent->touchStartY = ptInputEvent->touchExitY;
					flag = false;
				}
				if (DistanceBetweenTwoTouchP(ptInputEvent) > SLIP_MIN_DISTANCE) {
					/*�����ƶ��¼�*/
					return MOVE_PICTURE_POSITION;
				}
			}
			break;
		default:
			break;
	}
	return -1;
}

static int openNewDir(bool direction, char *shortDirName)
{
	int i;
	int OldDefaultNameLength;
	g_PageIndex = 0;
	if (direction) {
		/*���½���*/
		mergeFileNameToPath(DefaultDir, shortDirName);
	}
	else {
		/*���Ϸ���*/
		OldDefaultNameLength = strlen(DefaultDir);
		i = OldDefaultNameLength - 1;
		while (DefaultDir[i] != '/') {
			DefaultDir[i--] = '\0';
		}
		i = i == 0? i+1 : i;
		DefaultDir[i] = '\0';
	}
	printf("DIRNAME: %s\n", DefaultDir);
	for (i = 0; i < g_iDirContentsNumber; i++) {
		free(g_aptDirContents[i]);
	}
	free(g_aptDirContents);
	/*��ʾ�˵�*/
	ShowMenu(0xff);
	InitForShowIconsAndFileName(DefaultDir);		
	ShowPreOnePageForItems();
	ShowOnePicture();		/*ˢ���豸��ȥ*/
	return 0;
}

/*���ַ���FileName����DirPath���зָ���'/'�ĺ���*/
/*û�зָ������ֶ������γ�������Ŀ¼���ļ�·��*/
static int mergeFileNameToPath(char *DirPath, char *FileName)
{
	int DirPathLength;
	int FileNameLength;
	int NewCompleteDriNameLength;
	DirPathLength = strlen(DirPath);
	FileNameLength = strlen(FileName);
	
	if (DirPath[DirPathLength - 1] != '/') {
		DirPath[DirPathLength] = '/';
		DirPathLength++;
	}
	NewCompleteDriNameLength = DirPathLength + FileNameLength;
	memcpy(DirPath+DirPathLength, FileName, FileNameLength);
	DirPath[NewCompleteDriNameLength] = '\0';
	return 0;
}

/*�򿪿�֧�ֵ��ļ�*/
/*txt(ascii/gbk/utf-8/utf-16le/utf-16be)��bmp��jpg/jpeg*/
static int openFile(char *fileName)
{
	int fileType;
	T_ZoonInfos tZoonInfos;
	T_PosXY tPosXY;
	fileType = identifyFileType(fileName);
	if (5 == fileType) {
		/*�������ļ�����֧��*/
		printf("Can not support this type file: %s\n", fileName);
		/*���ļ�ʧ��*/
		/*ģʽ��ȻΪĿ¼״̬*/
		return 0;
	}
	/*�ڴ˽��ļ�����·��ƴ��*/
	/*�γ������Ĵ���·�����ļ���*/
	memcpy(CopyDefaultDir, DefaultDir, 256);
	mergeFileNameToPath(CopyDefaultDir, fileName);
	if (fileType == 1) {
		/*txt״̬*/
		/*���ļ�|  ӳ���ļ� | ���ݶ��ļ�ͷ�����ѡ��Ecoding*/
		printf("TXTCompletePathFile: %s\n", CopyDefaultDir);
		OpenTextFile(CopyDefaultDir);
		/*��ʼ����ѡ��Encoding��֧�ֵ�Fonts | ���ļ�| �ж��Ƿ�֧��FontSize*/
		/*һ���ı��ļ�������Ҫ����fontOpr��֧��*/
		InitEncoding_Fonts("/HZK16", "/MSYH.TTF", 24);		
		/*����*/ /*��ȡfontλͼ������ʾ*/
		ShowNextPage();
		return TXT_STATUS;
	}
	else {
		/*bmp/jpg/jpeg״̬��ͳһ����*/		
		printf("PICCompletePathFile: %s\n", CopyDefaultDir);
		tZoonInfos.Staus = PIC_STATUS;
		tPosXY.PosX = 0;
		tPosXY.PosY = 0;
		CleanScreen(0x00);
		PutItemOnCopyMem(CopyDefaultDir, tZoonInfos, tPosXY);
		ShowOnePicture();		/*ˢ���豸��ȥ*/
		return PIC_STATUS;
	}
	return 0;
}

static int ZoomMultiple(PT_ZoonInfos ptZoonInfos, int *ZoomFlag, int *TwiceTouchNumbers)
{
	ptZoonInfos->Staus = ZOOM_STATUS;
	(*TwiceTouchNumbers)++;
	if (*ZoomFlag == 1 || *ZoomFlag == 0) {
		/*�Ŵ�*/
		if (ptPictureInfos->addNumber >= ptPictureInfos->declineNumber) {
			ptPictureInfos->addNumber *= 2;
		}
		else {
			ptPictureInfos->declineNumber /= 2;
		}
	}
	else if (*ZoomFlag == -1){
		/*��С*/
		if (ptPictureInfos->addNumber > ptPictureInfos->declineNumber) {
			ptPictureInfos->addNumber /= 2;
			
		}
		else {
			ptPictureInfos->declineNumber *= 2;
		}	
	}
	ptZoonInfos->addNumber = ptPictureInfos->addNumber;
	ptZoonInfos->declineNumber = ptPictureInfos->declineNumber;
	
	if ((*ZoomFlag == 0) && *TwiceTouchNumbers == 2) {
		*ZoomFlag = -1;
		*TwiceTouchNumbers = 0;
	}
	if (*TwiceTouchNumbers == 4) {
		*ZoomFlag = -(*ZoomFlag);
		*TwiceTouchNumbers = 0;					
	}
	return 0;
}

static int handleAndResponseInputEvent(PT_InputEvent ptInputEvent)
{
	int i, touchNumber, itemIndex;
	static int NowStatus = DIR_STATUS;
	static int TwiceTouchNumbers = 0;
	static int ZoomFlag;		/*���ŵķ�����*/
	T_ZoonInfos tZoonInfos;
	T_PosXY tPosXY;
	/*��ȡ��������*/
	touchNumber = EventArea(NowStatus, ptInputEvent);
	if (touchNumber == -1) {
		return 0;
	}
	/*��Ŀ¼���¼�����������¼������֧*/
	switch (NowStatus) {
		case DIR_STATUS:
			if (touchNumber == 30) {
				/*��Ч�Ĵ���*/
				return 0;
			}
			else if (touchNumber == 22 || touchNumber == 23) {
				/*��������ҳ�л�*/			
				/*��ʾ�˵�*/
				ShowMenu(0xff);
				/*��ʾ����·��*/
				GetFontAndShowFileName(80, 3 , DefaultDir, g_tItemIconAndNameInfos.ptFileNameFontOpr);					
				/*��ʾ��Ŀ*/
				if (touchNumber == 22) {
					ShowPreOnePageForItems();
					ShowOnePicture();		/*ˢ���豸��ȥ���ͷŸ��ƿռ�*/
				}
				if (touchNumber == 23) {
					ShowNextOnePageForItems();
					ShowOnePicture();		/*ˢ���豸��ȥ���ͷŸ��ƿռ�*/
				}
			}
			else if (touchNumber == 20 || touchNumber == 21) {
				/*���غ�ѡ�������*/
				if (touchNumber == 20) {
					/*������һ��Ŀ¼*/
					openNewDir(false, NULL);
				}
				return 0;
			}
			else {
				/*�����Ŀ*/
				if (g_PageIndex == g_tItemIconAndNameInfos.pages - 1 && touchNumber > g_tItemIconAndNameInfos.leftIems - 1) {
					/*��һ�����������Ĵ������򳬹���Ŀ����*/
					return 0;
				}
				itemIndex = g_PageIndex*ONE_PAGE_ICON_NUMBERS + touchNumber;
				if (g_aptDirContents[itemIndex]->eFileType == FILETYPE_FILE) {
					/*��������Ŀ���ļ�*/
					NowStatus = openFile(g_aptDirContents[itemIndex]->strName);
					if (NowStatus == PIC_STATUS) {
						ZoomFlag = 0;
						TwiceTouchNumbers = 0;	
					}
					return 0;
				}
				else if (g_aptDirContents[itemIndex]->eFileType == FILETYPE_DIR) {
					/*��������Ŀ��Ŀ¼*/
					/*�򿪽���Ŀ¼����Ŀ¼*/
					openNewDir(true, g_aptDirContents[itemIndex]->strName);
					return 0;
				}
			}
			break;
		case TXT_STATUS:
			if (touchNumber == 20) {
				/*�ر�TXT�ļ�*/
				MunmapAndCloseFile();
				/*����Ŀ¼״̬*/
				NowStatus = DIR_STATUS;
				/*��������ҳ�л�*/
				/*��ʾ�˵�*/
				ShowMenu(0xff);
				/*��ʾ����·��*/
				GetFontAndShowFileName(80, 3 , DefaultDir, g_tItemIconAndNameInfos.ptFileNameFontOpr);					
				g_PageIndex++;
				ShowPreOnePageForItems();
				ShowOnePicture();		/*ˢ���豸��ȥ���ͷŸ��ƿռ�*/				
			}	
			else if (touchNumber == 0) {
				return 0;
			}
			else if (touchNumber == 1){
				/*��һҳ*/
				ShowPrePage();
			}
			else if (touchNumber == 2){
				/*��һҳ*/
				ShowNextPage();
			}
			break;
		case PIC_STATUS:
			if (touchNumber == 20) {
				/*����Ŀ¼״̬*/
				/*���ص�ʱ���ͷź͹ر��ļ�*/
				FreePixelDataMemery();	/*�ͷſռ䣬ֻ�з�����Ż��ͷ�*/
				RealseFile();	/*unmap֮ǰ���ļ���ֻ��֮ǰ�򿪹��ļ��Ż�����Ϊ*/	
				NowStatus = DIR_STATUS;
				/*��������ҳ�л�*/			
				/*��ʾ�˵�*/
				ShowMenu(0xff);
				/*��ʾ����·��*/
				GetFontAndShowFileName(80, 3 , DefaultDir, g_tItemIconAndNameInfos.ptFileNameFontOpr);					
				g_PageIndex++;
				ShowPreOnePageForItems();
				ShowOnePicture();		/*ˢ���豸��ȥ���ͷŸ��ƿռ�*/				
			}
			else if (touchNumber == MOVE_PICTURE_POSITION) {
				/*�ƶ���Ƭ����Ļ�ϵ�λ��*/
				/*��ڸ��ƿռ�*/
				tPosXY.PosX = g_tPosXY.PosX + (ptInputEvent->touchExitX - ptInputEvent->touchStartX);
				tPosXY.PosY = g_tPosXY.PosY + (ptInputEvent->touchExitY - ptInputEvent->touchStartY);
				g_tPosXY.PosX = tPosXY.PosX;
				g_tPosXY.PosY = tPosXY.PosY;
				if (!TestPosition(&g_tPosXY)) {
					CleanScreen(0x00);
				}
				CreateAndShowDisplayDatas(tPosXY);			/*�ŵ���Ӧ��λ��*/
				ShowOnePicture();		/*ˢ���豸��ȥ���ͷŸ��ƿռ�*/	
			}
			else if (touchNumber == TWICE_TOUCH) {
				/*˫�������¼�*/
				FreePixelDataMemery();	/*�ͷſռ䣬ֻ�з�����Ż��ͷ�*/
				RealseFile();	/*unmap֮ǰ���ļ���ֻ��֮ǰ�򿪹��ļ��Ż�����Ϊ*/	
				tPosXY.PosX = 0;
				tPosXY.PosY = 0;
				CleanScreen(0x00);
				ZoomMultiple(&tZoonInfos, &ZoomFlag, &TwiceTouchNumbers);
				PutItemOnCopyMem(CopyDefaultDir, tZoonInfos, tPosXY);
				ShowOnePicture();		/*ˢ���豸��ȥ*/	
			}
			break;
		default:
			break;
	}
	return 0;
}

/*���������¼�ˢ�¶�Ӧ��ҳ��*/
int RunRefreshPage(void) 
{
	int i;
	T_InputEvent tInputEvent;	/*�����¼��ṹ*/
	/*��ʾ�˵�*/
	ShowMenu(0xff);
	/*��ȡ����Ŀ¼��Ϣ*/
	InitForShowIconsAndFileName(DefaultDir);
	ShowPreOnePageForItems();
	ShowOnePicture();		/*ˢ���豸��ȥ*/		
	while(1){
		if(GetInputEventList(&tInputEvent) == 0){
			/*ִ�е�����˵���õ��������¼�*/
			/*������Ӧ�����¼�*/
			handleAndResponseInputEvent(&tInputEvent);
		}
	}		
}

#if 0
int CreatePageForDir(int PageIndex)
{
	int i;
	int x, y;
	int indexInItemList;
	int typeId;
	bool haveDir = false;
	
	/*�ȿ���ҳ��û��Ŀ¼��Ҫ��ʾ*/
	if (ONE_PAGE_ICON_NUMBERS * PageIndex < g_tItemIconAndNameInfos.dirNumbers) {
		SelectPicformatParserForFile(PathOfIcons[0]);	 /*��Ŀ¼icon�ļ�*/
		GetPixelDatasFrmFileInfos(1, 3);				/*����ռ䣬����ѹ���Ȼ�ȡͼƬ����������*/
		haveDir = true;
	}
	for (i = 0; i < ONE_PAGE_ICON_NUMBERS; i++) {
		x = i %5;
		y = i / 5;
		indexInItemList = i + ONE_PAGE_ICON_NUMBERS * PageIndex;
		/*Ŀ¼�Ѿ���ʾ��ϣ������������ļ����ʹ��µ�icons*/
		if (indexInItemList >= g_tItemIconAndNameInfos.dirNumbers) {
			/*�ж��ļ�����*/
			typeId = identifyFileType(g_aptDirContents[indexInItemList]->strName);
			if (haveDir) {
				FreePixelDataMemery();	/*�ͷſռ䣬ֻ�з�����Ż��ͷ�*/
				RealseFile();	/*unmap֮ǰ���ļ���ֻ��֮ǰ�򿪹��ļ��Ż�����Ϊ*/
			}
			/*�����ļ����ʹ򿪶�Ӧ��ͼ���ǰ���ͷ�֮ǰ���ļ�*/
			SelectPicformatParserForFile(PathOfIcons[typeId]);	       /*�򿪶�Ӧ�ļ�ͼ��icons*/
			GetPixelDatasFrmFileInfos(1, 3);					/*����ռ䣬����ѹ���Ȼ�ȡͼƬ����������*/
		}
		/*�ŵ���Ӧ��λ��,ֻ������һ��ԭͼ�����Դ治ͬ��λ��*/	
		CreateAndShowDisplayDatas(90 + 80 *x, 20 + 85 * y);
		if (!haveDir) {
			FreePixelDataMemery();	/*�ͷſռ䣬ֻ�з�����Ż��ͷ�*/
			RealseFile();	/*unmap֮ǰ���ļ���ֻ��֮ǰ�򿪹��ļ��Ż�����Ϊ*/
		}
		else if (indexInItemList >= g_tItemIconAndNameInfos.dirNumbers) {
			haveDir = false;
		}
		/*��ʾ��ͼ�����Ҫ��Ӧ����Ŀ����*/
		GetFontAndShowFileName(90 + 80 *x, 20 + 85 * y, g_aptDirContents[indexInItemList]->strName, g_tItemIconAndNameInfos.ptFileNameFontOpr);
		
	}
	if (haveDir) {
		FreePixelDataMemery();	/*�ͷſռ䣬ֻ�з�����Ż��ͷ�*/
		RealseFile();	/*unmap֮ǰ���ļ���ֻ��֮ǰ�򿪹��ļ��Ż�����Ϊ*/
	}
	return 0;
}
#endif
