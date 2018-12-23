#include "StarsGraphy.h"
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include <io.h>

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
	delete m_pkORBTool;
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

	m_pkORBTool = new cv::ORB(5000, 1.2f, 1, 3, 0, 2, 0, 21);
	m_pkMatcher = new cv::BFMatcher(cv::NORM_HAMMING);

	m_kScreenORBInfo.aiPixelData = new unsigned char[iScreenShotWidth * iScreenShotHeight * 3];
	m_kScreenORBInfo.img = new cv::Mat(iScreenShotHeight, iScreenShotWidth, CV_8UC3, m_kScreenORBInfo.aiPixelData);
	m_kScreenORBInfo.descriptors = new cv::Mat();

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
	if (timeGetTime() - m_iLastUpdateTime < 300)
	{
		return;
	}

	m_iLastUpdateTime = timeGetTime();

	m_pkScreenShotDDRAW->CaptureScreen(m_pkScreenShotData, m_iImgDataSize);
	for (int i = 0; i < iScreenShotHeight; ++i)
	{
		for (int j = 0; j < iScreenShotWidth; ++j)
		{
			int iIndex1 = (i * iScreenShotWidth + j) * 3;
			int iIndex2 = (i * iScreenShotWidth + j) * 4;
			m_kScreenORBInfo.aiPixelData[iIndex1] = ((unsigned char*)m_pkScreenShotData)[iIndex2 + 0];
			m_kScreenORBInfo.aiPixelData[iIndex1 + 1] = ((unsigned char*)m_pkScreenShotData)[iIndex2 + 1];
			m_kScreenORBInfo.aiPixelData[iIndex1 + 2] = ((unsigned char*)m_pkScreenShotData)[iIndex2 + 2];
		}
	}

	cv::Rect r1(0, 480, 800, 600);
	cv::Mat mask = cv::Mat::zeros(m_kScreenORBInfo.img->size(), CV_8UC1);
	mask(r1).setTo(255);

	m_pkORBTool->detect(*(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, mask);
	m_pkORBTool->compute(*(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, *(m_kScreenORBInfo.descriptors));

	//RotateImg(m_pkScreenShotData);
	//SaveBmpFile("1.bmp", m_pkScreenShotData, m_iImgDataSize);

	//ST_POS kPosPic = FindPicture("test2.bmp", ST_RECT(0, 60, 0, 60));

	//FIndPictureORB("test3.bmp");


	//cv::Mat img_1 = cv::imread("test5.bmp");
	//cv::Mat img_2 = cv::imread("test3.png");

	//// -- Step 1: Detect the keypoints using STAR Detector 
	//std::vector<cv::KeyPoint> keypoints_1, keypoints_2;
	//cv::ORB orb(5000, 1.2f, 1, 3, 0, 2, 0, 31);
	//orb.detect(img_1, keypoints_1);
	//orb.detect(img_2, keypoints_2);

	//// -- Stpe 2: Calculate descriptors (feature vectors) 
	//cv::Mat descriptors_1, descriptors_2;
	//orb.compute(img_1, keypoints_1, descriptors_1);
	//orb.compute(img_2, keypoints_2, descriptors_2);

	////-- Step 3: Matching descriptor vectors with a brute force matcher 
	//cv::BFMatcher matcher(cv::NORM_HAMMING);
	//std::vector<cv::DMatch> mathces;
	//std::vector<std::vector<cv::DMatch>> mathceskn;
	//matcher.knnMatch(descriptors_1, descriptors_2, mathceskn, 2);

	//for (int i = 0; i < (int)mathceskn.size(); i++)
	//{
	//	cv::DMatch bestMatch = mathceskn[i][0];
	//	cv::DMatch betterMatch = mathceskn[i][1];
	//	cv::Point p1 = keypoints_1[bestMatch.queryIdx].pt;
	//	cv::Point p2 = keypoints_2[bestMatch.trainIdx].pt;
	//	float distance = sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
	//	float distanceRatio = bestMatch.distance / betterMatch.distance;

	//	if (distanceRatio < 0.8/* && distance < 50*/)
	//	{
	//		mathces.push_back(bestMatch);
	//	}
	//}
	//// -- dwaw matches 
	//cv::Mat img_mathes;
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, mathces, img_mathes);
	//// -- show 
	//cv::imshow("Mathces", img_mathes);
}

