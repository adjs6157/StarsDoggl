#include "ScreenShotGDI.h"
#include <windows.h>
#include <wingdi.h>

ScreenShotGDI::ScreenShotGDI()
{

}

ScreenShotGDI::~ScreenShotGDI()
{

}

bool ScreenShotGDI::Initalize()
{
	return true;
}

bool ScreenShotGDI::Finitalize()
{
	return true;
}

bool ScreenShotGDI::CaptureScreen()
{
	//HDC       hScrDC, hMemDC;// 屏幕和内存设备描述表
	//HBITMAP    hBitmap, hOldBitmap;  // 位图句柄
	//int       nX, nY;      	// 选定区域坐标
	//int       nWidth, nHeight;

	//RECT* pRtWnd = NULL;
	//WINDOWINFO windowInfo;
	//windowInfo.cbSize = sizeof(WINDOWINFO);
	//GetWindowInfo(hWnd, &windowInfo);
	//pRtWnd = &windowInfo.rcWindow;

	//// 获得选定区域坐标
	//nX = pRtWnd->left;
	//nY = pRtWnd->top;

	//nWidth = pRtWnd->right - pRtWnd->left;
	//nHeight = pRtWnd->bottom - pRtWnd->top;


	//hScrDC = GetWindowDC(hWnd); //为屏幕创建设备描述表	
	//hMemDC = CreateCompatibleDC(hScrDC);             //为屏幕设备描述表创建兼容的内存设备描述表

	//hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight); // 创建一个与屏幕设备描述表兼容的位图	
	//hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);	// 把新位图选到内存设备描述表中

	//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);
	//hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

	//DeleteObject(hOldBitmap);
	//DeleteDC(hMemDC);
	//ReleaseDC(hWnd, hScrDC);

	//return hBitmap;
	return true;
}
