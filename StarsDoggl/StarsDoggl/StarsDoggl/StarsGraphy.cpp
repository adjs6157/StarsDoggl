#include "StarsGraphy.h"
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include <io.h>
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/features2d/features2d.hpp>

int iScreenShotWidth;
int iScreenShotHeight;

#define BACK_COLOR 0xFFFF00FF

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
	if (timeGetTime() - m_iLastUpdateTime < 100)
	{
		return;
	}

	m_iLastUpdateTime = timeGetTime();

	//m_pkScreenShotDDRAW->CaptureScreen(m_pkScreenShotData, m_iImgDataSize);
	//RotateImg(m_pkScreenShotData);
	//SaveBmpFile("1.bmp", m_pkScreenShotData, m_iImgDataSize);

	//ST_POS kPosPic = FindPicture("test2.bmp", ST_RECT(0, 60, 0, 60));

	cv::Mat img_1 = cv::imread("test1.png");
	cv::Mat img_2 = cv::imread("test3.png");

	// -- Step 1: Detect the keypoints using STAR Detector 
	std::vector<cv::KeyPoint> keypoints_1, keypoints_2;
	cv::ORB orb;
	orb.detect(img_1, keypoints_1);
	orb.detect(img_2, keypoints_2);

	// -- Stpe 2: Calculate descriptors (feature vectors) 
	cv::Mat descriptors_1, descriptors_2;
	orb.compute(img_1, keypoints_1, descriptors_1);
	orb.compute(img_2, keypoints_2, descriptors_2);

	//-- Step 3: Matching descriptor vectors with a brute force matcher 
	cv::BFMatcher matcher(cv::NORM_HAMMING);
	std::vector<cv::DMatch> mathces;
	matcher.match(descriptors_1, descriptors_2, mathces);
	// -- dwaw matches 
	cv::Mat img_mathes;
	drawMatches(img_1, keypoints_1, img_2, keypoints_2, mathces, img_mathes);
	// -- show 
	cv::imshow("Mathces", img_mathes);

	cv::waitKey(0);

}

ST_POS StarsGraphy::FindPicture(std::string kPictureName, ST_RECT kRect)
{
	ST_POS kPoint;
	kPoint.x = -1; kPoint.y = -1;
	std::map<std::string, GamePictureInfo>::iterator itr = m_akPicture.find(kPictureName);
	if (itr == m_akPicture.end())
	{
		std::string kStr = "FindPicture图片未加载:";
		kStr += kPictureName;
		MessageBoxA(NULL, kStr.c_str(), "Warning", MB_OK);
		return kPoint;
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
		if (akPictureName[i].find(".bmp") == std::string::npos)
		{
			continue;
		}
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
			for (int i = 0; i < bitMapIcon.bmHeight; ++i)
			{
				for (int j = 0; j < bitMapIcon.bmWidth; ++j)
				{
					DWORD color = GetPixel(hdcIcon, j, i);
					DWORD r = color & 0x000000FF;
					DWORD g = (color >> 8) & 0x000000FF;
					DWORD b = (color >> 16) & 0x000000FF;
					color = (r << 16) | (g << 8) | b | 0xFF000000;
					kGamePictureInfo.aiPixelData[i * bitMapIcon.bmWidth + j] = color;
					if (color != BACK_COLOR)
					{
						kGamePictureInfo.iPixelCount++;
					}
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
	/*DWORD* pImg32 = pImgData;
	for (int i = 0; i < m_iImgDataSize; ++i) {
		m_pkRotateImg32[i] = pImg32[m_iImgDataSize - i - 1];
	}

	DWORD* pRotateImg32_r = pImgData;
	for (int i = 0; i < iScreenShotHeight; ++i) {
		for (int j = 0; j < iScreenShotWidth; ++j) {
			pRotateImg32_r[j + i * iScreenShotWidth] = m_pkRotateImg32[iScreenShotWidth - j - 1 + i * iScreenShotWidth];
		}
	}*/

	DWORD* pImg32 = pImgData;
	for (int i = 0; i < m_iImgDataSize; ++i) {
		m_pkRotateImg32[i] = pImg32[i];
	}

	DWORD* pRotateImg32_r = pImgData;
	for (int i = 0; i < iScreenShotHeight; ++i) {
		for (int j = 0; j < iScreenShotWidth; ++j) {
			pRotateImg32_r[j + i * iScreenShotWidth] = m_pkRotateImg32[j + (iScreenShotHeight - 1 - i) * iScreenShotWidth];
		}
	}
}

void StarsGraphy::SaveBmpFile(const char *fileName, DWORD *pImgData, int iWidth, int iHeight)
{
	BITMAPFILEHEADER bmheader;
	memset(&bmheader, 0, sizeof(bmheader));
	bmheader.bfType = 0x4d42;     //图像格式。必须为'BM'格式。  
	bmheader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); //从文件开头到数据的偏移量  
	bmheader.bfSize = iWidth * iHeight * 4 + bmheader.bfOffBits;//文件大小  

	BITMAPINFOHEADER bmInfo;
	memset(&bmInfo, 0, sizeof(bmInfo));
	bmInfo.biSize = sizeof(bmInfo);
	bmInfo.biWidth = iWidth;
	bmInfo.biHeight = iHeight;
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
		bRet = WriteFile(hFile, pImgData, iWidth * iHeight * 4, &dwWritten, NULL);
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

	for (int i = iBeginY; i < iEndY; ++i)
	{
		bool iComPareSucess = false;
		for (int j = iBeginX; j < iEndX; ++j)
		{
			int iPixelPass = 0;
			for (int k = 0; k < akGamePictureInfo.iPixelHeight; ++k)
			{
				for (int l = 0; l < akGamePictureInfo.iPixelWidth; ++l)
				{
					//DWORD r1 = akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] & 0x000000FF;
					//DWORD g1 = (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] >> 8) & 0x000000FF;
					//DWORD b1 = (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] >> 16) & 0x000000FF;
					//DWORD a1 = (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] >> 24) & 0x000000FF;

					//DWORD r2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)]) & 0x000000FF;
					//DWORD g2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)] >> 8) & 0x000000FF;
					//DWORD b2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)] >> 16) & 0x000000FF;
					//DWORD a2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)] >> 24) & 0x000000FF;

					if (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] == BACK_COLOR)
					{
						continue;
					}

					if (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] == m_pkScreenShotData[(i + k) * iScreenShotWidth + (j + l)])
					{
						iPixelPass++;
					}
				}
			}

			//m_pkRotateImg32[i * iEndX + j] = m_pkScreenShotData[(i) * iScreenShotWidth + (j)];

			float fComPareRate = iPixelPass * 1.0f / akGamePictureInfo.iPixelCount;
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

	//SaveBmpFile("compare1.bmp", akGamePictureInfo.aiPixelData, akGamePictureInfo.iPixelWidth, akGamePictureInfo.iPixelHeight);
	//SaveBmpFile("compare2.bmp", m_pkRotateImg32, iEndX, iEndY);
}
