#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <input_manager.h>
/*NULL 在stdlib.h中定义*/

static int StdinInit(void);
static int StdinExit(void);
static int GetStdinEvent(PT_InputEvent ptInputEvent);

static T_InputOpr g_tStdinInputOpr = {
	.name = "stdin",
	.DeviceInit = StdinInit,
	.DeviceExit = StdinExit,
	.GetInputEvent = GetStdinEvent,
};

/*set the terminal mode to input one character return right now*/
static int StdinInit(void)
{
	struct termios tTTystate;

	/*get the terminal state*/
	tcgetattr(STDIN_FILENO, &tTTystate);
	/*turn off canonical mode*/
	tTTystate.c_lflag &= ~ICANON;
	/*minimum of number input read*/
	tTTystate.c_cc[VMIN] = 1;
	/*set the terminal attributes*/
	tcsetattr(STDIN_FILENO, TCSANOW, &tTTystate);
	return 0;	
}
static int StdinExit(void)
{
	struct termios tTTystate;
	/*get the terminal state*/
	tcgetattr(STDIN_FILENO, &tTTystate);
	/*turn off canonical mode*/
	tTTystate.c_lflag |=  ICANON;
	/*set the terminal attributes*/
	tcsetattr(STDIN_FILENO, TCSANOW, &tTTystate);
	return 0;	
}

static int GetStdinEvent(PT_InputEvent ptInputEvent)
{
	char cVal;
	/*不用select设置就是阻塞方式*/
	cVal = fgetc(stdin);	/*没有数据就休眠*/
	ptInputEvent->iType = INPUT_STDIN;
	gettimeofday(&ptInputEvent->tTime, NULL);
	switch(cVal){
		case 'n':
			ptInputEvent->iVal = PAGE_NEXT;
			break;
		case 'p':
			ptInputEvent->iVal = PAGE_PREVIOUS;
			break;
		case 'q':
			ptInputEvent->iVal = PAGE_EXIT;
			break;
		default:
			ptInputEvent->iVal = PAGE_UNKOWN;
			break;
	}
	return 0;

}

int StdinRegister(void)
{
	RegisterInputOpr(&g_tStdinInputOpr);
	return 0;
}


