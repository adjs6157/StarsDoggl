#include <d3d11.h>
#include <dxgi1_2.h>


class ScreenShotDXGI
{
public:
	ScreenShotDXGI();
	~ScreenShotDXGI();

	virtual bool Initalize();
	virtual bool Finitalize();
	virtual bool CaptureScreen(void *pImgData, INT &nImgSize);

private:
	ID3D11Device           *m_hDevice;
	ID3D11DeviceContext    *m_hContext;
	IDXGIOutputDuplication *m_hDeskDupl;
	DXGI_OUTPUT_DESC        m_dxgiOutDesc;
};