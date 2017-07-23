

/*fb 结构体有名字、设备初始化、显示像素、清除屏幕*/
/*switch case :生成列表
  *用case的值作为下标:
  *表项的个数为case最大值-最小值
  *寻找的第一步比较switch和case 中最大值
  *若大于则执行default
  *否则执行下表为switch(value)的那个列表项
  *整个过程没有任何判断过程
  *由于列表生成的过程是连续的
  *故对于零星分散的值比较浪费空间
  *另外一个限制就是case 的值只能常量
  */
 /*
   if else就是基本的一条条的比较语句
   */

#include <display_manager.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>
#include <stdio.h>

static int FBDeviceInit(void);
static int FBShowPixel(int iPenX, int iPenY, unsigned int dwColor);
static int FBShowCompleteOnePage(unsigned char *OnePagePixelDatasAddr);
static int FBCleanScreen(unsigned int dwBackColor);

static struct fb_var_screeninfo g_tVar;	/* Current var */
static unsigned char *g_pucFBMem;
static int g_iScreenSize;
static int g_iPixelWidth;
static int g_iLineWidth;

static T_DispOpr g_tFBOpr = {
	.name = "fb",
	.DeviceInit = FBDeviceInit,
	.ShowOnePixel = FBShowPixel,
	.ShowCompleteOnePage = FBShowCompleteOnePage,
	.CleanScreen = FBCleanScreen,
};

static int FBDeviceInit(void)
{
	int FBfd;

	FBfd = open("/dev/fb0", O_RDWR);
	if(FBfd < 0){
		printf("Can't open /dev/fb0\n");
		return -1;
	}
	
	if((ioctl(FBfd, FBIOGET_VSCREENINFO, &g_tVar)) < 0){
		printf("Can't get Var\n");
		return -1;
	}
	
	g_tFBOpr.iXres = g_tVar.xres;
	g_tFBOpr.iYres = g_tVar.yres;
	g_tFBOpr.iBpp = g_tVar.bits_per_pixel;
	g_iPixelWidth = g_tFBOpr.iBpp / 8;
	g_iLineWidth  = g_tFBOpr.iXres * g_iPixelWidth;
	g_iScreenSize = g_tFBOpr.iXres * g_tFBOpr.iYres * g_iPixelWidth;

	g_pucFBMem = (unsigned char *)mmap(NULL, g_iScreenSize , PROT_READ | PROT_WRITE, MAP_SHARED, FBfd, 0);
	if(g_pucFBMem == (unsigned char *)(-1)){
		printf("Failed to mmap\n");
		return -1;
	}
	g_tFBOpr.iLineWidthBytes = g_iLineWidth;
	g_tFBOpr.pucDispMem = g_pucFBMem;
	return 0;
}

static int FBShowPixel(int iPenX, int iPenY, unsigned int dwColor)
{
	unsigned char  *pucPen_8;	    /*One byte*/
	unsigned short *pwPen_16;	    /*Two bytes*/
	unsigned int     *pdwPen_32;	    /*Four bytes*/

	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;

	/*Calculate the position of(x and y) in video memory(显存)*/
	pucPen_8   = g_pucFBMem + iPenY * g_iLineWidth + iPenX * g_iPixelWidth;
	pwPen_16   = (unsigned short *)pucPen_8;		/*Just chage it's attribute(属性)*/
	pdwPen_32  = (unsigned int *)pucPen_8;
	/*Write the color date to the calculated poation according the "iBpp"*/
	switch(g_tFBOpr.iBpp){
		case 8:
			*pucPen_8 = dwColor;
			break;
		case 16:
			/*565*/
			dwRed = (dwColor >> 19) & 0x1f;
			dwGreen = (dwColor >>10) & 0x3f;
			dwBlue = (dwColor >> 3) & 0x1f;
			dwColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue << 0);
			*pwPen_16 = dwColor;
			break;
		case 32:
			*pdwPen_32 = dwColor;
			break;
		default:
			printf("Can't suppot this Pixelwidth\n ");
			return -1;
			break;
	}
	return 0;
}

static int FBShowCompleteOnePage(unsigned char *OnePagePixelDatasAddr)
{
	memcpy(g_pucFBMem, OnePagePixelDatasAddr, g_iScreenSize);
	return 0;
}

static int FBCleanScreen(unsigned int dwBackColor)
{
	unsigned char *pucPen_8;
	unsigned short *pwPen_16;
	unsigned int *pdwPen_32;

	unsigned int dwRed;
	unsigned int dwGreen;
	unsigned int dwBlue;
	int i = 0;

	pucPen_8 = g_pucFBMem;
	pwPen_16 = (unsigned short *)pucPen_8;
	pdwPen_32 = (unsigned int *)pucPen_8;
	
	switch(g_tFBOpr.iBpp){
		case 8:
			memset(g_pucFBMem, dwBackColor, g_iScreenSize);
			break;
		case 16:
			/*565*/
			dwRed = (dwBackColor >> 19) & 0x1f;
			dwGreen = (dwBackColor >>10) & 0x3f;
			dwBlue = (dwBackColor >> 3) & 0x1f;
			dwBackColor = (dwRed << 11) | (dwGreen << 5) | (dwBlue << 0);
			for(i = 0; i < g_iScreenSize; i+=2)
				*(pwPen_16++) = dwBackColor;
			break;
		case 32:
			for(i = 0; i < g_iScreenSize; i+=4)		
				*(pdwPen_32++) = dwBackColor;
			break;
		default:
			printf("Can't suppot this Pixelwidth\n ");
			return -1;
			break;
	}
	return 0;	
}

int FBRegister(void)
{
	return RegisterDispOpr(&g_tFBOpr);
	
}
