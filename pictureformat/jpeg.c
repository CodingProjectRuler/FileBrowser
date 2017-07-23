
#include <picformat_manager.h>
#include <file.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <jpeglib.h>

typedef struct MyErrorMgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	        /* for return to caller */
}T_MyErrorMgr, *PT_MyErrorMgr;

static int isJPEGFormat(PT_FileInfos ptFileInfos);
static int GetPixelDatasFrmJPEGFile(PT_FileInfos ptFileInfos, PT_pictureInfos pictureInfos, T_ZoonInfos tZoonInfos);
static int FreePixelDatasForJPEG(PT_pictureInfos pictureInfos);

static T_PictureFileParser g_tJPEGParser = {
	.name               = "jpeg",
	.isSupport         = isJPEGFormat,
	.GetPixelDatas  = GetPixelDatasFrmJPEGFile,
	.FreePixelDatas = FreePixelDatasForJPEG, 
};

/*自定义错误函数*/
static void MyErrorExit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	PT_MyErrorMgr ptMyerr = (PT_MyErrorMgr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(ptMyerr->setjmp_buffer, 1);
}


static int isJPEGFormat(PT_FileInfos ptFileInfos)
{
	int iRet;
	struct jpeg_decompress_struct cinfo;
	T_MyErrorMgr jerr;
	fseek(ptFileInfos->tFp, 0, SEEK_SET);
	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
#if 0	
	jerr.pub.error_exit =  MyErrorExit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		fclose(ptFileInfos->tFp);
		return 0;	
	}
#endif
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, ptFileInfos->tFp);
	iRet = jpeg_read_header(&cinfo, TRUE);
	jpeg_abort_decompress(&cinfo);
 
	return (iRet == JPEG_HEADER_OK);
}

static int GetPixelDatasFrmJPEGFile(PT_FileInfos ptFileInfos, PT_pictureInfos pictureInfos, T_ZoonInfos tZoonInfos)
{
	int iLineBytes;
	unsigned char *aucLineBuffer;
	unsigned char *pucDest;
	struct jpeg_decompress_struct cinfo;
	T_MyErrorMgr jerr;
	fseek(ptFileInfos->tFp, 0, SEEK_SET);	
	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
#if 0
	jerr.pub.error_exit =  MyErrorExit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		jpeg_destroy_decompress(&cinfo);
		fclose(ptFileInfos->tFp);
		return 0;	
	}
#endif 
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, ptFileInfos->tFp);
	(void)jpeg_read_header(&cinfo, TRUE);
	
	/* 源信息 */
	pictureInfos->iWidth  = cinfo.image_width;
	pictureInfos->iHeight = cinfo.image_height;
	
	if (ZoonHandler(tZoonInfos, pictureInfos)) {
		cinfo.scale_num = pictureInfos->addNumber;
		cinfo.scale_denom = pictureInfos->declineNumber;		
	}

	printf("%d/%d\n", cinfo.scale_num, cinfo.scale_denom);
	jpeg_start_decompress(&cinfo);

	iLineBytes = cinfo.output_width * cinfo.output_components;
	aucLineBuffer = malloc(iLineBytes);
	if (aucLineBuffer == NULL)  {
		return -1;
	}
	
	pictureInfos->iWidth  = cinfo.output_width;
	pictureInfos->iHeight = cinfo.output_height;
	pictureInfos->iLineBytes  = iLineBytes;
	pictureInfos->iTotalBytes = cinfo.output_height * pictureInfos->iLineBytes;

	/*确定真正的压缩比*/
	pictureInfos->addNumber = pictureInfos->declineNumber = 1;
	printf("%d/%d\n", cinfo.output_width, cinfo.image_width);
	if (cinfo.output_width > cinfo.image_width) {
		/*放大*/
		pictureInfos->addNumber = cinfo.output_width / cinfo.image_width;
		if (cinfo.output_width % cinfo.image_width) {
			pictureInfos->addNumber++;
		}
		pictureInfos->declineNumber = 1;
	}
	else if (cinfo.output_width < cinfo.image_width) {
		/*缩小*/
		pictureInfos->addNumber = 1;
		pictureInfos->declineNumber = cinfo.image_width / cinfo.output_width;
		if (cinfo.image_width % cinfo.output_width) {
			pictureInfos->declineNumber++;
		}
	}
	
	pictureInfos->pucDataAddress = malloc(pictureInfos->iTotalBytes);
	if (NULL == pictureInfos->pucDataAddress) {
		return -1;
	}

	pucDest = pictureInfos->pucDataAddress;

	// 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
	while (cinfo.output_scanline < cinfo.output_height) 
	{
        /* 得到一行数据,里面的颜色格式为0xRR, 0xGG, 0xBB */
		(void) jpeg_read_scanlines(&cinfo, &aucLineBuffer, 1);
		ConvertFormatForOneLine(pictureInfos->iWidth, cinfo.output_components*8, pictureInfos->iBpp, pucDest, aucLineBuffer, "jpeg");
		pucDest += pictureInfos->iLineBytes;
	}
	
	free(aucLineBuffer);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;	
}



static int FreePixelDatasForJPEG(PT_pictureInfos pictureInfos)
{
	free(pictureInfos->pucDataAddress);
	return 0;
}

int JPEGParserInit(void)
{
	return RegisterPicFileParser(&g_tJPEGParser);
}

