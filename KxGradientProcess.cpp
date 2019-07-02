
#include "KxGradientProcess.h"

CKxGradientProcess::CKxGradientProcess()
{

}

CKxGradientProcess::~CKxGradientProcess()
{

}

int CKxGradientProcess::ComputeGrand(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nSmoothSize)
{
	KxCallStatus hCall;
	return ComputeGrand(SrcImg, DstImg, nSmoothSize, hCall);
}

int CKxGradientProcess::ComputeGrand(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nSmoothSize, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	m_SoomthImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);

	IppiSize roiSize= {SrcImg.nWidth, SrcImg.nHeight};
     
	if (_5X5 == nSmoothSize)
	{		
		m_hFun.KxAverageFilterImage(SrcImg, m_SoomthImg, 5, 5, NULL, hTempCall);
		if (check_sts(hTempCall,"ComputeGrand_KxAverageFilterImage", hCall))
		{
			return 0;
		}
	}
	else if (_3X3 == nSmoothSize)
	{
		m_hFun.KxAverageFilterImage(SrcImg, m_SoomthImg, 3, 3, NULL, hTempCall);
		if (check_sts(hTempCall,"ComputeGrand_KxAverageFilterImage", hCall))
		{
			return 0;
		}
	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth-1, SrcImg.nHeight-1);
		m_hFun.KxCopyImage(SrcImg, m_SoomthImg, rc, hTempCall);
		if (check_sts(hTempCall,"ComputeGrand_KxCopyImage", hCall))
		{
			return 0;
		}
	}
	m_hFun.KxFilterSobelImage(m_SoomthImg, DstImg, _3X3, hTempCall);
	if (check_sts(hTempCall,"ComputeGrand_KxFilterSobelImage", hCall))
	{
		return 0;
	}

	return 1;
}

int CKxGradientProcess::GradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return GradientImageCompress(SrcImg, DstImg, nCompressSize, bordervalue, hCall);
}

int CKxGradientProcess::GradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	DstImg.Init(SrcImg.nWidth/nCompressSize, SrcImg.nHeight/nCompressSize, SrcImg.nChannel);
	memset(DstImg.buf, 0, DstImg.nHeight*DstImg.nPitch);
	m_GradientMaxImg.Init(DstImg.nWidth, DstImg.nHeight, DstImg.nChannel);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bool bModel = ((j == 1)?true:false);
			SingleDirGradient(SrcImg, m_GradientMaxImg, i, bModel, nCompressSize, bordervalue, 5, hCallInfo);
			if (check_sts(hCallInfo, "GradientImageCompress_SingleDirGradient", hCall))
			{
				return 0;
			}
		}	
		m_hFun.KxMaxEvery(m_GradientMaxImg, DstImg);
	}
	return 1;

}

int CKxGradientProcess::ParallelGradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return ParallelGradientImageCompress(SrcImg, DstImg, nCompressSize, bordervalue, hCall);
}


int CKxGradientProcess::ParallelGradientImageCompress(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nCompressSize, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	DstImg.Init(SrcImg.nWidth / nCompressSize, SrcImg.nHeight / nCompressSize, SrcImg.nChannel);
	memset(DstImg.buf, 0, DstImg.nHeight*DstImg.nPitch);
	m_GradientMaxImg.Init(DstImg.nWidth, DstImg.nHeight, DstImg.nChannel);
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bool bModel = ((j == 1) ? true : false);
			ParallelSingleDirGradient(SrcImg, m_GradientMaxImg, i, bModel, nCompressSize, bordervalue, 5, hCallInfo);
			if (check_sts(hCallInfo, "GradientImageCompress_SingleDirGradient", hCall))
			{
				return 0;
			}
		}
		m_hFun.KxMaxEvery(m_GradientMaxImg, DstImg);
	}
	return 1;

}

int CKxGradientProcess::ParallelSingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize)
{
	KxCallStatus hCall;
	return ParallelSingleDirGradient(SrcImg, DstImg, nDirection, bWhiteBlackMode, nCompressSize, bordervalue, nLargeSize, hCall);
}


