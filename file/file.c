#include <file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int OpenAndMapFile(PT_FileInfos ptFileInfos)
{
	int iFd;
	FILE *tFp;
	//�ļ����Խṹ
	struct stat tStat;

	/*open file */
	tFp = fopen(ptFileInfos->FileName, "r+");
	if (tFp == NULL) {
		printf("Open file fail!\n");
		return -1;
	}
	
	ptFileInfos->tFp = tFp;
	iFd = fileno(tFp);
	if(fstat(iFd, &tStat)){
		printf("Can't get fstat\n");
		return -1;
	}	
	ptFileInfos->iFileSize = tStat.st_size;
	ptFileInfos->pucFileMapMemery = (unsigned char *)mmap(NULL, tStat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, iFd, 0);
	if(ptFileInfos->pucFileMapMemery == (unsigned char *)-1){
		printf("Can't mmap for textfile\n");
		return -1;
	}
	return 0;
}
void UnmapAndCloseFile(PT_FileInfos ptFileInfos)
{
	munmap(ptFileInfos->pucFileMapMemery, ptFileInfos->iFileSize);
	fclose(ptFileInfos->tFp);
	
}

/*�ж���Ŀ¼�����ļ�*/
static int isDir(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

    if ((stat(strTmp, &tStat) == 0) && S_ISDIR(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*��Ŀ¼�Ļ����ǲ��ǳ���Ŀ¼*/
static int isRegDir(char *strDirPath, char *strSubDirName)
{
    static const char *astrSpecailDirs[] = {"sbin", "bin", "usr", "lib", "proc", "tmp", "dev", "sys", NULL};
    int i;
    
    /* ���Ŀ¼������"astrSpecailDirs"�е�����һ��, �򷵻�0 */
    if (0 == strcmp(strDirPath, "/"))
    {
        while (astrSpecailDirs[i])
        {
            if (0 == strcmp(strSubDirName, astrSpecailDirs[i]))
                return 0;
            i++;
        }
    }
    return 1;    
}

/*���ļ��Ļ��ǲ��ǳ����ļ�*/
static int isRegFile(char *strFilePath, char *strFileName)
{
    char strTmp[256];
    struct stat tStat;

    snprintf(strTmp, 256, "%s/%s", strFilePath, strFileName);
    strTmp[255] = '\0';

    if ((stat(strTmp, &tStat) == 0) && S_ISREG(tStat.st_mode))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int GetDirContents(char *strDirName, PT_DirContent **pptDirContents, int *piNumber)
{
	int i, j;
	PT_DirContent *aptDirContent;
	int ContentsNumber;
	struct dirent **ContentsNameList;

	ContentsNumber = scandir(strDirName, &ContentsNameList, 0, alphasort);
	if (ContentsNumber < 0) {
		printf("scan error: %s \n", strDirName);
		return -1;
	}

	aptDirContent = malloc(sizeof(PT_DirContent) * (ContentsNumber - 2));
	if (aptDirContent == NULL) {
		printf("Malloc fail\n");
		return -1;
	}
	*pptDirContents = aptDirContent;

	for (i = 0; i < ContentsNumber - 2; i ++) {
		aptDirContent[i] = malloc(sizeof(T_DirContent));
		if (aptDirContent[i] == NULL) {
			printf("Malloc fail\n");
			return -1;
		}
	}
	/*��������Ŀ¼���������ǰ�벿��*/
	for (i = 0, j= 0; i < ContentsNumber; i++) {
		if ((strcmp(ContentsNameList[i]->d_name, ".") == 0) || 
			(strcmp(ContentsNameList[i]->d_name, "..") == 0)) {
			continue;
		}
		if (isDir(strDirName, ContentsNameList[i]->d_name)) {
			strncpy(aptDirContent[j]->strName, ContentsNameList[i]->d_name, 256);
			aptDirContent[j]->strName[255] = '\0';
			aptDirContent[j]->eFileType = FILETYPE_DIR;
			free(ContentsNameList[i]);
			ContentsNameList[i] = NULL;
			j ++;
		}
	}
	/*֮�����ڳ�Ŀ¼������ļ������������ļ�*/
	for (i = 0; i < ContentsNumber; i++) {
		if (ContentsNameList[i] == NULL) {
			continue;
		}
		if ((strcmp(ContentsNameList[i]->d_name, ".") == 0) || 
			(strcmp(ContentsNameList[i]->d_name, "..") == 0)) {
			continue;
		}
		if (isRegFile(strDirName, ContentsNameList[i]->d_name)) {
			strncpy(aptDirContent[j]->strName, ContentsNameList[i]->d_name, 256);
			aptDirContent[j]->strName[255] = '\0';
			aptDirContent[j]->eFileType = FILETYPE_FILE;
			free(ContentsNameList[i]);
			ContentsNameList[i] = NULL;
			j ++;
		}
	}

	for (i = j; i < ContentsNumber - 2; i++) {
		free(aptDirContent[i]);
	}

	/*�ͷ�scandir������ڴ�*/
	for (i= 0; i < ContentsNumber; i ++) {
		if (ContentsNameList[i]) {
			free(ContentsNameList[i]);
		}
	}
	free(ContentsNameList);
	*piNumber = j;
	return 0;
}

void FreeDirContents(PT_DirContent *aptDirContents, int Number)
{
	int i;
	for (i = 0; i < Number; i ++) {
		free(aptDirContents[i]);
	}
	free(aptDirContents);
}

