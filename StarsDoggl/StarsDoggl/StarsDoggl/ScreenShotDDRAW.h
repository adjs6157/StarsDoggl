#include <ddraw.h>

class ScreenShotDDRAW
{
public:
	ScreenShotDDRAW();
	~ScreenShotDDRAW();

	bool Initalize();
	bool Finitalize();
	bool CaptureScreen(void *pData, INT &nLen);

private:

	LPDIRECTDRAW        m_lpDDraw;
	LPDIRECTDRAWSURFACE m_lpDDSPrime;
	LPDIRECTDRAWSURFACE m_lpDDSBack;
	DDSURFACEDESC       m_DDSdesc;
};