int CKxGradientProcess::ParallelSingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{

	return SingleDirGradient(SrcImg, DstImg, nDirection, bWhiteBlackMode, nCompressSize, bordervalue, nLargeSize, hCall);
}

int CKxGradientProcess::SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize)
{
	KxCallStatus hCall;
	return SingleDirGradient(SrcImg, DstImg, nDirection, bWhiteBlackMode, nCompressSize, bordervalue, nLargeSize, hCall);
}



int CKxGradientProcess::SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, bool bWhiteBlackMode, int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall)
{	
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	IppStatus status = ippStsNoErr;
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	m_TmpImg.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	//5*5膨胀
	if (nLargeSize == 5)
	{
		Ipp8u pMask1[4][5][5] =
		{
			1, 1, 1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,

			1, 1, 1, 1, 1,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,

			1, 0, 0, 0, 0,
			0, 1, 0, 0, 0,
			0, 0, 1, 0, 0,
			0, 0, 0, 1, 0,
			0, 0, 0, 0, 1,

			0, 0, 0, 0, 1,
			0, 0, 0, 1, 0,
			0, 0, 1, 0, 0,
			0, 1, 0, 0, 0,
			1, 0, 0, 0, 0
		};
		IppiSize maskSize1[4] = { 5, 5, 5, 5, 5, 5, 5, 5 };

		if (bWhiteBlackMode)
		{
			m_hFun.KxDilateImage(SrcImg, m_TmpImg, maskSize1[nDirection].width, maskSize1[nDirection].height, (Ipp8u*)pMask1[nDirection], hCallInfo);
		}
		else
		{
			m_hFun.KxErodeImage(SrcImg, m_TmpImg, maskSize1[nDirection].width, maskSize1[nDirection].height, (Ipp8u*)pMask1[nDirection], hCallInfo);
		}
	}
	else if (nLargeSize == 3)
	{
		Ipp8u pMask1[4][3][3] =
		{
			1, 1, 1,
			0, 0, 0,
			0, 0, 0,

			1, 1, 1,
			0, 0, 0,
			0, 0, 0,

			1, 0, 0,
			0, 1, 0,
			0, 0, 1,

			0, 0, 0,
			0, 0, 0,
			0, 0, 1,
		};
		IppiSize maskSize1[4] = { 3, 3, 3, 3, 3, 3, 3, 3 };

		if (bWhiteBlackMode)
		{
			m_hFun.KxDilateImage(SrcImg, m_TmpImg, maskSize1[nDirection].width, maskSize1[nDirection].height, (Ipp8u*)pMask1[nDirection], hCallInfo);
		}
		else
		{
			m_hFun.KxErodeImage(SrcImg, m_TmpImg, maskSize1[nDirection].width, maskSize1[nDirection].height, (Ipp8u*)pMask1[nDirection], hCallInfo);
		}
	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);

		m_hFun.KxCopyImage(SrcImg, m_TmpImg, rc, hCallInfo);

	}



	if (check_sts(hCallInfo, "SingleDirGradient_KxDilate/KxErodeImage", hCall))
	{
		return 0;
	}
	//4*4压缩
	roiSize.width /= nCompressSize;
	roiSize.height /= nCompressSize;
	m_TmpImg1.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	m_hFun.KxResizeImage(m_TmpImg, m_TmpImg1, KxSuper, hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradient_KxResizeImage", hCall))
	{
		return 0;
	}

	//5*5平滑滤波
	Ipp16s pKernel3[4][5][5] =
	{
		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,

		1, 1, 1, 1, 1,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,

		0, 0, 0, 0, 1,
		0, 0, 0, 1, 0,
		0, 0, 1, 0, 0,
		0, 1, 0, 0, 0,
		1, 0, 0, 0, 0,

		1, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 1, 0,
		0, 0, 0, 0, 1
	};
	IppiSize kernelSize3[4] = { 5, 5, 5, 5, 5, 5, 5, 5 };
	m_TmpImg2.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	m_hFun.KxAverageFilterImage(m_TmpImg1, m_TmpImg2, kernelSize3[nDirection].width, kernelSize3[nDirection].height, (Ipp16s*)pKernel3[nDirection], hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradient_KxAverageFilterImage", hCall))
	{
		return 0;
	}

	//3*3梯度
	Ipp16s pKernel4[4][3][3] =
	{
		2, 0, -2,
		2, 0, -2,
		2, 0, -2,

		2, 2, 2,
		0, 0, 0,
		-2, -2, -2,

		2, 2, 0,
		2, 0, -2,
		0, -2, -2,

		0, 2, 2,
		-2, 0, 2,
		-2, -2, 0,
	};
	IppiSize kernelSize4 = { 3, 3 };
	m_TmpImg3.Init(m_TmpImg2.nWidth, m_TmpImg2.nHeight, m_TmpImg2.nChannel);
	m_hFun.KxGeneralFilterImage(m_TmpImg2, m_TmpImg3, kernelSize4.width, kernelSize4.height, (Ipp16s*)pKernel4[nDirection], 1, hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradient_KxGeneralFilterImage", hCall))
	{
		return 0;
	}

	DstImg.Init(m_TmpImg3.nWidth, m_TmpImg3.nHeight, m_TmpImg3.nChannel);
	ippsSet_8u(bordervalue, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize Roi = { DstImg.nWidth - 2, DstImg.nHeight - 2 };
	if (DstImg.nChannel == _Type_G24)
	{
		ippiCopy_8u_C3R(m_TmpImg3.buf + 3 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 3 + DstImg.nPitch, DstImg.nPitch, Roi);
	}
	else
	{
		ippiCopy_8u_C1R(m_TmpImg3.buf + 1 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 1 + DstImg.nPitch, DstImg.nPitch, Roi);

	}

	return 1;
}

int CKxGradientProcess::SingleDirGradient(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection,  int nCompressSize, Ipp8u bordervalue, int nLargeSize, KxCallStatus& hCall)
{
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	memset(DstImg.buf, 0, DstImg.nPitch * DstImg.nHeight);
	kxCImageBuf imgMax;
	for (int i = 0; i < 2; i++)
	{
		SingleDirGradient(SrcImg, imgMax, nDirection, (i == 0 ? false:true), nCompressSize, bordervalue, nLargeSize, hCall);
		m_hFun.KxMaxEvery(imgMax, DstImg);
	}
	return 1;
}



int CKxGradientProcess::GradientImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType, int nSoomthSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return GradientImage(SrcImg, DstImg, nFilterType, nSoomthSize, bordervalue, hCall);
}


int CKxGradientProcess::GradientImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	memset(DstImg.buf, 0, DstImg.nHeight*DstImg.nPitch);
	m_GradientMaxImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	for (int i = 0; i < 2; i++)
	{
		SingleDirGradientSame(SrcImg, m_GradientMaxImg, i, nFilterType, nSoomthSize, bordervalue, hCallInfo);
		if (check_sts(hCallInfo, "GradientImage_SingleDirGradientSame", hCall))
		{
			return 0;
		}
		m_hFun.KxMaxEvery(m_GradientMaxImg, DstImg, hCallInfo);
		if (check_sts(hCallInfo, "GradientImage_KxMaxEvery", hCall))
		{
			return 0;
		}
	}
	return 1;
}

int CKxGradientProcess::GradientImageWithFour(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nFilterType, int nSoomthSize, Ipp8u bordervalue)
{
	KxCallStatus hCallInfo;
	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	memset(DstImg.buf, 0, DstImg.nHeight*DstImg.nPitch);
	m_GradientMaxImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	for (int i = 0; i < 4; i++)
	{
		SingleDirGradientSameWithBigSmooth(SrcImg, m_GradientMaxImg, i, nFilterType, nSoomthSize, bordervalue, hCallInfo);
		m_hFun.KxMaxEvery(m_GradientMaxImg, DstImg, hCallInfo);

	}
	return 1;
}

int CKxGradientProcess::SingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return SingleDirGradientSame(SrcImg, DstImg, nDirection, nFilterType, nSoomthSize, bordervalue, hCall);
}

