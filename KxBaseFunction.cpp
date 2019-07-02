
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



int CKxBaseFunction::KxCopyImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, const kxRect<int>& rc, KxCallStatus& hCall)
{
	return KxCopyImage(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel,
		               DstImg.buf, DstImg.nWidth, DstImg.nHeight, DstImg.nPitch, DstImg.nChannel,
					   rc, hCall);

}

int CKxBaseFunction::KxCopyImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, const kxRect<int>& rc)
{
	KxCallStatus hCall;
	return KxCopyImage(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel,
		DstImg.buf, DstImg.nWidth, DstImg.nHeight, DstImg.nPitch, DstImg.nChannel,
		rc, hCall);

}


int CKxBaseFunction::KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel,
								 unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
								 const kxRect<int>& rc, KxCallStatus& hCall)

{
	IppStatus  status;
	hCall.Clear();

	if (nSrcChannel != nDstChannel)
	{
		return 0;
	}

	//从大图拷贝一块变为小图
	if(nSrcWidth >= nDstWidth && nSrcHeight >= nDstHeight && nSrcPitch >= nDstPitch)
	{
		if (rc.bottom  > nSrcHeight || rc.right > nSrcWidth || rc.left < 0 || rc.top < 0)
		{
			return 0;
		}
		IppiSize OffSize = {rc.Width(), rc.Height()};
		if (nSrcChannel == _Type_G8)
		{
			status = ippiCopy_8u_C1R( pSrc + rc.left + rc.top * nSrcPitch, nSrcPitch,
				pDst, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Big2Small_Type_G8_ippiCopy", hCall))
			{
				return 0;
			}

		}
		if (nSrcChannel == _Type_G24)
		{
			status = ippiCopy_8u_C3R( pSrc + rc.left*3 + rc.top * nSrcPitch, nSrcPitch,
				pDst, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Big2Small_Type_G24_ippiCopy", hCall))
			{
				return 0;
			}

		}
		if (nSrcChannel == _Type_G32)
		{
			status = ippiCopy_8u_C4R( pSrc + rc.left*4 + rc.top * nSrcPitch, nSrcPitch,
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
		if (rc.bottom  > nDstHeight || rc.right > nDstWidth || rc.left < 0 || rc.top < 0)
		{
			return 0;
		}
		IppiSize OffSize = {rc.Width(), rc.Height()};
		if (nSrcChannel == _Type_G8)
		{
			status = ippiCopy_8u_C1R( pSrc, nSrcPitch,
				pDst + rc.left + rc.top * nDstPitch, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Small2Big_Type_G8_ippiCopy", hCall))
			{
				return 0;
			}
		}
		if (nSrcChannel == _Type_G24)
		{
			status = ippiCopy_8u_C3R( pSrc , nSrcPitch,
				pDst + rc.left*3 + rc.top * nDstPitch, nDstPitch, OffSize);

			if (check_sts(status, "KxCopyImage_Small2Big_Type_G24_ippiCopy", hCall))
			{
				return 0;
			}

		}
		if (nSrcChannel == _Type_G32)
		{
			status = ippiCopy_8u_C4R( pSrc , nSrcPitch,
				pDst + rc.left*4 + rc.top * nDstPitch, nDstPitch, OffSize);

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
	const kxRect<int>& rc)
{
	KxCallStatus hCall;
	return KxCopyImage(pSrc, nSrcWidth, nSrcHeight, nSrcPitch, nSrcChannel, pDst, nDstWidth, nDstHeight, nDstPitch, nDstChannel, rc, hCall);
}



float CKxBaseFunction::kxImageAlign( kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer, KxCallStatus& hCall)
{
	switch (nPyramidLayer)
	{
	case  2:
	case  3:
	{
			   int nResize = int(pow(2, nPyramidLayer - 1));
			   int nResizeW = (BigImg.nWidth / nResize) * nResize;
			   int nResizeH = (BigImg.nHeight / nResize) * nResize;
			   m_NormBigImg.SetImageBuf(BigImg.buf, nResizeW, nResizeH, BigImg.nPitch, BigImg.nChannel, false);
			   nResizeW = (SamllImg.nWidth / nResize) * nResize;
			   nResizeH = (SamllImg.nHeight / nResize) * nResize;
			   m_NormSmallImg.SetImageBuf(SamllImg.buf, nResizeW, nResizeH, SamllImg.nPitch, SamllImg.nChannel, false);

			   m_PyramidBigImg.Init(m_NormBigImg.nWidth / nResize, m_NormBigImg.nHeight / nResize, BigImg.nChannel);
			   m_PyramidSmallImg.Init(m_NormSmallImg.nWidth / nResize, m_NormSmallImg.nHeight / nResize, SamllImg.nChannel);
			   KxResizeImage(m_NormBigImg, m_PyramidBigImg, KxSuper);
			   KxResizeImage(m_NormSmallImg, m_PyramidSmallImg, KxSuper);
			   float fRatio = kxImageAlign(pos, m_PyramidBigImg.buf, m_PyramidBigImg.nWidth, m_PyramidBigImg.nHeight, m_PyramidBigImg.nPitch,
				   m_PyramidSmallImg.buf, m_PyramidSmallImg.nWidth, m_PyramidSmallImg.nHeight, m_PyramidSmallImg.nPitch, m_PyramidSmallImg.nChannel, hCall);
			   pos.x = nResize * pos.x;
			   pos.y = nResize * pos.y;

			   return fRatio;
	}

	default:
	{
			   return kxImageAlign(pos, BigImg.buf, BigImg.nWidth, BigImg.nHeight, BigImg.nPitch,
				   SamllImg.buf, SamllImg.nWidth, SamllImg.nHeight, SamllImg.nPitch, SamllImg.nChannel, hCall);
	}

	}
}

float CKxBaseFunction::kxImageAlign(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer)
{
	KxCallStatus hCall;
	return kxImageAlign(pos, BigImg, SamllImg, nPyramidLayer, hCall);
}



float CKxBaseFunction::kxImageAlignBySSD(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer, KxCallStatus& hCall)
{
	switch (nPyramidLayer)
	{
	case  2:
	case  3:
	{
			   int nResize = int(pow(2, nPyramidLayer - 1));
			   int nResizeW = (BigImg.nWidth / nResize) * nResize;
			   int nResizeH = (BigImg.nHeight / nResize) * nResize;
			   m_NormBigImg.SetImageBuf(BigImg.buf, nResizeW, nResizeH, BigImg.nPitch, BigImg.nChannel, false);
			   nResizeW = (SamllImg.nWidth / nResize) * nResize;
			   nResizeH = (SamllImg.nHeight / nResize) * nResize;
			   m_NormSmallImg.SetImageBuf(SamllImg.buf, nResizeW, nResizeH, SamllImg.nPitch, SamllImg.nChannel, false);

			   m_PyramidBigImg.Init(m_NormBigImg.nWidth / nResize, m_NormBigImg.nHeight / nResize, BigImg.nChannel);
			   m_PyramidSmallImg.Init(m_NormSmallImg.nWidth / nResize, m_NormSmallImg.nHeight / nResize, SamllImg.nChannel);
			   KxResizeImage(m_NormBigImg, m_PyramidBigImg, KxSuper);
			   KxResizeImage(m_NormSmallImg, m_PyramidSmallImg, KxSuper);
			   float fRatio = kxImageAlignBySSD(pos, m_PyramidBigImg.buf, m_PyramidBigImg.nWidth, m_PyramidBigImg.nHeight, m_PyramidBigImg.nPitch,
				   m_PyramidSmallImg.buf, m_PyramidSmallImg.nWidth, m_PyramidSmallImg.nHeight, m_PyramidSmallImg.nPitch, m_PyramidSmallImg.nChannel, hCall);
			   pos.x = nResize * pos.x;
			   pos.y = nResize * pos.y;

			  // const int nExtend = 9;
			  // kxPoint<float> pos1, pos2;
			  // int left = gMax((int)pos.x - nExtend, 0);
			  // int top = gMax((int)pos.y - nExtend, 0);
			  // int nW = m_NormSmallImg.nWidth + 2 * nExtend;
			  // int nH = m_NormSmallImg.nHeight + 2 * nExtend;
			  // nW = (left + nW < m_NormBigImg.nWidth - 1) ? nW : (m_NormBigImg.nWidth - 1 - left);
			  // nH = (top + nH < m_NormBigImg.nHeight - 1) ? nH : (m_NormBigImg.nHeight - 1 - top);
			  //// if ((left + nW < m_NormBigImg.nWidth-1) && (top + nH < m_NormBigImg.nHeight-1))
			  // {
				 //  m_NormBigImgLittle.SetImageBuf(m_NormBigImg.buf + top*m_NormBigImg.nPitch + m_NormBigImg.nChannel*left, nW, nH, m_NormBigImg.nPitch, m_NormBigImg.nChannel, false);
				 //  fRatio = kxImageAlignBySSD(pos2, m_NormBigImgLittle.buf, m_NormBigImgLittle.nWidth, m_NormBigImgLittle.nHeight, m_NormBigImgLittle.nPitch,
					//   m_NormSmallImg.buf, m_NormSmallImg.nWidth, m_NormSmallImg.nHeight, m_NormSmallImg.nPitch, m_NormSmallImg.nChannel, hCall);
				 //  pos1.x = pos2.x + left;
				 //  pos1.y = pos2.y + top;
				 //  pos.x = pos1.x;
				 //  pos.y = pos1.y;
			  // }
			   return fRatio;
	}

	default:
	{
			   return kxImageAlignBySSD(pos, BigImg.buf, BigImg.nWidth, BigImg.nHeight, BigImg.nPitch,
				   SamllImg.buf, SamllImg.nWidth, SamllImg.nHeight, SamllImg.nPitch, SamllImg.nChannel, hCall);
	}

	}

	//return kxImageAlignBySSD(pos, BigImg.buf, BigImg.nWidth, BigImg.nHeight, BigImg.nPitch,
	//	SamllImg.buf, SamllImg.nWidth, SamllImg.nHeight, SamllImg.nPitch, SamllImg.nChannel, hCall);

}


float CKxBaseFunction::kxImageAlignBySSD(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg)
{
	KxCallStatus hCall;
	return kxImageAlignBySSD(pos, BigImg.buf, BigImg.nWidth, BigImg.nHeight, BigImg.nPitch,
		SamllImg.buf, SamllImg.nWidth, SamllImg.nHeight, SamllImg.nPitch, SamllImg.nChannel, hCall);

}



//float CKxBaseFunction::kxImageAlign( kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
//{// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置
//	IppStatus status;
//	hCall.Clear();
//
//	assert( sw >= tw && sh >= th );
//	IppiSize srcRoiSize;
//	srcRoiSize.width = sw*nImgType;
//	srcRoiSize.height = sh;
//
//	IppiSize tplRoiSize;
//	tplRoiSize.width = tw*nImgType;
//	tplRoiSize.height = th;
//
//	int cw = sw - tw + 1;
//	int ch = sh - th + 1;
//
//	int corrSize = cw * ch;
//	Ipp32f pMax = 0;
//	IppiSize corrRoiSize;
//	//corrRoiSize.width = cw*nImgType;
//	//corrRoiSize.height = ch;
//
//	corrRoiSize.width = srcRoiSize.width - tplRoiSize.width + 1 + 2;
//	corrRoiSize.height = srcRoiSize.height - tplRoiSize.height + 1;
//
//
//	m_CorrImage32f.Init(corrRoiSize.width, corrRoiSize.height);
//	memset(m_CorrImage32f.buf, 0, sizeof(Ipp32f)*m_CorrImage32f.nWidth*m_CorrImage32f.nHeight);
//
//    int nBuffsize;
//	IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNormCoefficient);
//	status = ippiCrossCorrNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);
//
//	if (check_sts(status, "kxImageAlign_ippiCrossCorrNormGetBufferSize", hCall))
//	{
//		return 0;
//	}
//	Ipp8u* pBuffer;
//	pBuffer = ippsMalloc_8u(nBuffsize);
//
//	int nSrcPitch = sw*nImgType;
//	int nTplPitch = tw*nImgType;
//
//	status = ippiCrossCorrNorm_8u32f_C1R(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_CorrImage32f.buf, m_CorrImage32f.nPitch, funCfg, pBuffer);
//
//	ippsFree(pBuffer);
//
//
//	if (check_sts(status, "kxImageAlign_ippiCrossCorrNorm_8u32f_C1R", hCall))
//	{
//		return 0;
//	}
//
//	m_Result32f.Init(m_CorrImage32f.nWidth / nImgType, m_CorrImage32f.nHeight);
//	IppiSize Ri = { m_CorrImage32f.nWidth / nImgType, m_CorrImage32f.nHeight };
//	if (nImgType == _Type_G24)
//	{
//		ippiCopy_32f_C3C1R(m_CorrImage32f.buf , m_CorrImage32f.nPitch, m_Result32f.buf, m_Result32f.nPitch, Ri);
//	}
//	else
//	{
//		ippiCopy_32f_C1R(m_CorrImage32f.buf, m_CorrImage32f.nPitch, m_Result32f.buf, m_Result32f.nPitch, Ri);
//	}
//
//
//	IppiPoint maxPos;
//	status = ippiMaxIndx_32f_C1R(m_Result32f.buf , m_Result32f.nPitch, Ri, &pMax, &maxPos.x, &maxPos.y);
//	//IppiSize roiSize = { m_CorrImage32f.nWidth, m_CorrImage32f.nHeight };
//	//status = ippiMaxIndx_32f_C1R(m_CorrImage32f.buf, m_CorrImage32f.nPitch, roiSize, &pMax, &maxPos.x, &maxPos.y);
//
//	if (check_sts(status, "kxImageAlign_ippsMaxIndx_32f", hCall))
//	{
//		return 0;
//	}
//
//	pos.x = (float)maxPos.x ;
//	pos.y = (float)maxPos.y;
//
//	return pMax;
//}

//float CKxBaseFunction::kxImageAlign(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
//{// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置
//	IppStatus status;
//	hCall.Clear();
//
//	assert(sw >= tw && sh >= th);
//	IppiSize srcRoiSize;
//	srcRoiSize.width = sw*nImgType;
//	srcRoiSize.height = sh;
//
//	IppiSize tplRoiSize;
//	tplRoiSize.width = tw*nImgType;
//	tplRoiSize.height = th;
//
//
//	Ipp32f pMax = 0;
//	IppiSize corrRoiSize;
//
//
//	corrRoiSize.width = srcRoiSize.width - tplRoiSize.width + 1 + 2*(nImgType==_Type_G24 ? 1 : 0);
//	corrRoiSize.height = srcRoiSize.height - tplRoiSize.height + 1;
//
//
//	m_CorrImage32f.Init(corrRoiSize.width, corrRoiSize.height);
//	memset(m_CorrImage32f.buf, 0, sizeof(unsigned char)*m_CorrImage32f.nWidth*m_CorrImage32f.nHeight);
//
//	int nBuffsize;
//	IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNormCoefficient);
//	status = ippiCrossCorrNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);
//
//	if (check_sts(status, "kxImageAlign_ippiCrossCorrNormGetBufferSize", hCall))
//	{
//		return 0;
//	}
//	Ipp8u* pBuffer;
//	pBuffer = ippsMalloc_8u(nBuffsize);
//
//	int nSrcPitch = sw*nImgType;
//	int nTplPitch = tw*nImgType;
//
//	status = ippiCrossCorrNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_CorrImage32f.buf, m_CorrImage32f.nPitch, -7,  funCfg, pBuffer);
//
//	ippsFree(pBuffer);
//
//
//	if (check_sts(status, "kxImageAlign_ippiCrossCorrNorm_8u32f_C1R", hCall))
//	{
//		return 0;
//	}
//
//
//	IppiSize Ri = { m_CorrImage32f.nWidth, m_CorrImage32f.nHeight };
//	IppiPoint maxPos;
//	Ipp8u max8u;
//	status = ippiMaxIndx_8u_C1R(m_CorrImage32f.buf, m_CorrImage32f.nPitch, Ri, &max8u, &maxPos.x, &maxPos.y);
//
//	if (check_sts(status, "kxImageAlign_ippsMaxIndx_32f", hCall))
//	{
//		return 0;
//	}
//
//	pos.x = (float)maxPos.x / nImgType;
//	pos.y = (float)maxPos.y;
//
//	return max8u*1.0f/128;
//}



float CKxBaseFunction::kxImageAlignBySSD(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
{// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置
	//IppStatus status;
	//hCall.Clear();

	//if (!(sw >= tw && sh >= th))
	//{
	//	status = IppStatus(-1000);
	//	if (check_sts(status, "Template_Test_Size_Donot_Match", hCall))
	//	{
	//		return 0;
	//	}
	//}

	//IppiSize srcRoiSize;
	//srcRoiSize.width = sw;
	//srcRoiSize.height = sh;

	//IppiSize tplRoiSize;
	//tplRoiSize.width = tw;
	//tplRoiSize.height = th;


	//Ipp32f pMax = 0;
	//IppiSize corrRoiSize;


	//corrRoiSize.width = srcRoiSize.width - tplRoiSize.width + 1;
	//corrRoiSize.height = srcRoiSize.height - tplRoiSize.height + 1;


	//m_CorrImage32f.Init(corrRoiSize.width, corrRoiSize.height);
	//memset(m_CorrImage32f.buf, 0, sizeof(unsigned char)*m_CorrImage32f.nWidth*m_CorrImage32f.nHeight);
	//m_Result32f.Init(corrRoiSize.width, corrRoiSize.height);
	//memset(m_Result32f.buf, 0, sizeof(unsigned char)*m_Result32f.nWidth*m_Result32f.nHeight);

	//int nBuffsize;
	//IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNorm);
	//status = ippiSqrDistanceNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);

	//if (check_sts(status, "kxImageAlignBySSD_ippiSqrDistanceNormGetBufferSize", hCall))
	//{
	//	return 0;
	//}
	//Ipp8u* pBuffer;
	//pBuffer = ippsMalloc_8u(nBuffsize);

	//if (nImgType == _Type_G24)
	//{
	//	unsigned char* pDst[3];
	//	unsigned char* pTl[3];
	//	for (int i = 0; i < 3; i++)
	//	{
	//		m_SplitImg[i].Init(sw, sh);
	//		pDst[i] = m_SplitImg[i].buf;
	//		m_SplitTpl[i].Init(tw, th);
	//		pTl[i] = m_SplitTpl[i].buf;
	//	}

	//	ippiCopy_8u_C3P3R(srcBuf, ss, pDst, sw, srcRoiSize);
	//	ippiCopy_8u_C3P3R(tplBuf, ts, pTl, tw, tplRoiSize);

	//	for (int i = 0; i < 3; i++)
	//	{
	//		status = ippiSqrDistanceNorm_8u_C1RSfs(pDst[i], sw, srcRoiSize, pTl[i], tw, tplRoiSize, m_CorrImage32f.buf, m_CorrImage32f.nPitch, -7, funCfg, pBuffer);
	//		ippiMaxEvery_8u_C1IR(m_CorrImage32f.buf, m_CorrImage32f.nPitch, m_Result32f.buf, m_Result32f.nPitch, corrRoiSize);
	//	}
	//}
	//else
	//{
	//	status = ippiSqrDistanceNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_Result32f.buf, m_Result32f.nPitch, -7, funCfg, pBuffer);

	//}

	//ippsFree(pBuffer);

	//IppiPoint maxPos;
	//Ipp8u max8u;
	//status = ippiMinIndx_8u_C1R(m_Result32f.buf, m_Result32f.nPitch, corrRoiSize, &max8u, &maxPos.x, &maxPos.y);

	//if (check_sts(status, "kxImageAlignBySSD_ippsMaxIndx_32f", hCall))
	//{
	//	return 0;
	//}

	//pos.x = (float)maxPos.x;
	//pos.y = (float)maxPos.y;
	//return max8u*1.0f / 128;

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


	m_CorrImage32f.Init(corrRoiSize.width, corrRoiSize.height);
	memset(m_CorrImage32f.buf, 0, sizeof(unsigned char)*m_CorrImage32f.nWidth*m_CorrImage32f.nHeight);
	m_Result32f.Init(corrRoiSize.width, corrRoiSize.height);
	memset(m_Result32f.buf, 0, sizeof(unsigned char)*m_Result32f.nWidth*m_Result32f.nHeight);

	int nBuffsize;
	IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNorm);
	status = ippiSqrDistanceNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);

	if (check_sts(status, "kxImageAlignBySSD_ippiSqrDistanceNormGetBufferSize", hCall))
	{
		return 0;
	}
	Ipp8u* pBuffer;
	pBuffer = ippsMalloc_8u(nBuffsize);

	status = ippiSqrDistanceNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_Result32f.buf, m_Result32f.nPitch, -7, funCfg, pBuffer);

	ippsFree(pBuffer);

	m_SplitImg[0].Init(corrRoiSize.width / nImgType, corrRoiSize.height);
	srcRoiSize.width = corrRoiSize.width / nImgType;
	srcRoiSize.height = corrRoiSize.height;

	if (nImgType == _Type_G24)
	{
		unsigned char* pDst[3];
		for (int i = 0; i < 3; i++)
		{
			m_SplitImg[i].Init(corrRoiSize.width / nImgType, corrRoiSize.height);
			pDst[i] = m_SplitImg[i].buf;
		}
		ippiCopy_8u_C3P3R(m_Result32f.buf, m_Result32f.nPitch, pDst, m_SplitImg[0].nPitch, srcRoiSize);
	}
	else
	{
		ippiCopy_8u_C1R(m_Result32f.buf, m_Result32f.nPitch, m_SplitImg[0].buf, m_SplitImg[0].nPitch, srcRoiSize);
	}

	IppiPoint maxPos;
	Ipp8u max8u;
	status = ippiMinIndx_8u_C1R(m_SplitImg[0].buf, m_SplitImg[0].nPitch, srcRoiSize, &max8u, &maxPos.x, &maxPos.y);

	if (check_sts(status, "kxImageAlignBySSD_ippsMaxIndx_8u", hCall))
	{
		return 0;
	}

	pos.x = (float)maxPos.x;
	pos.y = (float)maxPos.y;

	return max8u*1.0f / 128;


}


float CKxBaseFunction::kxImageAlignBySSD(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType)
{
	KxCallStatus hCall;
	return kxImageAlignBySSD(pos, srcBuf, sw, sh, ss, tplBuf, tw, th, ts, nImgType, hCall);
}


float CKxBaseFunction::kxImageAlign(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType)
{
	KxCallStatus hCall;
	return kxImageAlign(pos, srcBuf, sw, sh, ss, tplBuf, tw, th, ts, nImgType, hCall);
}

float CKxBaseFunction::kxImageAlign(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
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


	m_CorrImage32f.Init(corrRoiSize.width, corrRoiSize.height);
	memset(m_CorrImage32f.buf, 0, sizeof(unsigned char)*m_CorrImage32f.nWidth*m_CorrImage32f.nHeight);
	m_Result32f.Init(corrRoiSize.width, corrRoiSize.height);
	memset(m_Result32f.buf, 0, sizeof(unsigned char)*m_Result32f.nWidth*m_Result32f.nHeight);

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
			m_SplitImg[i].Init(sw, sh);
			pDst[i] = m_SplitImg[i].buf;
			m_SplitTpl[i].Init(tw, th);
			pTl[i] = m_SplitTpl[i].buf;
		}

		ippiCopy_8u_C3P3R(srcBuf, ss, pDst, sw, srcRoiSize);
		ippiCopy_8u_C3P3R(tplBuf, ts, pTl, tw, tplRoiSize);

		for (int i = 0; i < 3; i++)
		{
			status = ippiCrossCorrNorm_8u_C1RSfs(pDst[i], sw, srcRoiSize, pTl[i], tw, tplRoiSize, m_CorrImage32f.buf, m_CorrImage32f.nPitch, -7, funCfg, pBuffer);
			ippiMaxEvery_8u_C1IR(m_CorrImage32f.buf, m_CorrImage32f.nPitch, m_Result32f.buf, m_Result32f.nPitch, corrRoiSize);
		}
    }
	else
	{
		status = ippiCrossCorrNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_Result32f.buf, m_Result32f.nPitch, -7, funCfg, pBuffer);

	}

	ippsFree(pBuffer);

	IppiPoint maxPos;
	Ipp8u max8u;
	status = ippiMaxIndx_8u_C1R(m_Result32f.buf, m_Result32f.nPitch, corrRoiSize, &max8u, &maxPos.x, &maxPos.y);

	if (check_sts(status, "kxImageAlign_ippsMaxIndx_32f", hCall))
	{
		return 0;
	}

	pos.x = (float)maxPos.x;
	pos.y = (float)maxPos.y;

	return max8u*1.0f / 128;
}

float CKxBaseFunction::kxImageAlignColor(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg,
	 float fcompressFactorx /*= 2.0*/, float fcompressFactory /*= 2.0*/)
{
	assert(fcompressFactorx >= 1.0f && fcompressFactory >= 1.0f);

	int nResizex = int(pow(2, fcompressFactorx - 1));
	int nResizey = int(pow(2, fcompressFactory - 1));
	if (nResizex == 1 && nResizey == 1)
	{
		kxPoint<float> Pos1(0, 0);
		float f = kxImageAlignColor(Pos1, BigImg.buf, BigImg.nWidth, BigImg.nHeight, BigImg.nPitch,
			SamllImg.buf, SamllImg.nWidth, SamllImg.nHeight, SamllImg.nPitch, SamllImg.nChannel);
		pos.x = Pos1.x;
		pos.y = Pos1.y;
		return f;
	}

	int nResizeW = (BigImg.nWidth / nResizex) * nResizex;
	int nResizeH = (BigImg.nHeight / nResizey) * nResizey;
	m_NormBigImg.SetImageBuf(BigImg.buf, nResizeW, nResizeH, BigImg.nPitch, BigImg.nChannel, false);
	nResizeW = (SamllImg.nWidth / nResizex) * nResizex;
	nResizeH = (SamllImg.nHeight / nResizey) * nResizey;
	m_NormSmallImg.SetImageBuf(SamllImg.buf, nResizeW, nResizeH, SamllImg.nPitch, SamllImg.nChannel, false);

	m_PyramidBigImg.Init(m_NormBigImg.nWidth / nResizex, m_NormBigImg.nHeight / nResizey, BigImg.nChannel);
	m_PyramidSmallImg.Init(m_NormSmallImg.nWidth / nResizex, m_NormSmallImg.nHeight / nResizey, SamllImg.nChannel);
	KxResizeImage(m_NormBigImg, m_PyramidBigImg, KxLinear);
	KxResizeImage(m_NormSmallImg, m_PyramidSmallImg, KxLinear);


	kxPoint<float> Pos1(0, 0);
	float f = kxImageAlignColor(Pos1, m_PyramidBigImg.buf, m_PyramidBigImg.nWidth, m_PyramidBigImg.nHeight, m_PyramidBigImg.nPitch,
		m_PyramidSmallImg.buf, m_PyramidSmallImg.nWidth, m_PyramidSmallImg.nHeight, m_PyramidSmallImg.nPitch, m_PyramidSmallImg.nChannel);
	Pos1.x = Pos1.x*nResizex;
	Pos1.y = Pos1.y*nResizey;


	//考虑定位精度为4个像素
	int  nExpandx = 4 * nResizex + 1;
	int  nExpandy = 4 * nResizey + 1;
	int  nTop = (std::max)(0, (int)(Pos1.y - nExpandy));
	int  nLeft = (std::max)(0, (int)(Pos1.x - nExpandx));
	int  nRight = (std::min)(BigImg.nWidth - 1, (int)(Pos1.x + nExpandx + SamllImg.nWidth));
	int  nBottom = (std::min)(BigImg.nHeight - 1, (int)(Pos1.y + nExpandy + SamllImg.nHeight));
	if (nRight - nLeft + 1 < SamllImg.nWidth || nBottom - nTop + 1 < SamllImg.nHeight)
	{
		return  kxImageAlignColor(pos, BigImg.buf, BigImg.nWidth, BigImg.nHeight, BigImg.nPitch,
			SamllImg.buf, SamllImg.nWidth, SamllImg.nHeight, SamllImg.nPitch, SamllImg.nChannel);
	}
	float fRatio = kxImageAlignColor(Pos1, BigImg.buf + nTop*BigImg.nPitch + nLeft * BigImg.nChannel,
		nRight - nLeft + 1, nBottom - nTop + 1, BigImg.nPitch, SamllImg.buf, SamllImg.nWidth, SamllImg.nHeight, SamllImg.nPitch, SamllImg.nChannel);
	pos.x = Pos1.x + nLeft;
	pos.y = Pos1.y + nTop;

	return  fRatio;

}



float CKxBaseFunction::kxImageAlignColor(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType)
{
	KxCallStatus hCall;
	return kxImageAlignColor(pos, srcBuf, sw, sh, ss, tplBuf, tw, th, ts, nImgType, hCall);
}

float CKxBaseFunction::kxImageAlignColor(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall)
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

	m_Result32f.Init(corrRoiSize.width, corrRoiSize.height);
	memset(m_Result32f.buf, 0, sizeof(unsigned char)*m_Result32f.nWidth*m_Result32f.nHeight);

	int nBuffsize;
	IppEnum  funCfg = (IppEnum)(ippAlgAuto | ippiROIValid | ippiNormCoefficient);
	status = ippiCrossCorrNormGetBufferSize(srcRoiSize, tplRoiSize, funCfg, &nBuffsize);

	if (check_sts(status, "kxImageAlign_ippiCrossCorrNormGetBufferSize", hCall))
	{
		return 0;
	}
	Ipp8u* pBuffer;
	pBuffer = ippsMalloc_8u(nBuffsize);

	status = ippiCrossCorrNorm_8u_C1RSfs(srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, m_Result32f.buf, m_Result32f.nPitch, -7, funCfg, pBuffer);

	m_SplitImg[0].Init(corrRoiSize.width / nImgType, corrRoiSize.height);
	srcRoiSize.width = corrRoiSize.width / nImgType;
	srcRoiSize.height = corrRoiSize.height;

	if (nImgType == _Type_G24)
	{
		unsigned char* pDst[3];
		for (int i = 0; i < 3; i++)
		{
			m_SplitImg[i].Init(corrRoiSize.width / nImgType, corrRoiSize.height);
			pDst[i] = m_SplitImg[i].buf;
		}
		ippiCopy_8u_C3P3R(m_Result32f.buf, m_Result32f.nPitch, pDst, m_SplitImg[0].nPitch, srcRoiSize);
	}
	else
	{
		ippiCopy_8u_C1R(m_Result32f.buf, m_Result32f.nPitch, m_SplitImg[0].buf, m_SplitImg[0].nPitch, srcRoiSize);
	}

	IppiPoint maxPos;
	Ipp8u max8u;
	status = ippiMaxIndx_8u_C1R(m_SplitImg[0].buf, m_SplitImg[0].nPitch, srcRoiSize, &max8u, &maxPos.x, &maxPos.y);

	if (check_sts(status, "kxImageAlignColor_ippsMaxIndx_8u", hCall))
	{
		return 0;
	}

	pos.x = (float)maxPos.x;
	pos.y = (float)maxPos.y;

	ippsFree(pBuffer);

	return max8u*1.0f / 128;


}



int CKxBaseFunction::KxDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxDilateImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}



