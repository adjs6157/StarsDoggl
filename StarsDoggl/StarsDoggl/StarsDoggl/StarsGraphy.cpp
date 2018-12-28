#include "StarsGraphy.h"
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include <io.h>
#include <queue>

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

	m_pkORBTool = new cv::ORB(1000, 1.2f, 1, 3, 0, 2, 0, 21);
	m_pkMatcher = new cv::BFMatcher(cv::NORM_HAMMING);
	m_aiVisitPoint = new bool[iScreenShotWidth * iScreenShotHeight];

	m_kScreenORBInfo.aiPixelData = (unsigned char*)m_pkScreenShotData;// new unsigned char[iScreenShotWidth * iScreenShotHeight * 4];
	m_kScreenORBInfo.img = new cv::Mat(iScreenShotHeight, iScreenShotWidth, CV_8UC4, m_kScreenORBInfo.aiPixelData);
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

void StarsGraphy::Update(const ST_RECT& kGameRect)
{
	if (timeGetTime() - m_iLastUpdateTime < 300)
	{
		return;
	}

	m_iLastUpdateTime = timeGetTime();

	m_pkScreenShotDDRAW->CaptureScreen(m_pkScreenShotData, m_iImgDataSize);
	/*for (int i = 0; i < iScreenShotHeight; ++i)
	{
		for (int j = 0; j < iScreenShotWidth; ++j)
		{
			int iIndex1 = (i * iScreenShotWidth + j) * 3;
			int iIndex2 = (i * iScreenShotWidth + j) * 4;
			m_kScreenORBInfo.aiPixelData[iIndex1] = ((unsigned char*)m_pkScreenShotData)[iIndex2 + 0];
			m_kScreenORBInfo.aiPixelData[iIndex1 + 1] = ((unsigned char*)m_pkScreenShotData)[iIndex2 + 1];
			m_kScreenORBInfo.aiPixelData[iIndex1 + 2] = ((unsigned char*)m_pkScreenShotData)[iIndex2 + 2];
		}
	}*/


	cv::Rect r1(kGameRect.left, kGameRect.top, kGameRect.right - kGameRect.left, kGameRect.bottom - kGameRect.top);
	if (r1.x + r1.width > iScreenShotWidth) r1.width = iScreenShotWidth - r1.x;
	if (r1.y + r1.height > iScreenShotHeight) r1.height = iScreenShotHeight - r1.y;
	cv::Mat mask = cv::Mat::zeros(m_kScreenORBInfo.img->size(), CV_8UC1);
	mask(r1).setTo(255);


	m_pkORBTool->detect(*(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, mask);
	m_pkORBTool->compute(*(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, *(m_kScreenORBInfo.descriptors));

	//RotateImg(m_pkScreenShotData);
	//SaveBmpFile("1.bmp", m_pkScreenShotData, m_iImgDataSize);

	ST_POS kPosPic = FindPicture("test2.bmp", ST_RECT(0, 60, 0, 60));

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
		std::string kStr = "FindPicture图片未加载:";
		kStr += kPictureName;
		MessageBoxA(NULL, kStr.c_str(), "Warning", MB_OK);
		return kPoint;
	}
	CheckRect(kRect, itr->second.iPixelWidth, itr->second.iPixelHeight);
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
		std::string kStr = "FIndPictureORB图片未加载:";
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

ST_POS StarsGraphy::FindFont(std::string kStr, ST_RECT kRect)
{
	return ST_POS(-1, -1);
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
			kGameORBInfo.aiPixelData = new unsigned char[bitMapIcon.bmHeight * bitMapIcon.bmWidth * 4];

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
					/*kGameORBInfo.aiPixelData[(bitMapIcon.bmWidth * i + j) * 3] = (unsigned char)r;
					kGameORBInfo.aiPixelData[(bitMapIcon.bmWidth * i + j) * 3 + 1] = (unsigned char)g;
					kGameORBInfo.aiPixelData[(bitMapIcon.bmWidth * i + j) * 3 + 2] = (unsigned char)b;*/
					memcpy(&(kGameORBInfo.aiPixelData[(bitMapIcon.bmWidth * i + j) * 4]), &color, 4);
				}
			}
			DeleteDC(hdcIcon);
			::DeleteObject(hBitMapIcon);
			
			kGameORBInfo.img = new cv::Mat(bitMapIcon.bmHeight, bitMapIcon.bmWidth, CV_8UC4, kGameORBInfo.aiPixelData);
			kGameORBInfo.descriptors = new cv::Mat();
			m_pkORBTool->detect(*(kGameORBInfo.img), kGameORBInfo.keypoints);
			m_pkORBTool->compute(*(kGameORBInfo.img), kGameORBInfo.keypoints, *(kGameORBInfo.descriptors));
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


void StarsGraphy::ComPareImageNormal(int iBeginX, int iEndX, int iBeginY, int iEndY, GamePictureInfo& kGamePictureInfo)
{
	memset(m_aiVisitPoint, 0, iScreenShotWidth * iScreenShotHeight);
	std::queue<ST_POS> kQueue;
	ST_POS kStarPoint((iBeginX + iEndX) / 2, (iBeginY + iEndY) / 2);
	kQueue.push(kStarPoint);
	int iMaxIndex = iScreenShotWidth * iScreenShotHeight - 1;
	const int aiDir[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	ST_POS kTempPoint;
	int iTempIndex;
	while (!kQueue.empty())
	{
		kStarPoint = kQueue.front();
		kQueue.pop();
		for (int k = 0; k < 4; ++k)
		{
			kTempPoint.x = aiDir[k][0] + kStarPoint.x;
			kTempPoint.y = aiDir[k][1] + kStarPoint.y;
			iTempIndex = kTempPoint.x + kTempPoint.y * iScreenShotWidth;
			if (kTempPoint.x >= iBeginX && kTempPoint.x <= iEndX && kTempPoint.y >= iBeginY && kTempPoint.y <= iEndY && !m_aiVisitPoint[iTempIndex])
			{
				kQueue.push(kTempPoint);
				m_aiVisitPoint[iTempIndex] = true;
			}
		}

		int iPixelPass = 0;
		for (int k = 0; k < kGamePictureInfo.iPixelHeight; ++k)
		{
			for (int l = 0; l < kGamePictureInfo.iPixelWidth; ++l)
			{
				iTempIndex = k * kGamePictureInfo.iPixelWidth + l;
				if (kGamePictureInfo.aiPixelData[iTempIndex] == BACK_COLOR)
				{
					continue;
				}

				if (kGamePictureInfo.aiPixelData[iTempIndex] == m_pkScreenShotData[(kStarPoint.y + k) * iScreenShotWidth + (kStarPoint.x + l)])
				{
					iPixelPass++;
				}
				else
				{
					iPixelPass = -1;
					break;
				}
			}
			if (iPixelPass == -1) break;
		}

		if (iPixelPass == kGamePictureInfo.iPixelCount)
		{
			kGamePictureInfo.fComPareRate = 1;
			kGamePictureInfo.iComPareBeginX = kStarPoint.x;
			kGamePictureInfo.iComPareBeginY = kStarPoint.y;
		}
	}


	//akGamePictureInfo.fComPareRate = 0;

	//for (int i = iBeginY; i < iEndY; ++i)
	//{
	//	bool iComPareSucess = false;
	//	for (int j = iBeginX; j < iEndX; ++j)
	//	{
	//		int iPixelPass = 0;
	//		for (int k = 0; k < akGamePictureInfo.iPixelHeight; ++k)
	//		{
	//			for (int l = 0; l < akGamePictureInfo.iPixelWidth; ++l)
	//			{
	//				//DWORD r1 = akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] & 0x000000FF;
	//				//DWORD g1 = (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] >> 8) & 0x000000FF;
	//				//DWORD b1 = (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] >> 16) & 0x000000FF;
	//				//DWORD a1 = (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] >> 24) & 0x000000FF;

	//				//DWORD r2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)]) & 0x000000FF;
	//				//DWORD g2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)] >> 8) & 0x000000FF;
	//				//DWORD b2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)] >> 16) & 0x000000FF;
	//				//DWORD a2 = (m_pkScreenShotData[(iScreenShotHeight - 1 - i - k) * iScreenShotWidth + (j + l)] >> 24) & 0x000000FF;

	//				if (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] == BACK_COLOR)
	//				{
	//					continue;
	//				}

	//				if (akGamePictureInfo.aiPixelData[k * akGamePictureInfo.iPixelWidth + l] == m_pkScreenShotData[(i + k) * iScreenShotWidth + (j + l)])
	//				{
	//					iPixelPass++;
	//				}
	//			}
	//		}

	//		//m_pkRotateImg32[i * iEndX + j] = m_pkScreenShotData[(i) * iScreenShotWidth + (j)];

	//		float fComPareRate = iPixelPass * 1.0f / akGamePictureInfo.iPixelCount;
	//		if (fComPareRate > akGamePictureInfo.fComPareRate)
	//		{
	//			akGamePictureInfo.fComPareRate = fComPareRate;
	//			if (fComPareRate > 0.9f)
	//			{
	//				akGamePictureInfo.iComPareBeginX = i;
	//				akGamePictureInfo.iComPareBeginY = j;
	//				iComPareSucess = true;
	//				break;
	//			}
	//		}
	//	}
	//	if (iComPareSucess) break;
	//}

	//SaveBmpFile("compare1.bmp", akGamePictureInfo.aiPixelData, akGamePictureInfo.iPixelWidth, akGamePictureInfo.iPixelHeight);
	//SaveBmpFile("compare2.bmp", m_pkRotateImg32, iEndX, iEndY);
}

void StarsGraphy::CheckRect(ST_RECT& kRect, int iWidth, int iHeight)
{
	if (kRect.left < 0) kRect.left = 0;
	if (kRect.right > iScreenShotWidth - iWidth) kRect.right = iScreenShotWidth - iWidth;
	if (kRect.top < 0) kRect.top = 0;
	if (kRect.bottom > iScreenShotHeight - iHeight) kRect.bottom = iScreenShotHeight - iHeight;
}