#ifndef _KXDISTANCEMODULEHH
#define _KXDISTANCEMODULEHH

//距离检查模块
//2017/11/13
//add  by  lyl
//参数初始版本为：Distance2.0


#include "KxBaseFunction.h"
#include "kxDef.h"
#include "gxMath.h"
#include "KxGradientProcess.h"
#include <algorithm>

class CKxDistanceModule
{
public:
	CKxDistanceModule();
	~CKxDistanceModule();

	enum 
	{
		_SEARCH_VERTICAL_LINE = 0,            //搜垂直线
		_SEARCH_HORIZONTAL_LINE = 1,            //搜水平线	
		_SEARCH_IMAGE = 2,            //搜图文
		_MAX_MEASURE_GROUP = 32,           //最大测量组数
		_SEARCH_LEN = 3,
	};

//private:

#pragma pack(push, 1)

	struct MeasurePara
	{
		MeasurePara()
		{
			m_nSearchExtend = 30;
		}
		std::string    m_szName;                  //名称
		int            m_nIsOpen;                 //是否打开
		int            m_nRefSearchType;          //参考点搜索定位类型（搜垂直线/搜水平线/搜图文）
		kxRect<int>    m_rcRefInfo[2];            //参考点位置信息 /*(搜索框 + 定位框)*/
		int            m_nRefSearchDir;              //搜索方向
		int            m_nRefSearchThresh;           //搜索阈值

		int            m_nFixed;                  //参考点位置是否固定
		int            m_nMeasureSearchType;      //测量点搜索定位类型
		kxRect<int>    m_rcMeasureInfo[2];        //测量点搜索定位类型
		int            m_nMeasureSearchDir;              //搜索方向
		int            m_nMeasureSearchThresh;           //搜索阈值

		int            m_nMeasureDirection;       //测量方向(垂直/水平)
		int            m_nSearchExtend;           //搜索范围
		
		
		float          m_fStandardValue;          //标准值(mm)
		float          m_fErrorValue;             //误差值(mm)
	};

	struct Parameter
	{
		Parameter()
		{
			m_fActualWidth = 1;
			m_fActualHeight = 1;
			m_fProcessWidth = 1;
			m_fProcessHeight = 1;
			m_nGroup = 0;
			memset(m_hMeasure, 0, sizeof(MeasurePara)* _MAX_MEASURE_GROUP);
		}
		int                 m_nGroup;                 //距离测量组数（0-16）
		MeasurePara         m_hMeasure[_MAX_MEASURE_GROUP];
		//用于相机分辨率计算
		float               m_fActualWidth;      //标定物宽（mm）
		float               m_fActualHeight;     //标定物高
		float               m_fProcessWidth;     //处理图宽，对应多少像素
		float               m_fProcessHeight;    //处理图高，对应多少像素
	};

	struct SingleMeasureResult
	{
		std::string    m_szName;             //名字
		std::string    m_szStandardValue;    //标准值范围('6.52-7.58')
		float          m_fMeasureValue;      //测量值
		int            m_nIsOk;              //是否正常

		void clear()
		{
			m_fMeasureValue = -1.0f;
			m_nIsOk = 0;
		}
	};

	struct Result
	{
		Result()
		{
			memset(m_hMeasureResults, 0, sizeof(SingleMeasureResult)* _MAX_MEASURE_GROUP);
		}

		void clear()
		{
			m_nGroup = 0;
			memset(m_hMeasureResults, 0, sizeof(SingleMeasureResult)* _MAX_MEASURE_GROUP);
		}
		int                 m_nGroup;
		SingleMeasureResult m_hMeasureResults[_MAX_MEASURE_GROUP];
	};

#pragma pack(pop)

private:
	//参数
	Parameter             m_hParameter;
	Result                m_hResult;
	SingleMeasureResult   m_hSingleRes[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_Imgkern[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgBase;
	kxCImageBuf           m_TmpImg;
	CKxBaseFunction       m_hBaseFun[_MAX_MEASURE_GROUP];
	CKxGradientProcess    m_hGradient[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgCopy[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgGradientDir[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgMaxGradient[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgThreshGradient[_MAX_MEASURE_GROUP];

private:
	int PreAnalyseImage(const kxCImageBuf& SrcImg, kxRect<int>& rcSearch, int nSearchType, int nIndex, Ipp32f* pProject);
	int SingleMeasure(const kxCImageBuf& ImgCheck, const MeasurePara& para, int nIndex, SingleMeasureResult& res, KxCallStatus& hCall);
	int ComputeDistance(const kxCImageBuf& SrcImg, const kxCImageBuf& BaseImg, int nSearchType, int nSearchDir, int nThresh, const kxRect<int> rc[2], int nIndex, int& nDx, int& nDy, KxCallStatus& hCall);

public:
	//从xml中读取文件
	bool ReadXml(const char* filePath);
	bool ReadXmlinEnglish(const char* filePath);

	//检测距离标定
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);

	
	
	Parameter& GetParameter()
	{
		return m_hParameter;
	}

	Result& GetResult()
	{
		return m_hResult;
	}
};

#endif
