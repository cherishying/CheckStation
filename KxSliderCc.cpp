
#include "KxSliderCc.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
#include "ippi_tl.h"
#include "KxBaseFunction.h"
using namespace tbb;

CKxSliderCc::CKxSliderCc()
{

}

CKxSliderCc::~CKxSliderCc()
{
}


void CKxSliderCc::GetExImgWidthAndHight(int nSrcW, int nSrcH,int &nDstW,int &nDstH, int GridX, int GridY)
{
	nDstH = ((nSrcH+(GridY-1))/GridY)*GridY;
	nDstW = ((nSrcW+(GridX-1))/GridX)*GridX;
}

int  CKxSliderCc::SilderCc(const kxCImageBuf& SrcBuf, const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf, int nStatus, int nGridX, int nGridY)
{
	KxCallStatus hCall;
	return SilderCc(SrcBuf, ModelBuf, DstBuf, nStatus, nGridX, nGridY, hCall);
}

int  CKxSliderCc::SilderCc(const kxCImageBuf& SrcBuf, const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall)
{
	if (SrcBuf.nWidth != ModelBuf.nWidth || SrcBuf.nHeight != ModelBuf.nHeight || SrcBuf.nPitch != ModelBuf.nPitch)
	{
		ippsSet_8u(0, DstBuf.buf, DstBuf.nPitch*DstBuf.nHeight);
		IppStatus status = IppStatus(kxSliderCcModeSizeErr);
		if (check_sts(status, "ModeSizeErr", hCall))
		{
			return 0;
		}
		return 0;
	}
	else
	{
		return SilderCc(SrcBuf.buf, SrcBuf.nPitch, ModelBuf.buf, ModelBuf.nPitch, DstBuf.buf, DstBuf.nPitch, 
			SrcBuf.nWidth, SrcBuf.nHeight, SrcBuf.nChannel, nStatus, nGridX, nGridY, hCall);
	}

}

