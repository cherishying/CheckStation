#ifndef _KXROTATECARDSHHHH
#define _KXROTATECARDSHHHH

// add by lyl 2015/10/4 used to rotate card
////参数初始版本名为：RotateCard1.0

#include "kxDef.h"
#include "gxMath.h"
#include "ippi.h"
//#include "ippm.h"
#include "mkl.h"
#include "KxBaseFunction.h"

class CKxRotateCards
{
public:
	CKxRotateCards();
	~CKxRotateCards();
public:
	enum
	{
		_SEARCHAREA_NUM = 2,
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
			memcpy(m_szVersion,  "RotateCard1.0", _VersionNameLen);
			m_rcVerSearch[0][0].setup(435, 135, 487, 377);
			m_rcVerSearch[0][1].setup(1225, 42, 1265, 390);

			m_rcVerSearch[1][0].setup(568, 880, 600, 1042);
			m_rcVerSearch[1][1].setup(1044, 881, 1077, 1043);

			m_rcHorSearch[0][0].setup(50,375,500,450);
			m_rcHorSearch[0][1].setup(58,528,470,580);

			m_rcHorSearch[1][0].setup(1311,359,1555,407);
			m_rcHorSearch[1][1].setup(1320,560,1562,604);

			m_nSearchLen = 400;
			m_nExtendWidth = 1;

			m_nVerThresh = 15;
			m_nHorThresh = 15;

		}
        char          m_szVersion[_VersionNameLen];    //记录参数的版本信息
		kxRect<int>   m_rcVerSearch[2][_SEARCHAREA_NUM];    //vertical search area, is used to location the start point
		kxRect<int>   m_rcHorSearch[2][_SEARCHAREA_NUM];
		int           m_nSearchLen;
		int           m_nExtendWidth;   //
		int           m_nVerThresh;
		int           m_nHorThresh;

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
			memset(m_nVerGradient, 0, sizeof(int)*_SEARCHAREA_NUM*_SEARCH_HEIGHT*2);
			memset(m_nHorGradient, 0, sizeof(int)*_SEARCHAREA_NUM*_SEARCH_WIDTH*2);

			memset(m_ptVerBestPosition, 0, sizeof(kxPoint<int>)*_SEARCHAREA_NUM*2);
			memset(m_ptHorBestPosition, 0, sizeof(kxPoint<int>)*_SEARCHAREA_NUM*2);

			memset(m_nVerMaxGradient, 0, sizeof(int)*_SEARCHAREA_NUM*2);
			memset(m_nHorMaxGradient, 0, sizeof(int)*_SEARCHAREA_NUM*2);

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
	Parameter   m_Parameter;
	Result      m_hResult;

	kxImageBuf  m_KernImg;
	kxImageBuf  m_TmpBuf;

	kxImageBuf  m_ModelImg;

	int         m_nWarpWidth;
	int         m_nWarpHeight;

	int         m_nExtendLen;
	CKxBaseFunction  m_hBaseFun;

	bool FindMaxGradientPosition(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nImgType, kxPoint<int> ptStartPoint, int nSearchDir, int nDir, int nGradThresh,
		int* pGradValue, kxPoint<int>& ptMaxGrad, int& pMaxGradValue);
	bool GetTwoLinesIntersection(kxPoint<int> pt1[2], kxPoint<int> pt2[2], kxPoint<int>& pt);
	int GetRotateOffTransform( const double srcQuad[2][2], const double dstQuad[2][2], double coeffs[2][3] );
protected:
	//参数版本1的读写
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;

public:
	bool Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nImgType, int nMode = _SEARCH_FOUR_EDGE, bool bModel = false, int bRoteCard = 0);

	int  Read( unsigned char *& point );
	int  Write( unsigned char *& point );
	bool Read( FILE* );
	bool Write( FILE* );
	int  Load( const char* lpszFile );
	int  Save( const char* lpszFile );


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

};

#endif
