#include <windows.h>
#include <wingdi.h>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include "StarsGamePlayer.h"
#include <DbgHelp.h>

HWND				g_hGameWnd = NULL;
int					g_iWidth = 400;
int					g_iHeight = 300;
ULONG_PTR			g_uiToken = 0;
HINSTANCE			g_hInstance = 0;

StarsGamePlayer g_kGamePlayer;

HWND g_hButtonCurrPut = 0;
HWND g_hButtonAltStopAttack = 0;
HWND g_hButtonAutoFish = 0;
HWND g_hButtonOD = 0;
HWND g_hTextLog = 0;

bool g_bClose = false;
std::string g_kLog = "";
void PrintLog(const char *format, ...)
{
	char str[256];
	va_list ap;
	va_start(ap, format);
	vsprintf_s(str, format, ap);
	va_end(ap);
	g_kLog = "\r\n" + g_kLog;
	g_kLog = str +  g_kLog;
	SetWindowTextA(g_hTextLog, g_kLog.c_str());
}
void CreateControl(HWND hWnd)
{
	g_hButtonCurrPut = (HWND)CreateWindow(TEXT("Button"),  //Button是预定义 窗体类
		TEXT("自动治疗宠物"),
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 45, 160, 35,
		hWnd,
		(HMENU)520,  //(重点)这里设置按钮id,但是 原本是设置菜单的 所以需要HMENU
		g_hInstance,
		NULL);

	g_hButtonAltStopAttack = (HWND)CreateWindow(TEXT("Button"),  //Button是预定义 窗体类
		TEXT("ALT键停止攻击"),
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 85, 160, 35,
		hWnd,
		(HMENU)521,  //(重点)这里设置按钮id,但是 原本是设置菜单的 所以需要HMENU
		g_hInstance,
		NULL);

	g_hButtonAutoFish = (HWND)CreateWindow(TEXT("Button"),  //Button是预定义 窗体类
		TEXT("自动钓鱼"),
		WS_VISIBLE | WS_CHILD | BS_CHECKBOX,
		10, 125, 160, 35,
		hWnd,
		(HMENU)522,  //(重点)这里设置按钮id,但是 原本是设置菜单的 所以需要HMENU
		g_hInstance,
		NULL);

	g_hButtonOD = (HWND)CreateWindow(TEXT("Button"),  //Button是预定义 窗体类
		TEXT("0"),
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
		10, 165, 160, 35,
		hWnd,
		(HMENU)523,  //(重点)这里设置按钮id,但是 原本是设置菜单的 所以需要HMENU
		g_hInstance,
		NULL);

	g_hTextLog = (HWND)CreateWindow(TEXT("edit"),  //Button是预定义 窗体类
		TEXT("023"),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_MULTILINE /*| ES_READONLY*/,
		175, 2, 205, 200,
		hWnd,
		(HMENU)524,  //(重点)这里设置按钮id,但是 原本是设置菜单的 所以需要HMENU
		g_hInstance,
		NULL);
}

void ProcessControl(HWND hWnd, DWORD wParam)
{
	char str[124];
	if (LOWORD(wParam) == 520 && HIWORD(wParam) == BN_CLICKED)
	{
		if (SendMessage(g_hButtonCurrPut, BM_GETCHECK, 0, 0) == BST_CHECKED)
		{
			SendMessage(g_hButtonCurrPut, BM_SETCHECK, BST_UNCHECKED, 0);
			g_kGamePlayer.SetSceneState(StarsSceneState_None);
		}
		else
		{
			SendMessage(g_hButtonCurrPut, BM_SETCHECK, BST_CHECKED, 0);
			g_kGamePlayer.SetSceneState(StarsSceneState_Battle);
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


		std::string kStrWaring = "加载ICON图片失败:";
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
		case WM_CLOSE:
			g_bClose = true;
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

//生产DUMP文件
int GenerateMiniDump(HANDLE hFile, PEXCEPTION_POINTERS pExceptionPointers, PWCHAR pwAppName)
{
	BOOL bOwnDumpFile = FALSE;
	HANDLE hDumpFile = hFile;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	typedef BOOL(WINAPI * MiniDumpWriteDumpT)(
		HANDLE,
		DWORD,
		HANDLE,
		MINIDUMP_TYPE,
		PMINIDUMP_EXCEPTION_INFORMATION,
		PMINIDUMP_USER_STREAM_INFORMATION,
		PMINIDUMP_CALLBACK_INFORMATION
		);

	MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;
	HMODULE hDbgHelp = LoadLibrary("DbgHelp.dll");
	if (hDbgHelp)
		pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

	if (pfnMiniDumpWriteDump)
	{
		if (hDumpFile == NULL || hDumpFile == INVALID_HANDLE_VALUE)
		{
			//TCHAR szPath[MAX_PATH] = { 0 };
			TCHAR szFileName[MAX_PATH] = { 0 };
			//TCHAR* szAppName = pwAppName;
			TCHAR* szVersion = "v1.0";
			TCHAR dwBufferSize = MAX_PATH;
			SYSTEMTIME stLocalTime;

			GetLocalTime(&stLocalTime);
			//GetTempPath(dwBufferSize, szPath);

			//wsprintf(szFileName, L"%s%s", szPath, szAppName);
			CreateDirectory(szFileName, NULL);

			wsprintf(szFileName, "%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
				//szPath, szAppName, szVersion,
				szVersion,
				stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
				stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
				GetCurrentProcessId(), GetCurrentThreadId());
			hDumpFile = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

			bOwnDumpFile = TRUE;
			OutputDebugString(szFileName);
		}

		if (hDumpFile != INVALID_HANDLE_VALUE)
		{
			ExpParam.ThreadId = GetCurrentThreadId();
			ExpParam.ExceptionPointers = pExceptionPointers;
			ExpParam.ClientPointers = FALSE;

			pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
				hDumpFile, MiniDumpWithDataSegs, (pExceptionPointers ? &ExpParam : NULL), NULL, NULL);

			if (bOwnDumpFile)
				CloseHandle(hDumpFile);
		}
	}

	if (hDbgHelp != NULL)
		FreeLibrary(hDbgHelp);

	return EXCEPTION_EXECUTE_HANDLER;
}


LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS lpExceptionInfo)
{
	if (IsDebuggerPresent())
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return GenerateMiniDump(NULL, lpExceptionInfo, L"test");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if (!CreateWnd(hInstance))
	{
		return 0;
	}

	g_hInstance = hInstance;

	if (!g_kGamePlayer.Initalize())
	{
		MessageBoxA(NULL, "g_kGamePlayer初始化失败", "Warning", MB_OK);
		return 0;
	}

	SetUnhandledExceptionFilter(ExceptionFilter);

	MSG msg;
	while (true)
	{
		if (g_bClose)
		{
			break;
		}
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

		g_kGamePlayer.Update();
	}
	g_kGamePlayer.Finitalize();

	FILE* pkFileLog = nullptr;
	fopen_s(&pkFileLog,"StarsLog.txt", "a+");
	if (pkFileLog)
	{
		fputs(g_kLog.c_str(), pkFileLog);
	}

	return true;
}