int CKxBaseFunction::KxDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

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
    if (SrcImg.nChannel == _Type_G8)
    {
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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
		status = ippiDilateBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
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
	else if (SrcImg.nChannel == _Type_G24)
	{
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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
		status = ippiDilateBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
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


int CKxBaseFunction::KxErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxErodeImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}


int CKxBaseFunction::KxErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

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
	if (SrcImg.nChannel == 1)
	{
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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
		status = ippiErodeBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
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
	else if (SrcImg.nChannel == 3)
	{
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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
		status = ippiErodeBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize,
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

int CKxBaseFunction::KxOpenImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxOpenImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}



int CKxBaseFunction::KxOpenImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	if (SrcImg.nChannel == 1)
	{
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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

		if (check_sts(status = ippiMorphOpenBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, borderType, borderValue, pSpec, pBuffer),
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
	else if (SrcImg.nChannel == 3)
	{
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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

		if (check_sts(status = ippiMorphOpenBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, borderType, borderValue, pSpec, pBuffer),
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


int CKxBaseFunction::KxCloseImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxCloseImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}



int CKxBaseFunction::KxCloseImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,  unsigned char* pMask,  KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
    bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	if (SrcImg.nChannel == 1)
	{
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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

		if (check_sts(status = ippiMorphCloseBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, borderType, borderValue, pSpec, pBuffer),
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
	else if (SrcImg.nChannel == 3)
	{
		IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
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

		if (check_sts(status = ippiMorphCloseBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, borderType, borderValue, pSpec, pBuffer),
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


int CKxBaseFunction::KxConvertImageLayer(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nChangeType)
{
	KxCallStatus hCall;
	return KxConvertImageLayer(SrcImg, DstImg, nChangeType, hCall);
}


int CKxBaseFunction::KxConvertImageLayer(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nChangeType, KxCallStatus& hCall)
{

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	return KxConvertImageLayer(SrcImg.buf, SrcImg.nPitch, SrcImg.nChannel,
		                       DstImg.buf, DstImg.nPitch, SrcImg.nWidth, SrcImg.nHeight, nChangeType, hCall);
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
			ippiRGBToHSV_8u_C3R(srcBuf,srcPitch,m_ImgHSV.buf,m_ImgHSV.nPitch,Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.buf,m_ImgHSV.nPitch, dstBuf, dstPitch, Roi);
		}
		if (HSV_S==nChangeType)
		{
			m_ImgHSV.Init(width,height,3);
			ippiRGBToHSV_8u_C3R(srcBuf, srcPitch, m_ImgHSV.buf, m_ImgHSV.nPitch,Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.buf+1, m_ImgHSV.nPitch, dstBuf, dstPitch, Roi);
		}
		if (HSV_V==nChangeType)
		{
			m_ImgHSV.Init(width,height,3);
			ippiRGBToHSV_8u_C3R(srcBuf,srcPitch,m_ImgHSV.buf,m_ImgHSV.nPitch,Roi);
			ippiCopy_8u_C3C1R(m_ImgHSV.buf+2,m_ImgHSV.nPitch, dstBuf, dstPitch, Roi);
		}

		if (LAB_L<=nChangeType && nChangeType<= LAB_B)
		{
			m_ImgLAB.Init(width,height,3);
			ippiBGRToLab_8u_C3R(srcBuf, srcPitch, m_ImgLAB.buf, m_ImgLAB.nPitch, Roi);
		}

		if (LAB_L==nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.buf, m_ImgLAB.nPitch, dstBuf, dstPitch, Roi);
		}
		if (LAB_A==nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.buf+1, m_ImgLAB.nPitch, dstBuf, dstPitch, Roi);
		}
		if (LAB_B==nChangeType)
		{
			ippiCopy_8u_C3C1R(m_ImgLAB.buf+2, m_ImgLAB.nPitch, dstBuf, dstPitch, Roi);
		}
		if (BGR_GRAY == nChangeType)
		{
			int order[3] = { 2, 1, 0 };
			ippiSwapChannels_8u_C3R(srcBuf, srcPitch, m_ImgBGR.buf, m_ImgBGR.nPitch, Roi, order);
			ippiRGBToGray_8u_C3C1R(m_ImgBGR.buf, m_ImgBGR.nPitch, dstBuf, dstPitch, Roi);
		}


	}
	return 1;
}




int CKxBaseFunction::readImgBufFromMemory(kxCImageBuf& imgBuf, unsigned char *&pt)
{
	imgBuf.Release();
	int /*nWidth,nHeight,nPitch,nChannel,*/nToken;
	memcpy( &imgBuf.nWidth, pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &imgBuf.nHeight, pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &imgBuf.nPitch, pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &imgBuf.nChannel, pt,sizeof(int));
	pt+=sizeof(int);
	memcpy( &nToken, pt,sizeof(int));
	pt+=sizeof(int);

	if (imgBuf.nWidth<0 || imgBuf.nWidth>10000000 || imgBuf.nHeight<0 || imgBuf.nHeight>10000000 ||  imgBuf.nPitch<0 || imgBuf.nPitch>10000000)
	{
		return false;
	}
	if( nToken )
	{
		imgBuf.buf = ippNew( imgBuf.nPitch*imgBuf.nHeight);
		imgBuf.bAuto = true;
		ippsCopy_8u(pt, imgBuf.buf,  sizeof(unsigned char)*imgBuf.nPitch*imgBuf.nHeight );
	}
	pt+=imgBuf.nPitch*imgBuf.nHeight*sizeof(unsigned char);
	return true;
}

int CKxBaseFunction::writeImgBufToMemory(kxCImageBuf& imgBuf, unsigned char *&pt)
{
	memcpy( pt, &imgBuf.nWidth, sizeof(int));
	pt+=sizeof(int);
	memcpy( pt, &imgBuf.nHeight, sizeof(int));
	pt+=sizeof(int);
	memcpy(pt, &imgBuf.nPitch, sizeof(int));
	pt+=sizeof(int);
	memcpy(pt, &imgBuf.nChannel, sizeof(int));
	pt+=sizeof(int);
	int  nToken = ( imgBuf.buf != NULL ? 1:0 );
	memcpy(pt, &nToken, sizeof(int));
	pt+=sizeof(int);

	if( nToken )
	{
		ippsCopy_8u(imgBuf.buf, pt,  sizeof(unsigned char)*imgBuf.nPitch*imgBuf.nHeight );
	}
	pt+=sizeof(unsigned char)*imgBuf.nPitch*imgBuf.nHeight;
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


std::string CKxBaseFunction::FormatImageToString(const kxCImageBuf& hImageBuffer)
{
	std::ostringstream os;
	int nToken = (NULL != hImageBuffer.buf ? 1 : 0);
	os.write(reinterpret_cast<const char *>(&hImageBuffer.nWidth), sizeof(int));
	os.write(reinterpret_cast<const char *>(&hImageBuffer.nHeight), sizeof(int));
	os.write(reinterpret_cast<const char *>(&hImageBuffer.nPitch), sizeof(int));
	os.write(reinterpret_cast<const char *>(&hImageBuffer.nChannel), sizeof(int));
	os.write(reinterpret_cast<const char *>(&nToken), sizeof(int));
	if (nToken)
	{
		os.write(reinterpret_cast<const char *>(hImageBuffer.buf), sizeof(unsigned char) * hImageBuffer.nPitch * hImageBuffer.nHeight);
	}
	if (os.fail())
	{
		return "";
	}
	return os.str();
}


//void CKxBaseFunction:: FreeImage_h(MV_IMAGE * img)
//{
//	if ((img->width > 0 || img->height > 0)&&(img->data))
//	{
//		delete []img->data;
//		img->data = NULL;
//		img->width = 0;
//		img->height = 0;
//		img->pitch = 0;
//		img->type = 0;
//		img = NULL;
//	}
//
//
//}
////add by lyl
//MV_IMAGE* CKxBaseFunction::LoadBMPImage_h(const char* path)
//{
//
//	MV_IMAGE* bmpImg;
//	FILE* pFile;
//	unsigned short fileType;
//	ClBitMapFileHeader bmpFileHeader;
//	ClBitMapInfoHeader bmpInfoHeader;
//	int channels = 1;
//	int width = 0;
//	int height = 0;
//	int step = 0;
//	int offset = 0;
//
//	ClRgbQuad* quad;
//
//
//	bmpImg = new MV_IMAGE;
//	//pFile = fopen(path, "rb");
//
//#if defined( _WIN32 ) || defined ( _WIN64 )
//	if (fopen_s(&pFile, path, "rb") != 0)
//	{
//		delete bmpImg;
//		return 0;
//	}
//#else
//	pFile = fopen(path, "rb");
//	if (pFile == NULL)
//	{
//		delete bmpImg;
//		return 0;
//	}
//#endif
//
//
//	fread(&fileType, sizeof(unsigned short), 1, pFile);
//	if (fileType == 0x4D42)
//	{
//		fread(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);
//		fread(&bmpInfoHeader, sizeof(ClBitMapInfoHeader), 1, pFile);
//
//		if (bmpInfoHeader.biBitCount == 8)
//		{
//			channels = 1;
//			width = bmpInfoHeader.biWidth;
//			height = bmpInfoHeader.biHeight;
//
//			bmpImg->width = width;
//			bmpImg->height = height;
//
//			offset = (channels*width)%4;
//			if (offset != 0)
//			{
//				offset = 4 - offset;
//			}
//			bmpImg->pitch = width + offset;
//
//
//			bmpImg->type = _Type_G8;
//			bmpImg->data = new unsigned char[bmpImg->pitch*bmpImg->height];
//
//
//			quad = new ClRgbQuad[256];
//			fread(quad, sizeof(ClRgbQuad), 256, pFile);
//			delete []quad;
//
//			unsigned char* pBuf = new unsigned char[bmpImg->pitch*bmpImg->height];
//			fread(pBuf, sizeof(unsigned char), bmpImg->pitch*bmpImg->height, pFile);
//			IppiSize roiSize = {bmpImg->width, bmpImg->height};
//			ippiCopy_8u_C1R(pBuf, bmpImg->pitch, (Ipp8u*)bmpImg->data, bmpImg->pitch, roiSize);
//			ippiMirror_8u_C1IR((Ipp8u*)bmpImg->data, bmpImg->pitch, roiSize, ippAxsHorizontal);
//			delete []pBuf;
//
//
//		}
//		else if (bmpInfoHeader.biBitCount == 24)
//		{
//			channels = 3;
//			width = bmpInfoHeader.biWidth;
//			height = bmpInfoHeader.biHeight;
//
//			bmpImg->width = width;
//			bmpImg->height = height;
//			bmpImg->type = _Type_G24;
//
//			offset = 3*width %4;
//			if (offset != 0)
//			{
//				offset = 4 - offset;
//			}
//
//			bmpImg->pitch = 3*width + offset;
//			bmpImg->data = (unsigned char*)new unsigned char[bmpImg->pitch*bmpImg->height];
//
//			unsigned char* pBuf = new unsigned char[bmpImg->pitch*bmpImg->height];
//			fread(pBuf, sizeof(unsigned char), bmpImg->pitch*bmpImg->height, pFile);
//			IppiSize roiSize = {bmpImg->width, bmpImg->height};
//			ippiCopy_8u_C3R(pBuf, bmpImg->pitch, (Ipp8u*)bmpImg->data, bmpImg->pitch, roiSize);
//			ippiMirror_8u_C3IR((Ipp8u*)bmpImg->data, bmpImg->pitch, roiSize, ippAxsHorizontal);
//			delete []pBuf;
//
//		}
//	}
//	fclose( pFile );
//
//	return bmpImg;
//}

int CKxBaseFunction::LoadBMPImage_h(const char* path, kxCImageBuf& readImg)
{
	KxCallStatus hCall;
	return LoadBMPImage_h(path, readImg, hCall);
}


//add by lyl  2016/7/5
int CKxBaseFunction::LoadBMPImage_h(const char* path, kxCImageBuf& readImg, KxCallStatus& hCall)
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

			if( readImg.buf && readImg.nWidth == width && readImg.nHeight == height && readImg.nChannel == _Type_G8)
			{

			}
			else
			{
				readImg.Release();
				readImg.buf = new unsigned char[nPitch*height];

			}

			readImg.nWidth = width;
			readImg.nHeight = height;
			readImg.nPitch = width + offset;
			readImg.nChannel = _Type_G8;
            readImg.bAuto = true;

			quad = new ClRgbQuad[256];
			fread(quad, sizeof(ClRgbQuad), 256, pFile);
			delete []quad;

			//unsigned char* pBuf = new unsigned char[readImg.nPitch*readImg.nHeight];
			unsigned char* pBuf = ippsMalloc_8u(readImg.nPitch*readImg.nHeight);
			fread(pBuf, sizeof(unsigned char), readImg.nPitch*readImg.nHeight, pFile);
			//m_TempImg.Init(readImg.nPitch, readImg.nHeight);
			//fread(m_TempImg.buf, sizeof(unsigned char), readImg.nPitch*readImg.nHeight, pFile);
			IppiSize roiSize = {readImg.nWidth, readImg.nHeight};
			status = ippiCopy_8u_C1R(pBuf, readImg.nPitch, readImg.buf, readImg.nPitch, roiSize);
			if (check_sts(status, "LoadBMPImage_h_ippiCopy_8u_C1R", hCall))
			{
				return 0;
			}

			status = ippiMirror_8u_C1IR(readImg.buf, readImg.nPitch, roiSize, ippAxsHorizontal);

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

			if( readImg.buf && readImg.nWidth == width && readImg.nHeight == height && readImg.nChannel == _Type_G24)
			{

			}
			else
			{
				readImg.Release();
				readImg.buf = new unsigned char[nPitch*height];
			}

			readImg.nWidth = width;
			readImg.nHeight = height;
			readImg.nChannel = _Type_G24;
			readImg.nPitch = 3*width + offset;
			readImg.bAuto = true;

			unsigned char* pBuf = ippsMalloc_8u(readImg.nPitch*readImg.nHeight);
			fread(pBuf, sizeof(unsigned char), readImg.nPitch*readImg.nHeight, pFile);

			//m_TempImg.Init(readImg.nPitch, readImg.nHeight);
			//fread(m_TempImg.buf, sizeof(unsigned char), readImg.nPitch*readImg.nHeight, pFile);
			IppiSize roiSize = {readImg.nWidth, readImg.nHeight};
			status = ippiCopy_8u_C3R(pBuf, readImg.nPitch, readImg.buf, readImg.nPitch, roiSize);

			if (check_sts(status, "LoadBMPImage_h_ippiCopy_8u_C3R", hCall))
			{
				return 0;
			}

			status = ippiMirror_8u_C3IR(readImg.buf, readImg.nPitch, roiSize, ippAxsHorizontal);

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

bool CKxBaseFunction::SaveBMPImage_h(const char* path, const kxCImageBuf& writeImg)
{
	KxCallStatus hCall;
	return SaveBMPImage_h(path, writeImg, hCall);
}

//add by lyl 2016//7/6
bool CKxBaseFunction::SaveBMPImage_h(const char* path, const kxCImageBuf& writeImg, KxCallStatus& hCall)
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

	if (writeImg.nChannel == _Type_G24)//24位，通道，彩图
	{
		step = writeImg.nPitch;
		offset = step%4;
		if (offset != 0)
		{
			step += (4-offset);
		}

		bmpFileHeader.bfSize = writeImg.nHeight*step + 54;
		bmpFileHeader.bfReserved1 = 0;
		bmpFileHeader.bfReserved2 = 0;
		bmpFileHeader.bfOffBits = 54;
		fwrite(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);

		bmpInfoHeader.biSize = 40;
		bmpInfoHeader.biWidth = writeImg.nWidth;
		bmpInfoHeader.biHeight = writeImg.nHeight;
		bmpInfoHeader.biPlanes = 1;
		bmpInfoHeader.biBitCount = 24;
		bmpInfoHeader.biCompression = 0;
		bmpInfoHeader.biSizeImage = writeImg.nHeight*step;
		bmpInfoHeader.biXPelsPerMeter = 0;
		bmpInfoHeader.biYPelsPerMeter = 0;
		bmpInfoHeader.biClrUsed = 0;
		bmpInfoHeader.biClrImportant = 0;
		fwrite(&bmpInfoHeader, sizeof(ClBitMapInfoHeader), 1, pFile);

		m_WriteTemp.Init(step, writeImg.nHeight);
		memset(m_WriteTemp.buf, 0, sizeof(unsigned char)*(writeImg.nHeight * step));
		IppiSize roiSize = {writeImg.nWidth, writeImg.nHeight};
		status = ippiCopy_8u_C3R(writeImg.buf, writeImg.nPitch, m_WriteTemp.buf, step, roiSize);

		if (check_sts(status, "SaveBMPImage_h_ippiCopy_8u_C3R", hCall))
		{
			return 0;
		}

		status = ippiMirror_8u_C3IR(m_WriteTemp.buf, step, roiSize, ippAxsHorizontal);

		if (check_sts(status, "SaveBMPImage_h_ippiCopy_8u_C3R", hCall))
		{
			return 0;
		}

		fwrite(m_WriteTemp.buf, sizeof(unsigned char), step*writeImg.nHeight, pFile);

	}
	else if (writeImg.nChannel == _Type_G8)//8位，单通道，灰度图
	{
		step = writeImg.nPitch;
		offset = step%4;
		if (offset != 0)
		{
			step += 4-offset;
		}

		bmpFileHeader.bfSize = 54 + 256*4 + step * writeImg.nHeight;
		bmpFileHeader.bfReserved1 = 0;
		bmpFileHeader.bfReserved2 = 0;
		bmpFileHeader.bfOffBits = 54 + 256*4;
		fwrite(&bmpFileHeader, sizeof(ClBitMapFileHeader), 1, pFile);

		bmpInfoHeader.biSize = 40;
		bmpInfoHeader.biWidth = writeImg.nWidth;
		bmpInfoHeader.biHeight = writeImg.nHeight;
		bmpInfoHeader.biPlanes = 1;
		bmpInfoHeader.biBitCount = 8;
		bmpInfoHeader.biCompression = 0;
		bmpInfoHeader.biSizeImage = writeImg.nHeight*step;
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

		m_WriteTemp.Init(step, writeImg.nHeight);
		memset(m_WriteTemp.buf, 0, sizeof(unsigned char)*(writeImg.nHeight * step));
		IppiSize roiSize = {writeImg.nWidth, writeImg.nHeight};
		status = ippiCopy_8u_C1R(writeImg.buf, writeImg.nPitch, m_WriteTemp.buf, step, roiSize);

		if (check_sts(status, "SaveBMPImage_h_ippiCopy_8u_C1R", hCall))
		{
			return 0;
		}

		status = ippiMirror_8u_C1IR(m_WriteTemp.buf, step, roiSize, ippAxsHorizontal);

		if (check_sts(status, "SaveBMPImage_h_ippiMirror_8u_C1IR", hCall))
		{
			return 0;
		}

		fwrite(m_WriteTemp.buf, sizeof(unsigned char), step*writeImg.nHeight, pFile);
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
//		ippiCopy_8u_C1R((Ipp8u*)bmpImg->data, bmpImg->pitch, TmpBuf.buf, TmpBuf.nPitch, roiSize);
//		MV_IMAGE hImg;
//		hImg.data = TmpBuf.GetImageBuf(hImg.width, hImg.height, hImg.pitch, hImg.type);
//		SaveBMPImage(path, &hImg);
//	}
//	if (bmpImg->type == _Type_G24)
//	{
//		TmpBuf.Init(bmpImg->width, bmpImg->height, 3);
//		IppiSize roiSize = {bmpImg->width, bmpImg->height};
//		ippiCopy_8u_C3R((Ipp8u*)bmpImg->data, bmpImg->pitch, TmpBuf.buf, TmpBuf.nPitch, roiSize);
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



int CKxBaseFunction::KxParallelCopy(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg)
{
	KxCallStatus hCall;
	//SrcDstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	return KxParallelCopy(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxParallelCopy(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	parallel_for(blocked_range<int>(0, SrcImg.nHeight, 16),
		[&](const blocked_range<int>& range)
	{
		IppiSize Roi = { (int)SrcImg.nWidth, (int)range.size() };
		Ipp8u* pSrc = SrcImg.buf + range.begin() * SrcImg.nPitch;
		Ipp8u* pDst = SrcDstImg.buf + range.begin() * SrcDstImg.nPitch ;
		if (_Type_G8 == SrcImg.nChannel)
		{
			ippiCopy_8u_C1R(pSrc, SrcImg.nPitch, pDst, SrcDstImg.nPitch, Roi);
		}
		if (_Type_G24 == SrcImg.nChannel)
		{
			ippiCopy_8u_C3R(pSrc, SrcImg.nPitch, pDst, SrcDstImg.nPitch, Roi);
		}		
	}, affinity_partitioner());

	return 1;
}




int CKxBaseFunction::KxParallelMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg)
{
	KxCallStatus hCall;
	return KxParallelMaxEvery(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxParallelMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, 0, SrcImg.nWidth),
		[&](const blocked_range2d<int, int>& range)
	{
		IppiSize Roi = { (int)range.cols().size(), (int)range.rows().size() };
		Ipp8u* pSrc = SrcImg.buf + range.rows().begin() * SrcImg.nPitch + range.cols().begin() * SrcImg.nChannel;
		Ipp8u* pDst = SrcDstImg.buf + range.rows().begin() * SrcDstImg.nPitch + range.cols().begin() * SrcDstImg.nChannel;

		if (_Type_G8 == SrcImg.nChannel)
		{
			ippiMaxEvery_8u_C1IR(pSrc, SrcImg.nPitch, pDst, SrcDstImg.nPitch, Roi);
		}
		if (_Type_G24 == SrcImg.nChannel)
		{
			ippiMaxEvery_8u_C3IR(pSrc, SrcImg.nPitch, pDst, SrcDstImg.nPitch, Roi);
		}
	}, auto_partitioner());

	return 1;
}

int CKxBaseFunction::KxParallelMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg)
{
	KxCallStatus hCall;
	return KxParallelMinEvery(SrcImg, SrcDstImg, hCall);
}


int CKxBaseFunction::KxParallelMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, 0, SrcImg.nWidth),
		[&](const blocked_range2d<int, int>& range)
	{
		IppiSize Roi = { (int)range.cols().size(), (int)range.rows().size() };
		Ipp8u* pSrc = SrcImg.buf + range.rows().begin() * SrcImg.nPitch + range.cols().begin() * SrcImg.nChannel;
		Ipp8u* pDst = SrcDstImg.buf + range.rows().begin() * SrcDstImg.nPitch + range.cols().begin() * SrcDstImg.nChannel;

		if (_Type_G8 == SrcImg.nChannel)
		{
			ippiMinEvery_8u_C1IR(pSrc, SrcImg.nPitch, pDst, SrcDstImg.nPitch, Roi);
		}
		if (_Type_G24 == SrcImg.nChannel)
		{
			ippiMinEvery_8u_C3IR(pSrc, SrcImg.nPitch, pDst, SrcDstImg.nPitch, Roi);
		}
	}, auto_partitioner());

	return 1;
}


