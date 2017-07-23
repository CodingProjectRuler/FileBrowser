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
		g_ptTs = ts_open(pcTsDevice, 0);	    /*��������ʽ��*/
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

/*����ǰ�����static ��ʲô����*/
/*�����¼��ļ��ʱ�䳬����time��ִ�в���*/
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

	/*������ʽ,���û�����ݾ�����*/
	iRet = ts_read(g_ptTs, &tSamp, 1);
	if(iRet != 1) {
		return -1;
	}
#if 0	
	/*��ѹ֮���ܵ������ж��Ƿ��ɿ�*/
	if (tSamp.pressure == 1) {
		/*�����û�ɿ����ͼ����жϣ�ֱ���ɿ��Ų���Ч��*/
		/*ֻ�ڸս�����ʱ����һ��*/
		if (!RunFalg) {
			if (ptInputEvent->Falg && isSmallTime(&lastTime, &tSamp.tv)) {
			/*��һ���ɹ���ѹ�¼�֮��*/
			/*��һ�εİ�ѹ�¼�����ʱ*/
			/*��֮ǰ�İ�ѹ�¼����¼����С��300ms���ϴ˴β���*/
				ptInputEvent->Falg = false;
			}
			else if (!ptInputEvent->Falg) {
				ptInputEvent->Falg = true;
			}
			if (ptInputEvent->Falg) {
				/*����һ�γɹ��Ĵ����Ŀ�ʼ*/
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
		/*��ѹ�������ɿ���˲��*/
		if (!ptInputEvent->Falg) {
			/*������ε��󴥺�*/
			/*��Ϊһ�γɹ��İ�ѹ���Ա��´εĲ����������֮��*/
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
		/*��ѹ��һ˲��*/
		if (!RunFalg) {
			if (isSmallTime(&lastTime, &tSamp.tv)) {
				ptInputEvent->Falg = true;
				/*˫���ı�־�����ʹ���*/
				//printf("twice touch\n");
			}
			else {
				ptInputEvent->Falg = false;
			}
			RunFalg = true;
		}
	}
	/*��¼����������λ��*/
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