int CKxGradientProcess::SingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	m_TmpImg2.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	if (nSoomthSize > 2)
	{
		Ipp16s* pKernel = new Ipp16s[nSoomthSize * 1];
		for (int i = 0; i < nSoomthSize; i++)
		{
			pKernel[i] = 1;
		}
		IppiSize kernelSize;
		if (nDirection == 0)
		{
			kernelSize.width = 1;
			kernelSize.height = nSoomthSize;
		}
		else
		{
			kernelSize.width = nSoomthSize;
			kernelSize.height = 1;
		}

		if (nFilterType == _AverageFilter)
		{
			m_hFun.KxAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel,  hCallInfo);
			if (check_sts(hCallInfo, "SingleDirGradientSame_KxAverageFilterImage", hCall))
			{
				return 0;
			}
		}
		if (nFilterType == _MedianFilter)
		{
			m_hFun.KxMedianFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, hCallInfo);
			if (check_sts(hCallInfo, "SingleDirGradientSame_KxMedianFilterImage", hCall))
			{
				return 0;
			}
		}
		delete[] pKernel;
	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		m_hFun.KxCopyImage(SrcImg, m_TmpImg2, rc, hCallInfo);
		if (check_sts(hCallInfo, "SingleDirGradientSame_KxCopyImage", hCall))
		{
			return 0;
		}

	}

	//3*3梯度
	Ipp16s pKernel4[2][3][3] =
	{
		2, 0, -2,
		2, 0, -2,
		2, 0, -2,

		2, 2, 2,
		0, 0, 0,
		-2, -2, -2,
	};
	IppiSize kernelSize4 = { 3, 3 };

	m_TmpImg3.Init(m_TmpImg2.nWidth, m_TmpImg2.nHeight, m_TmpImg2.nChannel);
	m_hFun.KxGeneralFilterImage(m_TmpImg2, m_TmpImg3, kernelSize4.width, kernelSize4.height, (Ipp16s*)pKernel4[nDirection], 1, hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradientSame_KxGeneralFilterImage", hCall))
	{
		return 0;
	}


	DstImg.Init(m_TmpImg3.nWidth, m_TmpImg3.nHeight, m_TmpImg3.nChannel);
	ippsSet_8u(bordervalue, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize Roi = { DstImg.nWidth - 2, DstImg.nHeight - 2 };
	if (DstImg.nChannel == _Type_G24)
	{
		ippiCopy_8u_C3R(m_TmpImg3.buf + 3 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 3 + DstImg.nPitch, DstImg.nPitch, Roi);
	}
	else
	{
		ippiCopy_8u_C1R(m_TmpImg3.buf + 1 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 1 + DstImg.nPitch, DstImg.nPitch, Roi);

	}

	
	return 1;


}

int CKxGradientProcess::ParallelSingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return ParallelSingleDirGradientSame(SrcImg, DstImg, nDirection, nFilterType, nSoomthSize, bordervalue, hCall);
}


