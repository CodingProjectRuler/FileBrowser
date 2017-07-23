#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>

typedef struct FileInfos {
	char FileName[256];
	FILE *tFp; 	/*文件句柄*/
	int iFileSize;	
	unsigned char *pucFileMapMemery;
}T_FileInfos, *PT_FileInfos;

typedef enum {
	FILETYPE_DIR = 0,		/*目录*/
	FILETYPE_FILE,		/*文件*/
}E_FileType;

/*目录下顶层目录的内容*/
typedef struct DirContent {
	char strName[256]; 
	E_FileType eFileType;
}T_DirContent, *PT_DirContent;


int OpenAndMapFile(PT_FileInfos ptFileInfos);
void UnmapAndCloseFile(PT_FileInfos ptFileInfos);

#endif
