#include "KxBaseFunction.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
using namespace tbb;


CKxBaseFunction::CKxBaseFunction()
{

}

CKxBaseFunction::~CKxBaseFunction()
{

}

// ----------- new 2019.07.03 -------------------//
/*
author:			CSQ、 HYH
date:			2019.07.03
description:	update library, clear cancer(毒瘤)
*/


int CKxBaseFunction::KxCopyImage(InputArray SrcImg, OutputArray DstImg, const Rect& rc, KxCallStatus& hCall)
{
	cv::Mat srcmat = SrcImg.getMat();
	cv::Mat dstmat = DstImg.getMat();
	return KxCopyImage(srcmat.data, srcmat.cols, srcmat.rows, srcmat.step, srcmat.channels(),
		dstmat.data, DstImg.cols, dstmat.rows, dstmat.step, dstmat.channels(),
					   rc, hCall);

}

int CKxBaseFunction::KxCopyImage(InputArray SrcImg, OutputArray DstImg, const Rect& rc)
{
	KxCallStatus hCall;
	cv::Mat srcmat = SrcImg.getMat();
	cv::Mat dstmat = DstImg.getMat();
	return KxCopyImage(srcmat.data, srcmat.cols, srcmat.rows, srcmat.step, srcmat.channels(),
		dstmat.data, dstmat.cols, dstmat.rows, dstmat.step, dstmat.channels(),
		rc, hCall);

}


int CKxBaseFunction::KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel,
								 unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
								 const Rect& rc, KxCallStatus& hCall)

{
	IppStatus  status;
	hCall.Clear();

	if (nSrcChannel != nDstChannel)
	{
		return 0;
	}
	int right = rc.x + rc.width - 1;
	int bottom = rc.y + rc.height - 1;
	int top = rc.y;
	int left = rc.x;
	//从大图拷贝一块变为小图
	if(nSrcWidth >= nDstWidth && nSrcHeight >= nDstHeight && nSrcPitch >= nDstPitch)
	{
		if (bottom  > nSrcHeight || right > nSrcWidth || left < 0 || top < 0)
		{
			return 0;
		}
		IppiSize OffSize = {rc.width, rc.height};
		if (nSrcChannel == _Type_G8)
		{
			status = ippiCopy_8u_C1R( pSrc + left + top * nSrcPitch, nSrcPitch,
				pDst, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Big2Small_Type_G8_ippiCopy", hCall))
			{
				return 0;
			}

		}
		if (nSrcChannel == _Type_G24)
		{
			status = ippiCopy_8u_C3R( pSrc + left*3 + top * nSrcPitch, nSrcPitch,
				pDst, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Big2Small_Type_G24_ippiCopy", hCall))
			{
				return 0;
			}

		}
		if (nSrcChannel == _Type_G32)
		{
			status = ippiCopy_8u_C4R( pSrc + left*4 + top * nSrcPitch, nSrcPitch,
				pDst, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Big2Small_Type_G32_ippiCopy", hCall))
			{
				return 0;
			}
		}
	}

	//从小图整块拷贝一块到大图的中间
	else if(nSrcWidth <= nDstWidth && nSrcHeight <= nDstHeight /*&& nSrcPitch <= nDstPitch*/)
	{
		if (bottom  > nDstHeight || right > nDstWidth || left < 0 || top < 0)
		{
			return 0;
		}
		IppiSize OffSize = {rc.width, rc.height};
		if (nSrcChannel == _Type_G8)
		{
			status = ippiCopy_8u_C1R( pSrc, nSrcPitch,
				pDst + left + top * nDstPitch, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Small2Big_Type_G8_ippiCopy", hCall))
			{
				return 0;
			}
		}
		if (nSrcChannel == _Type_G24)
		{
			status = ippiCopy_8u_C3R( pSrc , nSrcPitch,
				pDst + left*3 + top * nDstPitch, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Small2Big_Type_G24_ippiCopy", hCall))
			{
				return 0;
			}

		}
		if (nSrcChannel == _Type_G32)
		{
			status = ippiCopy_8u_C4R( pSrc , nSrcPitch,
				pDst + left*4 + top * nDstPitch, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Small2Big_Type_G32_ippiCopy", hCall))
			{
				return 0;
			}

		}
	}
	else
	{
		return 0;
	}

	return 1;

}

int CKxBaseFunction::KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel,
	unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
	const Rect& rc)
{
	KxCallStatus hCall;
	return KxCopyImage(pSrc, nSrcWidth, nSrcHeight, nSrcPitch, nSrcChannel, pDst, nDstWidth, nDstHeight, nDstPitch, nDstChannel, rc, hCall);
}


float CKxBaseFunction::kxImageAlign(Point2f& pos, InputArray BigImg, InputArray SamllImg, int nPyramidLayer, KxCallStatus& hCall)
{
	Mat matBigImg = BigImg.getMat();
	Mat matSmallImg = SamllImg.getMat();
	switch (nPyramidLayer)
	{
	case  2:
	case  3:
	{
			   int nResize = int(pow(2, nPyramidLayer - 1));
			   int nResizeW = (matBigImg.cols / nResize) * nResize;
			   int nResizeH = (matBigImg.rows / nResize) * nResize;
			   m_NormBigImg = matBigImg; 
			   nResizeW = (matSmallImg.cols / nResize) * nResize;
			   nResizeH = (matSmallImg.rows / nResize) * nResize;
			   m_NormSmallImg = matSmallImg;

			   m_PyramidBigImg = Mat(m_NormBigImg.rows / nResize, m_NormBigImg.cols / nResize, matBigImg.channels());
			   m_PyramidSmallImg = Mat(m_NormSmallImg.rows / nResize, m_NormSmallImg.cols / nResize, matSmallImg.channels());
			   KxResizeImage(m_NormBigImg, m_PyramidBigImg, KxSuper);
			   KxResizeImage(m_NormSmallImg, m_PyramidSmallImg, KxSuper);
			   float fRatio = kxImageAlign(pos, m_PyramidBigImg.data, m_PyramidBigImg.cols, m_PyramidBigImg.rows, m_PyramidBigImg.step,
				   m_PyramidSmallImg.data, m_PyramidSmallImg.cols, m_PyramidSmallImg.rows, m_PyramidSmallImg.step, m_PyramidSmallImg.channels(), hCall);
			   pos.x = nResize * pos.x;
			   pos.y = nResize * pos.y;

			   return fRatio;
	}

	default:
	{
			   return kxImageAlign(pos, matBigImg.data, matBigImg.cols, matBigImg.rows, matBigImg.step,
				   matSmallImg.data, matSmallImg.cols, matSmallImg.rows, matSmallImg.step, matSmallImg.channels(), hCall);
	}

	}
}

float CKxBaseFunction::kxImageAlign(Point2f& pos, InputArray BigImg, InputArray SamllImg, int nPyramidLayer)
{
	KxCallStatus hCall;
	return kxImageAlign(pos, BigImg, SamllImg, nPyramidLayer, hCall);
}

float CKxBaseFunction::kxImageAlignBySSD(Point2f& pos, InputArray BigImg, InputArray SamllImg, int nPyramidLayer, KxCallStatus& hCall)
{
	if (BigImg.kind() == cv::_InputArray::MAT) //CPU
	{
		Mat matBigImg = BigImg.getMat();
		Mat matSamllImg = SamllImg.getMat();
		switch (nPyramidLayer)
		{
		case  2:
		case  3:
		{
			int nResize = int(pow(2, nPyramidLayer - 1));
			int nResizeW = (matBigImg.cols / nResize) * nResize;
			int nResizeH = (matBigImg.rows / nResize) * nResize;
			m_NormBigImg = matBigImg;
			nResizeW = (matSamllImg.cols / nResize) * nResize;
			nResizeH = (matSamllImg.rows / nResize) * nResize;
			m_NormSmallImg = matSamllImg;

			m_PyramidBigImg = Mat( m_NormBigImg.rows / nResize, m_NormBigImg.cols / nResize, matBigImg.channels());
			m_PyramidSmallImg = Mat( m_NormSmallImg.rows / nResize, m_NormSmallImg.cols / nResize, matSamllImg.channels());
			KxResizeImage(m_NormBigImg, m_PyramidBigImg, KxSuper);
			KxResizeImage(m_NormSmallImg, m_PyramidSmallImg, KxSuper);
			float fRatio = kxImageAlignBySSD(pos, m_PyramidBigImg.data, m_PyramidBigImg.cols, m_PyramidBigImg.rows, m_PyramidBigImg.step,
				m_PyramidSmallImg.data, m_PyramidSmallImg.cols, m_PyramidSmallImg.rows, m_PyramidSmallImg.step, m_PyramidSmallImg.channels(), hCall);
			pos.x = nResize * pos.x;
			pos.y = nResize * pos.y;
			return fRatio;
		}

		default:
		{
			return kxImageAlignBySSD(pos, matBigImg.data, matBigImg.cols, matBigImg.rows, matBigImg.step,
				matSamllImg.data, matSamllImg.cols, matSamllImg.rows, matSamllImg.step, matSamllImg.channels(), hCall);
		}

		}
	}
	else if (BigImg.kind() == cv::_InputArray::CUDA_GPU_MAT)//GPU
	{

	}
	else
	{

	}


}

float CKxBaseFunction::kxImageAlignBySSD(Point2f& pos, InputArray BigImg, InputArray SamllImg)
{
	KxCallStatus hCall;
	Mat matBigImg = BigImg.getMat();
	Mat matSamllImg = SamllImg.getMat();
	return kxImageAlignBySSD(pos, matBigImg.data, matBigImg.cols, matBigImg.rows, matBigImg.step,
		matSamllImg.data, matSamllImg.cols, matSamllImg.rows, matSamllImg.step, matSamllImg.channels(), hCall);

}

float CKxBaseFunction::kxImageAlignBySSD(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
{// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置
	IppStatus status;
	hCall.Clear();

	if (!(sw >= tw && sh >= th))
	{
		status = IppStatus(-1000);
		if (check_sts(status, "Template_Test_Size_Donot_Match", hCall))
		{
			return 0;
		}
	}

	IppiSize srcRoiSize;
	srcRoiSize.width = sw * nImgType;
	srcRoiSize.height = sh;

	IppiSize tplRoiSize;
	tplRoiSize.width = tw * nImgType;
	tplRoiSize.height = th;


	Ipp32f pMax = 0;
	IppiSize corrRoiSize;


	corrRoiSize.width = srcRoiSize.width - tplRoiSize.width + 1 + 2 * (nImgType == _Type_G24 ? 1 : 0);
	corrRoiSize.height = srcRoiSize.height - tplRoiSize.height + 1;


	m_CorrImage32f = Mat(corrRoiSize.height, corrRoiSize.width, CV_8UC1);
	memset(m_CorrImage32f.data, 0, sizeof(unsigned char)*m_CorrImage32f.cols*m_CorrImage32f.rows);
	m_Result32f = Mat(corrRoiSize.height, corrRoiSize.width, CV_8UC1);
	memset(m_Result32f.data, 0, sizeof(unsigned char)*m_Result32f.cols*m_Result32f.rows);

	int nBuffsize;
	IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNorm);
	status = ippiSqrDistanceNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);

	if (check_sts(status, "kxImageAlignBySSD_ippiSqrDistanceNormGetBufferSize", hCall))
	{
		return 0;
	}
	Ipp8u* pBuffer;
	pBuffer = ippsMalloc_8u(nBuffsize);

	status = ippiSqrDistanceNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_Result32f.data, m_Result32f.step, -7, funCfg, pBuffer);

	ippsFree(pBuffer);

	m_SplitImg[0] = Mat(corrRoiSize.height, corrRoiSize.width / nImgType, CV_8UC1);
	srcRoiSize.width = corrRoiSize.width / nImgType;
	srcRoiSize.height = corrRoiSize.height;

	if (nImgType == _Type_G24)
	{
		unsigned char* pDst[3];
		for (int i = 0; i < 3; i++)
		{
			m_SplitImg[i] = Mat(corrRoiSize.height, corrRoiSize.width / nImgType, CV_8UC1);
			pDst[i] = m_SplitImg[i].data;
		}
		ippiCopy_8u_C3P3R(m_Result32f.data, m_Result32f.step, pDst, m_SplitImg[0].step, srcRoiSize);
	}
	else
	{
		ippiCopy_8u_C1R(m_Result32f.data, m_Result32f.step, m_SplitImg[0].data, m_SplitImg[0].step, srcRoiSize);
	}

	IppiPoint maxPos;
	Ipp8u max8u;
	status = ippiMinIndx_8u_C1R(m_SplitImg[0].data, m_SplitImg[0].step, srcRoiSize, &max8u, &maxPos.x, &maxPos.y);

	if (check_sts(status, "kxImageAlignBySSD_ippsMaxIndx_8u", hCall))
	{
		return 0;
	}

	pos.x = (float)maxPos.x;
	pos.y = (float)maxPos.y;

	return max8u*1.0f / 128;


}


float CKxBaseFunction::kxImageAlignBySSD(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType)
{
	KxCallStatus hCall;
	return kxImageAlignBySSD(pos, srcBuf, sw, sh, ss, tplBuf, tw, th, ts, nImgType, hCall);
}


float CKxBaseFunction::kxImageAlign(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType)
{
	KxCallStatus hCall;
	return kxImageAlign(pos, srcBuf, sw, sh, ss, tplBuf, tw, th, ts, nImgType, hCall);
}

float CKxBaseFunction::kxImageAlign(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
{// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置
	IppStatus status;
	hCall.Clear();

	if (sw < tw || sh < th)
	{
		IppStatus nStatus = IppStatus(kxImageAlignImageSizeErr);
		if (check_sts(nStatus, "KxImageAlign_", hCall))
		{
			return 0;
		}
	}
	IppiSize srcRoiSize;
	srcRoiSize.width = sw;
	srcRoiSize.height = sh;

	IppiSize tplRoiSize;
	tplRoiSize.width = tw;
	tplRoiSize.height = th;


	Ipp32f pMax = 0;
	IppiSize corrRoiSize;


	corrRoiSize.width = srcRoiSize.width - tplRoiSize.width + 1 ;
	corrRoiSize.height = srcRoiSize.height - tplRoiSize.height + 1;


	m_CorrImage32f = Mat(corrRoiSize.height, corrRoiSize.width, CV_8UC1);
	memset(m_CorrImage32f.data, 0, sizeof(unsigned char)*m_CorrImage32f.cols*m_CorrImage32f.rows);
	m_Result32f = Mat(corrRoiSize.height, corrRoiSize.width, CV_8UC1);
	memset(m_Result32f.data, 0, sizeof(unsigned char)*m_Result32f.cols*m_Result32f.rows);

	int nBuffsize;
	IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNormCoefficient);
	status = ippiCrossCorrNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);

	if (check_sts(status, "kxImageAlign_ippiCrossCorrNormGetBufferSize", hCall))
	{
		return 0;
	}
	Ipp8u* pBuffer;
	pBuffer = ippsMalloc_8u(nBuffsize);

    if (nImgType == _Type_G24)
    {
		unsigned char* pDst[3];
		unsigned char* pTl[3];
		for (int i = 0; i < 3; i++)
		{
			m_SplitImg[i] = Mat(sh, sw, CV_8UC1);
			pDst[i] = m_SplitImg[i].data;
			m_SplitTpl[i] = Mat(th, tw, CV_8UC1);
			pTl[i] = m_SplitTpl[i].data;
		}

		ippiCopy_8u_C3P3R(srcBuf, ss, pDst, sw, srcRoiSize);
		ippiCopy_8u_C3P3R(tplBuf, ts, pTl, tw, tplRoiSize);

		for (int i = 0; i < 3; i++)
		{
			status = ippiCrossCorrNorm_8u_C1RSfs(pDst[i], sw, srcRoiSize, pTl[i], tw, tplRoiSize, m_CorrImage32f.data, m_CorrImage32f.step, -7, funCfg, pBuffer);
			ippiMaxEvery_8u_C1IR(m_CorrImage32f.data, m_CorrImage32f.step, m_Result32f.data, m_Result32f.step, corrRoiSize);
		}
    }
	else
	{
		status = ippiCrossCorrNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_Result32f.data, m_Result32f.step, -7, funCfg, pBuffer);

	}

	ippsFree(pBuffer);

	IppiPoint maxPos;
	Ipp8u max8u;
	status = ippiMaxIndx_8u_C1R(m_Result32f.data, m_Result32f.step, corrRoiSize, &max8u, &maxPos.x, &maxPos.y);

	if (check_sts(status, "kxImageAlign_ippsMaxIndx_32f", hCall))
	{
		return 0;
	}

	pos.x = (float)maxPos.x;
	pos.y = (float)maxPos.y;

	return max8u*1.0f / 128;
}

float CKxBaseFunction::kxImageAlignColor(Point2f& pos, InputArray BigImg, InputArray SmallImg,
	 float fcompressFactorx /*= 2.0*/, float fcompressFactory /*= 2.0*/)
{
	Mat matBigImg = BigImg.getMat();
	Mat matSmallImg = SmallImg.getMat();
	assert(fcompressFactorx >= 1.0f && fcompressFactory >= 1.0f);

	int nResizex = int(pow(2, fcompressFactorx - 1));
	int nResizey = int(pow(2, fcompressFactory - 1));
	if (nResizex == 1 && nResizey == 1)
	{
		Point2f Pos1(0, 0);
		float f = kxImageAlignColor(Pos1, matBigImg.data, matBigImg.cols, matBigImg.rows, matBigImg.step,
			matSmallImg.data, matSmallImg.cols, matSmallImg.rows, matSmallImg.step, matSmallImg.channels());
		pos.x = Pos1.x;
		pos.y = Pos1.y;
		return f;
	}

	int nResizeW = (matBigImg.cols / nResizex) * nResizex;
	int nResizeH = (matBigImg.rows / nResizey) * nResizey;
	m_NormBigImg = matBigImg;
	nResizeW = (matSmallImg.cols / nResizex) * nResizex;
	nResizeH = (matSmallImg.rows / nResizey) * nResizey;
	m_NormSmallImg = matSmallImg;

	m_PyramidBigImg = Mat(m_NormBigImg.rows / nResizey, m_NormBigImg.cols / nResizex, matBigImg.channels());
	m_PyramidSmallImg = Mat(m_NormSmallImg.rows / nResizey, m_NormSmallImg.cols / nResizex, matSmallImg.channels());
	KxResizeImage(m_NormBigImg, m_PyramidBigImg, KxLinear);
	KxResizeImage(m_NormSmallImg, m_PyramidSmallImg, KxLinear);


	Point2f Pos1(0, 0);
	float f = kxImageAlignColor(Pos1, m_PyramidBigImg.data, m_PyramidBigImg.cols, m_PyramidBigImg.rows, m_PyramidBigImg.step,
		m_PyramidSmallImg.data, m_PyramidSmallImg.cols, m_PyramidSmallImg.rows, m_PyramidSmallImg.step, m_PyramidSmallImg.channels());
	Pos1.x = Pos1.x*nResizex;
	Pos1.y = Pos1.y*nResizey;


	//考虑定位精度为4个像素
	int  nExpandx = 4 * nResizex + 1;
	int  nExpandy = 4 * nResizey + 1;
	int  nTop = (std::max)(0, (int)(Pos1.y - nExpandy));
	int  nLeft = (std::max)(0, (int)(Pos1.x - nExpandx));
	int  nRight = (std::min)(matBigImg.cols - 1, (int)(Pos1.x + nExpandx + matSmallImg.cols));
	int  nBottom = (std::min)(matBigImg.rows - 1, (int)(Pos1.y + nExpandy + matSmallImg.rows));
	if (nRight - nLeft + 1 < matSmallImg.cols || nBottom - nTop + 1 < matSmallImg.rows)
	{
		return  kxImageAlignColor(pos, matBigImg.data, matBigImg.cols, matBigImg.rows, matBigImg.step,
			matSmallImg.data, matSmallImg.cols, matSmallImg.rows, matSmallImg.step, matSmallImg.channels());
	}
	float fRatio = kxImageAlignColor(Pos1, matBigImg.data + nTop*matBigImg.step + nLeft * matBigImg.channels(),
		nRight - nLeft + 1, nBottom - nTop + 1, matBigImg.step, matSmallImg.data, matSmallImg.cols, matSmallImg.rows, matSmallImg.step, matSmallImg.channels());
	pos.x = Pos1.x + nLeft;
	pos.y = Pos1.y + nTop;

	return  fRatio;

}

float CKxBaseFunction::kxImageAlignColor(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType)
{
	KxCallStatus hCall;
	return kxImageAlignColor(pos, srcBuf, sw, sh, ss, tplBuf, tw, th, ts, nImgType, hCall);
}

float CKxBaseFunction::kxImageAlignColor(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
{// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置
	IppStatus status;
	hCall.Clear();

	if (sw < tw || sh < th)
	{
		IppStatus nStatus = IppStatus(kxImageAlignImageSizeErr);
		if (check_sts(nStatus, "KxImageAlign_", hCall))
		{
			return 0;
		}
	}
	IppiSize srcRoiSize;
	srcRoiSize.width = sw * nImgType;
	srcRoiSize.height = sh;

	IppiSize tplRoiSize;
	tplRoiSize.width = tw * nImgType;
	tplRoiSize.height = th;


	Ipp32f pMax = 0;
	IppiSize corrRoiSize;

	corrRoiSize.width = srcRoiSize.width - tplRoiSize.width + 1 + 2 * (nImgType == _Type_G24 ? 1 : 0);
	corrRoiSize.height = srcRoiSize.height - tplRoiSize.height + 1;

	m_Result32f = Mat(corrRoiSize.height, corrRoiSize.width, CV_8UC1);
	memset(m_Result32f.data, 0, sizeof(unsigned char)*m_Result32f.cols*m_Result32f.rows);

	int nBuffsize;
	IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNormCoefficient);
	status = ippiCrossCorrNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);

	if (check_sts(status, "kxImageAlign_ippiCrossCorrNormGetBufferSize", hCall))
	{
		return 0;
	}
	Ipp8u* pBuffer;
	pBuffer = ippsMalloc_8u(nBuffsize);

	status = ippiCrossCorrNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_Result32f.data, m_Result32f.step, -7, funCfg, pBuffer);

	m_SplitImg[0] = Mat(corrRoiSize.height, corrRoiSize.width / nImgType, CV_8UC1);
	srcRoiSize.width = corrRoiSize.width / nImgType;
	srcRoiSize.height = corrRoiSize.height;

	if (nImgType == _Type_G24)
	{
		unsigned char* pDst[3];
		for (int i = 0; i < 3; i++)
		{
			m_SplitImg[i] = Mat(corrRoiSize.height, corrRoiSize.width / nImgType, CV_8UC1);
			pDst[i] = m_SplitImg[i].data;
		}
		ippiCopy_8u_C3P3R(m_Result32f.data, m_Result32f.step, pDst, m_SplitImg[0].step, srcRoiSize);
	}
	else
	{
		ippiCopy_8u_C1R(m_Result32f.data, m_Result32f.step, m_SplitImg[0].data, m_SplitImg[0].step, srcRoiSize);
	}

	IppiPoint maxPos;
	Ipp8u max8u;
	status = ippiMaxIndx_8u_C1R(m_SplitImg[0].data, m_SplitImg[0].step, srcRoiSize, &max8u, &maxPos.x, &maxPos.y);

	if (check_sts(status, "kxImageAlignColor_ippsMaxIndx_8u", hCall))
	{
		return 0;
	}

	pos.x = (float)maxPos.x;
	pos.y = (float)maxPos.y;

	ippsFree(pBuffer);

	return max8u*1.0f / 128;


}

