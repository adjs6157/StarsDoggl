#include <windows.h>

class StarsControl
{
public:
	StarsControl();
	~StarsControl();

	bool Initalize();
	bool Finitalize();
	void Update();
	void OnKeyDown(DWORD dwKey);
	void OnKeyUp(DWORD dwKey);
};