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
	//HDC       hScrDC, hMemDC;// ��Ļ���ڴ��豸������
	//HBITMAP    hBitmap, hOldBitmap;  // λͼ���
	//int       nX, nY;      	// ѡ����������
	//int       nWidth, nHeight;

	//RECT* pRtWnd = NULL;
	//WINDOWINFO windowInfo;
	//windowInfo.cbSize = sizeof(WINDOWINFO);
	//GetWindowInfo(hWnd, &windowInfo);
	//pRtWnd = &windowInfo.rcWindow;

	//// ���ѡ����������
	//nX = pRtWnd->left;
	//nY = pRtWnd->top;

	//nWidth = pRtWnd->right - pRtWnd->left;
	//nHeight = pRtWnd->bottom - pRtWnd->top;


	//hScrDC = GetWindowDC(hWnd); //Ϊ��Ļ�����豸������	
	//hMemDC = CreateCompatibleDC(hScrDC);             //Ϊ��Ļ�豸�����������ݵ��ڴ��豸������

	//hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight); // ����һ������Ļ�豸��������ݵ�λͼ	
	//hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);	// ����λͼѡ���ڴ��豸��������

	//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);
	//hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

	//DeleteObject(hOldBitmap);
	//DeleteDC(hMemDC);
	//ReleaseDC(hWnd, hScrDC);

	//return hBitmap;
	return true;
}
