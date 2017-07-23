/*显示当前顶层目录页面*/
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

static PT_DirContent *g_aptDirContents;	/*指向的指针是指向顶层目录各项的名字和类型*/
static int g_iDirContentsNumber;			/*顶层目录中的项目数*/

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
/*一页有15个条目*/
/*每个条目开始的起点*/
/*每个条目的大小为42x42*/
/*每个条目在以下面对应的起点开始，大小为80x85的矩形中显示*/
static const int startPosForItemsList[15][2] = {
		{90, 20},   {170, 20} ,  {250, 20},   {330, 20} ,  {410, 20}, 
		{90, 105}, {170, 105} ,{250, 105}, {330, 105} ,{410, 105}, 
		{90, 190}, {170, 190} ,{250, 190}, {330, 190} ,{410, 190}, 	
};

PT_pictureInfos ptPictureInfos;
T_PosXY g_tPosXY;

/*显示一个条目的名称*/
static int GetFontAndShowFileName(const int x, const int y, char *FileName, PT_FontOpr FileNameFontOpr)
{
	int i, FileNameLength;
	int rowNumbers;
	int leftCharNumbers;
	T_FontBitMap FileNameFontBitMap;
	FileNameFontBitMap.iY = y + 43;
	/*判断字体是否是16*8*/
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
	/*文件名在icon下面居中显示*/
	/*一行的长度是64可以容纳8个字符*/
	/*只多显示两行字*/
	rowNumbers = FileNameLength / 8;    	/*行数*/
	leftCharNumbers = FileNameLength % 8;	/*未满一行的字符个数*/
	rowNumbers = rowNumbers > 2? 2: rowNumbers;
	/*先显示满行数的字符*/
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
		
	/*获取本目录顶层下的条目信息*/
	GetDirContents(DirName, &g_aptDirContents, &g_iDirContentsNumber);
	g_tItemIconAndNameInfos.pages = g_iDirContentsNumber / 15;
	g_tItemIconAndNameInfos.leftIems = g_iDirContentsNumber % 15;
	if (g_tItemIconAndNameInfos.leftIems > 0) {
		g_tItemIconAndNameInfos.pages++;
	}
	g_tItemIconAndNameInfos.ptFileNameFontOpr = GetFontOpr("ascii");			/*得到由编码获取字体模块*/
	for (i = 0; i < g_iDirContentsNumber; i++) {
		if (g_aptDirContents[i]->eFileType == FILETYPE_FILE) {
			g_tItemIconAndNameInfos.dirNumbers = i;
			break;
		}
		if (i == g_iDirContentsNumber-1) {
			g_tItemIconAndNameInfos.dirNumbers = g_iDirContentsNumber;
		}
	}
	/*显示完整路径Show complete path*/
	GetFontAndShowFileName(80, 3 , DirName, g_tItemIconAndNameInfos.ptFileNameFontOpr);	
	return 0;
}

/*先简单的通过文件名称识别文件类型*/
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
	Filesuffix = fileName + i;		/*指向从.开始的文件后缀字符串指针*/
	for (j = 0; j < sizeof(supportFileTypeList) /sizeof(supportFileTypeList[0]); j++) {
		if (strcmp(Filesuffix, supportFileTypeList[j]) == 0) {
			return j+1;
		} 
	}
	return j+1;		/*未识别的文件*/
}

/*上一页的条目*/
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

/*下一页的条目*/
int ShowPreOnePageForItems(void)
{
	g_PageIndex--;
	if (g_PageIndex < 0) {
		g_PageIndex = 0;
	}
	printf("PreIndex: %d\n", g_PageIndex);
	CreatePageForDir(g_PageIndex);
}

