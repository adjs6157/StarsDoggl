#include "StarsGraphy.h"
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include <io.h>

int iScreenShotWidth;
int iScreenShotHeight;

StarsGraphy::StarsGraphy()
{
	m_pkScreenShotDDRAW = new ScreenShotDDRAW();
	m_pkScreenShotData = nullptr;
}

StarsGraphy::~StarsGraphy()
{
	delete m_pkScreenShotDDRAW;
}

bool StarsGraphy::Initalize()
{
	iScreenShotWidth = GetSystemMetrics(SM_CXSCREEN);
	iScreenShotHeight = GetSystemMetrics(SM_CYSCREEN);

	m_pkScreenShotData = new DWORD[iScreenShotWidth * iScreenShotHeight * 4];
	m_pkRotateImg32 = new DWORD[iScreenShotWidth * iScreenShotHeight * 4];
	m_iImgDataSize = iScreenShotWidth * iScreenShotHeight * 4;

	if (!m_pkScreenShotDDRAW->Initalize())
	{
		return false;
	}

	LoadLocalPicture();

	m_iLastUpdateTime = timeGetTime();
	return true;
}

bool StarsGraphy::Finitalize()
{
	if (!m_pkScreenShotDDRAW->Finitalize())
	{
		return false;
	}

	if (m_pkScreenShotData)
	{
		delete m_pkScreenShotData;
		m_pkScreenShotData = nullptr;
	}
	return true;
}

void StarsGraphy::Update()
{
	m_pkScreenShotDDRAW->CaptureScreen(m_pkScreenShotData, m_iImgDataSize);
	RotateImg(m_pkScreenShotData);
}

POINT StarsGraphy::FindPicture(std::string kPictureName, RECT kRect)
{
	POINT kPoint;
	kPoint.x = -1; kPoint.y = -1;
	std::map<std::string, GamePictureInfo>::iterator itr = m_akPicture.find(kPictureName);
	if (itr == m_akPicture.end())
	{
		MessageBoxA(NULL, "FindPicture失败", "Warning", MB_OK);
	}

	ComPareImageNormal(kRect.left, kRect.right, kRect.top, kRect.bottom, itr->second);
	if (itr->second.fComPareRate > 0.9f)
	{
		kPoint.x = itr->second.iComPareBeginX;
		kPoint.y = itr->second.iComPareBeginY;
	}

	return kPoint;
}