int CKxBaseFunction::KxMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg)
{
	KxCallStatus hCall;
	return KxMaxEvery(SrcImg, SrcDstImg, hCall);
}

int CKxBaseFunction::KxMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppiSize Roi = {SrcImg.nWidth, SrcImg.nHeight};
	if(_Type_G8 == SrcImg.nChannel)
	{
		IppStatus status = ippiMaxEvery_8u_C1IR(SrcImg.buf, SrcImg.nPitch, SrcDstImg.buf, SrcDstImg.nPitch, Roi);
		if (check_sts(status, "KxMaxEvery_ippiMaxEvery_8u_C1IR", hCall))
		{
			return 0;
		}
	}
	if(_Type_G24 == SrcImg.nChannel)
	{
		IppStatus status = ippiMaxEvery_8u_C3IR(SrcImg.buf, SrcImg.nPitch, SrcDstImg.buf, SrcDstImg.nPitch, Roi);
		if (check_sts(status, "KxMaxEvery_ippiMaxEvery_8u_C3IR", hCall))
		{
			return 0;
		}
	}
	return 1;
}


int CKxBaseFunction::KxMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg)
{
	KxCallStatus hCall;
	return KxMinEvery(SrcImg, SrcDstImg, hCall);
}



int CKxBaseFunction::KxMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppiSize Roi = {SrcImg.nWidth, SrcImg.nHeight};
	if(_Type_G8 == SrcImg.nChannel)
	{
		IppStatus status = ippiMinEvery_8u_C1IR(SrcImg.buf, SrcImg.nPitch, SrcDstImg.buf, SrcDstImg.nPitch, Roi);

		if (check_sts(status, "KxMaxEvery_ippiMinEvery_8u_C1IR", hCall))
		{
			return 0;
		}
	}
	if(_Type_G24 == SrcImg.nChannel)
	{
		IppStatus status = ippiMinEvery_8u_C3IR(SrcImg.buf, SrcImg.nPitch, SrcDstImg.buf, SrcDstImg.nPitch, Roi);

		if (check_sts(status, "KxMaxEvery_ippiMinEvery_8u_C3IR", hCall))
		{
			return 0;
		}
	}
	return 1;
}


