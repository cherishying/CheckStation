//add by lyl 2017/9/22
//This class is use to check card's chip


#ifndef _KXCHIPCHECKH
#define _KXCHIPCHECKH
//#include "KxAlogrithm.h"

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

#ifdef OPENCV
#include <opencv2/opencv.hpp>
#include "opencv2/features2d/features2d.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/calib3d/calib3d.hpp" 
#include "opencv2/imgproc/imgproc_c.h" 
#include "opencv2/imgproc/imgproc.hpp"   
using namespace cv;
#endif


class CKxChipCheck
{
public:
	CKxChipCheck();
	~CKxChipCheck();

public:
	enum
	{
		_Valid_Layer = 2,
		_Max_Layer = 10,


		_SAMLLDOTS_INDEX_END = 2,
		_LINE_INDEX_END = 6,
		_SCRATCH_INDEX_END = 7,
		_GRAY_INDEX_END = 9, //灰度检
		_LOSS_INDEX_END = 10,

		_CheckModeNum = 2,

		_Max_BlobCount = 4,

		_MAX_OFFSET = 10,
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
			memcpy(m_szVersion, "CheckControl1.0", _VersionNameLen);
			m_nIsOpenSurfaceCheck = 1;
			m_nAreaNumber = 0;
			m_nCheckMethod = 0;
			m_nModelProcessSize = _3X3;
			m_nIsLearn = 0;
			m_nOffset = 5;
			m_nSmallDotOffset = 10;
			m_nLineOffset = 10;
			m_nLossOffset = 10;
			m_nScratchOffset = 10;
			m_nGrayOffset = 10;
			m_rcEdgeRect.setup(0, 0, 1399, 899);
		}
		char              m_szVersion[_VersionNameLen];  //记录参数的版本信息
		int               m_nIsOpenSurfaceCheck;  //是否打开质量检查
		int               m_nAreaNumber;          //区域号
		int               m_nCheckMethod;         //检测方法
		int               m_nModelProcessSize;    //模板处理尺寸
		int               m_nIsLearn;             //是否学习
		int               m_nOffset;
		int               m_nSmallDotOffset;      //检小点偏移
		int               m_nLineOffset;          //检细线偏移
		int               m_nScratchOffset;       //检划伤偏移
		int               m_nLossOffset;          //检缺失偏移
		int               m_nGrayOffset;          //检灰度偏移
		int               m_nGrayLowOffset;       //检灰度低偏移
		kxRect<int>       m_rcEdgeRect;           //边缘框

		//芯片检查特别增加的参数
		int               m_nCopperWireOffset;    //检查铜线
		int               m_nChipCellHighOfffset;
		int               m_nChipCellLowOffset;   //检查槽
		int               m_nSegmentLayer;        //分隔层面
		kxRect<int>       m_rcColor;              //采样的颜色
		int               m_nSegmentThresh;       //阈值

	};

	struct Result
	{
		Result()
		{
			memset(m_hBlobInfo, 0, sizeof(BlobInfo)*_Max_BlobCount);
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

		BlobInfo      m_hBlobInfo[_Max_BlobCount];   //缺陷特征
		int           m_nCount;                      //实际缺陷个数
		int           m_nStatus;                     //结果状态信息
		int           m_nKernIndex;                  //相似度错误定位核标号
		int           m_nDistanceX;                  //距离错误返回实际距离
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
			for (int k = 0; k < _Max_BlobCount; k++)
			{
				m_hBlobInfo[k].m_ErrImg.Release();
			}
		}
	};

#pragma pack(pop)


public:
	kxCImageBuf              m_ImgTemplate[_Max_Layer + 1];  //模板+一副合成模板
	kxCImageBuf              m_ImgCompose;
	kxCImageBuf              m_ImgLearnTemplate[_Max_Layer + 1];  //局部学习模板+一副合成模板

