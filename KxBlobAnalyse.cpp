
#include "KxBlobAnalyse.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
using namespace tbb;

CKxBlobAnalyse::CKxBlobAnalyse()
{
	m_nCount = 0;
	m_nCountBlob = 0;
	m_nMinDots = 1;
	m_nOpenSize = 1;

	m_nGridXStep = 1;
	m_nGridYStep = 1;
	m_nGridX = 0;
	m_nGridY = 0;

	m_pBlobInfo = NULL;
	m_pSortBlobInfo = NULL;
	m_pBlobArea = NULL;
	m_pGrid = NULL;

	m_bOpenAdditionalInfo = true;
	m_bUseFoodFillAlogrithm = true;

	m_nConnectType = _USE8;

}

CKxBlobAnalyse::~CKxBlobAnalyse()
{
	Clear();
	ClearSortInfo();

}

void CKxBlobAnalyse::Clear()
{
	if (m_pBlobInfo)
	{
		delete []m_pBlobInfo;
		m_pBlobInfo = NULL;
	}
	if (m_pBlobArea)
	{
		delete []m_pBlobArea;
		m_pBlobArea = NULL;
	}
	m_nCount = 0;
	m_nCountBlob = 0;

}

int CKxBlobAnalyse::ToBlob(const unsigned char* buf, int nWidth, int nHeight, int nPitch)
{
	KxCallStatus hCall;
	Rect rc(0, 0, nWidth, nHeight);
	//rc.setup(0, 0, nWidth - 1, nHeight - 1);
	return ToBlob(buf, nPitch, rc, hCall);
}

int CKxBlobAnalyse::ToBlob( const unsigned char* buf, int nWidth, int nHeight, int nPitch, KxCallStatus& hCall )
{
	Rect rc(0, 0, nWidth, nHeight);
	//rc.setup(0, 0, nWidth-1, nHeight-1);
	return ToBlob(buf, nPitch, rc, hCall);
}

int CKxBlobAnalyse::ToBlob(const unsigned char* buf, int nPitch, const cv::Rect& rcBlob)
{
	KxCallStatus hCall;
	return ToSimpBlob(buf, nPitch, rcBlob, hCall);
}

int CKxBlobAnalyse::ToBlob( const unsigned char* buf, int nPitch, const cv::Rect& rcBlob, KxCallStatus& hCall)
{
	return ToSimpBlob(buf, nPitch, rcBlob, hCall);

}

int CKxBlobAnalyse::ToBlob(cv::InputArray SrcImg)
{
	KxCallStatus hCall;
	cv::Mat matSrcImg = SrcImg.getMat();
	return ToBlob(matSrcImg.data, matSrcImg.cols, matSrcImg.rows, matSrcImg.step, hCall);

}

int CKxBlobAnalyse::ToBlob(cv::InputArray SrcImg, KxCallStatus& hCall)
{
	cv::Mat matSrcImg = SrcImg.getMat();
	return ToBlob(matSrcImg.data, matSrcImg.cols, matSrcImg.rows, matSrcImg.step, hCall);

}