//现在只支持水平、垂直方向
int CKxGradientProcess::ParallelSingleDirGradientSame(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	
	m_TmpImg2.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	if (nSoomthSize > 2)
	{
		Ipp16s* pKernel = new Ipp16s[nSoomthSize * 1];
		for (int i = 0; i < nSoomthSize; i++)
		{
			pKernel[i] = 1;
		}
		IppiSize kernelSize;
		if (nDirection == 0) //垂直
		{
			kernelSize.width = 1;
			kernelSize.height = nSoomthSize;
		}
		else                 //水平
		{
			kernelSize.width = nSoomthSize;
			kernelSize.height = 1;
		}

		if (nFilterType == _AverageFilter)
		{
			//m_hFun.KxParallelAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo, nRowGrain, nColGrain);
			m_hFun.KxAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
			if (check_sts(hCallInfo, "SingleDirGradientSame_KxAverageFilterImage", hCall))
			{
				return 0;
			}
		}
		if (nFilterType == _MedianFilter)
		{
			m_hFun.KxParallelMedianFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, hCallInfo, nRowGrain, nColGrain);
			if (check_sts(hCallInfo, "SingleDirGradientSame_KxMedianFilterImage", hCall))
			{
				return 0;
			}
		}
		delete[] pKernel;
	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		m_hFun.KxCopyImage(SrcImg, m_TmpImg2, rc, hCallInfo);
		if (check_sts(hCallInfo, "SingleDirGradientSame_KxCopyImage", hCall))
		{
			return 0;
		}

	}

	//3*3梯度
	Ipp16s pKernel4[2][3][3] =
	{
		2, 0, -2,
		2, 0, -2,
		2, 0, -2,

		2, 2, 2,
		0, 0, 0,
		-2, -2, -2,
    };
	IppiSize kernelSize4 = { 3, 3 };

	m_TmpImg3.Init(m_TmpImg2.nWidth, m_TmpImg2.nHeight, m_TmpImg2.nChannel);
	m_hFun.KxParallelGeneralFilterImage(m_TmpImg2, m_TmpImg3, kernelSize4.width, kernelSize4.height, (Ipp16s*)pKernel4[nDirection], hCallInfo, nRowGrain, nColGrain);
	if (check_sts(hCallInfo, "SingleDirGradientSame_KxGeneralFilterImage", hCall))
	{
		return 0;
	}

	DstImg.Init(m_TmpImg3.nWidth, m_TmpImg3.nHeight, m_TmpImg3.nChannel);
	ippsSet_8u(bordervalue, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize Roi = { DstImg.nWidth - 2, DstImg.nHeight - 2 };
	if (DstImg.nChannel == _Type_G24)
	{
		ippiCopy_8u_C3R(m_TmpImg3.buf + 3 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 3 + DstImg.nPitch, DstImg.nPitch, Roi);
	}
	else
	{
		ippiCopy_8u_C1R(m_TmpImg3.buf + 1 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 1 + DstImg.nPitch, DstImg.nPitch, Roi);

	}


	return 1;

}


