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
//#include "kxDef.h"
//#include "gxcv::Math.h"
#include "KxMinRectangle.h"
#include "KxBaseFunction.h"
#include "opencv2/opencv.hpp"

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

	enum
	{
		_PARALLEL_SEGMENTATION_H = 1024,	//启用并行blob的最小行数
		_MAX_OMP_THREAD = 5,				//最多并行使用这么多MP线程

	};

	struct CGridD            //Grid struct
	{
		Rect m_rc;
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
			m_rc = cv::Rect(0, 0, 0, 0);
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
		Rect	     m_rc;
		int          m_nLabel;
		Point        m_PtSeed; //seed point
		//about some smallest rectangle's info
		Point	     m_Pt[4];
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
	cv::Rect*			 m_pBlobArea;
	cv::Mat              m_pImg16u;   //Temporary space
	cv::Mat	             m_pTmpImg;
	cv::Mat              m_ImgCopy16u;
	cv::Mat              m_Img16u;

	cv::Mat		         m_LabelImg;

	cv::Mat				 m_BufferImg;
	cv::Mat				 m_BufferImgX;
	cv::Mat				 m_Buf;
	cv::Mat				 m_TmpImg;
	cv::Mat				 m_OpenImg;
	cv::Mat			     m_MaskImg;
	cv::Mat		         m_PreImg;
	cv::Mat			     m_Img;

	cv::Mat				 m_SrcImg;
	cv::Mat				 m_DstImg;
	
	cv::Rect      m_rcCheck;
	CKxMinRect       m_hKxMinRect;
	CKxBaseFunction  m_hBaseFun;

	cv::Mat		         m_ImgFilter16u;

	//Parameter about merge some blobs
	int              m_nOpenSize;
	int              m_nMinDots;
    //whether open the status to compute the blob additional inforcv::Mation
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
	int  ToSimpBlob( const unsigned char* buf, int nPitch, const Rect& rcBlob, KxCallStatus& hCall );
    int  ToGridBlob( const unsigned char* buf, int nPitch, const Rect& rcBlob, KxCallStatus& hCall);
    int  UseFloodAlgorithmComputeBlob(cv::Mat& Img16u, int nCount, KxCallStatus& hCall );
	void SetGridXY( int nGridX, int nGridY );
	void InitGrid( const cv::Rect& rc );
	int ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo, KxCallStatus& hCall );
	int ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo);

public:
	int ToBlob( const unsigned char* buf, int nPitch, const cv::Rect& rcBlob, KxCallStatus& hCall);
	int ToBlob(const unsigned char* buf, int nPitch, const cv::Rect& rcBlob);


	int ToBlob( const unsigned char* buf, int nWidth, int nHeight, int nPitch, KxCallStatus& hCall);
	int ToBlob(const unsigned char* buf, int nWidth, int nHeight, int nPitch);

	int ToBlob(InputArray SrcImg, KxCallStatus& hCall);
	int ToBlob(InputArray SrcImg);
	int ToBlobParallel(InputArray SrcImg, int nSortByMode, int nMaxSortDots, int nMergeSize, int nOpenComputeAdanceFeatures, KxCallStatus& hCall);
	int ToBlobParallel(InputArray SrcImg, int nSortByMode = _SORT_BYDOTS, int nMaxSortDots = _DEFAULT_MAX_COUNT, int nMergeSize = 1, int nOpenComputeAdanceFeatures = 1);

	//挑选连通域最大的区域，其他区域都去掉
	int SelectMaxRegionByDots(InputArray SrcImg, cv::OutputArray DstImg, KxCallStatus& hCall);
	int SelectMaxRegionByDots(InputArray SrcImg, cv::OutputArray DstImg);

	//按点数、能量、宽、高挑选满足条件的连通域
	//szType 可取值为 "Dots","Energy","Width","Height"
	int SelectRegion(InputArray SrcImg, cv::OutputArray DstImg, std::string szType, int nMinRange, int nMaxRange, KxCallStatus& hCall);
	int SelectRegion(InputArray SrcImg, cv::OutputArray DstImg, std::string szType, int nMinRange, int nMaxRange);


	int SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus & hCall);
	int SortByDot(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall);
	int SortByDots(int nSortCount,   int nOpenComputeAdanceFeatures, KxCallStatus& hCall);
	int SortBySize(int nSortCount,   int nOpenComputeAdanceFeatures, KxCallStatus& hCall);

	int SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures = 0);
	int SortByDots(int nSortCount, int nOpenComputeAdanceFeatures = 0);
	int SortBySize(int nSortCount, int nOpenComputeAdanceFeatures = 0);

	//返回只有对应label的连通域，在做完连通域分析后调研
	void GetBlobImage(int nLabel, cv::Rect rc, cv::OutputArray blobimg);



	// ------- 2019.07.03 new ------------//
	// 并行blob，输入的SrcImg行数需为3、4、5的公倍数
	void BlobParallel(InputArray SrcImg, int nSortByMode = _SORT_BYDOTS, int nMaxSortDots = _DEFAULT_MAX_COUNT, int nMergeSize = 1);


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