public:
	CKxBaseFunction          m_hFun;
	CKxBaseFunction          m_hBaseFun[_Max_Layer];
	CKxColorDiff             m_hColorDiff;          //色差检查
	CKxCalDistance           m_hCalDistance;        //距离检测
	CKxWarpStretch           m_hWarpStrech;
	CKxLightAdjust           m_hLightAdjust;
	CKxGradientProcess       m_hGradient[_Max_Layer];
	CKxLearn                 m_hLearn[_Max_Layer];  //四个学习对象对应四副处理图
	CKxSliderCc              m_hSlider[_Max_Layer];
	CKxImageMask             m_hImgMask;            //图像掩膜
	CkxPolygonMask		     m_hPolygonMask;					//多边形掩膜
	CKxBlobAnalyse           m_hBlobAnaly;          //Blob分析

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

	int                      m_nOffsetX, m_nOffsetY;
	int                      m_nSegmentThresh;
	int                      m_nSegmentLayer;
	CKxBlobAnalyse           m_hSelectBlobAnalyse;

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

	kxCImageBuf      m_TmpImg[_SAMLLDOTS_INDEX_END];
	kxCImageBuf      m_TmpImg1[_SAMLLDOTS_INDEX_END];
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

	//kxCImageBuf       m_ImgGray, m_ImgBinary, m_ImgFillHoles, m_ImgBigArea, m_ImgMask, m_ImgClose, m_ImgWhite, m_ImgSingle[3], m_ImgColor, m_ImgMask1;
	//kxCImageBuf      m_ImgPos;
	//kxCImageBuf      m_ImgEdge, m_ImgGray1, m_ImgBinary1, m_ImgFillHoles1, m_ImgClose1;


	kxCImageBuf		 m_ImgHSV, m_ImgHSVTmp;
	kxCImageBuf      m_ImgHist;  //模板对应的直方图
	kxCImageBuf      m_ImgBackproj;
	kxCImageBuf      m_ImgSegmentThresh;
	kxCImageBuf      m_ImgFilterSmall;
	kxCImageBuf      m_ImgClose, m_ImgClose1, m_ImgClose2;
	kxCImageBuf      m_ImgBlack; //芯片槽内
	kxCImageBuf      m_ImgFill;
	CKxBlobAnalyse   m_hBlobAnalySelect;          //Blob分析
	kxCImageBuf      m_ImgChipAreaMask;

	kxCImageBuf      m_ImgOffset1[_Max_Layer];
	kxCImageBuf      m_ImgOffset2[_Max_Layer];
	kxCImageBuf      m_ImgChip[_Max_Layer];
	kxCImageBuf      m_ImgOutChip[_Max_Layer];
	kxCImageBuf      m_ImgOutChipAreaMask;
	kxCImageBuf      m_ImgCopperLine;

	kxCImageBuf      m_ImgDilate;
	kxCImageBuf      m_ImgXor;

	kxCImageBuf      m_ImgcloseBig;

#ifdef OPENCV
	cv::Mat          m_hist;
#endif

public:

	int ReadImg(FILE* fp);
	int WriteImg(FILE* fp);

protected:
	//参数版本1的读写
	bool ReadVesion1Para(FILE* fp);
	bool ReadVesion2Para(FILE* fp);
	bool ReadVesion3Para(FILE* fp);   //读取版本3参数
	bool ReadVesion4Para(FILE* fp);   //读取版本4参数


	bool ReadPara(FILE*  fp);
	bool WriteVesion1Para(FILE* fp);


	bool WritePara(FILE*  fp);
	int  CopyEdges(const kxCImageBuf& SrcImg, const kxCImageBuf CopyImg, kxCImageBuf& DstImg,
		kxRect<int>& rcEdge, kxRect<int>& rcCopy);
	int AnalysisCheckStatus(int nImageType);
	int AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg);
	int SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate);
	int SaveMidImage(bool bSave);
	int SetCheckExceptionStatus(const kxCImageBuf& ImgCheck); //出现异常、判为废品
	int CreateMaskByBackProject(const kxCImageBuf& SrcImg, kxCImageBuf& MaskImg, int bModel = false);

public:

	//参数的文件读写操作
	bool Read(FILE*);
	bool Write(FILE*);
	//从网络中获取参数
	int ReadParaFromNet(unsigned char*& point);
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

	//获得学习的状态
	bool IsLearn()
	{
		return  m_hParameter.m_nIsLearn > 0 ? true : false;;
	}

	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);


	int  ProcessSurfaceWarpImages(KxCallStatus& hCall);
	int  LearnTemplate(KxCallStatus& hCall);
	int  ResidualsAnalys(KxCallStatus& hCall);
	int  ConnectAnalysis(const kxCImageBuf& SrcImg, int nTestmode, KxCallStatus& hCall);
	int  ConnectAnalysis(const kxCImageBuf& SrcImg, int nTestmode = 0);


	int  SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall);
	int  SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex);

	int  SingleImageLearn(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int  SingleImageLearn(const kxCImageBuf& ImgCheck);

	int  ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6], KxCallStatus& hCall);
	int  ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6]);
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


	//获取图像掩膜对象，用于自动mask图像显示
	void  GetImageMaskImage(kxCImageBuf& AutoImg)
	{
		m_hImgMask.ShowMaskImage(m_BaseImg, m_hWarpStrech.GetParameter().m_rcCheckArea);
		AutoImg.buf = m_hImgMask.GetAutoMaskImage(AutoImg.nWidth, AutoImg.nHeight, AutoImg.nPitch, AutoImg.nChannel);
	}

	//获取图像校正后的图像
	unsigned char* GetWarpImage(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		return m_WarpImg.GetImageBuf(nWidth, nHeight, nPitch, nChannel);
	}
	kxCImageBuf& GetWarpImage() { return m_WarpImg; }

	//获取残差图像
	unsigned char* GetImageCc(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		return m_ImgCc.GetImageBuf(nWidth, nHeight, nPitch, nChannel);
	}
	kxCImageBuf& GetImageCc() { return m_ImgCc; }

	//获取是否打开质量检测模块
	//1 代表打开  0代表关闭
	int IsOpenSurfaceCheck() { return m_hParameter.m_nIsOpenSurfaceCheck; }



};

#endif

