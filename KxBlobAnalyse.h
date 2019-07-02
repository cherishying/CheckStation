#ifndef _KXBLOBANALYSEHHHH
#define _KXBLOBANALYSEHHHH
//////////////////////////////////////////////////////////////////////////
// Add by lyl 2016/3/2
// In order to analysis Image with connected component analysis
// To speeds up the original Blob algorithm

#include "ippch.h"
#include "ippi.h"
#include "ipps.h"
#include "ippcv.h"
#include "kxDef.h"
#include "gxMath.h"
#include "KxMinRectangle.h"
#include "KxBaseFunction.h"


class CKxBlobAnalyse
{
public:
	CKxBlobAnalyse();
	~CKxBlobAnalyse();
public:
	enum
	{
		_USE4 = 2,      //Con_4
		_USE8 = 4,      //Con_8
		_Min_Lable = 300,
		_Max_FloodBlob_Count = 200,
		_MAX_Lable_Count = 50000,
		_NORM_WIDTH = 100,
		_NORM_HEIGHT = 100,

		_SORT_BYDOTS       = 0,  //按点数排序
		_SORT_BYENERGY     = 1,  //按能量排序
		_SORT_BYSIZE       = 2,  //按尺寸（点数、能量联合）排序
		_DEFAULT_MAX_COUNT = 4,  //默认最大排序个数

		
	};

	struct CGridD            //Grid struct
	{
		kxRect<int> m_rc;
		int  m_nDots;   
		int  m_nPower;   
	};


	struct SingleBlobInfo
	{
		SingleBlobInfo()
		{
			m_nDots = 0;
			m_nEnergy = 0;
			m_nSize = 0;
			m_rc.setup(0,0,0,0);
			m_nLabel = 0;

			memset(m_Pt, 0, sizeof(kxPoint<int>)*4);
			m_fAngle = 0.0;
			m_fMinArea = 0;
			m_nMinRectWidth = 0;
			m_nMinRectHeight = 0;
			m_fRatio = 0.0;
			m_nCircumference = 0;
			m_fSumEnergy = 0.0;

		}
		int          m_nDots;
		int          m_nEnergy;
		int          m_nSize;  //dot and energy 
		kxRect<int>  m_rc;
		int          m_nLabel;
		kxPoint<int> m_PtSeed; //seed point
		//about some smallest rectangle's info
		kxPoint<int>   m_Pt[4];
		float          m_fAngle;
		int            m_fMinArea;
		int            m_nMinRectWidth;
		int            m_nMinRectHeight;
		float          m_fRatio;  //aspect ratio
		int            m_nCircumference; //circumference
		double         m_fSumEnergy;
	};



private:
	int				 m_nCount;    //Blob count
	SingleBlobInfo*  m_pBlobInfo; //All blobs' info
	SingleBlobInfo*  m_pSortBlobInfo; //After sort by dots 
	kxRect<int>*     m_pBlobArea;
	kxImg16u         m_pImg16u;   //Temporary space
	kxImg16u         m_pTmpImg;
	kxImg16u         m_ImgCopy16u;
	kxImg16u         m_Img16u;

	kxCImageBuf      m_LabelImg;

	kxCImageBuf      m_BufferImg;
	kxCImageBuf      m_BufferImgX;
	kxCImageBuf      m_Buf;
	kxCImageBuf      m_TmpImg;
	kxCImageBuf      m_OpenImg;
	kxCImageBuf      m_MaskImg;
	kxCImageBuf      m_PreImg;
	kxCImageBuf      m_Img;

	kxCImageBuf      m_SrcImg;
	kxCImageBuf      m_DstImg;
	
	kxRect<int>      m_rcCheck;
	CKxMinRect       m_hKxMinRect;
	CKxBaseFunction  m_hBaseFun;

	kxImg16u      m_ImgFilter16u;

	//Parameter about merge some blobs
	int              m_nOpenSize;
	int              m_nMinDots;
    //whether open the status to compute the blob additional information
	bool             m_bOpenAdditionalInfo;

