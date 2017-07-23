#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <drawtxt.h>
#include <drawpicture.h>
#include <explorePage.h>
#include <encoding_manager.h>
#include <font_manager.h>
#include <picformat_manager.h>
#include <display_manager.h>
#include <input_manager.h>
#include <string.h>

/*Usage: ./FileBrowser*/
int main(int argc, char *argv[])
{
	/*Initial List*/
	/*初始化各处理模块构成各自的链表*/
	RegisterListp();
	/*根据名字选择显示器并初始化读出相关参数*/
	SelectAndInitDisplayp("fb");
	/*初始化所有的输入设备读出参数，并为每个设备创建一个线程*/
	/*打开设备并为之创建线程:从现在开始已经有三条线程在跑了*/
	AllInputDeviceInit();
	/*分配显存的复制空间*/
	MallocCopyOfVideoMemery();	/*申请显存的复制空间*/
	/*清白复制空间*/
	CleanScreen(0xff);
	/*在这里循环接受输入事件和处理响应输入事件*/
	RunRefreshPage();		
	return 0;
}


