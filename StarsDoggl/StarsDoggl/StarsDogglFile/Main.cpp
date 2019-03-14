#include "lz4.h"
#include "zlib.h"
#include <string>
#include <io.h>
#include <direct.h>
#include <vector>
#include <map>
#include "sha256.h"

enum compressType{ COMP_NONE = 5, COMP_ZLIB = 6, COMP_ZLIB2 = 7, COMP_UDEF = 0 };
enum colorFormat{ ARGB8888 = 0x10, ARGB4444 = 0x0F, ARGB1555 = 0x0E, LINK = 0x11, DDS_DXT1 = 0x12, DDS_DXT3 = 0x13, DDS_DXT5 = 0x14, COLOR_UDEF = 0, V4_FMT, RGB565 };
enum starsImgType{ SIMG_NONE = 0, SIMG_MONSTER = 1, SIMG_OBJECT = 2, SIMG_BLOCK = 3, SIMG_PATHGATE = 4, SIMG_BACKGROUND = 5, SIMG_BOSS = 6};
enum starsImgColor{ 
	SCOLOR_NONE = 0, 
	SCOLOR_MONSTER = 0xFFFF00FF, 
	SCOLOR_BOSS = 0xFFFF00C8,
	SCOLOR_OBJECT = 0xFFFFFF00, 
	SCOLOR_BLOCK = 0xFFFF0000, 
	SCOLOR_PATHGATE = 0xFF00FF00,
	SCOLOR_PATHGATE_UP = 0xFF00EE00,
	SCOLOR_PATHGATE_DOWN = 0xFF00DD00,
	SCOLOR_ITEM = 0xFFFF8000, 
	SCOLOR_MINIMAP = 0xFF0000EE, 
	SCOLOR_MINIMAP_OPEN = 0xFF0000DD, 
	SCOLOR_MINIMAP_UNKONW = 0xFF0000AA, 
	SCOLOR_MINIMAP_UNKONW_OPEN = 0xFF000099, 
	SCOLOR_MINIMAP_DOOR = 0xFF000022,
	SCOLOR_MONSTERICON = 0xFFC8C800
};


#define GAME_IMG_PATH "./NPKBack/"

struct NPK_Header
{
	char flag[16]; // �ļ���ʶ "NeoplePack_Bill"
	int count;     // �����ļ�����Ŀ
};

struct NPK_Index
{
	unsigned int offset;  // �ļ��İ���ƫ����
	unsigned int size;    // �ļ��Ĵ�С
	char name[256];// �ļ���
};

char decord_flag[256] = "puchikon@neople dungeon and fighter DNF";

struct NImgF_Header
{
	char flag[20]; // �ļ���ʯ"Neople Img File"
	int index_size;	// �������С�����ֽ�Ϊ��λ
	int unknown1;
	int version;
	int index_count;// ��������Ŀ
	int iDDSCount;
	int iTotalLength;
	std::vector<std::vector<unsigned int>> aiPaletteData;
};

struct NImgF_Index
{
	unsigned int dwType; //Ŀǰ��֪�������� 0x0E(1555��ʽ) 0x0F(4444��ʽ) 0x10(8888��ʽ) 0x11(�������κ����ݣ�������ָ����ͬ��һ֡)
	unsigned int dwCompress; // Ŀǰ��֪�������� 0x06(zlibѹ��) 0x05(δѹ��)
	int width;        // ���
	int height;       // �߶�
	int size;         // ѹ��ʱsizeΪѹ�����С��δѹ��ʱsizeΪת����8888��ʽʱռ�õ��ڴ��С
	int key_x;        // X�ؼ��㣬��ǰͼƬ����ͼ�е�X����
	int key_y;        // Y�ؼ��㣬��ǰͼƬ����ͼ�е�Y����
	int max_width;    // ��ͼ�Ŀ��
	int max_height;   // ��ͼ�ĸ߶ȣ��д�������Ϊ�˶��뾫��
	int iLinkNum;
	void* pkData;
};