int CKxBaseFunction::GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg, kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy)
{
	KxCallStatus hCall;
	return GetImgOffset(SrcImg, KernImg, rcKern, nSearchExpand, nSearchDir, nDx, nDy, hCall);
}

int CKxBaseFunction::GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg,  kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall)
{
	kxRect<int> rcSearch;
	int nleft,nright,ntop,nbottom;
	if (nSearchDir == _Horiz_Vertical_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft() - nSearchExpand);
		ntop  = gMax(0, rcKern.GetTop() - nSearchExpand);
		nright = gMin(SrcImg.nWidth-1, rcKern.GetRight() + nSearchExpand);
		nbottom = gMin(SrcImg.nHeight-1, rcKern.GetBottom() + nSearchExpand);
	}
	else if(nSearchDir == _Vertical_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft());
		ntop = gMax(0, rcKern.GetTop() - nSearchExpand);
		nright = gMin(SrcImg.nWidth-1, rcKern.GetRight());
		nbottom = gMin(SrcImg.nHeight-1, rcKern.GetBottom()+ nSearchExpand);
	}
	else if (nSearchDir == _Horizontal_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft()- nSearchExpand);
		ntop = gMax(0, rcKern.GetTop());
		nright = gMin(SrcImg.nWidth-1, rcKern.GetRight() + nSearchExpand);
		nbottom = gMin(SrcImg.nHeight-1, rcKern.GetBottom());
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
	kxPoint<float>  pos;

    KxCallStatus hCallFirst;
	hCallFirst.Clear();

	//float fratio = kxImageAlign( pos, SrcImg.buf + rcSearch.GetTop()*SrcImg.nPitch+rcSearch.GetLeft()*SrcImg.nChannel, rcSearch.Width(), rcSearch.Height(), SrcImg.nPitch,
	//	                        KernImg.buf, KernImg.nWidth, KernImg.nHeight, KernImg.nPitch , KernImg.nChannel, hCallFirst);

	kxCImageBuf test;
	test.SetImageBuf(SrcImg.buf + rcSearch.GetTop()*SrcImg.nPitch + rcSearch.GetLeft()*SrcImg.nChannel, rcSearch.Width(), rcSearch.Height(), SrcImg.nPitch, SrcImg.nChannel, false);
	//float fratio = kxImageAlignColor(pos, test, KernImg);
	float fratio = kxImageAlign(pos, test, KernImg);

	//float fratio = kxImageAlignColor(pos, test.buf, test.nWidth, test.nHeight, test.nPitch,
	//	KernImg.buf, KernImg.nWidth, KernImg.nHeight, KernImg.nPitch, KernImg.nChannel);

	//char sz[128];
	//kxCImageBuf test;
	//test.Init(rcSearch.Width(), rcSearch.Height(), SrcImg.nChannel);
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

int  CKxBaseFunction::KxResizeNearestImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = {SrcImg.nWidth, SrcImg.nHeight};
	IppiSize dstSize = {ResizeImg.nWidth, ResizeImg.nHeight};
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
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, SrcImg.nChannel, &nBufferSize);
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

	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiResizeNearest_8u_C1R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
			dstSize, pSpec, pBuffer);
	}
	else if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiResizeNearest_8u_C3R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
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

int  CKxBaseFunction::KxParallelResizeNearestImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize dstSize = { ResizeImg.nWidth, ResizeImg.nHeight };
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

	int nRowActualGrain = ((std::min))(nRowGrain, ResizeImg.nHeight);
	int nColActualGrain = ((std::min))(nColGrain, ResizeImg.nWidth);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.nHeight, nRowActualGrain, 0, ResizeImg.nWidth, nColActualGrain),
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
			ResizeImg.nChannel, &localBufSize);

		Ipp8u *localBuffer = (Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);


		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.buf + (srcOffset.y*SrcImg.nPitch) + srcOffset.x*SrcImg.nChannel;
		pDstT = ResizeImg.buf + (dstOffset.y*ResizeImg.nPitch) + dstOffset.x*ResizeImg.nChannel;

		// do the resize for grayscale or color
		switch (ResizeImg.nChannel)
		{
		case 1:
			status = ippiResizeNearest_8u_C1R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeNearest_8u_C3R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}

int  CKxBaseFunction::KxParallelResizeLinearImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain , int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize dstSize = { ResizeImg.nWidth, ResizeImg.nHeight };
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

	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.nHeight);
	int nColActualGrain = ((std::min))(nColGrain, ResizeImg.nWidth);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.nHeight, nRowActualGrain, 0, ResizeImg.nWidth, nColActualGrain),
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
			ResizeImg.nChannel, &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.buf + (srcOffset.y*SrcImg.nPitch) + srcOffset.x*SrcImg.nChannel;
		pDstT = ResizeImg.buf + (dstOffset.y*ResizeImg.nPitch) + dstOffset.x*ResizeImg.nChannel;

		// do the resize for grayscale or color
		switch (ResizeImg.nChannel)
		{
		case 1:
			status = ippiResizeLinear_8u_C1R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeLinear_8u_C3R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}

int  CKxBaseFunction::KxParallelResizeCubicImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize dstSize = { ResizeImg.nWidth, ResizeImg.nHeight };
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
	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.nHeight);
	int nColActualGrain = (std::min)(nColGrain, ResizeImg.nWidth);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.nHeight, nRowActualGrain, 0, ResizeImg.nWidth, nColActualGrain),
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
			ResizeImg.nChannel, &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.buf + (srcOffset.y*SrcImg.nPitch) + srcOffset.x*SrcImg.nChannel;
		pDstT = ResizeImg.buf + (dstOffset.y*ResizeImg.nPitch) + dstOffset.x*ResizeImg.nChannel;

		// do the resize for grayscale or color
		switch (ResizeImg.nChannel)
		{
		case 1:
			status = ippiResizeCubic_8u_C1R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeCubic_8u_C3R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}


int  CKxBaseFunction::KxParallelResizeLanczosImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize dstSize = { ResizeImg.nWidth, ResizeImg.nHeight };
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

	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.nHeight);
	int nColActualGrain = (std::min)(nColGrain, ResizeImg.nWidth);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.nHeight, nRowActualGrain, 0, ResizeImg.nWidth, nColActualGrain),
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
			ResizeImg.nChannel, &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.buf + (srcOffset.y*SrcImg.nPitch) + srcOffset.x*SrcImg.nChannel;
		pDstT = ResizeImg.buf + (dstOffset.y*ResizeImg.nPitch) + dstOffset.x*ResizeImg.nChannel;

		// do the resize for grayscale or color
		switch (ResizeImg.nChannel)
		{
		case 1:
			status = ippiResizeLanczos_8u_C1R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		case 3:
			status = ippiResizeLanczos_8u_C3R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT, ippBorderRepl, 0, pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);

	return 1;
}


int  CKxBaseFunction::KxParallelResizeSuperImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus & hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize dstSize = { ResizeImg.nWidth, ResizeImg.nHeight };
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

	int nRowActualGrain = (std::min)(nRowGrain, ResizeImg.nHeight);
	int nColActualGrain = (std::min)(nColGrain, ResizeImg.nWidth);

	//* add parallel algorithm */
	parallel_for(blocked_range2d<int, int>(0, ResizeImg.nHeight, nRowActualGrain, 0, ResizeImg.nWidth, nColActualGrain),
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
			ResizeImg.nChannel, &localBufSize);

		Ipp8u *localBuffer =
			(Ipp8u*)scalable_aligned_malloc(localBufSize*sizeof(Ipp8u), 32);

		// given the destination offset, calculate the offset in the source image
		dstOffset.x = (int)range.cols().begin();
		dstOffset.y = (int)range.rows().begin();
		status = ippiResizeGetSrcOffset_8u(pSpec, dstOffset, &srcOffset);

		// pointers to the starting points within the buffers that this thread
		// will read from/write to
		pSrcT = SrcImg.buf + (srcOffset.y*SrcImg.nPitch) + srcOffset.x*SrcImg.nChannel;
		pDstT = ResizeImg.buf + (dstOffset.y*ResizeImg.nPitch) + dstOffset.x*ResizeImg.nChannel;

		// do the resize for grayscale or color
		switch (ResizeImg.nChannel)
		{
		case 1:
			status = ippiResizeSuper_8u_C1R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT,  pSpec, localBuffer); break;
		case 3:
			status = ippiResizeSuper_8u_C3R(pSrcT, SrcImg.nPitch, pDstT, ResizeImg.nPitch,
				dstOffset, dstSizeT,  pSpec, localBuffer); break;
		default:break; //only 1 and 3 channel images
		}

		scalable_aligned_free((void*)localBuffer);

	}, simple_partitioner());

	ippsFree(pInitBuf);
	ippsFree(pSpec);


	return 1;

}




int  CKxBaseFunction::KxResizeLinearImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = {SrcImg.nWidth, SrcImg.nHeight};
	IppiSize dstSize = {ResizeImg.nWidth, ResizeImg.nHeight};
	IppiRect srcRoi = {0, 0, srcSize.width, srcSize.height};

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
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, SrcImg.nChannel, &nBufferSize);
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
	IppiPoint dstOffset = {0, 0};
	IppiBorderType border = ippBorderRepl;

	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiResizeLinear_8u_C1R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}
	else if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiResizeLinear_8u_C3R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
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

int  CKxBaseFunction::KxResizeCubicImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = {SrcImg.nWidth, SrcImg.nHeight};
	IppiSize dstSize = {ResizeImg.nWidth, ResizeImg.nHeight};
	IppiRect srcRoi = {0, 0, srcSize.width, srcSize.height};

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
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, SrcImg.nChannel, &nBufferSize);
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
	IppiPoint dstOffset = {0, 0};
	IppiBorderType border = ippBorderRepl;

	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiResizeCubic_8u_C1R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}
	else if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiResizeCubic_8u_C3R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
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


int  CKxBaseFunction::KxResizeSuperImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = {SrcImg.nWidth, SrcImg.nHeight};
	IppiSize dstSize = {ResizeImg.nWidth, ResizeImg.nHeight};
	IppiRect srcRoi = {0, 0, srcSize.width, srcSize.height};

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
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, SrcImg.nChannel, &nBufferSize);
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
	IppiPoint dstOffset = {0, 0};
	IppiBorderType border = ippBorderRepl;

	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiResizeSuper_8u_C1R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
			dstSize, pSpec, pBuffer);
	}
	else if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiResizeSuper_8u_C3R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
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

int  CKxBaseFunction::KxResizeLanczosImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize srcSize = {SrcImg.nWidth, SrcImg.nHeight};
	IppiSize dstSize = {ResizeImg.nWidth, ResizeImg.nHeight};
	IppiRect srcRoi = {0, 0, srcSize.width, srcSize.height};
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
	status = ippiResizeGetBufferSize_8u(pSpec, dstSize, SrcImg.nChannel, &nBufferSize);
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
	IppiPoint dstOffset = {0, 0};
	//IppiBorderType border = ippBorderInMem;

	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiResizeLanczos_8u_C1R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
			dstSize, border, 0, pSpec, pBuffer);
	}
	else if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiResizeLanczos_8u_C3R(SrcImg.buf, SrcImg.nPitch, ResizeImg.buf, ResizeImg.nPitch, dstOffset,
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

int CKxBaseFunction::KxResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode)
{
	KxCallStatus hCall;
	return KxResizeImage(SrcImg, ResizeImg, nInterpolationMode, hCall);
}

