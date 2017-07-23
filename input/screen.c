/*
export TSLIB_TSDEVICE=/dev/event0
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/lib/ts
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
*/
#include <input_manager.h>
#include <display_manager.h>
#include <stdlib.h>
#include <stdio.h>
#include <tslib.h>

extern PT_DispOpr g_ptDisOprShare;

static struct tsdev *g_ptTs;
static int g_iX, g_iY;

static int ScreenInit(void);
static int ScreenExit(void);
static int GetScreenEvent(PT_InputEvent ptInputEvent);

static T_InputOpr g_tScreenInputOpr = {
	.name = "screen",
	.DeviceInit = ScreenInit,
	.DeviceExit = ScreenExit,
	.GetInputEvent = GetScreenEvent,
};

static int ScreenInit(void)
{
	char *pcTsDevice = NULL;
	if((pcTsDevice = getenv("TSLIB_TSDEVICE")) != NULL)
		g_ptTs = ts_open(pcTsDevice, 0);	    /*以阻塞方式打开*/
	else
		g_ptTs = ts_open("/dev/event0", 0);
	if(!g_ptTs){
		printf("Ts_open failed\n");		
		return -1;
	}
	if(ts_config(g_ptTs)){
		printf("Ts_config failed\n");
		return -1;
	}
	if(g_ptDisOprShare == NULL) {
		return -1;
	}
	else {
		g_iX = g_ptDisOprShare->iXres;
		g_iY = g_ptDisOprShare->iYres;		
	}
	return 0;
}
static int ScreenExit(void)
{
	return 0;
}

/*函数前面加了static 有什么区别*/
/*两次事件的间隔时间超过了time才执行操作*/
int isSmallTime(struct timeval *PtPreSTime, struct timeval *PtNowTime)
{
	long iPreMs;
	long iNowMs;

	iPreMs = PtPreSTime->tv_sec * 1000 + PtPreSTime->tv_usec /1000;
	iNowMs = PtNowTime->tv_sec * 1000 + PtNowTime->tv_usec /1000;

	if (iNowMs > iPreMs + 50 && iNowMs < iPreMs + 300) {
		return 1;
	}
	return 0;
}

static int GetScreenEvent(PT_InputEvent ptInputEvent)
{
	static struct timeval lastTime;
	static bool RunFalg = false;
	struct ts_sample tSamp;
	tSamp.tv.tv_sec = 0;
	tSamp.tv.tv_usec = 0;
	int iRet ;

	/*阻塞方式,如果没有数据就休眠*/
	iRet = ts_read(g_ptTs, &tSamp, 1);
	if(iRet != 1) {
		return -1;
	}
#if 0	
	/*按压之后跑到这里判断是否松开*/
	if (tSamp.pressure == 1) {
		/*如果还没松开，就继续判断，直到松开才产生效果*/
		/*只在刚进来的时候跑一次*/
		if (!RunFalg) {
			if (ptInputEvent->Falg && isSmallTime(&lastTime, &tSamp.tv)) {
			/*在一个成功按压事件之后*/
			/*又一次的按压事件来临时*/
			/*与之前的按压事件的事件间隔小于300ms作废此次操作*/
				ptInputEvent->Falg = false;
			}
			else if (!ptInputEvent->Falg) {
				ptInputEvent->Falg = true;
			}
			if (ptInputEvent->Falg) {
				/*这是一次成功的触摸的开始*/
				ptInputEvent->touchStartX = tSamp.x;
				ptInputEvent->touchStartY = tSamp.y;
			}
			printf("Sx:%d, Sy:%d Flag:%d P:%d\n", ptInputEvent->touchStartX, ptInputEvent->touchStartY, ptInputEvent->Falg, tSamp.pressure);
			RunFalg = true;
		}
		return -1;
	}
	else if(tSamp.pressure == 0) {
		lastTime = tSamp.tv;
		RunFalg = false;
		/*按压结束，松开的瞬间*/
		if (!ptInputEvent->Falg) {
			/*作废这次的误触后*/
			/*计为一次成功的按压，以便下次的操作是在这次之后*/
			ptInputEvent->tTime = tSamp.tv;
			ptInputEvent->Falg = true;
			return -1;
		}
#endif
	if (tSamp.pressure == 0) {
		lastTime = tSamp.tv;
		RunFalg = false;
	}
	else {
		/*按压的一瞬间*/
		if (!RunFalg) {
			if (isSmallTime(&lastTime, &tSamp.tv)) {
				ptInputEvent->Falg = true;
				/*双击的标志触摸就触发*/
				//printf("twice touch\n");
			}
			else {
				ptInputEvent->Falg = false;
			}
			RunFalg = true;
		}
	}
	/*记录触摸的坐标位置*/
	ptInputEvent->touchStartX = ptInputEvent->touchExitX;
	ptInputEvent->touchStartY = ptInputEvent->touchExitY;
	ptInputEvent->tTime = tSamp.tv;
	ptInputEvent->iType = INPUT_SCREEN;
	ptInputEvent->touchExitX = tSamp.x;
	ptInputEvent->touchExitY = tSamp.y;
	//printf("Ex:%d, Ey:%d Flag:%d P:%d\n", ptInputEvent->touchExitX, ptInputEvent->touchExitY, ptInputEvent->Falg, tSamp.pressure);
	ptInputEvent->Pressure = tSamp.pressure;
		
	return 0;	
	
}

int ScreenRegister(void)
{
	RegisterInputOpr(&g_tScreenInputOpr);
	return 0;
}