int  CKxBaseFunction::KxResizeLinearImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	IppiSize srcSize = { matSrcImg.cols, matSrcImg.rows };
	IppiSize dstSize = { matDstImg.cols, matDstImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippLinear, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeLinearImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeLinearInit_8u(srcSize, dstSize, pSpec);
	if (check_sts(status, "KxResizeLinearImage_ippiResizeLinearInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	//* work buffer size */
	int nBufferSize;
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, matSrcImg.type(), &nBufferSize);
	if (check_sts(status, "KxResizeLinearImage_ippiResizeGetBufferSize_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (pBuffer == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	//* Resize processing */
	IppiPoint dstOffset = { 0, 0 };
	IppiBorderType border = ippBorderRepl;

	if (matSrcImg.type() == CV_8UC1)
	{
		status = ippiResizeLinear_8u_C1R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}
	else if (matSrcImg.type() == CV_8UC3)
	{
		status = ippiResizeLinear_8u_C3R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}

	if (check_sts(status, "KxResizeLinearImage_ippiResizeLinear", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	ippsFree(pInitBuf);
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return 1;

}

int  CKxBaseFunction::KxResizeCubicImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	IppiSize srcSize = { matSrcImg.cols, matSrcImg.rows };
	IppiSize dstSize = { matDstImg.cols, matDstImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippCubic, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeCubicImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeCubicInit_8u(srcSize, dstSize, 1, 1, pSpec, pInitBuf);
	if (check_sts(status, "KxResizeCubicImage_ippiResizeCubicInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	//* work buffer size */
	int nBufferSize;
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, matSrcImg.type(), &nBufferSize);
	if (check_sts(status, "KxResizeCubicImage_ippiResizeGetBufferSize_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (pBuffer == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	//* Resize processing */
	IppiPoint dstOffset = { 0, 0 };
	IppiBorderType border = ippBorderRepl;

	if (matSrcImg.type() == CV_8UC1)
	{
		status = ippiResizeCubic_8u_C1R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}
	else if (matSrcImg.type() == CV_8UC3)
	{
		status = ippiResizeCubic_8u_C3R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}

	if (check_sts(status, "KxResizeCubicImage_ippiResizeCubic", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	ippsFree(pInitBuf);
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return 1;

}

int  CKxBaseFunction::KxResizeSuperImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	IppiSize srcSize = { matSrcImg.cols, matSrcImg.rows };
	IppiSize dstSize = { matDstImg.cols, matDstImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippSuper, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeSuperImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeSuperInit_8u(srcSize, dstSize, pSpec);
	if (check_sts(status, "KxResizeSuperImage_ippiResizeSuperInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	//* work buffer size */
	int nBufferSize;
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, matSrcImg.type(), &nBufferSize);
	if (check_sts(status, "KxResizeCubicImage_ippiResizeGetBufferSize_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (pBuffer == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	//* Resize processing */
	IppiPoint dstOffset = { 0, 0 };
	IppiBorderType border = ippBorderRepl;

	if (matSrcImg.type() == CV_8UC1)
	{
		status = ippiResizeSuper_8u_C1R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, pSpec, pBuffer);
	}
	else if (matSrcImg.type() == CV_8UC3)
	{
		status = ippiResizeSuper_8u_C3R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, pSpec, pBuffer);
	}

	if (check_sts(status, "KxResizeCubicImage_ippiResizeSuper", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	ippsFree(pInitBuf);
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return 1;

}

int  CKxBaseFunction::KxResizeLanczosImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	IppiSize srcSize = { matSrcImg.cols, matSrcImg.rows };
	IppiSize dstSize = { matDstImg.cols, matDstImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };
	IppiBorderType border = ippBorderRepl;

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippLanczos, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeLanczosImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeLanczosInit_8u(srcSize, dstSize, 3, pSpec, pInitBuf);
	if (check_sts(status, "KxResizeLanczosImage_ippiResizeLanczosInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	//* work buffer size */
	int nBufferSize;
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, matSrcImg.type(), &nBufferSize);
	if (check_sts(status, "KxResizeCubicImage_ippiResizeGetBufferSize_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (pBuffer == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	//* Resize processing */
	IppiPoint dstOffset = { 0, 0 };
	//IppiBorderType border = ippBorderInMem;

	if (matSrcImg.type() == CV_8UC1)
	{
		status = ippiResizeLanczos_8u_C1R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}
	else if (matSrcImg.type() == CV_8UC3)
	{
		status = ippiResizeLanczos_8u_C3R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}

	if (check_sts(status, "KxResizeLanczosImage_ippiResizeLanczos", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	ippsFree(pInitBuf);
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return 1;

}

int CKxBaseFunction::KxResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode)
{
	KxCallStatus hCall;
	return KxResizeImage(SrcImg, DstImg, nInterpolationMode, hCall);
}

int CKxBaseFunction::KxResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode, KxCallStatus& hCall)
{
	hCall.Clear();
	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	if (matSrcImg.cols == matDstImg.cols && matSrcImg.rows == matDstImg.rows && matSrcImg.step == matDstImg.step)
	{
		KxCallStatus hCallInfo;
		hCallInfo.Clear();
		Rect rc(0, 0, matSrcImg.cols - 1, matSrcImg.rows - 1);
		//rc.setup(0, 0, matSrcImg.cols - 1, matSrcImg.rows - 1);
		KxCopyImage(matSrcImg, matDstImg, rc, hCallInfo);
		if (check_sts(hCallInfo, "KxResizeImage_", hCall))
		{
			return 0;
		}
		return 1;
	}
	int nStatus = 1;

	switch (nInterpolationMode)
	{
	case KxNearest:
	{
		nStatus = KxResizeNearestImage(matSrcImg, matDstImg, hCall);
		break;
	}
	case KxLinear:
	{
		nStatus = KxResizeLinearImage(matSrcImg, matDstImg, hCall);
		break;
	}
	case KxCubic:
	{
		nStatus = KxResizeCubicImage(matSrcImg, matDstImg, hCall);
		break;

	}
	case KxLanczos:
	{
		nStatus = KxResizeLanczosImage(matSrcImg, matDstImg, hCall);
		break;

	}
	case KxSuper:
	{
		nStatus = KxResizeSuperImage(matSrcImg, matDstImg, hCall);
		break;

	}
	default:
	{
		nStatus = KxResizeLinearImage(matSrcImg, matDstImg, hCall);
		break;
	}

	}

	return nStatus;

}

int CKxBaseFunction::KxAverageFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, cv::Mat Mask, KxCallStatus& hCall)
{
	if (Mask.type() == CV_32SC1)
	{
		return KxAverageFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, (int*)Mask.data, hCall);
	}
	return 0;
}

int CKxBaseFunction::KxAverageFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, int* pMask, KxCallStatus& hCall)
{
	hCall.Clear();

	if (SrcImg.kind() == cv::_InputArray::MAT)
	{
		IppStatus status;
		cv::Mat Src = SrcImg.getMat();
		cv::Mat Dst = SrcImg.getMat();
		Dst.create(Src.size(), Src.type());

		bool bKernelNotInit = false;
		Ipp16s* pKernel = NULL;
		pKernel = ippsMalloc_16s(nMaskWidth*nMaskHeight);
		if (NULL == pMask)
		{
			ippsSet_16s(1, pKernel, nMaskWidth*nMaskHeight);
			bKernelNotInit = true;
		}
		else
		{
			ippsConvert_32s16s(pMask, pKernel, nMaskWidth * nMaskHeight);
		}

		IppiSize kernSize = { nMaskWidth, nMaskHeight };
		IppiSize dstRoiSize = { Src.size().width, Src.size().height };
		Ipp16s pSum;
		ippsSum_16s_Sfs(pKernel, nMaskWidth*nMaskHeight, &pSum, 0);
		int nDivisor = pSum;
		int nSpecSize = 0, nBufferSize = 0;
		//* Initializes the Buffer */
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, Src.channels(), &nSpecSize, &nBufferSize);
		if (check_sts(status, "KxAverageFilterImage_ippiFilterBorderGetSize", hCall))
		{
			ippsFree(pKernel);
			return 0;
		}
		IppiFilterBorderSpec* pSepc = (IppiFilterBorderSpec*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

		// * Initializes the filter specification structure */
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, Src.channels(), ippRndNear, pSepc);
		if (check_sts(status, "KxAverageFilterImage_ippiFilterBorderInit_16s", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (Src.type() == CV_8SC1)
		{
			Ipp8u borderValue = 0;
			status = ippiFilterBorder_8u_C1R(Src.data, Src.step, Dst.data, Dst.step, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
		}
		if (Src.type() == CV_8SC3)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_8u_C3R(Src.data, Src.step, Dst.data, Dst.step, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
		}

		if (check_sts(status, "KxAverageFilterImage_ippiFilterBorder_8u", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}
		ippsFree(pKernel);
		ippsFree(pSepc);
		ippsFree(pBuffer);
	}
	else if (SrcImg.kind() == cv::_InputArray::CUDA_GPU_MAT)
	{
		cv::cuda::GpuMat Src = SrcImg.getGpuMat();
		cv::cuda::GpuMat Dst = DstImg.getGpuMat();
		Dst.create(Src.size(), Src.type());
		NppStatus status;

		// initial kernel
		bool bKernelNotInit = false;
		Npp32s* pKernel = NULL;
		Ipp32s* pHostKernel = NULL;
		pKernel = nppsMalloc_32s(nMaskWidth*nMaskHeight);
		if (NULL == pMask)
		{
			pHostKernel = ippsMalloc_32s(nMaskWidth*nMaskHeight);
			ippsSet_32s(1, pHostKernel, nMaskWidth*nMaskHeight);
			nppsSet_32s(1, pKernel, nMaskWidth*nMaskHeight);
			bKernelNotInit = true;
		}
		else
		{
			ippsCopy_32s(pMask, pHostKernel, nMaskWidth*nMaskHeight);
			cudaMemcpy(pKernel, pMask, nMaskWidth*nMaskHeight * sizeof(Npp32s), cudaMemcpyHostToDevice);
		}

		// calculate divisor
		int nDivisor;
		ippsSum_32s_Sfs(pHostKernel, nMaskWidth*nMaskHeight, &nDivisor, 0);
		NppiSize roiSize = { Src.size().width, Src.size().height };
		NppiSize kernelSize = { nMaskWidth, nMaskHeight };
		NppiPoint anchor = { nMaskWidth / 2, nMaskHeight / 2 };
		NppiPoint srcOffset = { 0, 0 };

		if (Src.type() == CV_8UC1)
		{
			status = nppiFilterBorder_8u_C1R(Src.data, Src.step, roiSize, srcOffset,
				Dst.data, Dst.step, roiSize, pKernel, kernelSize, anchor, nDivisor, NPP_BORDER_REPLICATE);
		}
		if (Src.type() == CV_8UC3)
		{
			status = nppiFilterBorder_8u_C3R(Src.data, Src.step, roiSize, srcOffset,
				Dst.data, Dst.step, roiSize, pKernel, kernelSize, anchor, nDivisor, NPP_BORDER_REPLICATE);
		}

		cudaFree(pKernel);
		ippsFree(pHostKernel);
	}
	else
	{
		return 0;
	}

	return 1;

}

int CKxBaseFunction::KxGeneralFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, int* pMask, int scale, KxCallStatus& hCall)
{
	hCall.Clear();
	if (SrcImg.kind() == cv::_InputArray::MAT)
	{
		IppStatus status;
		cv::Mat Src = SrcImg.getMat();
		cv::Mat Dst = DstImg.getMat();
		Dst.create(Src.size(), Src.type());

		Ipp16s* pKernel = NULL;
		pKernel = ippsMalloc_16s(nMaskWidth*nMaskHeight);
		if (NULL == pMask)
		{
			ippsSet_16s(1, pKernel, nMaskWidth*nMaskHeight);
		}
		else
		{
			ippsConvert_32s16s(pMask, pKernel, nMaskWidth*nMaskHeight);
		}

		IppiSize kernSize = { nMaskWidth, nMaskHeight };
		IppiSize dstRoiSize = { Src.size().width, Src.size().height };
		//Ipp16s pSum;
		//ippsSum_16s_Sfs(pMask, nMaskWidth*nMaskHeight, &pSum, 0);
		int nDivisor = scale;
		int nSpecSize = 0, nBufferSize = 0;
		//* Convert 8u image to 16s */
		m_Mat16s.create(Src.size(), Src.type());
		m_Mat16sAbs.create(Src.size(), Src.type());
		if (CV_8UC1 == Src.type())
		{
			status = ippiConvert_8u16s_C1R(Src.data, Src.step, (Ipp16s*)m_Mat16s.data, m_Mat16s.step, dstRoiSize);
		}
		else if (CV_8UC3 == Src.type())
		{
			status = ippiConvert_8u16s_C3R(Src.data, Src.step, (Ipp16s*)m_Mat16s.data, m_Mat16s.step, dstRoiSize);
		}
		else
		{
			return 0;
		}

		//* Initializes the Buffer */
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp16s, ipp16s, Src.channels(), &nSpecSize, &nBufferSize);
		if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderGetSize", hCall))
		{
			ippsFree(pKernel);
			return 0;
		}
		IppiFilterBorderSpec* pSepc = (IppiFilterBorderSpec*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

		// * Initializes the filter specification structure */
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp16s, Src.channels(), ippRndNear, pSepc);
		if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderInit_16s", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (Src.type() == CV_8UC1)
		{
			Ipp16s borderValue = 0;
			status = ippiFilterBorder_16s_C1R((Ipp16s*)m_Mat16s.data, m_Mat16s.step, (Ipp16s*)m_Mat16sAbs.data, m_Mat16sAbs.step, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
			if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorder_16s_C1R", hCall))
			{
				ippsFree(pKernel);
				ippsFree(pSepc);
				ippsFree(pBuffer);
				return 0;
			}
			ippiAbs_16s_C1IR((Ipp16s*)m_Mat16sAbs.data, m_Mat16sAbs.step, dstRoiSize);
			ippiConvert_16s8u_C1R((Ipp16s*)m_Mat16sAbs.data, m_Mat16sAbs.step, Dst.data, Dst.step, dstRoiSize);

		}
		if (Src.type() == CV_8UC1)
		{
			Ipp16s borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_16s_C3R((Ipp16s*)m_Mat16s.data, m_Mat16s.step, (Ipp16s*)m_Mat16sAbs.data, m_Mat16sAbs.step, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
			if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorder_16s_C3R", hCall))
			{
				ippsFree(pKernel);
				ippsFree(pSepc);
				ippsFree(pBuffer);
				return 0;
			}
			ippiAbs_16s_C3IR((Ipp16s*)m_Mat16sAbs.data, m_Mat16sAbs.step, dstRoiSize);
			ippiConvert_16s8u_C3R((Ipp16s*)m_Mat16sAbs.data, m_Mat16sAbs.step, Dst.data, Dst.step, dstRoiSize);
		}

		ippsFree(pKernel);
		ippsFree(pSepc);
		ippsFree(pBuffer);
	}
	else if (SrcImg.kind() == cv::_InputArray::CUDA_GPU_MAT)
	{

	}
	return 1;
}

// -------- OLD  -------------//

int CKxBaseFunction::KxDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxDilateImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	
	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}
	int npSpecSize, nBufferSize;
	IppiMorphState* pMorphSpec = NULL;
	Ipp8u* pBuffer = NULL;
    if (SrcImg.type() == _Type_G8)
    {
		IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		status = ippiMorphologyBorderGetSize_8u_C1R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		if (check_sts(status, "KxDilateImage_Type_G8_ippiMorphologyBorderGetSize_8u_C1R", hCall))
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);
		status = ippiMorphologyBorderInit_8u_C1R(roiSize, pMask, maskSize, pMorphSpec, pBuffer );
		if (check_sts(status, "KxDilateImage_Type_G8_ippiMorphologyBorderInit_8u_C1R", hCall))
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}
        IppiBorderType borderType = ippBorderRepl;
		status = ippiDilateBorder_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize,
			                              borderType, 0, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (check_sts(status, "KxDilateImage_Type_G8_ippiDilateBorder_8u_C1R", hCall))
		{
			return 0;
		}

    }
	else if (SrcImg.type() == _Type_G24)
	{
		IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		status = ippiMorphologyBorderGetSize_8u_C3R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		if (check_sts(status, "KxDilateImage_Type_G24_ippiMorphologyBorderGetSize_8u_C3R", hCall))
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);

		status = ippiMorphologyBorderInit_8u_C3R(roiSize, pMask, maskSize, pMorphSpec, pBuffer );
		if (check_sts(status, "KxDilateImage_Type_G24_ippiMorphologyBorderInit_8u_C3R", hCall))
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}
		IppiBorderType borderType = ippBorderRepl;
		Ipp8u bordervalue[3] = {0, 0, 0};
		status = ippiDilateBorder_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize,
			borderType, bordervalue, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (check_sts(status, "KxDilateImage_Type_G24_ippiDilateBorder_8u_C3R", hCall))
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;

}

int CKxBaseFunction::KxErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxErodeImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	//DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}
	int npSpecSize, nBufferSize;
	IppiMorphState* pMorphSpec = NULL;
	Ipp8u* pBuffer = NULL;
	if (matSrcImg.type() == CV_8UC1)
	{
		IppiSize roiSize = { matSrcImg.cols, matSrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		status = ippiMorphologyBorderGetSize_8u_C1R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		if (check_sts(status, "KxErodeImage_Type_G8_ippiMorphologyBorderGetSize_8u_C1R", hCall))
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);

		status = ippiMorphologyBorderInit_8u_C1R(roiSize, pMask, maskSize, pMorphSpec, pBuffer );
		if (check_sts(status, "KxErodeImage_Type_G8_ippiMorphologyBorderInit_8u_C1R", hCall))
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}
			return 0;
		}
		IppiBorderType borderType = ippBorderRepl;
		status = ippiErodeBorder_8u_C1R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, roiSize,
			borderType, 0, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (check_sts(status, "KxErodeImage_Type_G8_ippiErodeBorder_8u_C1R", hCall))
		{
			return 0;
		}

	}
	else if (matSrcImg.type() == CV_8UC3)
	{
		IppiSize roiSize = { matSrcImg.cols, matSrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		status = ippiMorphologyBorderGetSize_8u_C3R(roiSize, maskSize, &npSpecSize, &nBufferSize);
		if (check_sts(status, "KxErodeImage_Type_G24_ippiMorphologyBorderGetSize_8u_C3R", hCall))
		{
			return 0;
		}

		pMorphSpec = (IppiMorphState*)ippsMalloc_8u(npSpecSize);
		pBuffer = ippsMalloc_8u(nBufferSize);

		status = ippiMorphologyBorderInit_8u_C3R(roiSize, pMask, maskSize, pMorphSpec, pBuffer );
		if (check_sts(status, "KxErodeImage_Type_G24_ippiMorphologyBorderInit_8u_C3R", hCall))
		{
			ippsFree(pBuffer);
			ippsFree(pMorphSpec);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}
		IppiBorderType borderType = ippBorderRepl;
		Ipp8u bordervalue[3] = {0, 0, 0};
		status = ippiErodeBorder_8u_C3R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, roiSize,
			borderType, bordervalue, pMorphSpec, pBuffer);

		ippsFree(pBuffer);
		ippsFree(pMorphSpec);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


		if (check_sts(status, "KxErodeImage_Type_G24_ippiErodeBorder_8u_C3R", hCall))
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
	return 1;

}

int CKxBaseFunction::KxOpenImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxOpenImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxOpenImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	if (SrcImg.type() == 1)
	{
		IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		IppiBorderType borderType = ippBorderRepl;
		Ipp8u borderValue = 0;

		int nSpecSize = 0, nBufferSize = 0;
		if (check_sts(status = ippiMorphAdvGetSize_8u_C1R(roiSize, maskSize, &nSpecSize, &nBufferSize),
			"KxOpenImage_ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			return 0;
		}

		IppiMorphAdvState* pSpec = (IppiMorphAdvState*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

		if (check_sts(status = ippiMorphAdvInit_8u_C1R(roiSize, pMask, maskSize, pSpec, pBuffer),
			"KxOpenImage_ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}
			return 0;
		}

		if (check_sts(status = ippiMorphOpenBorder_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, borderType, borderValue, pSpec, pBuffer),
			"KxOpenImage_ippiMorphOpenBorder_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}

		ippsFree(pSpec);
		ippsFree(pBuffer);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}

	}
	else if (SrcImg.type() == 3)
	{
		IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		IppiBorderType borderType = ippBorderRepl;
		Ipp8u borderValue[3] = {0, 0, 0};

		int nSpecSize = 0, nBufferSize = 0;
		if (check_sts(status = ippiMorphAdvGetSize_8u_C3R(roiSize, maskSize, &nSpecSize, &nBufferSize),
			"KxOpenImage_ippiMorphAdvGetSize_8u_C3R", hCall))
		{
			return 0;
		}

		IppiMorphAdvState* pSpec = (IppiMorphAdvState*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

		if (NULL == pMask)
		{
			pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
			ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		}

		if (check_sts(status = ippiMorphAdvInit_8u_C3R(roiSize, pMask, maskSize, pSpec, pBuffer),
			"KxOpenImage_ippiMorphAdvGetSize_8u_C3R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}
			return 0;
		}

		if (check_sts(status = ippiMorphOpenBorder_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, borderType, borderValue, pSpec, pBuffer),
			"KxOpenImage_ippiMorphOpenBorder_8u_C3R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}
    		return 0;
		}

		ippsFree(pSpec);
		ippsFree(pBuffer);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


	}
	else
	{
		return 0;
	}
	return 1;

}

int CKxBaseFunction::KxCloseImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxCloseImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxCloseImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
    bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	if (SrcImg.type() == 1)
	{
		IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		IppiBorderType borderType = ippBorderRepl;
		Ipp8u borderValue = 0;

		int nSpecSize = 0, nBufferSize = 0;
		if (check_sts(status = ippiMorphAdvGetSize_8u_C1R(roiSize, maskSize, &nSpecSize, &nBufferSize),
			"KxCloseImage_ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			return 0;
		}

		IppiMorphAdvState* pSpec = (IppiMorphAdvState*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

		if (check_sts(status = ippiMorphAdvInit_8u_C1R(roiSize, pMask, maskSize, pSpec, pBuffer),
			"KxCloseImage_ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}

		if (check_sts(status = ippiMorphCloseBorder_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, borderType, borderValue, pSpec, pBuffer),
			"KxCloseImage_ippiMorphClosenBorder_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}

			return 0;
		}

		ippsFree(pSpec);
		ippsFree(pBuffer);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}


	}
	else if (SrcImg.type() == 3)
	{
		IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
		IppiSize maskSize = {nMaskWidth, nMaskHeight};

		IppiBorderType borderType = ippBorderRepl;
		Ipp8u borderValue[3] = {0, 0, 0};

		int nSpecSize = 0, nBufferSize = 0;
		if (check_sts(status = ippiMorphAdvGetSize_8u_C3R(roiSize, maskSize, &nSpecSize, &nBufferSize),
			"KxCloseImage_ippiMorphAdvGetSize_8u_C3R", hCall))
		{
			return 0;
		}

		IppiMorphAdvState* pSpec = (IppiMorphAdvState*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

		if (check_sts(status = ippiMorphAdvInit_8u_C3R(roiSize, pMask, maskSize, pSpec, pBuffer),
			"KxCloseImage_ippiMorphAdvGetSize_8u_C3R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}
			return 0;
		}

		if (check_sts(status = ippiMorphCloseBorder_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, borderType, borderValue, pSpec, pBuffer),
			"KxCloseImage_ippiMorphCloseBorder_8u_C3R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			if (bMaskNotInit)
			{
				ippsFree(pMask);
			}
			return 0;
		}

		ippsFree(pSpec);
		ippsFree(pBuffer);
		if (bMaskNotInit)
		{
			ippsFree(pMask);
		}
	}
	else
	{
		return 0;
	}
	return 1;

}

int CKxBaseFunction::KxThreshImage(InputArray SrcImg, OutputArray DstImg, int nLowThresh, int nHighThresh, int nThreshLayer)
{
	KxCallStatus hCall;
	return KxThreshImage(SrcImg, DstImg, nLowThresh, nHighThresh, nThreshLayer, hCall);
}

int CKxBaseFunction::KxThreshImage(InputArray SrcImg, OutputArray DstImg, int nLowThresh, int nHighThresh, int nThreshLayer, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	DstImg.Init(SrcImg.cols, SrcImg.rows);
	m_ImgLowThresh.Init(SrcImg.cols, SrcImg.rows);
	m_ImgHighThresh.Init(SrcImg.cols, SrcImg.rows);
	IppiSize RoiSize = { SrcImg.cols, SrcImg.rows };
	if (_Type_G8 == SrcImg.type())
	{
		status = ippiCompareC_8u_C1R(SrcImg.data, SrcImg.step, (Ipp8u)nLowThresh, m_ImgLowThresh.data, m_ImgLowThresh.step, RoiSize, ippCmpGreaterEq);
		if (check_sts(status, "KxThreshImage_G8_ippiCompareC_8u_C1R_first", hCall))
		{
			return 0;
		}

		if (nHighThresh != 255)
		{
			status = ippiCompareC_8u_C1R(SrcImg.data, SrcImg.step, (Ipp8u)nHighThresh, m_ImgHighThresh.data, m_ImgHighThresh.step, RoiSize, ippCmpLessEq);
			if (check_sts(status, "KxThreshImage_G8_ippiCompareC_8u_C1R_second", hCall))
			{
				return 0;
			}

			status = ippiAnd_8u_C1R(m_ImgHighThresh.data, m_ImgHighThresh.step, m_ImgLowThresh.data, m_ImgLowThresh.step, DstImg.data, DstImg.step, RoiSize);
			if (check_sts(status, "KxThreshImage_G8_ippiAnd_8u_C1R", hCall))
			{
				return 0;
			}
		}
		else
		{
			status = ippiCopy_8u_C1R(m_ImgLowThresh.data, m_ImgLowThresh.step, DstImg.data, DstImg.step, RoiSize);
		}

	}
	if (_Type_G24 == SrcImg.type())
	{
		m_ImgGray.Init(SrcImg.cols, SrcImg.rows);
		KxConvertImageLayer(SrcImg, m_ImgGray, nThreshLayer);
		status = ippiCompareC_8u_C1R(m_ImgGray.data, m_ImgGray.step, (Ipp8u)nLowThresh, m_ImgLowThresh.data, m_ImgLowThresh.step, RoiSize, ippCmpGreaterEq);
		if (check_sts(status, "KxThreshImage_G24_ippiCompareC_8u_C1R_first", hCall))
		{
			return 0;
		}

		if (nHighThresh != 255)
		{
			status = ippiCompareC_8u_C1R(m_ImgGray.data, m_ImgGray.step, (Ipp8u)nHighThresh, m_ImgHighThresh.data, m_ImgHighThresh.step, RoiSize, ippCmpLessEq);
			if (check_sts(status, "KxThreshImage_G24_ippiCompareC_8u_C1R_second", hCall))
			{
				return 0;
			}
			status = ippiAnd_8u_C1R(m_ImgHighThresh.data, m_ImgHighThresh.step, m_ImgLowThresh.data, m_ImgLowThresh.step, DstImg.data, DstImg.step, RoiSize);
			if (check_sts(status, "KxThreshImage_G24_ippiAnd_8u_C1R", hCall))
			{
				return 0;
			}
		}
		else
		{
			status = ippiCopy_8u_C1R(m_ImgLowThresh.data, m_ImgLowThresh.step, DstImg.data, DstImg.step, RoiSize);
		}
	}

	return 1;
}





int CKxBaseFunction::KxConvertImageLayer(InputArray SrcImg, OutputArray DstImg, int nChangeType)
{
	KxCallStatus hCall;
	return KxConvertImageLayer(SrcImg, DstImg, nChangeType, hCall);
}


int CKxBaseFunction::KxConvertImageLayer(InputArray SrcImg, OutputArray DstImg, int nChangeType, KxCallStatus& hCall)
{

	DstImg.Init(SrcImg.cols, SrcImg.rows);
	return KxConvertImageLayer(SrcImg.data, SrcImg.step, SrcImg.type(),
		                       DstImg.data, DstImg.step, SrcImg.cols, SrcImg.rows, nChangeType, hCall);
}


int CKxBaseFunction::KxConvertImageLayer(const unsigned char* srcBuf, int srcPitch, int nImgType,
						                 unsigned char* dstBuf, int dstPitch, int width, int height, int nChangeType, KxCallStatus& hCall)
{
	IppStatus status = ippStsNoErr;
	hCall.Clear();
	//Nothing to compare!
	if (check_sts(status, "KxConvertImageLayer", hCall))
	{
		return 0;
	}

	if (nImgType == _Type_G8)
	{
		IppiSize roiSize={width,height};
		ippiCopy_8u_C1R(srcBuf, srcPitch, dstBuf, dstPitch, roiSize);
	}
	if (nImgType == _Type_G24)
	{
		IppiSize Roi={width,height};

		m_ImgBGR.Init(width,height,3);


		if (RGB_R == nChangeType)
		{
			ippiCopy_8u_C3C1R(srcBuf+2,srcPitch,dstBuf,dstPitch,Roi);

		}
		if (RGB_G == nChangeType)
		{
			ippiCopy_8u_C3C1R(srcBuf+1,srcPitch,dstBuf,dstPitch,Roi);
		}
		if (RGB_B==nChangeType)
		{
			ippiCopy_8u_C3C1R(srcBuf,srcPitch,dstBuf,dstPitch,Roi);

		}
		if (RGB_GRAY == nChangeType)
		{
			ippiRGBToGray_8u_C3C1R(srcBuf, srcPitch, dstBuf, dstPitch, Roi);
		}

		if (HSV_H == nChangeType)
		{
			m_ImgHSV.Init(width,height,3);
			ippiRGBToHSV_8u_C3R(srcBuf,srcPitch,m_ImgHSV.data,m_ImgHSV.step,Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.data,m_ImgHSV.step, dstBuf, dstPitch, Roi);
		}
		if (HSV_S==nChangeType)
		{
			m_ImgHSV.Init(width,height,3);
			ippiRGBToHSV_8u_C3R(srcBuf, srcPitch, m_ImgHSV.data, m_ImgHSV.step,Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.data+1, m_ImgHSV.step, dstBuf, dstPitch, Roi);
		}
		if (HSV_V==nChangeType)
		{
			m_ImgHSV.Init(width,height,3);
			ippiRGBToHSV_8u_C3R(srcBuf,srcPitch,m_ImgHSV.data,m_ImgHSV.step,Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.data+2,m_ImgHSV.step, dstBuf, dstPitch, Roi);
		}

		if (LAB_L<=nChangeType && nChangeType<= LAB_B)
		{
			m_ImgLAB.Init(width,height,3);
			ippiBGRToLab_8u_C3R(srcBuf, srcPitch, m_ImgLAB.data, m_ImgLAB.step, Roi);
		}

		if (LAB_L==nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.data, m_ImgLAB.step, dstBuf, dstPitch, Roi);
		}
		if (LAB_A==nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.data+1, m_ImgLAB.step, dstBuf, dstPitch, Roi);
		}
		if (LAB_B==nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.data+2, m_ImgLAB.step, dstBuf, dstPitch, Roi);
		}
		if (BGR_GRAY == nChangeType)
		{
			int order[3] = { 2, 1, 0 };
			ippiSwapChannels_8u_C3R(srcBuf, srcPitch, m_ImgBGR.data, m_ImgBGR.step, Roi, order);
			ippiRGBToGray_8u_C3C1R(m_ImgBGR.data, m_ImgBGR.step, dstBuf, dstPitch, Roi);
		}


	}
	return 1;
}


int CKxBaseFunction::readImgBufFromMemory(InputArray imgBuf, unsigned char *&pt)
{
	imgBuf.Release();
	int /*nWidth,nHeight,nPitch,nChannel,*/nToken;
	memcpy( &imgBuf.cols, pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &imgBuf.rows, pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &imgBuf.step, pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &imgBuf.type(), pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &nToken, pt,sizeof(int));
	pt+=sizeof(int);

	if (imgBuf.cols<0 || imgBuf.cols>10000000 || imgBuf.rows<0 || imgBuf.rows>10000000 ||  imgBuf.step<0 || imgBuf.step>10000000)
	{
		return false;
	}
	if( nToken )
	{
		imgBuf.data = ippNew( imgBuf.step*imgBuf.rows);
		imgBuf.bAuto = true;
		ippsCopy_8u(pt, imgBuf.data,  sizeof(unsigned char)*imgBuf.step*imgBuf.rows );
	}
	pt+=imgBuf.step*imgBuf.rows*sizeof(unsigned char);
	return true;
}

int CKxBaseFunction::writeImgBufToMemory(InputArray imgBuf, unsigned char *&pt)
{
	memcpy( pt, &imgBuf.cols, sizeof(int));
	pt+=sizeof(int);
	memcpy( pt, &imgBuf.rows, sizeof(int));
	pt+=sizeof(int);
	memcpy(pt, &imgBuf.step, sizeof(int));
	pt+=sizeof(int);
	memcpy(pt, &imgBuf.type(), sizeof(int));
	pt+=sizeof(int);
	int  nToken = ( imgBuf.data != NULL ? 1:0 );
	memcpy(pt, &nToken, sizeof(int));
	pt+=sizeof(int);

	if( nToken )
	{
		ippsCopy_8u(imgBuf.data, pt,  sizeof(unsigned char)*imgBuf.step*imgBuf.rows );
	}
	pt+=sizeof(unsigned char)*imgBuf.step*imgBuf.rows;
	return true;
}



std::string CKxBaseFunction::FormatIntToString(const int& n)
{
	std::ostringstream os;
	os.write(reinterpret_cast<const char *>(&n), sizeof(int));
	if (os.fail())
	{
		return "";
	}
	return os.str();

}


std::string CKxBaseFunction::FormatImageToString(InputArray hImageBuffer)
{
	std::ostringstream os;
	int nToken = (NULL != hImageBuffer.data ? 1 : 0);
	os.write(reinterpret_cast<const char *>(&hImageBuffer.cols), sizeof(int));
	os.write(reinterpret_cast<const char *>(&hImageBuffer.rows), sizeof(int));
	os.write(reinterpret_cast<const char *>(&hImageBuffer.step), sizeof(int));
	os.write(reinterpret_cast<const char *>(&hImageBuffer.type()), sizeof(int));
	os.write(reinterpret_cast<const char *>(&nToken), sizeof(int));
	if (nToken)
	{
		os.write(reinterpret_cast<const char *>(hImageBuffer.data), sizeof(unsigned char) * hImageBuffer.step * hImageBuffer.rows);
	}
	if (os.fail())
	{
		return "";
	}
	return os.str();
}

int CKxBaseFunction::LoadBMPImage_h(const char* path, InputArray readImg)
{
	KxCallStatus hCall;
	return LoadBMPImage_h(path, readImg, hCall);
}


//add by lyl  2016/7/5
int CKxBaseFunction::LoadBMPImage_h(const char* path, InputArray readImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	FILE* pFile;
	unsigned short fileType;
	ClBitMapFileHeader bmpFileHeader;
	ClBitMapInfoHeader bmpInfoHeader;
	int channels = 1;
	int width = 0;
	int height = 0;
	int step = 0;
	int offset = 0;

	ClRgbQuad* quad;

#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&pFile, path, "rb") != 0)
	{
		return 0;
	}
#else
	pFile = fopen(path, "rb");
	if (pFile == NULL)
	{
		return 0;
	}
#endif


	fread(&fileType, sizeof(unsigned short), 1, pFile);
	if (fileType == 0x4D42)
	{
		fread(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);
		fread(&bmpInfoHeader, sizeof(ClBitMapInfoHeader), 1, pFile);

		if (bmpInfoHeader.biBitCount == 8)
		{
			channels = 1;
			width = bmpInfoHeader.biWidth;
			height = bmpInfoHeader.biHeight;

			offset = (channels*width)%4;
			if (offset != 0)
			{
				offset = 4 - offset;
			}
            int nPitch = width + offset;

			if( readImg.data && readImg.cols == width && readImg.rows == height && readImg.type() == _Type_G8)
			{

			}
			else
			{
				readImg.Release();
				readImg.data = new unsigned char[nPitch*height];

			}

			readImg.cols = width;
			readImg.rows = height;
			readImg.step = width + offset;
			readImg.type() = _Type_G8;
            readImg.bAuto = true;

			quad = new ClRgbQuad[256];
			fread(quad, sizeof(ClRgbQuad), 256, pFile);
			delete []quad;

			//unsigned char* pBuf = new unsigned char[readImg.step*readImg.rows];
			unsigned char* pBuf = ippsMalloc_8u(readImg.step*readImg.rows);
			fread(pBuf, sizeof(unsigned char), readImg.step*readImg.rows, pFile);
			//m_TempImg.Init(readImg.step, readImg.rows);
			//fread(m_TempImg.data, sizeof(unsigned char), readImg.step*readImg.rows, pFile);
			IppiSize roiSize = {readImg.cols, readImg.rows};
			status = ippiCopy_8u_C1R(pBuf, readImg.step, readImg.data, readImg.step, roiSize);
			if (check_sts(status, "LoadBMPImage_h_ippiCopy_8u_C1R", hCall))
			{
				return 0;
			}

			status = ippiMirror_8u_C1IR(readImg.data, readImg.step, roiSize, ippAxsHorizontal);

			if (check_sts(status, "LoadBMPImage_h_ippiMirror_8u_C1IR", hCall))
			{
				return 0;
			}

			ippsFree(pBuf);


		}
		else if (bmpInfoHeader.biBitCount == 24)
		{
			channels = 3;
			width = bmpInfoHeader.biWidth;
			height = bmpInfoHeader.biHeight;
			offset = 3*width %4;
			if (offset != 0)
			{
				offset = 4 - offset;
			}
			int nPitch = 3*width + offset;

			if( readImg.data && readImg.cols == width && readImg.rows == height && readImg.type() == _Type_G24)
			{

			}
			else
			{
				readImg.Release();
				readImg.data = new unsigned char[nPitch*height];
			}

			readImg.cols = width;
			readImg.rows = height;
			readImg.type() = _Type_G24;
			readImg.step = 3*width + offset;
			readImg.bAuto = true;

			unsigned char* pBuf = ippsMalloc_8u(readImg.step*readImg.rows);
			fread(pBuf, sizeof(unsigned char), readImg.step*readImg.rows, pFile);

			//m_TempImg.Init(readImg.step, readImg.rows);
			//fread(m_TempImg.data, sizeof(unsigned char), readImg.step*readImg.rows, pFile);
			IppiSize roiSize = {readImg.cols, readImg.rows};
			status = ippiCopy_8u_C3R(pBuf, readImg.step, readImg.data, readImg.step, roiSize);

			if (check_sts(status, "LoadBMPImage_h_ippiCopy_8u_C3R", hCall))
			{
				return 0;
			}

			status = ippiMirror_8u_C3IR(readImg.data, readImg.step, roiSize, ippAxsHorizontal);

			if (check_sts(status, "LoadBMPImage_h_ippiMirror_8u_C3IR", hCall))
			{
				return 0;
			}
			ippsFree(pBuf);


		}
	}
	fclose( pFile );
	return 1;

}

bool CKxBaseFunction::SaveBMPImage_h(const char* path, InputArray writeImg)
{
	KxCallStatus hCall;
	return SaveBMPImage_h(path, writeImg, hCall);
}

//add by lyl 2016//7/6
bool CKxBaseFunction::SaveBMPImage_h(const char* path, InputArray writeImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	FILE *pFile;
	unsigned short fileType;
	ClBitMapFileHeader bmpFileHeader;
	ClBitMapInfoHeader bmpInfoHeader;
	int step;
	int offset;
	unsigned char pixVal = '\0';

	ClRgbQuad* quad;

#if defined( _WIN32 ) || defined ( _WIN64 )
	errno_t err = fopen_s(&pFile, path, "wb");
	if (err)
	{
		return false;
	}
#else
	pFile = fopen(path, "wb");
	if (pFile == NULL)
	{
		return 0;
	}
#endif





	fileType = 0x4D42;
	fwrite(&fileType, sizeof(unsigned short), 1, pFile);

	if (writeImg.type() == _Type_G24)//24位，通道，彩图
	{
		step = writeImg.step;
		offset = step%4;
		if (offset != 0)
		{
			step += (4-offset);
		}

		bmpFileHeader.bfSize = writeImg.rows*step + 54;
		bmpFileHeader.bfReserved1 = 0;
		bmpFileHeader.bfReserved2 = 0;
		bmpFileHeader.bfOffBits = 54;
		fwrite(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);

		bmpInfoHeader.biSize = 40;
		bmpInfoHeader.biWidth = writeImg.cols;
		bmpInfoHeader.biHeight = writeImg.rows;
		bmpInfoHeader.biPlanes = 1;
		bmpInfoHeader.biBitCount = 24;
		bmpInfoHeader.biCompression = 0;
		bmpInfoHeader.biSizeImage = writeImg.rows*step;
		bmpInfoHeader.biXPelsPerMeter = 0;
		bmpInfoHeader.biYPelsPerMeter = 0;
		bmpInfoHeader.biClrUsed = 0;
		bmpInfoHeader.biClrImportant = 0;
		fwrite(&bmpInfoHeader, sizeof(ClBitMapInfoHeader), 1, pFile);

		m_WriteTemp.Init(step, writeImg.rows);
		memset(m_WriteTemp.data, 0, sizeof(unsigned char)*(writeImg.rows * step));
		IppiSize roiSize = {writeImg.cols, writeImg.rows};
		status = ippiCopy_8u_C3R(writeImg.data, writeImg.step, m_WriteTemp.data, step, roiSize);

		if (check_sts(status, "SaveBMPImage_h_ippiCopy_8u_C3R", hCall))
		{
			return 0;
		}

		status = ippiMirror_8u_C3IR(m_WriteTemp.data, step, roiSize, ippAxsHorizontal);

		if (check_sts(status, "SaveBMPImage_h_ippiCopy_8u_C3R", hCall))
		{
			return 0;
		}

		fwrite(m_WriteTemp.data, sizeof(unsigned char), step*writeImg.rows, pFile);

	}
	else if (writeImg.type() == _Type_G8)//8位，单通道，灰度图
	{
		step = writeImg.step;
		offset = step%4;
		if (offset != 0)
		{
			step += 4-offset;
		}

		bmpFileHeader.bfSize = 54 + 256*4 + step * writeImg.rows;
		bmpFileHeader.bfReserved1 = 0;
		bmpFileHeader.bfReserved2 = 0;
		bmpFileHeader.bfOffBits = 54 + 256*4;
		fwrite(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);

		bmpInfoHeader.biSize = 40;
		bmpInfoHeader.biWidth = writeImg.cols;
		bmpInfoHeader.biHeight = writeImg.rows;
		bmpInfoHeader.biPlanes = 1;
		bmpInfoHeader.biBitCount = 8;
		bmpInfoHeader.biCompression = 0;
		bmpInfoHeader.biSizeImage = writeImg.rows*step;
		bmpInfoHeader.biXPelsPerMeter = 0;
		bmpInfoHeader.biYPelsPerMeter = 0;
		bmpInfoHeader.biClrUsed = 256;
		bmpInfoHeader.biClrImportant = 256;
		fwrite(&bmpInfoHeader, sizeof(ClBitMapInfoHeader), 1, pFile);

		quad = new ClRgbQuad[256];
		for (int i=0; i<256; i++)
		{
			quad[i].rgbBlue = i;
			quad[i].rgbGreen = i;
			quad[i].rgbRed = i;
			quad[i].rgbReserved = 0;
		}
		fwrite(quad, sizeof(ClRgbQuad), 256, pFile);
		delete []quad;

		m_WriteTemp.Init(step, writeImg.rows);
		memset(m_WriteTemp.data, 0, sizeof(unsigned char)*(writeImg.rows * step));
		IppiSize roiSize = {writeImg.cols, writeImg.rows};
		status = ippiCopy_8u_C1R(writeImg.data, writeImg.step, m_WriteTemp.data, step, roiSize);

		if (check_sts(status, "SaveBMPImage_h_ippiCopy_8u_C1R", hCall))
		{
			return 0;
		}

		status = ippiMirror_8u_C1IR(m_WriteTemp.data, step, roiSize, ippAxsHorizontal);

		if (check_sts(status, "SaveBMPImage_h_ippiMirror_8u_C1IR", hCall))
		{
			return 0;
		}

		fwrite(m_WriteTemp.data, sizeof(unsigned char), step*writeImg.rows, pFile);
	}
	fclose(pFile);

	return true;
}

////add by lyl
//bool CKxBaseFunction::SaveBMPImage(const char* path, MV_IMAGE* bmpImg)
//{
//	FILE *pFile;
//	unsigned short fileType;
//	ClBitMapFileHeader bmpFileHeader;
//	ClBitMapInfoHeader bmpInfoHeader;
//	int step;
//	int offset;
//	unsigned char pixVal = '\0';
//
//	ClRgbQuad* quad;
//
//
//#if defined( _WIN32 ) || defined ( _WIN64 )
//	errno_t err = fopen_s(&pFile, path, "wb");
//	if (err)
//	{
//		return false;
//	}
//#else
//	pFile = fopen(path, "wb");
//	if (pFile == NULL)
//	{
//		return 0;
//	}
//#endif
//
//
//	fileType = 0x4D42;
//	fwrite(&fileType, sizeof(unsigned short), 1, pFile);
//
//	if (bmpImg->type == _Type_G24)//24位，通道，彩图
//	{
//		step = bmpImg->pitch;
//		offset = step%4;
//		if (offset != 0)
//		{
//			step += (4-offset);
//		}
//
//		bmpFileHeader.bfSize = bmpImg->height*step + 54;
//		bmpFileHeader.bfReserved1 = 0;
//		bmpFileHeader.bfReserved2 = 0;
//		bmpFileHeader.bfOffBits = 54;
//		fwrite(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);
//
//		bmpInfoHeader.biSize = 40;
//		bmpInfoHeader.biWidth = bmpImg->width;
//		bmpInfoHeader.biHeight = bmpImg->height;
//		bmpInfoHeader.biPlanes = 1;
//		bmpInfoHeader.biBitCount = 24;
//		bmpInfoHeader.biCompression = 0;
//		bmpInfoHeader.biSizeImage = bmpImg->height*step;
//		bmpInfoHeader.biXPelsPerMeter = 0;
//		bmpInfoHeader.biYPelsPerMeter = 0;
//		bmpInfoHeader.biClrUsed = 0;
//		bmpInfoHeader.biClrImportant = 0;
//		fwrite(&bmpInfoHeader, sizeof(ClBitMapInfoHeader), 1, pFile);
//
//		unsigned char* pTmpBuf = new unsigned char[bmpImg->height  * step];
//		memset(pTmpBuf, 0, sizeof(unsigned char)*(bmpImg->height * step));
//		IppiSize roiSize = {bmpImg->width, bmpImg->height};
//		ippiCopy_8u_C3R((Ipp8u*)bmpImg->data, bmpImg->pitch, pTmpBuf, step, roiSize);
//		IppiSize dstSize = {bmpImg->width, bmpImg->height};
//		ippiMirror_8u_C3IR(pTmpBuf, step, dstSize, ippAxsHorizontal);
//		fwrite(pTmpBuf, sizeof(unsigned char), step*bmpImg->height, pFile);
//
//		delete []pTmpBuf;
//
//	}
//	else if (bmpImg->type == _Type_G8)//8位，单通道，灰度图
//	{
//		step = bmpImg->width;
//		offset = step%4;
//		if (offset != 0)
//		{
//			step += 4-offset;
//		}
//
//		bmpFileHeader.bfSize = 54 + 256*4 + step * bmpImg->height;
//		bmpFileHeader.bfReserved1 = 0;
//		bmpFileHeader.bfReserved2 = 0;
//		bmpFileHeader.bfOffBits = 54 + 256*4;
//		fwrite(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);
//
//		bmpInfoHeader.biSize = 40;
//		bmpInfoHeader.biWidth = bmpImg->width;
//		bmpInfoHeader.biHeight = bmpImg->height;
//		bmpInfoHeader.biPlanes = 1;
//		bmpInfoHeader.biBitCount = 8;
//		bmpInfoHeader.biCompression = 0;
//		bmpInfoHeader.biSizeImage = bmpImg->height*step;
//		bmpInfoHeader.biXPelsPerMeter = 0;
//		bmpInfoHeader.biYPelsPerMeter = 0;
//		bmpInfoHeader.biClrUsed = 256;
//		bmpInfoHeader.biClrImportant = 256;
//		fwrite(&bmpInfoHeader, sizeof(ClBitMapInfoHeader), 1, pFile);
//
//		quad = new ClRgbQuad[256];
//		for (int i=0; i<256; i++)
//		{
//			quad[i].rgbBlue = i;
//			quad[i].rgbGreen = i;
//			quad[i].rgbRed = i;
//			quad[i].rgbReserved = 0;
//		}
//		fwrite(quad, sizeof(ClRgbQuad), 256, pFile);
//		delete []quad;
//
//		unsigned char* pTmpBuf = new unsigned char[bmpImg->height * step];
//		memset(pTmpBuf, 0, sizeof(unsigned char)*(bmpImg->height * step));
//		IppiSize roiSize = {bmpImg->width, bmpImg->height};
//		ippiCopy_8u_C1R((Ipp8u*)bmpImg->data, bmpImg->pitch, pTmpBuf, step, roiSize);
//		ippiMirror_8u_C1IR(pTmpBuf, step, roiSize, ippAxsHorizontal);
//		fwrite(pTmpBuf, sizeof(unsigned char), step*bmpImg->height, pFile);
//
//		delete []pTmpBuf;
//	}
//	fclose(pFile);
//
//	return true;
//}
//
//bool CKxBaseFunction::SaveBMPImage_h(const char* path, MV_IMAGE* bmpImg)
//{
//	kxImageBuf TmpBuf;
//	if (bmpImg->type == _Type_G8)
//	{
//		TmpBuf.Init(bmpImg->width, bmpImg->height);
//		IppiSize roiSize = {bmpImg->width, bmpImg->height};
//		ippiCopy_8u_C1R((Ipp8u*)bmpImg->data, bmpImg->pitch, TmpBuf.data, TmpBuf.step, roiSize);
//		MV_IMAGE hImg;
//		hImg.data = TmpBuf.GetImageBuf(hImg.width, hImg.height, hImg.pitch, hImg.type);
//		SaveBMPImage(path, &hImg);
//	}
//	if (bmpImg->type == _Type_G24)
//	{
//		TmpBuf.Init(bmpImg->width, bmpImg->height, 3);
//		IppiSize roiSize = {bmpImg->width, bmpImg->height};
//		ippiCopy_8u_C3R((Ipp8u*)bmpImg->data, bmpImg->pitch, TmpBuf.data, TmpBuf.step, roiSize);
//		MV_IMAGE hImg;
//		hImg.data = TmpBuf.GetImageBuf(hImg.width, hImg.height, hImg.pitch, hImg.type);
//		SaveBMPImage(path, &hImg);
//	}
//	return true;
//
//}


bool CKxBaseFunction::SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall)
{
	hCall.Clear();
	m_ImgSave.SetImageBuf(pSrc, nWidth, nHeight, nPitch, nChannel, false);
	return SaveBMPImage_h(file,m_ImgSave, hCall);
}

bool CKxBaseFunction::SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel)
{
	KxCallStatus hCall;
	hCall.Clear();
	m_ImgSave.SetImageBuf(pSrc, nWidth, nHeight, nPitch, nChannel, false);
	return SaveBMPImage_h(file, m_ImgSave, hCall);
}



int CKxBaseFunction::KxParallelCopy(InputArray SrcImg, OutputArray SrcDstImg)
{
	KxCallStatus hCall;
	//SrcDstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
	return KxParallelCopy(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxParallelCopy(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	parallel_for(blocked_range<int>(0, SrcImg.rows, 16),
		[&](const blocked_range<int>& range)
	{
		IppiSize Roi = { (int)SrcImg.cols, (int)range.size() };
		Ipp8u* pSrc = SrcImg.data + range.begin() * SrcImg.step;
		Ipp8u* pDst = SrcDstImg.data + range.begin() * SrcDstImg.step ;
		if (_Type_G8 == SrcImg.type())
		{
			ippiCopy_8u_C1R(pSrc, SrcImg.step, pDst, SrcDstImg.step, Roi);
		}
		if (_Type_G24 == SrcImg.type())
		{
			ippiCopy_8u_C3R(pSrc, SrcImg.step, pDst, SrcDstImg.step, Roi);
		}		
	}, affinity_partitioner());

	return 1;
}

int CKxBaseFunction::KxParallelMaxEvery(InputArray SrcImg, OutputArray SrcDstImg)
{
	KxCallStatus hCall;
	return KxParallelMaxEvery(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxParallelMaxEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, 0, SrcImg.cols),
		[&](const blocked_range2d<int, int>& range)
	{
		IppiSize Roi = { (int)range.cols().size(), (int)range.rows().size() };
		Ipp8u* pSrc = SrcImg.data + range.rows().begin() * SrcImg.step + range.cols().begin() * SrcImg.type();
		Ipp8u* pDst = SrcDstImg.data + range.rows().begin() * SrcDstImg.step + range.cols().begin() * SrcDstImg.type();

		if (_Type_G8 == SrcImg.type())
		{
			ippiMaxEvery_8u_C1IR(pSrc, SrcImg.step, pDst, SrcDstImg.step, Roi);
		}
		if (_Type_G24 == SrcImg.type())
		{
			ippiMaxEvery_8u_C3IR(pSrc, SrcImg.step, pDst, SrcDstImg.step, Roi);
		}
	}, auto_partitioner());

	return 1;
}

int CKxBaseFunction::KxParallelMinEvery(InputArray SrcImg, OutputArray SrcDstImg)
{
	KxCallStatus hCall;
	return KxParallelMinEvery(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxParallelMinEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, 0, SrcImg.cols),
		[&](const blocked_range2d<int, int>& range)
	{
		IppiSize Roi = { (int)range.cols().size(), (int)range.rows().size() };
		Ipp8u* pSrc = SrcImg.data + range.rows().begin() * SrcImg.step + range.cols().begin() * SrcImg.type();
		Ipp8u* pDst = SrcDstImg.data + range.rows().begin() * SrcDstImg.step + range.cols().begin() * SrcDstImg.type();

		if (_Type_G8 == SrcImg.type())
		{
			ippiMinEvery_8u_C1IR(pSrc, SrcImg.step, pDst, SrcDstImg.step, Roi);
		}
		if (_Type_G24 == SrcImg.type())
		{
			ippiMinEvery_8u_C3IR(pSrc, SrcImg.step, pDst, SrcDstImg.step, Roi);
		}
	}, auto_partitioner());

	return 1;
}

int CKxBaseFunction::KxMaxEvery(InputArray SrcImg, OutputArray SrcDstImg)
{
	KxCallStatus hCall;
	return KxMaxEvery(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxMaxEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppiSize Roi = {SrcImg.cols, SrcImg.rows};
	if(_Type_G8 == SrcImg.type())
	{
		IppStatus status = ippiMaxEvery_8u_C1IR(SrcImg.data, SrcImg.step, SrcDstImg.data, SrcDstImg.step, Roi);
		if (check_sts(status, "KxMaxEvery_ippiMaxEvery_8u_C1IR", hCall))
		{
			return 0;
		}
	}
	if(_Type_G24 == SrcImg.type())
	{
		IppStatus status = ippiMaxEvery_8u_C3IR(SrcImg.data, SrcImg.step, SrcDstImg.data, SrcDstImg.step, Roi);
		if (check_sts(status, "KxMaxEvery_ippiMaxEvery_8u_C3IR", hCall))
		{
			return 0;
		}
	}
	return 1;
}

int CKxBaseFunction::KxMinEvery(InputArray SrcImg, OutputArray SrcDstImg)
{
	KxCallStatus hCall;
	return KxMinEvery(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxMinEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppiSize Roi = {SrcImg.cols, SrcImg.rows};
	if(_Type_G8 == SrcImg.type())
	{
		IppStatus status = ippiMinEvery_8u_C1IR(SrcImg.data, SrcImg.step, SrcDstImg.data, SrcDstImg.step, Roi);

		if (check_sts(status, "KxMaxEvery_ippiMinEvery_8u_C1IR", hCall))
		{
			return 0;
		}
	}
	if(_Type_G24 == SrcImg.type())
	{
		IppStatus status = ippiMinEvery_8u_C3IR(SrcImg.data, SrcImg.step, SrcDstImg.data, SrcDstImg.step, Roi);

		if (check_sts(status, "KxMaxEvery_ippiMinEvery_8u_C3IR", hCall))
		{
			return 0;
		}
	}
	return 1;
}

int CKxBaseFunction::GetImgOffset(InputArray SrcImg, InputArray KernImg, Rect& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy)
{
	KxCallStatus hCall;
	return GetImgOffset(SrcImg, KernImg, rcKern, nSearchExpand, nSearchDir, nDx, nDy, hCall);
}

int CKxBaseFunction::GetImgOffset(InputArray SrcImg, InputArray KernImg,  Rect& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall)
{
	Rect rcSearch;
	int nleft,nright,ntop,nbottom;
	if (nSearchDir == _Horiz_Vertical_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft() - nSearchExpand);
		ntop  = gMax(0, rcKern.GetTop() - nSearchExpand);
		nright = gMin(SrcImg.cols-1, rcKern.GetRight() + nSearchExpand);
		nbottom = gMin(SrcImg.rows-1, rcKern.GetBottom() + nSearchExpand);
	}
	else if(nSearchDir == _Vertical_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft());
		ntop = gMax(0, rcKern.GetTop() - nSearchExpand);
		nright = gMin(SrcImg.cols-1, rcKern.GetRight());
		nbottom = gMin(SrcImg.rows-1, rcKern.GetBottom()+ nSearchExpand);
	}
	else if (nSearchDir == _Horizontal_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft()- nSearchExpand);
		ntop = gMax(0, rcKern.GetTop());
		nright = gMin(SrcImg.cols-1, rcKern.GetRight() + nSearchExpand);
		nbottom = gMin(SrcImg.rows-1, rcKern.GetBottom());
	}
	else
	{
		//直接拷贝,不定位
		nDx = 0;
		nDy = 0;
		return 1;
	}

	rcSearch.setup(nleft,ntop,nright,nbottom);
	//-------------------------------------------------------------------------------------------------
	Point2f  pos;

    KxCallStatus hCallFirst;
	hCallFirst.Clear();

	//float fratio = kxImageAlign( pos, SrcImg.data + rcSearch.GetTop()*SrcImg.step+rcSearch.GetLeft()*SrcImg.type(), rcSearch.Width(), rcSearch.Height(), SrcImg.step,
	//	                        KernImg.data, KernImg.cols, KernImg.rows, KernImg.step , KernImg.type(), hCallFirst);

	InputArray test;
	test.SetImageBuf(SrcImg.data + rcSearch.GetTop()*SrcImg.step + rcSearch.GetLeft()*SrcImg.type(), rcSearch.Width(), rcSearch.Height(), SrcImg.step, SrcImg.type(), false);
	//float fratio = kxImageAlignColor(pos, test, KernImg);
	float fratio = kxImageAlign(pos, test, KernImg);

	//float fratio = kxImageAlignColor(pos, test.data, test.cols, test.rows, test.step,
	//	KernImg.data, KernImg.cols, KernImg.rows, KernImg.step, KernImg.type());

	//char sz[128];
	//InputArray test;
	//test.Init(rcSearch.Width(), rcSearch.Height(), SrcImg.type());
	//KxCopyImage(SrcImg, test, rcSearch);
	//sprintf_s(sz, 128, "d:\\Test\\test.bmp");
	//SaveBMPImage_h(sz, test);

	//sprintf_s(sz, 128, "d:\\Test\\KernImg.bmp");
	//SaveBMPImage_h(sz, KernImg);


	//if (check_sts(hCallFirst, "GetImgOffset", hCall))
	//{
	//	return 0;
	//}

	if (fratio < gMinSimilarity)
	{
		IppStatus  status = IppStatus(kxImageAlignRatioLow);
		if (check_sts(status, "KxAlign_Low_", hCall))
		{
			nDx = 0;
			nDy = 0;
			return 0;
		}
	}

	nDx = int(rcSearch.GetLeft() + pos.x - rcKern.left);
	nDy = int(rcSearch.GetTop() + pos.y - rcKern.top);


	return 1;
}

int  CKxBaseFunction::KxResizeNearestImage(InputArray SrcImg, OutputArray ResizeImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = {SrcImg.cols, SrcImg.rows};
	IppiSize dstSize = {ResizeImg.cols, ResizeImg.rows};
	IppiRect srcRoi = {0, 0, srcSize.width, srcSize.height};

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippNearest, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeNearestImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeNearestInit_8u(srcSize, dstSize, pSpec);
	if (check_sts(status, "KxResizeNearestImage_ippiResizeNearestInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	//* work buffer size */
	int nBufferSize;
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, SrcImg.type(), &nBufferSize);
	if (check_sts(status, "KxResizeNearestImage_ippiResizeGetBufferSize_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (pBuffer == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}
	//* Resize processing */
	IppiPoint dstOffset = {0, 0};
	IppiBorderType border = ippBorderInMem;

	if (SrcImg.type() == _Type_G8)
	{
		status = ippiResizeNearest_8u_C1R(SrcImg.data, SrcImg.step, ResizeImg.data, ResizeImg.step, dstOffset,
			dstSize, pSpec, pBuffer);
	}
	else if (SrcImg.type() == _Type_G24)
	{
		status = ippiResizeNearest_8u_C3R(SrcImg.data, SrcImg.step, ResizeImg.data, ResizeImg.step, dstOffset,
			dstSize, pSpec, pBuffer);
	}

	if (check_sts(status, "KxResizeNearestImage_ippiResizeNearest", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		ippsFree(pBuffer);
		return 0;
	}

	ippsFree(pInitBuf);
	ippsFree(pSpec);
	ippsFree(pBuffer);

	return 1;

}

int  CKxBaseFunction::KxParallelResizeNearestImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize dstSize = { ResizeImg.cols, ResizeImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippNearest, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeNearestImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	//* Filter initialization */
	status = ippiResizeNearestInit_8u(srcSize, dstSize, pSpec);
	if (check_sts(status, "KxResizeNearestImage_ippiResizeNearestInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	int nRowActualGrain = ((std::min))(nRowGrain, ResizeImg.rows);
	int nColActualGrain = ((std::min))(nColGrain, ResizeImg.cols);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.rows, nRowActualGrain, 0, ResizeImg.cols, nColActualGrain),
		[&SrcImg, &ResizeImg, pSpec](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		Ipp8u *pSrcT, *pDstT;
		IppiPoint srcOffset = { 0, 0 };
		IppiPoint dstOffset = { 0, 0 };

		// resized region is the full width of the image,
		// The height is set by TBB via range.size()
		IppiSize  dstSizeT = { (int)range.cols().size(), (int)range.rows().size() };

		// set up working buffer for this thread's resize
		Ipp32s localBufSize = 0;
		status = ippiResizeGetBufferSize_8u(pSpec, dstSizeT,
			ResizeImg.type(), &localBufSize);

		Ipp8u *localBuffer = (Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);


		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.data + (srcOffset.y*SrcImg.step) + srcOffset.x*SrcImg.type();
		pDstT = ResizeImg.data + (dstOffset.y*ResizeImg.step) + dstOffset.x*ResizeImg.type();

		// do the resize for grayscale or color
		switch (ResizeImg.type())
		{
		case 1:
			status = ippiResizeNearest_8u_C1R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeNearest_8u_C3R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}

int  CKxBaseFunction::KxParallelResizeLinearImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain , int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize dstSize = { ResizeImg.cols, ResizeImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippLinear, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeLinearImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeLinearInit_8u(srcSize, dstSize, pSpec);
	if (check_sts(status, "KxResizeLinearImage_ippiResizeLinearInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.rows);
	int nColActualGrain = ((std::min))(nColGrain, ResizeImg.cols);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.rows, nRowActualGrain, 0, ResizeImg.cols, nColActualGrain),
		[&SrcImg, &ResizeImg, pSpec](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		Ipp8u *pSrcT, *pDstT;
		IppiPoint srcOffset = { 0, 0 };
		IppiPoint dstOffset = { 0, 0 };

		// resized region is the full width of the image,
		// The height is set by TBB via range.size()
		IppiSize  dstSizeT = { (int)range.cols().size(), (int)range.rows().size() };

		// set up working buffer for this thread's resize
		Ipp32s localBufSize = 0;
		status = ippiResizeGetBufferSize_8u(pSpec, dstSizeT,
			ResizeImg.type(), &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.data + (srcOffset.y*SrcImg.step) + srcOffset.x*SrcImg.type();
		pDstT = ResizeImg.data + (dstOffset.y*ResizeImg.step) + dstOffset.x*ResizeImg.type();

		// do the resize for grayscale or color
		switch (ResizeImg.type())
		{
		case 1:
			status = ippiResizeLinear_8u_C1R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeLinear_8u_C3R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}

int  CKxBaseFunction::KxParallelResizeCubicImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize dstSize = { ResizeImg.cols, ResizeImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippCubic, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeCubicImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeCubicInit_8u(srcSize, dstSize, 1, 1, pSpec, pInitBuf);
	if (check_sts(status, "KxResizeCubicImage_ippiResizeCubicInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}
	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.rows);
	int nColActualGrain = (std::min)(nColGrain, ResizeImg.cols);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.rows, nRowActualGrain, 0, ResizeImg.cols, nColActualGrain),
		[&SrcImg, &ResizeImg, pSpec](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		Ipp8u *pSrcT, *pDstT;
		IppiPoint srcOffset = { 0, 0 };
		IppiPoint dstOffset = { 0, 0 };

		// resized region is the full width of the image,
		// The height is set by TBB via range.size()
		IppiSize  dstSizeT = { (int)range.cols().size(), (int)range.rows().size() };

		// set up working buffer for this thread's resize
		Ipp32s localBufSize = 0;
		status = ippiResizeGetBufferSize_8u(pSpec, dstSizeT,
			ResizeImg.type(), &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.data + (srcOffset.y*SrcImg.step) + srcOffset.x*SrcImg.type();
		pDstT = ResizeImg.data + (dstOffset.y*ResizeImg.step) + dstOffset.x*ResizeImg.type();

		// do the resize for grayscale or color
		switch (ResizeImg.type())
		{
		case 1:
			status = ippiResizeCubic_8u_C1R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeCubic_8u_C3R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}

int  CKxBaseFunction::KxParallelResizeLanczosImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize dstSize = { ResizeImg.cols, ResizeImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };
	IppiBorderType border = ippBorderRepl;

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippLanczos, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeLanczosImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeLanczosInit_8u(srcSize, dstSize, 3, pSpec, pInitBuf);
	if (check_sts(status, "KxResizeLanczosImage_ippiResizeLanczosInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.rows);
	int nColActualGrain = (std::min)(nColGrain, ResizeImg.cols);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.rows, nRowActualGrain, 0, ResizeImg.cols, nColActualGrain),
		[&SrcImg, &ResizeImg, pSpec](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		Ipp8u *pSrcT, *pDstT;
		IppiPoint srcOffset = { 0, 0 };
		IppiPoint dstOffset = { 0, 0 };

		// resized region is the full width of the image,
		// The height is set by TBB via range.size()
		IppiSize  dstSizeT = { (int)range.cols().size(), (int)range.rows().size() };

		// set up working buffer for this thread's resize
		Ipp32s localBufSize = 0;
		status = ippiResizeGetBufferSize_8u(pSpec, dstSizeT,
			ResizeImg.type(), &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.data + (srcOffset.y*SrcImg.step) + srcOffset.x*SrcImg.type();
		pDstT = ResizeImg.data + (dstOffset.y*ResizeImg.step) + dstOffset.x*ResizeImg.type();

		// do the resize for grayscale or color
		switch (ResizeImg.type())
		{
		case 1:
			status = ippiResizeLanczos_8u_C1R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeLanczos_8u_C3R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);

	return 1;
}

int  CKxBaseFunction::KxParallelResizeSuperImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus & hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize dstSize = { ResizeImg.cols, ResizeImg.rows };
	IppiRect srcRoi = { 0, 0, srcSize.width, srcSize.height };

	//* Spec and init buffer sizes */
	int nSpecSize, nInitBufSize;
	status = ippiResizeGetSize_8u(srcSize, dstSize, ippSuper, 0, &nSpecSize, &nInitBufSize);
	if (check_sts(status, "KxResizeSuperImage_ippiResizeGetSize_8u", hCall))
	{
		return 0;
	}
	//* Memory allocation */
	Ipp8u* pInitBuf = ippsMalloc_8u(nInitBufSize);
	IppiResizeSpec_32f* pSpec = (IppiResizeSpec_32f*)ippsMalloc_8u(nSpecSize);

	if (pInitBuf == NULL || pSpec == NULL)
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	//* Filter initialization */
	status = ippiResizeSuperInit_8u(srcSize, dstSize, pSpec);
	if (check_sts(status, "KxResizeSuperImage_ippiResizeSuperInit_8u", hCall))
	{
		ippsFree(pInitBuf);
		ippsFree(pSpec);
		return 0;
	}

	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.rows);
	int nColActualGrain = (std::min)(nColGrain, ResizeImg.cols);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.rows, nRowActualGrain, 0, ResizeImg.cols, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		Ipp8u *pSrcT, *pDstT;
		IppiPoint srcOffset = { 0, 0 };
		IppiPoint dstOffset = { 0, 0 };

		// resized region is the full width of the image,
		// The height is set by TBB via range.size()
		IppiSize  dstSizeT = { (int)range.cols().size(), (int)range.rows().size() };

		// set up working buffer for this thread's resize
		Ipp32s localBufSize = 0;
		status = ippiResizeGetBufferSize_8u(pSpec, dstSizeT,
			ResizeImg.type(), &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.data + (srcOffset.y*SrcImg.step) + srcOffset.x*SrcImg.type();
		pDstT = ResizeImg.data + (dstOffset.y*ResizeImg.step) + dstOffset.x*ResizeImg.type();

		// do the resize for grayscale or color
		switch (ResizeImg.type())
		{
		case 1:
			status = ippiResizeSuper_8u_C1R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT,  pSpec, localBuffer); break;
		case 3:
			status = ippiResizeSuper_8u_C3R(pSrcT, SrcImg.step, pDstT, ResizeImg.step,
				dstOffset, dstSizeT,  pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}

int CKxBaseFunction::KxParallelAverageFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxParallelAverageFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxParallelAverageFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	///IppStatus status;
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
	Ipp16s* pKernel = NULL;
	pKernel = ippsMalloc_16s(nMaskWidth*nMaskHeight);
	if (NULL == pMask)
	{
		ippsSet_16s(1, pKernel, nMaskWidth*nMaskHeight);
	}
	else
	{
		ippsCopy_16s(pMask, pKernel, nMaskWidth*nMaskHeight);
	}

	IppiSize kernSize = { nMaskWidth, nMaskHeight };
	IppiSize dstRoiSize = { SrcImg.cols, SrcImg.rows };
	Ipp16s pSum;
	ippsSum_16s_Sfs(pKernel, nMaskWidth*nMaskHeight, &pSum, 0);
	int nDivisor = pSum;

	int nRowActualGrain = gMin(nRowGrain, SrcImg.rows);
	int nColActualGrain = gMin(nColGrain, SrcImg.cols);

	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, nRowActualGrain, 0, SrcImg.cols, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.data + range.rows().begin() * SrcImg.step + range.cols().begin() * SrcImg.type();
		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();

		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, SrcImg.type(), &nSpecSize, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiFilterBorderSpec *pSepc = (IppiFilterBorderSpec*)scalable_aligned_malloc(nSpecSize*sizeof(Ipp8u), 32);

		// * Initializes the filter specification structure */
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, SrcImg.type(), ippRndNear, pSepc);

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (SrcImg.type() == _Type_G8)
		{
			Ipp8u borderValue = 0;
			status = ippiFilterBorder_8u_C1R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
		}
		if (SrcImg.type() == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_8u_C3R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
		}

		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pSepc);

	}, simple_partitioner());

	ippsFree(pKernel);

	return 1;

}

int CKxBaseFunction::KxParallelBoxFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight)
{
	KxCallStatus hCall;
	return KxParallelBoxFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, hCall);
}

int CKxBaseFunction::KxParallelBoxFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,  KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	///IppStatus status;
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());


	IppiSize kernSize = { nMaskWidth, nMaskHeight };
	IppiSize dstRoiSize = { SrcImg.cols, SrcImg.rows };


	int nRowActualGrain = gMin(nRowGrain, SrcImg.rows);
	int nColActualGrain = gMin(nColGrain, SrcImg.cols);

	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, nRowActualGrain, 0, SrcImg.cols, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.data + range.rows().begin() * SrcImg.step + range.cols().begin() * SrcImg.type();
		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();

		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBoxBorderGetBufferSize(dstRoiSize, kernSize, ipp8u, SrcImg.type(), &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (SrcImg.type() == _Type_G8)
		{
			Ipp8u borderValue = 0;
			status = ippiFilterBoxBorder_8u_C1R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, kernSize, borderType, &borderValue, pBuffer);
		}
		if (SrcImg.type() == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBoxBorder_8u_C3R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, kernSize, borderType, borderValue, pBuffer);
		}
		scalable_aligned_free((void*)pBuffer);

	}, auto_partitioner());



	return 1;

}

int CKxBaseFunction::KxMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight)
{
	KxCallStatus hCall;
	return KxMedianFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, hCall);
}

int CKxBaseFunction::KxMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall)
{
	hCall.Clear();
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
    IppStatus status;
	IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
	IppiSize maskSize = {nMaskWidth, nMaskHeight};
	int nBufferSize = 0;
	if (check_sts( status = ippiFilterMedianBorderGetBufferSize(roiSize, maskSize, ipp8u, SrcImg.type(), &nBufferSize), "KxMedianFilterImage_ippiFilterMedianBorderGetBufferSize" , hCall))
	{
		return 0;
	}
	Ipp8u*	pBuffer = ippsMalloc_8u(nBufferSize);
    IppiBorderType borderType = ippBorderRepl;
	if (SrcImg.type() == _Type_G8)
	{
		Ipp8u borderValue = 0;
		if (check_sts( status = ippiFilterMedianBorder_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, maskSize, borderType, borderValue, pBuffer),
			           "KxMedianFilterImage_ippiFilterMedianBorder_8u_C1R" , hCall ))
		{
			ippsFree(pBuffer);
			return 0;
		}
	}
	if (SrcImg.type() == _Type_G24)
	{
		Ipp8u borderValue[3] = {0, 0, 0};
		if (check_sts( status = ippiFilterMedianBorder_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, maskSize, borderType, borderValue, pBuffer),
			          "KxMedianFilterImage_ippiFilterMedianBorder_8u_C3R" , hCall))
		{
			ippsFree(pBuffer);
			return 0;
		}
	}
	ippsFree(pBuffer);

	return 1;

}

int CKxBaseFunction::KxParallelMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight)
{
	KxCallStatus hCall;
	return KxParallelMedianFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, hCall);
}

int CKxBaseFunction::KxParallelMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
	//IppStatus status;
	IppiSize roiSize = { SrcImg.cols, SrcImg.rows };
	IppiSize maskSize = { nMaskWidth, nMaskHeight };


	int nRowActualGrain = gMin(nRowGrain, SrcImg.rows);
	int nColActualGrain = gMin(nColGrain, SrcImg.cols);

	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, 0, SrcImg.cols),
		[&SrcImg, &DstImg, &maskSize](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.data + range.rows().begin() * SrcImg.step + range.cols().begin() * SrcImg.type();
		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();

		//* Initializes the Buffer */
		int nBufferSize = 0;
		status = ippiFilterMedianBorderGetBufferSize(dstRoiSize, maskSize, ipp8u, SrcImg.type(), &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiBorderType borderType = ippBorderRepl;
		if (SrcImg.type() == _Type_G8)
		{
			Ipp8u borderValue = 0;
			status = ippiFilterMedianBorder_8u_C1R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, maskSize, borderType, borderValue, pBuffer);

		}
		if (SrcImg.type() == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterMedianBorder_8u_C3R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, maskSize, borderType, borderValue, pBuffer);

		}
		scalable_aligned_free((void*)pBuffer);

	},simple_partitioner());


	return 1;

}


int CKxBaseFunction::KxPolyFit(Ipp32f* pX, Ipp32f* pY, int nDotCount, int nPoly, Ipp32f* pCoeff)
{
	KxCallStatus hCall;
	return KxPolyFit(pX, pY, nDotCount, nPoly, pCoeff, hCall);
}

int CKxBaseFunction::KxPolyFit(Ipp32f* pX, Ipp32f* pY, int nDotCount, int nPoly, Ipp32f* pCoeff, KxCallStatus& hCall)
{
	hCall.Clear();

    if(nDotCount <= nPoly)
	{
		return 0;
	}
	float* pSrc = new float[nDotCount*(nPoly+1)];
	float* pBeta = new float[nDotCount];
	for (int i = 0; i < nDotCount; i++)
	{
		for(int j = nPoly; j >= 0; j--)
		{
			pSrc[i*(nPoly+1)+(nPoly-j)] = pow(pX[i], j);
		}
		pBeta[i] = pY[i];
	}

	float *ps = new float[nPoly+1];
	lapack_int rank;
	int info = LAPACKE_sgelsd(LAPACK_ROW_MAJOR, nDotCount,nPoly+1,1,pSrc,nPoly+1,pBeta,1,ps,-1.0, &rank);
	if (check_sts(info, "KxPolyFit_LAPACKE_dgelsd", hCall))
	{
		return 0;
	}
	delete []ps;

	for (int j = 0; j <= nPoly; j++)
	{
		pCoeff[j] = pBeta[j];
	}


	return 1;


}

int CKxBaseFunction::KxFitLine(Ipp32f* pX, Ipp32f* pY, int nDotCount, Ipp32f* pCoeff)
{
	KxCallStatus hCall;
	return KxFitLine(pX, pY, nDotCount, pCoeff, hCall);
}

//n点最小二乘法直线拟合 （n >= 3）
//方程形式为：ax + by + c = 0
//约束条件 a + b + c = 1000
//对应系数 pCoeff[0], pCoeff[1], pCoeff[2]
int CKxBaseFunction::KxFitLine(Ipp32f* pX, Ipp32f* pY, int nDotCount, Ipp32f* pCoeff, KxCallStatus& hCall)
{
	hCall.Clear();
    if (nDotCount < 3)
    {
		return 0;
    }
	float* pSrc = new float[nDotCount*3];
	float* pBeta = new float[nDotCount];
	float pB[3] = { 1, 1, 1 };
	for (int i = 0; i < nDotCount; i++)
	{
		pSrc[3 * i + 0] = pX[i];
		pSrc[3 * i + 1] = pY[i];
		pSrc[3 * i + 2] = 1;
		pBeta[i] = 0;
	}

	float *ps = new float[3];


	float d = 1000;
	float x[3];
	int info = LAPACKE_sgglse(LAPACK_ROW_MAJOR, nDotCount, 3, 1, pSrc, 3, pB, 3, pBeta, &d, x);
	if (check_sts(info, "KxFitLine_LAPACKE_dgelsd", hCall))
	{
		return 0;
	}
	delete[]ps;

	for (int j = 0; j <= 2; j++)
	{
		pCoeff[j] = x[j];
	}


	return 1;


}

int CKxBaseFunction::KxRotateImageSpecialAngle(InputArray SrcImg, OutputArray DstImg, int nAngle)
{
	KxCallStatus hCall;
	return KxRotateImageSpecialAngle(SrcImg, DstImg, nAngle, hCall);
}

int CKxBaseFunction::KxRotateImageSpecialAngle(InputArray SrcImg, OutputArray DstImg,  int nAngle, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize  Size = {SrcImg.cols, SrcImg.rows};
	switch (nAngle)
	{
	case  _Angle_0:
		{
			DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
			Rect rc;
			rc.setup(0, 0, SrcImg.cols-1, SrcImg.rows-1);
			KxCallStatus hTmpCall;
			hTmpCall.Clear();
			KxCopyImage(SrcImg, DstImg, rc, hTmpCall);
			if (check_sts(hTmpCall, "KxRotateImageSpecialAngle", hCall))
			{
				return 0;
			}
			break;
		}
	case _Angle_90:
		{
			DstImg.Init(SrcImg.rows, SrcImg.cols, SrcImg.type());
			m_ImgRotate.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
			if (_Type_G8 == SrcImg.type())
			{
				status = ippiMirror_8u_C1R(SrcImg.data, SrcImg.step, m_ImgRotate.data, m_ImgRotate.step,Size,ippAxsHorizontal);
				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_90_ippiMirror_8u_C1R", hCall))
				{
					return 0;
				}
				status = ippiTranspose_8u_C1R(m_ImgRotate.data, m_ImgRotate.step, DstImg.data, DstImg.step, Size);
				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_90_ippiTranspose_8u_C1R", hCall))
				{
					return 0;
				}
			}
			if (_Type_G24 == SrcImg.type())
			{
				status = ippiMirror_8u_C3R(SrcImg.data, SrcImg.step, m_ImgRotate.data, m_ImgRotate.step,Size,ippAxsHorizontal);
				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_90_ippiMirror_8u_C3R", hCall))
				{
					return 0;
				}

				status = ippiTranspose_8u_C3R(m_ImgRotate.data, m_ImgRotate.step, DstImg.data, DstImg.step, Size);
				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_90_ippiTranspose_8u_C3R", hCall))
				{
					return 0;
				}
			}
			break;
		}
	case _Angle_180:
		{
			DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
			if (_Type_G8 == SrcImg.type())
			{
				status = ippiMirror_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step,Size,ippAxsBoth);

				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_180_ippiMirror_8u_C1R", hCall))
				{
					return 0;
				}

			}
			if (_Type_G24 == SrcImg.type())
			{
				status = ippiMirror_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step,Size,ippAxsBoth);

				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_180_ippiMirror_8u_C3R", hCall))
				{
					return 0;
				}
			}
			break;
		}
	case _Angle_270:
		{
			DstImg.Init(SrcImg.rows, SrcImg.cols, SrcImg.type());
			m_ImgRotate.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
			if (_Type_G8 == SrcImg.type())
			{
				status = ippiMirror_8u_C1R(SrcImg.data, SrcImg.step, m_ImgRotate.data, m_ImgRotate.step,Size,ippAxsVertical);

				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_270_ippiMirror_8u_C1R", hCall))
				{
					return 0;
				}

				status = ippiTranspose_8u_C1R(m_ImgRotate.data, m_ImgRotate.step, DstImg.data, DstImg.step, Size);

				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_270_ippiTranspose_8u_C1R", hCall))
				{
					return 0;
				}
			}
			if (_Type_G24 == SrcImg.type())
			{
				status = ippiMirror_8u_C3R(SrcImg.data, SrcImg.step, m_ImgRotate.data, m_ImgRotate.step,Size,ippAxsVertical);
				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_270_ippiMirror_8u_C3R", hCall))
				{
					return 0;
				}
				status = ippiTranspose_8u_C3R(m_ImgRotate.data, m_ImgRotate.step, DstImg.data, DstImg.step, Size);
				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_270_ippiTranspose_8u_C3R", hCall))
				{
					return 0;
				}
			}
			break;
		}
	default:
			break;
	}

	return 1;
}

int CKxBaseFunction::KxWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxWarpBilinearLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}

