#ifndef _FILE_H
#define _FILE_H

#include <stdio.h>

typedef struct FileInfos {
	char FileName[256];
	FILE *tFp; 	/*�ļ����*/
	int iFileSize;	
	unsigned char *pucFileMapMemery;
}T_FileInfos, *PT_FileInfos;

typedef enum {
	FILETYPE_DIR = 0,		/*Ŀ¼*/
	FILETYPE_FILE,		/*�ļ�*/
}E_FileType;

/*Ŀ¼�¶���Ŀ¼������*/
typedef struct DirContent {
	char strName[256]; 
	E_FileType eFileType;
}T_DirContent, *PT_DirContent;


int OpenAndMapFile(PT_FileInfos ptFileInfos);
void UnmapAndCloseFile(PT_FileInfos ptFileInfos);

#endif
