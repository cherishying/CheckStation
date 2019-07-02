#ifndef _KXSEARCHCARDHHHHH
#define _KXSEARCHCARDHHHHH

// add by lyl 2017/1/5 used to auto search card
// no parameter

#include "kxDef.h"
#include "gxMath.h"
#include "ippi.h"
//#include "ippm.h"
#include "mkl.h"
#include "KxBaseFunction.h"

class CKxSearchCards
{
public:
	CKxSearchCards();
	~CKxSearchCards();
public:
	enum
	{
		_SEARCHAREA_NUM = 2,
		_HORIZONSEARCH_NUM = 8,  //水平搜索点数
		_VERSEARCH_NUM = 8,      //垂直搜索点数

		_SEARCH_EDGE_SIDE = 4,

		_SEARCH_VER_DIR = 8,
		_SEARCH_HOR_DIR = 9,

		_UP_TO_BOTTOM,
		_BOTTOM_TO_UP,

		_LEFT_TO_RIGHT,
		_RIGHT_TO_LEFT,

		_SEARCH_WIDTH = 1600,
		_SEARCH_HEIGHT = 1200,

		_SEARCH_FOUR_EDGE = 0,  //搜索四边
		_SEARCH_THREE_EDGE = 1, //搜索三边
	};
	enum
	{
		_EXTEND_LEN = 0, //外里进去5个像素
	};

#pragma pack(push, 1)

	struct Parameter
	{
		Parameter()
		{
			memcpy(m_szVersion, "SearchCard1.0", _VersionNameLen);
			m_nVerThresh = 10;
			m_nHorThresh = 10;
		}
		char          m_szVersion[_VersionNameLen];    //记录参数的版本信息
		int           m_nVerThresh;                   //垂直阈值
		int           m_nHorThresh;                   //水平阈值
	};

#pragma pack(pop)

	struct Result
	{
		Result()
		{
			Clear();
		}

		void Clear()
		{
			memset(m_nVerGradient, 0, sizeof(int)*_SEARCHAREA_NUM*_SEARCH_HEIGHT * 2);
			memset(m_nHorGradient, 0, sizeof(int)*_SEARCHAREA_NUM*_SEARCH_WIDTH * 2);

			memset(m_ptVerBestPosition, 0, sizeof(kxPoint<int>)*_SEARCHAREA_NUM * 2);
			memset(m_ptHorBestPosition, 0, sizeof(kxPoint<int>)*_SEARCHAREA_NUM * 2);

			memset(m_nVerMaxGradient, 0, sizeof(int)*_SEARCHAREA_NUM * 2);
			memset(m_nHorMaxGradient, 0, sizeof(int)*_SEARCHAREA_NUM * 2);

			//m_fAngle = 0.0;
			for (int i = 0; i < _SEARCH_EDGE_SIDE; i++)
			{
				m_ptAnchor[i].setup(0, 0);
			}
		}

		int               m_nVerMaxGradient[2][_SEARCHAREA_NUM];
		int               m_nVerGradient[2][_SEARCHAREA_NUM][_SEARCH_HEIGHT];
		kxPoint<int>      m_ptVerBestPosition[2][_SEARCHAREA_NUM];
		int               m_nHorMaxGradient[2][_SEARCHAREA_NUM];
		int               m_nHorGradient[2][_SEARCHAREA_NUM][_SEARCH_WIDTH];
		kxPoint<int>      m_ptHorBestPosition[2][_SEARCHAREA_NUM];
		//double            m_fAngle;
		kxPoint<int>   m_ptAnchor[_SEARCH_EDGE_SIDE];

	};
protected:
	//参数版本1的读写
	bool ReadVesion1Para(FILE* fp);
	bool WriteVesion1Para(FILE* fp);
public:
	int  Load(const char* lpszFile);
	int  Read(unsigned char *& point);
	bool Read(FILE*);
protected:
	Parameter   m_Parameter;
	Result      m_hResult;

	kxImageBuf  m_KernImg;
	kxImageBuf  m_TmpBuf;
	kxImageBuf  m_BinaryImg;
	kxImageBuf  m_CopyImg;

	kxImageBuf  m_ModelImg;

	int         m_nWarpWidth;
	int         m_nWarpHeight;

	int         m_nCardVerStart;
	int         m_nCardVerEnd;

	int         m_nExtendLen;
	CKxBaseFunction  m_hBaseFun;

	int           m_nBeltHorizStart;  //皮带的水平起始位置
	int           m_nBeltHorizEnd;    //皮带的水平终止位置
	int           m_nSampleStep;      //采样步幅

	bool FindMaxGradientPosition(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, kxPoint<int> ptStartPoint, int nSearchDir, int nDir, int nGradThresh,
		int* pGradValue, kxPoint<int>& ptMaxGrad, int& pMaxGradValue);
	int MarkPoint(kxCImageBuf& DstImg, kxPoint<int> pts[], int N);
	bool FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float* pCoeff);
	void  combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result);
	float FitLineByInteration(kxPoint<int>* pts, int nLen, int nSampleCount, int nFitLineDots, float* fLineCoeff);
	int   FiltLine2D(kxPoint<int>* pts, int n, float* pCoeff);
public:
	bool Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, bool bModel, KxCallStatus& hCall);
	bool Check(const kxCImageBuf& SrcImg, bool bModel, KxCallStatus& hCall);
	bool Check(const kxCImageBuf& SrcImg, bool bModel = false);

	unsigned char* GetModelImg(int& nW, int& nH, int& nP, int& nChannel)
	{
		return m_ModelImg.GetImageBuf(nW, nH, nP, nChannel);
	}
	kxCImageBuf& GetModelImg() { return m_ModelImg; }

	Result& GetResult()
	{
		return m_hResult;
	}

	void SetWarpSize(int& nW, int& nH)
	{
		m_nWarpHeight = nH;
		m_nWarpWidth = nW;
	}

	void SetExtendLen(int nLen)
	{
		m_nExtendLen = nLen;
	}

	void SetBeltHorizontalPos(int nStart, int nEnd)
	{
		m_nBeltHorizStart = nStart;
		m_nBeltHorizEnd = nEnd;
	}

	void SetSearchCardThresh(int nVerThresh, int nHorThresh)
	{
		m_Parameter.m_nHorThresh = nHorThresh;
		m_Parameter.m_nVerThresh = nVerThresh;
	}



};

#endif