bool ReadNPKFile(std::string path, NPK_Header& kNpkHeader, std::vector<NPK_Index>& akNpkIndex, unsigned char* dataSHA, std::vector<NImgF_Header>& akImgHeader, std::vector<std::vector<NImgF_Index>>& akImgIndex)
{
	FILE *fp = fopen(path.c_str(), "rb");
	if (!fp)
	{
		return false;
	}

	fread(&kNpkHeader, sizeof(NPK_Header), 1, fp);

	akNpkIndex.resize(kNpkHeader.count);
	//akNpkIndex.reserve(kNpkHeader.count);
	for (int i = 0; i < kNpkHeader.count; ++i)
	{
		fread(&(akNpkIndex[i]), sizeof(NPK_Index), 1, fp);
	}

	fread(dataSHA, 32, 1, fp);

	akImgHeader.resize(kNpkHeader.count);
	akImgIndex.resize(kNpkHeader.count);
	for (int i = 0; i < kNpkHeader.count; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			akNpkIndex[i].name[j] ^= decord_flag[j];
		}

		fseek(fp, akNpkIndex[i].offset, SEEK_SET);

		// img file
		fread(akImgHeader[i].flag, 20, 1, fp);
		if (strcmp(akImgHeader[i].flag, "Neople Img File") == 0)
		{
			fseek(fp, -4, SEEK_CUR);
			fread(&(akImgHeader[i].index_size), sizeof(int), 1, fp);
		}
		else if (strcmp(akImgHeader[i].flag, "Neople Image File") == 0)
		{
			akImgHeader[i].index_size = -1;
		}
		else
		{
			printf("ERROR:imgHeader[i].flag\n");
			return false;
		}
		fread(&(akImgHeader[i].unknown1), sizeof(int), 1, fp);
		fread(&(akImgHeader[i].version), sizeof(int), 1, fp);
		if (akImgHeader[i].version != 2 && akImgHeader[i].version != 4 && akImgHeader[i].version != 5 && akImgHeader[i].version != 6)
		{
			printf("ERROR:imgHeader[i].version\n");
			return false;
		}
		fread(&(akImgHeader[i].index_count), sizeof(int), 1, fp);
		if (akImgHeader[i].version == 5)
		{
			fread(&(akImgHeader[i].iDDSCount), sizeof(int), 1, fp);
			fread(&(akImgHeader[i].iTotalLength), sizeof(int), 1, fp);
		}
		if (akImgHeader[i].version == 4 || akImgHeader[i].version == 5)
		{
			int iColorCount;
			fread(&iColorCount, sizeof(int), 1, fp);
			std::vector<unsigned int> aiColor;
			for (int j = 0; j < iColorCount; ++j)
			{
				unsigned int iColor;
				fread(&iColor, sizeof(int), 1, fp);
				aiColor.push_back(iColor);
			}
			akImgHeader[i].aiPaletteData.push_back(aiColor);
		}
		if (akImgHeader[i].version == 6)
		{
			int iPaletteCount;
			fread(&iPaletteCount, sizeof(int), 1, fp);
			for (int j = 0; j < iPaletteCount; ++j)
			{
				int iColorCount;
				std::vector<unsigned int> aiColor;
				fread(&iColorCount, sizeof(int), 1, fp);
				for (int k = 0; k < iColorCount; ++k)
				{
					unsigned int iColor;
					fread(&iColor, sizeof(int), 1, fp);
					aiColor.push_back(iColor);
				}
				akImgHeader[i].aiPaletteData.push_back(aiColor);
			}
		}
		if (akImgHeader[i].version == 5)
		{
			unsigned int iDDS;
			for (int j = 0; j < akImgHeader[i].iDDSCount; ++j)
			{
				fread(&iDDS, sizeof(int), 1, fp);
				fread(&iDDS, sizeof(int), 1, fp);
				fread(&iDDS, sizeof(int), 1, fp);
				fread(&iDDS, sizeof(int), 1, fp);
				fread(&iDDS, sizeof(int), 1, fp);
				fread(&iDDS, sizeof(int), 1, fp);
				fread(&iDDS, sizeof(int), 1, fp);
			}
		}
		if (akImgHeader[i].version != 1)
		{
			akImgIndex[i].resize(akImgHeader[i].index_count);
			for (int j = 0; j < akImgHeader[i].index_count; ++j)
			{
				fread(&(akImgIndex[i][j].dwType), sizeof(int), 1, fp);
				if (akImgIndex[i][j].dwType == LINK)
				{
					fread(&(akImgIndex[i][j].iLinkNum), sizeof(int), 1, fp);
				}
				else if (akImgIndex[i][j].dwType < LINK)
				{
					fread(&(akImgIndex[i][j].dwCompress), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].width), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].height), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].size), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].key_x), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].key_y), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].max_width), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].max_height), sizeof(int), 1, fp);
				}
				else
				{
					fread(&(akImgIndex[i][j].dwCompress), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].width), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].height), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].size), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].key_x), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].key_y), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].max_width), sizeof(int), 1, fp);
					fread(&(akImgIndex[i][j].max_height), sizeof(int), 1, fp);
					unsigned int iData;
					fread(&iData, sizeof(int), 1, fp);
					fread(&iData, sizeof(int), 1, fp);
					fread(&iData, sizeof(int), 1, fp);
					fread(&iData, sizeof(int), 1, fp);
					fread(&iData, sizeof(int), 1, fp);
					fread(&iData, sizeof(int), 1, fp);
					fread(&iData, sizeof(int), 1, fp);
				}
				//akImgIndex[i][j].pkData = malloc(akImgIndex[i][j].size);
			}
			/*for (int j = 0; j < akImgHeader[i].index_count; ++j)
			{
				if (akImgIndex[i][j].dwType != LINK)
				{
					fread(akImgIndex[i][j].pkData, akImgIndex[i][j].size, 1, fp);
				}
			}*/
		}

		///////////////
	}
	fclose(fp);
	return true;
}

void SplitStr(const std::string& rkStr, const std::string& rkSep, std::vector<std::string>& rkResult)
{
	if (rkStr.empty())
	{
		return;
	}

	rkResult.clear();

	size_t uiStart = 0;
	size_t uiPos = 0;
	do
	{
		uiPos = rkStr.find_first_of(rkSep, uiStart);
		if (uiPos == uiStart)
		{
			uiStart = uiPos + 1;
		}
		else if (uiPos == std::string::npos)
		{
			rkResult.push_back(rkStr.substr(uiStart));
			break;
		}
		else
		{
			rkResult.push_back(rkStr.substr(uiStart, uiPos - uiStart));
			uiStart = uiPos + 1;
		}
		uiStart = rkStr.find_first_not_of(rkSep, uiStart);
	} while (uiPos != std::string::npos);
}

bool CheckNPKNameType(const std::string kPictureName, starsImgType eType)
{
	std::string kPictureNameTemp = kPictureName.substr(0, kPictureName.size() - 4);
	std::string kFilePreName = "";
	std::string kFileEndName = "";
	std::string kFileEndName1 = "";
	switch (eType)
	{
	case SIMG_NONE:
		return true;
		break;
	case SIMG_MONSTER:
		kFilePreName = "sprite_monster";
		break;
	case SIMG_OBJECT:
	case SIMG_BLOCK:
		kFilePreName = "sprite_map";
		kFileEndName = "obj";
		kFileEndName1 = "object";
		break;
	case SIMG_PATHGATE:
		kFilePreName = "sprite_map";
		kFileEndName = "gate";
		kFileEndName1 = "pathgate";
		break;
	case SIMG_BACKGROUND:
		kFileEndName = "background";
		break;
	default:
		break;
	}

	if (kPictureNameTemp[0] != 's') return false;
	if (kPictureNameTemp.find(kFilePreName) != 0) return false;
	if (kFileEndName.length() == 0 && kFileEndName1.length() == 0) return true;
	if (kFileEndName.length() != 0 && kPictureNameTemp.find(kFileEndName) == kPictureNameTemp.size() - kFileEndName.size()) return true;
	if (kFileEndName1.length() != 0 && kPictureNameTemp.find(kFileEndName1) == kPictureNameTemp.size() - kFileEndName1.size()) return true;

	return false;
}

