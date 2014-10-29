#pragma once
#include "ImageProcessingDlg.h"
#include "afxdialogex.h"
#include "core.h"

void ShowMatImgToWnd(CWnd* pWnd, Mat img)
{	
    if(img.empty())  
        return;  
	
    CRect drect;       
    pWnd->GetClientRect(drect);    //(drect);  (&drect);  两种方式均可，竟然

	//resize(img,img,Size(drect.right,drect.bottom),0,0,CV_INTER_LINEAR);

	CClientDC dc(pWnd);
	HDC hDC =dc.GetSafeHdc();
	
	//内存中的图像数据拷贝到屏幕上
	BYTE *bitBuffer		   = NULL;
	BITMAPINFO *bitMapinfo = NULL;
	
	int ichannels =img.channels();
	if( ichannels == 1)
	{
		bitBuffer  = new BYTE[40+4*256]; 
	}
	else if( ichannels == 3)
	{
		bitBuffer  = new BYTE[sizeof(BITMAPINFO)]; 
	}
	else
	{
		return;
	}


	if(bitBuffer == NULL)
	{	
		return;
	}


	bitMapinfo = (BITMAPINFO *)bitBuffer;
	bitMapinfo->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bitMapinfo->bmiHeader.biHeight		    = -img.rows;  //如果高度为正的，位图的起始位置在左下角。如果高度为负，起始位置在左上角。
	bitMapinfo->bmiHeader.biWidth		    = img.cols;
	bitMapinfo->bmiHeader.biPlanes			= 1;      // 目标设备的级别，必须为1	
	bitMapinfo->bmiHeader.biBitCount		= ichannels *8;     // 每个像素所需的位数，必须是1(双色), 4(16色)，8(256色)或24(真彩色)之一
	bitMapinfo->bmiHeader.biCompression	    = BI_RGB; //位图压缩类型，必须是 0(不压缩), 1(BI_RLE8压缩类型)或2(BI_RLE4压缩类型)之一
	bitMapinfo->bmiHeader.biSizeImage		= 0;      // 位图的大小，以字节为单位
	bitMapinfo->bmiHeader.biXPelsPerMeter	= 0;	  // 位图水平分辨率，每米像素数
	bitMapinfo->bmiHeader.biYPelsPerMeter	= 0;	  // 位图垂直分辨率，每米像素数
	bitMapinfo->bmiHeader.biClrUsed			= 0;	  // 位图实际使用的颜色表中的颜色数
	bitMapinfo->bmiHeader.biClrImportant	= 0;	  // 位图显示过程中重要的颜色数

	if(ichannels == 1)
	{
		for(int i=0; i<256; i++)
		{	//颜色的取值范围 (0-255)
			bitMapinfo->bmiColors[i].rgbBlue  =bitMapinfo->bmiColors[i].rgbGreen =bitMapinfo->bmiColors[i].rgbRed   =(BYTE) i;
		}

		bitMapinfo->bmiHeader.biClrUsed			= 256;	  // 位图实际使用的颜色表中的颜色数
	}
	SetStretchBltMode(hDC, COLORONCOLOR);


	StretchDIBits(hDC,
					0,
					0,
					drect.right,		//显示窗口宽度
					drect.bottom,		//显示窗口高度
					0,
					0,
					img.cols,		   //图像宽度
					img.rows,		   //图像高度
					img.data,			
					bitMapinfo,			
					DIB_RGB_COLORS, 
					SRCCOPY
				  );
	
	delete []bitBuffer;

}