int CKxGradientProcess::SingleDirGradientLV(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return SingleDirGradientLV(SrcImg, DstImg, nDirection, bordervalue, hCall);
}

int CKxGradientProcess::SingleDirGradientLV(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	IppStatus status = ippStsNoErr;
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };

	//4*4压缩
	roiSize.width /= 4;
	roiSize.height /= 4;
	m_TmpImg1.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	m_hFun.KxResizeImage(SrcImg, m_TmpImg1, KxSuper, hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradientLV_KxResizeImage", hCall))
	{
		return 0;
	}

	//5*5平滑滤波
	Ipp16s pKernel3[3] = {1,1,1};

	IppiSize kernelSize3[2] = { 1, 3, 3, 1};
	m_TmpImg2.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	m_hFun.KxAverageFilterImage(m_TmpImg1, m_TmpImg2, kernelSize3[nDirection].width, kernelSize3[nDirection].height, (Ipp16s*)pKernel3, hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradientLV_KxAverageFilterImage", hCall))
	{
		return 0;
	}

	//3*3梯度
	Ipp16s pKernel4[2][3][3] =
	{
		2, 0, -2,
		2, 0, -2,
		2, 0, -2,

		2, 2, 2,
		0, 0, 0,
		-2, -2, -2,
	};
	IppiSize kernelSize4 = { 3, 3 };
	m_TmpImg3.Init(m_TmpImg2.nWidth, m_TmpImg2.nHeight, m_TmpImg2.nChannel);
	m_hFun.KxGeneralFilterImage(m_TmpImg2, m_TmpImg3, kernelSize4.width, kernelSize4.height, (Ipp16s*)pKernel4[nDirection], 1, hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradientLV_KxGeneralFilterImage", hCall))
	{
		return 0;
	}

	DstImg.Init(m_TmpImg3.nWidth, m_TmpImg3.nHeight, m_TmpImg3.nChannel);
	ippsSet_8u(bordervalue, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize Roi = { DstImg.nWidth - 2, DstImg.nHeight - 2 };
	if (DstImg.nChannel == _Type_G24)
	{
		ippiCopy_8u_C3R(m_TmpImg3.buf + 3 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 3 + DstImg.nPitch, DstImg.nPitch, Roi);
	}
	else
	{
		ippiCopy_8u_C1R(m_TmpImg3.buf + 1 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 1 + DstImg.nPitch, DstImg.nPitch, Roi);

	}

	return 1;

}

int CKxGradientProcess::SingleDirGradientSameWithBigSmooth(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return SingleDirGradientSameWithBigSmooth(SrcImg, DstImg, nDirection, nFilterType, nSoomthSize, bordervalue, hCall);
}

int CKxGradientProcess::SingleDirGradientSameWithBigSmooth(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	m_TmpImg2.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	if (nSoomthSize > 2)
	{
		if (nDirection < 2)
		{
			Ipp16s* pKernel = new Ipp16s[nSoomthSize * 1];
			for (int i = 0; i < nSoomthSize; i++)
			{
				pKernel[i] = 1;
			}
			IppiSize kernelSize;
			if (nDirection == 0) //垂直
			{
				kernelSize.width = 1;
				kernelSize.height = nSoomthSize;
			}
			else                 //水平
			{
				kernelSize.width = nSoomthSize;
				kernelSize.height = 1;
			}

			if (nFilterType == _AverageFilter)
			{
				//m_hFun.KxParallelAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo, nRowGrain, nColGrain);
				m_hFun.KxAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
				if (check_sts(hCallInfo, "SingleDirGradientSame_KxAverageFilterImage", hCall))
				{
					return 0;
				}
			}
			delete[] pKernel;
		}
		else  //2,3 direction
		{
			Ipp16s* pKernel = new Ipp16s[nSoomthSize * nSoomthSize];
			memset(pKernel, 0, nSoomthSize * nSoomthSize * sizeof(Ipp16s));
			for (int i = 0; i < nSoomthSize; i++)
			{
				if (nDirection == 2)
				{
					pKernel[i*nSoomthSize + nSoomthSize - 1] = 1;
				}
				else
				{
					pKernel[i*nSoomthSize + i] = 1;
				}
				
			}
			IppiSize kernelSize;
			kernelSize.width = nSoomthSize;
			kernelSize.height = nSoomthSize;


			if (nFilterType == _AverageFilter)
			{
				//m_hFun.KxParallelAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo, nRowGrain, nColGrain);
				m_hFun.KxAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
				if (check_sts(hCallInfo, "SingleDirGradientSame_KxAverageFilterImage", hCall))
				{
					return 0;
				}
			}

			delete[] pKernel;
		}

	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		m_hFun.KxCopyImage(SrcImg, m_TmpImg2, rc, hCallInfo);
		if (check_sts(hCallInfo, "SingleDirGradientSame_KxCopyImage", hCall))
		{
			return 0;
		}

	}

	//3*3梯度
	Ipp16s pKernel4[4][3][3] =
	{
		2, 0, -2,
		2, 0, -2,
		2, 0, -2,

		2, 2, 2,
		0, 0, 0,
		-2, -2, -2,

		2, 2, 0,
		2, 0, -2,
		0, -2, -2,

		0, 2, 2,
		-2, 0, 2,
		-2, -2, 0,
	};
	IppiSize kernelSize4 = { 3, 3 };

	m_TmpImg3.Init(m_TmpImg2.nWidth, m_TmpImg2.nHeight, m_TmpImg2.nChannel);
	m_hFun.KxGeneralFilterImage(m_TmpImg2, m_TmpImg3, kernelSize4.width, kernelSize4.height, (Ipp16s*)pKernel4[nDirection], 1, hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradientSame_KxGeneralFilterImage", hCall))
	{
		return 0;
	}

	DstImg.Init(m_TmpImg3.nWidth, m_TmpImg3.nHeight, m_TmpImg3.nChannel);
	ippsSet_8u(bordervalue, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize Roi = { DstImg.nWidth - 2, DstImg.nHeight - 2 };
	if (DstImg.nChannel == _Type_G24)
	{
		ippiCopy_8u_C3R(m_TmpImg3.buf + 3 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 3 + DstImg.nPitch, DstImg.nPitch, Roi);
	}
	else
	{
		ippiCopy_8u_C1R(m_TmpImg3.buf + 1 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 1 + DstImg.nPitch, DstImg.nPitch, Roi);

	}


	return 1;

}


int CKxGradientProcess::SingleDirGradientSameWithBigSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return SingleDirGradientSameWithBigSmoothParallel(SrcImg, DstImg, nDirection, nFilterType, nSoomthSize, bordervalue, hCall);
}

int CKxGradientProcess::SingleDirGradientSameWithBigSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, int nFilterType, int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	m_TmpImg2.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	if (nSoomthSize > 2)
	{
		if (nDirection < 2)
		{
			Ipp16s* pKernel = new Ipp16s[nSoomthSize * 1];
			for (int i = 0; i < nSoomthSize; i++)
			{
				pKernel[i] = 1;
			}
			IppiSize kernelSize;
			if (nDirection == 0) //垂直
			{
				kernelSize.width = 1;
				kernelSize.height = nSoomthSize;
			}
			else                 //水平
			{
				kernelSize.width = nSoomthSize;
				kernelSize.height = 1;
			}

			if (nFilterType == _AverageFilter)
			{
				m_hFun.KxParallelAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
				//m_hFun.KxAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
				if (check_sts(hCallInfo, "SingleDirGradientSame_KxAverageFilterImage", hCall))
				{
					return 0;
				}
			}
			delete[] pKernel;
		}
		else  //2,3 direction
		{
			Ipp16s* pKernel = new Ipp16s[nSoomthSize * nSoomthSize];
			memset(pKernel, 0, nSoomthSize * nSoomthSize * sizeof(Ipp16s));
			for (int i = 0; i < nSoomthSize; i++)
			{
				if (nDirection == 2)
				{
					pKernel[i*nSoomthSize + nSoomthSize - 1] = 1;
				}
				else
				{
					pKernel[i*nSoomthSize + i] = 1;
				}

			}
			IppiSize kernelSize;
			kernelSize.width = nSoomthSize;
			kernelSize.height = nSoomthSize;


			if (nFilterType == _AverageFilter)
			{
				m_hFun.KxParallelAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
				//m_hFun.KxAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
				if (check_sts(hCallInfo, "SingleDirGradientSame_KxAverageFilterImage", hCall))
				{
					return 0;
				}
			}

			delete[] pKernel;
		}

	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		m_hFun.KxCopyImage(SrcImg, m_TmpImg2, rc, hCallInfo);
		if (check_sts(hCallInfo, "SingleDirGradientSame_KxCopyImage", hCall))
		{
			return 0;
		}

	}

	//3*3梯度
	Ipp16s pKernel4[4][3][3] =
	{
		2, 0, -2,
		2, 0, -2,
		2, 0, -2,

		2, 2, 2,
		0, 0, 0,
		-2, -2, -2,

		2, 2, 0,
		2, 0, -2,
		0, -2, -2,

		0, 2, 2,
		-2, 0, 2,
		-2, -2, 0,
	};
	IppiSize kernelSize4 = { 3, 3 };

	m_TmpImg3.Init(m_TmpImg2.nWidth, m_TmpImg2.nHeight, m_TmpImg2.nChannel);
	m_hFun.KxParallelGeneralFilterImage(m_TmpImg2, m_TmpImg3, kernelSize4.width, kernelSize4.height, (Ipp16s*)pKernel4[nDirection], hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradientSame_KxGeneralFilterImage", hCall))
	{
		return 0;
	}

	DstImg.Init(m_TmpImg3.nWidth, m_TmpImg3.nHeight, m_TmpImg3.nChannel);
	ippsSet_8u(bordervalue, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize Roi = { DstImg.nWidth - 2, DstImg.nHeight - 2 };
	if (DstImg.nChannel == _Type_G24)
	{
		ippiCopy_8u_C3R(m_TmpImg3.buf + 3 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 3 + DstImg.nPitch, DstImg.nPitch, Roi);
	}
	else
	{
		ippiCopy_8u_C1R(m_TmpImg3.buf + 1 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 1 + DstImg.nPitch, DstImg.nPitch, Roi);

	}


	return 1;

}


int CKxGradientProcess::SingleDirGradientSameWithBoxSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection,  int nSoomthSize, Ipp8u bordervalue)
{
	KxCallStatus hCall;
	return SingleDirGradientSameWithBoxSmoothParallel(SrcImg, DstImg, nDirection, nSoomthSize, bordervalue, hCall);
}

int CKxGradientProcess::SingleDirGradientSameWithBoxSmoothParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection,  int nSoomthSize, Ipp8u bordervalue, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	m_TmpImg2.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	if (nSoomthSize > 2)
	{
		if (nDirection < 2)
		{
			Ipp16s* pKernel = new Ipp16s[nSoomthSize * 1];
			for (int i = 0; i < nSoomthSize; i++)
			{
				pKernel[i] = 1;
			}
			IppiSize kernelSize;
			if (nDirection == 0) //垂直
			{
				kernelSize.width = 1;
				kernelSize.height = nSoomthSize;
			}
			else                 //水平
			{
				kernelSize.width = nSoomthSize;
				kernelSize.height = 1;
			}

			m_hFun.KxParallelBoxFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, hCallInfo);
			//m_hFun.KxAverageFilterImage(SrcImg, m_TmpImg2, kernelSize.width, kernelSize.height, (Ipp16s*)pKernel, hCallInfo);
			if (check_sts(hCallInfo, "SingleDirGradientSameWithBoxSmoothParallel_KxParallelBoxFilterImage", hCall))
			{
				return 0;
			}
		
			delete[] pKernel;
		}
	}
	else
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		m_hFun.KxCopyImage(SrcImg, m_TmpImg2, rc, hCallInfo);
		if (check_sts(hCallInfo, "SingleDirGradientSameWithBoxSmoothParallel_KxCopyImage", hCall))
		{
			return 0;
		}

	}

	//3*3梯度
	Ipp16s pKernel4[4][3][3] =
	{
		2, 0, -2,
		2, 0, -2,
		2, 0, -2,

		2, 2, 2,
		0, 0, 0,
		-2, -2, -2,

		2, 2, 0,
		2, 0, -2,
		0, -2, -2,

		0, 2, 2,
		-2, 0, 2,
		-2, -2, 0,
	};
	IppiSize kernelSize4 = { 3, 3 };

	m_TmpImg3.Init(m_TmpImg2.nWidth, m_TmpImg2.nHeight, m_TmpImg2.nChannel);
	m_hFun.KxParallelGeneralFilterImage(m_TmpImg2, m_TmpImg3, kernelSize4.width, kernelSize4.height, (Ipp16s*)pKernel4[nDirection], hCallInfo);
	if (check_sts(hCallInfo, "SingleDirGradientSame_KxGeneralFilterImage", hCall))
	{
		return 0;
	}

	DstImg.Init(m_TmpImg3.nWidth, m_TmpImg3.nHeight, m_TmpImg3.nChannel);
	ippsSet_8u(bordervalue, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
	IppiSize Roi = { DstImg.nWidth - 2, DstImg.nHeight - 2 };
	if (DstImg.nChannel == _Type_G24)
	{
		ippiCopy_8u_C3R(m_TmpImg3.buf + 3 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 3 + DstImg.nPitch, DstImg.nPitch, Roi);
	}
	else
	{
		ippiCopy_8u_C1R(m_TmpImg3.buf + 1 + m_TmpImg3.nPitch, m_TmpImg3.nPitch, DstImg.buf + 1 + DstImg.nPitch, DstImg.nPitch, Roi);

	}


	return 1;

}


