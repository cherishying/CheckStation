#ifndef _KXSIGNATURECHECKBETAH
#define _KXSIGNATURECHECKBETAH

//add by lyl 2017/11/16
//据烫印检查的需要，签名条检测模块升级版本


#include "kxDef.h"
#include "gxMath.h"
#include "KxCommonDef.h"            //常用的变量申明
#include "KxBaseFunction.h"         //比较常用的函数
#include "KxCalculateDistance.h"    //距离标定检测
#include "KxLightAdjust.h"          //光影校正算法
#include "KxWarpStretch.h"          //图像校正算法
#include "KxColorDiff.h"            //色差检测
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
using namespace tbb;

#define SUBSECTION 3

class CKxSignatureCheckBeta
{
public:
	CKxSignatureCheckBeta();
	~CKxSignatureCheckBeta();

	enum
	{
		_Valid_Layer = 2,
		_Max_Layer = 8,


		_Check_Layer = 3,
		_Max_BlobCount = 8,

		_Edge_Extend = 10,
		_MAX_OFFSET = 10,
	};

	enum
	{
		_Max_Point = 8				//最大定位点数
	};

	//错误类型
	enum
	{
		_Check_Ok = 0,
		_Check_Err = 1,
		_ColorDiff_Err = 2,
		_Distance_Err = 3,
		_Barcode2D_Err = 4,
		_Barcode_Err = 5,
		_Number_Err = 6,
		_Similarity_Err = 7,
		_SearchEdge_Err = 8,
		_BlockImage_Err = 9,
		_Identify_Check = 10,
	};


#pragma pack(push, 1)
	//一些控制参数
	struct  Parameter
	{
		Parameter()
		{
			for (int i = 0; i < _Max_Point; i++)
				m_Point[i].setup(0, 0);
			m_nPointCount = 0;
			m_nSearchEdgeThresh = 30;
			m_nWhiteAreaCheckThresh = 60;
		}
		kxPoint<int>	  m_Point[_Max_Point];	  //定位点坐标
		int			      m_nPointCount;	      //定位点个数
		int               m_nAreaNumber;          //区域号
		int               m_nIsLearn;             //是否学习
		int               m_nSmallDotOffset;      //检小点偏移
		int               m_nLossOffset;          //检缺失偏移
		int               m_nIsOpenWhiteArea;     //是否打开白条检查
		kxRect<int>       m_rcWhiteArea;          //白条检查区域
		int               m_nWhiteAreaCheckThresh; //白条检查阈值
		int               m_nSearchEdgeThresh;    //搜边阈值
	};

	struct Result
	{
		Result()
		{
			memset(m_hBlobInfo, 0, sizeof(BlobInfo)*_Max_BlobCount*4);
			m_nCount = 0;
			m_nStatus = _Check_Ok;
			m_nKernIndex = -1;
			m_nDistanceX = 0;
			m_nDistanceY = 0;
			m_nColorDiff = 0;
		}

		struct BlobInfo
		{
			int                 m_nAreaType;   //区域号
			int                 m_nDots;
			int                 m_nEnergy;
			int                 m_nLeft;
			int                 m_nTop;
			int                 m_nBlobWidth;
			int                 m_nBlobHeight;
			int                 m_nWHRatio;
			int                 m_nAvgEnergy;
			int                 m_nAvgArea;
			int                 m_nMinRectWidth;
			int                 m_nMinRectHeight;
			kxCImageBuf         m_ErrImg;
		};

		BlobInfo      m_hBlobInfo[_Max_BlobCount * 4];   //缺陷特征
		int           m_nCount;                      //实际缺陷个数
		int           m_nStatus;                     //结果状态信息
		int           m_nKernIndex;                  //相似度错误定位核标号
		int           m_nDistanceX;                   //距离错误返回实际距离
		int           m_nDistanceY;
		int           m_nColorDiff;                  //色差

		void Clear()
		{
			m_nCount = 0;
			m_nStatus = _Check_Ok;
			m_nKernIndex = -1;
			m_nDistanceX = 0;
			m_nDistanceY = 0;
			m_nColorDiff = 0;
			for (int k = 0; k < _Max_BlobCount * 4; k++)
			{
				m_hBlobInfo[k].m_ErrImg.Release();
			}
		}
	};

#pragma pack(pop)

public:
	kxCImageBuf			m_ImgTemplate[_Max_Layer + 1];	//模板
	kxCImageBuf         m_ImgLearnTemplate[_Max_Layer + 1];  //局部学习模板+一副合成模板

protected:

	CKxBaseFunction		m_hBaseFun;
	CKxBaseFunction     m_hFun[_Max_Layer];
	CKxCalDistance		m_hCalDistance;					//距离检测
	CKxWarpStretch		m_hWarpStrech;
	CKxColorDiff        m_hColorDiff;          //色差检查
	CKxGradientProcess	m_hGradient;
	CKxLearn			m_hLearn[_Max_Layer];			//四个学习对象对应四副处理图
	CKxSliderCc			m_hSlider[_Max_Layer];
	CKxImageMask		m_hImgMask;						//图像掩膜
	CkxPolygonMask		m_hPolygonMask;					//多边形掩膜
	CKxBlobAnalyse		m_hBlobAnaly;					//Blob分析

