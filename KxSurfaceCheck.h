
//add by lyl 2016/12/27
//This class is a general surface inspect class.
//It use intel's thread building block to parallel computing,
//which speed up the algorithm.
//This class's the interface as:
//input is a image,
//output is the result is some blob analysis results.

#ifndef _KXSURFACECHECKH
#define _KXSURFACECHECKH

#include "kxDef.h"
#include "gxMath.h"
#include "KxCommonDef.h"            //常用的变量申明
#include "KxBaseFunction.h"         //比较常用的函数
#include "KxCalculateDistance.h"    //距离标定检测
#include "KxLightAdjust.h"          //光影校正算法
#include "KxWarpStretch.h"          //图像校正算法
#include "KxGradientProcess.h"      //梯度算法
#include "KxLearn.h"                //学习算法
#include "KxSliderCc.h"             //滑动残差算法
#include "KxImageMask.h"            //图像掩膜设置算法
#include "KxBlobAnalyse.h"          //改进后的连通域分析算法
#include "kxPolygonMask.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
#include "tinyxml2.h"
using namespace tbb;


class CKxSurfaceCheck
{
public:
	CKxSurfaceCheck();
	~CKxSurfaceCheck();

public:
	enum
	{
		_Valid_Layer = 2,
		_Max_Layer = 6,
		_CheckModeNum = 2,
		_Max_BlobCount = 4,
		_MAX_OFFSET = 10,
		_MAX_DEFECT_COUNT = 2,
	};
	//错误类型
	enum
	{
		_Check_Ok       = 0,
		_Check_Err      = 1,
		_Similarity_Err = 2,
		_FATAL_Err      = 3,
	};


#pragma pack(push, 1)
	//一些控制参数
	struct  Parameter
	{
		Parameter()
		{
			m_nIsLearn        = 0;
			m_nLineOffset     = 2;
			m_nGrayHighOffset = 5;
			m_nGrayLowOffset  = 10;

			m_nGrayLowSmallDots    = 100;
			m_nGrayLowSmallEnergy  = 100;

			m_nGrayHighSmallDots   = 100;
			m_nGrayHighSmallEnergy = 100;

			m_nGradientSmallDots   = 100;
			m_nGradientSmallEnergy = 100;
		}
		//定位参数
		int               m_nBlockIdx;            //块号
		kxRect<int>       m_rcCheck;              //检查区域
		kxRect<int>       m_rcKernel;       
		int               m_nExpand;            
		int               m_nSearchDir;
		int               m_nOffsetX;

		//检查参数
		int               m_nIsLearn;             //是否学习
		int               m_nLineOffset;          //检细线偏移
		int               m_nGrayHighOffset;      //检灰度高偏移
		int               m_nGrayLowOffset;       //检灰度低偏移

		//质量标准
		int               m_nGrayLowSmallDots;        //灰度低最小点数
		int               m_nGrayLowSmallEnergy;      //灰度低最小能量
		int               m_nGrayHighSmallDots;       //灰度高最小点数
		int               m_nGrayHighSmallEnergy;     //灰度高最小能量

		int               m_nGradientSmallDots;       //梯度最小点数
		int               m_nGradientSmallEnergy;     //梯度最小点数

		int               m_nIsFilterFakeDefect;     //是否去掉假缺陷

	};