void GetFiles(std::string path, std::vector<std::string>& filePaths, std::vector<std::string>& fileNames)
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

void OutPutAutoConfig(starsImgType eType)
{
	std::map<std::string, std::map<std::string, int>> akImgType;

	NPK_Header kNpkHeader;
	std::vector<NPK_Index> akNpkIndex;
	unsigned char dataSHA[32];
	std::vector<NImgF_Header> akImgHeader;
	std::vector<std::vector<NImgF_Index>> akImgIndex;

	std::vector<std::string> akPicturePath;
	std::vector<std::string> akPictureName;
	GetFiles(GAME_IMG_PATH, akPicturePath, akPictureName);

	for (int i = 0; i < akPictureName.size(); ++i)
	{
		printf("%d/%d\n", i, akPictureName.size());
		std::string& kPictureName = akPictureName[i];
		//if (SIMG_NONE != eType)
		{
			if (!CheckNPKNameType(kPictureName, eType)) continue;
		}
		

		akNpkIndex.clear();
		akImgHeader.clear();
		akImgIndex.clear();
		ReadNPKFile(akPicturePath[i], kNpkHeader, akNpkIndex, dataSHA, akImgHeader, akImgIndex);

		std::string kAutoConfitPath = "./FileConfig/Auto/";
		if (SIMG_NONE == eType) kAutoConfitPath = "./FileConfig/Temp/";
		kAutoConfitPath += kPictureName.substr(0, kPictureName.size() - 4);
		kAutoConfitPath += ".txt";
		FILE *fp = fopen(kAutoConfitPath.c_str(), "w+");
		if (!fp)
		{
			printf("fopen->kAutoConfitPath Faild!");
			return;
		}

		for (int j = 0; j < akNpkIndex.size(); ++j)
		{
			bool fPass = false;
			std::string kImgName = akNpkIndex[j].name;
			if (eType == SIMG_MONSTER)
			{
				kImgName  += "|SIMG_MONSTER\n";
				fputs(kImgName.c_str(), fp);
				fPass = true;
			}
			else if (eType == SIMG_OBJECT || eType == SIMG_BLOCK)
			{
				if (j + 1 < akNpkIndex.size())
				{
					std::string kImgNameTemp = kImgName;
					kImgNameTemp.insert(kImgNameTemp.size() - 4, "_particle");
					std::string kImgNameNext = akNpkIndex[j + 1].name;
					if (kImgNameTemp == kImgNameNext)
					{
						kImgName += "|SIMG_OBJECT\n";
						fputs(kImgName.c_str(), fp);
						fPass = true;
					}
				}
			}
			else if (eType == SIMG_PATHGATE)
			{
				kImgName += "|SIMG_PATHGATE\n";
				fputs(kImgName.c_str(), fp);
				fPass = true;
			}
			else if (eType == SIMG_BACKGROUND)
			{
				kImgName += "|SIMG_BACKGROUND\n";
				fputs(kImgName.c_str(), fp);
				fPass = true;
			}
			
			if (!fPass)
			{
				kImgName += "|SIMG_NONE\n";
				fputs(kImgName.c_str(), fp);
			}
		}
		fclose(fp);
	}
}

void OutPutCombineConfig()
{
	char kStr[256];
	std::vector<std::string> akSplitStr;

	std::map<std::string, std::map<std::string, std::string>> akImgComfig;
	std::vector<std::string> akPicturePathAuto;
	std::vector<std::string> akPictureNameAuto;
	GetFiles("./FileConfig/Auto/", akPicturePathAuto, akPictureNameAuto);
	for (int i = 0; i < akPicturePathAuto.size(); ++i)
	{
		FILE *fp = fopen(akPicturePathAuto[i].c_str(), "r+");
		if (!fp)
		{
			printf("fopen->akPicturePathAuto Faild!");
			return;
		}
		while (fgets(kStr, 256, fp) != NULL)
		{
			SplitStr(kStr, "|", akSplitStr);
			if (akSplitStr.size() == 2)
			{
				akSplitStr[1].erase(akSplitStr[1].size() - 1);
				akImgComfig[akPictureNameAuto[i]][akSplitStr[0]] = akSplitStr[1];
			}
		}
		fclose(fp);
	}
	printf("%d/%d\n", 1, 3);
	std::vector<std::string> akPicturePathManual;
	std::vector<std::string> akPictureNameManual;
	GetFiles("./FileConfig/Manual/", akPicturePathManual, akPictureNameManual);
	for (int i = 0; i < akPicturePathManual.size(); ++i)
	{
		FILE *fp = fopen(akPicturePathManual[i].c_str(), "r+");
		if (!fp)
		{
			printf("fopen->akPicturePathManual Faild!");
			return;
		}
		while (fgets(kStr, 256, fp) != NULL)
		{
			SplitStr(kStr, "|", akSplitStr);
			if (akSplitStr.size() == 2)
			{
				akSplitStr[1].erase(akSplitStr[1].size() - 1);
				akImgComfig[akPictureNameManual[i]][akSplitStr[0]] = akSplitStr[1];
			}
		}
		fclose(fp);
	}
	printf("%d/%d\n", 2, 3);
	std::map<std::string, std::map<std::string, std::string>>::iterator itr = akImgComfig.begin();
	for (; itr != akImgComfig.end(); itr++)
	{
		std::string kFileName = "./FileConfig/Combine/";
		kFileName += itr->first;
		FILE *fp = fopen(kFileName.c_str(), "w+");
		if (!fp)
		{
			printf("fopen->Combine Faild!");
			return;
		}

		std::map<std::string, std::string>::iterator itrSub = itr->second.begin();
		for (; itrSub != itr->second.end(); itrSub++)
		{
			std::string kResult = itrSub->first;
			kResult += "|";
			kResult += itrSub->second;
			kResult += "\n";
			fputs(kResult.c_str(), fp);
		}

		fclose(fp);
	}
	printf("%d/%d\n", 3, 3);
}