	Parameter			m_hParameter;
	kxCImageBuf			m_BaseImg;						//建模时用的底板，用于光影校正、自动掩膜
	int					m_nModelNumber;					//模板张数
	Result				m_hResult;						//结果

	kxPoint<int>		m_Offset[_Max_Point];			//相对偏移值
	int					m_nOffsetCount;					//相对偏移个数
	kxPoint<int>		m_OffsetMin;					//最小偏移
	kxPoint<int>		m_OffsetMax;					//最大偏移

	kxRect<int>			m_rcMattingPos;					//签名条搜边偏移
	kxRect<int>         m_rcSignaturePos;               //签名条在卡面的位置，用于定位
	kxRect<int>         m_rcBaseSignaturePos;          //底板签名条的位置

protected:
	//处理申请的临时图像空间
	kxCImageBuf			m_OriginalImg;					//原始图像
	kxCImageBuf			m_WarpImg;						//精确校正后的图像

	kxCImageBuf			m_ImgCheck[_Max_Layer];			//处理图像,梯黑/梯白对应90,0,45 135方向图像
	kxCImageBuf         m_ImgLearn[_Max_Layer];
	kxCImageBuf			m_ImgSlidCc[_Max_Layer];
	kxCImageBuf			m_ImgOffset[_Max_Layer];
	kxCImageBuf         m_ImgSlidCcMax[_Max_Layer];
	kxCImageBuf         m_ImgCompose[_Max_Layer];
	kxCImageBuf         m_ImgOpen[_Max_Layer];
	kxCImageBuf         m_ImgFilterEdge[_Max_Layer];


	kxCImageBuf			m_ImgTemp;						//
	kxCImageBuf			m_ImgCc;						//合并残差图(灰度图)

	//kxCImageBuf			m_StdImg;					//色差检查标准
	//kxCImageBuf			m_TestImg;						//色差检查被测试图
	//kxCImageBuf			m_ColorCheckImg;
	kxCImageBuf			m_CreateMask;

	kxCImageBuf         m_EdgeImg;
	kxCImageBuf			m_ImageTemp;					//处理中间图像
	kxCImageBuf         m_ImgGradient;                  //搜边梯度图像

	kxCImageBuf         m_ImgGradient0;
	kxCImageBuf         m_ImgGradient1;
	kxCImageBuf         m_ImgSignature;                 //签名条区域
	kxCImageBuf         m_ImgCreateTemp;
	kxCImageBuf         m_ImgCreateLocation;
	kxCImageBuf         m_ImgSingleLocation[SUBSECTION];
	kxCImageBuf         m_ImgCreateGradient[_Max_Layer];
	kxCImageBuf         m_ImgCreateGradientTmp;
	kxCImageBuf         m_ImgCreateBase[_Max_Layer];
	kxCImageBuf         m_ImgSingleBase[_Max_Layer];

	kxCImageBuf         m_ImgLocationSmall[SUBSECTION];
	kxCImageBuf         m_ImgLocationBig[SUBSECTION];
	kxCImageBuf         m_ImgSplitSearch[SUBSECTION];
	//与生成彩色残差图显示有关变量
	kxCImageBuf         m_ColorImg;             //彩色图像
	kxCImageBuf         m_ResizeImg;            //图像
	kxCImageBuf         m_ResidImg[3];          //红色\绿色\蓝色残差图
	kxCImageBuf         m_CompositionImg[3];    //红色\绿色\蓝色合成残差图
	kxCImageBuf         m_AddImage;
	kxCImageBuf         m_BinaryImg;
	kxCImageBuf         m_DupImg;
	kxCImageBuf         m_ShowImg;
	kxCImageBuf         m_ResizeImg1;

	kxCImageBuf			m_ImageEdge;					//边缘图像
	kxCImageBuf         m_ImgEdgeTemp;
	kxCImageBuf			m_ImageCc;						//残差图像
	kxCImageBuf			m_ImageLocation[SUBSECTION];	//搜边中间图像

	kxCImageBuf			m_ImageLocationTmp;
	kxCImageBuf         m_ImagePosSignature;

	kxCImageBuf         m_ImgAverage;
	kxCImageBuf         m_ImgGradientPos[2];
	kxCImageBuf         m_ImgGradientMaxPos[2];
	kxCImageBuf         m_ImgThreshPos[2];
	kxCImageBuf         m_ImgFilterSpeckle[2];
	kxCImageBuf         m_ImgClosePos[2];
	kxCImageBuf         m_ImgWhiteArea;
	kxCImageBuf         m_ImgPos;

	int                 m_nLearnHighLowMode[_Max_Layer];
	kxRect<int>			m_rcLocationPos[SUBSECTION];	//签名条定位偏移
	kxRect<int>         m_rcSplitSmall[SUBSECTION];

	int                 m_nLearnPage;                     //学习张数
	bool                m_bIsLearnCompleted[_Max_Layer];  //学习是否刚完成
	bool                m_bAllCompleted;