int CKxBaseFunction::KxResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode, KxCallStatus& hCall)
{
	hCall.Clear();

	if (SrcImg.nWidth == ResizeImg.nWidth && SrcImg.nHeight == ResizeImg.nHeight && SrcImg.nPitch == ResizeImg.nPitch)
	{
		KxCallStatus hCallInfo;
		hCallInfo.Clear();
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		KxCopyImage(SrcImg, ResizeImg, rc, hCallInfo);
		if (check_sts(hCallInfo, "KxResizeImage_", hCall))
		{
			return 0;
		}
		return 1;
	}
	int nStatus = 1;

	switch( nInterpolationMode )
	{
	case KxNearest:
		{
			nStatus = KxResizeNearestImage(SrcImg, ResizeImg, hCall);
			break;
		}
	case KxLinear:
		{
			nStatus = KxResizeLinearImage(SrcImg, ResizeImg, hCall);
			break;
		}
	case KxCubic:
		{
			nStatus = KxResizeCubicImage(SrcImg, ResizeImg, hCall);
			break;

		}
	case KxLanczos:
		{
			nStatus = KxResizeLanczosImage(SrcImg, ResizeImg, hCall);
			break;

		}
	case KxSuper:
		{
			nStatus = KxResizeSuperImage(SrcImg, ResizeImg, hCall);
			break;

		}
	default:
		{
			nStatus = KxResizeLinearImage(SrcImg, ResizeImg, hCall);
			break;
		}

	}

	return nStatus;

}

int CKxBaseFunction::KxAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxAverageFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
    DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
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
		ippsCopy_16s(pMask, pKernel,nMaskWidth*nMaskHeight);
	}

	IppiSize kernSize = {nMaskWidth, nMaskHeight};
	IppiSize dstRoiSize = {SrcImg.nWidth, SrcImg.nHeight};
	Ipp16s pSum;
	ippsSum_16s_Sfs(pKernel, nMaskWidth*nMaskHeight, &pSum, 0);
	int nDivisor = pSum;
	int nSpecSize = 0, nBufferSize = 0;
	//* Initializes the Buffer */
	status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, SrcImg.nChannel, &nSpecSize, &nBufferSize);
	if (check_sts(status, "KxAverageFilterImage_ippiFilterBorderGetSize", hCall))
	{
		ippsFree(pKernel);
		return 0;
	}
	IppiFilterBorderSpec* pSepc = (IppiFilterBorderSpec*)ippsMalloc_8u(nSpecSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

    // * Initializes the filter specification structure */
	status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, SrcImg.nChannel, ippRndNear, pSepc);
	if (check_sts(status, "KxAverageFilterImage_ippiFilterBorderInit_16s", hCall))
	{
		ippsFree(pKernel);
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}

	//Filters an image */
	IppiBorderType borderType = ippBorderRepl;
	if (SrcImg.nChannel == _Type_G8)
	{
		Ipp8u borderValue = 0;
		status = ippiFilterBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
	}
	if (SrcImg.nChannel == _Type_G24)
	{
		Ipp8u borderValue[3] = {0, 0, 0};
		status = ippiFilterBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
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

	return 1;

}

int CKxBaseFunction::KxParallelAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxParallelAverageFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}


int CKxBaseFunction::KxParallelAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	///IppStatus status;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
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
	IppiSize dstRoiSize = { SrcImg.nWidth, SrcImg.nHeight };
	Ipp16s pSum;
	ippsSum_16s_Sfs(pKernel, nMaskWidth*nMaskHeight, &pSum, 0);
	int nDivisor = pSum;

	int nRowActualGrain = gMin(nRowGrain, SrcImg.nHeight);
	int nColActualGrain = gMin(nColGrain, SrcImg.nWidth);

	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, nRowActualGrain, 0, SrcImg.nWidth, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.buf + range.rows().begin() * SrcImg.nPitch + range.cols().begin() * SrcImg.nChannel;
		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;

		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, SrcImg.nChannel, &nSpecSize, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiFilterBorderSpec *pSepc = (IppiFilterBorderSpec*)scalable_aligned_malloc(nSpecSize*sizeof(Ipp8u), 32);

		// * Initializes the filter specification structure */
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, SrcImg.nChannel, ippRndNear, pSepc);

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (SrcImg.nChannel == _Type_G8)
		{
			Ipp8u borderValue = 0;
			status = ippiFilterBorder_8u_C1R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
		}
		if (SrcImg.nChannel == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_8u_C3R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
		}

		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pSepc);

	}, simple_partitioner());

	ippsFree(pKernel);

	return 1;

}

int CKxBaseFunction::KxParallelBoxFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight)
{
	KxCallStatus hCall;
	return KxParallelBoxFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, hCall);
}

int CKxBaseFunction::KxParallelBoxFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,  KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	///IppStatus status;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);


	IppiSize kernSize = { nMaskWidth, nMaskHeight };
	IppiSize dstRoiSize = { SrcImg.nWidth, SrcImg.nHeight };


	int nRowActualGrain = gMin(nRowGrain, SrcImg.nHeight);
	int nColActualGrain = gMin(nColGrain, SrcImg.nWidth);

	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, nRowActualGrain, 0, SrcImg.nWidth, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.buf + range.rows().begin() * SrcImg.nPitch + range.cols().begin() * SrcImg.nChannel;
		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;

		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBoxBorderGetBufferSize(dstRoiSize, kernSize, ipp8u, SrcImg.nChannel, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (SrcImg.nChannel == _Type_G8)
		{
			Ipp8u borderValue = 0;
			status = ippiFilterBoxBorder_8u_C1R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, kernSize, borderType, &borderValue, pBuffer);
		}
		if (SrcImg.nChannel == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBoxBorder_8u_C3R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, kernSize, borderType, borderValue, pBuffer);
		}
		scalable_aligned_free((void*)pBuffer);

	}, auto_partitioner());



	return 1;

}


int CKxBaseFunction::KxMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight)
{
	KxCallStatus hCall;
	return KxMedianFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, hCall);
}



int CKxBaseFunction::KxMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall)
{
	hCall.Clear();
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
    IppStatus status;
	IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
	IppiSize maskSize = {nMaskWidth, nMaskHeight};
	int nBufferSize = 0;
	if (check_sts( status = ippiFilterMedianBorderGetBufferSize(roiSize, maskSize, ipp8u, SrcImg.nChannel, &nBufferSize), "KxMedianFilterImage_ippiFilterMedianBorderGetBufferSize" , hCall))
	{
		return 0;
	}
	Ipp8u*	pBuffer = ippsMalloc_8u(nBufferSize);
    IppiBorderType borderType = ippBorderRepl;
	if (SrcImg.nChannel == _Type_G8)
	{
		Ipp8u borderValue = 0;
		if (check_sts( status = ippiFilterMedianBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, maskSize, borderType, borderValue, pBuffer),
			           "KxMedianFilterImage_ippiFilterMedianBorder_8u_C1R" , hCall ))
		{
			ippsFree(pBuffer);
			return 0;
		}
	}
	if (SrcImg.nChannel == _Type_G24)
	{
		Ipp8u borderValue[3] = {0, 0, 0};
		if (check_sts( status = ippiFilterMedianBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, maskSize, borderType, borderValue, pBuffer),
			          "KxMedianFilterImage_ippiFilterMedianBorder_8u_C3R" , hCall))
		{
			ippsFree(pBuffer);
			return 0;
		}
	}
	ippsFree(pBuffer);

	return 1;

}


int CKxBaseFunction::KxParallelMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight)
{
	KxCallStatus hCall;
	return KxParallelMedianFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, hCall);
}


int CKxBaseFunction::KxParallelMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	//IppStatus status;
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize maskSize = { nMaskWidth, nMaskHeight };


	int nRowActualGrain = gMin(nRowGrain, SrcImg.nHeight);
	int nColActualGrain = gMin(nColGrain, SrcImg.nWidth);

	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, 0, SrcImg.nWidth),
		[&SrcImg, &DstImg, &maskSize](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.buf + range.rows().begin() * SrcImg.nPitch + range.cols().begin() * SrcImg.nChannel;
		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;

		//* Initializes the Buffer */
		int nBufferSize = 0;
		status = ippiFilterMedianBorderGetBufferSize(dstRoiSize, maskSize, ipp8u, SrcImg.nChannel, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiBorderType borderType = ippBorderRepl;
		if (SrcImg.nChannel == _Type_G8)
		{
			Ipp8u borderValue = 0;
			status = ippiFilterMedianBorder_8u_C1R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, maskSize, borderType, borderValue, pBuffer);

		}
		if (SrcImg.nChannel == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterMedianBorder_8u_C3R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, maskSize, borderType, borderValue, pBuffer);

		}
		scalable_aligned_free((void*)pBuffer);

	},simple_partitioner());


	return 1;

}

int CKxBaseFunction::KxThreshImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nLowThresh, int nHighThresh, int nThreshLayer)
{
	KxCallStatus hCall;
	return KxThreshImage(SrcImg, DstImg, nLowThresh, nHighThresh, nThreshLayer, hCall);
}

int CKxBaseFunction::KxThreshImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nLowThresh, int nHighThresh, int nThreshLayer, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_ImgLowThresh.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_ImgHighThresh.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize RoiSize = {SrcImg.nWidth, SrcImg.nHeight};
	if (_Type_G8 == SrcImg.nChannel)
	{
		status = ippiCompareC_8u_C1R(SrcImg.buf, SrcImg.nPitch, (Ipp8u)nLowThresh,  m_ImgLowThresh.buf, m_ImgLowThresh.nPitch,  RoiSize, ippCmpGreaterEq);
		if (check_sts(status, "KxThreshImage_G8_ippiCompareC_8u_C1R_first", hCall))
		{
			return 0;
		}

		if (nHighThresh != 255)
		{
			status = ippiCompareC_8u_C1R(SrcImg.buf, SrcImg.nPitch, (Ipp8u)nHighThresh, m_ImgHighThresh.buf, m_ImgHighThresh.nPitch, RoiSize, ippCmpLessEq);
			if (check_sts(status, "KxThreshImage_G8_ippiCompareC_8u_C1R_second", hCall))
			{
				return 0;
			}

			status = ippiAnd_8u_C1R(m_ImgHighThresh.buf, m_ImgHighThresh.nPitch, m_ImgLowThresh.buf, m_ImgLowThresh.nPitch, DstImg.buf, DstImg.nPitch, RoiSize);
			if (check_sts(status, "KxThreshImage_G8_ippiAnd_8u_C1R", hCall))
			{
				return 0;
			}
		}
		else
		{
			status = ippiCopy_8u_C1R(m_ImgLowThresh.buf, m_ImgLowThresh.nPitch,  DstImg.buf, DstImg.nPitch, RoiSize);
		}

	}
	if (_Type_G24 == SrcImg.nChannel)
	{
		m_ImgGray.Init(SrcImg.nWidth, SrcImg.nHeight);
		KxConvertImageLayer(SrcImg, m_ImgGray, nThreshLayer);
		status = ippiCompareC_8u_C1R(m_ImgGray.buf, m_ImgGray.nPitch, (Ipp8u)nLowThresh, m_ImgLowThresh.buf, m_ImgLowThresh.nPitch, RoiSize, ippCmpGreaterEq);
		if (check_sts(status, "KxThreshImage_G24_ippiCompareC_8u_C1R_first", hCall))
		{
			return 0;
		}

		if (nHighThresh != 255)
		{
			status = ippiCompareC_8u_C1R(m_ImgGray.buf, m_ImgGray.nPitch, (Ipp8u)nHighThresh, m_ImgHighThresh.buf, m_ImgHighThresh.nPitch, RoiSize, ippCmpLessEq);
			if (check_sts(status, "KxThreshImage_G24_ippiCompareC_8u_C1R_second", hCall))
			{
				return 0;
			}
			status = ippiAnd_8u_C1R(m_ImgHighThresh.buf, m_ImgHighThresh.nPitch, m_ImgLowThresh.buf, m_ImgLowThresh.nPitch, DstImg.buf, DstImg.nPitch, RoiSize);
			if (check_sts(status, "KxThreshImage_G24_ippiAnd_8u_C1R", hCall))
			{
				return 0;
			}
		}
		else
		{
			status = ippiCopy_8u_C1R(m_ImgLowThresh.buf, m_ImgLowThresh.nPitch, DstImg.buf, DstImg.nPitch, RoiSize);
		}
	}

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

int CKxBaseFunction::KxRotateImageSpecialAngle(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nAngle)
{
	KxCallStatus hCall;
	return KxRotateImageSpecialAngle(SrcImg, DstImg, nAngle, hCall);
}


int CKxBaseFunction::KxRotateImageSpecialAngle(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg,  int nAngle, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize  Size = {SrcImg.nWidth, SrcImg.nHeight};
	switch (nAngle)
	{
	case  _Angle_0:
		{
			DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
			kxRect<int> rc;
			rc.setup(0, 0, SrcImg.nWidth-1, SrcImg.nHeight-1);
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
			DstImg.Init(SrcImg.nHeight, SrcImg.nWidth, SrcImg.nChannel);
			m_ImgRotate.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
			if (_Type_G8 == SrcImg.nChannel)
			{
				status = ippiMirror_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_ImgRotate.buf, m_ImgRotate.nPitch,Size,ippAxsHorizontal);
				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_90_ippiMirror_8u_C1R", hCall))
				{
					return 0;
				}
				status = ippiTranspose_8u_C1R(m_ImgRotate.buf, m_ImgRotate.nPitch, DstImg.buf, DstImg.nPitch, Size);
				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_90_ippiTranspose_8u_C1R", hCall))
				{
					return 0;
				}
			}
			if (_Type_G24 == SrcImg.nChannel)
			{
				status = ippiMirror_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ImgRotate.buf, m_ImgRotate.nPitch,Size,ippAxsHorizontal);
				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_90_ippiMirror_8u_C3R", hCall))
				{
					return 0;
				}

				status = ippiTranspose_8u_C3R(m_ImgRotate.buf, m_ImgRotate.nPitch, DstImg.buf, DstImg.nPitch, Size);
				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_90_ippiTranspose_8u_C3R", hCall))
				{
					return 0;
				}
			}
			break;
		}
	case _Angle_180:
		{
			DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
			if (_Type_G8 == SrcImg.nChannel)
			{
				status = ippiMirror_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch,Size,ippAxsBoth);

				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_180_ippiMirror_8u_C1R", hCall))
				{
					return 0;
				}

			}
			if (_Type_G24 == SrcImg.nChannel)
			{
				status = ippiMirror_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch,Size,ippAxsBoth);

				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_180_ippiMirror_8u_C3R", hCall))
				{
					return 0;
				}
			}
			break;
		}
	case _Angle_270:
		{
			DstImg.Init(SrcImg.nHeight, SrcImg.nWidth, SrcImg.nChannel);
			m_ImgRotate.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
			if (_Type_G8 == SrcImg.nChannel)
			{
				status = ippiMirror_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_ImgRotate.buf, m_ImgRotate.nPitch,Size,ippAxsVertical);

				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_270_ippiMirror_8u_C1R", hCall))
				{
					return 0;
				}

				status = ippiTranspose_8u_C1R(m_ImgRotate.buf, m_ImgRotate.nPitch, DstImg.buf, DstImg.nPitch, Size);

				if (check_sts(status, "KxRotateImageSpecialAngle_G8_Angle_270_ippiTranspose_8u_C1R", hCall))
				{
					return 0;
				}
			}
			if (_Type_G24 == SrcImg.nChannel)
			{
				status = ippiMirror_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ImgRotate.buf, m_ImgRotate.nPitch,Size,ippAxsVertical);
				if (check_sts(status, "KxRotateImageSpecialAngle_G24_Angle_270_ippiMirror_8u_C3R", hCall))
				{
					return 0;
				}
				status = ippiTranspose_8u_C3R(m_ImgRotate.buf, m_ImgRotate.nPitch, DstImg.buf, DstImg.nPitch, Size);
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

int CKxBaseFunction::KxWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxWarpBilinearLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}

