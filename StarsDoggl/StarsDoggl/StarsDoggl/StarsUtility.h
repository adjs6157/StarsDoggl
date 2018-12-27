#include <windows.h>

struct ST_RECT
{
	ST_RECT()
	{
		left = right = top = bottom = 0;
	}

	ST_RECT(int _left, int _right, int _top, int _bottom)
	{
		left = _left;
		right = _right;
		top = _top;
		bottom = _bottom;
	}

	int left;
	int right;
	int top;
	int bottom;
};

struct ST_POS
{
	ST_POS()
	{
		x = 0;
		y = 0;
	}

	ST_POS(int _posX, int _posY)
	{
		x = _posX;
		y = _posY;
	}

	int x;
	int y;
};