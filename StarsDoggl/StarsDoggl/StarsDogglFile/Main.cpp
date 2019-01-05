#include "lz4.h"
#include "zlib.h"
#include <string>
#include <io.h>
#include <direct.h>
#include <vector>
#include <map>

enum compressType{ COMP_NONE = 5, COMP_ZLIB = 6, COMP_ZLIB2 = 7, COMP_UDEF = 0 };
enum colorFormat{ ARGB8888 = 0x10, ARGB4444 = 0x0F, ARGB1555 = 0x0E, LINK = 0x11, DDS_DXT1 = 0x12, DDS_DXT3 = 0x13, DDS_DXT5 = 0x14, COLOR_UDEF = 0, V4_FMT, RGB565 };
enum starsImgType{ SIMG_NONE = 0, SIMG_MONSTER = 1, SIMG_OBJECT = 2, SIMG_BLOCK = 3, SIMG_PATHGATE = 4, SIMG_ITEM = 5 };
enum starsImgColor{ SCOLOR_NONE = 0, SCOLOR_MONSTER = 0xFFFF00FF, SCOLOR_OBJECT = 0xFFFFFF00, SCOLOR_BLOCK = 0xFFFF0000, SCOLOR_PATHGATE = 0xFF00FF00, SCOLOR_ITEM = 0xFF00FF00 };
#define GAME_IMG_PATH "E:\\Game\\腾讯游戏\\地下城与勇士\\ImagePacks2\\"

struct NPK_Header
{
	char flag[16]; // 文件标识 "NeoplePack_Bill"
	int count;     // 包内文件的数目
};

struct NPK_Index
{
	unsigned int offset;  // 文件的包内偏移量
	unsigned int size;    // 文件的大小
	char name[256];// 文件名
};

char decord_flag[256] = "puchikon@neople dungeon and fighter DNF";

struct NImgF_Header
{
	char flag[20]; // 文件标石"Neople Img File"
	int index_size;	// 索引表大小，以字节为单位
	int unknown1;
	int version;
	int index_count;// 索引表数目
	int iDDSCount;
	int iTotalLength;
	std::vector<std::vector<unsigned int>> aiPaletteData;
};

struct NImgF_Index
{
	unsigned int dwType; //目前已知的类型有 0x0E(1555格式) 0x0F(4444格式) 0x10(8888格式) 0x11(不包含任何数据，可能是指内容同上一帧)
	unsigned int dwCompress; // 目前已知的类型有 0x06(zlib压缩) 0x05(未压缩)
	int width;        // 宽度
	int height;       // 高度
	int size;         // 压缩时size为压缩后大小，未压缩时size为转换成8888格式时占用的内存大小
	int key_x;        // X关键点，当前图片在整图中的X坐标
	int key_y;        // Y关键点，当前图片在整图中的Y坐标
	int max_width;    // 整图的宽度
	int max_height;   // 整图的高度，有此数据是为了对齐精灵
	int iLinkNum;
};

bool ReadNPKFile(std::string path, NPK_Header& kNpkHeader, std::vector<NPK_Index>& akNpkIndex, std::vector<NImgF_Header>& akImgHeader, std::vector<std::vector<NImgF_Index>>& akImgIndex)
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

			}
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
		return false;
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
	case SIMG_ITEM:
		return false;
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

