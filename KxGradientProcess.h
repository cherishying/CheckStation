#ifndef _KXGRADIENTPROCESSHH
#define _KXGRADIENTPROCESSHH

//add by lyl 2016/6/12
//整理、归纳和增加图像梯度处理算法模块
//包括以下功能：
// 0. 梯度同向算法；
// 1. 单向梯度算法；
//2016/12/6 升级Ipp库，重新改写

#include "KxBaseFunction.h"
#include "kxDef.h"
#include "gxMath.h"
//#include "ippm.h"
#include "ippi.h"
#include "ippcv.h"


class CKxGradientProcess
{
public:
	CKxGradientProcess();
	~CKxGradientProcess();

	enum
	{
		_AverageFilter = 1,
		_MedianFilter  = 2,
	};



private:
    //新梯度算法申请的中间变量
	kxCImageBuf m_TmpImg;
	kxCImageBuf m_DilateImg, m_ErodeImg;
	kxCImageBuf m_TmpImg1;
	kxCImageBuf m_TmpImg2;
	kxCImageBuf m_TmpImg3;

	//全梯度算法申请的中间变量
	kxCImageBuf  m_SoomthImg;

	//合成梯度图
	kxCImageBuf  m_GradientMaxImg;

	CKxBaseFunction m_hFun;

	//外部函数
public: 
	//各向同性的梯度算法（C1/C3）
	//输入：一张待处理的图像 SrcImg
	//输出：一张处理的全向梯度图像 DstImg
	int ComputeGrand(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nSmoothSize, KxCallStatus& hCall);
	int ComputeGrand(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nSmoothSize = _3X3);

	//压缩图像单向的梯度算法（C1/C3）
	//输入：一张待处理的图像SrcImg，
	//      方向  nDirection(竖(0)、横(1)、撇(2)、拉(3))
	//      模式  bWhiteBlackMode（黑(0)、白(1)）
	//      压缩比例 nCompressSize
	//      变大尺度 nLargeSize (只支持1， 3， 5)
	//输出：一张处理的单向梯度图像DstImg
	int SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall);
	int SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize = 4, Ipp8u bordervalue = 255, int nLargeSize = 5);

	int ParallelSingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall , int nRowGrain = 120, int nColGrain = 120);
	int ParallelSingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize = 4, Ipp8u bordervalue = 255, int nLargeSize = 5);
	
	//合成的单向压缩梯度图像(Max关系)
	//输入：SrcImg      待处理图像
	//      DstImg      目标图像
	//    nCompressSize 压缩比例
	//注：合成SingleDirGradient函数生成的8副梯度图像
	int GradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int GradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue);


	int ParallelGradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int ParallelGradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize = 4, Ipp8u bordervalue = 255);
	//原始图像单向的梯度算法（C1/C3）
	//输入：一张待处理的图像SrcImg，
	//      方向    nDirection(竖(0)、横(1))
	//    平滑模式  nFilterType（均值(1)、中值(2)）
	//    平滑尺寸  nSoomthSize 
	//输出：一张处理的单向梯度图像DstImg
	int  SingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int  SingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType = _AverageFilter, int nSoomthSize = 5, Ipp8u bordervalue = 255);


	int  ParallelSingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int  ParallelSingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType = _AverageFilter, int nSoomthSize = 5, Ipp8u bordervalue = 255);
	
	
	//合成的单向梯度图像(Max关系)
	//输入：SrcImg      待处理图像
	//      DstImg      目标图像
	//    平滑模式  nFilterType（均值(1)、中值(2)）
	//    平滑尺寸  nSoomthSize (现支持尺寸3*3/5*5)
	//注：合成SingleDirGradientSame函数生成的2副梯度图像
	int  GradientImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int  GradientImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType = _AverageFilter, int nSoomthSize = 5, Ipp8u bordervalue = 255);

	
	//用于检查比较严重划伤，检查能力弱于单向梯度算法，低配版的检小点算法
	//	输入： 一张待处理的图像SrcImg，
	//  方向    nDirection(竖(0)、横(1))
	//  输出：一张处理的压缩单向梯度图像DstImg
	int  SingleDirGradientLV(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, Ipp8u bordervalue , KxCallStatus& hCall);
	int  SingleDirGradientLV(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, Ipp8u bordervalue = 255);

	//原图大尺度的单向梯度
	int SingleDirGradientSameWithBigSmooth(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int SingleDirGradientSameWithBigSmooth(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue);

	int SingleDirGradientSameWithBigSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue);
	int SingleDirGradientSameWithBigSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall);

	int GradientImageWithFour(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType = _AverageFilter, int nSoomthSize = 5, Ipp8u bordervalue = 0);

	int SingleDirGradientSameWithBoxSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection,  int nSoomthSize, Ipp8u bordervalue);
	int SingleDirGradientSameWithBoxSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall);

	int SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall);
};

#endif