void ExportSpecialNPKFileMiniMapBack()
{
	char kStr[256];
	std::vector<std::string> akSplitStr;

	NPK_Header kNpkHeader;
	std::vector<NPK_Index> akNpkIndex;
	unsigned char dataSHA[32];
	std::vector<NImgF_Header> akImgHeader;
	std::vector<std::vector<NImgF_Index>> akImgIndex;

	unsigned int iColorTemp[3500 * 3500];
	int iCurNpkCount = 0;

	printf("ExportSpecialNPKFiles\n");
	std::string kName = GAME_IMG_PATH;
	kName += "sprite_map_minimap.NPK";

	akNpkIndex.clear();
	akImgHeader.clear();
	akImgIndex.clear();
	ReadNPKFile(kName, kNpkHeader, akNpkIndex, dataSHA, akImgHeader, akImgIndex);

	kName = "./FileExport/";
	kName += "a";
	kName += "sprite_map_minimap";
	kName += ".NPK";

	FILE *fpMy = fopen(kName.c_str(), "w+");
	if (!fpMy)
	{
		printf("FileExport Faild!");
		return;
	}
	fclose(fpMy);

	fpMy = fopen(kName.c_str(), "wb");
	if (!fpMy)
	{
		printf("FileExport Faild!");
		return;
	}
	// �����ļ���С
	std::vector<std::string> akNameUncompress;
	int iSizeOffset = 0;
	iSizeOffset += sizeof(NPK_Header)+sizeof(NPK_Index)* kNpkHeader.count + 32;
	for (int i = 0; i < kNpkHeader.count; ++i)
	{
		int iSizeCount = 0;
		iSizeCount += 4 * 8;
		akImgHeader[i].index_size = 0;
		for (int j = 0; j < akImgHeader[i].index_count; ++j)
		{
			if (j % 4 == 0 || j == 120 || j == 121)
			{
				akImgHeader[i].index_size += 4 * 9;
				iSizeCount += 4 * 9;
				iSizeCount += akImgIndex[i][j].width * akImgIndex[i][j].height * 4;
				akImgIndex[i][j].size = akImgIndex[i][j].width * akImgIndex[i][j].height * 4;
			}
			else
			{
				akImgHeader[i].index_size += 4 * 2;
				iSizeCount += 4 * 2;
			}
		}

		akNameUncompress.push_back(akNpkIndex[i].name);
		akNpkIndex[i].size = iSizeCount;
		akNpkIndex[i].offset = iSizeOffset;
		for (int j = 0; j < 256; ++j)
		{
			akNpkIndex[i].name[j] ^= decord_flag[j];
		}

		iSizeOffset += iSizeCount;
	}

	const int iTempBuffSize = sizeof(NPK_Header)+sizeof(NPK_Index)* kNpkHeader.count;
	char* kTempBuff = new char[iTempBuffSize];
	int iTempBuffIndex = 0;

	//fwrite(&kNpkHeader, sizeof(NPK_Header), 1, fpMy);
	memcpy(kTempBuff + iTempBuffIndex, &kNpkHeader, sizeof(NPK_Header));
	iTempBuffIndex += sizeof(NPK_Header);
	for (int i = 0; i < kNpkHeader.count; ++i)
	{
		//fwrite(&(akNpkIndex[i]), sizeof(NPK_Index), 1, fpMy);
		memcpy(kTempBuff + iTempBuffIndex, &(akNpkIndex[i]), sizeof(NPK_Index));
		iTempBuffIndex += sizeof(NPK_Index);
	}

	int len = iTempBuffSize / 17 * 17;
	KoishiSHA256::SHA256 sha;
	sha.reset();
	sha.add(kTempBuff, len);
	sha.getHash(dataSHA);

	fwrite(kTempBuff, iTempBuffSize, 1, fpMy);
	fwrite(dataSHA, 32, 1, fpMy);

	int aiDoor[15][4] = { { 0, 0, 0, 1 }, { 1, 0, 0, 0 }, { 1, 0, 0, 1 }, { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 1, 0, 1, 0 }, { 1, 0, 1, 1 }, 
	{ 0, 1, 0, 0 }, { 0, 1, 0, 1 }, { 1, 1, 0, 0 }, { 1, 1, 0, 1 }, { 0, 1, 1, 0 }, { 0, 1, 1, 1 }, { 1, 1, 1, 0 }, { 1, 1, 1, 1 } };
	for (int i = 0; i < kNpkHeader.count; ++i)
	{
		fwrite("Neople Img File", 16, 1, fpMy);
		fwrite(&(akImgHeader[i].index_size), sizeof(int), 1, fpMy);
		fwrite(&(akImgHeader[i].unknown1), sizeof(int), 1, fpMy);
		akImgHeader[i].version = 2;
		fwrite(&(akImgHeader[i].version), sizeof(int), 1, fpMy);
		fwrite(&(akImgHeader[i].index_count), sizeof(int), 1, fpMy);
		int iAllPixlCount = 0;
		for (int j = 0; j < akImgHeader[i].index_count; ++j)
		{
			if (j % 4 == 0 || j == 120 || j == 121)
			{
				akImgIndex[i][j].dwType = ARGB8888;
				fwrite(&(akImgIndex[i][j].dwType), sizeof(int), 1, fpMy);
				akImgIndex[i][j].dwCompress = COMP_NONE;
				fwrite(&(akImgIndex[i][j].dwCompress), sizeof(int), 1, fpMy);
				fwrite(&(akImgIndex[i][j].width), sizeof(int), 1, fpMy);
				fwrite(&(akImgIndex[i][j].height), sizeof(int), 1, fpMy);
				fwrite(&(akImgIndex[i][j].size), sizeof(int), 1, fpMy);
				fwrite(&(akImgIndex[i][j].key_x), sizeof(int), 1, fpMy);
				fwrite(&(akImgIndex[i][j].key_y), sizeof(int), 1, fpMy);
				fwrite(&(akImgIndex[i][j].max_width), sizeof(int), 1, fpMy);
				fwrite(&(akImgIndex[i][j].max_height), sizeof(int), 1, fpMy);

				int iMiniSize = 4;
				int iMaxSize = 14;
				unsigned int iTempColor = 0;
				if (akNameUncompress[i][akNameUncompress[i].length() - 8] == 'a')
				{
					if (j == 120)
					{
						iTempColor = SCOLOR_MINIMAP_UNKONW;
					}
					else if (j == 121)
					{
						iTempColor = SCOLOR_MINIMAP_UNKONW_OPEN;
					}
					else if ((j / 4) % 2 == 0)
					{
						iTempColor = SCOLOR_MINIMAP;
					}
					else if ((j / 4) % 2 == 1)
					{
						iTempColor = SCOLOR_MINIMAP_OPEN;
					}
				}

				int iDoorArrayIndex = j / 8;
				for (int k = 0; k < akImgIndex[i][j].height; ++k)
				{
					for (int l = 0; l < akImgIndex[i][j].width; ++l)
					{
						if (k > iMiniSize && k < iMaxSize && l > iMiniSize && l < iMaxSize)
						{
							iColorTemp[iAllPixlCount++] = iTempColor;
						}
						else if (j == 120 || j == 121)
						{
							iColorTemp[iAllPixlCount++] = 0x00000000;
						}
						else if (aiDoor[iDoorArrayIndex][0] > 0 && l > iMiniSize && l < iMaxSize && k > iMiniSize - 3 && k <= iMiniSize)
						{
							iColorTemp[iAllPixlCount++] = SCOLOR_MINIMAP_DOOR;
						}
						else if (aiDoor[iDoorArrayIndex][1] > 0 && l > iMiniSize && l < iMaxSize && k >= iMaxSize && k < iMaxSize + 3)
						{
							iColorTemp[iAllPixlCount++] = SCOLOR_MINIMAP_DOOR;
						}
						else if (aiDoor[iDoorArrayIndex][2] > 0 && l > iMiniSize - 3 && l <= iMiniSize && k > iMiniSize && k < iMaxSize)
						{
							iColorTemp[iAllPixlCount++] = SCOLOR_MINIMAP_DOOR;
						}
						else if (aiDoor[iDoorArrayIndex][3] > 0 && l >= iMaxSize && l < iMaxSize + 3 && k > iMiniSize && k < iMaxSize)
						{
							iColorTemp[iAllPixlCount++] = SCOLOR_MINIMAP_DOOR;
						}
						else
						{
							iColorTemp[iAllPixlCount++] = 0x00000000;
						}
					}
				}
			}
			else
			{
				akImgIndex[i][j].dwType = LINK;
				fwrite(&(akImgIndex[i][j].dwType), sizeof(int), 1, fpMy);
				akImgIndex[i][j].iLinkNum = j / 4 * 4;
				fwrite(&(akImgIndex[i][j].iLinkNum), sizeof(int), 1, fpMy);
			}
		}
		fwrite(iColorTemp, sizeof(int), iAllPixlCount, fpMy);
	}
	fclose(fpMy);
}