int CKxBaseFunction::KxWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize DstSize = { DstImg.cols, DstImg.rows };

	int nBufferSize = 0;
	int nChannelNum = SrcImg.type();

	IppiRect srcRoi = { 0, 0, SrcImg.cols, SrcImg.rows };
	IppiRect dstRoi = { 0, 0, DstImg.cols, DstImg.rows };
	//* Spec and init buffer sizes */
	status = ippiWarpBilinearQuadGetBufferSize(SrcSize, srcRoi, warpSrcQuad, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, &nBufferSize);

	if (check_sts(status, "KxWarpBilinearLinearQuadImgae_ippiWarpBilinearQuadGetBufferSize", hCall))
	{
		return 0;
	}

	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (NULL == pBuffer)
	{
		ippsFree(pBuffer);
		return 0;
	}

	//* Perform Bilinear transform  */
	if (1 == nChannelNum)
	{
		status = ippiWarpBilinearQuad_8u_C1R(SrcImg.data, SrcSize, SrcImg.step, srcRoi, warpSrcQuad, DstImg.data, DstImg.step, dstRoi, warpDstQuad, IPPI_INTER_LINEAR,  pBuffer);
	}
	else if (3 == nChannelNum)
	{
		status = ippiWarpBilinearQuad_8u_C3R(SrcImg.data, SrcSize, SrcImg.step, srcRoi, warpSrcQuad, DstImg.data, DstImg.step, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
	}
	else
	{
		int nSpecialstatus = -10001;
		if (check_sts(nSpecialstatus, "KxWarpBilinearLinearQuadImgae_ImageChannelNotValid", hCall))
		{
			ippsFree(pBuffer);
			return 0;
		}

	}

	if (check_sts(status, "KxWarpBilinearLinearQuadImgae_ippiWarpPerspectiveLinear", hCall))
	{
		ippsFree(pBuffer);
		return 0;
	}

	ippsFree(pBuffer);

	return 1;
}