	struct  Result
	{
		Result()
		{
			Clear();
		}
		void Clear()
		{
			nStatus = _Check_Ok;  //表示正常检查
			szErrInfo = "ok";
			nDefectCount = 0;
			mapFeaturelists.clear();
			mapFeaturelists.resize(_MAX_DEFECT_COUNT);
			for (int i = 0; i < _MAX_DEFECT_COUNT; i++)
			{
				mapFeaturelists[i].push_back(std::pair<std::string, float>("点数", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("能量", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("X坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("Y坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("缺陷宽", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("缺陷高", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("类型", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("块号", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("大图坐标X", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("大图坐标Y", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("块宽", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("块高", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("X原坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("Y原坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("块X坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("块Y坐标", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("POSY(mm)", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("POSX(mm)", 12222.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("行号", 0.0f));

			}
		}
		int                                        nStatus;    //检查状态
		std::string                                szErrInfo;       //备注信息
		int                                        nDefectCount;    //缺陷个数
		std::vector<std::vector<std::pair<std::string, float>>>  mapFeaturelists; //每个缺陷的特征列表
	};



#pragma pack(pop)

public:
	kxCImageBuf              m_ImgKernel;     //定位核图像
	kxCImageBuf              m_ImgBigKernel;

public:
	kxCImageBuf              m_ImgTemplate[_Max_Layer + 1];  //模板+一副合成模板
	kxCImageBuf              m_ImgCompose;
	kxCImageBuf              m_ImgLearnTemplate[_Max_Layer + 1];  //局部学习模板+一副合成模板

public:
	CKxBaseFunction          m_hFun;
	CKxBaseFunction          m_hBaseFun[_Max_Layer];
	CKxWarpStretch           m_hWarpStrech;
	CKxLightAdjust           m_hLightAdjust;
	CKxGradientProcess       m_hGradient[_Max_Layer];
	CKxLearn                 m_hLearn[_Max_Layer];  //四个学习对象对应四副处理图
	CKxSliderCc              m_hSlider[_Max_Layer];
	CKxImageMask             m_hImgMask;            //图像掩膜
	CkxPolygonMask		     m_hPolygonMask;					//多边形掩膜
	CKxBlobAnalyse           m_hBlobAnaly;          //Blob分析
	CKxBlobAnalyse           m_hGradBlobAnaly;
	Parameter                m_hParameter;
	kxCImageBuf              m_BaseImg; //建模时用的底板，用于光影校正、自动掩膜

	int                      m_nModelNumber;                 //模板张数
	Result                   m_hResult;                      //结果

	int                      m_nLearnPage;                     //学习张数
	bool                     m_bIsLearnCompleted[_Max_Layer];  //学习是否刚完成
	bool                     m_bAllCompleted;

	int                      m_nAlgorithmStatus[_Max_Layer];               //8个层面检查打开状态
	int                      m_nLearnStatus;

	int                      m_nLargeSize;
	int                      m_nSpeckleMaxSize;                 //噪点最大大小

	int                      m_nHighOffset;
	int                      m_nLowOffset;

	int                      m_nDx, m_nDy;
	int                      m_nIsLoadParamsSuccess;
	int                      m_nStandard[3];
	int                      m_nIsStandardChange;
	int                      m_nResizeFactorX, m_nResizeFactorY;

	int                      m_nBestLocationLayer;

	int                      m_nGrayModelSize; //

	int                      m_nIsBinary;
	int                      m_nThreshold;  //二值化阈值

	int                      m_nLowTemplateThreshold; //低模板二值化
	int                      m_nSlider;
protected:
	//处理申请的临时图像空间
	kxCImageBuf      m_OriginalImg;           //原始图像
	kxCImageBuf      m_WarpImg;               //精确校正后的图像
	kxCImageBuf      m_ExtendWarpImg;         //扩通边缘图像
	kxCImageBuf      m_ExtendWarpImg1;        //边界填0
	kxCImageBuf      m_MidImg[_Max_Layer];                //中间结果图像
	kxCImageBuf      m_ImgCheck[_Max_Layer];  //6张处理图像,梯黑/梯白对应90,0,45 135方向图像， 全梯2张
	kxCImageBuf      m_ImgSlidCc[_Max_Layer];
	kxCImageBuf      m_ImgSliderNorm[_Max_Layer];
	kxCImageBuf      m_ImgSlidCcMax[_Max_Layer];
	kxCImageBuf      m_ImgOffset[_Max_Layer];
	kxCImageBuf      m_ResImg[_Max_Layer];
	kxCImageBuf      m_ImgCcTemp;
	kxCImageBuf      m_ColorCheckImg;

	//2018/11/1 用于去掉FPC假短路
	kxCImageBuf      m_ImgAnalyse;
	kxCImageBuf      m_ImgAnalyseBinary;
	kxCImageBuf      m_ImgAnalyseCc, m_ImgAnalyseCc1;
	kxCImageBuf      m_ImgAnalyseCcBinary;
	kxCImageBuf      m_ImgSubCc;
	kxCImageBuf      m_ImgAnalyseCcDilate;


	//kxCImageBuf      m_TmpImg[_SAMLLDOTS_INDEX_END];
	//kxCImageBuf      m_TmpImg1[_SAMLLDOTS_INDEX_END];
	kxCImageBuf      m_ImgTemp;               //
	kxCImageBuf      m_ImgCc;                 //合并残差图(灰度图)

	kxCImageBuf      m_StdImg;                //色差检查标准
	kxCImageBuf      m_TestImg;               //色差检查被测试图
	kxCImageBuf      m_CreateMask;

	//与生成彩色残差图显示有关变量
	kxCImageBuf      m_ColorImg;             //彩色图像
	kxCImageBuf      m_ResizeImg;            //图像
	kxCImageBuf      m_ResidImg[4];          //红色\绿色\蓝色残差图
	kxCImageBuf      m_CompositionImg[3];    //红色\绿色\蓝色合成残差图
	kxCImageBuf      m_AddImage;
	kxCImageBuf      m_BinaryImg;
	kxCImageBuf      m_DupImg;
	kxCImageBuf      m_ShowImg;
	kxCImageBuf      m_ResizeImg1;
	//单缺陷学习
	kxCImageBuf      m_ImgSub[_Max_Layer];

	kxCImageBuf      m_ImgTransform[_Max_Layer];
	kxCImageBuf      m_ImgLineMax;
	kxCImageBuf      m_ImgResizeSuper;
	kxCImageBuf      m_ImgSearch;
	kxCImageBuf      m_WarpResizeImg, m_WarpResizeImg1;
	kxCImageBuf      m_ImgCropBig;
	kxCImageBuf      m_ReferImg;
	kxCImageBuf      m_ImgDefect;
	kxCImageBuf      m_ImgTransfer;
	kxCImageBuf      m_ImgSrc;
	kxCImageBuf      m_ImgWarpGray;
protected:
	int GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg, kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy);
	int  CopyEdges(const kxCImageBuf& SrcImg, const kxCImageBuf CopyImg, kxCImageBuf& DstImg,
		kxRect<int>& rcEdge, kxRect<int>& rcCopy);
	int AnalysisCheckStatus(int nImageType);
	int AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg);
	int SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate);
	int SaveMidImage(bool bSave);
	int SetCheckExceptionStatus(kxCImageBuf& ImgCheck); //出现异常、判为废品
	void convertcolorimage2gray(kxCImageBuf& img, kxCImageBuf& grayimg, int nlayer);
	void GetBestLocationLayer(kxCImageBuf& img, int& nLayer);
public:

	bool ReadXml(tinyxml2::XMLDocument& xmlHandle, int nCameraId, int nIdx, const kxCImageBuf& BaseImg);
	bool ReadParaXml(tinyxml2::XMLDocument& xmlHandle, int nCameraId, int nIdx, const kxCImageBuf& BaseImg);

	int ReadImg(FILE* fp);
	int WriteImg(FILE* fp);

	//保存模板图像
	int SaveTemplateImg(const char* lpszFile);
	//读取模板图像
	int LoadTemplateImg(const char* lpszFile);


	//获取检测结果
	Result&  GetCheckResult() 
	{
		return m_hResult;
	}
	//设置学习张数
	void SetLearnPage(int nLearn)
	{
		m_nLearnPage = nLearn;
	}

	//设置滑动大小
	void SetSliderRange(int nSlider)
	{
		m_nSlider = nSlider;
	}

	//获得学习的状态
	bool IsLearn()
	{
		return  m_hParameter.m_nIsLearn > 0 ? true : false;;
	}

	//设置压缩系数
	void  SetResizefactor(int nResizeX, int nResizeY)
	{
		m_nResizeFactorX = nResizeX;
		m_nResizeFactorY = nResizeY;
	}

	//设置低模板二值化
	void SetLowTemplateThreshold(Ipp8u nThresh)
	{
		m_nLowTemplateThreshold = (int)nThresh;
	}


	int Check(const kxCImageBuf& ImgCheck, kxCImageBuf& DstImg, KxCallStatus& hCall);


	int  ProcessSurfaceWarpImages(KxCallStatus& hCall);
	int  LearnTemplate(KxCallStatus& hCall);
	int  ResidualsAnalys(KxCallStatus& hCall);
	int  ConnectAnalysis(const kxCImageBuf& SrcImg, kxCImageBuf& DefectImg, KxCallStatus& hCall);

	//获取参数
	Parameter& GetParameter()
	{
		return m_hParameter;
	}
	//是否刚学习完，用于模块外调用，判定学习完成，做一系列的操作
	bool IsLearnCompletedStatus() const
	{
		return m_bAllCompleted;
	}
	void SetLearnCompletedStatus(bool bStatus)
	{
		m_bAllCompleted = bStatus;
	}

	//设置单向梯度预处理变大尺度
	void SetLargeSize(int nLargeSize)
	{
		m_nLargeSize = nLargeSize;
	}

	//设置最大噪点尺寸
	void SetSpeckleMaxSize(int nSpeckleMaxSize)
	{
		m_nSpeckleMaxSize = nSpeckleMaxSize;
	}

	//设置质量标准
	void SetCheckStandard(int* pStandard, int n)
	{
		assert(n == 3);
		memcpy(m_nStandard, pStandard, sizeof(int) * 3);
		m_nIsStandardChange = true;
	}

	//用大开水平偏移校正图像
	void SetOffsetX(int noffset)
	{
		m_nBigImgOffsetX = noffset;
	}

	void Setonlinestatus(int nstatus)
	{
		m_hParameter.m_nIsLearn = nstatus; 
		m_nLearnStatus = 1;
	}

	void SetLearnModeSize(int nModelsz)
	{
		m_nGrayModelSize = nModelsz;
	}


	//进行二值化预处理，用于检查铜箔
	void SetIsBinary(int nIsBinary, int nThreshold)
	{
		m_nIsBinary = nIsBinary;
		m_nThreshold = nThreshold;
	}

	//设置当时图起始行的米数
	void SetStartlines(__int64 nStartline)
	{
		m_nStartline = nStartline;
	}

	void SetCameraResolution(float nCameraXResolution, float nCameraYResolution)
	{
		m_nCameraYResolution = nCameraYResolution;
		m_nCameraXResolution = nCameraXResolution;

	}

	void SetOriginalX(int& nOriginalX)
	{
		m_nOriginalX = nOriginalX;
	}

	//在线局部学习
	int SetDefectLearnStatus(const kxCImageBuf& srcImg, kxRect<int>& rcDefect, std::string szModelDir);

	//根据稿件生成模板
	bool SetTemplatesFiles(const kxCImageBuf& srcImg, kxRect<int>& rcCheckArea, int nModelSize);

	//在线学习需要的图像
	protected:
		kxCImageBuf m_ImgOnlineDefectLearnGray;
		kxCImageBuf m_ImgOnlineDefectLearnGradient[4];
		int         m_nIsDefectLearn;
		CKxBaseFunction m_hDefectLearnFun;
		CKxGradientProcess m_hDefectLearnGradient;
		kxRect<int> m_rcDefectGray;
		kxRect<int> m_rcDefectGradient;
		int         m_nBigImgOffsetX;

	//根据软件算出的米数，定位缺陷
		__int64         m_nStartline;
		float       m_nCameraYResolution, m_nCameraXResolution;  //相机分辨率
		int         m_nOriginalX;   //横向原点坐标x


};

#endif
