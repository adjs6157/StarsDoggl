#include "ScreenShotDXGI.h"
#include "ScreenShotDDRAW.h"
#include "StarsUtility.h"
#include <string>
#include <map>
#include <vector>
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp> 
#include <opencv2/features2d/features2d.hpp>

#define SPECIAL_POS_NUM 5

struct GamePictureInfo
{
	GamePictureInfo()
	{
		iComPareBeginX = -1;
		iComPareBeginY = -1;
		fComPareRate = 0;
		iPixelWidth = 0;
		iPixelHeight = 0;
		aiPixelData = NULL;
		iPixelCount = 0;
	}
	~GamePictureInfo()
	{
		if (aiPixelData != NULL)
		{
			delete aiPixelData;
		}
	}

	int iComPareBeginX;
	int iComPareBeginY;
	float fComPareRate;
	int iPixelWidth;
	int iPixelHeight;
	int iPixelCount;
	DWORD* aiPixelData;
};

struct GameORBInfo
{
	GameORBInfo()
	{
		img = nullptr;
		keypoints.clear();
		descriptors = nullptr;
		aiPixelData = nullptr;
	}

	~GameORBInfo()
	{
		if (img != nullptr)
		{
			delete img;
		}
		if (descriptors != nullptr)
		{
			delete descriptors;
		}
		/*if (aiPixelData != nullptr)
		{
			delete[] aiPixelData;
		}*/
	}
	cv::Mat* img;
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat* descriptors;
	unsigned char* aiPixelData;
};

class StarsGraphy
{
public:
	StarsGraphy();
	~StarsGraphy();
	bool Initalize();
	bool Finitalize();
	void Update(const ST_RECT& kGameRect);

	ST_POS FindPicture(const std::string& kPictureName, ST_RECT kRect);
	ST_POS FIndPictureORB(const std::string& kPictureName);
	ST_POS FIndPictureORB(GameORBInfo& kGameORBInfo, const std::string& kPictureName);
	ST_POS FindFont(const std::string& kStr, ST_RECT kRect);
	ST_POS FindColor(DWORD dwColor, ST_RECT kRect, bool bFindColorBlock = true, ST_POS kStartPos = ST_POS(-1, -1));
	DWORD GetColor(ST_POS kPos);
	void SaveSpecialRandPoint();
	ST_POS GetSpecialPointOff();
	
private:
	void CheckRect(ST_RECT& kRect);
	void RotateImg(DWORD *pImgData);
	void SaveBmpFile(const char *fileName, DWORD *pImgData, int iWidth, int iHeight);
	void LoadLocalPicture();
	void LoadFont();
	void GetFiles(std::string path, std::vector<std::string>& filePaths, std::vector<std::string>& fileNames);
	void ComPareImageNormal(int iBeginX, int iEndX, int iBeginY, int iEndY, GamePictureInfo& akGamePictureInfo);
	ST_POS ComPareColorNormal(int iBeginX, int iEndX, int iBeginY, int iEndY, DWORD dwColor, bool bFindColorBlock = true, ST_POS kStartPos = ST_POS(-1, -1));
	void CheckRect(ST_RECT& kRect, int iWidth, int iHeight);
private:
	ScreenShotDDRAW* m_pkScreenShotDDRAW;
	int m_iLastUpdateTime;
	int m_iLastUpdateTimeORB;
	DWORD *m_pkScreenShotData;
	DWORD *m_pkRotateImg32;
	int	m_iImgDataSize;
	std::map<std::string, GamePictureInfo> m_akPicture;

	std::map<std::string, GameORBInfo> m_akORBInfo;
	std::map<std::string, GameORBInfo> m_akORBInfoFlip;
	GameORBInfo	m_kScreenORBInfo;
	cv::ORB* m_pkORBTool;
	cv::BFMatcher* m_pkMatcher;
	bool*	m_aiVisitPoint;
	ST_POS* m_akQueue;
	int	m_iQueueIndex;
	int m_iQueueNum;
	ST_POS m_akSpecialPos[SPECIAL_POS_NUM];
	DWORD m_aiSpecialPosColor[SPECIAL_POS_NUM];
};