int CKxBaseFunction::KxWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize DstSize = { DstImg.nWidth, DstImg.nHeight };

	int nBufferSize = 0;
	int nChannelNum = SrcImg.nChannel;

	IppiRect srcRoi = { 0, 0, SrcImg.nWidth, SrcImg.nHeight };
	IppiRect dstRoi = { 0, 0, DstImg.nWidth, DstImg.nHeight };
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
		status = ippiWarpBilinearQuad_8u_C1R(SrcImg.buf, SrcSize, SrcImg.nPitch, srcRoi, warpSrcQuad, DstImg.buf, DstImg.nPitch, dstRoi, warpDstQuad, IPPI_INTER_LINEAR,  pBuffer);
	}
	else if (3 == nChannelNum)
	{
		status = ippiWarpBilinearQuad_8u_C3R(SrcImg.buf, SrcSize, SrcImg.nPitch, srcRoi, warpSrcQuad, DstImg.buf, DstImg.nPitch, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
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

int CKxBaseFunction::KxParalleWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxParalleWarpBilinearLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}


int CKxBaseFunction::KxParalleWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain, int nColGrain)
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
	ippsSet_8u(0xff, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	//自动模式会出现个别位置没有值，手动分块模式减少概率，暂未找到原因？
	int nRowActualGrain = gMin(nRowGrain, DstImg.nHeight);
	int nColActualGrain = gMin(nColGrain, DstImg.nWidth);
	parallel_for(blocked_range2d<int, int>(0, DstImg.nHeight, nRowActualGrain, 0, DstImg.nWidth, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		//方法一、
		//IppStatus status;
		//Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;
		//IppiRect dstRoi = { range.cols().begin(), range.rows().begin(), (int)range.cols().size(), (int)range.rows().size() };
		//double warpSrcQuad[4][2];
		//status = ippiGetBilinearQuad(dstRoi, warpSrcQuad, coeff);
		//double warpDstQuad[4][2] = { { range.cols().begin(), range.rows().begin() }, { range.cols().begin() + (int)range.cols().size() - 1, range.rows().begin() },
		//{ range.cols().begin() + (int)range.cols().size() - 1, range.rows().begin() + (int)range.rows().size() - 1 }, { range.cols().begin(), range.rows().begin() + (int)range.rows().size() - 1 } };

		//IppiSize SrcSize = { SrcImg.nWidth, SrcImg.nHeight };
		//IppiSize DstSize = { (int)range.cols().size(), (int)range.rows().size() };

		//int nBufferSize = 0;
		//int nChannelNum = SrcImg.nChannel;

		//IppiRect srcRoi = { 0, 0, SrcImg.nWidth, SrcImg.nHeight };
		//
		////* Spec and init buffer sizes */
		//status = ippiWarpBilinearQuadGetBufferSize(SrcSize, srcRoi, warpSrcQuad, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, &nBufferSize);
		////status = ippiWarpBilinearGetBufferSize(SrcSize, srcRoi, dstRoi, ippWarpBackward, coeff, IPPI_INTER_LINEAR,  &nBufferSize);

		//Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		////* Perform Bilinear transform  */
		//if (1 == nChannelNum)
		//{
		//	status = ippiWarpBilinearQuad_8u_C1R(SrcImg.buf, SrcSize, SrcImg.nPitch, srcRoi, warpSrcQuad, DstImg.buf, DstImg.nPitch, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
		//}
		//else if (3 == nChannelNum)
		//{
		//	status = ippiWarpBilinearQuad_8u_C3R(SrcImg.buf, SrcSize, SrcImg.nPitch, srcRoi, warpSrcQuad, DstImg.buf, DstImg.nPitch, dstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
		//}

		//scalable_aligned_free((void*)pBuffer);

		IppStatus status;
		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;
		IppiRect dstRoi = { range.cols().begin(), range.rows().begin(), (int)range.cols().size(), (int)range.rows().size() };

		IppiSize SrcSize = { SrcImg.nWidth, SrcImg.nHeight };
		IppiSize DstSize = { (int)range.cols().size(), (int)range.rows().size() };

		int nBufferSize = 0;
		int nChannelNum = SrcImg.nChannel;

		IppiRect srcRoi = { 0, 0, SrcImg.nWidth, SrcImg.nHeight };

		//* Spec and init buffer sizes */
		status = ippiWarpBilinearGetBufferSize(SrcSize, srcRoi, dstRoi, ippWarpBackward, coeff, IPPI_INTER_LINEAR,  &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		//* Perform Bilinear transform  */
		if (1 == nChannelNum)
		{
			status = ippiWarpBilinearBack_8u_C1R(SrcImg.buf, SrcSize, SrcImg.nPitch, srcRoi, DstImg.buf, DstImg.nPitch, dstRoi, coeff, IPPI_INTER_LINEAR, pBuffer);
		}
		else if (3 == nChannelNum)
		{
			status = ippiWarpBilinearBack_8u_C3R(SrcImg.buf, SrcSize, SrcImg.nPitch, srcRoi, DstImg.buf, DstImg.nPitch, dstRoi, coeff, IPPI_INTER_LINEAR, pBuffer);
		}

		scalable_aligned_free((void*)pBuffer);



	}, simple_partitioner());




	return 1;
}


int CKxBaseFunction::KxWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxWarpPerspectiveLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}



int CKxBaseFunction::KxWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize DstSize = { DstImg.nWidth, DstImg.nHeight };
	IppiWarpTransformType warpTransformType = ippWarpPerspective;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.nChannel;

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
		status = ippiWarpPerspectiveLinear_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstOffset, DstSize, pSepc, pBuffer);
	}
	else if (3 == nChannelNum)
	{
		status = ippiWarpPerspectiveLinear_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstOffset, DstSize, pSepc, pBuffer);
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


int CKxBaseFunction::KxParalleWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxParalleWarpPerspectiveLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}


int CKxBaseFunction::KxParalleWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize DstSize = { DstImg.nWidth, DstImg.nHeight };
	IppiWarpTransformType warpTransformType = ippWarpPerspective;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.nChannel;



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

	int nRowActualGrain = gMin(nRowGrain, DstImg.nHeight);
	int nColActualGrain = gMin(nColGrain, DstImg.nWidth);

	//自动模式不会受影响
	parallel_for(blocked_range2d<int, int>(0, DstImg.nHeight, nRowActualGrain, 0, DstImg.nWidth, nColActualGrain),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };
		//Ipp8u* pOrigT = SrcImg.buf;
		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;
		int nBufferSize = 0;
		status = ippiWarpGetBufferSize(pSepc, dstRoiSize, &nBufferSize);
		//Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);

		IppiPoint dstOffset = { (int)range.cols().begin(), (int)range.rows().begin() };

		//* Perform perspective transform  */
		if (1 == SrcImg.nChannel)
		{
			status = ippiWarpPerspectiveLinear_8u_C1R(SrcImg.buf, SrcImg.nPitch, pDstT, DstImg.nPitch, dstOffset, dstRoiSize, pSepc, pBuffer);
		}
		else if (3 == SrcImg.nChannel)
		{
			status = ippiWarpPerspectiveLinear_8u_C3R(SrcImg.buf, SrcImg.nPitch, pDstT, DstImg.nPitch, dstOffset, dstRoiSize, pSepc, pBuffer);
		}
		scalable_aligned_free((void*)pBuffer);


	},auto_partitioner());



	ippsFree(pSepc);


	return 1;
}

int CKxBaseFunction::KxWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxWarpAffineLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}



int CKxBaseFunction::KxWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = {SrcImg.nWidth, SrcImg.nHeight};
	IppiSize DstSize = {DstImg.nWidth, DstImg.nHeight};
	IppiWarpTransformType warpTransformType = ippWarpAffine;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.nChannel;

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
		status = ippiWarpAffineLinear_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstOffset, DstSize, pSepc, pBuffer);
	}
	else if (3 == nChannelNum)
	{
		status = ippiWarpAffineLinear_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstOffset, DstSize, pSepc, pBuffer);
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



int CKxBaseFunction::KxParalleWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2])
{
	KxCallStatus hCall;
	return KxParalleWarpAffineLinearQuadImgae(SrcImg, DstImg, warpSrcQuad, warpDstQuad, hCall);
}



int CKxBaseFunction::KxParalleWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	IppStatus status;

	IppiSize SrcSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppiSize DstSize = { DstImg.nWidth, DstImg.nHeight };
	IppiWarpTransformType warpTransformType = ippWarpAffine;
	IppiBorderType borderType = ippBorderTransp;
	int nSpecSize = 0;
	int nBufferSize = 0;
	int nChannelNum = SrcImg.nChannel;

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
	int nRowActualGrain = gMin(nRowGrain, DstImg.nHeight);
	int nColActualGrain = gMin(nColGrain, DstImg.nWidth);

	parallel_for(blocked_range2d<int, int>(0, DstImg.nHeight, 0, DstImg.nWidth),
		[&](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };
		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;
		int nBufferSize;
		status = ippiWarpGetBufferSize(pSepc, dstRoiSize, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiPoint dstOffset = { (int)range.cols().begin(), (int)range.rows().begin() };

		//* Perform perspective transform  */
		if (1 == SrcImg.nChannel)
		{
			status = ippiWarpAffineLinear_8u_C1R(SrcImg.buf, SrcImg.nPitch, pDstT, DstImg.nPitch, dstOffset, dstRoiSize, pSepc, pBuffer);
		}
		else if (3 == SrcImg.nChannel)
		{
			status = ippiWarpAffineLinear_8u_C3R(SrcImg.buf, SrcImg.nPitch, pDstT, DstImg.nPitch, dstOffset, dstRoiSize, pSepc, pBuffer);
		}

		scalable_aligned_free((void*)pBuffer);

	}, auto_partitioner());


	ippsFree(pSepc);

	return 1;

}

int CKxBaseFunction::KxFilterSobelImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskSize)
{
	KxCallStatus hCall;
	return KxFilterSobelImage(SrcImg, DstImg, nMaskSize, hCall);
}

int CKxBaseFunction::KxFilterSobelImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskSize, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
	DstImg.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
    m_ImageFilter.Init(SrcImg.nWidth, SrcImg.nHeight, sizeof(Ipp16s)*SrcImg.nChannel);

	if (_Type_G8 == SrcImg.nChannel)
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

		status = ippiFilterSobel_8u16s_C1R(SrcImg.buf, SrcImg.nPitch, (Ipp16s*)m_ImageFilter.buf, m_ImageFilter.nPitch, roiSize, MskSize, normType, ippBorderRepl, 0, pBuffer);

		if (check_sts(status,"KxFilterSobelImage_ippiFilterSobel_8u16s_C1R", hCall))
		{
			ippsFree(pBuffer);
			return 0;
		}

		ippsFree(pBuffer);
        ippiDivC_16s_C1IRSfs(factor, (Ipp16s*)m_ImageFilter.buf, m_ImageFilter.nPitch, roiSize, 0);
		ippiConvert_16s8u_C1R((Ipp16s*)m_ImageFilter.buf, m_ImageFilter.nPitch, DstImg.buf, DstImg.nPitch, roiSize);
	}
	if (_Type_G24 == SrcImg.nChannel)
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
		IppiSize dstRoiSize = {SrcImg.nWidth, SrcImg.nHeight};
		int nDivisor = 1;
		int nSpecSize = 0, nBufferSize = 0;
        m_ImageConvet.Init(SrcImg.nWidth, SrcImg.nWidth, sizeof(Ipp16s)*SrcImg.nChannel);
		m_ImageTemp.Init(SrcImg.nWidth, SrcImg.nWidth, sizeof(Ipp16s)*SrcImg.nChannel);
		ippsSet_8u(0, m_ImageFilter.buf, m_ImageFilter.nPitch*m_ImageFilter.nHeight);

		ippiConvert_8u16s_C3R(SrcImg.buf, SrcImg.nPitch, (Ipp16s*)m_ImageConvet.buf, m_ImageConvet.nPitch, roiSize);
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
			status = ippiFilterBorder_16s_C3R((Ipp16s*)m_ImageConvet.buf, m_ImageConvet.nPitch, (Ipp16s*)m_ImageTemp.buf, m_ImageTemp.nPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);

			if (check_sts(status, "KxFilterSobelImage_ippiFilterBorder_16s_C3R", hCall))
			{
				return 0;
			}

			ippiAbs_16s_C3IR((Ipp16s*)m_ImageTemp.buf, m_ImageTemp.nPitch, roiSize);
			ippiAdd_16s_C3IRSfs((Ipp16s*)m_ImageTemp.buf, m_ImageTemp.nPitch, (Ipp16s*)m_ImageFilter.buf, m_ImageFilter.nPitch, roiSize, 0);

			delete [] pKernel[i];
			pKernel[i] = NULL;
		}
		ippiDivC_16s_C3IRSfs(factor, (Ipp16s*)m_ImageFilter.buf, m_ImageFilter.nPitch, roiSize, 0);
		ippiConvert_16s8u_C3R((Ipp16s*)m_ImageFilter.buf, m_ImageFilter.nPitch, DstImg.buf, DstImg.nPitch, roiSize);
	}

	return 1;
}


int CKxBaseFunction::KxGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, Ipp16s scale)
{
	KxCallStatus hCall;
	return KxGeneralFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, scale, hCall);
}


int CKxBaseFunction::KxGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, Ipp16s scale, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	Ipp16s* pKernel = NULL;
	pKernel = ippsMalloc_16s(nMaskWidth*nMaskHeight);
	if (NULL == pMask)
	{
		ippsSet_16s(1, pKernel, nMaskWidth*nMaskHeight);
	}
	else
	{
		ippsCopy_16s(pMask, pKernel,nMaskWidth*nMaskHeight);
	}

	IppiSize kernSize = {nMaskWidth, nMaskHeight};
	IppiSize dstRoiSize = {SrcImg.nWidth, SrcImg.nHeight};
	//Ipp16s pSum;
	//ippsSum_16s_Sfs(pMask, nMaskWidth*nMaskHeight, &pSum, 0);
	int nDivisor = int(scale);
	int nSpecSize = 0, nBufferSize = 0;
	//* Convert 8u image to 16s */
	m_Image16s.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel * sizeof(Ipp16s));
	m_Image16sAbs.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel * sizeof(Ipp16s));
	if (_Type_G8 == SrcImg.nChannel)
	{
		status = ippiConvert_8u16s_C1R(SrcImg.buf, SrcImg.nPitch, (Ipp16s*)m_Image16s.buf, m_Image16s.nPitch, dstRoiSize);
	}
	else if (_Type_G24 == SrcImg.nChannel)
	{
		status = ippiConvert_8u16s_C3R(SrcImg.buf, SrcImg.nPitch, (Ipp16s*)m_Image16s.buf, m_Image16s.nPitch, dstRoiSize);
	}
	else
	{
		return 0;
	}

	//* Initializes the Buffer */
	status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp16s, ipp16s, SrcImg.nChannel, &nSpecSize, &nBufferSize);
	if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderGetSize", hCall))
	{
		ippsFree(pKernel);
		return 0;
	}
	IppiFilterBorderSpec* pSepc = (IppiFilterBorderSpec*)ippsMalloc_8u(nSpecSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

	// * Initializes the filter specification structure */
	status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp16s, SrcImg.nChannel, ippRndNear, pSepc);
	if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderInit_16s", hCall))
	{
		ippsFree(pKernel);
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}

	//Filters an image */
	IppiBorderType borderType = ippBorderRepl;
	if (SrcImg.nChannel == _Type_G8)
	{
		Ipp16s borderValue = 0;
		status = ippiFilterBorder_16s_C1R((Ipp16s*)m_Image16s.buf, m_Image16s.nPitch, (Ipp16s*)m_Image16sAbs.buf, m_Image16sAbs.nPitch, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
		if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorder_16s_C1R", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}
		ippiAbs_16s_C1IR((Ipp16s*)m_Image16sAbs.buf, m_Image16sAbs.nPitch, dstRoiSize);
		ippiConvert_16s8u_C1R((Ipp16s*)m_Image16sAbs.buf, m_Image16sAbs.nPitch, DstImg.buf, DstImg.nPitch, dstRoiSize);

	}
	if (SrcImg.nChannel == _Type_G24)
	{
		Ipp16s borderValue[3] = {0, 0, 0};
		status = ippiFilterBorder_16s_C3R((Ipp16s*)m_Image16s.buf, m_Image16s.nPitch, (Ipp16s*)m_Image16sAbs.buf, m_Image16sAbs.nPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
		if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorder_16s_C3R", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}
		ippiAbs_16s_C3IR((Ipp16s*)m_Image16sAbs.buf, m_Image16sAbs.nPitch, dstRoiSize);
		ippiConvert_16s8u_C3R((Ipp16s*)m_Image16sAbs.buf, m_Image16sAbs.nPitch, DstImg.buf, DstImg.nPitch, dstRoiSize);
	}


	ippsFree(pKernel);
	ippsFree(pSepc);
	ippsFree(pBuffer);

	return 1;

}

int CKxBaseFunction::KxGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxGeneralFilterImage8u(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

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
	IppiSize dstRoiSize = { SrcImg.nWidth, SrcImg.nHeight };
	//Ipp16s pSum;
	//ippsSum_16s_Sfs(pMask, nMaskWidth*nMaskHeight, &pSum, 0);
	int nDivisor = 1;
	int nSpecSize = 0, nBufferSize = 0;

	//* Initializes the Buffer */
	status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, SrcImg.nChannel, &nSpecSize, &nBufferSize);
	if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderGetSize", hCall))
	{
		ippsFree(pKernel);
		return 0;
	}
	IppiFilterBorderSpec* pSepc = (IppiFilterBorderSpec*)ippsMalloc_8u(nSpecSize);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

	// * Initializes the filter specification structure */
	status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, SrcImg.nChannel, ippRndNear, pSepc);
	if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorderInit_16s", hCall))
	{
		ippsFree(pKernel);
		ippsFree(pSepc);
		ippsFree(pBuffer);
		return 0;
	}

	//Filters an image */
	IppiBorderType borderType = ippBorderRepl;
	if (SrcImg.nChannel == _Type_G8)
	{
		Ipp8u borderValue = 0;
		status = ippiFilterBorder_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
		if (check_sts(status, "KxGeneralFilterImage_ippiFilterBorder_8u_C1R", hCall))
		{
			ippsFree(pKernel);
			ippsFree(pSepc);
			ippsFree(pBuffer);
			return 0;
		}


	}
	if (SrcImg.nChannel == _Type_G24)
	{
		Ipp8u borderValue[3] = { 0, 0, 0 };
		status = ippiFilterBorder_8u_C3R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
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

int CKxBaseFunction::KxParallelGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxParallelGeneralFilterImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}

