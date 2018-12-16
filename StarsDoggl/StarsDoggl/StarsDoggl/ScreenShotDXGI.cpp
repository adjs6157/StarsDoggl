#include "ScreenShotDXGI.h"
#include <stdio.h>
#include <assert.h>
#include <process.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

#define RESET_OBJECT(obj) { if(obj) obj->Release(); obj = NULL; }

extern int iScreenShotWidth;
extern int iScreenShotHeight;

ScreenShotDXGI::ScreenShotDXGI()
{

}

ScreenShotDXGI::~ScreenShotDXGI()
{

}

bool ScreenShotDXGI::Initalize()
{
	HRESULT hr = S_OK;


	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	//
	// Create D3D device
	//
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &m_hDevice, &FeatureLevel, &m_hContext);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Get DXGI device
	//
	IDXGIDevice *hDxgiDevice = NULL;
	hr = m_hDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&hDxgiDevice));
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Get DXGI adapter
	//
	IDXGIAdapter *hDxgiAdapter = NULL;
	hr = hDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&hDxgiAdapter));
	RESET_OBJECT(hDxgiDevice);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Get output
	//
	INT nOutput = 0;
	IDXGIOutput *hDxgiOutput = NULL;
	hr = hDxgiAdapter->EnumOutputs(nOutput, &hDxgiOutput);
	RESET_OBJECT(hDxgiAdapter);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// get output description struct
	//
	DXGI_OUTPUT_DESC        m_dxgiOutDesc;
	hDxgiOutput->GetDesc(&m_dxgiOutDesc);
	iScreenShotWidth = m_dxgiOutDesc.DesktopCoordinates.right - m_dxgiOutDesc.DesktopCoordinates.left;
	iScreenShotHeight = m_dxgiOutDesc.DesktopCoordinates.bottom - m_dxgiOutDesc.DesktopCoordinates.top;
	//
	// QI for Output 1
	//
	IDXGIOutput1 *hDxgiOutput1 = NULL;
	hr = hDxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&hDxgiOutput1));
	RESET_OBJECT(hDxgiOutput);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// Create desktop duplication
	//
	hr = hDxgiOutput1->DuplicateOutput(m_hDevice, &m_hDeskDupl);
	RESET_OBJECT(hDxgiOutput1);
	if (FAILED(hr))
	{
		return FALSE;
	}

	return TRUE;
}

bool ScreenShotDXGI::Finitalize()
{
	return true;
}

BOOL AttatchToThread(VOID)
{
	HDESK hold = GetThreadDesktop(GetCurrentThreadId());
	HDESK hCurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!hCurrentDesktop)
	{
		return FALSE;
	}

	// Attach desktop to this thread
	BOOL bDesktopAttached = SetThreadDesktop(hCurrentDesktop) ? true : false;
	int err = GetLastError();
	CloseDesktop(hold);
	CloseDesktop(hCurrentDesktop);
	hCurrentDesktop = NULL;


	return bDesktopAttached;
}

