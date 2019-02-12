#include "StarsGraphy.h"
#include <windows.h>
#include <strsafe.h>
#include <assert.h>
#include <io.h>
#include <queue>
#include <opencv2/nonfree/nonfree.hpp>

int iScreenShotWidth;
int iScreenShotHeight;

#define BACK_COLOR 0xFFFF00FF

#define COLOR_A(color)	(color >> 24)		& 0x000000FF
#define COLOR_R(color)	(color >> 16)		& 0x000000FF
#define COLOR_G(color)	(color >> 8)		& 0x000000FF
#define COLOR_B(color)	color				& 0x000000FF



extern void PrintLog(const char *format, ...);
StarsGraphy::StarsGraphy()
{
	m_pkScreenShotDDRAW = new ScreenShotDDRAW();
	m_pkScreenShotData = nullptr;
}

StarsGraphy::~StarsGraphy()
{
	delete m_pkScreenShotDDRAW;
	delete m_pkORBTool;
	delete[] m_akQueue;
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

	m_pkORBTool = new cv::ORB(5000, 1.2f, 3, 0, 0, 3, 0, 5);
	m_pkMatcher = new cv::BFMatcher(cv::NORM_HAMMING2);
	m_aiVisitPoint = new bool[iScreenShotWidth * iScreenShotHeight];
	m_akQueue = new ST_POS[iScreenShotHeight * iScreenShotWidth];

	m_kScreenORBInfo.aiPixelData = (unsigned char*)m_pkScreenShotData;// new unsigned char[iScreenShotWidth * iScreenShotHeight * 4];
	m_kScreenORBInfo.img = new cv::Mat(iScreenShotHeight, iScreenShotWidth, CV_8UC4, m_kScreenORBInfo.aiPixelData);
	m_kScreenORBInfo.descriptors = new cv::Mat();

	LoadLocalPicture();

	m_iLastUpdateTime = 0;
	m_iLastUpdateTimeORB = 0;
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

bool biggerSort(std::vector<cv::Point> v1, std::vector<cv::Point> v2)
{
	return cv::contourArea(v1) > cv::contourArea(v2);
}

void StarsGraphy::Update(const ST_RECT& kGameRect)
{
	//if (timeGetTime() - m_iLastUpdateTime < 300)
	//{
	//	return;
	//}

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

	//if (timeGetTime() - m_iLastUpdateTimeORB > 2000)
	//{
	//	m_iLastUpdateTimeORB = timeGetTime();
	//	cv::Rect r1(kGameRect.left, kGameRect.top, kGameRect.right - kGameRect.left, kGameRect.bottom - kGameRect.top);
	//	if (r1.x + r1.width > iScreenShotWidth) r1.width = iScreenShotWidth - r1.x;
	//	if (r1.y + r1.height > iScreenShotHeight) r1.height = iScreenShotHeight - r1.y;
	//	cv::Mat mask = cv::Mat::zeros(m_kScreenORBInfo.img->size(), CV_8UC1);
	//	mask(r1).setTo(255);


	//	m_pkORBTool->detect(*(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, mask);
	//	m_pkORBTool->compute(*(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, *(m_kScreenORBInfo.descriptors));
	//}

	//RotateImg(m_pkScreenShotData);
	//SaveBmpFile("1.bmp", m_pkScreenShotData, iScreenShotWidth, iScreenShotHeight);

	//ST_POS kPosPic = FindPicture("test2.bmp", ST_RECT(0, 60, 0, 60));

	//FIndPictureORB("Monster6.bmp");


	///////////////ORB////////////////////////////////////////
	//cv::Mat img_1 = cv::imread("Monster9.png");
	//cv::flip(img_1, img_1, 1);
	//cv::Mat img_2 = cv::imread("test6.png");

	//// -- Step 1: Detect the keypoints using STAR Detector 
	//std::vector<cv::KeyPoint> keypoints_1, keypoints_2;
	//cv::ORB orb(3000, 1.2f, 3, 0, 0, 4, 0, 5);
	//orb.detect(img_1, keypoints_1);
	//orb.detect(img_2, keypoints_2);

	//// -- Stpe 2: Calculate descriptors (feature vectors) 
	//cv::Mat descriptors_1, descriptors_2;
	//orb.compute(img_1, keypoints_1, descriptors_1);
	//orb.compute(img_2, keypoints_2, descriptors_2);

	////-- Step 3: Matching descriptor vectors with a brute force matcher 
	//cv::BFMatcher matcher(cv::NORM_HAMMING2);
	//std::vector<cv::DMatch> mathces;
	//std::vector<std::vector<cv::DMatch>> mathceskn;
	//matcher.knnMatch(descriptors_1, descriptors_2, mathceskn, 2);
	////matcher.match(descriptors_1, descriptors_2, mathces);

	//for (int i = 0; i < (int)mathceskn.size(); i++)
	//{
	//	cv::DMatch bestMatch = mathceskn[i][0];
	//	cv::DMatch betterMatch = mathceskn[i][1];
	//	cv::Point p1 = keypoints_1[bestMatch.queryIdx].pt;
	//	cv::Point p2 = keypoints_2[bestMatch.trainIdx].pt;
	//	float distance = sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
	//	float distanceRatio = bestMatch.distance / betterMatch.distance;

	//	if (distanceRatio < 0.1/* && distance < 50*/)
	//	{
	//		mathces.push_back(bestMatch);
	//	}
	//}
	//// -- dwaw matches 
	//cv::Mat img_mathes;
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, mathces, img_mathes);
	//// -- show 
	//cv::imshow("Mathces", img_mathes);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////FAST/////////////////////////////////////////////////////////////////////////
	//cv::Mat img_1 = cv::imread("Monster8.png");
	//cv::Mat img_2 = cv::imread("test6.png");

	//
	//// -- Step 1: Detect the keypoints using STAR Detector 
	//std::vector<cv::KeyPoint> keypoints_1, keypoints_2;
	//cv::FASTX(img_1, keypoints_1, 20, true, 2);
	//cv::FASTX(img_2, keypoints_2, 20, true, 2);
	////cv::StarDetector detector;
	////detector.detect(img_1, keypoints_1);
	////detector.detect(img_2, keypoints_2);

	//// -- Stpe 2: Calculate descriptors (feature vectors) 
	//cv::BriefDescriptorExtractor brief;
	//cv::Mat descriptors_1, descriptors_2;
	//brief.compute(img_1, keypoints_1, descriptors_1);
	//brief.compute(img_2, keypoints_2, descriptors_2);

	////-- Step 3: Matching descriptor vectors with a brute force matcher 
	//cv::BFMatcher matcher(cv::NORM_HAMMING);
	//std::vector<cv::DMatch> mathces;
	//matcher.match(descriptors_1, descriptors_2, mathces);

	//// -- dwaw matches 
	//cv::Mat img_mathes;
	//drawMatches(img_1, keypoints_1, img_2, keypoints_2, mathces, img_mathes);
	//// -- show 
	//cv::imshow("Mathces", img_mathes);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////SIFT//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//cv::Mat img_1 = cv::imread("Monster7.bmp");
	//cv::Mat img_2 = cv::imread("test6.png");

	//// -- Step 1: Detect the keypoints using STAR Detector 
	//std::vector<cv::KeyPoint> keypoints_1, keypoints_2;
	//cv::SiftFeatureDetector siftDetector(1000);
	//siftDetector.detect(img_1, keypoints_1);
	//siftDetector.detect(img_2, keypoints_2);

	//// -- Stpe 2: Calculate descriptors (feature vectors) 
	//cv::SiftDescriptorExtractor siftDescriptor;
	//cv::Mat descriptors_1, descriptors_2;
	//siftDescriptor.compute(img_1, keypoints_1, descriptors_1);
	//siftDescriptor.compute(img_2, keypoints_2, descriptors_2);

	////-- Step 3: Matching descriptor vectors with a brute force matcher 
	//cv::BFMatcher matcher(cv::NORM_L2);
	//std::vector<cv::DMatch> mathces;
	//std::vector<std::vector<cv::DMatch>> mathceskn;
	//matcher.knnMatch(descriptors_1, descriptors_2, mathceskn, 2);
	////matcher.match(descriptors_1, descriptors_2, mathces);

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
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////SURF////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//cv::Mat img_1 = cv::imread("Monster8.png");
	//cv::Mat img_2 = cv::imread("test7.png");

	//// -- Step 1: Detect the keypoints using STAR Detector 
	//std::vector<cv::KeyPoint> keypoints_1, keypoints_2;
	//cv::SURF surf(100);
	//surf.detect(img_1, keypoints_1);
	//surf.detect(img_2, keypoints_2);

	//// -- Stpe 2: Calculate descriptors (feature vectors) 
	//cv::Mat descriptors_1, descriptors_2;
	//surf.compute(img_1, keypoints_1, descriptors_1);
	//surf.compute(img_2, keypoints_2, descriptors_2);

	////-- Step 3: Matching descriptor vectors with a brute force matcher 
	//cv::BFMatcher matcher(cv::NORM_L2);
	//std::vector<cv::DMatch> mathces;
	//std::vector<std::vector<cv::DMatch>> mathceskn;
	//matcher.knnMatch(descriptors_1, descriptors_2, mathceskn, 2);
	////matcher.match(descriptors_1, descriptors_2, mathces);

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
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////轮廓匹配////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//cv::Mat srcImg = cv::imread("Monster7.png");  //模板图像
	//cv::imshow("src", srcImg);
	//cv::cvtColor(srcImg, srcImg, CV_BGR2GRAY);
	//cv::threshold(srcImg, srcImg, 100, 255, CV_THRESH_BINARY);
	//std::vector<std::vector<cv::Point>> contours;
	//std::vector<cv::Vec4i> hierarcy;
	//cv::findContours(srcImg, contours, hierarcy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	//cv::Mat srcImg2 = cv::imread("test6.png");  //待测试图片
	//cv::imshow("src2", srcImg2);
	//cv::Mat dstImg = srcImg2.clone();
	//cv::cvtColor(srcImg2, srcImg2, CV_BGR2GRAY);
	//cv::threshold(srcImg2, srcImg2, 100, 255, CV_THRESH_BINARY);
	//std::vector<std::vector<cv::Point>> contours2;
	//std::vector<cv::Vec4i> hierarcy2;
	//cv::findContours(srcImg2, contours2, hierarcy2, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	//while (1)
	//{
	//	for (int i = 0; i < contours2.size(); i++)
	//	{
	//		double matchRate = cv::matchShapes(contours[0], contours2[i], CV_CONTOURS_MATCH_I1, 0.0);//形状匹配:值越小越相似
	//		//cout << "index=" << i << "---" << setiosflags(ios::fixed) << matchRate << endl;//setiosflags(ios::fixed)是用定点方式表示实数，保留相同位数，相同格式输出
	//		if (matchRate <= 0.1)
	//			cv::drawContours(dstImg, contours2, i, cv::Scalar(0, 255, 0), 2, 8);
	//		cv::imshow("dst", dstImg);
	//		/*char key = waitKey();
	//		if (key == 27)
	//		break;*/
	//	}
	//	break;
	//}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////模板匹配/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*cv::Mat img_1 = cv::imread("test8.png");
	cv::Mat img_2 = cv::imread("Monster8.png");

	int resultImage_cols = img_1.cols - img_2.cols + 1;
	int resultImage_rows = img_1.rows - img_2.rows + 1;
	cv::Mat img_res;
	img_res.create(resultImage_cols, resultImage_rows, CV_32FC1);
	int iMatchMethod = 1;
	cv::matchTemplate(img_1, img_2, img_res, iMatchMethod);
	normalize(img_res, img_res, 0, 2, cv::NORM_MINMAX, -1, cv::Mat());
	double minValue, maxValue;
	cv::Point minLocation, maxLocation, matchLocation;
	minMaxLoc(img_res, &minValue, &maxValue, &minLocation, &maxLocation);

	if (iMatchMethod == cv::TM_SQDIFF || iMatchMethod == cv::TM_SQDIFF_NORMED)
	{
		matchLocation = minLocation;
	}
	else
	{
		matchLocation = maxLocation;
	}

	rectangle(img_1, matchLocation, cv::Point(matchLocation.x + img_2.cols, matchLocation.y + img_2.rows), cv::Scalar(0, 0, 255), 2, 8, 0);
	rectangle(img_res, matchLocation, cv::Point(matchLocation.x + img_2.cols, matchLocation.y + img_2.rows), cv::Scalar(0, 0, 255), 2, 8, 0);

	imshow("原始图", img_1);
	imshow("效果图", img_res);*/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

ST_POS StarsGraphy::FindPicture(const std::string& kPictureName, ST_RECT kRect)
{
	ST_POS kPoint;
	kPoint.x = -1; kPoint.y = -1;
	std::map<std::string, GamePictureInfo>::iterator itr = m_akPicture.find(kPictureName);
	if (itr == m_akPicture.end())
	{
		std::string kStr = "FindPicture图片未加载:";
		kStr += kPictureName;
		
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

ST_POS StarsGraphy::FIndPictureORB(const std::string& kPictureName/*, ST_RECT kRect*/)
{
	ST_POS kPoint;
	kPoint.x = -1; kPoint.y = -1;

	std::map<std::string, GameORBInfo>::iterator itr = m_akORBInfo.find(kPictureName);
	if (itr == m_akORBInfo.end())
	{
		std::string kStr = "FIndPictureORB图片未加载:";
		kStr += kPictureName;
		//MessageBoxA(NULL, kStr.c_str(), "Warning", MB_OK);
		return kPoint;
	}

	kPoint = FIndPictureORB(itr->second, kPictureName);
	if (kPoint.x != -1)
	{
		return kPoint;
	}

	std::map<std::string, GameORBInfo>::iterator itrFlip = m_akORBInfoFlip.find(kPictureName);
	if (itrFlip == m_akORBInfoFlip.end())
	{
		std::string kStr = "FIndPictureORB图片未加载:";
		kStr += kPictureName;
		//MessageBoxA(NULL, kStr.c_str(), "Warning", MB_OK);
		return kPoint;
	}

	kPoint = FIndPictureORB(itrFlip->second, kPictureName);

	return kPoint;
}

ST_POS StarsGraphy::FIndPictureORB(GameORBInfo& kGameORBInfo, const std::string& kPictureName)
{
	ST_POS kPoint;
	kPoint.x = -1; kPoint.y = -1;

	//cv::Rect r1(kRect.left, kRect.top, kRect.right - kRect.left, kRect.bottom - kRect.top);
	//cv::Mat mask = cv::Mat::zeros(m_kScreenORBInfo.img->size(), CV_8UC1);
	//mask(r1).setTo(255);

	std::vector<cv::DMatch> mathces;
	std::vector<std::vector<cv::DMatch>> mathceskn;
	m_pkMatcher->knnMatch(*(kGameORBInfo.descriptors), *(m_kScreenORBInfo.descriptors), mathceskn, 2);

	for (int i = 0; i < (int)mathceskn.size(); i++)
	{
		cv::DMatch bestMatch = mathceskn[i][0];
		cv::DMatch betterMatch = mathceskn[i][1];
		cv::Point p1 = m_kScreenORBInfo.keypoints[bestMatch.trainIdx].pt;
		cv::Point p2 = m_kScreenORBInfo.keypoints[betterMatch.trainIdx].pt;
		float distance = sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
		float distanceRatio = bestMatch.distance / betterMatch.distance;

		if (distanceRatio < 0.1/* && distance < 50*/)
		{
			mathces.push_back(bestMatch);
		}
	}

	////// -- dwaw matches 
	cv::Mat img_mathes;
	drawMatches(*(kGameORBInfo.img), kGameORBInfo.keypoints, *(m_kScreenORBInfo.img), m_kScreenORBInfo.keypoints, mathces, img_mathes);
	// -- show 
	cv::imshow("Mathces", img_mathes);

	//char str[100];
	//std::string kName = kPictureName.substr(0, kPictureName.size() - 4);
	//sprintf_s(str, "com%d_%s.jpg", timeGetTime(), kName.c_str());
	//cv::imwrite(str, img_mathes);

	if (mathces.size() < 2) return kPoint;

	for (int i = 0; i < mathces.size(); ++i)
	{
		kPoint.x += m_kScreenORBInfo.keypoints[mathces[i].trainIdx].pt.x;
		kPoint.y += m_kScreenORBInfo.keypoints[mathces[i].trainIdx].pt.y;
	}

	kPoint.x /= mathces.size();
	kPoint.y /= mathces.size();

	

	return kPoint;
}

ST_POS StarsGraphy::FindFont(const std::string& kStr, ST_RECT kRect)
{
	return ST_POS(-1, -1);
}

ST_POS StarsGraphy::FindColor(DWORD dwColor, ST_RECT kRect, bool bFindColorBlock, ST_POS kStartPos)
{
	ST_POS kPoint;
	kPoint.x = -1; kPoint.y = -1;

	CheckRect(kRect, 0, 0);
	kPoint = ComPareColorNormal(kRect.left, kRect.right, kRect.top, kRect.bottom, dwColor, bFindColorBlock, kStartPos);

	return kPoint;
}

DWORD StarsGraphy::GetColor(ST_POS kPos)
{
	if (kPos.x >= 0 && kPos.x < iScreenShotWidth && kPos.y >= 0 && kPos.y < iScreenShotHeight)
	{
		return m_pkScreenShotData[kPos.y * iScreenShotWidth + kPos.x];
	}
	else
	{
		return 0;
	}
}

void StarsGraphy::SaveSpecialRandPoint()
{
	const int aiDir[8][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 }, { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 } };
	srand(timeGetTime());
	for (int i = 0; i < SPECIAL_POS_NUM; ++i)
	{
		bool bFind = false;
		while (!bFind)
		{
			int iPosX = rand() % iScreenShotWidth;
			if (iPosX <= 0) iPosX = 2; if (iPosX >= iScreenShotWidth - 1) iPosX = iScreenShotWidth - 3;
			int iPosY = rand() % iScreenShotHeight;
			if (iPosY <= 0) iPosY = 2; if (iPosY >= iScreenShotHeight - 1) iPosY = iScreenShotHeight - 3;

			int iCurColor = m_pkScreenShotData[iPosY * iScreenShotWidth + iPosX];
			int iColorR = COLOR_R(iCurColor);
			int iColorG = COLOR_G(iCurColor);
			int iColorB = COLOR_B(iCurColor);

			int iColorNoEqualCount = 0;
			int iTempColor;
			for (int j = 0; j < 8; ++j)
			{
				iTempColor = m_pkScreenShotData[(iPosY + aiDir[j][1]) * iScreenShotWidth + (iPosX + aiDir[j][0])];
				if (iTempColor != iCurColor) iColorNoEqualCount++;
				iColorR += COLOR_R(iTempColor);
				iColorG += COLOR_G(iTempColor);
				iColorB += COLOR_B(iTempColor);
			}

			if (iColorNoEqualCount >= 4)
			{
				bFind = true;
				m_akSpecialPos[i] = ST_POS(iPosX, iPosY);
				m_aiSpecialPosColor[i] = (iColorR / 9) << 16 + (iColorG / 9) << 8 + iColorB / 9;
			}
		}
	}
}

ST_POS StarsGraphy::GetSpecialPointOff()
{

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
			
			GameORBInfo& kGameORBInfoFlip = m_akORBInfoFlip[akPictureName[i]];
			kGameORBInfoFlip.aiPixelData = new unsigned char[bitMapIcon.bmHeight * bitMapIcon.bmWidth * 4];

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
					memcpy(&(kGameORBInfoFlip.aiPixelData[(bitMapIcon.bmWidth * i + bitMapIcon.bmWidth - j) * 4]), &color, 4);
				}
			}
			DeleteDC(hdcIcon);
			::DeleteObject(hBitMapIcon);
			
			kGameORBInfo.img = new cv::Mat(bitMapIcon.bmHeight, bitMapIcon.bmWidth, CV_8UC4, kGameORBInfo.aiPixelData);
			kGameORBInfo.descriptors = new cv::Mat();
			m_pkORBTool->detect(*(kGameORBInfo.img), kGameORBInfo.keypoints);
			m_pkORBTool->compute(*(kGameORBInfo.img), kGameORBInfo.keypoints, *(kGameORBInfo.descriptors));

			kGameORBInfoFlip.img = new cv::Mat(bitMapIcon.bmHeight, bitMapIcon.bmWidth, CV_8UC4, kGameORBInfoFlip.aiPixelData);
			kGameORBInfoFlip.descriptors = new cv::Mat();
			m_pkORBTool->detect(*(kGameORBInfoFlip.img), kGameORBInfoFlip.keypoints);
			m_pkORBTool->compute(*(kGameORBInfoFlip.img), kGameORBInfoFlip.keypoints, *(kGameORBInfoFlip.descriptors));
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
	kGamePictureInfo.fComPareRate = 0;
	memset(m_aiVisitPoint, 0, iScreenShotWidth * iScreenShotHeight);
	m_iQueueIndex = 0;
	m_iQueueNum = 0;
	ST_POS kStarPoint((iBeginX + iEndX) / 2, (iBeginY + iEndY) / 2);
	m_akQueue[m_iQueueNum] = kStarPoint;
	m_iQueueNum++;
	int iMaxIndex = iScreenShotWidth * iScreenShotHeight - 1;
	const int aiDir[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	ST_POS kTempPoint;
	int iTempIndex;
	while (m_iQueueIndex < m_iQueueNum)
	{
		ST_POS &kCurPoint = m_akQueue[m_iQueueIndex];
		m_iQueueIndex++;
		for (int k = 0; k < 4; ++k)
		{
			kTempPoint.x = aiDir[k][0] + kCurPoint.x;
			kTempPoint.y = aiDir[k][1] + kCurPoint.y;
			iTempIndex = kTempPoint.x + kTempPoint.y * iScreenShotWidth;
			if (!m_aiVisitPoint[iTempIndex] && kTempPoint.x >= iBeginX && kTempPoint.x <= iEndX && kTempPoint.y >= iBeginY && kTempPoint.y <= iEndY)
			{
				m_akQueue[m_iQueueNum] = kTempPoint;
				m_iQueueNum++;
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

				if (kGamePictureInfo.aiPixelData[iTempIndex] == m_pkScreenShotData[(kCurPoint.y + k) * iScreenShotWidth + (kCurPoint.x + l)])
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
			kGamePictureInfo.iComPareBeginX = kCurPoint.x;
			kGamePictureInfo.iComPareBeginY = kCurPoint.y;
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

ST_POS StarsGraphy::ComPareColorNormal(int iBeginX, int iEndX, int iBeginY, int iEndY, DWORD dwColor, bool bFindColorBlock, ST_POS kStartPos)
{
	memset(m_aiVisitPoint, 0, iScreenShotWidth * iScreenShotHeight);
	m_iQueueIndex = 0;
	m_iQueueNum = 0;
	
	if (kStartPos.x == -1 && kStartPos.y == -1)
	{
		m_akQueue[m_iQueueNum] = ST_POS((iBeginX + iEndX) / 2, (iBeginY + iEndY) / 2);
	}
	else
	{
		m_akQueue[m_iQueueNum] = kStartPos;
	}

	m_iQueueNum++;
	int iMaxIndex = iScreenShotWidth * iScreenShotHeight - 1;
	const int aiDir[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };
	ST_POS kTempPoint;
	int iTempIndex;
	int iCurIndex;
	while (m_iQueueIndex < m_iQueueNum)
	{
		ST_POS &kCurPoint = m_akQueue[m_iQueueIndex];
		iCurIndex = (kCurPoint.y) * iScreenShotWidth + (kCurPoint.x);
		m_iQueueIndex++;
		for (int k = 0; k < 4; ++k)
		{
			kTempPoint.x = aiDir[k][0] + kCurPoint.x;
			kTempPoint.y = aiDir[k][1] + kCurPoint.y;
			iTempIndex = kTempPoint.x + kTempPoint.y * iScreenShotWidth;
			if (!m_aiVisitPoint[iTempIndex] && kTempPoint.x >= iBeginX && kTempPoint.x <= iEndX && kTempPoint.y >= iBeginY && kTempPoint.y <= iEndY)
			{
				m_akQueue[m_iQueueNum] = kTempPoint;
				m_iQueueNum++;
				m_aiVisitPoint[iTempIndex] = true;
			}
		}

		if (dwColor == m_pkScreenShotData[iCurIndex])
		{
			// 不需要找色块直接返回
			if (!bFindColorBlock)
			{
				return ST_POS(kCurPoint.x, kCurPoint.y);
			}
			else
			{
				int iTempBeginX = kCurPoint.x;
				while (iTempBeginX > iBeginX && m_pkScreenShotData[kCurPoint.y * iScreenShotWidth + iTempBeginX - 1] == dwColor)
				{
					iTempBeginX--;
				}
				int iTempEndX = kCurPoint.x;
				while (iTempEndX < iEndX && m_pkScreenShotData[kCurPoint.y * iScreenShotWidth + iTempEndX + 1] == dwColor)
				{
					iTempEndX++;
				}
				int iTempBeginY = kCurPoint.y;
				while (iTempBeginY > iBeginY && m_pkScreenShotData[(iTempBeginY - 1) * iScreenShotWidth + kCurPoint.x] == dwColor)
				{
					iTempBeginY--;
				}
				int iTempEndY = kCurPoint.y;
				while (iTempEndY < iEndY && m_pkScreenShotData[(iTempEndY + 1) * iScreenShotWidth + kCurPoint.x] == dwColor)
				{
					iTempEndY++;
				}

				int iDifX = iTempEndX - iTempBeginX;
				int iDifY = iTempEndY - iTempBeginY;
				if (iDifX* iDifY >= 4)
				{
					return ST_POS(iTempBeginX + iDifX / 2, iTempBeginY + iDifY * 0.8);
				}
			}

		}
	}

	return ST_POS(-1, -1);
}

void StarsGraphy::CheckRect(ST_RECT& kRect, int iWidth, int iHeight)
{
	if (kRect.left < 0) kRect.left = 0;
	if (kRect.right > iScreenShotWidth - iWidth) kRect.right = iScreenShotWidth - iWidth;
	if (kRect.top < 0) kRect.top = 0;
	if (kRect.bottom > iScreenShotHeight - iHeight) kRect.bottom = iScreenShotHeight - iHeight;
}