ST_POS StarsGraphy::FindPicture(std::string kPictureName, ST_RECT kRect)
{
	ST_POS kPoint;
	kPoint.x = -1; kPoint.y = -1;
	std::map<std::string, GamePictureInfo>::iterator itr = m_akPicture.find(kPictureName);
	if (itr == m_akPicture.end())
	{
		std::string kStr = "FindPictureͼƬδ����:";
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

ST_POS StarsGraphy::FIndPictureORB(std::string kPictureName/*, ST_RECT kRect*/)
{
	ST_POS kPoint;
	kPoint.x = -1; kPoint.y = -1;

	std::map<std::string, GameORBInfo>::iterator itr = m_akORBInfo.find(kPictureName);
	if (itr == m_akORBInfo.end())
	{
		std::string kStr = "FIndPictureORBͼƬδ����:";
		kStr += kPictureName;
		MessageBoxA(NULL, kStr.c_str(), "Warning", MB_OK);
		return kPoint;
	}

	//cv::Rect r1(kRect.left, kRect.top, kRect.right - kRect.left, kRect.bottom - kRect.top);
	//cv::Mat mask = cv::Mat::zeros(m_kScreenORBInfo.img->size(), CV_8UC1);
	//mask(r1).setTo(255);

	std::vector<cv::DMatch> mathces;
	std::vector<std::vector<cv::DMatch>> mathceskn;
	m_pkMatcher->knnMatch(*(itr->second.descriptors), *(m_kScreenORBInfo.descriptors), mathceskn, 2);

	for (int i = 0; i < (int)mathceskn.size(); i++)
	{
		cv::DMatch bestMatch = mathceskn[i][0];
		cv::DMatch betterMatch = mathceskn[i][1];
		cv::Point p1 = itr->second.keypoints[bestMatch.queryIdx].pt;
		cv::Point p2 = m_kScreenORBInfo.keypoints[bestMatch.trainIdx].pt;
		float distance = sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		if (distanceRatio < 0.8/* && distance < 50*/)
		{
			mathces.push_back(bestMatch);
		}
	}

	if (mathces.size() == 0) return kPoint;

	for (int i = 0; i < mathces.size(); ++i)
	{
		kPoint.x += m_kScreenORBInfo.keypoints[mathces[i].trainIdx].pt.x;
		kPoint.y += m_kScreenORBInfo.keypoints[mathces[i].trainIdx].pt.y;
	}

	kPoint.x /= mathces.size();
	kPoint.y /= mathces.size();
	
	return kPoint;

	//// -- dwaw matches 
	//cv::Mat img_mathes;
	//drawMatches(*(itr->second.img), itr->second.keypoints, *(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, mathces, img_mathes);
	//// -- show 
	//cv::imshow("Mathces", img_mathes);

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

			GameORBInfo& kGameORBInfo = m_akORBInfo[akPictureName[i]];
			kGameORBInfo.aiPixelData = new unsigned char[bitMapIcon.bmHeight * bitMapIcon.bmWidth * 3];

			GamePictureInfo& kGamePictureInfo = m_akPicture[akPictureName[i]];
			kGamePictureInfo.iPixelWidth = bitMapIcon.bmWidth;
			kGamePictureInfo.iPixelHeight = bitMapIcon.bmHeight;
			kGamePictureInfo.aiPixelData = new DWORD[bitMapIcon.bmWidth * bitMapIcon.bmHeight];
			for (int i = 0; i < bitMapIcon.bmHeight; ++i)
			{
				for (int j = 0; j < bitMapIcon.bmWidth; ++j)
				{
					DWORD color = GetPixel(hdcIcon, j, i);
					DWORD b = color & 0x000000FF;
					DWORD g = (color >> 8) & 0x000000FF;
					DWORD r = (color >> 16) & 0x000000FF;
					color = (b << 16) | (g << 8) | r | 0xFF000000;
					kGamePictureInfo.aiPixelData[i * bitMapIcon.bmWidth + j] = color;
					if (color != BACK_COLOR)
					{
						kGamePictureInfo.iPixelCount++;
					}
					kGameORBInfo.aiPixelData[(bitMapIcon.bmWidth * i + j) * 3] = (unsigned char)r;
					kGameORBInfo.aiPixelData[(bitMapIcon.bmWidth * i + j) * 3 + 1] = (unsigned char)g;
					kGameORBInfo.aiPixelData[(bitMapIcon.bmWidth * i + j) * 3 + 2] = (unsigned char)b;
				}
			}
			DeleteDC(hdcIcon);
			::DeleteObject(hBitMapIcon);
			
			kGameORBInfo.img = new cv::Mat(bitMapIcon.bmHeight, bitMapIcon.bmWidth, CV_8UC3, kGameORBInfo.aiPixelData);
			kGameORBInfo.descriptors = new cv::Mat();
			m_pkORBTool->detect(*(kGameORBInfo.img), kGameORBInfo.keypoints);
			m_pkORBTool->compute(*(kGameORBInfo.img), kGameORBInfo.keypoints, *(kGameORBInfo.descriptors));
		}
		else
		{
			std::string kStrWaring = "����ICONͼƬʧ��:";
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
	bmheader.bfType = 0x4d42;     //ͼ���ʽ������Ϊ'BM'��ʽ��  
	bmheader.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER); //���ļ���ͷ�����ݵ�ƫ����  
	bmheader.bfSize = iWidth * iHeight * 4 + bmheader.bfOffBits;//�ļ���С  

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
	//�ļ����  
	long   hFile = 0;
	//�ļ���Ϣ  
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼,����֮  
			//�������,�����б�  
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