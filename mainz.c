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
	/*��ʼ��������ģ�鹹�ɸ��Ե�����*/
	RegisterListp();
	/*��������ѡ����ʾ������ʼ��������ز���*/
	SelectAndInitDisplayp("fb");
	/*��ʼ�����е������豸������������Ϊÿ���豸����һ���߳�*/
	/*���豸��Ϊ֮�����߳�:�����ڿ�ʼ�Ѿ��������߳�������*/
	AllInputDeviceInit();
	/*�����Դ�ĸ��ƿռ�*/
	MallocCopyOfVideoMemery();	/*�����Դ�ĸ��ƿռ�*/
	/*��׸��ƿռ�*/
	CleanScreen(0xff);
	/*������ѭ�����������¼��ʹ�����Ӧ�����¼�*/
	RunRefreshPage();		
	return 0;
}