int CKxBaseFunction::KxParallelGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

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
	IppiSize RoiSize = { SrcImg.nWidth, SrcImg.nHeight };


	m_Image16s.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel * sizeof(Ipp16s));
	m_Image16sAbs.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel * sizeof(Ipp16s));


	int nRowActualGrain = gMin(nRowGrain, DstImg.nHeight);
	int nColActualGrain = gMin(nColGrain, DstImg.nWidth);

	Ipp8u* pSrc = m_Image16s.buf;
	Ipp8u* pAbs = m_Image16sAbs.buf;
	int nSrcPitch = m_Image16s.nPitch;

	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, nRowActualGrain, 0, SrcImg.nWidth, nColActualGrain),
		[&SrcImg, pSrc, &nSrcPitch, pAbs, &DstImg, &kernSize, pKernel ](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

 		Ipp8u* pOrigT = SrcImg.buf + range.rows().begin() * SrcImg.nPitch + range.cols().begin() * SrcImg.nChannel;
		Ipp8u* pSrcT = pSrc + range.rows().begin() * nSrcPitch + range.cols().begin() * DstImg.nChannel*sizeof(Ipp16s);
		Ipp8u* pAbsT = pAbs + range.rows().begin() * nSrcPitch + range.cols().begin() * DstImg.nChannel*sizeof(Ipp16s);
		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;

		//* Convert 8u image to 16s */
		if (_Type_G8 == DstImg.nChannel)
		{
			status = ippiConvert_8u16s_C1R(pOrigT, SrcImg.nPitch, (Ipp16s*)pSrcT, nSrcPitch, dstRoiSize);
		}
		else if (_Type_G24 == DstImg.nChannel)
		{
			status = ippiConvert_8u16s_C3R(pOrigT, SrcImg.nPitch, (Ipp16s*)pSrcT, nSrcPitch, dstRoiSize);
		}
		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp16s, ipp16s, DstImg.nChannel, &nSpecSize, &nBufferSize);

		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiFilterBorderSpec *pSepc = (IppiFilterBorderSpec*)scalable_aligned_malloc(nSpecSize*sizeof(Ipp8u), 32);

		// * Initializes the filter specification structure */
		int nDivisor = 1;
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp16s, DstImg.nChannel, ippRndNear, pSepc);

		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (DstImg.nChannel == _Type_G8)
		{
			Ipp16s borderValue = 0;
			status = ippiFilterBorder_16s_C1R((Ipp16s*)pSrcT, nSrcPitch, (Ipp16s*)pAbsT, nSrcPitch, dstRoiSize, borderType, &borderValue, pSepc, pBuffer);
			ippiAbs_16s_C1IR((Ipp16s*)pAbsT, nSrcPitch, dstRoiSize);
			ippiConvert_16s8u_C1R((Ipp16s*)pAbsT, nSrcPitch, pDstT, DstImg.nPitch, dstRoiSize);

		}
		if (DstImg.nChannel == _Type_G24)
		{
			Ipp16s borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_16s_C3R((Ipp16s*)pSrcT, nSrcPitch, (Ipp16s*)pAbsT, nSrcPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);
			ippiAbs_16s_C3IR((Ipp16s*)pAbsT, nSrcPitch, dstRoiSize);
			ippiConvert_16s8u_C3R((Ipp16s*)pAbsT, nSrcPitch, pDstT, DstImg.nPitch, dstRoiSize);
		}

		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pSepc);

	}, simple_partitioner());

	ippsFree(pKernel);

	return 1;

}

int CKxBaseFunction::KxParallelGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask)
{
	KxCallStatus hCall;
	return KxParallelGeneralFilterImage8u(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}


int CKxBaseFunction::KxParallelGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

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
	IppiSize RoiSize = { SrcImg.nWidth, SrcImg.nHeight };

	int nRowActualGrain = gMin(nRowGrain, DstImg.nHeight);
	int nColActualGrain = gMin(nColGrain, DstImg.nWidth);

	Ipp8u* pSrc = m_Image16s.buf;
	Ipp8u* pAbs = m_Image16sAbs.buf;
	int nSrcPitch = m_Image16s.nPitch;


	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, 0, SrcImg.nWidth),
		[&SrcImg, pSrc, &nSrcPitch, pAbs, &DstImg, &kernSize, pKernel](const blocked_range2d<int, int>& range)
	{
		IppStatus status;
		IppiSize dstRoiSize = { (int)range.cols().size(), (int)range.rows().size() };

		Ipp8u* pOrigT = SrcImg.buf + range.rows().begin() * SrcImg.nPitch + range.cols().begin() * SrcImg.nChannel;

		Ipp8u* pDstT = DstImg.buf + range.rows().begin() * DstImg.nPitch + range.cols().begin() * DstImg.nChannel;


		//* Initializes the Buffer */
		int nSpecSize = 0, nBufferSize = 0;
		status = ippiFilterBorderGetSize(kernSize, dstRoiSize, ipp8u, ipp16s, DstImg.nChannel, &nSpecSize, &nBufferSize);


		Ipp8u *pBuffer = (Ipp8u*)scalable_aligned_malloc(nBufferSize*sizeof(Ipp8u), 32);
		IppiFilterBorderSpec *pSepc = (IppiFilterBorderSpec*)scalable_aligned_malloc(nSpecSize*sizeof(Ipp8u), 32);

		// * Initializes the filter specification structure */
		int nDivisor = 1;
		status = ippiFilterBorderInit_16s(pKernel, kernSize, nDivisor, ipp8u, DstImg.nChannel, ippRndNear, pSepc);



		//Filters an image */
		IppiBorderType borderType = ippBorderRepl;
		if (DstImg.nChannel == _Type_G8)
		{
			Ipp8u borderValue[1] = { 0 };
			status = ippiFilterBorder_8u_C1R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);


		}
		if (DstImg.nChannel == _Type_G24)
		{
			Ipp8u borderValue[3] = { 0, 0, 0 };
			status = ippiFilterBorder_8u_C3R(pOrigT, SrcImg.nPitch, pDstT, DstImg.nPitch, dstRoiSize, borderType, borderValue, pSepc, pBuffer);

		}

		scalable_aligned_free((void*)pBuffer);
		scalable_aligned_free((void*)pSepc);

	}, auto_partitioner());



	ippsFree(pKernel);


	return 1;

}

int  CKxBaseFunction::KxHistogram(const kxCImageBuf& SrcImg, int nBins, Ipp32u nHist[])
{
	KxCallStatus hCall;
	return KxHistogram(SrcImg, nBins, nHist, hCall);
}


int  CKxBaseFunction::KxHistogram( const kxCImageBuf& SrcImg,  int nBins, Ipp32u nHist[], KxCallStatus& hCall )
{//直方图统计-------->
	IppiSize roiSize = {SrcImg.nWidth, SrcImg.nHeight};
	//* get sizes for spec and buffer */
	IppStatus status;
	//const int nBins = 256;
	int nLevels[] = {nBins + 1, nBins+1, nBins+1};
	Ipp32f lowerLevel[] = {0, 0, 0};
	Ipp32f upperLevel[] = {255, 255, 255};
	int nSizeHistObj, nSizeBuffer;
	status = ippiHistogramGetBufferSize(ipp8u, roiSize, nLevels, SrcImg.nChannel, 1, &nSizeHistObj, &nSizeBuffer);
	if (check_sts(status, "KxHistogram_ippiHistogramGetBufferSize", hCall))
	{
		return 0;
	}

	IppiHistogramSpec* pHistObj = (IppiHistogramSpec* )ippsMalloc_8u(nSizeHistObj);
	Ipp8u* pBuffer = ippsMalloc_8u(nSizeBuffer);
	//initialize spec */
	status = ippiHistogramUniformInit(ipp8u, lowerLevel, upperLevel, nLevels, SrcImg.nChannel, pHistObj);
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
	if (_Type_G8 == SrcImg.nChannel)
	{
		Ipp32u* pHistVec = ippsMalloc_32u(nBins);
		status = ippiHistogram_8u_C1R(SrcImg.buf, SrcImg.nPitch, roiSize, pHistVec, pHistObj, pBuffer);
		ippsCopy_8u((Ipp8u*)pHistVec, (Ipp8u*)nHist, sizeof(Ipp32u)*nBins);
		ippsFree(pHistVec);
	}
	if (_Type_G24 == SrcImg.nChannel)
	{
		Ipp32u* pHistVec[3];
		for (int i = 0; i < 3; i++)
		{
			pHistVec[i] = ippsMalloc_32u(nBins);
		}
		status = ippiHistogram_8u_C3R(SrcImg.buf, SrcImg.nPitch, roiSize, pHistVec, pHistObj, pBuffer);
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


int CKxBaseFunction::KxProjectImage(const kxCImageBuf& SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale)
{
	KxCallStatus hCall;
	return KxProjectImage(SrcImg, nDir, fProject, fScale, hCall);
}
//
int CKxBaseFunction::KxProjectImage(const kxCImageBuf& SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale, KxCallStatus& hCall)
{
	IppStatus status;
	if (SrcImg.nChannel != _Type_G8)
	{
		if (check_sts(-10000, "KxProjectImage_Image_TypeErr", hCall))
		{
			return 0;
		}
	}

	if (nDir == _Horizontal_Project_Dir)
	{
		IppiSize dstRoi = { 1, SrcImg.nHeight};
		int anchor = (SrcImg.nWidth - 1) / 2;
		status = ippiSumWindowRow_8u32f_C1R(SrcImg.buf + anchor, SrcImg.nPitch, fProject, sizeof(Ipp32f), dstRoi, SrcImg.nWidth, anchor);
		if (check_sts(status, "KxProjectImage_ippiSumWindowRow_8u32f_C1R", hCall))
		{
			return 0;
		}
		ippsDivC_32f_I(fScale, fProject, SrcImg.nHeight);
	}
	if (nDir == _Vertical_Project_Dir)
	{
		IppiSize dstRoi = {SrcImg.nWidth, 1 };
		int anchor = (SrcImg.nHeight - 1) / 2;
		status = ippiSumWindowColumn_8u32f_C1R(SrcImg.buf + anchor * SrcImg.nPitch, SrcImg.nPitch, fProject, SrcImg.nWidth*sizeof(Ipp32f), dstRoi, SrcImg.nHeight, anchor);
		if (check_sts(status, "KxProjectImage_ippiSumWindowColumn_8u32f_C1R", hCall))
		{
			return 0;
		}
		ippsDivC_32f_I(fScale, fProject, SrcImg.nWidth);
	}



	return 1;
}

int CKxBaseFunction::KxAddImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return KxAddImage(SrcImg, DstImg, hCall);
}

int CKxBaseFunction::KxAddImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppStatus status;
	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiAdd_8u_C1IRSfs(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, 0);
		if (check_sts(status, "KxAddImage_ippiAdd_8u_C1IRSfs", hCall))
		{
			return 0;
		}
	}
	if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiAdd_8u_C3IRSfs(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, 0);
		if (check_sts(status, "KxAddImage_ippiAdd_8u_C1IRSfs", hCall))
		{
			return 0;
		}
	}

	return 1;
}

int CKxBaseFunction::KxSubCImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Ipp8u value)
{
	KxCallStatus hCall;
	return KxSubCImage(SrcImg, DstImg, value, hCall);
}


int CKxBaseFunction::KxSubCImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Ipp8u value, KxCallStatus& hCall)
{
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);


	IppStatus status;
	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiSubC_8u_C1RSfs(SrcImg.buf, SrcImg.nPitch, value, DstImg.buf, DstImg.nPitch, roiSize, 0);
		if (check_sts(status, "KxSubCImage_ippiSubC_8u_C1RSfs", hCall))
		{
			return 0;
		}
	}
	if (SrcImg.nChannel == _Type_G24)
	{
		Ipp8u nValue[3];
		for (int i = 0; i < 3; i++)
	    {
			nValue[i] = value;
	    }
		status = ippiSubC_8u_C3RSfs(SrcImg.buf, SrcImg.nPitch, nValue, DstImg.buf, DstImg.nPitch, roiSize, 0);
		if (check_sts(status, "KxSubCImage_ippiSubC_8u_C3RSfs", hCall))
		{
			return 0;
		}
	}

	return 1;
}

int CKxBaseFunction::KxImageMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return KxImageMaxEvery(SrcImg, DstImg, hCall);

}
int CKxBaseFunction::KxImageMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	IppiSize size = { SrcImg.nWidth, SrcImg.nHeight };
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiGradientColorToGray_8u_C3C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, size, ippiNormInf);
		if (check_sts(status, "KxImageMaxEvery_ippiGradientColorToGray_8u_C3C1R", hCall))
		{
			return 0;
		}
	}
	if (SrcImg.nChannel == _Type_G8)
	{
		status = ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, size);
		if (check_sts(status, "KxImageMaxEvery_ippiCopy_8u_C1R", hCall))
		{
			return 0;
		}

	}
	return 1;

}

