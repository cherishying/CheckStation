#ifndef _KXGRADIENTPROCESSHH
#define _KXGRADIENTPROCESSHH

//add by lyl 2016/6/12
//�������ɺ�����ͼ���ݶȴ����㷨ģ��
//�������¹��ܣ�
// 0. �ݶ�ͬ���㷨��
// 1. �����ݶ��㷨��
//2016/12/6 ����Ipp�⣬���¸�д

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
    //���ݶ��㷨������м����
	kxCImageBuf m_TmpImg;
	kxCImageBuf m_DilateImg, m_ErodeImg;
	kxCImageBuf m_TmpImg1;
	kxCImageBuf m_TmpImg2;
	kxCImageBuf m_TmpImg3;

	//ȫ�ݶ��㷨������м����
	kxCImageBuf  m_SoomthImg;

	//�ϳ��ݶ�ͼ
	kxCImageBuf  m_GradientMaxImg;

	CKxBaseFunction m_hFun;

	//�ⲿ����
public: 
	//����ͬ�Ե��ݶ��㷨��C1/C3��
	//���룺һ�Ŵ������ͼ�� SrcImg
	//�����һ�Ŵ����ȫ���ݶ�ͼ�� DstImg
	int ComputeGrand(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nSmoothSize, KxCallStatus& hCall);
	int ComputeGrand(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nSmoothSize = _3X3);

	//ѹ��ͼ������ݶ��㷨��C1/C3��
	//���룺һ�Ŵ������ͼ��SrcImg��
	//      ����  nDirection(��(0)����(1)��Ʋ(2)����(3))
	//      ģʽ  bWhiteBlackMode����(0)����(1)��
	//      ѹ������ nCompressSize
	//      ���߶� nLargeSize (ֻ֧��1�� 3�� 5)
	//�����һ�Ŵ���ĵ����ݶ�ͼ��DstImg
	int SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall);
	int SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize = 4, Ipp8u bordervalue = 255, int nLargeSize = 5);

	int ParallelSingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall , int nRowGrain = 120, int nColGrain = 120);
	int ParallelSingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize = 4, Ipp8u bordervalue = 255, int nLargeSize = 5);
	
	//�ϳɵĵ���ѹ���ݶ�ͼ��(Max��ϵ)
	//���룺SrcImg      ������ͼ��
	//      DstImg      Ŀ��ͼ��
	//    nCompressSize ѹ������
	//ע���ϳ�SingleDirGradient�������ɵ�8���ݶ�ͼ��
	int GradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int GradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue);


	int ParallelGradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int ParallelGradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize = 4, Ipp8u bordervalue = 255);
	//ԭʼͼ������ݶ��㷨��C1/C3��
	//���룺һ�Ŵ������ͼ��SrcImg��
	//      ����    nDirection(��(0)����(1))
	//    ƽ��ģʽ  nFilterType����ֵ(1)����ֵ(2)��
	//    ƽ���ߴ�  nSoomthSize 
	//�����һ�Ŵ���ĵ����ݶ�ͼ��DstImg
	int  SingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int  SingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType = _AverageFilter, int nSoomthSize = 5, Ipp8u bordervalue = 255);


	int  ParallelSingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int  ParallelSingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType = _AverageFilter, int nSoomthSize = 5, Ipp8u bordervalue = 255);
	
	
	//�ϳɵĵ����ݶ�ͼ��(Max��ϵ)
	//���룺SrcImg      ������ͼ��
	//      DstImg      Ŀ��ͼ��
	//    ƽ��ģʽ  nFilterType����ֵ(1)����ֵ(2)��
	//    ƽ���ߴ�  nSoomthSize (��֧�ֳߴ�3*3/5*5)
	//ע���ϳ�SingleDirGradientSame�������ɵ�2���ݶ�ͼ��
	int  GradientImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall);
	int  GradientImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType = _AverageFilter, int nSoomthSize = 5, Ipp8u bordervalue = 255);

	
	//���ڼ��Ƚ����ػ��ˣ�����������ڵ����ݶ��㷨�������ļ�С���㷨
	//	���룺 һ�Ŵ������ͼ��SrcImg��
	//  ����    nDirection(��(0)����(1))
	//  �����һ�Ŵ����ѹ�������ݶ�ͼ��DstImg
	int  SingleDirGradientLV(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, Ipp8u bordervalue , KxCallStatus& hCall);
	int  SingleDirGradientLV(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, Ipp8u bordervalue = 255);

	//ԭͼ��߶ȵĵ����ݶ�
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