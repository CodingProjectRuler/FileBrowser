#include <input_manager.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
static PT_InputOpr g_ptInputOprHead;
/*������*/
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*��������*/
static pthread_cond_t condvar = PTHREAD_COND_INITIALIZER;

static T_InputEvent g_tInputEvent;

int RegisterInputOpr(PT_InputOpr ptInputOpr)
{
	PT_InputOpr ptTemp;
	ptInputOpr->ptNext = NULL;
	if(g_ptInputOprHead == NULL)
		g_ptInputOprHead = ptInputOpr;
	else{
		ptTemp = g_ptInputOprHead;
		while(ptTemp->ptNext != NULL)
			ptTemp = ptTemp->ptNext;
		ptTemp->ptNext = ptInputOpr;
	}
	return 0;
}

/*�����¼��̺߳���*/
/*�̺߳������ܻ�ȡ�����¼��ĺ�����ַ��Ϊ����*/
/*�������¼��н�void *��ַǿתΪ	int (*GetInputEvent)(PT_InputEvent ptInputEvent);�ຯ��ָ��*/
/*(int (*)(PT_InputEvent))ָ������Ϊ����PT_InputEvent�ͷ���int���͵�ָ��*/
/**/
static void *InputEventThreadFunction(void *pvoid)
{
	T_InputEvent tInputEvent;
	/*��Ҫ���¶���һ������ָ��*/
	/*��Ϊpvoidָ��Ҫ����ǿ��ת������ʹ��*/
	int (*GetInputEventThread)(PT_InputEvent ptInputEvent);		/*����һ������ָ��*/
	GetInputEventThread = (int (*)(PT_InputEvent))pvoid;		/*�Ѵ������ĺ���ָ��ǿת�ش����߳�ʱ�ĺ�������*/
	while(1){
		if(GetInputEventThread(&tInputEvent) == 0){			/*���ú���ָ�������ö�Ӧ�Ļ�ȡ�����¼�����*/
			pthread_mutex_lock(&mutex);					/*�����豸����������ʽ�򿪵ģ���û�������¼�������ʱ��*/
			g_tInputEvent = tInputEvent;						/*���������ģʽ�������ݵ�ʱ��ᱻ����*/
			pthread_cond_signal(&condvar);
			pthread_mutex_unlock(&mutex);
		}
	}
	return NULL;
}   

/*��һ�������豸��ʼ���ɹ�����ɹ�:��������*/
/*Ϊÿһ����ʼ���ɹ��������豸����һ���߳�*/
/*�������߳�:���������߳���ͬʱ����*/
int AllInputDeviceInit(void)
{
	int iError = -1;
	PT_InputOpr ptTemp;
	ptTemp = g_ptInputOprHead;
	while(ptTemp != NULL){
		iError = ptTemp->DeviceInit();
		if(iError == 0){
			/*Create Child thread, send the GetInputEvent function to the Child thread*/
						   /*�߳�ID   | �߳�����| �߳����к���|�̺߳����Ĳ�������Ĳ���Ϊһ������*/
			pthread_create(&ptTemp->tThreadID, NULL, &InputEventThreadFunction, ptTemp->GetInputEvent);
		}
		ptTemp = ptTemp->ptNext;
	}
	return iError;
}

int GetInputEventList(PT_InputEvent ptInputEvent)
{
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&condvar, &mutex);	/*�ȴ�����������û���źž�����*/

	*ptInputEvent = g_tInputEvent;
	
	pthread_mutex_unlock(&mutex);
	return 0;
}

int AllInputDeviceExit(void)
{
	PT_InputOpr ptTemp;
	ptTemp = g_ptInputOprHead;
	while(ptTemp != NULL){
		ptTemp->DeviceExit();
		ptTemp = ptTemp->ptNext;
	}
	return 0;
	
}

int InputRegister(void)
{
	ScreenRegister();
	StdinRegister();
	return 0;
}