void OutPutAutoConfig(starsImgType eType)
{
	std::map<std::string, std::map<std::string, int>> akImgType;

	NPK_Header kNpkHeader;
	std::vector<NPK_Index> akNpkIndex;
	std::vector<NImgF_Header> akImgHeader;
	std::vector<std::vector<NImgF_Index>> akImgIndex;

	std::vector<std::string> akPicturePath;
	std::vector<std::string> akPictureName;
	GetFiles(GAME_IMG_PATH, akPicturePath, akPictureName);

	for (int i = 0; i < akPictureName.size(); ++i)
	{
		printf("%d/%d\n", i, akPictureName.size());
		std::string& kPictureName = akPictureName[i];
		if (!CheckNPKNameType(kPictureName, eType)) continue;

		akNpkIndex.clear();
		akImgHeader.clear();
		akImgIndex.clear();
		ReadNPKFile(akPicturePath[i], kNpkHeader, akNpkIndex, akImgHeader, akImgIndex);

		std::string kAutoConfitPath = "./FileConfig/Auto/";
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
			else if (eType == SIMG_ITEM)
			{
				
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
		if (!CheckNPKNameType(akPictureNameCombine[i], eStarsImgTYpe)) continue;

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
				akSplitStr[1].erase(akSplitStr.size() - 1);
				akImgComfig[akPictureNameCombine[i]][akSplitStr[0]] = akSplitStr[1];
			}
		}
		fclose(fp);
		iNpcCount++;
	}

	NPK_Header kNpkHeader;
	std::vector<NPK_Index> akNpkIndex;
	std::vector<NImgF_Header> akImgHeader;
	std::vector<std::vector<NImgF_Index>> akImgIndex;
	std::map<std::string, std::map<std::string, std::string>>::iterator itr = akImgComfig.begin();
	unsigned int iColorTemp[1920 * 1080];
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
		ReadNPKFile(kName, kNpkHeader, akNpkIndex, akImgHeader, akImgIndex);

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
		// 计算文件大小
		std::vector<std::string> akNameUncompress;
		int iSizeOffset = 0;
		iSizeOffset += sizeof(NPK_Header)+sizeof(NPK_Index)* kNpkHeader.count;
		for (int i = 0; i < kNpkHeader.count; ++i)
		{
			int iSizeCount = 0;
			iSizeCount += 4 * 8;
			for (int j = 0; j < akImgHeader[i].index_count; ++j)
			{
				if (j == 0)
				{
					iSizeCount += 4 * 9;
					iSizeCount += akImgIndex[i][j].width * akImgIndex[i][j].height * 4;
				}
				else
				{
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

		fwrite(&kNpkHeader, sizeof(NPK_Header), 1, fpMy);
		for (int i = 0; i < kNpkHeader.count; ++i)
		{
			fwrite(&(akNpkIndex[i]), sizeof(NPK_Index), 1, fpMy);
		}
		
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
						iTempColor = SCOLOR_PATHGATE;
					}
					else if (kSImgType == "SIMG_ITEM")
					{
						iMonsterHeight = 0;
						iTempColor = SCOLOR_ITEM;
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

	//FILE *fp = fopen("test1 (1).NPK", "rb");
	//if (!fp)
	//{
	//	return 0;
	//}
	//unsigned int nFileLen = 0;
	//fseek(fp, 0, SEEK_END); //定位到文件末
	//nFileLen = ftell(fp); //文件长度 

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

	//// 计算文件大小
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
	
	printf("1:ExportAutoConfig\n2:CombineConfig\n3:ExportNPKFiles\n");
	char a;
	while (a = getchar())
	{
		getchar();
		if (a == '1' || a == '3')
		{
			
			if (a == '1')
			{
				printf("ExportAutoConfig\n");
			}
			else
			{
				printf("ExportNPKFiles\n");
			}
			printf("1:SIMG_MONSTER\n2:SIMG_OBJECT\n3:SIMG_BLOCK\n4:SIMG_PATHGATE\n5:SIMG_ITEM\n");
			char b = getchar();
			getchar();
			if (a == '1')
			{
				OutPutAutoConfig(starsImgType(b - '1' + 1));
				printf("Done!\n");
			}
			else
			{
				ExportNPKFiles(SIMG_OBJECT/*starsImgType(b - '1' + 1)*/);
				printf("Done!\n");
			}
		}
		else if (a == '2')
		{
			OutPutCombineConfig();
			printf("Done!\n");
		}
		printf("1:ExportAutoConfig\n2:CombineConfig\n3:ExportNPKFiles\n");
	}
	return 0;
}