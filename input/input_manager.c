#include <input_manager.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
static PT_InputOpr g_ptInputOprHead;
/*互斥量*/
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
/*条件变量*/
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

/*输入事件线程函数*/
/*线程函数接受获取输入事件的函数地址作为函数*/
/*在输入事件中将void *地址强转为	int (*GetInputEvent)(PT_InputEvent ptInputEvent);类函数指针*/
/*(int (*)(PT_InputEvent))指针属性为接受PT_InputEvent和返回int类型的指针*/
/**/
static void *InputEventThreadFunction(void *pvoid)
{
	T_InputEvent tInputEvent;
	/*需要重新定义一个函数指针*/
	/*因为pvoid指针要进行强制转换才能使用*/
	int (*GetInputEventThread)(PT_InputEvent ptInputEvent);		/*声明一个函数指针*/
	GetInputEventThread = (int (*)(PT_InputEvent))pvoid;		/*把传过来的函数指针强转回创建线程时的函数类型*/
	while(1){
		if(GetInputEventThread(&tInputEvent) == 0){			/*利用函数指针来调用对应的获取输入事件函数*/
			pthread_mutex_lock(&mutex);					/*输入设备是以阻塞方式打开的，在没有输入事件产生的时候*/
			g_tInputEvent = tInputEvent;						/*会进入休眠模式，有数据的时候会被唤醒*/
			pthread_cond_signal(&condvar);
			pthread_mutex_unlock(&mutex);
		}
	}
	return NULL;
}   

/*有一个输入设备初始化成功就算成功:可以输入*/
/*为每一个初始化成功的输入设备开启一个线程*/
/*包括主线程:共有三条线程在同时运行*/
int AllInputDeviceInit(void)
{
	int iError = -1;
	PT_InputOpr ptTemp;
	ptTemp = g_ptInputOprHead;
	while(ptTemp != NULL){
		iError = ptTemp->DeviceInit();
		if(iError == 0){
			/*Create Child thread, send the GetInputEvent function to the Child thread*/
						   /*线程ID   | 线程属性| 线程运行函数|线程函数的参数这里的参数为一个函数*/
			pthread_create(&ptTemp->tThreadID, NULL, &InputEventThreadFunction, ptTemp->GetInputEvent);
		}
		ptTemp = ptTemp->ptNext;
	}
	return iError;
}

int GetInputEventList(PT_InputEvent ptInputEvent)
{
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&condvar, &mutex);	/*等待条件变量，没有信号就休眠*/

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
