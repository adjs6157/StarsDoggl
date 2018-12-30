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
	
private:
	void CheckRect(ST_RECT& kRect);
	void RotateImg(DWORD *pImgData);
	void SaveBmpFile(const char *fileName, DWORD *pImgData, int iWidth, int iHeight);
	void LoadLocalPicture();
	void LoadFont();
	void GetFiles(std::string path, std::vector<std::string>& filePaths, std::vector<std::string>& fileNames);
	void ComPareImageNormal(int iBeginX, int iEndX, int iBeginY, int iEndY, GamePictureInfo& akGamePictureInfo);
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
};