void StarsGraphy::LoadLocalPicture()
{
	std::vector<std::string> akPicturePath;
	std::vector<std::string> akPictureName;
	GetFiles("./Picture", akPicturePath, akPictureName);

	for (int i = 0; i < akPicturePath.size(); ++i)
	{
		HBITMAP hBitMapIcon = (HBITMAP)::LoadImageA(NULL, akPicturePath[i].c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		if (hBitMapIcon != 0)
		{
			HDC hdcIcon = CreateCompatibleDC(NULL);
			BITMAP  bitMapIcon;
			GetObject(hBitMapIcon, sizeof(BITMAP), &bitMapIcon);
			SelectObject(hdcIcon, hBitMapIcon);

			GamePictureInfo& kGamePictureInfo = m_akPicture[akPictureName[i]];
			kGamePictureInfo.iPixelWidth = bitMapIcon.bmWidth;
			kGamePictureInfo.iPixelHeight = bitMapIcon.bmHeight;
			kGamePictureInfo.aiPixelData = new DWORD[bitMapIcon.bmWidth * bitMapIcon.bmHeight];
			for (int i = 0; i < bitMapIcon.bmWidth; ++i)
			{
				for (int j = 0; j < bitMapIcon.bmHeight; ++j)
				{
					kGamePictureInfo.aiPixelData[i * bitMapIcon.bmHeight + j] = GetPixel(hdcIcon, i, j);
				}
			}
			DeleteDC(hdcIcon);
			::DeleteObject(hBitMapIcon);
		}
		else
		{
			std::string kStrWaring = "加载ICON图片失败:";
			kStrWaring = kStrWaring + akPicturePath[i];
			MessageBoxA(NULL, kStrWaring.c_str(), "Warning", MB_OK);
		}
	}
}

void StarsGraphy::RotateImg(DWORD *pImgData)
{
	DWORD* pImg32 = pImgData;
	for (int i = 0; i < m_iImgDataSize; ++i) {
		m_pkRotateImg32[i] = pImg32[m_iImgDataSize - i - 1];
	}

	DWORD* pRotateImg32_r = pImgData;
	for (int i = 0; i < iScreenShotHeight; ++i) {
		for (int j = 0; j < iScreenShotWidth; ++j) {
			pRotateImg32_r[j + i * iScreenShotWidth] = m_pkRotateImg32[iScreenShotWidth - j - 1 + i * iScreenShotWidth];
		}
	}
}

void StarsGraphy::SaveBmpFile(const char *fileName, unsigned char *pImgData, int imgLength)
{
	BITMAPFILEHEADER bmheader;
	memset(&bmheader, 0, sizeof(bmheader));
	bmheader.bfType = 0x4d42;     //图像格式。必须为'BM'格式。  
	bmheader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); //从文件开头到数据的偏移量  
	bmheader.bfSize = imgLength + bmheader.bfOffBits;//文件大小  

	BITMAPINFOHEADER bmInfo;
	memset(&bmInfo, 0, sizeof(bmInfo));
	bmInfo.biSize = sizeof(bmInfo);
	bmInfo.biWidth = iScreenShotWidth;
	bmInfo.biHeight = iScreenShotHeight;
	bmInfo.biPlanes = 1;
	bmInfo.biBitCount = 32;
	bmInfo.biCompression = BI_RGB;

	HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD dwWritten;
		BOOL bRet = WriteFile(hFile, &bmheader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		assert(TRUE == bRet);
		bRet = WriteFile(hFile, &bmInfo, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
		assert(TRUE == bRet);
		bRet = WriteFile(hFile, pImgData, imgLength, &dwWritten, NULL);
		assert(TRUE == bRet);
		CloseHandle(hFile);
	}
}

void StarsGraphy::GetFiles(std::string path, std::vector<std::string>& filePaths, std::vector<std::string>& fileNames)
{
	//文件句柄  
	long   hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					GetFiles(p.assign(path).append("\\").append(fileinfo.name), filePaths, fileNames);
			}
			else
			{
				filePaths.push_back(p.assign(path).append("\\").append(fileinfo.name));
				fileNames.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}


void StarsGraphy::ComPareImageNormal(int iBeginX, int iEndX, int iBeginY, int iEndY, GamePictureInfo& akGamePictureInfo)
{
	akGamePictureInfo.fComPareRate = 0;

	for (int i = iBeginX; i < iEndX; ++i)
	{
		bool iComPareSucess = false;
		for (int j = iBeginY; j < iEndY; ++j)
		{
			int iPixelPass = 0;
			for (int k = 0; k < akGamePictureInfo.iPixelWidth; ++k)
			{
				for (int l = 0; l < akGamePictureInfo.iPixelHeight; ++l)
				{
					if (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelHeight + l] == m_pkScreenShotData[i + k * iScreenShotWidth + j + l])
					{
						iPixelPass++;
					}
				}
			}

			float fComPareRate = iPixelPass * 1.0f / akGamePictureInfo.iPixelWidth / akGamePictureInfo.iPixelHeight;
			if (fComPareRate > akGamePictureInfo.fComPareRate)
			{
				akGamePictureInfo.fComPareRate = fComPareRate;
				if (fComPareRate > 0.9f)
				{
					akGamePictureInfo.iComPareBeginX = i;
					akGamePictureInfo.iComPareBeginY = j;
					iComPareSucess = true;
					break;
				}
			}
		}
		if (iComPareSucess) break;
	}
}