int CKxBlobAnalyse::MergeSomeConnections(const unsigned char* buf, int nWidth, int nHeight, int nPitch,unsigned char* pDst, int nDstPitch, KxCallStatus& hCall)
{
	IppStatus  status = ippStsNoErr;
    //use open operate to merge some connections
	m_OpenImg = Mat(nHeight, nWidth, CV_8UC1);
	if (m_nOpenSize > 1)
	{
		IppiBorderType borderType = ippBorderRepl;
		Ipp8u borderValue = 0;
		IppiSize roiSize = {nWidth, nHeight};
		IppiSize maskSize = {m_nOpenSize, m_nOpenSize};
		int nSpecSize = 0, nBufferSize = 0;
		if (check_sts(status = ippiMorphAdvGetSize_8u_C1R(roiSize, maskSize, &nSpecSize, &nBufferSize), 
			           "ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			return 0;
		}

		IppiMorphAdvState* pSpec = (IppiMorphAdvState*)ippsMalloc_8u(nSpecSize);
		Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

		m_MaskImg = Mat(maskSize.height, maskSize.width, CV_8UC1);
		ippsSet_8u(1, m_MaskImg.data, maskSize.width*maskSize.height);

		if (check_sts(status = ippiMorphAdvInit_8u_C1R(roiSize, m_MaskImg.data, maskSize, pSpec, pBuffer), 
			"ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			return 0;
		}

		if (check_sts(status = ippiMorphCloseBorder_8u_C1R(buf, nPitch, pDst, nDstPitch, roiSize, borderType, borderValue, pSpec, pBuffer), 
			"ippiMorphAdvGetSize_8u_C1R", hCall))
		{
			ippsFree(pSpec);
			ippsFree(pBuffer);
			return 0;
		}
		ippsFree(pSpec);
		ippsFree(pBuffer);
	}
	else
	{
		IppiSize Roi = {nWidth, nHeight};
		if (check_sts(status  = ippiCopy_8u_C1R(buf, nPitch, pDst, nWidth, Roi), "ippiCopy", hCall))
		{
			return 0;
		}
	}
	return 1;
}

int CKxBlobAnalyse::ToSimpBlob( const unsigned char* buf, int nPitch, const cv::Rect& rcBlob, KxCallStatus& hCall)
{
	/*
		这个函数应该是上古时代的 blob, 
	*/

	Clear(); //clear the data struct
	
	hCall.Clear();
	IppStatus status;

	IppiSize Roi = {rcBlob.width, rcBlob.height};
	if (m_BufferImg.data == NULL || Roi.width != m_Img.cols || Roi.height != m_Img.rows)
	{
		int  nBufferSize;
		status = ippiLabelMarkersGetBufferSize_16u_C1R(Roi, &nBufferSize);

		if (check_sts(status, "ToSimpBlob_ippiLabelMarkersGetBufferSize", hCall))
		{
			return 0;
		}
		m_BufferImg.create(1, nBufferSize, CV_8UC1);
	}
	//copy a Img
	m_Img.create(Roi.height, Roi.width, CV_8UC1);
	status = ippiCopy_8u_C1R(buf, nPitch, m_Img.data, m_Img.step, Roi);

	if (check_sts(status, "ToSimpBlob_ippiCopy", hCall))
	{
		return 0;
	}

	//merge some connections
	m_PreImg.create(Roi.height, Roi.width, CV_8UC1);
	KxCallStatus hTempCall;
	MergeSomeConnections(buf, Roi.width, Roi.height, nPitch, m_PreImg.data, m_PreImg.step, hTempCall);

	if (check_sts(hTempCall, "ToSimpBlob_MergeSomeConnections", hCall))
	{
		return 0;
	}


	m_pTmpImg.create(Roi.height, Roi.width, CV_16UC1);
	int left = rcBlob.x + rcBlob.width - 1;
	//int right = 
	status = ippiConvert_8u16u_C1R(m_PreImg.data + rcBlob.y * m_PreImg.step + left, m_PreImg.step, (Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi);
	
	if (check_sts(status, "ToSimpBlob_ippiConvert", hCall))
	{
		return 0;
	}

	//first,label the connections components
	int nCount = 0;
	status = ippiLabelMarkers_16u_C1IR((Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1 ), &nCount, m_BufferImg.data);

	if (check_sts(status, "ToSimpBlob_ippiLabelMarkers", hCall))
	{
		return 0;
	}

	if ( nCount == 0 )
	{
		return 1;
	}

	if ( nCount >= _MAX_Lable_Count )
	{
		status = IppStatus(kxBlobAnalyseCountErr);
		if (check_sts(status, "ToSimpBlob_TheBlobCountsGreateMaxLabelCount", hCall))
		{
			return 0;
		}
	}

	if (nCount > _Max_FloodBlob_Count)
	{
		if (m_nGridX != rcBlob.width/m_nGridXStep || m_nGridY != rcBlob.height/m_nGridYStep)
		{
			SetGridXY(rcBlob.width/m_nGridXStep, rcBlob.height/m_nGridYStep);
		}
		hTempCall.Clear();
		ToGridBlob(m_PreImg.data, m_PreImg.step, rcBlob, hTempCall);
		m_bUseFoodFillAlogrithm = false;

		if (check_sts(hTempCall, "ToSimpBlob_ToGridBlob", hCall))
		{
			return 0;
		}
	}
	else
	{
		hTempCall.Clear();
		UseFloodAlgorithmComputeBlob(m_pTmpImg, nCount, hTempCall);
		if (check_sts(hTempCall, "ToSimpBlob_UseFloodAlgorithmComputeBlob", hCall))
		{
			return 0;
		}

		m_bUseFoodFillAlogrithm = true;
	}

	return 1;
}

int CKxBlobAnalyse::UseFloodAlgorithmComputeBlob(Mat& Img16u, int nCount, KxCallStatus& hCall)
{
	IppStatus status;

	hCall.Clear();
	//second, use flood fill algorithm to find every blob's info
	m_pBlobInfo = new SingleBlobInfo[nCount];
	status = ippsZero_8u( (Ipp8u*)m_pBlobInfo, nCount*sizeof(SingleBlobInfo) );

	if (check_sts(status, "ippsZero", hCall))
	{
		return 0;
	}

	int nBufferSize;
	IppiSize Roi = {Img16u.cols, Img16u.rows};


	status = ippiFloodFillGetSize(Roi, &nBufferSize);

	if (check_sts(status, "ippiFloodFillGetSize", hCall))
	{
		return 0;
	}


	m_BufferImgX.create(1, nBufferSize, CV_8UC1);


	m_pImg16u.create(Roi.height, Roi.width, CV_16UC1);
	status = ippiCopy_16u_C1R((Ipp16u*)Img16u.data, Img16u.step, (Ipp16u*)m_pImg16u.data, m_pImg16u.step, Roi);

	if (check_sts(status, "ippiCopy_16u", hCall))
	{
		return 0;
	}


	IppiConnectedComp pRegion;
	int nLen = Roi.width * Roi.height;
	int nPos = 0;

	Ipp16u* ToFind = new Ipp16u[nCount];
	for (int i = 0; i < nCount; i++)
	{
		ToFind[i] = _Min_Lable + i;
	}
	IppiPoint seed;
	int k = 0;
	//Ipp16u valFind;
	//int nPos1;
	int nStart = 0;
	int nIndex = 0;
	int nSearchLen = nLen;
	int nBlob = 0;

	int n = _Min_Lable +_MAX_Lable_Count+1;

	while (k < nCount)
	{
		//first, find a best seed
		//status =  ippsFindCAny_16u(m_pImg16u.data + nStart, nSearchLen, ToFind, k, &nPos);
		status = ippsFind_8u((Ipp8u*)m_pImg16u.data, nSearchLen*sizeof(Ipp16u), (Ipp8u*)(&ToFind[k]), sizeof(Ipp16u), &nPos);

		if (check_sts(status, "ippsFindCAny", hCall))
		{
			return 0;
		}

		seed.y = nPos / (Roi.width*sizeof(Ipp16u));
		seed.x =  (nPos % (Roi.width*sizeof(Ipp16u)))/sizeof(Ipp16u);

		//valFind = m_pImg16u.data[seed.y * m_pImg16u.nWidth + seed.x];

		//status = ippsFindC_16u(ToFind, k, valFind, &nPos1);

		//if (check_sts(status, "ippsFindC", hCall))
		//{
		//	return 0;
		//}

		//status = ippsRemove_16u_I(ToFind, &k, nPos1, 1);

		//if (check_sts(status, "ippsRemove", hCall))
		//{
		//	return 0;
		//}

		status = ippiFloodFill_8Con_16u_C1IR((Ipp16u*)m_pImg16u.data, m_pImg16u.step, Roi, seed, n++, &pRegion, m_BufferImgX.data);

		if (check_sts(status, "ippiFloodFill_8Con", hCall))
		{
			return 0;
		}

		m_pBlobInfo[nBlob].m_nDots = (int)pRegion.area;
		m_pBlobInfo[nBlob].m_rc = Rect(pRegion.rect.x, pRegion.rect.y, pRegion.rect.width, pRegion.rect.height);
		m_pBlobInfo[nBlob].m_nLabel = (int)pRegion.value[0];
		m_pBlobInfo[nBlob].m_PtSeed.x = seed.x;
		m_pBlobInfo[nBlob].m_PtSeed.y = seed.y;

		if (pRegion.area > m_nMinDots)
		{
			nIndex++;
		}

		nBlob++;
		k++;
		//nStart += nPos;
		//nSearchLen = nLen - nStart;

	}
	m_nCount = nCount;
	m_nCountBlob = nIndex;


	delete []ToFind;

	return 1;

}

void CKxBlobAnalyse::ClearSortInfo()
{
	if (m_pSortBlobInfo)
	{
		delete []m_pSortBlobInfo;
		m_pSortBlobInfo = NULL;
	}

}


int  CKxBlobAnalyse::SortByDots(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
    IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount , m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pDots = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pDots[i] = m_pBlobInfo[i].m_nDots;
	}
	status = ippsSortIndexDescend_64f_I(pDots, pIndex, m_nCount);

	if (check_sts(status, "SortByDots_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u( (Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortByDots_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		//compute blob's general information
		IppiSize roiSize = {m_pSortBlobInfo[i].m_rc.width, m_pSortBlobInfo[i].m_rc.height};
        
		m_bOpenAdditionalInfo = nOpenComputeAdanceFeatures > 0 ? true : false;
		if (m_bOpenAdditionalInfo)
		{
			//compute some smallest rectangle's information
			IppiSize roi = {m_pImg16u.cols, m_pImg16u.rows};
			Rect rc;
			int nFacotorX, nFacotorY;
			if (m_bUseFoodFillAlogrithm)
			{
				rc = m_pSortBlobInfo[i].m_rc;
				nFacotorX = 1;
				nFacotorY = 1;
			}
			else
			{
				rc = m_pBlobArea[pIndex[i]];
				nFacotorX = m_nGridXStep;
				nFacotorY = m_nGridYStep;
			}

			Ipp16u newval = m_pSortBlobInfo[i].m_nLabel ;
			IppiSize Roi = {rc.width, rc.height};
			status = ippiCopy_16u_C1R((Ipp16u*)m_pImg16u.data + rc.y * m_pImg16u.cols + rc.x, m_pImg16u.step,
				(Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi);

			if (check_sts(status, "SortByDots_ippiCopy_first", hCall))
			{
				return 0;
			}

			status = ippiThreshold_LTValGTVal_16u_C1IR((Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi, newval, 0, newval, 0);

			if (check_sts(status, "SortByDots_ippiThreshold_LTValGTVal", hCall))
			{
				return 0;
			}

			//status = ippiCompareC_16u_C1R(m_pTmpImg.data, m_pTmpImg.nPitch, newval-1, m_PreImg.data, m_PreImg.nPitch,Roi, ippCmpGreater);
            status = ippiConvert_16u8u_C1R((Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, m_PreImg.data, m_PreImg.step, Roi);
			if (check_sts(status, "SortByDots_ippiConvert_16u8u_C1R", hCall))
			{
				return 0;
			}


			if (m_bUseFoodFillAlogrithm)
			{
				status = ippiAnd_8u_C1IR(m_Img.data + rc.y * m_Img.step + rc.x, m_Img.step, m_PreImg.data, m_PreImg.step, Roi);
				
				if (check_sts(status, "SortByDots_ippiAnd", hCall))
				{
					return 0;
				}
				
				Ipp64f pSum;
				status = ippiSum_8u_C1R(m_PreImg.data, m_PreImg.step, Roi, &pSum);
				if (check_sts(status, "SortByDots_ippiSum", hCall))
				{
					return 0;
				}

				m_pSortBlobInfo[i].m_nEnergy = int(pSum);
			}
			else
			{
				m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
			}

			double factorX, factorY;
			int nNormWidth, nNormHeight;
			if ((Roi.width + Roi.height > 200) && (Roi.height > 5) && (Roi.width > 5))
			{
				IppiSize srcSize = {Roi.width, Roi.height};
				IppiRect srcRoi = {0, 0, srcSize.width, srcSize.height};
				nNormWidth  = gMin(int(_NORM_WIDTH), Roi.width);
				nNormHeight = gMin(int(_NORM_HEIGHT), Roi.height);
				IppiSize dstRoi = {nNormWidth, nNormHeight};
				factorX = dstRoi.width*1.0 / srcRoi.width;
				factorY = dstRoi.height*1.0 / srcRoi.height;

				//m_OpenImg.Init(nNormWidth, nNormHeight);
				m_SrcImg = m_PreImg;
				m_DstImg = m_OpenImg;
				m_hBaseFun.KxResizeImage(m_SrcImg, m_DstImg, KxLinear, hTmpCall);
				if (check_sts(hTmpCall, "KxResizeImage", hCall))
				{
					return 0;
				}

				//ippiResize_8u_C1R(m_PreImg.data, srcSize, m_PreImg.nPitch, srcRoi, m_OpenImg.data,
				//	m_OpenImg.nPitch, dstRoi, factorX, factorY, IPPI_INTER_LINEAR);
				//IppiRect dstRect = {0, 0, dstRoi.width, dstRoi.height};
				//int nBuffer;
				//status = ippiResizeGetBufSize(srcRoi, dstRect, 1, IPPI_INTER_LINEAR, &nBuffer);

				//if (check_sts(status, "SortByDots_ippiResizeGetBufSize", hCall))
				//{
				//	return 0;
				//}
				//
    //            Ipp8u* pBuffer = new Ipp8u[nBuffer];
				//status = ippiResizeSqrPixel_8u_C1R(m_PreImg.data, srcSize, m_PreImg.nPitch, srcRoi, m_OpenImg.data,
				//	m_OpenImg.nPitch, dstRect, factorX, factorY, 0, 0, IPPI_INTER_LINEAR, pBuffer);

				//if (check_sts(status, "SortByDots_ippiResizeSqrPixel", hCall))
				//{
				//	return 0;
				//}

				//delete []pBuffer;
			}
			else
			{
				nNormWidth = Roi.width;
                nNormHeight = Roi.height;
				IppiSize srcSize = {Roi.width, Roi.height};
				m_DstImg = m_OpenImg;				
				status = ippiCopy_8u_C1R(m_PreImg.data, m_PreImg.step, m_DstImg.data, m_DstImg.step, srcSize);

				if (check_sts(status, "SortByDots_ippiCopy_second", hCall))
				{
					return 0;
				}


				factorX = 1.0;
				factorY = 1.0;
			}


			m_hKxMinRect.Check(m_DstImg.data, nNormWidth, nNormHeight, m_DstImg.step);

			m_pSortBlobInfo[i].m_fAngle = m_hKxMinRect.GetResult().m_fAngle;		
			m_pSortBlobInfo[i].m_nCircumference = int(m_hKxMinRect.GetResult().m_nCircumference*nFacotorX*nFacotorY*1.0/(factorX*factorY));

            int nH = m_hKxMinRect.GetResult().m_nMinRectHeight;
			int nW = m_hKxMinRect.GetResult().m_nMinRectWidth;
			float fAngle = float(m_hKxMinRect.GetResult().m_fAngle * PI / 180);
			m_pSortBlobInfo[i].m_nMinRectHeight = int(sqrt((nH*sin(fAngle)/factorX)*(nH*sin(fAngle)/factorX) + (nH*cos(fAngle)/factorY)*(nH*cos(fAngle)/factorY)) + 0.5);
			m_pSortBlobInfo[i].m_nMinRectWidth =  int(sqrt((nW*sin(fAngle)/factorY)*(nW*sin(fAngle)/factorY) + (nW*cos(fAngle)/factorX)*(nW*cos(fAngle)/factorX)) + 0.5);
            m_pSortBlobInfo[i].m_fMinArea = m_pSortBlobInfo[i].m_nMinRectHeight*m_pSortBlobInfo[i].m_nMinRectWidth ;


			m_pSortBlobInfo[i].m_fRatio = float(gMax(m_pSortBlobInfo[i].m_nMinRectWidth,m_pSortBlobInfo[i].m_nMinRectHeight)/gMin(m_pSortBlobInfo[i].m_nMinRectWidth,m_pSortBlobInfo[i].m_nMinRectHeight));
			for (int k = 0; k <4; k++)
			{
				m_pSortBlobInfo[i].m_Pt[k].x = int((m_hKxMinRect.GetResult().m_Pt[k].x/factorX + rc.x)*nFacotorX*1.0);
				m_pSortBlobInfo[i].m_Pt[k].y = int((m_hKxMinRect.GetResult().m_Pt[k].y/factorY + rc.y)*nFacotorY*1.0);
			}
		}
	}


	delete []pDots;
	delete []pIndex;

	return 1;

}


void CKxBlobAnalyse::SetGridXY( int nGridX, int nGridY )
{
	assert( nGridX > 0 && nGridY > 0 );

	if( m_pGrid )
		delete m_pGrid;
	m_nGridX = nGridX;
	m_nGridY = nGridY;
	m_bOpenGrid = true;
	m_pGrid = new CGridD[ m_nGridX * m_nGridY ];
}


void CKxBlobAnalyse::InitGrid( const cv::Rect& rc )
{
	int  nX = rc.width / m_nGridX;
	int  nY = rc.height / m_nGridY;
	assert( nX > 0 && nY > 0 );  
	int  nXL = rc.width % m_nGridX;
	int  nYL = rc.height % m_nGridY;
	for( int i = 0; i < m_nGridX; i++ )
	{
		m_pGrid[i].m_rc.y = rc.y;
		if( i == 0 )
			m_pGrid[i].m_rc.x = rc.x;
		else
			m_pGrid[i].m_rc.x = m_pGrid[i-1].m_rc.width;
		//m_pGrid[i].m_rc.right = m_pGrid[i].m_rc.left + nX - 1;
		m_pGrid[i].m_rc.width = nX;
		if( nXL )
		{
			m_pGrid[i].m_rc.width++;
			nXL--;
		}
		//m_pGrid[i].m_rc.bottom = m_pGrid[i].m_rc.top + nY - 1;
		m_pGrid[i].m_rc.height = nY;

	}
	for( int y = 1; y < m_nGridY; y++ )
	{
		int  kL = 0;
		if( nYL )
		{
			kL = 1;
			nYL--;
		}
		for( int x = 0; x < m_nGridX; x++ )
		{
			//m_pGrid[y*m_nGridX+x].m_rc.left = m_pGrid[(y-1)*m_nGridX+x].m_rc.left;
			//m_pGrid[y*m_nGridX+x].m_rc.right = m_pGrid[(y-1)*m_nGridX+x].m_rc.right;
			//m_pGrid[y*m_nGridX+x].m_rc.top = m_pGrid[(y-1)*m_nGridX+x].m_rc.bottom + 1;
			//m_pGrid[y*m_nGridX+x].m_rc.bottom = m_pGrid[y*m_nGridX+x].m_rc.top + nY -1 + kL;
			// --------------- new ---------------------//
			int left = m_pGrid[(y - 1)*m_nGridX + x].m_rc.x;
			int right = m_pGrid[(y - 1)*m_nGridX + x].m_rc.width + m_pGrid[(y - 1)*m_nGridX + x].m_rc.x;
			int top = m_pGrid[(y - 1)*m_nGridX + x].m_rc.y + m_pGrid[(y - 1)*m_nGridX + x].m_rc.height + 1;
			int bottom = m_pGrid[y*m_nGridX + x].m_rc.y + nY - 1 + kL;

			m_pGrid[y*m_nGridX + x].m_rc.x = left;
			m_pGrid[y*m_nGridX + x].m_rc.y = top;
			m_pGrid[y*m_nGridX + x].m_rc.width = right - left + 1;
			m_pGrid[y*m_nGridX + x].m_rc.height = bottom - top + 1;

		}
	}
}


int  CKxBlobAnalyse::ToGridBlob( const unsigned char* buf, int nPitch, const cv::Rect& rcBlob, KxCallStatus& hCall)
{
	// buf 是一张未做blob的图, 输入的rect不清楚是用在哪的
	IppStatus  status;
	//KxCallStatus hCall;
	hCall.Clear();

	if (m_rcCheck.y != rcBlob.y || (m_rcCheck.y + m_rcCheck.height - 1) != (rcBlob.y + rcBlob.height - 1)
		|| m_rcCheck.x != rcBlob.x || (m_rcCheck.x + m_rcCheck.width - 1) != (rcBlob.x + rcBlob.width - 1))
	{
		InitGrid( rcBlob );
	}
	//-----------------------------------------------------------------------------------------
	for( int i = 0; i < (m_nGridX*m_nGridY); i++ )
	{
		m_pGrid[i].m_nDots = 0;
		m_pGrid[i].m_nPower = 0;
		for( int y = m_pGrid[i].m_rc.y; y <= (m_pGrid[i].m_rc.y + m_pGrid[i].m_rc.height - 1); y++ )
		{
			for( int x = m_pGrid[i].m_rc.x; x <= (m_pGrid[i].m_rc.x + m_pGrid[i].m_rc.width - 1); x++ )
			{
				int  nOff = y * nPitch + x;
				if( buf[nOff] )
				{
					m_pGrid[i].m_nDots++;
					m_pGrid[i].m_nPower += buf[nOff];
				}
			}
		}
	}
	//-------------------------------------------------------------------------------------------
	IppiSize   roiSize;
	roiSize.height = m_nGridY;
	roiSize.width = m_nGridX;

	m_pImg16u.create(m_nGridY, m_nGridX, CV_16UC1);

	for( int y = 0; y < m_nGridY; y++ )
	{
		for( int x = 0; x < m_nGridX; x++ )
		{
			if(	m_pGrid[y*m_nGridX+x].m_nDots < 1 )
			{
				m_pImg16u.data[y*roiSize.width + x] = 0;
				m_pGrid[y*m_nGridX+x].m_nDots = 0;
				m_pGrid[y*m_nGridX+x].m_nPower = 0;
			}
			else
			{
				m_pImg16u.data[y*roiSize.width + x] = 0xFF;
			}

		}
	}

	Clear();

	//first,label the connections components
	int nBufferSize;	
	status = ippiLabelMarkersGetBufferSize_16u_C1R(roiSize, &nBufferSize);

	if (check_sts(status, "ippiLabelMarkersGetBufferSize", hCall))
	{
		return 0;
	}

	Ipp8u* pBuffer;
	pBuffer = new Ipp8u[nBufferSize];
	status = ippiLabelMarkers_16u_C1IR((Ipp16u*)m_pImg16u.data, m_pImg16u.step, roiSize, _Min_Lable, _Min_Lable + _MAX_Lable_Count, 
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1 ), &m_nCount, pBuffer);

	if (check_sts(status, "ippiLabelMarkers_16u", hCall))
	{
		return 0;
	}

	delete []pBuffer;

	if (m_nCount == 0)
	{
		return 1;
	}
	m_pBlobInfo = new SingleBlobInfo[m_nCount];
	status = ippsZero_8u((Ipp8u*)m_pBlobInfo, m_nCount*sizeof(SingleBlobInfo));

	if (check_sts(status, "ippsZero_8u", hCall))
	{
		return 0;
	}


	m_pBlobArea = new Rect[m_nCount];

	for (int i = 0; i < m_nCount; i++)
	{
		m_pBlobArea[i] = Rect(Point(INT_MAX, INT_MAX), Point(-INT_MAX, -INT_MAX));
	}

	for( int y = 0; y < m_pImg16u.rows; y++ )
	{
		for( int x = 0; x < m_pImg16u.cols; x++ )
		{
			int  n = (int)m_pImg16u.data[y*m_pImg16u.cols+x]-_Min_Lable;
            if (n >= 0)
            {
				m_pBlobInfo[n].m_nLabel = (int)m_pImg16u.data[y*m_pImg16u.cols+x];
				m_pBlobInfo[n].m_PtSeed = Point(x, y);
				m_pBlobInfo[n].m_nDots += m_pGrid[y*m_nGridX + x].m_nDots;
				m_pBlobInfo[n].m_nEnergy += m_pGrid[y*m_nGridX + x].m_nPower;
				m_pBlobArea[n].x = gMin(m_pBlobArea[n].x, x);
				m_pBlobArea[n].y = gMin(m_pBlobArea[n].y, y);
				m_pBlobArea[n].width = gMax(m_pBlobArea[n].x + m_pBlobArea[n].width - 1, x) - m_pBlobArea[n].x + 1;
				m_pBlobArea[n].height = gMax(m_pBlobArea[n].y + m_pBlobArea[n].height - 1, y) - m_pBlobArea[n].y + 1;
            }
		}
	}
    
	int nIndex = 0;
	for( int i = 0; i < m_nCount; i++)
	{
		int areatop = m_pBlobArea[i].y;
		int areabottom = m_pBlobArea[i].y + m_pBlobArea[i].height - 1;
		int arealeft = m_pBlobArea[i].x;
		int arearight = m_pBlobArea[i].x + m_pBlobArea[i].width - 1;;

		int nLeft = m_pGrid[areatop*m_nGridX + arealeft].m_rc.x;
		int nTop = m_pGrid[areatop*m_nGridX + arealeft].m_rc.y;
		int nWidth = m_pGrid[areabottom*m_nGridX + arearight].m_rc.width;
		int nHeight = m_pGrid[areabottom*m_nGridX + arearight].m_rc.height;
		m_pBlobInfo[i].m_rc = Rect( nLeft, nTop, nWidth, nHeight);

		if (m_pBlobInfo[i].m_nDots > m_nMinDots)
		{
			nIndex++;
		}

	}

	m_nCountBlob = nIndex;

	return 1;
}

int CKxBlobAnalyse::ToBlobParallel(cv::InputArray SrcImg, int nSortByMode, int nMaxSortDots, int nMergeSize, int nOpenComputeAdanceFeatures)
{
	KxCallStatus hCall;
	return ToBlobParallel(SrcImg, nSortByMode, nMaxSortDots, nMergeSize, nOpenComputeAdanceFeatures, hCall);
}

int CKxBlobAnalyse::ToBlobParallel(cv::InputArray SrcImg, int nSortByMode, int nMaxSortDots, int nMergeSize, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	Clear(); //clear the data struct

	hCall.Clear();
	IppStatus status;
	m_nOpenSize = nMergeSize;

	Mat matSrcImg = SrcImg.getMat();

	IppiSize Roi = { SrcImg.cols, SrcImg.rows };
	if (m_BufferImg.data == NULL || Roi.width != m_Img.cols || Roi.height != m_Img.rows)
	{
		int  nBufferSize;
		status = ippiLabelMarkersGetBufferSize_16u_C1R(Roi, &nBufferSize);

		if (check_sts(status, "ToBlobParallel_ippiLabelMarkersGetBufferSize", hCall))
		{
			return 0;
		}

		m_BufferImg.create(1, nBufferSize, CV_8UC1);
	}
	//copy a Img
	m_Img.create(Roi.height, Roi.width, CV_8UC1);
	status = ippiCopy_8u_C1R(matSrcImg.data, matSrcImg.step, m_Img.data, m_Img.step, Roi);

	if (check_sts(status, "ToBlobParallel_ippiCopy", hCall))
	{
		return 0;
	}

	//merge some connections
	m_PreImg.create(Roi.height, Roi.width, CV_8UC1);
	KxCallStatus hTempCall;
	hTempCall.Clear();
	MergeSomeConnections(matSrcImg.data, Roi.width, Roi.height, matSrcImg.step, m_PreImg.data, m_PreImg.step, hTempCall);

	if (check_sts(hTempCall, "ToBlobParallel_MergeSomeConnections", hCall))
	{
		return 0;
	}


	m_pTmpImg.create(Roi.height, Roi.width, CV_16UC1);
	m_pImg16u.create(Roi.height, Roi.width, CV_16UC1);

	status = ippiConvert_8u16u_C1R(m_PreImg.data, m_PreImg.step, (Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi);

	if (check_sts(status, "ToBlobParallel_ippiConvert", hCall))
	{
		return 0;
	}

	//first,label the connections components
	int nCount = 0;
	status = ippiLabelMarkers_16u_C1IR((Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1), &nCount, m_BufferImg.data);

	if (check_sts(status, "ToBlobParallel_ippiLabelMarkers", hCall))
	{
		return 0;
	}

	if (nCount == 0)
	{
		return 1;
	}

	m_nCount = nCount;

	//sort by dots or energy or size
	switch (nSortByMode)
	{
	    case _SORT_BYDOTS:
	    {
							 SortByDot(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
			break;
	    }
		case _SORT_BYENERGY:
		{
							   SortByEnergy(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
		   break;
	    }
		case _SORT_BYSIZE:
		{
							 SortBySize(nMaxSortDots, nOpenComputeAdanceFeatures, hTempCall);
			break;
		}
		default:
			break;
	}

	if (check_sts(hTempCall, "ToBlobParallel_", hCall))
	{
		return 0;
	}

	return 1;


}

void CKxBlobAnalyse::GetBlobImage(int nLabel, cv::Rect rc, cv::OutputArray blobimg)
{
	if (m_pTmpImg.cols == 0 || m_pTmpImg.rows == 0)
		return;

	Mat matblobimg = blobimg.getMat();

	Ipp16u newval = nLabel;
	IppiSize Roi = { rc.width, rc.height };
	m_ImgFilter16u.create(Roi.height, Roi.width, CV_16UC1);
	matblobimg.create(Roi.height, Roi.width, CV_8UC1);
	ippiCopy_16u_C1R((Ipp16u*)m_pTmpImg.data + rc.y * m_pTmpImg.cols + rc.x, m_pTmpImg.step,
		(Ipp16u*)m_ImgFilter16u.data, m_ImgFilter16u.step, Roi);
	ippiThreshold_LTValGTVal_16u_C1IR((Ipp16u*)m_ImgFilter16u.data, m_ImgFilter16u.step, Roi, newval, 0, newval, 0);
	ippiConvert_16u8u_C1R((Ipp16u*)m_ImgFilter16u.data, m_ImgFilter16u.step, matblobimg.data, matblobimg.step, Roi);

}

int CKxBlobAnalyse::SelectMaxRegionByDots(cv::InputArray SrcImg, cv::OutputArray DstImg)
{
	KxCallStatus hCall;
	return SelectMaxRegionByDots(SrcImg, DstImg, hCall);
}

int CKxBlobAnalyse::SelectMaxRegionByDots(cv::InputArray SrcImg, cv::OutputArray DstImg, KxCallStatus& hCall)
{
	Clear(); //clear the data struct

	Mat matSrcImg = SrcImg.getMat();
	Mat matDstImg = DstImg.getMat();

	matDstImg.create(matSrcImg.rows, matSrcImg.cols, CV_8UC1);
	//DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize Roi = { matSrcImg.cols, matSrcImg.rows };
	ippiCopy_8u_C1R(matSrcImg.data, matSrcImg.step, matDstImg.data, matDstImg.step, Roi);

	hCall.Clear();
	IppStatus status;
	m_nOpenSize = 1;
	
	if (m_BufferImg.data == NULL || Roi.width != m_Img.cols || Roi.height != m_Img.rows)
	{
		int  nBufferSize;
		status = ippiLabelMarkersGetBufferSize_16u_C1R(Roi, &nBufferSize);

		if (check_sts(status, "SelectMaxRegionByDots_ippiLabelMarkersGetBufferSize", hCall))
		{
			return 0;
		}

		//m_BufferImg.Init(nBufferSize, 1);
		m_BufferImg.create(1, nBufferSize, CV_8UC1);
	}
	//copy a Img
	//m_Img.Init(Roi.width, Roi.height);
	m_Img.create(Roi.height, Roi.width, CV_8UC1);
	status = ippiCopy_8u_C1R(matSrcImg.data, matSrcImg.step, m_Img.data, m_Img.step, Roi);

	if (check_sts(status, "SelectMaxRegionByDots_ippiCopy", hCall))
	{
		return 0;
	}

	//merge some connections
	m_PreImg.create(Roi.height, Roi.width, CV_8UC1);
	KxCallStatus hTempCall;
	hTempCall.Clear();
	m_nOpenSize = 1;
	MergeSomeConnections(matSrcImg.data, Roi.width, Roi.height, matSrcImg.step, m_PreImg.data, m_PreImg.step, hTempCall);

	if (check_sts(hTempCall, "SelectMaxRegionByDots_MergeSomeConnections", hCall))
	{
		return 0;
	}

	//m_pTmpImg.Init(Roi.width, Roi.height);
	//m_pImg16u.Init(Roi.width, Roi.height);

	m_pTmpImg.create(Roi.height, Roi.width, CV_8UC1);
	m_pImg16u.create(Roi.height, Roi.width, CV_8UC1);

	status = ippiConvert_8u16u_C1R(m_PreImg.data, m_PreImg.step, (Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi);

	if (check_sts(status, "SelectMaxRegionByDots_ippiConvert", hCall))
	{
		return 0;
	}

	//first,label the connections components
	int nCount = 0;
	status = ippiLabelMarkers_16u_C1IR((Ipp16u*)m_pTmpImg.data, m_pTmpImg.step, Roi, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1), &nCount, m_BufferImg.data);

	if (check_sts(status, "SelectMaxRegionByDots_ippiLabelMarkers", hCall))
	{
		return 0;
	}

	if (nCount == 0)
	{
		return 1;
	}

	m_nCount = nCount;
	//m_pBlobInfo = new SingleBlobInfo[nCount];

	return 1;
}

int CKxBlobAnalyse::SelectRegion(cv::InputArray SrcImg, cv::OutputArray DstImg, std::string szType, int nMinRange, int nMaxRange)
{
	KxCallStatus hCall;
	return SelectRegion(SrcImg, DstImg, szType, nMinRange, nMaxRange, hCall);
}

int CKxBlobAnalyse::SelectRegion(cv::InputArray SrcImg, cv::OutputArray DstImg, std::string szType, int nMinRange, int nMaxRange, KxCallStatus& hCall)
{// 2019.07.02 怀疑缺失
	/*
	Clear(); //clear the data struct


	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight);
	IppiSize Roi = { SrcImg.nWidth, SrcImg.nHeight };
	ippiCopy_8u_C1R(SrcImg.data, SrcImg.nPitch, DstImg.data, DstImg.nPitch, Roi);

	hCall.Clear();
	IppStatus status;
	m_nOpenSize = 1;

	if (m_BufferImg.data == NULL || Roi.width != m_Img.nWidth || Roi.height != m_Img.nHeight)
	{
		int  nBufferSize;
		status = ippiLabelMarkersGetBufferSize_16u_C1R(Roi, &nBufferSize);

		if (check_sts(status, "SelectMaxRegionByDots_ippiLabelMarkersGetBufferSize", hCall))
		{
			return 0;
		}

		m_BufferImg.Init(nBufferSize, 1);
	}
	//copy a Img
	m_Img.Init(Roi.width, Roi.height);
	status = ippiCopy_8u_C1R(SrcImg.data, SrcImg.nPitch, m_Img.data, m_Img.nPitch, Roi);

	if (check_sts(status, "SelectMaxRegionByDots_ippiCopy", hCall))
	{
		return 0;
	}

	//merge some connections
	m_PreImg.Init(Roi.width, Roi.height);
	KxCallStatus hTempCall;
	hTempCall.Clear();
	m_nOpenSize = 1;
	MergeSomeConnections(SrcImg.data, Roi.width, Roi.height, SrcImg.nPitch, m_PreImg.data, m_PreImg.nPitch, hTempCall);

	if (check_sts(hTempCall, "SelectMaxRegionByDots_MergeSomeConnections", hCall))
	{
		return 0;
	}


	m_pTmpImg.Init(Roi.width, Roi.height);
	m_pImg16u.Init(Roi.width, Roi.height);

	status = ippiConvert_8u16u_C1R(m_PreImg.data, m_PreImg.step, m_pTmpImg.data, m_pTmpImg.nPitch, Roi);

	if (check_sts(status, "SelectMaxRegionByDots_ippiConvert", hCall))
	{
		return 0;
	}

	//first,label the connections components
	int nCount = 0;
	status = ippiLabelMarkers_16u_C1IR(m_pTmpImg.data, m_pTmpImg.nPitch, Roi, _Min_Lable, _Min_Lable + _MAX_Lable_Count,
		(m_nConnectType == _USE8 ? ippiNormInf : ippiNormL1), &nCount, m_BufferImg.data);

	if (check_sts(status, "SelectMaxRegionByDots_ippiLabelMarkers", hCall))
	{
		return 0;
	}

	if (nCount == 0)
	{
		return 0;
	}

	m_nCount = nCount;
	//m_pBlobInfo = new SingleBlobInfo[nCount];


	*/
	return 1;
}

int  CKxBlobAnalyse::SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures)
{
	KxCallStatus hCall;
	return SortByEnergy(m_nCount, nOpenComputeAdanceFeatures, hCall);
}

int  CKxBlobAnalyse::SortByEnergy(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pEnergys = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pEnergys[i] = m_pBlobInfo[i].m_nEnergy;
	}
	status = ippsSortIndexDescend_64f_I(pEnergys, pIndex, m_nCount);

	if (check_sts(status, "SortByEnergy_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortByEnergy_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{			
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pEnergys;
	delete[]pIndex;

	return 1;

}

int  CKxBlobAnalyse::SortBySize(int nSortCount, int nOpenComputeAdanceFeatures)
{
	KxCallStatus hCall;
	return SortBySize(m_nCount, nOpenComputeAdanceFeatures, hCall);
}

int  CKxBlobAnalyse::SortBySize(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pEnergys = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pEnergys[i] = m_pBlobInfo[i].m_nSize;
	}
	status = ippsSortIndexDescend_64f_I(pEnergys, pIndex, m_nCount);

	if (check_sts(status, "SortBySize_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortBySize_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pEnergys;
	delete[]pIndex;

	return 1;

}

int  CKxBlobAnalyse::SortByDot(int nSortCount, int nOpenComputeAdanceFeatures, KxCallStatus& hCall)
{
	IppStatus  status;
	KxCallStatus hTmpCall;
	hCall.Clear();
	hTmpCall.Clear();

	nSortCount = gMin(nSortCount, m_nCountBlob);
	if (nSortCount == 0)
	{
		return 1;
	}

	ClearSortInfo();
	Ipp64f* pDots = new Ipp64f[m_nCount];
	int* pIndex = new int[m_nCount];
	for (int i = 0; i < m_nCount; i++)
	{
		pDots[i] = m_pBlobInfo[i].m_nDots;
	}
	status = ippsSortIndexDescend_64f_I(pDots, pIndex, m_nCount);

	if (check_sts(status, "SortByDot_ippsSortIndexDescend", hCall))
	{
		return 0;
	}


	m_pSortBlobInfo = new SingleBlobInfo[m_nCountBlob];
	status = ippsZero_8u((Ipp8u*)m_pSortBlobInfo, m_nCountBlob*sizeof(SingleBlobInfo));

	if (check_sts(status, "SortByDot_ippsZero", hCall))
	{
		return 0;
	}


	for (int i = 0; i < nSortCount; i++)
	{
		m_pSortBlobInfo[i].m_nDots = m_pBlobInfo[pIndex[i]].m_nDots;
		m_pSortBlobInfo[i].m_nLabel = m_pBlobInfo[pIndex[i]].m_nLabel;
		m_pSortBlobInfo[i].m_rc = m_pBlobInfo[pIndex[i]].m_rc;
		m_pSortBlobInfo[i].m_nEnergy = m_pBlobInfo[pIndex[i]].m_nEnergy;
		m_pSortBlobInfo[i].m_nSize = m_pBlobInfo[pIndex[i]].m_nSize;
		m_pSortBlobInfo[i].m_fSumEnergy = m_nEnergySum;
		//最小外界矩形特征
		if (nOpenComputeAdanceFeatures)
		{
			ComputeBlobMinRectangle(m_pSortBlobInfo[i]);
		}

	}


	delete[]pDots;
	delete[]pIndex;

	return 1;

}

int CKxBlobAnalyse::ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo)
{
	KxCallStatus hCall;
	return ComputeBlobMinRectangle(hSortBlobInfo, hCall);
}

int CKxBlobAnalyse::ComputeBlobMinRectangle(SingleBlobInfo& hSortBlobInfo, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	Ipp16u newval = hSortBlobInfo.m_nLabel;
	IppiSize Roi = { hSortBlobInfo.m_rc.width, hSortBlobInfo.m_rc.height };
	
	IppStatus status = ippiCopy_16u_C1R((Ipp16u*)m_pTmpImg.data + hSortBlobInfo.m_rc.y * m_pTmpImg.cols + hSortBlobInfo.m_rc.x, m_pTmpImg.step,
		(Ipp16u*)m_pImg16u.data, m_pImg16u.step,  Roi);

	if (check_sts(status, "ComputeBlobMinRectangle_ippiCopy_first", hCall))
	{
		return 0;
	}

	status = ippiThreshold_LTValGTVal_16u_C1IR((Ipp16u*)m_pImg16u.data, m_pImg16u.step, Roi, newval, 0, newval, 0);

	if (check_sts(status, "ComputeBlobMinRectangle_ippiThreshold_LTValGTVal", hCall))
	{
		return 0;
	}

	status = ippiConvert_16u8u_C1R((Ipp16u*)m_pImg16u.data, m_pImg16u.step, m_PreImg.data, m_PreImg.step, Roi);


	m_hKxMinRect.Check(m_PreImg.data, Roi.width, Roi.height, m_PreImg.step);


	hSortBlobInfo.m_nMinRectHeight = m_hKxMinRect.GetResult().m_nMinRectHeight;
	hSortBlobInfo.m_nMinRectWidth = m_hKxMinRect.GetResult().m_nMinRectWidth;

	hSortBlobInfo.m_fRatio = m_hKxMinRect.GetResult().m_fRatio;

	return 1;
}


/*
	author:			HYH
	date:			2019.07.03
	description:	
*/

void CKxBlobAnalyse::BlobParallel(cv::InputArray SrcImg, int nSortByMode, int nMaxSortDots, int nMergeSize)
{
	if (SrcImg.kind() == cv::_InputArray::MAT)//CPU
	{
		cv::Mat matSrcImg = SrcImg.getMat();
		if (matSrcImg.rows > _PARALLEL_SEGMENTATION_H)
		{
			//if 
		}

	}
	else if (SrcImg.kind() == cv::_InputArray::CUDA_GPU_MAT)//GPU
	{

	}
	else
	{
		// 暂不支持其它类型
	}

}
