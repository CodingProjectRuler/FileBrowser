#ifndef _INPUT_MANAGER_H
#define _INPUT_MANAGER_H

#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>

#define INPUT_STDIN      3
#define INPUT_SCREEN   4

#define PAGE_NEXT         0
#define PAGE_PREVIOUS  1
#define PAGE_EXIT           5
#define PAGE_UNKOWN   2

typedef struct InputEvent{
	struct timeval tTime;
	int iType;
	int iVal;
	bool Falg;		/*���δ���ʱ����̫����Ϊ��*/
	int touchStartX;	/*������ʼ������*/
	int touchStartY;		
	int touchExitX;		/*��������������*/
	int touchExitY;
	int Pressure;	/*ѹ��ֵ��0ʱ�����ɿ�*/
}T_InputEvent, *PT_InputEvent;

typedef struct InuptOpr{
	char *name;
	pthread_t tThreadID;
	int (*DeviceInit)(void);
	int (*DeviceExit)(void);
	int (*GetInputEvent)(PT_InputEvent ptInputEvent);
	struct InuptOpr *ptNext;
}T_InputOpr, *PT_InputOpr;

int RegisterInputOpr(PT_InputOpr ptInputOpr);
int ScreenRegister(void);
int StdinRegister(void);
int InputRegister(void);
int AllInputDeviceInit(void);
int GetInputEventList(PT_InputEvent ptInputEvent);
int AllInputDeviceExit(void);

#endif


