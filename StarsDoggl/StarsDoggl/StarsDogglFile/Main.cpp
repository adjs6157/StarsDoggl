#include "lz4.h"
#include "zlib.h"
#include <string>
#include <io.h>
#include <direct.h>
#include <vector>

enum compressType{ COMP_NONE = 5, COMP_ZLIB = 6, COMP_ZLIB2 = 7, COMP_UDEF = 0 };
enum colorFormat{ ARGB8888 = 0x10, ARGB4444 = 0x0F, ARGB1555 = 0x0E, LINK = 0x11, DDS_DXT1 = 0x12, DDS_DXT3 = 0x13, DDS_DXT5 = 0x14, COLOR_UDEF = 0, V4_FMT, RGB565 };

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
};


std::string GetNpkFileName(char *pData, unsigned int maxLen)
{
	std::string name;
	for (int i = 0; i < maxLen; ++i)
	{
		if ((pData[i] >= 'a' && pData[i] <= 'z')
			|| (pData[i] >= 'A' && pData[i] <= 'Z'))
		{
			name += pData[i];
		}
	}
	return name;
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

	FILE *fp = fopen("test1 (1).NPK", "rb");
	if (!fp)
	{
		return 0;
	}
	unsigned int nFileLen = 0;
	fseek(fp, 0, SEEK_END); //��λ���ļ�ĩ
	nFileLen = ftell(fp); //�ļ����� 

	fseek(fp, 0, SEEK_SET);

	NPK_Header npkHeader;
	fread(&npkHeader, sizeof(NPK_Header), 1, fp);

	NPK_Index* npkIndex = new NPK_Index[npkHeader.count];
	fread(npkIndex, sizeof(NPK_Index), npkHeader.count, fp);
	NImgF_Header* imgHeader = new NImgF_Header[npkHeader.count];
	std::vector<NImgF_Index*> akImgIndex;
	for (int i = 0; i < npkHeader.count; ++i)
	{
		for (int j = 0; j < 256; ++j)
		{
			npkIndex[i].name[j] ^= decord_flag[j];
		}

		fseek(fp, npkIndex[i].offset, SEEK_SET);

		// img file
		fread(imgHeader[i].flag, 20, 1, fp);
		if (strcmp(imgHeader[i].flag, "Neople Img File") == 0)
		{
			fseek(fp, -4, SEEK_CUR);
			fread(&(imgHeader[i].index_size), sizeof(int), 1, fp);
		}
		else if (strcmp(imgHeader[i].flag, "Neople Image File") == 0)
		{
			imgHeader[i].index_size = -1;
		}
		else
		{
			printf("ERROR:imgHeader[i].flag\n");
		}
		fread(&(imgHeader[i].unknown1), sizeof(int), 1, fp);
		fread(&(imgHeader[i].version), sizeof(int), 1, fp);
		if (imgHeader[i].version != 2 && imgHeader[i].version != 4 && imgHeader[i].version != 5 && imgHeader[i].version != 6)
		{
			printf("ERROR:imgHeader[i].version\n");
		}
		fread(&(imgHeader[i].index_count), sizeof(int), 1, fp);
		if (imgHeader[i].version == 5)
		{
			fread(&(imgHeader[i].iDDSCount), sizeof(int), 1, fp);
			fread(&(imgHeader[i].iTotalLength), sizeof(int), 1, fp);
		}
		if (imgHeader[i].version == 4 || imgHeader[i].version == 5)
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
			imgHeader[i].aiPaletteData.push_back(aiColor);
		}
		if (imgHeader[i].version == 6)
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
				imgHeader[i].aiPaletteData.push_back(aiColor);
			}
		}
		if (imgHeader[i].version == 5)
		{
			unsigned int iDDS;
			for (int j = 0; j < imgHeader[i].iDDSCount; ++j)
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
		if (imgHeader[i].version != 1)
		{
			NImgF_Index* imgIndex = new NImgF_Index[imgHeader[i].index_count];
			for (int j = 0; j < imgHeader[i].index_count; ++j)
			{
				fread(&(imgIndex[j].dwType), sizeof(int), 1, fp);
				if (imgIndex[j].dwType == LINK)
				{
					fread(&(imgIndex[j].iLinkNum), sizeof(int), 1, fp);
				}
				else if (imgIndex[j].dwType < LINK)
				{
					fread(&(imgIndex[j].dwCompress), sizeof(int), 1, fp);
					fread(&(imgIndex[j].width), sizeof(int), 1, fp);
					fread(&(imgIndex[j].height), sizeof(int), 1, fp);
					fread(&(imgIndex[j].size), sizeof(int), 1, fp);
					fread(&(imgIndex[j].key_x), sizeof(int), 1, fp);
					fread(&(imgIndex[j].key_y), sizeof(int), 1, fp);
					fread(&(imgIndex[j].max_width), sizeof(int), 1, fp);
					fread(&(imgIndex[j].max_height), sizeof(int), 1, fp);
				}
				else
				{
					fread(&(imgIndex[j].dwCompress), sizeof(int), 1, fp);
					fread(&(imgIndex[j].width), sizeof(int), 1, fp);
					fread(&(imgIndex[j].height), sizeof(int), 1, fp);
					fread(&(imgIndex[j].size), sizeof(int), 1, fp);
					fread(&(imgIndex[j].key_x), sizeof(int), 1, fp);
					fread(&(imgIndex[j].key_y), sizeof(int), 1, fp);
					fread(&(imgIndex[j].max_width), sizeof(int), 1, fp);
					fread(&(imgIndex[j].max_height), sizeof(int), 1, fp);
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
			akImgIndex.push_back(imgIndex);
		}
	
		///////////////
	}
	fclose(fp);

	FILE *fpMy = fopen("atest1 (1).NPK", "wb");
	if (!fpMy)
	{
		return 0;
	}

	// �����ļ���С
	int iSizeOffset = 0;
	iSizeOffset += sizeof(NPK_Header) + sizeof(NPK_Index) * npkHeader.count;
	for (int i = 0; i < npkHeader.count; ++i)
	{
		int iSizeCount = 0;
		iSizeCount += 4 * 8;
		for (int j = 0; j < imgHeader[i].index_count; ++j)
		{
			iSizeCount += 4 * 9;
			iSizeCount += akImgIndex[i][j].width * akImgIndex[i][j].height * 4;
		}

		npkIndex[i].size = iSizeCount;
		npkIndex[i].offset = iSizeOffset;
		for (int j = 0; j < 256; ++j)
		{
			npkIndex[i].name[j] ^= decord_flag[j];
		}

		iSizeOffset += iSizeCount;
	}

	unsigned int* iColorTemp = new unsigned int[1920 * 1080];
	for (int i = 0; i < 1080; ++i)
	{
		for (int j = 0; j < 1920; ++j)
		{
			iColorTemp[i * 1920 + j] = 0xFFFF00FF;
		}
	}
	fwrite(&npkHeader, sizeof(NPK_Header), 1, fpMy);
	fwrite(npkIndex, sizeof(NPK_Index), npkHeader.count, fpMy);
	for (int i = 0; i < npkHeader.count; ++i)
	{
		fwrite("Neople Img File", 16, 1, fpMy);
		fwrite(&(imgHeader[i].index_size), sizeof(int), 1, fpMy);
		fwrite(&(imgHeader[i].unknown1), sizeof(int), 1, fpMy);
		imgHeader[i].version = 2;
		fwrite(&(imgHeader[i].version), sizeof(int), 1, fpMy);
		fwrite(&(imgHeader[i].index_count), sizeof(int), 1, fpMy);
		int iAllPixlCount = 0;
		for (int j = 0; j < imgHeader[i].index_count; ++j)
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
		}
		fwrite(iColorTemp, sizeof(int), iAllPixlCount, fpMy);
	}
	fclose(fpMy);

	getchar();
	return 0;
}