int CKxBaseFunction::GetBilinearTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4], KxCallStatus& hCall)
{
	hCall.Clear();
	/* Source matrix with widthHeight=3 */
	double pSrc[4 * 4] = { srcQuad[0][0] * srcQuad[0][1], srcQuad[0][0], srcQuad[0][1], 1,
		srcQuad[1][0] * srcQuad[1][1], srcQuad[1][0], srcQuad[1][1], 1,
		srcQuad[2][0] * srcQuad[2][1], srcQuad[2][0], srcQuad[2][1], 1,
		srcQuad[3][0] * srcQuad[3][1], srcQuad[3][0], srcQuad[3][1], 1 };
	double pSrc2[4 * 2] = { dstQuad[0][0], dstQuad[0][1], dstQuad[1][0], dstQuad[1][1],
		dstQuad[2][0], dstQuad[2][1], dstQuad[3][0], dstQuad[3][1] };

	//////////////////////////////////////////////////////////////////////////
	double *ps = new double[4];
	lapack_int rank;
	int info = LAPACKE_dgelsd(LAPACK_ROW_MAJOR, 4, 4, 2, pSrc, 4, pSrc2, 2, ps, -1.0, &rank);
	if (check_sts(info, "GetBilinearTransform_LAPACKE_dgelsd", hCall))
	{
		return 0;
	}
	delete[]ps;

	for (int i = 0; i < 4; i++)
	{
		coeffs[0][i] = pSrc2[2 * i];
		coeffs[1][i] = pSrc2[2 * i + 1];
	}
	return 1;


}