bool ScreenShotDXGI::CaptureScreen(void *pImgData, INT &nImgSize)
{

	if (!AttatchToThread())
	{
		return FALSE;
	}

	nImgSize = 0;

	IDXGIResource *hDesktopResource = NULL;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	HRESULT hr = m_hDeskDupl->AcquireNextFrame(500, &FrameInfo, &hDesktopResource);
	if (FAILED(hr))
	{
		//
		// ��һЩwin10��ϵͳ��,�������û�б仯������£�
		// ����ᷢ����ʱ���󣬵����Ⲣ���Ƿ����˴��󣬶���ϵͳ�Ż���ˢ�¶������µġ�
		// ���ԣ�����û��Ҫ����FALSE�����ز����κ����ݵ�TRUE����
		//
		return TRUE;
	}

	//
	// query next frame staging buffer
	//
	ID3D11Texture2D *hAcquiredDesktopImage = NULL;
	hr = hDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&hAcquiredDesktopImage));
	RESET_OBJECT(hDesktopResource);
	if (FAILED(hr))
	{
		return FALSE;
	}

	//
	// copy old description
	//
	D3D11_TEXTURE2D_DESC frameDescriptor;
	hAcquiredDesktopImage->GetDesc(&frameDescriptor);

	//
	// create a new staging buffer for fill frame image
	//
	ID3D11Texture2D *hNewDesktopImage = NULL;
	frameDescriptor.Usage = D3D11_USAGE_STAGING;
	frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	frameDescriptor.BindFlags = 0;
	frameDescriptor.MiscFlags = 0;
	frameDescriptor.MipLevels = 1;
	frameDescriptor.ArraySize = 1;
	frameDescriptor.SampleDesc.Count = 1;
	hr = m_hDevice->CreateTexture2D(&frameDescriptor, NULL, &hNewDesktopImage);
	if (FAILED(hr))
	{
		RESET_OBJECT(hAcquiredDesktopImage);
		m_hDeskDupl->ReleaseFrame();
		return FALSE;
	}

	//
	// copy next staging buffer to new staging buffer
	//
	m_hContext->CopyResource(hNewDesktopImage, hAcquiredDesktopImage);

	RESET_OBJECT(hAcquiredDesktopImage);
	m_hDeskDupl->ReleaseFrame();

	//
	// create staging buffer for map bits
	//
	IDXGISurface *hStagingSurf = NULL;
	hr = hNewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void **)(&hStagingSurf));
	RESET_OBJECT(hNewDesktopImage);
	if (FAILED(hr))
	{
		return FALSE;
	}

	DXGI_SURFACE_DESC hStagingSurfDesc;
	// BGRA8
	hStagingSurf->GetDesc(&hStagingSurfDesc);
	//
	// copy bits to user space
	//
	DXGI_MAPPED_RECT mappedRect;
	hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
	int imgSize = iScreenShotWidth * iScreenShotHeight * 4;
	if (SUCCEEDED(hr))
	{
		nImgSize = imgSize;
		memcpy(pImgData, mappedRect.pBits, imgSize);
		hStagingSurf->Unmap();
	}
	else {
		printf("failed.\n");
	}
	printf("finish.\n");
	RESET_OBJECT(hStagingSurf);
	return SUCCEEDED(hr);
}