int  CKxSliderCc::SilderCc(const unsigned char* srcTestBuf, int srcTestPitch,
	const unsigned char* srcModelBuf, int srcModelPitch,
	unsigned char* dstBuf, int dstPitch,
	int width, int height, int nChannel, int nStatus, int nGridX, int nGridY)
{
	KxCallStatus hCall;
	if (nChannel == _Type_G8)
	{
		return SilderCcC1(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	if (nChannel == _Type_G24)
	{
		return SilderCcC3(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	return 0;
}


int  CKxSliderCc::SilderCc( const unsigned char* srcTestBuf, int srcTestPitch,
							 const unsigned char* srcModelBuf, int srcModelPitch,
							 unsigned char* dstBuf, int dstPitch,
							 int width, int height, int nChannel, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall)
{
	if (nChannel == _Type_G8)
	{
		return SilderCcC1(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	if (nChannel == _Type_G24)
	{
		return SilderCcC3(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	return 0;
}

//  add by lyl, 2016/1/24 ,一直模板为底板，测试图像在上面滑动
// nStatus = 1 为检多， nStatus = 0 为检少
int  CKxSliderCc::SilderCcC1( const unsigned char* srcTestBuf, int srcTestPitch,
						  const unsigned char* srcModelBuf, int srcModelPitch,
						  unsigned char* dstBuf, int dstPitch,
						  int width, int height, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	int  nDstW,nDstH;

	int GridX = nGridX;
	int GridY = nGridY;

	GetExImgWidthAndHight(width,height,nDstW,nDstH,GridX,GridY);
	if (nStatus == 1) //检多
	{
		m_ImgCCBig.Init(nDstW, nDstH);
		m_ExtendImg.Init(nDstW + 2 * _Dixy, nDstH + 2 * _Dixy);
		IppiSize Roi = { m_ExtendImg.nWidth, m_ExtendImg.nHeight };
		status = ippiSet_8u_C1R(255, m_ExtendImg.buf, m_ExtendImg.nPitch, Roi);
		IppiSize roiSize = { m_ImgCCBig.nWidth, m_ImgCCBig.nHeight };
		status = ippiSet_8u_C1R(0, m_ImgCCBig.buf, m_ImgCCBig.nPitch, roiSize);

		IppiSize sizBig = { width, height };
		status = ippiCopy_8u_C1R(srcModelBuf, srcModelPitch, m_ExtendImg.buf + _Dixy*m_ExtendImg.nPitch + _Dixy * 1, m_ExtendImg.nPitch, sizBig);
		status = ippiCopy_8u_C1R(srcTestBuf, srcTestPitch, m_ImgCCBig.buf, m_ImgCCBig.nPitch, sizBig);

	}
	else  //检少
	{
		m_ImgCCBig.Init(nDstW + 2 * _Dixy, nDstH + 2 * _Dixy);
		m_ExtendImg.Init(nDstW, nDstH);
		IppiSize Roi = { m_ExtendImg.nWidth, m_ExtendImg.nHeight };
		status = ippiSet_8u_C1R(0, m_ExtendImg.buf, m_ExtendImg.nPitch, Roi);
		IppiSize roiSize = { m_ImgCCBig.nWidth, m_ImgCCBig.nHeight };
		status = ippiSet_8u_C1R(255, m_ImgCCBig.buf, m_ImgCCBig.nPitch, roiSize);

		IppiSize sizBig = { width, height };
		status = ippiCopy_8u_C1R(srcTestBuf, srcTestPitch, m_ImgCCBig.buf + _Dixy*m_ImgCCBig.nPitch + _Dixy * 1, m_ImgCCBig.nPitch, sizBig);
		status = ippiCopy_8u_C1R(srcModelBuf, srcModelPitch, m_ExtendImg.buf, m_ExtendImg.nPitch, sizBig);

	}
	if (check_sts(status, "SilderCcC1_ippiSet_8u_C1R", hCall))
	{
		return 0;
	}


	int  widthCC = nDstW / GridX; 
	int  heightCC = nDstH / GridY;


	m_GridImg.Init(GridX, GridY);
	m_ImgSlider.Init(nDstW, nDstH);
	m_MinImg.Init(GridX, GridY);

	//int nHStep = 3;
	//int nWStep = 3;

	int nX = (m_hParameter.m_nEndX - m_hParameter.m_nStartX)/m_hParameter.m_nStepX + 1;
	int nNumX =  nX > 0 ? nX:0;
	if (nNumX == 0)
	{
		return 0;
	}
	int* pIndexX = new int[nNumX];
	memset(pIndexX, 0, sizeof(int)*nNumX);

	for (int i = 0; i < nNumX; i++)
	{
		pIndexX[i] = m_hParameter.m_nStartX + i*m_hParameter.m_nStepX;
	}


	int nY = (m_hParameter.m_nEndY - m_hParameter.m_nStartY) / m_hParameter.m_nStepY + 1;
	int nNumY = nY > 0 ? nY : 0;
	if (nNumY == 0)
	{
		return 0;
	}
	int* pIndexY = new int[nNumY];
	memset(pIndexY, 0, sizeof(int)*nNumY);

	for (int i = 0; i < nNumY; i++)
	{
		pIndexY[i] = m_hParameter.m_nStartY + i*m_hParameter.m_nStepY;
	}


	//#pragma omp parallel for num_threads(4)

	for( int j = 0; j < heightCC; j++ )
	{
		for( int i = 0; i < widthCC; i++ )
		{
			Ipp64f nMin = INT_MAX;
			IppiSize  Size = { GridX, GridY };
			for( int y = 0; y < nNumY; y++ )
			{
				for( int x = 0; x < nNumX; x++ )
				{
					// src1 - src2
					if (nStatus == _Check_High)
					{
						//high - model
						status = ippiSub_8u_C1RSfs(  m_ExtendImg.buf + ( _Dixy + j*GridY + pIndexY[y] )*m_ExtendImg.nPitch + _Dixy + i*GridX + pIndexX[x], m_ExtendImg.nPitch, m_ImgCCBig.buf + j*GridY*m_ImgCCBig.nPitch + i*GridX,  m_ImgCCBig.nPitch,  
							m_GridImg.buf, m_GridImg.nPitch, Size, 0 );

						if (check_sts(status, "SilderCcC1_ippiSub_8u_C1RSfs_High", hCall))
						{
							return 0;
						}
					}
					else
					{   
						//model - low
						status = ippiSub_8u_C1RSfs(m_ImgCCBig.buf + (_Dixy + j*GridY + pIndexY[y])*m_ImgCCBig.nPitch + _Dixy + i*GridX + pIndexX[x], m_ImgCCBig.nPitch, m_ExtendImg.buf + j*GridY *m_ExtendImg.nPitch + i*GridX, m_ExtendImg.nPitch,
							m_GridImg.buf, m_GridImg.nPitch, Size, 0 );

						if (check_sts(status, "SilderCcC1_ippiSub_8u_C1RSfs_Low", hCall))
						{
							return 0;
						}
					}



					Ipp64f  nSum = 0;
					status = ippiSum_8u_C1R( m_GridImg.buf, m_GridImg.nPitch, Size, &nSum );

					if (check_sts(status, "SilderCcC1_ippiSum_8u_C1R", hCall))
					{
						return 0;
					}

					if ( nMin > nSum )
					{
						nMin = nSum;
						status = ippiCopy_8u_C1R(m_GridImg.buf, m_GridImg.nPitch, m_MinImg.buf, m_MinImg.nPitch, Size);

						if (check_sts(status, "SilderCcC1_ippiCopy_8u_C1R_third", hCall))
						{
							return 0;
						}

					}
				}
			}
			status = ippiCopy_8u_C1R( m_MinImg.buf, m_MinImg.nPitch, m_ImgSlider.buf + GridY*j*m_ImgSlider.nPitch + GridX*i, 
				m_ImgSlider.nPitch, Size);


			if (check_sts(status, "SilderCcC1_ippiCopy_8u_C1R_fourth", hCall))
			{
				return 0;
			}
		}
	}

	IppiSize Siz={width,height};    
	status = ippiCopy_8u_C1R(m_ImgSlider.buf, m_ImgSlider.nPitch, dstBuf, dstPitch, Siz);

	if (check_sts(status, "SilderCcC1_ippiCopy_8u_C1R_fiveth", hCall))
	{
		return 0;
	}

	delete []pIndexX;
	delete []pIndexY;

	return 1;
}

// add by lyl, 2016/1/24 ,一直模板为底板，测试图像在上面滑动
// nStatus = 1 为检多， nStatus = 0 为检少
int  CKxSliderCc::SilderCcC3( const unsigned char* srcTestBuf, int srcTestPitch,
							const unsigned char* srcModelBuf, int srcModelPitch,
							unsigned char* dstBuf, int dstPitch,
							int width, int height, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;

	int  nDstW,nDstH;

	int GridX = nGridX;
	int GridY = nGridY;

	GetExImgWidthAndHight(width,height,nDstW,nDstH,GridX,GridY);
	
	if (nStatus == 1) //检多
	{
		m_ImgCCBig.Init(nDstW, nDstH, 3);
		m_ExtendImg.Init(nDstW + 2 * _Dixy, nDstH + 2 * _Dixy, 3);
		IppiSize Roi = { m_ExtendImg.nWidth, m_ExtendImg.nHeight };

		Ipp8u pvalue[3] = { 255, 255, 255 };
		status = ippiSet_8u_C3R(pvalue, m_ExtendImg.buf, m_ExtendImg.nPitch, Roi);
		Ipp8u pvalue1[3] = { 0, 0, 0 };
		IppiSize roiSize = { m_ImgCCBig.nWidth, m_ImgCCBig.nHeight };
		status = ippiSet_8u_C3R(pvalue1, m_ImgCCBig.buf, m_ImgCCBig.nPitch, roiSize);

		IppiSize sizBig = { width, height };
		status = ippiCopy_8u_C3R(srcModelBuf, srcModelPitch, m_ExtendImg.buf + _Dixy*m_ExtendImg.nPitch + _Dixy * 3, m_ExtendImg.nPitch, sizBig);
		status = ippiCopy_8u_C3R(srcTestBuf, srcTestPitch, m_ImgCCBig.buf, m_ImgCCBig.nPitch, sizBig);

	}
	else  //检少
	{
		m_ImgCCBig.Init(nDstW + 2 * _Dixy, nDstH + 2 * _Dixy, 3);
		m_ExtendImg.Init(nDstW, nDstH, 3);
		IppiSize Roi = { m_ExtendImg.nWidth, m_ExtendImg.nHeight };

		Ipp8u pvalue[3] = { 0, 0, 0 };
		status = ippiSet_8u_C3R(pvalue, m_ExtendImg.buf, m_ExtendImg.nPitch, Roi);
		Ipp8u pvalue1[3] = { 255, 255, 255 };
		IppiSize roiSize = { m_ImgCCBig.nWidth, m_ImgCCBig.nHeight };
		status = ippiSet_8u_C3R(pvalue1, m_ImgCCBig.buf, m_ImgCCBig.nPitch, roiSize);

		IppiSize sizBig = { width, height };
		status = ippiCopy_8u_C3R(srcTestBuf, srcTestPitch, m_ImgCCBig.buf + _Dixy*m_ImgCCBig.nPitch + _Dixy * 3, m_ImgCCBig.nPitch, sizBig);
		status = ippiCopy_8u_C3R(srcModelBuf, srcModelPitch, m_ExtendImg.buf, m_ExtendImg.nPitch, sizBig);

	}


	int  widthCC = nDstW / GridX; 
	int  heightCC = nDstH / GridY;


	m_GridImg.Init(GridX, GridY, 3);
	m_ImgSlider.Init(nDstW, nDstH, 3);
	for (int k = 0; k < 3; k++)
	{
		m_MinImgRGB[k].Init(GridX, GridY);
	}


	int nX = (m_hParameter.m_nEndX - m_hParameter.m_nStartX) / m_hParameter.m_nStepX + 1;
	int nNumX = nX > 0 ? nX : 0;
	if (nNumX == 0)
	{
		return 0;
	}
	int* pIndexX = new int[nNumX];
	memset(pIndexX, 0, sizeof(int)*nNumX);

	for (int i = 0; i < nNumX; i++)
	{
		pIndexX[i] = m_hParameter.m_nStartX + i*m_hParameter.m_nStepX;
	}


	int nY = (m_hParameter.m_nEndY - m_hParameter.m_nStartY) / m_hParameter.m_nStepY + 1;
	int nNumY = nY > 0 ? nY : 0;
	if (nNumY == 0)
	{
		return 0;
	}
	int* pIndexY = new int[nNumY];
	memset(pIndexY, 0, sizeof(int)*nNumY);

	for (int i = 0; i < nNumY; i++)
	{
		pIndexY[i] = m_hParameter.m_nStartY + i*m_hParameter.m_nStepY;
	}



	for( int j = 0; j < heightCC; j++ )
	{
		for( int i = 0; i < widthCC; i++ )
		{
			Ipp64f nMin[3];
			for (int k = 0; k < 3; k++)
			{
				nMin[k] = INT_MAX;
			}

			IppiSize  Size = { GridX, GridY };
			for( int y = 0; y < nNumY; y++ )
			{
				for( int x = 0; x < nNumX; x++ )
				{
					// src1 - src2
					if (nStatus == _Check_High)
					{
						//high - model
						status = ippiSub_8u_C3RSfs(  m_ExtendImg.buf + ( _Dixy + j*GridY + pIndexY[y] )*m_ExtendImg.nPitch + (_Dixy + i*GridX + pIndexX[x])*3, m_ExtendImg.nPitch, m_ImgCCBig.buf + j*GridY*m_ImgCCBig.nPitch + i*GridX*3,  m_ImgCCBig.nPitch,  
							m_GridImg.buf, m_GridImg.nPitch, Size, 0 );
					}
					else
					{   
						//model - low
						status = ippiSub_8u_C3RSfs(m_ImgCCBig.buf + (_Dixy + j*GridY + pIndexY[y])*m_ImgCCBig.nPitch + (_Dixy + i*GridX + pIndexX[x]) * 3, m_ImgCCBig.nPitch, m_ExtendImg.buf + j*GridY *m_ExtendImg.nPitch +  i*GridX * 3, m_ExtendImg.nPitch,
							m_GridImg.buf, m_GridImg.nPitch, Size, 0 );
					}

					if (check_sts(status, "SilderCcC3_ippiSub_8u_C3RSfs", hCall))
					{
						return 0;
					}


					Ipp64f  nSum[3] = {0};
					status = ippiSum_8u_C3R( m_GridImg.buf, m_GridImg.nPitch, Size, nSum );
					if (check_sts(status, "SilderCcC3_ippiSum_8u_C3R", hCall))
					{
						return 0;
					}

					for (int k = 0; k < 3; k++)
					{
						if ( nMin[k] > nSum[k])
						{
							nMin[k] = nSum[k];
							status = ippiCopy_8u_C3C1R(m_GridImg.buf + k , m_GridImg.nPitch, m_MinImgRGB[k].buf , m_MinImgRGB[k].nPitch, Size);
							
							if (check_sts(status, "SilderCcC3_ippiCopy_8u_C3C1R", hCall))
							{
								return 0;
							}
						}
					}

				}

			}
			unsigned char* pSrc[3];
			for (int k = 0; k < 3; k++)
			{
				pSrc[k] = m_MinImgRGB[k].buf;
			}
			status = ippiCopy_8u_P3C3R(pSrc, m_MinImgRGB[0].nPitch, m_ImgSlider.buf + GridY*j*m_ImgSlider.nPitch + GridX*i*3, 
				m_ImgSlider.nPitch, Size);

			if (check_sts(status, "SilderCcC3_ippiCopy_8u_P3C3R", hCall))
			{
				return 0;
			}

		}
	}

	IppiSize Siz={width,height};    
	status = ippiCopy_8u_C3R(m_ImgSlider.buf, m_ImgSlider.nPitch, dstBuf, dstPitch, Siz);
	//ippiGradientColorToGray_8u_C3C1R(m_ImgSlider.buf, m_ImgSlider.nPitch, dstBuf, dstPitch, Siz, ippiNormInf);
	if (check_sts(status, "SilderCcC3_ippiCopy_8u_C3R_third", hCall))
	{
		return 0;
	}

	delete []pIndexX;
	delete []pIndexY;


	return 1;
}




int  CKxSliderCc::SilderCcParallel(const kxCImageBuf& SrcBuf, const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf, int nStatus, int nGridX, int nGridY)
{
	KxCallStatus hCall;
	if (SrcBuf.nWidth != ModelBuf.nWidth || SrcBuf.nHeight != ModelBuf.nHeight || SrcBuf.nPitch != ModelBuf.nPitch)
	{
		ippsSet_8u(0, DstBuf.buf, DstBuf.nPitch*DstBuf.nHeight);
		IppStatus status = IppStatus(kxSliderCcModeSizeErr);
		if (check_sts(status, "模板尺寸错误，请重新学习", hCall))
		{
			return 0;
		}
		return 0;
	}
	else
	{
		return SilderCc(SrcBuf.buf, SrcBuf.nPitch, ModelBuf.buf, ModelBuf.nPitch, DstBuf.buf, DstBuf.nPitch,
			SrcBuf.nWidth, SrcBuf.nHeight, SrcBuf.nChannel, nStatus, nGridX, nGridY, hCall);
	}

}


int  CKxSliderCc::SilderCcParallel(const kxCImageBuf& SrcBuf, const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall)
{
	if (SrcBuf.nWidth != ModelBuf.nWidth || SrcBuf.nHeight != ModelBuf.nHeight || SrcBuf.nPitch != ModelBuf.nPitch)
	{
		ippsSet_8u(0, DstBuf.buf, DstBuf.nPitch*DstBuf.nHeight);
		IppStatus status = IppStatus(kxSliderCcModeSizeErr);
		if (check_sts(status, "模板尺寸错误，请重新学习", hCall))
		{
			return 0;
		}
		return 0;
	}
	else
	{
		return SilderCc(SrcBuf.buf, SrcBuf.nPitch, ModelBuf.buf, ModelBuf.nPitch, DstBuf.buf, DstBuf.nPitch,
			SrcBuf.nWidth, SrcBuf.nHeight, SrcBuf.nChannel, nStatus, nGridX, nGridY, hCall);
	}

}



int  CKxSliderCc::SilderCcParallel(const unsigned char* srcTestBuf, int srcTestPitch,
	const unsigned char* srcModelBuf, int srcModelPitch,
	unsigned char* dstBuf, int dstPitch,
	int width, int height, int nChannel, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall)
{
	if (nChannel == _Type_G8)
	{
		return SilderCcC1(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	if (nChannel == _Type_G24)
	{
		return SilderCcC3(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	return 0;

}

int  CKxSliderCc::SilderCcParallel(const unsigned char* srcTestBuf, int srcTestPitch,
	const unsigned char* srcModelBuf, int srcModelPitch,
	unsigned char* dstBuf, int dstPitch,
	int width, int height, int nChannel, int nStatus, int nGridX, int nGridY)
{
	KxCallStatus hCall;
	if (nChannel == _Type_G8)
	{
		return SilderCcC1(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	if (nChannel == _Type_G24)
	{
		return SilderCcC3(srcTestBuf, srcTestPitch, srcModelBuf, srcModelPitch, dstBuf, dstPitch, width, height, nStatus, nGridX, nGridY, hCall);
	}
	return 0;

}