int CKxBaseFunction::KxParalleWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxParalleWarpBilinearLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}

int CKxBaseFunction::KxParalleWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	double coeff[2][4];
	GetBilinearTransform(warpDstQuad, warpSrcQuad, coeff, hCallInfo);
	if (check_sts(hCallInfo, "KxParalleWarpBilinearLinearQuadImgae_", hCall))
	{
		return 0;
	}
	ippsSet_8u(0xff, DstImg.data, DstImg.step * DstImg.rows);
	//自动模式会出现个别位置没有值，手动分块模式减少概率，暂未找到原因？
	int nRowActualGrain = gMin(nRowGrain, DstImg.rows);
	int nColActualGrain = gMin(nColGrain, DstImg.cols);
	parallel_for(blocked_range2d<int, int>(0, DstImg.rows, nRowActualGrain, 0, DstImg.cols, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		//方法一、
		//IppStatus status;
		//Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();
		//IppiRect dstRoi = { range.cols().begin(), range.rows().begin(), (int)range.cols().size(), (int)range.rows().size() };
		//double warpSrcQuad[4][2];
		//status = ippiGetBilinearQuad(dstRoi, warpSrcQuad, coeff);
		//double warpDstQuad[4][2] = { { range.cols().begin(), range.rows().begin() }, { range.cols().begin() + (int)range.cols().size() - 1, range.rows().begin() },
		//{ range.cols().begin() + (int)range.cols().size() - 1, range.rows().begin() + (int)range.rows().size() - 1 }, { range.cols().begin(), range.rows().begin() + (int)range.rows().size() - 1 } };

		//IppiSize SrcSize = { SrcImg.cols, SrcImg.rows };
		//IppiSize DstSize = { (int)range.cols().size(), (int)range.rows().size() };

		//int nBufferSize = 0;
		//int nChannelNum = SrcImg.type();

		//IppiRect srcRoi = { 0, 0, SrcImg.cols, SrcImg.rows };
		//
		////* Spec and init buffer sizes */
		//status = ippiWarpBilinearQuadGetBufferSize(SrcSize, srcRoi, warpSrcQuad, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, &nBufferSize);
		////status = ippiWarpBilinearGetBufferSize(SrcSize, srcRoi, dstRoi, ippWarpBackward, coeff, IPPI_INTER_LINEAR,  &nBufferSize);

		//Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		////* Perform Bilinear transform  */
		//if (1 == nChannelNum)
		//{
		//	status = ippiWarpBilinearQuad_8u_C1R(SrcImg.data, SrcSize, SrcImg.step, srcRoi, warpSrcQuad, DstImg.data, DstImg.step, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
		//}
		//else if (3 == nChannelNum)
		//{
		//	status = ippiWarpBilinearQuad_8u_C3R(SrcImg.data, SrcSize, SrcImg.step, srcRoi, warpSrcQuad, DstImg.data, DstImg.step, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
		//}

		//scalable_aligned_free((void*)pBuffer);

		IppStatus status;
		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();
		IppiRect dstRoi = { range.cols().begin(), range.rows().begin(), (int)range.cols().size(), (int)range.rows().size() };

		IppiSize SrcSize = { SrcImg.cols, SrcImg.rows };
		IppiSize DstSize = { (int)range.cols().size(), (int)range.rows().size() };

		int nBufferSize = 0;
		int nChannelNum = SrcImg.type();

		IppiRect srcRoi = { 0, 0, SrcImg.cols, SrcImg.rows };

		//* Spec and init buffer sizes */
		status = ippiWarpBilinearGetBufferSize(SrcSize, srcRoi, dstRoi, ippWarpBackward, coeff, IPPI_INTER_LINEAR,  &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		//* Perform Bilinear transform  */
		if (1 == nChannelNum)
		{
			status = ippiWarpBilinearBack_8u_C1R(SrcImg.data, SrcSize, SrcImg.step, srcRoi, DstImg.data, DstImg.step, dstRoi, coeff, IPPI_INTER_LINEAR, pBuffer);
		}
		else if (3 == nChannelNum)
		{
			status = ippiWarpBilinearBack_8u_C3R(SrcImg.data, SrcSize, SrcImg.step, srcRoi, DstImg.data, DstImg.step, dstRoi, coeff, IPPI_INTER_LINEAR, pBuffer);
		}

		scalable_aligned_free((void*)pBuffer);



	}, simple_partitioner());




	return 1;
}