void ExportSpecialNPKFiles()
{
	ExportSpecialNPKFileMiniMapBack();
}

void ExportNPKFiles(starsImgType eStarsImgTYpe)
{
	char kStr[256];
	std::vector<std::string> akSplitStr;

	std::map<std::string, std::map<std::string, std::string>> akImgComfig;
	std::vector<std::string> akPicturePathCombine;
	std::vector<std::string> akPictureNameCombine;
	GetFiles("./FileConfig/Combine/", akPicturePathCombine, akPictureNameCombine);
	int iNpcCount = 0;
	for (int i = 0; i < akPicturePathCombine.size(); ++i)
	{
		//if (!CheckNPKNameType(akPictureNameCombine[i], eStarsImgTYpe)) continue;

		FILE *fp = fopen(akPicturePathCombine[i].c_str(), "r+");
		if (!fp)
		{
			printf("fopen->akPicturePathAuto Faild!");
			return;
		}
		while (fgets(kStr, 256, fp) != NULL)
		{
			SplitStr(kStr, "|", akSplitStr);
			if (akSplitStr.size() == 2)
			{
				akSplitStr[1].erase(akSplitStr[1].length() - 1);
				akImgComfig[akPictureNameCombine[i]][akSplitStr[0]] = akSplitStr[1];
			}
		}
		fclose(fp);
		iNpcCount++;
	}

	NPK_Header kNpkHeader;
	std::vector<NPK_Index> akNpkIndex;
	unsigned char dataSHA[32];
	std::vector<NImgF_Header> akImgHeader;
	std::vector<std::vector<NImgF_Index>> akImgIndex;
	std::map<std::string, std::map<std::string, std::string>>::iterator itr = akImgComfig.begin();
	unsigned int iColorTemp[3500 * 3500];
	int iCurNpkCount = 0;
	for (; itr != akImgComfig.end(); itr++)
	{
		printf("%d/%d\n", iCurNpkCount, iNpcCount);
		iCurNpkCount++;
		std::string kName = GAME_IMG_PATH;
		kName += itr->first.substr(0, itr->first.size() - 4);
		kName += ".NPK";

		akNpkIndex.clear();
		akImgHeader.clear();
		akImgIndex.clear();
		ReadNPKFile(kName, kNpkHeader, akNpkIndex, dataSHA, akImgHeader, akImgIndex);

		kName = "./FileExport/";
		kName += "a";
		kName += itr->first.substr(0, itr->first.size() - 4);
		kName += ".NPK";

		FILE *fpMy = fopen(kName.c_str(), "w+");
		if (!fpMy)
		{
			printf("FileExport Faild!");
			return;
		}
		fclose(fpMy);

		fpMy = fopen(kName.c_str(), "wb");
		if (!fpMy)
		{
			printf("FileExport Faild!");
			return;
		}

		// �����ļ���С
		std::vector<std::string> akNameUncompress;
		int iSizeOffset = 0;
		iSizeOffset += sizeof(NPK_Header)+sizeof(NPK_Index)* kNpkHeader.count+32;
		for (int i = 0; i < kNpkHeader.count; ++i)
		{
			int iSizeCount = 0;
			iSizeCount += 4 * 8;
			akImgHeader[i].index_size = 0;
			for (int j = 0; j < akImgHeader[i].index_count; ++j)
			{
				if (j == 0)
				{
					if (akImgIndex[i][j].width < 5) akImgIndex[i][j].width = 100;
					if (akImgIndex[i][j].height < 5) akImgIndex[i][j].height = 100;

					akImgHeader[i].index_size += 4 * 9;
					iSizeCount += 4 * 9;
					iSizeCount += akImgIndex[i][j].width * akImgIndex[i][j].height * 4;
					akImgIndex[i][j].size = akImgIndex[i][j].width * akImgIndex[i][j].height * 4;
				}
				else
				{
					akImgHeader[i].index_size += 4 * 2;
					iSizeCount += 4 * 2;
				}
			}

			akNameUncompress.push_back(akNpkIndex[i].name);
			akNpkIndex[i].size = iSizeCount;
			akNpkIndex[i].offset = iSizeOffset;
			for (int j = 0; j < 256; ++j)
			{
				akNpkIndex[i].name[j] ^= decord_flag[j];
			}

			iSizeOffset += iSizeCount;
		}

		const int iTempBuffSize = sizeof(NPK_Header)+sizeof(NPK_Index)* kNpkHeader.count;
		char* kTempBuff = new char[iTempBuffSize];
		int iTempBuffIndex = 0;

		//fwrite(&kNpkHeader, sizeof(NPK_Header), 1, fpMy);
		memcpy(kTempBuff + iTempBuffIndex, &kNpkHeader, sizeof(NPK_Header));
		iTempBuffIndex += sizeof(NPK_Header);
		for (int i = 0; i < kNpkHeader.count; ++i)
		{
			//fwrite(&(akNpkIndex[i]), sizeof(NPK_Index), 1, fpMy);
			memcpy(kTempBuff + iTempBuffIndex, &(akNpkIndex[i]), sizeof(NPK_Index));
			iTempBuffIndex += sizeof(NPK_Index);
		}
		
		int len = iTempBuffSize / 17 * 17;
		KoishiSHA256::SHA256 sha;
		sha.reset();
		sha.add(kTempBuff, len);
		sha.getHash(dataSHA);

		fwrite(kTempBuff, iTempBuffSize, 1, fpMy);
		fwrite(dataSHA, 32, 1, fpMy);
		
		for (int i = 0; i < kNpkHeader.count; ++i)
		{
			fwrite("Neople Img File", 16, 1, fpMy);
			fwrite(&(akImgHeader[i].index_size), sizeof(int), 1, fpMy);
			fwrite(&(akImgHeader[i].unknown1), sizeof(int), 1, fpMy);
			akImgHeader[i].version = 2;
			fwrite(&(akImgHeader[i].version), sizeof(int), 1, fpMy);
			fwrite(&(akImgHeader[i].index_count), sizeof(int), 1, fpMy);
			int iAllPixlCount = 0;
			for (int j = 0; j < akImgHeader[i].index_count; ++j)
			{
				if (j == 0)
				{
					akImgIndex[i][j].dwType = ARGB8888;
					fwrite(&(akImgIndex[i][j].dwType), sizeof(int), 1, fpMy);
					akImgIndex[i][j].dwCompress = COMP_NONE;
					fwrite(&(akImgIndex[i][j].dwCompress), sizeof(int), 1, fpMy);
					fwrite(&(akImgIndex[i][j].width), sizeof(int), 1, fpMy);
					fwrite(&(akImgIndex[i][j].height), sizeof(int), 1, fpMy);
					fwrite(&(akImgIndex[i][j].size), sizeof(int), 1, fpMy);
					fwrite(&(akImgIndex[i][j].key_x), sizeof(int), 1, fpMy);
					fwrite(&(akImgIndex[i][j].key_y), sizeof(int), 1, fpMy);
					fwrite(&(akImgIndex[i][j].max_width), sizeof(int), 1, fpMy);
					fwrite(&(akImgIndex[i][j].max_height), sizeof(int), 1, fpMy);
					iAllPixlCount += akImgIndex[i][j].width * akImgIndex[i][j].height;
					std::string& kSImgType = itr->second[akNameUncompress[i]];

					int iMonsterHeight = 0;
					unsigned int iTempColor = 0;
					if (kSImgType == "SIMG_MONSTER")
					{
						iMonsterHeight = akImgIndex[i][j].height - 35;
						iTempColor = SCOLOR_MONSTER;
					}
					else if (kSImgType == "SIMG_BOSS")
					{
						iMonsterHeight = 0;
						iTempColor = SCOLOR_BOSS;
					}
					else if (kSImgType == "SIMG_OBJECT")
					{
						iMonsterHeight = akImgIndex[i][j].height - 35;
						iTempColor = SCOLOR_OBJECT;
					}
					else if (kSImgType == "SIMG_BLOCK")
					{
						iMonsterHeight = akImgIndex[i][j].height - 35;
						iTempColor = SCOLOR_BLOCK;
					}
					else if (kSImgType == "SIMG_PATHGATE")
					{
						iMonsterHeight = 0;
						if (akNameUncompress[i].find("light") != std::string::npos)
						{
							iTempColor = 0;
						}
						else if (akNameUncompress[i].find("up") != std::string::npos)
						{
							iTempColor = SCOLOR_PATHGATE_UP;
						}
						else if (akNameUncompress[i].find("down") != std::string::npos)
						{
							iTempColor = SCOLOR_PATHGATE_DOWN;
						}
						else
						{
							iTempColor = SCOLOR_PATHGATE;
						}
					}
					else if (kSImgType == "SIMG_BACKGROUND")
					{
						iMonsterHeight = 0;
						iTempColor = SCOLOR_BLOCK;
					}
					else
					{
						iMonsterHeight = 0;
						iTempColor = 0;
					}

					for (int k = 0; k < akImgIndex[i][j].height; ++k)
					{
						for (int l = 0; l < akImgIndex[i][j].width; ++l)
						{
							if (k >= iMonsterHeight)
							{
								iColorTemp[k * akImgIndex[i][j].width + l] = iTempColor;
							}
							else
							{
								iColorTemp[k * akImgIndex[i][j].width + l] = 0x00000000;
							}
						}
					}
				}
				else
				{
					akImgIndex[i][j].dwType = LINK;
					fwrite(&(akImgIndex[i][j].dwType), sizeof(int), 1, fpMy);
					akImgIndex[i][j].iLinkNum = 0;
					fwrite(&(akImgIndex[i][j].iLinkNum), sizeof(int), 1, fpMy);
				}
			}
			fwrite(iColorTemp, sizeof(int), iAllPixlCount, fpMy);
		}
		fclose(fpMy);
	}
}