	//Parameter about Grid Blob
	bool             m_bOpenGrid;
	CGridD*          m_pGrid;
	int              m_nGridX;
	int              m_nGridY;
	int              m_nGridXStep;
	int              m_nGridYStep;
	int              m_nCountBlob;

	double           m_nEnergySum;

	//
	bool             m_bUseFoodFillAlogrithm;
	int              m_nConnectType; //

protected:
	void Clear();
	void ClearSortInfo();
	int  MergeSomeConnections(const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDst, int nDstPitch, KxCallStatus& hCall);
	int  ToSimpBlob( const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob, KxCallStatus& hCall );
    int  ToGridBlob( const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob, KxCallStatus& hCall);
    int  UseFloodAlgorithmComputeBlob(kxImg16u& Img16u, int nCount, KxCallStatus& hCall );
	void SetGridXY( int nGridX, int nGridY );
	void InitGrid( const kxRect<int>& rc );
	int ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo, KxCallStatus& hCall );
	int ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo);

public:
	int ToBlob( const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob, KxCallStatus& hCall);
	int ToBlob(const unsigned char* buf, int nPitch, const kxRect<int>& rcBlob);


	int ToBlob( const unsigned char* buf, int nWidth, int nHeight, int nPitch, KxCallStatus& hCall);
	int ToBlob(const unsigned char* buf, int nWidth, int nHeight, int nPitch);

	int ToBlob(const kxCImageBuf& SrcImg, KxCallStatus& hCall);
	int ToBlob(const kxCImageBuf& SrcImg);
	int ToBlobParallel(const kxCImageBuf& SrcImg, int nSortByMode, int nMaxSortDots, int nMergeSize, int nOpenComputeAdanceFeatures, KxCallStatus& hCall);
	int ToBlobParallel(const kxCImageBuf& SrcImg, int nSortByMode = _SORT_BYDOTS, int nMaxSortDots = _DEFAULT_MAX_COUNT, int nMergeSize = 1, int nOpenComputeAdanceFeatures = 1);

	//挑选连通域最大的区域，其他区域都去掉
	int SelectMaxRegionByDots(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int SelectMaxRegionByDots(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	//按点数、能量、宽、高挑选满足条件的连通域
	//szType 可取值为 "Dots","Energy","Width","Height"
	int SelectRegion(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, std::string szType, int nMinRange, int nMaxRange, KxCallStatus& hCall);
	int SelectRegion(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, std::string szType, int nMinRange, int nMaxRange);


	int SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus & hCall);
	int SortByDot(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall);
	int SortByDots(int nSortCount,   int nOpenComputeAdanceFeatures, KxCallStatus& hCall);
	int SortBySize(int nSortCount,   int nOpenComputeAdanceFeatures, KxCallStatus& hCall);

	int SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures = 0);
	int SortByDots(int nSortCount, int nOpenComputeAdanceFeatures = 0);
	int SortBySize(int nSortCount, int nOpenComputeAdanceFeatures = 0);

	//返回只有对应label的连通域，在做完连通域分析后调研
	void GetBlobImage(int nLabel, kxRect<int> rc, kxCImageBuf& blobimg);


	void SetMergeSize(int nMergeSize)
	{
		m_nOpenSize = nMergeSize;
	}
	void SetGridXYStep( int nStepX, int nStepY )
	{
		m_nGridXStep = nStepX;
		m_nGridYStep = nStepY;
	}
	void SetConnectionType(int nConType)
	{
		m_nConnectType = nConType;
	}

	void SetMinDot(int nMinDot)
	{
		m_nMinDots = nMinDot;
	}

	int GetBlobCount() const 
	{ 
		return m_nCountBlob;  //Get blob count
	}
	SingleBlobInfo& GetSingleBlob( int nIndex )
	{
		assert( nIndex >=0 && nIndex < m_nCountBlob );
		return m_pBlobInfo[nIndex];
	}

	SingleBlobInfo& GetSortSingleBlob( int nIndex )
	{
		assert( nIndex >=0 && nIndex < m_nCountBlob );
		return m_pSortBlobInfo[nIndex];
	}



};

#endif