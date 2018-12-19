#include <windows.h>
#include <wingdi.h>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "StarsGraphy.h"
#include "StarsControl.h"

HWND				g_hGameWnd = NULL;
int					g_iWidth = 400;
int					g_iHeight = 300;
ULONG_PTR			g_uiToken = 0;
HINSTANCE			g_hInstance = 0;
StarsGraphy			g_kStarsGraphy;
StarsControl		g_kStarsControl;

HWND g_hButtonCurrPut = 0;
HWND g_hButtonAltStopAttack = 0;
HWND g_hButtonAutoFish = 0;

HWND g_hButtonOD = 0;

int* iTest = 0;
void CreateControl(HWND hWnd)
{
	g_hButtonCurrPut = (HWND)CreateWindow(TEXT("Button"),  //Button��Ԥ���� ������
		TEXT("�Զ����Ƴ���"),
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 45, 160, 35,
		hWnd,
		(HMENU)520,  //(�ص�)�������ð�ťid,���� ԭ�������ò˵��� ������ҪHMENU
		g_hInstance,
		NULL);

	g_hButtonAltStopAttack = (HWND)CreateWindow(TEXT("Button"),  //Button��Ԥ���� ������
		TEXT("ALT��ֹͣ����"),
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 85, 160, 35,
		hWnd,
		(HMENU)521,  //(�ص�)�������ð�ťid,���� ԭ�������ò˵��� ������ҪHMENU
		g_hInstance,
		NULL);

	g_hButtonAutoFish = (HWND)CreateWindow(TEXT("Button"),  //Button��Ԥ���� ������
		TEXT("�Զ�����"),
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 125, 160, 35,
		hWnd,
		(HMENU)522,  //(�ص�)�������ð�ťid,���� ԭ�������ò˵��� ������ҪHMENU
		g_hInstance,
		NULL);

	iTest = new int;
	iTest = 0;

	g_hButtonOD = (HWND)CreateWindow(TEXT("Button"),  //Button��Ԥ���� ������
		TEXT("0"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		10, 165, 160, 35,
		hWnd,
		(HMENU)523,  //(�ص�)�������ð�ťid,���� ԭ�������ò˵��� ������ҪHMENU
		g_hInstance,
		NULL);
}

void ProcessControl(HWND hWnd, DWORD wParam)
{
	if (LOWORD(wParam) == 520 && HIWORD(wParam) == BN_CLICKED)
	{
		if (SendMessage(g_hButtonCurrPut, BM_GETCHECK, 0, 0) == BST_CHECKED)
		{
			SendMessage(g_hButtonCurrPut, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		else
		{
			SendMessage(g_hButtonCurrPut, BM_SETCHECK, BST_CHECKED, 0);
		}
	}
	else if (LOWORD(wParam) == 521 && HIWORD(wParam) == BN_CLICKED)
	{
		if (SendMessage(g_hButtonAltStopAttack, BM_GETCHECK, 0, 0) == BST_CHECKED)
		{
			SendMessage(g_hButtonAltStopAttack, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		else
		{
			SendMessage(g_hButtonAltStopAttack, BM_SETCHECK, BST_CHECKED, 0);
		}
	}
	else if (LOWORD(wParam) == 522 && HIWORD(wParam) == BN_CLICKED)
	{
		if (SendMessage(g_hButtonAutoFish, BM_GETCHECK, 0, 0) == BST_CHECKED)
		{
			SendMessage(g_hButtonAutoFish, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		else
		{
			SendMessage(g_hButtonAutoFish, BM_SETCHECK, BST_CHECKED, 0);
		}
	}
	else if (LOWORD(wParam) == 523 && HIWORD(wParam) == BN_CLICKED)
	{
		CHAR str[100];
		GetWindowTextA(g_hButtonOD, str, 100);
		int iTestNum = atoi(str);
		iTestNum++;
		_itoa_s(iTestNum, str, 100);
		SetWindowTextA(g_hButtonOD, str);


		std::string kStrWaring = "����ICONͼƬʧ��:";
		kStrWaring = kStrWaring;
		MessageBoxA(NULL, kStrWaring.c_str(), "Warning", MB_OK);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_PAINT:
			DefWindowProc(hWnd, message, wParam, lParam);
			break;
		case WM_CREATE:
			CreateControl(hWnd);
			DefWindowProc(hWnd, message, wParam, lParam);
			break;
		case WM_COMMAND:
			ProcessControl(hWnd, wParam);
			DefWindowProc(hWnd, message, wParam, lParam);
			break;
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
	}
}

BOOL CreateWnd(HINSTANCE hInstance)
{
	CHAR acWindowClass[256] = "StarsDoggl";

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = acWindowClass;
	wcex.hIconSm = NULL;

	RegisterClassEx(&wcex);

	DEVMODE devmode;
	memset(&devmode, 0, sizeof(devmode));
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
	int iMaxGraphWidth = devmode.dmPelsWidth;
	int iMaxGraphHeight = devmode.dmPelsHeight;
	int iWindowX = iMaxGraphWidth / 2 - g_iWidth / 2;
	int iWindowY = iMaxGraphHeight / 2 - g_iHeight / 2 - 32;

	g_hGameWnd = CreateWindow(acWindowClass, acWindowClass, (WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN) & ~WS_THICKFRAME, iWindowX, iWindowY, g_iWidth, g_iHeight, NULL, NULL, hInstance, NULL);
	if (!g_hGameWnd)
	{
		return FALSE;
	}

	ShowWindow(g_hGameWnd, SW_SHOW);
	UpdateWindow(g_hGameWnd);
	BringWindowToTop(g_hGameWnd);
	return TRUE;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (!CreateWnd(hInstance))
	{
		return 0;
	}

	g_hInstance = hInstance;

	if (!g_kStarsGraphy.Initalize())
	{
		MessageBoxA(NULL, "g_kStarsGraphy��ʼ��ʧ��", "Warning", MB_OK);
		return 0;
	}

	if (!g_kStarsControl.Initalize())
	{
		MessageBoxA(NULL, "g_kStarsControl��ʼ��ʧ��", "Warning", MB_OK);
		return 0;
	}

	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			continue;
		}

		static int s_iFrameTime = timeGetTime();

		int iDeltaTime = timeGetTime() - s_iFrameTime;
		iDeltaTime = 33 - iDeltaTime;

		if (iDeltaTime > 0)
		{
			Sleep(iDeltaTime);
		}

		s_iFrameTime = timeGetTime();

		g_kStarsGraphy.Update();
		g_kStarsControl.Update();
	}

	g_kStarsGraphy.Finitalize();
	g_kStarsControl.Finitalize();

	return true;
}