int CKxBaseFunction::KxWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxWarpPerspectiveLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}

int CKxBaseFunction::KxWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize DstSize = { DstImg.cols, DstImg.rows };
	IppiWarpTransformType warpTransformType = ippWarpPerspective;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.type();

	//* Spec and init buffer sizes */
	status = ippiWarpQuadGetSize(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, ippLinear,
		borderType, &nSpecSize, &nBufferSize);

	if (check_sts(status, "KxWarpPerspectiveQuadImgae_ippiWarpQuadGetSize", hCall))
	{
		return 0;
	}

	IppiWarpSpec* pSepc = (IppiWarpSpec*)ippsMalloc_8u(nSpecSize);
	if (pSepc == NULL)
	{
		int nSpecialstatus = -10000;
		if (check_sts(nSpecialstatus, "KxWarpPerspectiveQuadImgae_ippsMalloc_8u", hCall))
		{
			ippsFree(pSepc);
			return 0;
		}
	}

	//* Filter initialization */
	status = ippiWarpQuadLinearInit(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, nChannelNum, borderType, 0, 0, pSepc);
	if (check_sts(status, "KxWarpPerspectiveQuadImgae_ippiWarpQuadLinearInit", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}

	status = ippiWarpGetBufferSize(pSepc, DstSize, &nBufferSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (NULL == pBuffer)
	{
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}
	IppiPoint dstOffset = { 0, 0 };

	//* Perform perspective transform  */
	if (1 == nChannelNum)
	{
		status = ippiWarpPerspectiveLinear_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, dstOffset, DstSize, pSepc, pBuffer);
	}
	else if (3 == nChannelNum)
	{
		status = ippiWarpPerspectiveLinear_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, dstOffset, DstSize, pSepc, pBuffer);
	}
	else
	{
		int nSpecialstatus = -10001;
		if (check_sts(nSpecialstatus, "KxWarpPerspectiveQuadImgae_ImageChannelNotValid", hCall))
		{
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}

	}

	if (check_sts(status, "KxWarpPerspectiveQuadImgae_ippiWarpPerspectiveLinear", hCall))
	{
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}

	ippsFree(pSepc);
	ippsFree(pBuffer);

	return 1;
}

int CKxBaseFunction::KxParalleWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxParalleWarpPerspectiveLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}

int CKxBaseFunction::KxParalleWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize DstSize = { DstImg.cols, DstImg.rows };
	IppiWarpTransformType warpTransformType = ippWarpPerspective;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.type();



	//* Spec and init buffer sizes */
	status = ippiWarpQuadGetSize(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, ippLinear,
		borderType, &nSpecSize, &nBufferSize);

	if (check_sts(status, "KxWarpPerspectiveQuadImgae_ippiWarpQuadGetSize", hCall))
	{
		return 0;
	}

	IppiWarpSpec* pSepc = (IppiWarpSpec*)ippsMalloc_8u(nSpecSize);
	if (pSepc == NULL)
	{
		int nSpecialstatus = -10000;
		if (check_sts(nSpecialstatus, "KxWarpPerspectiveQuadImgae_ippsMalloc_8u", hCall))
		{
			ippsFree(pSepc);
			return 0;
		}
	}

	//* Filter initialization */
	status = ippiWarpQuadLinearInit(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, nChannelNum, borderType, 0, 0, pSepc);
	if (check_sts(status, "KxWarpPerspectiveQuadImgae_ippiWarpQuadLinearInit", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}

	int nRowActualGrain = gMin(nRowGrain, DstImg.rows);
	int nColActualGrain = gMin(nColGrain, DstImg.cols);

	//自动模式不会受影响
	parallel_for(blocked_range2d<int, int>(0, DstImg.rows, nRowActualGrain, 0, DstImg.cols, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };
		//Ipp8u* pOrigT = SrcImg.data;
		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();
		int nBufferSize = 0;
		status = ippiWarpGetBufferSize(pSepc, dstRoiSize, &nBufferSize);
		//Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		IppiPoint dstOffset = { (int)range.cols().begin(), (int)range.rows().begin() };

		//* Perform perspective transform  */
		if (1 == SrcImg.type())
		{
			status = ippiWarpPerspectiveLinear_8u_C1R(SrcImg.data, SrcImg.step, pDstT, DstImg.step, dstOffset, dstRoiSize, pSepc, pBuffer);
		}
		else if (3 == SrcImg.type())
		{
			status = ippiWarpPerspectiveLinear_8u_C3R(SrcImg.data, SrcImg.step, pDstT, DstImg.step, dstOffset, dstRoiSize, pSepc, pBuffer);
		}
		scalable_aligned_free((void*)pBuffer);


	},auto_partitioner());



	ippsFree(pSepc);


	return 1;
}

int CKxBaseFunction::KxWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxWarpAffineLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}

int CKxBaseFunction::KxWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = {SrcImg.cols, SrcImg.rows};
	IppiSize DstSize = {DstImg.cols, DstImg.rows};
	IppiWarpTransformType warpTransformType = ippWarpAffine;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.type();

	//* Spec and init buffer sizes */
	status = ippiWarpQuadGetSize(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, ippLinear,
		borderType, &nSpecSize, &nBufferSize);

	if (check_sts(status, "KxWarpAffineQuadImgae_ippiWarpQuadGetSize", hCall))
	{
		return 0;
	}

	IppiWarpSpec* pSepc = (IppiWarpSpec*)ippsMalloc_8u(nSpecSize);
	if (pSepc == NULL)
	{
		int nSpecialstatus = -10000;
		if (check_sts(nSpecialstatus, "KxWarpAffineQuadImgae_ippsMalloc_8u", hCall))
		{
			ippsFree(pSepc);
			return 0;
		}
	}

	//* Filter initialization */
	status = ippiWarpQuadLinearInit(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, nChannelNum, borderType, 0, 0, pSepc);
	if (check_sts(status, "KxWarpAffineQuadImgae_ippiWarpQuadLinearInit", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}

	status = ippiWarpGetBufferSize(pSepc, DstSize, &nBufferSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	if (NULL == pBuffer)
	{
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}
	IppiPoint dstOffset = {0, 0};

	//* Perform perspective transform  */
	if (1 == nChannelNum)
	{
		status = ippiWarpAffineLinear_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, dstOffset, DstSize, pSepc, pBuffer);
	}
	else if (3 == nChannelNum)
	{
		status = ippiWarpAffineLinear_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, dstOffset, DstSize, pSepc, pBuffer);
	}
	else
	{
		int nSpecialstatus = -10001;
		if (check_sts(nSpecialstatus, "KxWarpAffineQuadImgae_ImageChannelNotValid", hCall))
		{
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}

	}

	if (check_sts(status, "KxWarpAffineQuadImgae_ippiWarpPerspectiveLinear", hCall))
	{
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}

	ippsFree(pSepc);
	ippsFree(pBuffer);

	return 1;
}

int CKxBaseFunction::KxParalleWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxParalleWarpAffineLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}

int CKxBaseFunction::KxParalleWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.cols, SrcImg.rows };
	IppiSize DstSize = { DstImg.cols, DstImg.rows };
	IppiWarpTransformType warpTransformType = ippWarpAffine;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.type();

	//* Spec and init buffer sizes */
	status = ippiWarpQuadGetSize(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, ippLinear,
		borderType, &nSpecSize, &nBufferSize);

	if (check_sts(status, "KxWarpAffineQuadImgae_ippiWarpQuadGetSize", hCall))
	{
		return 0;
	}

	IppiWarpSpec* pSepc = (IppiWarpSpec*)ippsMalloc_8u(nSpecSize);
	if (pSepc == NULL)
	{
		int nSpecialstatus = -10000;
		if (check_sts(nSpecialstatus, "KxWarpAffineQuadImgae_ippsMalloc_8u", hCall))
		{
			ippsFree(pSepc);
			return 0;
		}
	}

	//* Filter initialization */
	status = ippiWarpQuadLinearInit(SrcSize, warpSrcQuad, DstSize, warpDstQuad, warpTransformType, ipp8u, nChannelNum, borderType, 0, 0, pSepc);
	if (check_sts(status, "KxWarpAffineQuadImgae_ippiWarpQuadLinearInit", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}
	int nRowActualGrain = gMin(nRowGrain, DstImg.rows);
	int nColActualGrain = gMin(nColGrain, DstImg.cols);

	parallel_for(blocked_range2d<int, int>(0, DstImg.rows, 0, DstImg.cols),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };
		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();
		int nBufferSize;
		status = ippiWarpGetBufferSize(pSepc, dstRoiSize, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiPoint dstOffset = { (int)range.cols().begin(), (int)range.rows().begin() };

		//* Perform perspective transform  */
		if (1 == SrcImg.type())
		{
			status = ippiWarpAffineLinear_8u_C1R(SrcImg.data, SrcImg.step, pDstT, DstImg.step, dstOffset, dstRoiSize, pSepc, pBuffer);
		}
		else if (3 == SrcImg.type())
		{
			status = ippiWarpAffineLinear_8u_C3R(SrcImg.data, SrcImg.step, pDstT, DstImg.step, dstOffset, dstRoiSize, pSepc, pBuffer);
		}

		scalable_aligned_free((void*)pBuffer);

	}, auto_partitioner());


	ippsFree(pSepc);

	return 1;

}

int CKxBaseFunction::KxFilterSobelImage(InputArray SrcImg, OutputArray DstImg, int nMaskSize)
{
	KxCallStatus hCall;
	return KxFilterSobelImage(SrcImg, DstImg, nMaskSize, hCall);
}

int CKxBaseFunction::KxFilterSobelImage(InputArray SrcImg, OutputArray DstImg, int nMaskSize, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
	DstImg.Init(roiSize.width, roiSize.height, SrcImg.type());
    m_ImageFilter.Init(SrcImg.cols, SrcImg.rows, sizeof(Ipp16s)*SrcImg.type());

	if (_Type_G8 == SrcImg.type())
	{
		Ipp16s factor = 1;
		IppiMaskSize MskSize = ippMskSize3x3;
		switch (nMaskSize)
		{
		case _3X3:
			MskSize = ippMskSize3x3;
			factor = 1;
			break;
		case _5X5:
			MskSize = ippMskSize5x5;
			factor = 8;
			break;
		default:
			break;
		}
		int npBufferSize = 0;
		IppNormType normType = ippNormL1;
		status = ippiFilterSobelGetBufferSize(roiSize, MskSize, normType, ipp8u, ipp16s, 1, &npBufferSize);
		if (check_sts(status,"KxFilterSobelImage_ippiFilterSobelGetBufferSize", hCall))
		{
			return 0;
		}
		Ipp8u* pBuffer = ippsMalloc_8u(npBufferSize);

		status = ippiFilterSobel_8u16s_C1R(SrcImg.data, SrcImg.step, (Ipp16s*)m_ImageFilter.data, m_ImageFilter.step, roiSize, MskSize, normType, ippBorderRepl, 0, pBuffer);

		if (check_sts(status,"KxFilterSobelImage_ippiFilterSobel_8u16s_C1R", hCall))
		{
			ippsFree(pBuffer);
			return 0;
		}

		ippsFree(pBuffer);
        ippiDivC_16s_C1IRSfs(factor, (Ipp16s*)m_ImageFilter.data, m_ImageFilter.step, roiSize, 0);
		ippiConvert_16s8u_C1R((Ipp16s*)m_ImageFilter.data, m_ImageFilter.step, DstImg.data, DstImg.step, roiSize);
	}
	if (_Type_G24 == SrcImg.type())
	{
		Ipp16s* pKernel[2];
		Ipp16s factor[] = {1, 1, 1};
		int nMskWidth, nMskHeight;
		switch (nMaskSize)
		{
		case _3X3:
			{
				nMskWidth = 3;
				nMskHeight = 3;
				pKernel[0] = new Ipp16s[nMskWidth*nMskHeight];
				Ipp16s pHorizTempKernel[3*3] = {1, 2, 1,
					                            0, 0, 0,
												-1, -2, -1};
				ippsCopy_16s(pHorizTempKernel, pKernel[0], nMskWidth*nMskHeight);
				pKernel[1] = new Ipp16s[nMskWidth*nMskHeight];
				Ipp16s pVertTmpKernel[3*3] = {-1, 0, 1,
					                          -2, 0, 2,
											  -1, 0, 1};
				ippsCopy_16s(pVertTmpKernel, pKernel[1], nMskWidth*nMskHeight);

				break;

			}
		case _5X5:
			{
				nMskWidth = 5;
				nMskHeight = 5;
				pKernel[0] = new Ipp16s[nMskWidth*nMskHeight];
				Ipp16s pHorizTempKernel[5*5] = {1, 4, 6, 4, 1,
					                            2, 8, 12, 8, 2,
												0, 0, 0, 0, 0,
												-2, -8, -12, -8, -2,
												-1, -4, -6, -4, -1};
				ippsCopy_16s(pHorizTempKernel, pKernel[0], nMskWidth*nMskHeight);
				pKernel[1] = new Ipp16s[nMskWidth*nMskHeight];
				Ipp16s pVertTmpKernel[5*5] = {-1, -2, 0, 2, 1,
					                          -4,-8, 0, 8, 4,
				                              -6, 12, 0, 12, 6,
				                              -4,-8, 0, 8, 4,
				                              -1, -2, 0, 2,1};
				ippsCopy_16s(pVertTmpKernel, pKernel[1], nMskWidth*nMskHeight);
				for (int i = 0; i < 3; i++)
				{
					factor[i] = 8;
				}
				break;
			}
		default:
			break;
		}

		IppiSize kernSize = {nMskWidth, nMskHeight};
		IppiSize dstRoiSize = {SrcImg.cols, SrcImg.rows};
		int nDivisor = 1;
		int nSpecSize = 0, nBufferSize = 0;
        m_ImageConvet.Init(SrcImg.cols, SrcImg.cols, sizeof(Ipp16s)*SrcImg.type());
		m_ImageTemp.Init(SrcImg.cols, SrcImg.cols, sizeof(Ipp16s)*SrcImg.type());
		ippsSet_8u(0, m_ImageFilter.data, m_ImageFilter.step*m_ImageFilter.rows);

		ippiConvert_8u16s_C3R(SrcImg.data, SrcImg.step, (Ipp16s*)m_ImageConvet.data, m_ImageConvet.step, roiSize);
		for (int i = 0; i < 2; i++)
		{
			//* Initializes the Buffer */
			status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp16s, ipp16s, 3, &nSpecSize, &nBufferSize);
			if (check_sts(status, "KxFilterSobelImage_ippiFilterBorderGetSize", hCall))
			{
				return 0;
			}
			IppiFilterBorderSpec* pSepc = (IppiFilterBorderSpec*)ippsMalloc_8u(nSpecSize);
			Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

			// * Initializes the filter specification structure */
			status = ippiFilterBorderInit_16s(pKernel[i], kernSize, nDivisor, ipp16s, 3, ippRndNear, pSepc);
			if (check_sts(status, "KxFilterSobelImage_ippiFilterBorderInit_16s", hCall))
			{
				ippsFree(pSepc);
				ippsFree(pBuffer);
				return 0;
			}

			//Filters an image */
			IppiBorderType borderType = ippBorderRepl;
			Ipp16s borderValue[3] = {0, 0, 0};
			status = ippiFilterBorder_16s_C3R((Ipp16s*)m_ImageConvet.data, m_ImageConvet.step, (Ipp16s*)m_ImageTemp.data, m_ImageTemp.step, dstRoiSize, borderType, borderValue, pSepc, pBuffer);

			if (check_sts(status, "KxFilterSobelImage_ippiFilterBorder_16s_C3R", hCall))
			{
				return 0;
			}

			ippiAbs_16s_C3IR((Ipp16s*)m_ImageTemp.data, m_ImageTemp.step, roiSize);
			ippiAdd_16s_C3IRSfs((Ipp16s*)m_ImageTemp.data, m_ImageTemp.step, (Ipp16s*)m_ImageFilter.data, m_ImageFilter.step, roiSize, 0);

			delete [] pKernel[i];
			pKernel[i] = NULL;
		}
		ippiDivC_16s_C3IRSfs(factor, (Ipp16s*)m_ImageFilter.data, m_ImageFilter.step, roiSize, 0);
		ippiConvert_16s8u_C3R((Ipp16s*)m_ImageFilter.data, m_ImageFilter.step, DstImg.data, DstImg.step, roiSize);
	}

	return 1;
}

int CKxBaseFunction::KxGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxGeneralFilterImage8u(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());

	Ipp16s* pKernel = NULL;
	pKernel = ippsMalloc_16s(nMaskWidth*nMaskHeight);
	if (NULL == pMask)
	{
		ippsSet_16s(1, pKernel, nMaskWidth*nMaskHeight);
	}
	else
	{
		ippsCopy_16s(pMask, pKernel, nMaskWidth*nMaskHeight);
	}

	IppiSize kernSize = { nMaskWidth, nMaskHeight };
	IppiSize dstRoiSize = { SrcImg.cols, SrcImg.rows };
	//Ipp16s pSum;
	//ippsSum_16s_Sfs(pMask, nMaskWidth*nMaskHeight, &pSum, 0);
	int nDivisor = 1;
	int nSpecSize = 0, nBufferSize = 0;

	//* Initializes the Buffer */
	status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, SrcImg.type(), &nSpecSize, &nBufferSize);
	if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderGetSize", hCall))
	{
		ippsFree(pKernel);
		return 0;
	}
	IppiFilterBorderSpec* pSepc = (IppiFilterBorderSpec*)ippsMalloc_8u(nSpecSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

	// * Initializes the filter specification structure */
	status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, SrcImg.type(), ippRndNear, pSepc);
	if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderInit_16s", hCall))
	{
		ippsFree(pKernel);
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}

	//Filters an image */
	IppiBorderType borderType = ippBorderRepl;
	if (SrcImg.type() == _Type_G8)
	{
		Ipp8u borderValue = 0;
		status = ippiFilterBorder_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
		if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorder_8u_C1R", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}


	}
	if (SrcImg.type() == _Type_G24)
	{
		Ipp8u borderValue[3] = { 0, 0, 0 };
		status = ippiFilterBorder_8u_C3R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
		if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorder_8u_C3R", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}

	}


	ippsFree(pKernel);
	ippsFree(pSepc);
	ippsFree(pBuffer);

	return 1;

}