//
//#include <stdio.h>
//#include <assert.h>
//#include <process.h>
//#include "D3D11.h"
//#include "DXGI1_2.h"
//#include "DXGICaptor.h"
//
//#pragma comment(lib, "D3D11.lib")
//#pragma comment(lib, "DXGI.lib")
//
//#define RESET_OBJECT(obj) { if(obj) obj->Release(); obj = NULL; }
//
//ID3D11Device           *m_hDevice = nullptr;
//ID3D11DeviceContext    *m_hContext = nullptr;
//IDXGIOutputDuplication *m_hDeskDupl = nullptr;
//
//int iWidth = 0;
//int iHeight = 0;
//
//BOOL Init()
//{
//	HRESULT hr = S_OK;
//
//
//	// Driver types supported
//	D3D_DRIVER_TYPE DriverTypes[] =
//	{
//		D3D_DRIVER_TYPE_HARDWARE,
//		D3D_DRIVER_TYPE_WARP,
//		D3D_DRIVER_TYPE_REFERENCE,
//	};
//	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);
//
//	// Feature levels supported
//	D3D_FEATURE_LEVEL FeatureLevels[] =
//	{
//		D3D_FEATURE_LEVEL_11_0,
//		D3D_FEATURE_LEVEL_10_1,
//		D3D_FEATURE_LEVEL_10_0,
//		D3D_FEATURE_LEVEL_9_1
//	};
//	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
//
//	D3D_FEATURE_LEVEL FeatureLevel;
//
//	//
//	// Create D3D device
//	//
//	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
//	{
//		hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &m_hDevice, &FeatureLevel, &m_hContext);
//		if (SUCCEEDED(hr))
//		{
//			break;
//		}
//	}
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//
//	//
//	// Get DXGI device
//	//
//	IDXGIDevice *hDxgiDevice = NULL;
//	hr = m_hDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&hDxgiDevice));
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//
//	//
//	// Get DXGI adapter
//	//
//	IDXGIAdapter *hDxgiAdapter = NULL;
//	hr = hDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&hDxgiAdapter));
//	RESET_OBJECT(hDxgiDevice);
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//
//	//
//	// Get output
//	//
//	INT nOutput = 0;
//	IDXGIOutput *hDxgiOutput = NULL;
//	hr = hDxgiAdapter->EnumOutputs(nOutput, &hDxgiOutput);
//	RESET_OBJECT(hDxgiAdapter);
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//
//	//
//	// get output description struct
//	//
//	DXGI_OUTPUT_DESC        m_dxgiOutDesc;
//	hDxgiOutput->GetDesc(&m_dxgiOutDesc);
//	iWidth = m_dxgiOutDesc.DesktopCoordinates.right - m_dxgiOutDesc.DesktopCoordinates.left;
//	iHeight = m_dxgiOutDesc.DesktopCoordinates.bottom - m_dxgiOutDesc.DesktopCoordinates.top;
//	//
//	// QI for Output 1
//	//
//	IDXGIOutput1 *hDxgiOutput1 = NULL;
//	hr = hDxgiOutput->QueryInterface(__uuidof(hDxgiOutput1), reinterpret_cast<void**>(&hDxgiOutput1));
//	RESET_OBJECT(hDxgiOutput);
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//
//	//
//	// Create desktop duplication
//	//
//	hr = hDxgiOutput1->DuplicateOutput(m_hDevice, &m_hDeskDupl);
//	RESET_OBJECT(hDxgiOutput1);
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//
//	return TRUE;
//}
//void Finit()
//{
//	RESET_OBJECT(m_hDeskDupl);
//	RESET_OBJECT(m_hContext);
//	RESET_OBJECT(m_hDevice);
//}
//
//BOOL AttatchToThread(VOID)
//{
//	HDESK hold = GetThreadDesktop(GetCurrentThreadId());
//	HDESK hCurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
//	if (!hCurrentDesktop)
//	{
//		return FALSE;
//	}
//
//	// Attach desktop to this thread
//	BOOL bDesktopAttached = SetThreadDesktop(hCurrentDesktop) ? true : false;
//	int err = GetLastError();
//	CloseDesktop(hold);
//	CloseDesktop(hCurrentDesktop);
//	//hCurrentDesktop = NULL;
//
//
//	return bDesktopAttached;
//}
//
//BOOL QueryFrame(const char *fileName, void *pImgData, INT &nImgSize)
//{
//	if (!AttatchToThread())
//	{
//		return FALSE;
//	}
//
//	nImgSize = 0;
//
//	IDXGIResource *hDesktopResource = NULL;
//	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
//	m_hDeskDupl->ReleaseFrame();
//	HRESULT hr = m_hDeskDupl->AcquireNextFrame(500, &FrameInfo, &hDesktopResource);
//	if (FAILED(hr))
//	{
//		//
//		// ��һЩwin10��ϵͳ��,�������û�б仯������£�
//		// ����ᷢ����ʱ���󣬵����Ⲣ���Ƿ����˴��󣬶���ϵͳ�Ż���ˢ�¶������µġ�
//		// ���ԣ�����û��Ҫ����FALSE�����ز����κ����ݵ�TRUE����
//		//
//		return TRUE;
//	}
//
//	//
//	// query next frame staging buffer
//	//
//	ID3D11Texture2D *hAcquiredDesktopImage = NULL;
//	hr = hDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&hAcquiredDesktopImage));
//	RESET_OBJECT(hDesktopResource);
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//
//	//
//	// copy old description
//	//
//	D3D11_TEXTURE2D_DESC frameDescriptor;
//	hAcquiredDesktopImage->GetDesc(&frameDescriptor);
//
//	//
//	// create a new staging buffer for fill frame image
//	//
//	ID3D11Texture2D *hNewDesktopImage = NULL;
//	frameDescriptor.Usage = D3D11_USAGE_STAGING;
//	frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//	frameDescriptor.BindFlags = 0;
//	frameDescriptor.MiscFlags = 0;
//	frameDescriptor.MipLevels = 1;
//	frameDescriptor.ArraySize = 1;
//	frameDescriptor.SampleDesc.Count = 1;
//	hr = m_hDevice->CreateTexture2D(&frameDescriptor, NULL, &hNewDesktopImage);
//	if (FAILED(hr))
//	{
//		RESET_OBJECT(hAcquiredDesktopImage);
//		m_hDeskDupl->ReleaseFrame();
//		return FALSE;
//	}
//
//	//
//	// copy next staging buffer to new staging buffer
//	//
//	m_hContext->CopyResource(hNewDesktopImage, hAcquiredDesktopImage);
//
//	RESET_OBJECT(hAcquiredDesktopImage);
//	m_hDeskDupl->ReleaseFrame();
//
//	//
//	// create staging buffer for map bits
//	//
//	IDXGISurface *hStagingSurf = NULL;
//	hr = hNewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void **)(&hStagingSurf));
//	RESET_OBJECT(hNewDesktopImage);
//	if (FAILED(hr))
//	{
//		return FALSE;
//	}
//	DXGI_SURFACE_DESC hStagingSurfDesc;
//	// BGRA8
//	hStagingSurf->GetDesc(&hStagingSurfDesc);
//	//
//	// copy bits to user space
//	//
//	DXGI_MAPPED_RECT mappedRect;
//	hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
//	int imgSize = iWidth * iHeight * 4;
//	if (SUCCEEDED(hr))
//	{
//		nImgSize = imgSize;
//		memcpy(pImgData, mappedRect.pBits, imgSize);
//		hStagingSurf->Unmap();
//	}
//	else {
//		printf("failed.\n");
//	}
//	printf("finish.\n");
//	RESET_OBJECT(hStagingSurf);
//	return SUCCEEDED(hr);
//}
//
//void rotateImg(unsigned char *pImgData)
//{
//	unsigned int* pImg32 = (unsigned int*)pImgData;
//	int iCount = iWidth * iHeight;
//	unsigned int* pRotateImg32 = new unsigned int[iCount];
//	for (int i = 0; i < iCount; ++i) {
//		pRotateImg32[i] = pImg32[iCount - i - 1];
//	}
//
//	unsigned int* pRotateImg32_r = (unsigned int*)pImgData;
//	for (int i = 0; i < iHeight; ++i) {
//		for (int j = 0; j < iWidth; ++j) {
//			pRotateImg32_r[j + i * iWidth] = pRotateImg32[iWidth - j - 1 + i * iWidth];
//		}
//	}
//	delete[] pRotateImg32;
//}
//
//void saveBmpFile(const char *fileName, unsigned char *pImgData, int imgLength)
//{
//	BITMAPFILEHEADER bmheader;
//	memset(&bmheader, 0, sizeof(bmheader));
//	bmheader.bfType = 0x4d42;     //ͼ���ʽ������Ϊ'BM'��ʽ��  
//	bmheader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); //���ļ���ͷ�����ݵ�ƫ����  
//	bmheader.bfSize = imgLength + bmheader.bfOffBits;//�ļ���С  
//
//	BITMAPINFOHEADER bmInfo;
//	memset(&bmInfo, 0, sizeof(bmInfo));
//	bmInfo.biSize = sizeof(bmInfo);
//	bmInfo.biWidth = iWidth;
//	bmInfo.biHeight = iHeight;
//	bmInfo.biPlanes = 1;
//	bmInfo.biBitCount = 32;
//	bmInfo.biCompression = BI_RGB;
//
//	HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
//	if (hFile != INVALID_HANDLE_VALUE) {
//		DWORD dwWritten;
//		BOOL bRet = WriteFile(hFile, &bmheader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
//		assert(TRUE == bRet);
//		bRet = WriteFile(hFile, &bmInfo, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
//		assert(TRUE == bRet);
//		bRet = WriteFile(hFile, pImgData, imgLength, &dwWritten, NULL);
//		assert(TRUE == bRet);
//		CloseHandle(hFile);
//	}
//}
//
//unsigned __stdcall capture(void *arg)
//{
//	unsigned char *pImgData = new unsigned char[iWidth * iHeight * 4];
//	int imgLength = iWidth * iHeight * 4;
//	int i = 1;
//	while (true) {
//		char buf[10] = { 0 };
//		sprintf(buf, "%d.bmp", i);
//		QueryFrame(buf, pImgData, imgLength);
//		rotateImg(pImgData);
//		saveBmpFile(buf, pImgData, imgLength);
//		++i;
//		Sleep(500);
//	}
//	return 0;
//}
//
//
//int main(int argc, TCHAR *argv[])
//{
//	if (!Init()) {
//		Finit();
//		printf("not support dxgi.");
//		return -1;
//	}
//	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, capture, NULL, 0, NULL);
//	CloseHandle(handle);
//	getchar();
//	Finit();
//	return 0;
//}