int CKxBaseFunction::KxZhuGuangCardFilter(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return KxZhuGuangCardFilter(SrcImg, DstImg, hCall);
}
int CKxBaseFunction::KxZhuGuangCardFilter(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	//
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	IppStatus status;
	if (SrcImg.nChannel != _Type_G8)
	{
		status = IppStatus(kxImageTypeErr);
		if (check_sts(status, "KxZhuGuangCardFilter_Image_TypeErr", hCall))
		{
			return 0;
		}

	}
	DstImg.Init(SrcImg.nWidth / 4, SrcImg.nHeight / 4);


	m_Img16s.Init(SrcImg.nWidth, SrcImg.nHeight, sizeof(Ipp16s));

	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	int nBufferSize;
	status = ippiFilterLaplacianGetBufferSize_8u16s_C1R(roiSize, ippMskSize5x5, &nBufferSize);
	if (check_sts(status, "KxZhuGuangCardFilter_ippiFilterLaplacian", hCall))
	{
		return 0;
	}
	Ipp8u* pBuffer = new Ipp8u[nBufferSize];
	status = ippiFilterLaplacianBorder_8u16s_C1R(SrcImg.buf, SrcImg.nPitch, (Ipp16s*)m_Img16s.buf, m_Img16s.nPitch, roiSize, ippMskSize5x5, ippBorderConst, 0, pBuffer);
	delete[]pBuffer;

	if (check_sts(status, "KxZhuGuangCardFilter_ippiFilterLaplacianBorder", hCall))
	{
		return 0;
	}

	ippiAbs_16s_C1IR((Ipp16s*)m_Img16s.buf, m_Img16s.nPitch, roiSize);
	ippiDivC_16s_C1IRSfs(3, (Ipp16s*)m_Img16s.buf, m_Img16s.nPitch, roiSize, 0);
	m_ImgLapFilter.Init(SrcImg.nWidth, SrcImg.nHeight);
	ippiConvert_16s8u_C1R((Ipp16s*)m_Img16s.buf, m_Img16s.nPitch, m_ImgLapFilter.buf, m_ImgLapFilter.nPitch, roiSize);



	KxDilateImage(m_ImgLapFilter, m_ImgDilate, 5, 5, NULL, hCallInfo);

	if (check_sts(hCallInfo, "KxZhuGuangCardFilter_", hCall))
	{
		return 0;
	}

	m_ImgResize.Init(SrcImg.nWidth / 4, SrcImg.nHeight / 4);
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


int CKxBaseFunction::KxFFtCheckPeriodic(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return KxFFtCheckPeriodic(SrcImg, DstImg, hCall);
}


int CKxBaseFunction::KxFFtCheckPeriodic(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall )
{
	IppStatus status;
	m_ImgCopy32f.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize Roi = { SrcImg.nWidth, SrcImg.nHeight };
	status = ippiConvert_8u32f_C1R(SrcImg.buf, SrcImg.nPitch, m_ImgCopy32f.buf, m_ImgCopy32f.nPitch, Roi);
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

	m_ImgFFt.Init(SrcImg.nWidth, SrcImg.nHeight);
	/* forward FFT transform : the source image is transformed into the frequency domain*/
	status = ippiDFTFwd_RToPack_32f_C1R(m_ImgCopy32f.buf, m_ImgCopy32f.nPitch, m_ImgFFt.buf, m_ImgFFt.nPitch, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTFwd_RToPack_32f_C1R", hCall))
	{
		return 0;
	}

	m_ImgFilter32fc.Init(SrcImg.nWidth, SrcImg.nHeight);
	Ipp32fc value = { 1.0, 0 };
	ippsSet_32fc(value, m_ImgFilter32fc.buf, m_ImgFilter32fc.nWidth * m_ImgFilter32fc.nHeight);
	//
	const int nPassW = 30;
	const int nPassH = 18;

	for (int i = nPassH; i < SrcImg.nHeight - nPassH; i++)
	{
		for (int j = 0; j < nPassW; j++)
		{
			m_ImgFilter32fc.buf[i * m_ImgFilter32fc.nWidth + j] = { 0, 0 };
		}
		for (int j = SrcImg.nWidth - nPassW; j < SrcImg.nWidth; j++)
		{
			m_ImgFilter32fc.buf[i * m_ImgFilter32fc.nWidth + j] = { 0, 0 };
		}
	}


	m_ImgFFtFilter.Init(SrcImg.nWidth, SrcImg.nHeight);
	status = ippiCplxExtendToPack_32fc32f_C1R(m_ImgFilter32fc.buf, m_ImgFilter32fc.nPitch, Roi, m_ImgFFtFilter.buf, m_ImgFFtFilter.nPitch);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiCplxExtendToPack_32fc32f_C1R", hCall))
	{
		return 0;
	}


	/* multiplying the packed data */
	status = ippiMulPack_32f_C1IR(m_ImgFFt.buf, m_ImgFFt.nPitch, m_ImgFFtFilter.buf, m_ImgFFtFilter.nPitch, Roi);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiMulPack_32f_C1IR", hCall))
	{
		return 0;
	}


	/* inverse FFT transform : the filtered data is transformed to the time domain */
	m_ImgFFtResult.Init(SrcImg.nWidth, SrcImg.nHeight);
	status = ippiDFTInv_PackToR_32f_C1R(m_ImgFFtFilter.buf, m_ImgFFtFilter.nPitch, m_ImgFFtResult.buf, m_ImgFFtResult.nPitch, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTInv_PackToR_32f_C1R", hCall))
	{
		return 0;
	}

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	status = ippiConvert_32f8u_C1R(m_ImgFFtResult.buf, m_ImgFFtResult.nPitch, DstImg.buf, DstImg.nPitch, Roi, ippRndZero);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiConvert_32f8u_C1R", hCall))
	{
		return 0;
	}


	ippsFree(pSepc);
	ippsFree(pBuffer);
	ippsFree(pMemInit);


	return 1;
}


int CKxBaseFunction::KxFFtCheckPeriodic1(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return KxFFtCheckPeriodic1(SrcImg, DstImg, hCall);
}



int CKxBaseFunction::KxFFtCheckPeriodic1(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	bool bFlag[2];
	bFlag[0] = (((SrcImg.nWidth)&(SrcImg.nWidth - 1)) == 0);
	bFlag[1] = (((SrcImg.nHeight)&(SrcImg.nHeight - 1)) == 0);
	if (!bFlag[0] || !bFlag[1])
	{
		IppStatus status = IppStatus(kxImageTypeErr);
		if (check_sts(status, "KxFFtCheckPeriodic1_ImageSizeMustbePowerOf2", hCall))
		{
			return 0;
		}
	}

	int nOrderX = int(log(double(SrcImg.nWidth))/log(2.0));
	int nOrderY = int(log(double(SrcImg.nHeight))/log(2.0));

	IppStatus status;
	m_ImgCopy32f.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize Roi = { SrcImg.nWidth, SrcImg.nHeight };
	status = ippiConvert_8u32f_C1R(SrcImg.buf, SrcImg.nPitch, m_ImgCopy32f.buf, m_ImgCopy32f.nPitch, Roi);
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

	m_ImgFFt.Init(SrcImg.nWidth, SrcImg.nHeight);
	/* forward FFT transform : the source image is transformed into the frequency domain*/
	status = ippiFFTFwd_RToPack_32f_C1R(m_ImgCopy32f.buf, m_ImgCopy32f.nPitch, m_ImgFFt.buf, m_ImgFFt.nPitch, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic1_ippiFFTFwd_RToPack_32f_C1R", hCall))
	{
		return 0;
	}

	m_ImgFilter32fc.Init(SrcImg.nWidth, SrcImg.nHeight);
	Ipp32fc value = { 1.0, 0 };
	ippsSet_32fc(value, m_ImgFilter32fc.buf, m_ImgFilter32fc.nWidth * m_ImgFilter32fc.nHeight);
	//
	const int nPassW = 15;
	const int nPassH = 50;

	for (int i = nPassH; i < SrcImg.nHeight - nPassH; i++)
	{
		for (int j = 0; j < nPassW; j++)
		{
			m_ImgFilter32fc.buf[i * m_ImgFilter32fc.nWidth + j] = { 0, 0 };
		}
		for (int j = SrcImg.nWidth - nPassW; j < SrcImg.nWidth; j++)
		{
			m_ImgFilter32fc.buf[i * m_ImgFilter32fc.nWidth + j] = { 0, 0 };
		}
	}


	m_ImgFFtFilter.Init(SrcImg.nWidth, SrcImg.nHeight);
	status = ippiCplxExtendToPack_32fc32f_C1R(m_ImgFilter32fc.buf, m_ImgFilter32fc.nPitch, Roi, m_ImgFFtFilter.buf, m_ImgFFtFilter.nPitch);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiCplxExtendToPack_32fc32f_C1R", hCall))
	{
		return 0;
	}


	/* multiplying the packed data */
	status = ippiMulPack_32f_C1IR(m_ImgFFt.buf, m_ImgFFt.nPitch, m_ImgFFtFilter.buf, m_ImgFFtFilter.nPitch, Roi);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiMulPack_32f_C1IR", hCall))
	{
		return 0;
	}


	/* inverse FFT transform : the filtered data is transformed to the time domain */
	m_ImgFFtResult.Init(SrcImg.nWidth, SrcImg.nHeight);
	status = ippiFFTInv_PackToR_32f_C1R(m_ImgFFtFilter.buf, m_ImgFFtFilter.nPitch, m_ImgFFtResult.buf, m_ImgFFtResult.nPitch, pSepc, pBuffer);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiDFTInv_PackToR_32f_C1R", hCall))
	{
		return 0;
	}

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	status = ippiConvert_32f8u_C1R(m_ImgFFtResult.buf, m_ImgFFtResult.nPitch, DstImg.buf, DstImg.nPitch, Roi, ippRndZero);
	if (check_sts(status, "KxFFtCheckPeriodic_ippiConvert_32f8u_C1R", hCall))
	{
		return 0;
	}


	ippsFree(pSepc);
	ippsFree(pBuffer);
	ippsFree(pMemInit);


	return 1;
}

int CKxBaseFunction::KxFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize)
{
	KxCallStatus hCall;
	return KxFilterSpeckles(SrcImg, DstImg, nMaxSpeckleSize, hCall);
}


int CKxBaseFunction::KxFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, KxCallStatus& hCall)
{
	if (SrcImg.nChannel != _Type_G8)
	{
		return 0;
	}
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppStatus status;
	int nBufferSize = 0;
	status = ippiMarkSpecklesGetBufferSize(roiSize, ipp8u, SrcImg.nChannel, &nBufferSize);
	if (check_sts(status, "KxFilterSpeckles_ippiMarkSpecklesGetBufferSize", hCall))
	{
		return 0;
	}
	m_MarkImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	ippiThreshold_GTVal_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_MarkImg.buf, m_MarkImg.nPitch, roiSize, 0, 20);
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
	//tick_count tbb_start, tbb_end;
	//tbb_start = tick_count::now();
	status = ippiMarkSpeckles_8u_C1IR(m_MarkImg.buf, m_MarkImg.nPitch, roiSize, 255, nMaxSpeckleSize, 10, ippiNormL1, pBuffer);
	//tbb_end = tick_count::now();
	//printf("KxFilterSpeckles filter cost: %f ms\n", (tbb_end - tbb_start).seconds());
	if (check_sts(status, "KxFilterSpeckles_ippiMarkSpeckles_8u_C1IR", hCall))
	{
		return 0;
	}

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	KxThreshImage(m_MarkImg, DstImg, 1, 254);
	ippiAnd_8u_C1IR(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize);

	ippsFree(pBuffer);

	return 1;
}


int CKxBaseFunction::KxFillHoles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return KxFillHoles(SrcImg, DstImg, hCall);
}



int CKxBaseFunction::KxFillHoles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	IppStatus status;
	if (SrcImg.nChannel != _Type_G8)
	{
		status = IppStatus(kxImageTypeErr);
		if (check_sts(status, "KxFillHoles_Image_TypeErr", hCall))
		{
			return 0;
		}

	}
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	kxCImageBuf m_InvertImg, m_MarkerImg;
	m_InvertImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	ippsSet_8u(0xff, m_InvertImg.buf, m_InvertImg.nPitch * m_InvertImg.nHeight);
	//图像取反
	ippiSub_8u_C1IRSfs(SrcImg.buf, SrcImg.nPitch, m_InvertImg.buf, m_InvertImg.nPitch, roiSize, 0);
	m_MarkerImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	ippsSet_8u(0xff, m_MarkerImg.buf, m_MarkerImg.nPitch * m_MarkerImg.nHeight);
	IppiSize Roi = { roiSize.width - 2, roiSize.height - 2 };
	ippiSet_8u_C1R(0, m_MarkerImg.buf + 1 + m_MarkerImg.nPitch, m_MarkerImg.nPitch, Roi);
	int size = 0;
	status = ippiMorphReconstructGetBufferSize(roiSize, ipp8u, 1, &size);
	if (check_sts(status, "KxFillHoles_ippiMorphReconstructGetBufferSize", hCall))
	{
		return 0;
	}

	Ipp8u* pBuf = ippsMalloc_8u(size);
	status = ippiMorphReconstructDilate_8u_C1IR(m_InvertImg.buf, m_InvertImg.nPitch, m_MarkerImg.buf, m_MarkerImg.nPitch, roiSize, pBuf, (IppiNorm)ippiNormL1);
	if (check_sts(status, "KxFillHoles_ippiMorphReconstructDilate_8u_C1IR", hCall))
	{
		return 0;
	}

	ippsFree(pBuf);
	ippsSet_8u(0xff, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	//图像取反
	ippiSub_8u_C1IRSfs(m_MarkerImg.buf, m_MarkerImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, 0);

	return 1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int CKxBaseFunction::KxParallelDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxParallelDilateImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}



//对于比较小的图并行速度更慢
int CKxBaseFunction::KxParallelDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	int nRowActualGrain = gMin(nRowGrain, DstImg.nHeight);
	int nColActualGrain = gMin(nColGrain, DstImg.nWidth);
	IppiSize maskSize = { nMaskWidth, nMaskHeight };
	//static_partitioner app;
	Ipp8u* pSrc = SrcImg.buf;
	Ipp8u* pDst = DstImg.buf;
	int nChannel = SrcImg.nChannel;
	int nSrcPitch = SrcImg.nPitch;

	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, 0, SrcImg.nWidth),
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



int CKxBaseFunction::KxParallelErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask)
{
	KxCallStatus hCall;
	return KxParallelErodeImage(SrcImg, DstImg, nMaskWidth, nMaskHeight, pMask, hCall);
}



int CKxBaseFunction::KxParallelErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	//IppStatus status;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	bool bMaskNotInit = false;
	if (NULL == pMask)
	{
		pMask = ippsMalloc_8u(nMaskWidth*nMaskHeight);
		ippsSet_8u(1, pMask, nMaskWidth*nMaskHeight);
		bMaskNotInit = true;
	}

	int nRowActualGrain = gMin(nRowGrain, DstImg.nHeight);
	int nColActualGrain = gMin(nColGrain, DstImg.nWidth);
	IppiSize maskSize = { nMaskWidth, nMaskHeight };
	//static_partitioner app;
	Ipp8u* pSrc = SrcImg.buf;
	Ipp8u* pDst = DstImg.buf;
	int nChannel = SrcImg.nChannel;
	int nSrcPitch = SrcImg.nPitch;

	parallel_for(blocked_range2d<int, int>(0, SrcImg.nHeight, 0, SrcImg.nWidth),
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

int CKxBaseFunction::KxParallelResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode)
{
	KxCallStatus hCall;
	return KxParallelResizeImage(SrcImg, ResizeImg, nInterpolationMode, hCall);
}

int CKxBaseFunction::KxParallelResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode, KxCallStatus& hCall, int nRowGrain, int nColGrain)
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

int CKxBaseFunction::KxThresholdAdaptiveGauss(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiSize mskSize, int nDelta, int nThreshLayer)
{
	KxCallStatus hCall;
	return KxThresholdAdaptiveGauss(SrcImg, DstImg, mskSize, nDelta, nThreshLayer, hCall);
}

int CKxBaseFunction::KxThresholdAdaptiveGauss(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiSize mskSize, int nDelta, int nThreshLayer, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	if (SrcImg.nChannel == _Type_G24)
	{
		KxConvertImageLayer(SrcImg, m_ImgGaussGray, nThreshLayer, hCallInfo);
		if (check_sts(hCallInfo, "KxThresholdAdaptiveGauss_", hCall))
		{
			return 0;
		}
	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		m_ImgGaussGray.Init(SrcImg.nWidth, SrcImg.nHeight);
		KxCopyImage(SrcImg, m_ImgGaussGray, rc);
	}
	IppStatus status;
	int nBufferSize, nSpecSize;
	IppiSize roi = { m_ImgGaussGray.nWidth, m_ImgGaussGray.nHeight };
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

	status = ippiThresholdAdaptiveGauss_8u_C1R(m_ImgGaussGray.buf, m_ImgGaussGray.nPitch, DstImg.buf, DstImg.nPitch, roi, (Ipp32f)nDelta, 255, 0, ippBorderConst, 0, pSpec, pBuffer);
	if (check_sts(status, "KxThresholdAdaptiveGauss_ippiThresholdAdaptiveGauss_8u_C1R", hCall))
	{
		return 0;
	}

	ippsFree(pBuffer);
	ippsFree(pSpec);

	return 1;

}
int CKxBaseFunction::KxInvertImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	return KxInvertImage(SrcImg, DstImg, hCall);
}


int CKxBaseFunction::KxInvertImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	ippsSet_8u(0xff, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	IppStatus status;
	if (SrcImg.nChannel == _Type_G24)
	{
		status = ippiSub_8u_C3IRSfs(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, 0);
	}
	else
	{
		status = ippiSub_8u_C1IRSfs(SrcImg.buf, SrcImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, 0);
	}

	if (check_sts(status, "KxInvertImage_ippiSub_8u", hCall))
	{
		return 0;
	}

	return 1;
}



#ifdef OPENCV
int CKxBaseFunction::KxImageBufToMat(const kxCImageBuf& SrcImg, cv::Mat& mat, bool bCopy)
{
	int nType = (SrcImg.nChannel == _Type_G8 ? CV_8UC1 : CV_8UC3);
	if (bCopy)
	{	
		mat.create(SrcImg.nHeight, SrcImg.nWidth, nType);
		IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		if (SrcImg.nChannel == _Type_G8)
		{
			ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, mat.data, (int)mat.step, roiSize);
		}
		else
		{
			ippiCopy_8u_C3R(SrcImg.buf, SrcImg.nPitch, mat.data, (int)mat.step, roiSize);
		}
	}
	else
	{
		mat = cv::Mat(SrcImg.nHeight, SrcImg.nWidth, nType, SrcImg.buf, SrcImg.nPitch);
	}
	return 1;
}

int CKxBaseFunction::MatToKxImageBuf(const cv::Mat& mat, kxCImageBuf& SrcImg, bool bCopy)
{
	int nChannel;
	if (mat.type() == CV_8UC1)
	{
		nChannel = 1;
	}
	else if (mat.type() == CV_8UC4)
	{
		nChannel = 4;
	}
	else
	{
		nChannel = 3;
	}
	if (bCopy)
	{
		SrcImg.Init(mat.cols, mat.rows, nChannel);
		IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		if (SrcImg.nChannel == _Type_G8)
		{
			ippiCopy_8u_C1R(mat.data, (int)mat.step, SrcImg.buf, SrcImg.nPitch, roiSize);
		}
		else if (SrcImg.nChannel == 4)
		{
			ippiCopy_8u_C4R(mat.data, (int)mat.step, SrcImg.buf, SrcImg.nPitch, roiSize);
		}
		else
		{
			ippiCopy_8u_C3R(mat.data, (int)mat.step, SrcImg.buf, SrcImg.nPitch, roiSize);
		}

	}
	else
	{
		SrcImg.SetImageBuf(mat.data, mat.cols, mat.rows, (int)mat.step, nChannel, false);
	}

	return 1;
}



#endif