	int                 m_nAlgorithmStatus[_Max_Layer];               //8个层面检查打开状态
	int                 m_nLearnStatus;

	int                m_nSignatureEdgeThresh;
	int                m_nSearchEdgeThresh;
	int                m_nSmoothSize;                //单向梯度平滑尺寸

	int                m_nSignatureModelWidth;            //签名条底板高
	int                m_nSignatureModelHeight;


	int                m_nSignatureWidth;            //签名条高
	int                m_nSignatureHeight;
	int                m_nSignatureSizeThresh;

	int                m_nSignatureExtendLen;   //签名条搜边缩进长度

	int                m_nSignatureBorderCheckThresh;

	int                m_nIsCreateBaseImg;      //是否创建底板
	int                m_nUseXml;
	kxRect<int>        m_rcWhiteArea;
	kxRect<int>        m_rcSignatureArea;

	int                m_nOpenVerticalSize;

protected:
	int ReadImg(FILE* fp);
	int WriteImg(FILE* fp);

	int Matting(const kxCImageBuf& SrcImg, bool bModel, KxCallStatus& hCall);
	int Matting(const kxCImageBuf& SrcImg, bool bModel = false);
	int LearnTemplate(KxCallStatus& hCall);
	int ResidualsAnalys(KxCallStatus& hCall);
	int ConnectAnalysis(const kxCImageBuf& SrcImg, KxCallStatus& hCall);
	int SaveMidImage(bool bSave);
	int AnalysisCheckStatus();
	int SetCheckExceptionStatus(const kxCImageBuf& ImgCheck);
	int SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate);
	int AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg);
public:



	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadParaXml(const char* filePath);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadParaXmlinEnglish(const char* filePath);

	//保存模板图像
	int SaveTemplateImg(const char* lpszFile);
	//读取模板图像
	int LoadTemplateImg(const char* lpszFile);

	//局部学习
	int SaveLearnTemplateImg(const char* lpszFile);
	int WriteLocalLearnImg(FILE* fp);
	int LoadLearnTemplateImg(const char* lpszFile);
	int ReadLocalLearnImg(FILE* fp);


	//获取检测结果
	const Result&  GetCheckResult() const
	{
		return m_hResult;
	}
	//设置学习张数
	void SetLearnPage(int nLearn)
	{
		m_nLearnPage = nLearn;
	}

	void SetEdgeCheckThresh(int nThresh)
	{
		m_nSignatureEdgeThresh = nThresh;
	}

	void SetSignatureBorderCheckThresh(int nThresh)
	{
		m_nSignatureBorderCheckThresh = nThresh;
	}

	void SetSmoothSize(int nSize)
	{
		m_nSmoothSize = nSize;
	}

	void SetSearchEdgeThresh(int nThresh)
	{
		m_nSearchEdgeThresh = nThresh;
	}

	void SetSignatureSizeThresh(int nThresh)
	{
		m_nSignatureSizeThresh = nThresh;
	}

	void SetSignatureExtendLen(int nLen)
	{
		m_nSignatureExtendLen = nLen;
	}

	//设置残差开操作
	void SetCcVerticalOpenSize(int nOpensize = 1)
	{
		m_nOpenVerticalSize = nOpensize;
	}
	//获得学习的状态
	bool IsLearn()
	{
		return m_hParameter.m_nIsLearn > 0 ? true : false;
	}

	//通用检测
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);
	int SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall);
	int SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex);
	int ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6], KxCallStatus& hCall);
	int ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6]);
	int SingleImageLearn(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int SingleImageLearn(const kxCImageBuf& ImgCheck);

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

	//获取图像掩膜对象，用于自动mask图像显示
	void  GetImageMaskImage(kxCImageBuf& AutoImg)
	{
		m_hImgMask.ShowMaskImage(m_BaseImg, m_hWarpStrech.GetParameter().m_rcCheckArea);
		AutoImg.buf = m_hImgMask.GetAutoMaskImage(AutoImg.nWidth, AutoImg.nHeight, AutoImg.nPitch, AutoImg.nChannel);
	}

	//获取图像校正后的图像
	unsigned char* GetWarpImage(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		nWidth = m_WarpImg.nWidth;
		nHeight = m_WarpImg.nHeight;
		nPitch = m_WarpImg.nPitch;
		nChannel = m_WarpImg.nChannel;
		return m_WarpImg.buf;
	}
	kxCImageBuf& GetWarpImage() { return m_WarpImg; }

	//获取残差图像
	unsigned char* GetImageCc(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		nWidth = m_ImgCc.nWidth;
		nHeight = m_ImgCc.nHeight;
		nPitch = m_ImgCc.nPitch;
		nChannel = m_ImgCc.nChannel;
		return m_ImgCc.buf;
	}
	kxCImageBuf& GetImageCc() { return m_ImgCc; }

	//获取是否打开签名条检测模块
	//1 代表打开  0代表关闭
	//int IsOpenSignatureCheck() { return m_hParameter.m_nIsCheckSignature; }
};

#endif
