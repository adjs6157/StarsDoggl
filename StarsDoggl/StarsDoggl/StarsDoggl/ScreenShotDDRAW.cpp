#include "ScreenShotDDRAW.h"

extern int iScreenShotWidth;
extern int iScreenShotHeight;

typedef HRESULT(*PFN_DirectDrawCreate)(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);

ScreenShotDDRAW::ScreenShotDDRAW()
{

}

ScreenShotDDRAW::~ScreenShotDDRAW()
{

}

bool ScreenShotDDRAW::Initalize()
{
	HMODULE hDll = LoadLibrary("ddraw.dll");
	if (hDll == NULL)
	{
		("无法载入ddraw.dll\n");
		return FALSE;
	}

	// 载入ddraw的导入函数
	PFN_DirectDrawCreate DirectDrawCreateFunc = (PFN_DirectDrawCreate)GetProcAddress(hDll, "DirectDrawCreate");
	if (DirectDrawCreateFunc == NULL)
	{
		("无法找到访问点：DirectDrawCreate\n");
		return FALSE;
	}

	//HRESULT hr = (*DirectDrawCreateFunc)(NULL, &m_lpDDraw, NULL);
	HRESULT hr = DirectDrawCreate(NULL, &m_lpDDraw, NULL);
	if (FAILED(hr))
	{
		("DirectDrawCreate失败\n");
		return FALSE;
	}

	hr = m_lpDDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
	if (FAILED(hr))
	{
		("SetCooperativeLevel失败\n");
		return FALSE;
	}

	DDSURFACEDESC DDSdesc;
	ZeroMemory(&DDSdesc, sizeof(DDSdesc));
	DDSdesc.dwSize = sizeof(DDSdesc);
	DDSdesc.dwFlags = DDSD_CAPS;
	DDSdesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	hr = m_lpDDraw->CreateSurface(&DDSdesc, &m_lpDDSPrime, NULL);
	if (FAILED(hr))
	{
		("CreateSurface 主表面失败\n");
		return FALSE;
	}

	ZeroMemory(&DDSdesc, sizeof(DDSdesc));
	DDSdesc.dwSize = sizeof(DDSdesc);
	DDSdesc.dwFlags = DDSD_ALL;
	hr = m_lpDDSPrime->GetSurfaceDesc(&DDSdesc);
	if (FAILED(hr))
	{
		("GetSurfaceDesc失败\n");
		return FALSE;
	}

	// 备份描述信息
	memcpy(&m_DDSdesc, &DDSdesc, sizeof(DDSdesc));
	DDSdesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	DDSdesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	hr = m_lpDDraw->CreateSurface(&DDSdesc, &m_lpDDSBack, 0);
	if (FAILED(hr))
	{
		("CreateSurface 后备表面失败\n");
		return FALSE;
	}
	return TRUE;
}

bool ScreenShotDDRAW::Finitalize()
{
	return true;
}

bool ScreenShotDDRAW::CaptureScreen(void *pData, INT &nLen)
{
	RECT rect;
	rect.left = 0; rect.right = iScreenShotWidth;
	rect.top = 0; rect.bottom = iScreenShotHeight;
	if (m_lpDDSBack == NULL)
	{
		("DDraw对象未初始化\n");
		return FALSE;
	}

	HRESULT hr = m_lpDDSBack->BltFast(rect.left, rect.top, m_lpDDSPrime, &rect, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
	if (FAILED(hr))
	{
		("BltFast失败\n");
		return FALSE;
	}

	DDSURFACEDESC surfDesc;
	ZeroMemory(&surfDesc, sizeof(surfDesc));
	surfDesc.dwSize = sizeof(surfDesc);
	hr = m_lpDDSBack->Lock(&rect, &surfDesc, DDLOCK_READONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
	if (FAILED(hr))
	{
		("Lock失败\n");
		return FALSE;
	}

	// 这里拷贝的是32位数据
	memcpy(pData, (BYTE*)surfDesc.lpSurface, surfDesc.dwWidth * surfDesc.dwHeight * surfDesc.ddpfPixelFormat.dwRGBBitCount / 8);

	m_lpDDSBack->Unlock(surfDesc.lpSurface);

	return TRUE;
}

