#include <windows.h>

struct ST_RECT
{
	ST_RECT()
	{
		left = right = top = bottom = 0;
	}

	ST_RECT(DWORD _left, DWORD _right, DWORD _top, DWORD _bottom)
	{
		left = _left;
		right = _right;
		top = _top;
		bottom = _bottom;
	}

	DWORD left;
	DWORD right;
	DWORD top;
	DWORD bottom;
};

struct ST_POS
{
	ST_POS()
	{
		x = 0;
		y = 0;
	}

	ST_POS(DWORD _posX, DWORD _posY)
	{
		x = _posX;
		y = _posY;
	}

	DWORD x;
	DWORD y;
};