int main()
{
	for (int i = 0; i < 256; ++i)
	{
		if (decord_flag[i] == 0)
		{
			int k = 0;
			for (int j = i; j < 256; ++j)
			{
				if (k == 0)
				{
					decord_flag[j] = 'D';
				}
				else if (k == 1)
				{
					decord_flag[j] = 'N';
				}
				else if (k == 2)
				{
					decord_flag[j] = 'F';
				}
				k++;
				k %= 3;
			}
			break;
		}
	}

	//FILE *fp = fopen("test1 (2).NPK", "rb");
	//if (!fp)
	//{
	//	return 0;
	//}
	//unsigned int nFileLen = 0;
	//fseek(fp, 0, SEEK_END); //��λ���ļ�ĩ
	//nFileLen = ftell(fp); //�ļ����� 

	//fseek(fp, 0, SEEK_SET);

	//NPK_Header npkHeader;
	//fread(&npkHeader, sizeof(NPK_Header), 1, fp);

	//NPK_Index* npkIndex = new NPK_Index[npkHeader.count];
	//fread(npkIndex, sizeof(NPK_Index), npkHeader.count, fp);
	//NImgF_Header* imgHeader = new NImgF_Header[npkHeader.count];
	//std::vector<NImgF_Index*> akImgIndex;
	//for (int i = 0; i < npkHeader.count; ++i)
	//{
	//	for (int j = 0; j < 256; ++j)
	//	{
	//		npkIndex[i].name[j] ^= decord_flag[j];
	//	}

	//	fseek(fp, npkIndex[i].offset, SEEK_SET);

	//	// img file
	//	fread(imgHeader[i].flag, 20, 1, fp);
	//	if (strcmp(imgHeader[i].flag, "Neople Img File") == 0)
	//	{
	//		fseek(fp, -4, SEEK_CUR);
	//		fread(&(imgHeader[i].index_size), sizeof(int), 1, fp);
	//	}
	//	else if (strcmp(imgHeader[i].flag, "Neople Image File") == 0)
	//	{
	//		imgHeader[i].index_size = -1;
	//	}
	//	else
	//	{
	//		printf("ERROR:imgHeader[i].flag\n");
	//	}
	//	fread(&(imgHeader[i].unknown1), sizeof(int), 1, fp);
	//	fread(&(imgHeader[i].version), sizeof(int), 1, fp);
	//	if (imgHeader[i].version != 2 && imgHeader[i].version != 4 && imgHeader[i].version != 5 && imgHeader[i].version != 6)
	//	{
	//		printf("ERROR:imgHeader[i].version\n");
	//	}
	//	fread(&(imgHeader[i].index_count), sizeof(int), 1, fp);
	//	if (imgHeader[i].version == 5)
	//	{
	//		fread(&(imgHeader[i].iDDSCount), sizeof(int), 1, fp);
	//		fread(&(imgHeader[i].iTotalLength), sizeof(int), 1, fp);
	//	}
	//	if (imgHeader[i].version == 4 || imgHeader[i].version == 5)
	//	{
	//		int iColorCount;
	//		fread(&iColorCount, sizeof(int), 1, fp);
	//		std::vector<unsigned int> aiColor;
	//		for (int j = 0; j < iColorCount; ++j)
	//		{
	//			unsigned int iColor;
	//			fread(&iColor, sizeof(int), 1, fp);
	//			aiColor.push_back(iColor);
	//		}
	//		imgHeader[i].aiPaletteData.push_back(aiColor);
	//	}
	//	if (imgHeader[i].version == 6)
	//	{
	//		int iPaletteCount;
	//		fread(&iPaletteCount, sizeof(int), 1, fp);
	//		for (int j = 0; j < iPaletteCount; ++j)
	//		{
	//			int iColorCount;
	//			std::vector<unsigned int> aiColor;
	//			fread(&iColorCount, sizeof(int), 1, fp);
	//			for (int k = 0; k < iColorCount; ++k)
	//			{
	//				unsigned int iColor;
	//				fread(&iColor, sizeof(int), 1, fp);
	//				aiColor.push_back(iColor);
	//			}
	//			imgHeader[i].aiPaletteData.push_back(aiColor);
	//		}
	//	}
	//	if (imgHeader[i].version == 5)
	//	{
	//		unsigned int iDDS;
	//		for (int j = 0; j < imgHeader[i].iDDSCount; ++j)
	//		{
	//			fread(&iDDS, sizeof(int), 1, fp);
	//			fread(&iDDS, sizeof(int), 1, fp);
	//			fread(&iDDS, sizeof(int), 1, fp);
	//			fread(&iDDS, sizeof(int), 1, fp);
	//			fread(&iDDS, sizeof(int), 1, fp);
	//			fread(&iDDS, sizeof(int), 1, fp);
	//			fread(&iDDS, sizeof(int), 1, fp);
	//		}
	//	}
	//	if (imgHeader[i].version != 1)
	//	{
	//		NImgF_Index* imgIndex = new NImgF_Index[imgHeader[i].index_count];
	//		for (int j = 0; j < imgHeader[i].index_count; ++j)
	//		{
	//			fread(&(imgIndex[j].dwType), sizeof(int), 1, fp);
	//			if (imgIndex[j].dwType == LINK)
	//			{
	//				fread(&(imgIndex[j].iLinkNum), sizeof(int), 1, fp);
	//			}
	//			else if (imgIndex[j].dwType < LINK)
	//			{
	//				fread(&(imgIndex[j].dwCompress), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].width), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].height), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].size), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].key_x), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].key_y), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].max_width), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].max_height), sizeof(int), 1, fp);
	//			}
	//			else
	//			{
	//				fread(&(imgIndex[j].dwCompress), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].width), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].height), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].size), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].key_x), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].key_y), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].max_width), sizeof(int), 1, fp);
	//				fread(&(imgIndex[j].max_height), sizeof(int), 1, fp);
	//				unsigned int iData;
	//				fread(&iData, sizeof(int), 1, fp);
	//				fread(&iData, sizeof(int), 1, fp);
	//				fread(&iData, sizeof(int), 1, fp);
	//				fread(&iData, sizeof(int), 1, fp);
	//				fread(&iData, sizeof(int), 1, fp);
	//				fread(&iData, sizeof(int), 1, fp);
	//				fread(&iData, sizeof(int), 1, fp);
	//			}

	//		}
	//		akImgIndex.push_back(imgIndex);
	//	}
	//
	//	///////////////
	//}
	//fclose(fp);

	//FILE *fpMy = fopen("atest1 (1).NPK", "wb");
	//if (!fpMy)
	//{
	//	return 0;
	//}

	//// �����ļ���С
	//int iSizeOffset = 0;
	//iSizeOffset += sizeof(NPK_Header) + sizeof(NPK_Index) * npkHeader.count;
	//for (int i = 0; i < npkHeader.count; ++i)
	//{
	//	int iSizeCount = 0;
	//	iSizeCount += 4 * 8;
	//	for (int j = 0; j < imgHeader[i].index_count; ++j)
	//	{
	//		iSizeCount += 4 * 9;
	//		iSizeCount += akImgIndex[i][j].width * akImgIndex[i][j].height * 4;
	//	}

	//	npkIndex[i].size = iSizeCount;
	//	npkIndex[i].offset = iSizeOffset;
	//	for (int j = 0; j < 256; ++j)
	//	{
	//		npkIndex[i].name[j] ^= decord_flag[j];
	//	}

	//	iSizeOffset += iSizeCount;
	//}

	//unsigned int* iColorTemp = new unsigned int[1920 * 1080];
	//for (int i = 0; i < 1080; ++i)
	//{
	//	for (int j = 0; j < 1920; ++j)
	//	{
	//		iColorTemp[i * 1920 + j] = 0xFFFF00FF;
	//	}
	//}
	//fwrite(&npkHeader, sizeof(NPK_Header), 1, fpMy);
	//fwrite(npkIndex, sizeof(NPK_Index), npkHeader.count, fpMy);
	//for (int i = 0; i < npkHeader.count; ++i)
	//{
	//	fwrite("Neople Img File", 16, 1, fpMy);
	//	fwrite(&(imgHeader[i].index_size), sizeof(int), 1, fpMy);
	//	fwrite(&(imgHeader[i].unknown1), sizeof(int), 1, fpMy);
	//	imgHeader[i].version = 2;
	//	fwrite(&(imgHeader[i].version), sizeof(int), 1, fpMy);
	//	fwrite(&(imgHeader[i].index_count), sizeof(int), 1, fpMy);
	//	int iAllPixlCount = 0;
	//	for (int j = 0; j < imgHeader[i].index_count; ++j)
	//	{
	//		akImgIndex[i][j].dwType = ARGB8888;
	//		fwrite(&(akImgIndex[i][j].dwType), sizeof(int), 1, fpMy);
	//		akImgIndex[i][j].dwCompress = COMP_NONE;
	//		fwrite(&(akImgIndex[i][j].dwCompress), sizeof(int), 1, fpMy);
	//		fwrite(&(akImgIndex[i][j].width), sizeof(int), 1, fpMy);
	//		fwrite(&(akImgIndex[i][j].height), sizeof(int), 1, fpMy);
	//		fwrite(&(akImgIndex[i][j].size), sizeof(int), 1, fpMy);
	//		fwrite(&(akImgIndex[i][j].key_x), sizeof(int), 1, fpMy);
	//		fwrite(&(akImgIndex[i][j].key_y), sizeof(int), 1, fpMy);
	//		fwrite(&(akImgIndex[i][j].max_width), sizeof(int), 1, fpMy);
	//		fwrite(&(akImgIndex[i][j].max_height), sizeof(int), 1, fpMy);
	//		iAllPixlCount += akImgIndex[i][j].width * akImgIndex[i][j].height;
	//	}
	//	fwrite(iColorTemp, sizeof(int), iAllPixlCount, fpMy);
	//}
	//fclose(fpMy);
	
	printf("1:ExportAutoConfig\n2:CombineConfig\n3:ExportNPKFiles\n4:ExportSpecialNPKFiles\n");
	char a;
	while (a = getchar())
	{
		getchar();
		if (a == '1' || a == '3')
		{
			
			if (a == '1')
			{
				printf("ExportAutoConfig\n");
				printf("0:SIMG_NONE\n1:SIMG_MONSTER\n2:SIMG_OBJECT\n3:SIMG_BLOCK\n4:SIMG_PATHGATE\n5:SIMG_BACKGROUND\n");
			}
			else
			{
				printf("ExportNPKFiles\n");
			}

			if (a == '1')
			{
				char b = getchar();
				getchar();
				OutPutAutoConfig(starsImgType(b - '1' + 1));
				printf("Done!\n");
			}
			else
			{
				ExportNPKFiles(SIMG_NONE);
				printf("Done!\n");
			}
		}
		else if (a == '2')
		{
			OutPutCombineConfig();
			printf("Done!\n");
		}
		else if (a == '4')
		{
			ExportSpecialNPKFiles();
			printf("Done!\n");
		}
		printf("1:ExportAutoConfig\n2:CombineConfig\n3:ExportNPKFiles\n4:ExportSpecialNPKFiles\n");
	}
	return 0;
}