int CKxBaseFunction::KxParallelGeneralFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxParallelGeneralFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxParallelGeneralFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());

	Ipp16s* pKernel = NULL;
	pKernel = ippsMalloc_16s(nMaskWidth*nMaskHeight);
	if (NULL == pMask)
	{
		ippsSet_16s(1, pKernel, nMaskWidth*nMaskHeight);
	}
	else
	{
		ippsCopy_16s(pMask, pKernel, nMaskWidth*nMaskHeight);
	}

	IppiSize kernSize = { nMaskWidth, nMaskHeight };
	IppiSize RoiSize = { SrcImg.cols, SrcImg.rows };


	m_Image16s.Init(SrcImg.cols, SrcImg.rows, SrcImg.type() * sizeof(Ipp16s));
	m_Image16sAbs.Init(SrcImg.cols, SrcImg.rows, SrcImg.type() * sizeof(Ipp16s));


	int nRowActualGrain = gMin(nRowGrain, DstImg.rows);
	int nColActualGrain = gMin(nColGrain, DstImg.cols);

	Ipp8u* pSrc = m_Image16s.data;
	Ipp8u* pAbs = m_Image16sAbs.data;
	int nSrcPitch = m_Image16s.step;

	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, nRowActualGrain, 0, SrcImg.cols, nColActualGrain),
		[&SrcImg, pSrc, &nSrcPitch, pAbs, &DstImg, &kernSize, pKernel ](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

 		Ipp8u* pOrigT = SrcImg.data + range.rows().begin() * SrcImg.step + range.cols().begin() * SrcImg.type();
		Ipp8u* pSrcT = pSrc + range.rows().begin() * nSrcPitch + range.cols().begin() * DstImg.type()*sizeof(Ipp16s);
		Ipp8u* pAbsT = pAbs + range.rows().begin() * nSrcPitch + range.cols().begin() * DstImg.type()*sizeof(Ipp16s);
		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();

		//* Convert 8u image to 16s */
		if (_Type_G8 == DstImg.type())
		{
			status = ippiConvert_8u16s_C1R(pOrigT, SrcImg.step, (Ipp16s*)pSrcT, nSrcPitch, dstRoiSize);
		}
		else if (_Type_G24 == DstImg.type())
		{
			status = ippiConvert_8u16s_C3R(pOrigT, SrcImg.step, (Ipp16s*)pSrcT, nSrcPitch, dstRoiSize);
		}
		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp16s, ipp16s, DstImg.type(), &nSpecSize, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiFilterBorderSpec *pSepc = (IppiFilterBorderSpec*)scalable_aligned_malloc(nSpecSize*sizeof(Ipp8u), 32);

		// * Initializes the filter specification structure */
		int nDivisor = 1;
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp16s, DstImg.type(), ippRndNear, pSepc);

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (DstImg.type() == _Type_G8)
		{
			Ipp16s borderValue = 0;
			status = ippiFilterBorder_16s_C1R((Ipp16s*)pSrcT, nSrcPitch, (Ipp16s*)pAbsT, nSrcPitch, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
			ippiAbs_16s_C1IR((Ipp16s*)pAbsT, nSrcPitch, dstRoiSize);
			ippiConvert_16s8u_C1R((Ipp16s*)pAbsT, nSrcPitch, pDstT, DstImg.step, dstRoiSize);

		}
		if (DstImg.type() == _Type_G24)
		{
			Ipp16s borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_16s_C3R((Ipp16s*)pSrcT, nSrcPitch, (Ipp16s*)pAbsT, nSrcPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
			ippiAbs_16s_C3IR((Ipp16s*)pAbsT, nSrcPitch, dstRoiSize);
			ippiConvert_16s8u_C3R((Ipp16s*)pAbsT, nSrcPitch, pDstT, DstImg.step, dstRoiSize);
		}

		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pSepc);

	}, simple_partitioner());

	ippsFree(pKernel);

	return 1;

}

int CKxBaseFunction::KxParallelGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxParallelGeneralFilterImage8u(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxParallelGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());

	Ipp16s* pKernel = NULL;
	pKernel = ippsMalloc_16s(nMaskWidth*nMaskHeight);
	if (NULL == pMask)
	{
		ippsSet_16s(1, pKernel, nMaskWidth*nMaskHeight);
	}
	else
	{
		ippsCopy_16s(pMask, pKernel, nMaskWidth*nMaskHeight);
	}

	IppiSize kernSize = { nMaskWidth, nMaskHeight };
	IppiSize RoiSize = { SrcImg.cols, SrcImg.rows };

	int nRowActualGrain = gMin(nRowGrain, DstImg.rows);
	int nColActualGrain = gMin(nColGrain, DstImg.cols);

	Ipp8u* pSrc = m_Image16s.data;
	Ipp8u* pAbs = m_Image16sAbs.data;
	int nSrcPitch = m_Image16s.step;


	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, 0, SrcImg.cols),
		[&SrcImg, pSrc, &nSrcPitch, pAbs, &DstImg, &kernSize, pKernel](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.data + range.rows().begin() * SrcImg.step + range.cols().begin() * SrcImg.type();

		Ipp8u* pDstT = DstImg.data + range.rows().begin() * DstImg.step + range.cols().begin() * DstImg.type();


		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, DstImg.type(), &nSpecSize, &nBufferSize);


		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiFilterBorderSpec *pSepc = (IppiFilterBorderSpec*)scalable_aligned_malloc(nSpecSize*sizeof(Ipp8u), 32);

		// * Initializes the filter specification structure */
		int nDivisor = 1;
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, DstImg.type(), ippRndNear, pSepc);



		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (DstImg.type() == _Type_G8)
		{
			Ipp8u borderValue[1] = { 0 };
			status = ippiFilterBorder_8u_C1R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, borderType, borderValue, pSepc, pBuffer);


		}
		if (DstImg.type() == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_8u_C3R(pOrigT, SrcImg.step, pDstT, DstImg.step, dstRoiSize, borderType, borderValue, pSepc, pBuffer);

		}

		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pSepc);

	}, auto_partitioner());



	ippsFree(pKernel);


	return 1;

}

int  CKxBaseFunction::KxHistogram(InputArray SrcImg, int nBins, Ipp32u nHist[])
{
	KxCallStatus hCall;
	return KxHistogram(SrcImg, nBins, nHist, hCall);
}

int  CKxBaseFunction::KxHistogram( InputArray SrcImg,  int nBins, Ipp32u nHist[], KxCallStatus& hCall )
{//直方图统计-------->
	IppiSize roiSize = {SrcImg.cols, SrcImg.rows};
	//* get sizes for spec and buffer */
	IppStatus status;
	//const int nBins = 256;
	int nLevels[] = {nBins + 1, nBins+1, nBins+1};
	Ipp32f lowerLevel[] = {0, 0, 0};
	Ipp32f upperLevel[] = {255, 255, 255};
	int nSizeHistObj, nSizeBuffer;
	status = ippiHistogramGetBufferSize(ipp8u, roiSize, nLevels, SrcImg.type(), 1, &nSizeHistObj, &nSizeBuffer);
	if (check_sts(status, "KxHistogram_ippiHistogramGetBufferSize", hCall))
	{
		return 0;
	}

	IppiHistogramSpec* pHistObj = (IppiHistogramSpec* )ippsMalloc_8u(nSizeHistObj);
	Ipp8u* pBuffer = ippsMalloc_8u(nSizeBuffer);
	//initialize spec */
	status = ippiHistogramUniformInit(ipp8u, lowerLevel, upperLevel, nLevels, SrcImg.type(), pHistObj);
	if (check_sts(status, "KxHistogram_ippiHistogramUniformInit", hCall))
	{
		ippsFree(pBuffer);
		ippsFree(pHistObj);
		return 0;
	}


	//check levels of bins
	//Ipp32f* pLevels = ippsMalloc_32f(nBins + 1);
	Ipp32f* ppLevels[3];
    for(int i = 0; i < 3; i++)
		ppLevels[i] = ippsMalloc_32f(nBins + 1);
	status = ippiHistogramGetLevels(pHistObj, ppLevels);
	if (check_sts(status, "KxHistogram_ippiHistogramGetLevels", hCall))
	{
		ippsFree(pBuffer);
		ippsFree(pHistObj);
		return 0;
	}
	for(int i = 0; i < 3; i++)
		ippsFree(ppLevels[i]);


	//calculate histogram
	if (_Type_G8 == SrcImg.type())
	{
		Ipp32u* pHistVec = ippsMalloc_32u(nBins);
		status = ippiHistogram_8u_C1R(SrcImg.data, SrcImg.step, roiSize, pHistVec, pHistObj, pBuffer);
		ippsCopy_8u((Ipp8u*)pHistVec, (Ipp8u*)nHist, sizeof(Ipp32u)*nBins);
		ippsFree(pHistVec);
	}
	if (_Type_G24 == SrcImg.type())
	{
		Ipp32u* pHistVec[3];
		for (int i = 0; i < 3; i++)
		{
			pHistVec[i] = ippsMalloc_32u(nBins);
		}
		status = ippiHistogram_8u_C3R(SrcImg.data, SrcImg.step, roiSize, pHistVec, pHistObj, pBuffer);
		int nStart = 0;
		for (int i = 0; i < 3; i++)
		{
			ippsCopy_8u((Ipp8u*)pHistVec[i], (Ipp8u*)nHist + nStart, sizeof(Ipp32u)*nBins);
			ippsFree(pHistVec[i]);
			pHistVec[i] = NULL;
			nStart += sizeof(Ipp32u)*nBins;
		}
	}

	if (check_sts(status, "KxHistogram_ippiHistogram_8u", hCall))
	{
		ippsFree(pBuffer);
		ippsFree(pHistObj);
		return 0;
	}


	ippsFree(pBuffer);
	ippsFree(pHistObj);

	return 1;

}

int CKxBaseFunction::KxProjectImage(InputArray SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale)
{
	KxCallStatus hCall;
	return KxProjectImage(SrcImg, nDir, fProject, fScale, hCall);
}

int CKxBaseFunction::KxProjectImage(InputArray SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale, KxCallStatus& hCall)
{
	IppStatus status;
	if (SrcImg.type() != _Type_G8)
	{
		if (check_sts(-10000, "KxProjectImage_Image_TypeErr", hCall))
		{
			return 0;
		}
	}

	if (nDir == _Horizontal_Project_Dir)
	{
		IppiSize dstRoi = { 1, SrcImg.rows};
		int anchor = (SrcImg.cols - 1) / 2;
		status = ippiSumWindowRow_8u32f_C1R(SrcImg.data + anchor, SrcImg.step, fProject, sizeof(Ipp32f), dstRoi, SrcImg.cols, anchor);
		if (check_sts(status, "KxProjectImage_ippiSumWindowRow_8u32f_C1R", hCall))
		{
			return 0;
		}
		ippsDivC_32f_I(fScale, fProject, SrcImg.rows);
	}
	if (nDir == _Vertical_Project_Dir)
	{
		IppiSize dstRoi = {SrcImg.cols, 1 };
		int anchor = (SrcImg.rows - 1) / 2;
		status = ippiSumWindowColumn_8u32f_C1R(SrcImg.data + anchor * SrcImg.step, SrcImg.step, fProject, SrcImg.cols*sizeof(Ipp32f), dstRoi, SrcImg.rows, anchor);
		if (check_sts(status, "KxProjectImage_ippiSumWindowColumn_8u32f_C1R", hCall))
		{
			return 0;
		}
		ippsDivC_32f_I(fScale, fProject, SrcImg.cols);
	}



	return 1;
}

int CKxBaseFunction::KxAddImage(InputArray SrcImg, OutputArray DstImg)
{
	KxCallStatus hCall;
	return KxAddImage(SrcImg, DstImg, hCall);
}

int CKxBaseFunction::KxAddImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	IppiSize roiSize = { SrcImg.cols, SrcImg.rows };
	IppStatus status;
	if (SrcImg.type() == _Type_G8)
	{
		status = ippiAdd_8u_C1IRSfs(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, 0);
		if (check_sts(status, "KxAddImage_ippiAdd_8u_C1IRSfs", hCall))
		{
			return 0;
		}
	}
	if (SrcImg.type() == _Type_G24)
	{
		status = ippiAdd_8u_C3IRSfs(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, 0);
		if (check_sts(status, "KxAddImage_ippiAdd_8u_C1IRSfs", hCall))
		{
			return 0;
		}
	}

	return 1;
}

int CKxBaseFunction::KxSubCImage(InputArray SrcImg, OutputArray DstImg, Ipp8u value)
{
	KxCallStatus hCall;
	return KxSubCImage(SrcImg, DstImg, value, hCall);
}

int CKxBaseFunction::KxSubCImage(InputArray SrcImg, OutputArray DstImg, Ipp8u value, KxCallStatus& hCall)
{
	IppiSize roiSize = { SrcImg.cols, SrcImg.rows };
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());


	IppStatus status;
	if (SrcImg.type() == _Type_G8)
	{
		status = ippiSubC_8u_C1RSfs(SrcImg.data, SrcImg.step, value, DstImg.data, DstImg.step, roiSize, 0);
		if (check_sts(status, "KxSubCImage_ippiSubC_8u_C1RSfs", hCall))
		{
			return 0;
		}
	}
	if (SrcImg.type() == _Type_G24)
	{
		Ipp8u nValue[3];
		for (int i = 0; i < 3; i++)
	    {
			nValue[i] = value;
	    }
		status = ippiSubC_8u_C3RSfs(SrcImg.data, SrcImg.step, nValue, DstImg.data, DstImg.step, roiSize, 0);
		if (check_sts(status, "KxSubCImage_ippiSubC_8u_C3RSfs", hCall))
		{
			return 0;
		}
	}

	return 1;
}

int CKxBaseFunction::KxImageMaxEvery(InputArray SrcImg, OutputArray DstImg)
{
	KxCallStatus hCall;
	return KxImageMaxEvery(SrcImg, DstImg, hCall);

}

int CKxBaseFunction::KxImageMaxEvery(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	IppiSize size = { SrcImg.cols, SrcImg.rows };
	DstImg.Init(SrcImg.cols, SrcImg.rows);
	if (SrcImg.type() == _Type_G24)
	{
		status = ippiGradientColorToGray_8u_C3C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, size, ippiNormInf);
		if (check_sts(status, "KxImageMaxEvery_ippiGradientColorToGray_8u_C3C1R", hCall))
		{
			return 0;
		}
	}
	if (SrcImg.type() == _Type_G8)
	{
		status = ippiCopy_8u_C1R(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, size);
		if (check_sts(status, "KxImageMaxEvery_ippiCopy_8u_C1R", hCall))
		{
			return 0;
		}

	}
	return 1;

}

int CKxBaseFunction::KxZhuGuangCardFilter(InputArray SrcImg, OutputArray DstImg)
{
	KxCallStatus hCall;
	return KxZhuGuangCardFilter(SrcImg, DstImg, hCall);
}

int CKxBaseFunction::KxZhuGuangCardFilter(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	//
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	IppStatus status;
	if (SrcImg.type() != _Type_G8)
	{
		status = IppStatus(kxImageTypeErr);
		if (check_sts(status, "KxZhuGuangCardFilter_Image_TypeErr", hCall))
		{
			return 0;
		}

	}
	DstImg.Init(SrcImg.cols / 4, SrcImg.rows / 4);


	m_Img16s.Init(SrcImg.cols, SrcImg.rows, sizeof(Ipp16s));

	IppiSize roiSize = { SrcImg.cols, SrcImg.rows };
	int nBufferSize;
	status = ippiFilterLaplacianGetBufferSize_8u16s_C1R(roiSize, ippMskSize5x5, &nBufferSize);
	if (check_sts(status, "KxZhuGuangCardFilter_ippiFilterLaplacian", hCall))
	{
		return 0;
	}
	Ipp8u* pBuffer = new Ipp8u[nBufferSize];
	status = ippiFilterLaplacianBorder_8u16s_C1R(SrcImg.data, SrcImg.step, (Ipp16s*)m_Img16s.data, m_Img16s.step, roiSize, ippMskSize5x5, ippBorderConst, 0, pBuffer);
	delete[]pBuffer;

	if (check_sts(status, "KxZhuGuangCardFilter_ippiFilterLaplacianBorder", hCall))
	{
		return 0;
	}

	ippiAbs_16s_C1IR((Ipp16s*)m_Img16s.data, m_Img16s.step, roiSize);
	ippiDivC_16s_C1IRSfs(3, (Ipp16s*)m_Img16s.data, m_Img16s.step, roiSize, 0);
	m_ImgLapFilter.Init(SrcImg.cols, SrcImg.rows);
	ippiConvert_16s8u_C1R((Ipp16s*)m_Img16s.data, m_Img16s.step, m_ImgLapFilter.data, m_ImgLapFilter.step, roiSize);



	KxDilateImage(m_ImgLapFilter, m_ImgDilate, 5, 5, NULL, hCallInfo);

	if (check_sts(hCallInfo, "KxZhuGuangCardFilter_", hCall))
	{
		return 0;
	}

	m_ImgResize.Init(SrcImg.cols / 4, SrcImg.rows / 4);
	KxResizeImage(m_ImgDilate, m_ImgResize, KxSuper, hCallInfo);

	if (check_sts(hCallInfo, "KxZhuGuangCardFilter_", hCall))
	{
		return 0;
	}


	KxMedianFilterImage(m_ImgResize, DstImg, 5, 5, hCallInfo);

	if (check_sts(hCallInfo, "KxZhuGuangCardFilter_", hCall))
	{
		return 0;
	}




	return 1;


}

int CKxBaseFunction::KxFFtCheckPeriodic(InputArray SrcImg, OutputArray DstImg)
{
	KxCallStatus hCall;
	return KxFFtCheckPeriodic(SrcImg, DstImg, hCall);
}

