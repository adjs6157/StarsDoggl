#include "ScreenShotDXGI.h"
#include "ScreenShotDDRAW.h"
#include <string>
#include <map>
#include <vector>

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
	DWORD* aiPixelData;
};


class StarsGraphy
{
public:
	StarsGraphy();
	~StarsGraphy();
	bool Initalize();
	bool Finitalize();
	void Update();

	POINT FindPicture(std::string kPictureName, RECT kRect);
	
private:
	void RotateImg(DWORD *pImgData);
	void SaveBmpFile(const char *fileName, unsigned char *pImgData, int imgLength);
	void LoadLocalPicture();
	void GetFiles(std::string path, std::vector<std::string>& filePaths, std::vector<std::string>& fileNames);
	void ComPareImageNormal(int iBeginX, int iEndX, int iBeginY, int iEndY, GamePictureInfo& akGamePictureInfo);

private:
	ScreenShotDDRAW* m_pkScreenShotDDRAW;
	int m_iLastUpdateTime;
	DWORD *m_pkScreenShotData;
	DWORD *m_pkRotateImg32;
	int	m_iImgDataSize;
	std::map<std::string, GamePictureInfo> m_akPicture;
};