/*将一个条目对应的图标放在复制显存中*/
/*只适用于bmp/jpeg/jpg文件*/
void PutItemOnCopyMem(char *itemName, T_ZoonInfos tZoonInfos, T_PosXY tPosXY)
{	
	SelectPicformatParserForFile(itemName);			/*打开文件*/
	GetPixelDatasFrmFileInfos(&ptPictureInfos, tZoonInfos);		/*申请空间，根据压缩比获取图片的像素数据*/
	if (tZoonInfos.Staus == PIC_STATUS || tZoonInfos.Staus == ZOOM_STATUS) {
		if (ptPictureInfos->iWidth < SCREEN_WIDTH) {
			tPosXY.PosX= (SCREEN_WIDTH - ptPictureInfos->iWidth) / 2;
		}
		if (ptPictureInfos->iHeight < SCREEN_HEIGHT) {
			tPosXY.PosY = (SCREEN_HEIGHT - ptPictureInfos->iHeight) / 2;
		}
	}
	/*放到对应的位置*/
	CreateAndShowDisplayDatas(tPosXY);
	if (tZoonInfos.Staus == PIC_STATUS || tZoonInfos.Staus == ZOOM_STATUS) {
		/*PIC_STATUS可能需要对照片进行缩放和移动*/
		/*所以在回到DIR_STATUS在做这件事*/
		/*记录前一次照片在显存上开始的位置*/
		g_tPosXY.PosX = tPosXY.PosX;
		g_tPosXY.PosY = tPosXY.PosY;
	}
	else {
		FreePixelDataMemery();						/*释放空间*/
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

/*显示一页条目的图标和对应的名称*/
/*显示出菜单以外的图标和对应的文件的名称*/
/*Icon's Size (Width*Height) : 42x42*/
/*输入参数为页码号*/
static int CreatePageForDir(int PageIndex)
{
	int i;
	int indexInItemList;
	int typeId;
	int currentPageItems;	/*本页要显示的条目数量*/
	T_ZoonInfos tZoonInfos;
	T_PosXY tPosXY;	
	tZoonInfos.addNumber = 1;
	tZoonInfos.declineNumber = 3;
	tZoonInfos.Staus = DIR_STATUS;		
	if (g_tItemIconAndNameInfos.pages == 0) {
		/*空目录*/
		return 0;
	}
	
	/*先看本页有没有目录需要显示*/
	if (ONE_PAGE_ICON_NUMBERS * PageIndex < g_tItemIconAndNameInfos.dirNumbers) {
		SelectPicformatParserForFile(PathOfIcons[0]);	       /*打开目录icon文件*/
		GetPixelDatasFrmFileInfos(ptPictureInfos, tZoonInfos);					/*申请空间，根据压缩比获取图片的像素数据*/
	}

	if ((PageIndex == g_tItemIconAndNameInfos.pages - 1) && g_tItemIconAndNameInfos.leftIems > 0) {
		currentPageItems = g_tItemIconAndNameInfos.leftIems;
	}
	else {
		currentPageItems = ONE_PAGE_ICON_NUMBERS;
	}
	for (i = 0; i < currentPageItems; i++) {
		indexInItemList = i + ONE_PAGE_ICON_NUMBERS * PageIndex;
		/*目录已经显示完毕，接下来根据文件类型打开新的icons*/
		if (indexInItemList >= g_tItemIconAndNameInfos.dirNumbers) {
			/*判断文件类型*/
			typeId = identifyFileType(g_aptDirContents[indexInItemList]->strName);
			if (i > 0) {
				FreePixelDataMemery();	/*释放空间，只有分配过才会释放*/
				RealseFile();	/*unmap之前的文件，只有之前打开过文件才会有行为*/
			}
			/*根据文件类型打开对应的图标打开前先释放之前的文件*/
			SelectPicformatParserForFile(PathOfIcons[typeId]);	       /*打开对应文件图标icons*/
			GetPixelDatasFrmFileInfos(ptPictureInfos, tZoonInfos);					/*申请空间，根据压缩比获取图片的像素数据*/
		}
		/*放到对应的位置,只不过将一张原图放在显存不同的位置*/	
		tPosXY.PosX = startPosForItemsList[i][0];
		tPosXY.PosY = startPosForItemsList[i][1];
		CreateAndShowDisplayDatas(tPosXY);

		/*显示完图标后需要对应的条目名称*/
		GetFontAndShowFileName(startPosForItemsList[i][0], startPosForItemsList[i][1], g_aptDirContents[indexInItemList]->strName, g_tItemIconAndNameInfos.ptFileNameFontOpr);
		
	}
	FreePixelDataMemery();	/*释放空间，只有分配过才会释放*/
	RealseFile();	/*unmap之前的文件，只有之前打开过文件才会有行为*/
	
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
				/*X落在菜单区域*/
				for (i = 0; i < sizeof(startPosForMenuOneList)/sizeof(startPosForMenuOneList[0]); i++) {
					if (ptInputEvent->touchExitY > startPosForMenuOneList[i][1]  && ptInputEvent->touchExitY < startPosForMenuOneList[i][1] + 64) {
						return i+20;
					}
				}
			}
			else if (ptInputEvent->touchExitX < 90) {
				/*X落在无效区域*/
				return 30;
			}
			else if (ptInputEvent->touchExitX < 452)
			{
				/*X落在条目区域*/
				if (ptInputEvent->touchExitY < startPosForItemsList[0][1] || ptInputEvent->touchExitY >  startPosForItemsList[10][1] + 42 + 16) {
					/*Y落在无效区域*/
					return 30;
				}
				/*Y落在条目区域*/
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
			return 30;		/*无效的区域*/
			break;
		case TXT_STATUS:
			if (ptInputEvent->Pressure != 0) {
				return -1;
			}
			if (ptInputEvent->touchExitX < 64 && ptInputEvent->touchExitY < 64) {
				/*点击右上角返回*/
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
				/*松开*/
				flag = true;
				if (ptInputEvent->touchExitX < 64 && ptInputEvent->touchExitY < 64 ) {
					/*点击右上角返回*/
					return 20;
				}				
				if (ptInputEvent->Falg && DistanceBetweenTwoTouchP(ptInputEvent) < SLIP_MIN_DISTANCE+2) {
					/*双击事件*/
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
					/*触摸移动事件*/
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
		/*向下进入*/
		mergeFileNameToPath(DefaultDir, shortDirName);
	}
	else {
		/*向上返回*/
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
	/*显示菜单*/
	ShowMenu(0xff);
	InitForShowIconsAndFileName(DefaultDir);		
	ShowPreOnePageForItems();
	ShowOnePicture();		/*刷到设备上去*/
	return 0;
}

/*把字符串FileName放在DirPath带有分隔符'/'的后面*/
/*没有分隔符就手动加上形成完整的目录或文件路径*/
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

/*打开可支持的文件*/
/*txt(ascii/gbk/utf-8/utf-16le/utf-16be)，bmp，jpg/jpeg*/
static int openFile(char *fileName)
{
	int fileType;
	T_ZoonInfos tZoonInfos;
	T_PosXY tPosXY;
	fileType = identifyFileType(fileName);
	if (5 == fileType) {
		/*触摸的文件不被支持*/
		printf("Can not support this type file: %s\n", fileName);
		/*打开文件失败*/
		/*模式仍然为目录状态*/
		return 0;
	}
	/*在此将文件名和路径拼接*/
	/*形成完整的带有路径的文件名*/
	memcpy(CopyDefaultDir, DefaultDir, 256);
	mergeFileNameToPath(CopyDefaultDir, fileName);
	if (fileType == 1) {
		/*txt状态*/
		/*打开文件|  映射文件 | 根据而文件头部标记选择Ecoding*/
		printf("TXTCompletePathFile: %s\n", CopyDefaultDir);
		OpenTextFile(CopyDefaultDir);
		/*初始化所选中Encoding所支持的Fonts | 打开文件| 判断是否支持FontSize*/
		/*一个文本文件可能需要多种fontOpr的支持*/
		InitEncoding_Fonts("/HZK16", "/MSYH.TTF", 24);		
		/*解码*/ /*获取font位图数据显示*/
		ShowNextPage();
		return TXT_STATUS;
	}
	else {
		/*bmp/jpg/jpeg状态，统一处理*/		
		printf("PICCompletePathFile: %s\n", CopyDefaultDir);
		tZoonInfos.Staus = PIC_STATUS;
		tPosXY.PosX = 0;
		tPosXY.PosY = 0;
		CleanScreen(0x00);
		PutItemOnCopyMem(CopyDefaultDir, tZoonInfos, tPosXY);
		ShowOnePicture();		/*刷到设备上去*/
		return PIC_STATUS;
	}
	return 0;
}

static int ZoomMultiple(PT_ZoonInfos ptZoonInfos, int *ZoomFlag, int *TwiceTouchNumbers)
{
	ptZoonInfos->Staus = ZOOM_STATUS;
	(*TwiceTouchNumbers)++;
	if (*ZoomFlag == 1 || *ZoomFlag == 0) {
		/*放大*/
		if (ptPictureInfos->addNumber >= ptPictureInfos->declineNumber) {
			ptPictureInfos->addNumber *= 2;
		}
		else {
			ptPictureInfos->declineNumber /= 2;
		}
	}
	else if (*ZoomFlag == -1){
		/*缩小*/
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
	static int ZoomFlag;		/*缩放的方向标记*/
	T_ZoonInfos tZoonInfos;
	T_PosXY tPosXY;
	/*获取触摸区域*/
	touchNumber = EventArea(NowStatus, ptInputEvent);
	if (touchNumber == -1) {
		return 0;
	}
	/*在目录上下级浏览的输入事件处理分支*/
	switch (NowStatus) {
		case DIR_STATUS:
			if (touchNumber == 30) {
				/*无效的触摸*/
				return 0;
			}
			else if (touchNumber == 22 || touchNumber == 23) {
				/*处理左右页切换*/			
				/*显示菜单*/
				ShowMenu(0xff);
				/*显示完整路径*/
				GetFontAndShowFileName(80, 3 , DefaultDir, g_tItemIconAndNameInfos.ptFileNameFontOpr);					
				/*显示条目*/
				if (touchNumber == 22) {
					ShowPreOnePageForItems();
					ShowOnePicture();		/*刷到设备上去并释放复制空间*/
				}
				if (touchNumber == 23) {
					ShowNextOnePageForItems();
					ShowOnePicture();		/*刷到设备上去并释放复制空间*/
				}
			}
			else if (touchNumber == 20 || touchNumber == 21) {
				/*返回和选择待处理*/
				if (touchNumber == 20) {
					/*返回上一级目录*/
					openNewDir(false, NULL);
				}
				return 0;
			}
			else {
				/*点击条目*/
				if (g_PageIndex == g_tItemIconAndNameInfos.pages - 1 && touchNumber > g_tItemIconAndNameInfos.leftIems - 1) {
					/*在一个不是满屏的触摸区域超过条目区域*/
					return 0;
				}
				itemIndex = g_PageIndex*ONE_PAGE_ICON_NUMBERS + touchNumber;
				if (g_aptDirContents[itemIndex]->eFileType == FILETYPE_FILE) {
					/*触摸的条目是文件*/
					NowStatus = openFile(g_aptDirContents[itemIndex]->strName);
					if (NowStatus == PIC_STATUS) {
						ZoomFlag = 0;
						TwiceTouchNumbers = 0;	
					}
					return 0;
				}
				else if (g_aptDirContents[itemIndex]->eFileType == FILETYPE_DIR) {
					/*触摸的条目是目录*/
					/*打开进入目录触摸目录*/
					openNewDir(true, g_aptDirContents[itemIndex]->strName);
					return 0;
				}
			}
			break;
		case TXT_STATUS:
			if (touchNumber == 20) {
				/*关闭TXT文件*/
				MunmapAndCloseFile();
				/*返回目录状态*/
				NowStatus = DIR_STATUS;
				/*处理左右页切换*/
				/*显示菜单*/
				ShowMenu(0xff);
				/*显示完整路径*/
				GetFontAndShowFileName(80, 3 , DefaultDir, g_tItemIconAndNameInfos.ptFileNameFontOpr);					
				g_PageIndex++;
				ShowPreOnePageForItems();
				ShowOnePicture();		/*刷到设备上去并释放复制空间*/				
			}	
			else if (touchNumber == 0) {
				return 0;
			}
			else if (touchNumber == 1){
				/*上一页*/
				ShowPrePage();
			}
			else if (touchNumber == 2){
				/*下一页*/
				ShowNextPage();
			}
			break;
		case PIC_STATUS:
			if (touchNumber == 20) {
				/*返回目录状态*/
				/*返回的时候释放和关闭文件*/
				FreePixelDataMemery();	/*释放空间，只有分配过才会释放*/
				RealseFile();	/*unmap之前的文件，只有之前打开过文件才会有行为*/	
				NowStatus = DIR_STATUS;
				/*处理左右页切换*/			
				/*显示菜单*/
				ShowMenu(0xff);
				/*显示完整路径*/
				GetFontAndShowFileName(80, 3 , DefaultDir, g_tItemIconAndNameInfos.ptFileNameFontOpr);					
				g_PageIndex++;
				ShowPreOnePageForItems();
				ShowOnePicture();		/*刷到设备上去并释放复制空间*/				
			}
			else if (touchNumber == MOVE_PICTURE_POSITION) {
				/*移动照片在屏幕上的位置*/
				/*清黑复制空间*/
				tPosXY.PosX = g_tPosXY.PosX + (ptInputEvent->touchExitX - ptInputEvent->touchStartX);
				tPosXY.PosY = g_tPosXY.PosY + (ptInputEvent->touchExitY - ptInputEvent->touchStartY);
				g_tPosXY.PosX = tPosXY.PosX;
				g_tPosXY.PosY = tPosXY.PosY;
				if (!TestPosition(&g_tPosXY)) {
					CleanScreen(0x00);
				}
				CreateAndShowDisplayDatas(tPosXY);			/*放到对应的位置*/
				ShowOnePicture();		/*刷到设备上去并释放复制空间*/	
			}
			else if (touchNumber == TWICE_TOUCH) {
				/*双击缩放事件*/
				FreePixelDataMemery();	/*释放空间，只有分配过才会释放*/
				RealseFile();	/*unmap之前的文件，只有之前打开过文件才会有行为*/	
				tPosXY.PosX = 0;
				tPosXY.PosY = 0;
				CleanScreen(0x00);
				ZoomMultiple(&tZoonInfos, &ZoomFlag, &TwiceTouchNumbers);
				PutItemOnCopyMem(CopyDefaultDir, tZoonInfos, tPosXY);
				ShowOnePicture();		/*刷到设备上去*/	
			}
			break;
		default:
			break;
	}
	return 0;
}

/*根据输入事件刷新对应的页面*/
int RunRefreshPage(void) 
{
	int i;
	T_InputEvent tInputEvent;	/*输入事件结构*/
	/*显示菜单*/
	ShowMenu(0xff);
	/*获取顶层目录信息*/
	InitForShowIconsAndFileName(DefaultDir);
	ShowPreOnePageForItems();
	ShowOnePicture();		/*刷到设备上去*/		
	while(1){
		if(GetInputEventList(&tInputEvent) == 0){
			/*执行到这里说明得到了输入事件*/
			/*处理并响应输入事件*/
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
	
	/*先看本页有没有目录需要显示*/
	if (ONE_PAGE_ICON_NUMBERS * PageIndex < g_tItemIconAndNameInfos.dirNumbers) {
		SelectPicformatParserForFile(PathOfIcons[0]);	 /*打开目录icon文件*/
		GetPixelDatasFrmFileInfos(1, 3);				/*申请空间，根据压缩比获取图片的像素数据*/
		haveDir = true;
	}
	for (i = 0; i < ONE_PAGE_ICON_NUMBERS; i++) {
		x = i %5;
		y = i / 5;
		indexInItemList = i + ONE_PAGE_ICON_NUMBERS * PageIndex;
		/*目录已经显示完毕，接下来根据文件类型打开新的icons*/
		if (indexInItemList >= g_tItemIconAndNameInfos.dirNumbers) {
			/*判断文件类型*/
			typeId = identifyFileType(g_aptDirContents[indexInItemList]->strName);
			if (haveDir) {
				FreePixelDataMemery();	/*释放空间，只有分配过才会释放*/
				RealseFile();	/*unmap之前的文件，只有之前打开过文件才会有行为*/
			}
			/*根据文件类型打开对应的图标打开前先释放之前的文件*/
			SelectPicformatParserForFile(PathOfIcons[typeId]);	       /*打开对应文件图标icons*/
			GetPixelDatasFrmFileInfos(1, 3);					/*申请空间，根据压缩比获取图片的像素数据*/
		}
		/*放到对应的位置,只不过将一张原图放在显存不同的位置*/	
		CreateAndShowDisplayDatas(90 + 80 *x, 20 + 85 * y);
		if (!haveDir) {
			FreePixelDataMemery();	/*释放空间，只有分配过才会释放*/
			RealseFile();	/*unmap之前的文件，只有之前打开过文件才会有行为*/
		}
		else if (indexInItemList >= g_tItemIconAndNameInfos.dirNumbers) {
			haveDir = false;
		}
		/*显示完图标后需要对应的条目名称*/
		GetFontAndShowFileName(90 + 80 *x, 20 + 85 * y, g_aptDirContents[indexInItemList]->strName, g_tItemIconAndNameInfos.ptFileNameFontOpr);
		
	}
	if (haveDir) {
		FreePixelDataMemery();	/*释放空间，只有分配过才会释放*/
		RealseFile();	/*unmap之前的文件，只有之前打开过文件才会有行为*/
	}
	return 0;
}
#endif