int CKxBaseFunction::KxFFtCheckPeriodic(const InputArray & SrcImg, OutputArray DstImg, KxCallStatus& hCall )
{
	IppStatus status;
	m_ImgCopy32f.Init(SrcImg.cols, SrcImg.rows);
	IppiSize Roi = { SrcImg.cols, SrcImg.rows };
	status = ippiConvert_8u32f_C1R(SrcImg.data, SrcImg.step, m_ImgCopy32f.data, m_ImgCopy32f.step, Roi);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiConvert_8u32f_C1R", hCall))
	{
		return 0;
	}

	int nSepSize, nSizeInit, nSizeBuf;
	status = ippiDFTGetSize_R_32f(Roi, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, &nSepSize, &nSizeInit, &nSizeBuf);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTGetSize_R_32f", hCall))
	{
		return 0;
	}

	/* memory allocation */
	IppiDFTSpec_R_32f * pSepc;
	pSepc = (IppiDFTSpec_R_32f*)ippMalloc(nSepSize);
	Ipp8u* pBuffer, *pMemInit;
	pBuffer = (Ipp8u*)ippMalloc(nSizeBuf);
	pMemInit = (Ipp8u*)ippMalloc(nSizeInit);

	status = ippiDFTInit_R_32f(Roi, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, pSepc, pMemInit);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTInit_R_32f", hCall))
	{
		return 0;
	}

	m_ImgFFt.Init(SrcImg.cols, SrcImg.rows);
	/* forward FFT transform : the source image is transformed into the frequency domain*/
	status = ippiDFTFwd_RToPack_32f_C1R(m_ImgCopy32f.data, m_ImgCopy32f.step, m_ImgFFt.data, m_ImgFFt.step, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTFwd_RToPack_32f_C1R", hCall))
	{
		return 0;
	}

	m_ImgFilter32fc.Init(SrcImg.cols, SrcImg.rows);
	Ipp32fc value = { 1.0, 0 };
	ippsSet_32fc(value, m_ImgFilter32fc.data, m_ImgFilter32fc.cols * m_ImgFilter32fc.rows);
	//
	const int nPassW = 30;
	const int nPassH = 18;

	for (int i = nPassH; i < SrcImg.rows - nPassH; i++)
	{
		for (int j = 0; j < nPassW; j++)
		{
			m_ImgFilter32fc.data[i * m_ImgFilter32fc.cols + j] = { 0, 0 };
		}
		for (int j = SrcImg.cols - nPassW; j < SrcImg.cols; j++)
		{
			m_ImgFilter32fc.data[i * m_ImgFilter32fc.cols + j] = { 0, 0 };
		}
	}


	m_ImgFFtFilter.Init(SrcImg.cols, SrcImg.rows);
	status = ippiCplxExtendToPack_32fc32f_C1R(m_ImgFilter32fc.data, m_ImgFilter32fc.step, Roi, m_ImgFFtFilter.data, m_ImgFFtFilter.step);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiCplxExtendToPack_32fc32f_C1R", hCall))
	{
		return 0;
	}


	/* multiplying the packed data */
	status = ippiMulPack_32f_C1IR(m_ImgFFt.data, m_ImgFFt.step, m_ImgFFtFilter.data, m_ImgFFtFilter.step, Roi);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiMulPack_32f_C1IR", hCall))
	{
		return 0;
	}


	/* inverse FFT transform : the filtered data is transformed to the time domain */
	m_ImgFFtResult.Init(SrcImg.cols, SrcImg.rows);
	status = ippiDFTInv_PackToR_32f_C1R(m_ImgFFtFilter.data, m_ImgFFtFilter.step, m_ImgFFtResult.data, m_ImgFFtResult.step, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTInv_PackToR_32f_C1R", hCall))
	{
		return 0;
	}

	DstImg.Init(SrcImg.cols, SrcImg.rows);
	status = ippiConvert_32f8u_C1R(m_ImgFFtResult.data, m_ImgFFtResult.step, DstImg.data, DstImg.step, Roi, ippRndZero);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiConvert_32f8u_C1R", hCall))
	{
		return 0;
	}


	ippsFree(pSepc);
	ippsFree(pBuffer);
	ippsFree(pMemInit);


	return 1;
}

int CKxBaseFunction::KxFFtCheckPeriodic1(InputArray SrcImg, OutputArray DstImg)
{
	KxCallStatus hCall;
	return KxFFtCheckPeriodic1(SrcImg, DstImg, hCall);
}

int CKxBaseFunction::KxFFtCheckPeriodic1(const InputArray & SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	bool bFlag[2];
	bFlag[0] = (((SrcImg.cols)&(SrcImg.cols - 1)) == 0);
	bFlag[1] = (((SrcImg.rows)&(SrcImg.rows - 1)) == 0);
	if (!bFlag[0] || !bFlag[1])
	{
		IppStatus status = IppStatus(kxImageTypeErr);
		if (check_sts(status, "KxFFtCheckPeriodic1_ImageSizeMustbePowerOf2", hCall))
		{
			return 0;
		}
	}

	int nOrderX = int(log(double(SrcImg.cols))/log(2.0));
	int nOrderY = int(log(double(SrcImg.rows))/log(2.0));

	IppStatus status;
	m_ImgCopy32f.Init(SrcImg.cols, SrcImg.rows);
	IppiSize Roi = { SrcImg.cols, SrcImg.rows };
	status = ippiConvert_8u32f_C1R(SrcImg.data, SrcImg.step, m_ImgCopy32f.data, m_ImgCopy32f.step, Roi);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiConvert_8u32f_C1R", hCall))
	{
		return 0;
	}

	int nSepSize, nSizeInit, nSizeBuf;
	status = ippiFFTGetSize_R_32f(nOrderX, nOrderY, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, &nSepSize, &nSizeInit, &nSizeBuf);
	if (check_sts(status, "KxFFtCheckPeriodic1_ippiFFTGetSize_R_32f", hCall))
	{
		return 0;
	}

	/* memory allocation */
	IppiFFTSpec_R_32f * pSepc;
	pSepc = (IppiFFTSpec_R_32f*)ippMalloc(nSepSize);
	Ipp8u* pBuffer, *pMemInit;
	pBuffer = (Ipp8u*)ippMalloc(nSizeBuf);
	pMemInit = (Ipp8u*)ippMalloc(nSizeInit);

	status = ippiFFTInit_R_32f(nOrderX, nOrderY, IPP_FFT_DIV_INV_BY_N, ippAlgHintNone, pSepc, pMemInit);
	if (check_sts(status, "KxFFtCheckPeriodic1_ippiFFTInit_R_32f", hCall))
	{
		return 0;
	}

	m_ImgFFt.Init(SrcImg.cols, SrcImg.rows);
	/* forward FFT transform : the source image is transformed into the frequency domain*/
	status = ippiFFTFwd_RToPack_32f_C1R(m_ImgCopy32f.data, m_ImgCopy32f.step, m_ImgFFt.data, m_ImgFFt.step, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic1_ippiFFTFwd_RToPack_32f_C1R", hCall))
	{
		return 0;
	}

	m_ImgFilter32fc.Init(SrcImg.cols, SrcImg.rows);
	Ipp32fc value = { 1.0, 0 };
	ippsSet_32fc(value, m_ImgFilter32fc.data, m_ImgFilter32fc.cols * m_ImgFilter32fc.rows);
	//
	const int nPassW = 15;
	const int nPassH = 50;

	for (int i = nPassH; i < SrcImg.rows - nPassH; i++)
	{
		for (int j = 0; j < nPassW; j++)
		{
			m_ImgFilter32fc.data[i * m_ImgFilter32fc.cols + j] = { 0, 0 };
		}
		for (int j = SrcImg.cols - nPassW; j < SrcImg.cols; j++)
		{
			m_ImgFilter32fc.data[i * m_ImgFilter32fc.cols + j] = { 0, 0 };
		}
	}


	m_ImgFFtFilter.Init(SrcImg.cols, SrcImg.rows);
	status = ippiCplxExtendToPack_32fc32f_C1R(m_ImgFilter32fc.data, m_ImgFilter32fc.step, Roi, m_ImgFFtFilter.data, m_ImgFFtFilter.step);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiCplxExtendToPack_32fc32f_C1R", hCall))
	{
		return 0;
	}


	/* multiplying the packed data */
	status = ippiMulPack_32f_C1IR(m_ImgFFt.data, m_ImgFFt.step, m_ImgFFtFilter.data, m_ImgFFtFilter.step, Roi);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiMulPack_32f_C1IR", hCall))
	{
		return 0;
	}


	/* inverse FFT transform : the filtered data is transformed to the time domain */
	m_ImgFFtResult.Init(SrcImg.cols, SrcImg.rows);
	status = ippiFFTInv_PackToR_32f_C1R(m_ImgFFtFilter.data, m_ImgFFtFilter.step, m_ImgFFtResult.data, m_ImgFFtResult.step, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTInv_PackToR_32f_C1R", hCall))
	{
		return 0;
	}

	DstImg.Init(SrcImg.cols, SrcImg.rows);
	status = ippiConvert_32f8u_C1R(m_ImgFFtResult.data, m_ImgFFtResult.step, DstImg.data, DstImg.step, Roi, ippRndZero);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiConvert_32f8u_C1R", hCall))
	{
		return 0;
	}


	ippsFree(pSepc);
	ippsFree(pBuffer);
	ippsFree(pMemInit);


	return 1;
}

int CKxBaseFunction::KxFilterSpeckles(InputArray SrcImg, OutputArray DstImg, int nMaxSpeckleSize)
{
	KxCallStatus hCall;
	return KxFilterSpeckles(SrcImg, DstImg, nMaxSpeckleSize, hCall);
}


int CKxBaseFunction::KxFilterSpeckles(InputArray SrcImg, OutputArray DstImg, int nMaxSpeckleSize, KxCallStatus& hCall)
{
	if (SrcImg.type() != _Type_G8)
	{
		return 0;
	}
	IppiSize roiSize = { SrcImg.cols, SrcImg.rows };
	IppStatus status;
	int nBufferSize = 0;
	status = ippiMarkSpecklesGetBufferSize(roiSize, ipp8u, SrcImg.type(), &nBufferSize);
	if (check_sts(status, "KxFilterSpeckles_ippiMarkSpecklesGetBufferSize", hCall))
	{
		return 0;
	}
	m_MarkImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
	ippiThreshold_GTVal_8u_C1R(SrcImg.data, SrcImg.step, m_MarkImg.data, m_MarkImg.step, roiSize, 0, 20);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	//tick_count tbb_start, tbb_end;
	//tbb_start = tick_count::now();
	status = ippiMarkSpeckles_8u_C1IR(m_MarkImg.data, m_MarkImg.step, roiSize, 255, nMaxSpeckleSize, 10, ippiNormL1, pBuffer);
	//tbb_end = tick_count::now();
	//printf("KxFilterSpeckles filter cost: %f ms\n", (tbb_end - tbb_start).seconds());
	if (check_sts(status, "KxFilterSpeckles_ippiMarkSpeckles_8u_C1IR", hCall))
	{
		return 0;
	}

	DstImg.Init(SrcImg.cols, SrcImg.rows);
	KxThreshImage(m_MarkImg, DstImg, 1, 254);
	ippiAnd_8u_C1IR(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize);

	ippsFree(pBuffer);

	return 1;
}

int CKxBaseFunction::KxFillHoles(InputArray SrcImg, OutputArray DstImg)
{
	KxCallStatus hCall;
	return KxFillHoles(SrcImg, DstImg, hCall);
}

int CKxBaseFunction::KxFillHoles(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	IppStatus status;
	if (SrcImg.type() != _Type_G8)
	{
		status = IppStatus(kxImageTypeErr);
		if (check_sts(status, "KxFillHoles_Image_TypeErr", hCall))
		{
			return 0;
		}

	}
	DstImg.Init(SrcImg.cols, SrcImg.rows);
	IppiSize roiSize = { SrcImg.cols, SrcImg.rows };
	InputArray m_InvertImg, m_MarkerImg;
	m_InvertImg.Init(SrcImg.cols, SrcImg.rows);
	ippsSet_8u(0xff, m_InvertImg.data, m_InvertImg.step * m_InvertImg.rows);
	//图像取反
	ippiSub_8u_C1IRSfs(SrcImg.data, SrcImg.step, m_InvertImg.data, m_InvertImg.step, roiSize, 0);
	m_MarkerImg.Init(SrcImg.cols, SrcImg.rows);
	ippsSet_8u(0xff, m_MarkerImg.data, m_MarkerImg.step * m_MarkerImg.rows);
	IppiSize Roi = { roiSize.width - 2, roiSize.height - 2 };
	ippiSet_8u_C1R(0, m_MarkerImg.data + 1 + m_MarkerImg.step, m_MarkerImg.step, Roi);
	int size = 0;
	status = ippiMorphReconstructGetBufferSize(roiSize, ipp8u, 1, &size);
	if (check_sts(status, "KxFillHoles_ippiMorphReconstructGetBufferSize", hCall))
	{
		return 0;
	}

	Ipp8u* pBuf = ippsMalloc_8u(size);
	status = ippiMorphReconstructDilate_8u_C1IR(m_InvertImg.data, m_InvertImg.step, m_MarkerImg.data, m_MarkerImg.step, roiSize, pBuf, (IppiNorm)ippiNormL1);
	if (check_sts(status, "KxFillHoles_ippiMorphReconstructDilate_8u_C1IR", hCall))
	{
		return 0;
	}

	ippsFree(pBuf);
	ippsSet_8u(0xff, DstImg.data, DstImg.step * DstImg.rows);
	//图像取反
	ippiSub_8u_C1IRSfs(m_MarkerImg.data, m_MarkerImg.step, DstImg.data, DstImg.step, roiSize, 0);

	return 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int CKxBaseFunction::KxParallelDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxParallelDilateImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}



//对于比较小的图并行速度更慢
int CKxBaseFunction::KxParallelDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;

	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	int nRowActualGrain = gMin(nRowGrain, DstImg.rows);
	int nColActualGrain = gMin(nColGrain, DstImg.cols);
	IppiSize maskSize = { nMaskWidth, nMaskHeight };
	//static_partitioner app;
	Ipp8u* pSrc = SrcImg.data;
	Ipp8u* pDst = DstImg.data;
	int nChannel = SrcImg.type();
	int nSrcPitch = SrcImg.step;

	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, 0, SrcImg.cols),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiMorphState* pMorphSpec = NULL;
		Ipp8u* pBuffer = NULL;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };
		int npSpecSize, nBufferSize;
		Ipp8u* pOrigT = pSrc + range.rows().begin() * nSrcPitch + range.cols().begin() * nChannel;
		Ipp8u* pDstT = pDst + range.rows().begin() * nSrcPitch + range.cols().begin() * nChannel;

		if (nChannel == _Type_G8)
		{
			status = ippiMorphologyBorderGetSize_8u_C1R(dstRoiSize, maskSize, &npSpecSize, &nBufferSize);
			pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
			pMorphSpec = (IppiMorphState*)scalable_aligned_malloc(npSpecSize*sizeof(Ipp8u), 32);
			status = ippiMorphologyBorderInit_8u_C1R(dstRoiSize, pMask, maskSize, pMorphSpec, pBuffer);

			IppiBorderType borderType = ippBorderRepl;
			status = ippiDilateBorder_8u_C1R(pOrigT, nSrcPitch, pDstT, nSrcPitch, dstRoiSize,
				borderType, 0, pMorphSpec, pBuffer);

		}
		else if (nChannel == _Type_G24)
		{
			status = ippiMorphologyBorderGetSize_8u_C3R(dstRoiSize, maskSize, &npSpecSize, &nBufferSize);
			pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
			pMorphSpec = (IppiMorphState*)scalable_aligned_malloc(npSpecSize*sizeof(Ipp8u), 32);
			status = ippiMorphologyBorderInit_8u_C3R(dstRoiSize, pMask, maskSize, pMorphSpec, pBuffer);

			IppiBorderType borderType = ippBorderRepl;
			Ipp8u bordervalue[3] = { 0, 0, 0 };

			status = ippiDilateBorder_8u_C3R(pOrigT, nSrcPitch, pDstT, nSrcPitch, dstRoiSize,
				borderType, bordervalue, pMorphSpec, pBuffer);

		}
		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pMorphSpec);

	}, auto_partitioner());

	if (bMaskNotInit)
	{
		ippsFree(pMask);
	}


	return 1;
}



int CKxBaseFunction::KxParallelErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxParallelErodeImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}



int CKxBaseFunction::KxParallelErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;

	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	int nRowActualGrain = gMin(nRowGrain, DstImg.rows);
	int nColActualGrain = gMin(nColGrain, DstImg.cols);
	IppiSize maskSize = { nMaskWidth, nMaskHeight };
	//static_partitioner app;
	Ipp8u* pSrc = SrcImg.data;
	Ipp8u* pDst = DstImg.data;
	int nChannel = SrcImg.type();
	int nSrcPitch = SrcImg.step;

	parallel_for(blocked_range2d<int, int>(0, SrcImg.rows, 0, SrcImg.cols),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiMorphState* pMorphSpec = NULL;
		Ipp8u* pBuffer = NULL;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };
		int npSpecSize, nBufferSize;
		Ipp8u* pOrigT = pSrc + range.rows().begin() * nSrcPitch + range.cols().begin() * nChannel;
		Ipp8u* pDstT = pDst + range.rows().begin() * nSrcPitch + range.cols().begin() * nChannel;

		if (nChannel == _Type_G8)
		{
			status = ippiMorphologyBorderGetSize_8u_C1R(dstRoiSize, maskSize, &npSpecSize, &nBufferSize);
			pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
			pMorphSpec = (IppiMorphState*)scalable_aligned_malloc(npSpecSize*sizeof(Ipp8u), 32);
			status = ippiMorphologyBorderInit_8u_C1R(dstRoiSize, pMask, maskSize, pMorphSpec, pBuffer);

			IppiBorderType borderType = ippBorderRepl;
			status = ippiErodeBorder_8u_C1R(pOrigT, nSrcPitch, pDstT, nSrcPitch, dstRoiSize,
				borderType, 0, pMorphSpec, pBuffer);

		}
		else if (nChannel == _Type_G24)
		{
			status = ippiMorphologyBorderGetSize_8u_C3R(dstRoiSize, maskSize, &npSpecSize, &nBufferSize);
			pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
			pMorphSpec = (IppiMorphState*)scalable_aligned_malloc(npSpecSize*sizeof(Ipp8u), 32);
			status = ippiMorphologyBorderInit_8u_C3R(dstRoiSize, pMask, maskSize, pMorphSpec, pBuffer);

			IppiBorderType borderType = ippBorderRepl;
			Ipp8u bordervalue[3] = { 0, 0, 0 };

			status = ippiErodeBorder_8u_C3R(pOrigT, nSrcPitch, pDstT, nSrcPitch, dstRoiSize,
				borderType, bordervalue, pMorphSpec, pBuffer);

		}
		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pMorphSpec);

	}, auto_partitioner());

	if (bMaskNotInit)
	{
		ippsFree(pMask);
	}


	return 1;
}

int CKxBaseFunction::KxParallelResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode)
{
	KxCallStatus hCall;
	return KxParallelResizeImage(SrcImg, ResizeImg, nInterpolationMode, hCall);
}

int CKxBaseFunction::KxParallelResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	int nStatus = 1;

	switch (nInterpolationMode)
	{
	case KxNearest:
	{
					  nStatus = KxParallelResizeNearestImage(SrcImg, ResizeImg, nRowGrain, nColGrain, hCall);
					  break;
	}
	case KxLinear:
	{
					 nStatus = KxParallelResizeLinearImage(SrcImg, ResizeImg, nRowGrain, nColGrain, hCall);
					 break;
	}
	case KxCubic:
	{
					nStatus = KxParallelResizeCubicImage(SrcImg, ResizeImg, nRowGrain, nColGrain, hCall);
					break;
	}
	case KxLanczos:
	{
					  nStatus = KxParallelResizeLanczosImage(SrcImg, ResizeImg, nRowGrain, nColGrain, hCall);
					  break;

	}
	case KxSuper:
	{
					nStatus = KxParallelResizeSuperImage(SrcImg, ResizeImg, nRowGrain, nColGrain, hCall);
					break;

	}
	default:
	{
			   nStatus = KxParallelResizeLinearImage(SrcImg, ResizeImg, nRowGrain, nColGrain, hCall);
			   break;
	}

	}
	return nStatus;

}

bool CKxBaseFunction::GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3])
{
	KxCallStatus hCall;
	return GetPerspectiveTransform(srcQuad, dstQuad, coeffs, hCall);
}

bool CKxBaseFunction::GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3], KxCallStatus& hCall)
{
	hCall.Clear();
	/* Source matrix with widthHeight=8 */
	double pSrc[8 * 8] = { srcQuad[0][0], srcQuad[0][1], 1, 0, 0, 0, -srcQuad[0][0] * dstQuad[0][0], -srcQuad[0][1] * dstQuad[0][0],
		0, 0, 0, srcQuad[0][0], srcQuad[0][1], 1, -srcQuad[0][0] * dstQuad[0][1], -srcQuad[0][1] * dstQuad[0][1],

		srcQuad[1][0], srcQuad[1][1], 1, 0, 0, 0, -srcQuad[1][0] * dstQuad[1][0], -srcQuad[1][1] * dstQuad[1][0],
		0, 0, 0, srcQuad[1][0], srcQuad[1][1], 1, -srcQuad[1][0] * dstQuad[1][1], -srcQuad[1][1] * dstQuad[1][1],

		srcQuad[2][0], srcQuad[2][1], 1, 0, 0, 0, -srcQuad[2][0] * dstQuad[2][0], -srcQuad[2][1] * dstQuad[2][0],
		0, 0, 0, srcQuad[2][0], srcQuad[2][1], 1, -srcQuad[2][0] * dstQuad[2][1], -srcQuad[2][1] * dstQuad[2][1],

		srcQuad[3][0], srcQuad[3][1], 1, 0, 0, 0, -srcQuad[3][0] * dstQuad[3][0], -srcQuad[3][1] * dstQuad[3][0],
		0, 0, 0, srcQuad[3][0], srcQuad[3][1], 1, -srcQuad[3][0] * dstQuad[3][1], -srcQuad[3][1] * dstQuad[3][1] };


	double pSrc2[8 * 1] = { dstQuad[0][0], dstQuad[0][1], dstQuad[1][0], dstQuad[1][1],
		dstQuad[2][0], dstQuad[2][1], dstQuad[3][0], dstQuad[3][1] };

	lapack_int* ipiv = new lapack_int[8];
	lapack_int info = LAPACKE_dgetrf(LAPACK_ROW_MAJOR, 8, 8, pSrc, 8, ipiv);
	if (check_sts(info, "GetPerspectiveTransform_LAPACKE_dgetrf", hCall))
	{
		return 0;
	}
	info = LAPACKE_dgetrs(LAPACK_ROW_MAJOR, 'N', 8, 1, pSrc, 8, ipiv, pSrc2, 1);
	if (check_sts(info, "GetPerspectiveTransform_LAPACKE_dgetrs", hCall))
	{
		return 0;
	}

	delete[]ipiv;

	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 3; i++)
		{
			if (j == 2 && i == 2)
			{
				break;
			}
			else
			{
				coeffs[j][i] = pSrc2[3 * j + i];
			}

		}
	}
	coeffs[2][2] = 1;

	return true;
}

int CKxBaseFunction::KxThresholdAdaptiveGauss(InputArray SrcImg, OutputArray DstImg, IppiSize mskSize, int nDelta, int nThreshLayer)
{
	KxCallStatus hCall;
	return KxThresholdAdaptiveGauss(SrcImg, DstImg, mskSize, nDelta, nThreshLayer, hCall);
}

int CKxBaseFunction::KxThresholdAdaptiveGauss(InputArray SrcImg, OutputArray DstImg, IppiSize mskSize, int nDelta, int nThreshLayer, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	if (SrcImg.type() == _Type_G24)
	{
		KxConvertImageLayer(SrcImg, m_ImgGaussGray, nThreshLayer, hCallInfo);
		if (check_sts(hCallInfo, "KxThresholdAdaptiveGauss_", hCall))
		{
			return 0;
		}
	}
	else
	{
		Rect rc;
		rc.setup(0, 0, SrcImg.cols - 1, SrcImg.rows - 1);
		m_ImgGaussGray.Init(SrcImg.cols, SrcImg.rows);
		KxCopyImage(SrcImg, m_ImgGaussGray, rc);
	}
	IppStatus status;
	int nBufferSize, nSpecSize;
	IppiSize roi = { m_ImgGaussGray.cols, m_ImgGaussGray.rows };
	status = ippiThresholdAdaptiveGaussGetBufferSize(roi, mskSize, ipp8u, 1, &nSpecSize, &nBufferSize);
	if (check_sts(status, "KxThresholdAdaptiveGauss_ippiThresholdAdaptiveGaussGetBufferSize", hCall))
	{
		return 0;
	}
	IppiThresholdAdaptiveSpec* pSpec = (IppiThresholdAdaptiveSpec*)ippsMalloc_8u(nSpecSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	DstImg.Init(roi.width, roi.height);
	Ipp32f sigma = Ipp32f(0.0);
	status  = ippiThresholdAdaptiveGaussInit(roi, mskSize, ipp8u, 1, sigma, pSpec);

	if (check_sts(status, "KxThresholdAdaptiveGauss_ippiThresholdAdaptiveGaussInit", hCall))
	{
		return 0;
	}

	status = ippiThresholdAdaptiveGauss_8u_C1R(m_ImgGaussGray.data, m_ImgGaussGray.step, DstImg.data, DstImg.step, roi, (Ipp32f)nDelta, 255, 0, ippBorderConst, 0, pSpec, pBuffer);
	if (check_sts(status, "KxThresholdAdaptiveGauss_ippiThresholdAdaptiveGauss_8u_C1R", hCall))
	{
		return 0;
	}

	ippsFree(pBuffer);
	ippsFree(pSpec);

	return 1;

}

int CKxBaseFunction::KxInvertImage(InputArray SrcImg, OutputArray DstImg)
{
	KxCallStatus hCall;
	return KxInvertImage(SrcImg, DstImg, hCall);
}


int CKxBaseFunction::KxInvertImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall)
{
	DstImg.Init(SrcImg.cols, SrcImg.rows, SrcImg.type());
	ippsSet_8u(0xff, DstImg.data, DstImg.step * DstImg.rows);
	IppiSize roiSize = { SrcImg.cols, SrcImg.rows };
	IppStatus status;
	if (SrcImg.type() == _Type_G24)
	{
		status = ippiSub_8u_C3IRSfs(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, 0);
	}
	else
	{
		status = ippiSub_8u_C1IRSfs(SrcImg.data, SrcImg.step, DstImg.data, DstImg.step, roiSize, 0);
	}

	if (check_sts(status, "KxInvertImage_ippiSub_8u", hCall))
	{
		return 0;
	}

	return 1;
}

