
#include "KxRotateCards.h"


CKxRotateCards::CKxRotateCards()
{
	m_nWarpWidth = 1400;
	m_nWarpHeight = 900;
	m_nExtendLen = 0;
}

CKxRotateCards::~CKxRotateCards()
{
}

int CKxRotateCards::Write( unsigned char *&pt )
{
	ippsCopy_8u((unsigned char*)&m_Parameter, pt, sizeof(Parameter));
	pt+=sizeof(Parameter);
	return 1;
}

int CKxRotateCards::Read( unsigned char *&pt )
{
	ippsCopy_8u(pt, (unsigned char*)&m_Parameter, sizeof(Parameter));
	pt+=sizeof(Parameter);
	return 1;
}


bool CKxRotateCards::WriteVesion1Para( FILE* fp)    //写入版本1参数
{
	if (fwrite(m_Parameter.m_szVersion, sizeof(m_Parameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}

	if (fwrite(&m_Parameter.m_rcVerSearch, sizeof(kxRect<int>), 2*_SEARCHAREA_NUM, fp) != 2*_SEARCHAREA_NUM)
	{
		return false;
	}
	if (fwrite(&m_Parameter.m_rcHorSearch, sizeof(kxRect<int>), 2*_SEARCHAREA_NUM, fp) != 2*_SEARCHAREA_NUM)
	{
		return false;
	}
	if (fwrite(&m_Parameter.m_nSearchLen, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_Parameter.m_nExtendWidth, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_Parameter.m_nVerThresh, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_Parameter.m_nHorThresh, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	return true;

}

bool CKxRotateCards::Write( FILE* fp )
{
	if (strcmp(m_Parameter.m_szVersion, "RotateCard1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return WriteVesion1Para(fp);
	}
	else
	{
		return false;
	}

}


bool CKxRotateCards::ReadVesion1Para( FILE* fp)    //读取版本1参数
{
	if (fread(&m_Parameter.m_rcVerSearch, sizeof(kxRect<int>), 2*_SEARCHAREA_NUM, fp) != 2*_SEARCHAREA_NUM)
	{
		return false;
	}
	if (fread(&m_Parameter.m_rcHorSearch, sizeof(kxRect<int>), 2*_SEARCHAREA_NUM, fp) != 2*_SEARCHAREA_NUM)
	{
		return false;
	}
	if (fread(&m_Parameter.m_nSearchLen, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_Parameter.m_nExtendWidth, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_Parameter.m_nVerThresh, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_Parameter.m_nHorThresh, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	return true;
}


bool CKxRotateCards::Read( FILE* fp )
{
	if (fread(m_Parameter.m_szVersion, sizeof(m_Parameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_Parameter.m_szVersion, "RotateCard1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}
}


int CKxRotateCards::Load( const char* lpszFile )
{
	FILE*   fp;


#if defined( _WIN32 ) || defined ( _WIN64 )
	if( fopen_s( &fp, lpszFile, "rb" ) != 0 )
	{
		return FALSE;
	}
#else
	fp = fopen(lpszFile, "rb");
	if (fp == NULL)
	{
		return FALSE;
	}

#endif


	int bRet = Read(fp);
	fclose(fp);

	return bRet;
}
int CKxRotateCards::Save( const char* lpszFile )
{
	FILE*   fp;


#if defined( _WIN32 ) || defined ( _WIN64 )
	if( fopen_s( &fp, lpszFile, "wb" ) != 0 )
	{
		return FALSE;
	}
#else
	fp = fopen(lpszFile, "wb");
	if (fp == NULL)
	{
		return FALSE;
	}

#endif


	int bRet = Write(fp);
	fclose(fp);

	return bRet;
}

bool CKxRotateCards::FindMaxGradientPosition(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nImgType, kxPoint<int> ptStartPoint, int nSearchDir, int nDir, int nGradThresh,
											 int* pGradValue, kxPoint<int>& ptMaxGrad, int& pMaxGradValue)
{
	int nSearchLen = m_Parameter.m_nSearchLen;
	int nSearchExtend = m_Parameter.m_nExtendWidth;

	ptMaxGrad.setup(0,0);
	bool bFindPoint = false;

	if (nSearchDir == _SEARCH_VER_DIR)
	{
		ptMaxGrad.x = ptStartPoint.x;

		int* nGridValues = new int[nHeight];
		memset(nGridValues, 0, sizeof(int)*nHeight);
		int nTop = gMax(2, ptStartPoint.y + 2);
		int nBottom = gMin(ptStartPoint.y + nSearchLen - 2, nHeight - 3);
		unsigned char* pSrcData;
		if (nImgType)
		{
			pSrcData = (unsigned char*)buf + (ptStartPoint.x - nSearchExtend) * 3 + nTop * nPitch;
		}
		else
		{
			pSrcData = (unsigned char*)buf + (ptStartPoint.x - nSearchExtend)  + nTop * nPitch;

		}

		int nTwoLinePitch = 2*nPitch;
		pMaxGradValue = 0;
		for(int y = nTop; y < nBottom; y++)
		{
			unsigned char * pData = pSrcData;
			//TRACE("%d---%d\n",y, *pData);


			int nGridTemp = 0;
			for(int x = ptStartPoint.x - nSearchExtend; x <= ptStartPoint.x + nSearchExtend; x++)
			{
				if (nImgType)
				{
					nGridTemp = abs(int(*(pData - nTwoLinePitch)) - int(*(pData + nTwoLinePitch)));
					nGridTemp = gMax(nGridTemp,abs(int(*(pData + 1 - nTwoLinePitch)) - int(*(pData + 1 + nTwoLinePitch))));
					nGridTemp = gMax(nGridTemp,abs(int(*(pData + 2 - nTwoLinePitch)) - int(*(pData + 2 + nTwoLinePitch))));
					nGridValues[y] += nGridTemp; //寻找3个色彩层面上最大梯度值
					pData += 3;
				}
				else
				{
					nGridTemp = abs(int(*(pData - nTwoLinePitch)) - int(*(pData + nTwoLinePitch)));
					nGridValues[y] += nGridTemp; //寻找3个色彩层面上最大梯度值
					pData += 1;

				}

			}
			nGridValues[y] = nGridValues[y] / (2*nSearchExtend+1);

			pSrcData += nPitch;
		}


		memcpy(pGradValue, nGridValues + nTop, sizeof(int)*(nBottom - nTop + 1));

		if(nDir == _UP_TO_BOTTOM)
		{
			int nCurValue, nLastValue;
			nLastValue = -1;
			for(int y = nTop; y < nBottom; y++)
			{
				if(nGridValues[y] > nGradThresh)
				{

					nCurValue = nGridValues[y];
					if (nCurValue >= nLastValue)
					{
						nLastValue = nCurValue;
					}
					else
					{
						ptMaxGrad.y = y-1;
						pMaxGradValue = nGridValues[y-1];
						bFindPoint = true;
						break;
					}

				}
				else
				{
					if (nLastValue != -1)
					{
						ptMaxGrad.y = y-1;
						pMaxGradValue = nGridValues[y-1];
						bFindPoint = true;
						break;
					}
				}
			}

		}
		else
		{
			int nCurValue, nLastValue;
			nLastValue = -1;
			for(int y = nBottom-1; y >= nTop; y--)
			{
				if(nGridValues[y] > nGradThresh)
				{

					nCurValue = nGridValues[y];
					if (nCurValue >= nLastValue)
					{
						nLastValue = nCurValue;
					}
					else
					{
						ptMaxGrad.y = y+1;
						pMaxGradValue = nGridValues[y+1];
						bFindPoint = true;
						break;
					}

				}
				else
				{
					if (nLastValue != -1)
					{
						ptMaxGrad.y = y+1;
						pMaxGradValue = nGridValues[y+1];
						bFindPoint = true;
						break;
					}
				}
			}


		}


		delete []nGridValues;


	}
	else
	{
		ptMaxGrad.y = ptStartPoint.y;

		int* nGridValues = new int[nWidth];
		memset(nGridValues, 0, sizeof(int)*nWidth);
		int nLeft = gMax(2, ptStartPoint.x + 2);
		int nRight = gMin(ptStartPoint.x + nSearchLen - 2, nWidth - 3);
		unsigned char* pSrcData;
		if (nImgType)
		{
			pSrcData = (unsigned char*)buf + (ptStartPoint.y - nSearchExtend) * nPitch + nLeft*3;
		}
		else
		{
			pSrcData = (unsigned char*)buf + (ptStartPoint.y - nSearchExtend) * nPitch + nLeft;

		}



		pMaxGradValue = 0;
		for(int x = nLeft; x < nRight; x++)
		{
			unsigned char * pData = pSrcData;
			/*TRACE("%d---%d\n",x, *pData);*/
			int nGridTemp = 0;
			for(int y = ptStartPoint.y - nSearchExtend; y <= ptStartPoint.y + nSearchExtend; y++)
			{
				if (nImgType)
				{
					nGridTemp = abs(int(*(pData - 2*3)) - int(*(pData + 2*3)));
					nGridTemp = gMax(nGridTemp,abs(int(*(pData + 1 - 2*3)) - int(*(pData + 1 + 2*3))));
					nGridTemp = gMax(nGridTemp,abs(int(*(pData + 2 - 2*3)) - int(*(pData + 2 + 2*3))));
					nGridValues[x] += nGridTemp; //寻找3个色彩层面上最大梯度值
					pData += nPitch;
				}
				else
				{
					nGridTemp = abs(int(*(pData - 2)) - int(*(pData + 2)));
					nGridValues[x] += nGridTemp; //寻找3个色彩层面上最大梯度值
					pData += nPitch;

				}

			}
			nGridValues[x] = nGridValues[x] / (2*nSearchExtend+1);
			if (nImgType)
			{
				pSrcData += 3;
			}
			else
			{
				pSrcData += 1;

			}

		}

		memcpy(pGradValue, nGridValues + nLeft, sizeof(int)*(nRight - nLeft + 1));

		if(nDir == _LEFT_TO_RIGHT)
		{
			int nCurValue, nLastValue;
			nLastValue = -1;

			for(int x = nLeft; x < nRight; x++)
			{
				if(nGridValues[x] > nGradThresh)
				{

					nCurValue = nGridValues[x];
					if (nCurValue >= nLastValue)
					{
						nLastValue = nCurValue;
					}
					else
					{
						ptMaxGrad.x = x-1;
						pMaxGradValue = nGridValues[x-1];
						bFindPoint = true;
						break;
					}

				}
				else
				{
					if (nLastValue != -1)
					{
						ptMaxGrad.x = x-1;
						pMaxGradValue = nGridValues[x-1];
						bFindPoint = true;
						break;
					}
				}
			}


		}
		else
		{
			int nCurValue, nLastValue;
			nLastValue = -1;
			for(int x = nRight-1; x >= nLeft; x--)
			{
				if(nGridValues[x] > nGradThresh)
				{

					nCurValue = nGridValues[x];
					if (nCurValue >= nLastValue)
					{
						nLastValue = nCurValue;
					}
					else
					{
						ptMaxGrad.x = x+1;
						pMaxGradValue = nGridValues[x+1];
						bFindPoint = true;
						break;
					}

				}
				else
				{
					if (nLastValue != -1)
					{
						ptMaxGrad.x = x+1;
						pMaxGradValue = nGridValues[x+1];
						bFindPoint = true;
						break;
					}
				}
			}




		}


		delete []nGridValues;

	}
	return bFindPoint;

}
const double c_bigK = 89.9;   //非常大的斜率
bool CKxRotateCards::GetTwoLinesIntersection(kxPoint<int> pt1[2], kxPoint<int> pt2[2], kxPoint<int>& pt)
{
	if ((pt1[0].x == pt1[1].x)&&(pt2[0].x == pt2[1].x))
	{
		return false;
	}
	else if (pt1[0].x == pt1[1].x)
	{
		pt.x = pt1[0].x;
		double k = double(pt2[0].y - pt2[1].y)/double(pt2[0].x - pt2[1].x);
		double b = pt2[1].y - k*pt2[1].x;
		pt.y = (int)(k*pt.x + b);
		return true;
	}
	else if (pt2[0].x == pt2[1].x)
	{
		pt.x = pt2[0].x;
		double k = double(pt1[0].y - pt1[1].y)/double(pt1[0].x - pt1[1].x);
		double b = pt1[1].y - k*pt1[1].x;
		pt.y = (int)(k*pt.x + b);
		return true;
	}
	else
	{
		double k1 = double(pt1[0].y - pt1[1].y)/double(pt1[0].x - pt1[1].x);
		double k2 = double(pt2[0].y - pt2[1].y)/double(pt2[0].x - pt2[1].x);
		double b1 = pt1[1].y - k1*pt1[1].x;
		double b2 = pt2[1].y - k2*pt2[1].x;
		double beta1 = atan(k1) * 180/PI;
		double beta2 = atan(k2) * 180/PI;
		if (fabs(beta1 - beta2) < 40)
		{
			return false;
		}
		else
		{
			if (fabs(beta1) > c_bigK)
			{
				pt.x = pt1[0].x;
				pt.y = int(k2*pt.x + b2);
			}
			else if (fabs(beta2) > c_bigK)
			{
				pt.x = pt2[0].x;
				pt.y = int(k1*pt.x + b1);
			}
			else
			{
				pt.x = int((b2 - b1)/(k1 - k2));
				pt.y = int(k1*pt.x + b1);
			}
			return true;

		}
	}

}

int CKxRotateCards::GetRotateOffTransform( const double srcQuad[2][2], const double dstQuad[2][2], double coeffs[2][3] )
{
	double pSrc[4*4] = { srcQuad[0][0], -srcQuad[0][1], 1, 0,
		srcQuad[0][1],  srcQuad[0][0], 0, 1,
		srcQuad[1][0], -srcQuad[1][1], 1, 0,
		srcQuad[1][1],  srcQuad[1][0], 0, 1};
	double pBeta[4] = {dstQuad[0][0], dstQuad[0][1], dstQuad[1][0], dstQuad[1][1]};

	lapack_int* ipiv = new lapack_int[4];
	lapack_int info = LAPACKE_dgetrf(LAPACK_ROW_MAJOR,4,4,pSrc,4, ipiv);
	if (info != 0)
	{
		return 0;
	}
	info = LAPACKE_dgetrs(LAPACK_ROW_MAJOR, 'N', 4, 1,pSrc,4,ipiv,pBeta,1);
	if (info != 0)
	{
		return 0;
	}

	delete []ipiv;

	coeffs[0][0] = pBeta[0];
	coeffs[0][1] = -pBeta[1];
	coeffs[0][2] = pBeta[2];
	coeffs[1][0] = pBeta[1];
	coeffs[1][1] = pBeta[0];
	coeffs[1][2] = pBeta[3];

	return 1;
	//Ipp64f pSrc[4*4] = { srcQuad[0][0], -srcQuad[0][1], 1, 0,
	//	srcQuad[0][1],  srcQuad[0][0], 0, 1,
	//	srcQuad[1][0], -srcQuad[1][1], 1, 0,
	//	srcQuad[1][1],  srcQuad[1][0], 0, 1};
	//Ipp64f pBeta[4] = {dstQuad[0][0], dstQuad[0][1], dstQuad[1][0], dstQuad[1][1]};

	//Ipp64f pInvert[4*4];
	//Ipp64f pBuffer[4*4+4];
	//ippmInvert_m_64f(pSrc, 4*sizeof(Ipp64f), sizeof(Ipp64f), pBuffer, pInvert, 4*sizeof(Ipp64f), sizeof(Ipp64f), 4);
	//Ipp64f pResult[4];
	//ippmMul_mv_64f(pInvert, 4*sizeof(Ipp64f), sizeof(Ipp64f), 4, 4, pBeta, sizeof(Ipp64f), 4, pResult, sizeof(Ipp64f));

	//coeffs[0][0] = pResult[0];
	//coeffs[0][1] = -pResult[1];
	//coeffs[0][2] = pResult[2];
	//coeffs[1][0] = pResult[1];
	//coeffs[1][1] = pResult[0];
	//coeffs[1][2] = pResult[3];

	return 1;
}


bool CKxRotateCards::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nImgType, int nMode, bool bModel, int bRoteCard)
{
	if(nWidth <= 0 || nHeight <= 0 || nPitch <= 0)
	{
		return false;
	}
	bool bFlag;
	//get the card's location, as anchor and angle
	//First, search the vertical direction
    m_hResult.Clear();
	int nSearchEdgeCount = 2;
	if (_SEARCH_THREE_EDGE == nMode)
	{
		nSearchEdgeCount = 1;
	}

	kxPoint<int> nVerStartPoint[_SEARCHAREA_NUM];
	int nVerSearchDir[2] = {_UP_TO_BOTTOM, _BOTTOM_TO_UP};
	for (int k = 0; k < nSearchEdgeCount; k++)
	{
		for (int i = 0; i < _SEARCHAREA_NUM; i++)
		{
			nVerStartPoint[i].x = (m_Parameter.m_rcVerSearch[k][i].left + m_Parameter.m_rcVerSearch[k][i].right)/2;
			nVerStartPoint[i].y = m_Parameter.m_rcVerSearch[k][i].top;

			bFlag = FindMaxGradientPosition(buf, nWidth, nHeight, nPitch, nImgType, nVerStartPoint[i], _SEARCH_VER_DIR, nVerSearchDir[k], m_Parameter.m_nVerThresh,
				m_hResult.m_nVerGradient[k][i], m_hResult.m_ptVerBestPosition[k][i], m_hResult.m_nVerMaxGradient[k][i]);
			if (bFlag == false)
			{
				return false;
			}

		}
	}

	//Secondly, search the horizontal direction
	kxPoint<int> nHorStartPoint[_SEARCHAREA_NUM];
	int nHorSearchDir[2] = {_LEFT_TO_RIGHT, _RIGHT_TO_LEFT};
	for (int k = 0; k < 2; k++)
	{
		for (int i = 0; i < _SEARCHAREA_NUM; i++)
		{
			nHorStartPoint[i].x = m_Parameter.m_rcHorSearch[k][i].left;
			nHorStartPoint[i].y = (m_Parameter.m_rcHorSearch[k][i].top + m_Parameter.m_rcHorSearch[k][i].bottom)/2;

			bFlag = FindMaxGradientPosition(buf, nWidth, nHeight, nPitch,nImgType, nHorStartPoint[i], _SEARCH_HOR_DIR, nHorSearchDir[k], m_Parameter.m_nHorThresh,
				m_hResult.m_nHorGradient[k][i], m_hResult.m_ptHorBestPosition[k][i], m_hResult.m_nHorMaxGradient[k][i]);

			if (bFlag == false)
			{
				return false;
			}

		}
	}


	////Thirdly, get the card's anchor and angle
	//m_hResult.m_fAngle =(float(m_hResult.m_ptVerUpBestPosition[1].y - m_hResult.m_ptVerUpBestPosition[0].y)/float(m_hResult.m_ptVerUpBestPosition[1].x - m_hResult.m_ptVerUpBestPosition[0].x))*180/3.1415927;
    kxPoint<int> VerPos[2], HorPos[2];
	for (int i = 0; i < 2; i++)
	{
		VerPos[i].setup(m_hResult.m_ptVerBestPosition[0][i].x, m_hResult.m_ptVerBestPosition[0][i].y - m_nExtendLen);
		HorPos[i].setup(m_hResult.m_ptHorBestPosition[0][i].x - m_nExtendLen, m_hResult.m_ptHorBestPosition[0][i].y);
	}
	bool bStatu = GetTwoLinesIntersection(VerPos, HorPos, m_hResult.m_ptAnchor[0]);
	if (bStatu == false)
	{
		return false;
	}
	for (int i = 0; i < 2; i++)
	{
		VerPos[i].setup(m_hResult.m_ptVerBestPosition[0][i].x, m_hResult.m_ptVerBestPosition[0][i].y - m_nExtendLen);
		HorPos[i].setup(m_hResult.m_ptHorBestPosition[1][i].x + m_nExtendLen, m_hResult.m_ptHorBestPosition[1][i].y);
	}
	bStatu = GetTwoLinesIntersection(VerPos, HorPos, m_hResult.m_ptAnchor[1]);
	if (bStatu == false)
	{
		return false;
	}


	if (_SEARCH_FOUR_EDGE == nMode)
	{
		for (int i = 0; i < 2; i++)
		{
			VerPos[i].setup(m_hResult.m_ptVerBestPosition[1][i].x, m_hResult.m_ptVerBestPosition[1][i].y + m_nExtendLen);
			HorPos[i].setup(m_hResult.m_ptHorBestPosition[1][i].x + m_nExtendLen, m_hResult.m_ptHorBestPosition[1][i].y);
		}
		bStatu = GetTwoLinesIntersection(VerPos, HorPos, m_hResult.m_ptAnchor[2]);
		if (bStatu == false)
		{
			return false;
		}
		for (int i = 0; i < 2; i++)
		{
			VerPos[i].setup(m_hResult.m_ptVerBestPosition[1][i].x, m_hResult.m_ptVerBestPosition[1][i].y + m_nExtendLen);
			HorPos[i].setup(m_hResult.m_ptHorBestPosition[0][i].x - m_nExtendLen, m_hResult.m_ptHorBestPosition[0][i].y);
		}
		bStatu = GetTwoLinesIntersection(VerPos, HorPos, m_hResult.m_ptAnchor[3]);
		if (bStatu == false)
		{
			return false;
		}

	}
	else
	{
		double matchQuad[2][2] = {0};
		double kernQuad[2][2] = {0};

		matchQuad[0][0] = m_hResult.m_ptAnchor[0].x;
		matchQuad[0][1] = m_hResult.m_ptAnchor[0].y;
		matchQuad[1][0] = m_hResult.m_ptAnchor[1].x;
		matchQuad[1][1] = m_hResult.m_ptAnchor[1].y;

		kernQuad[0][0] = 0;
		kernQuad[0][1] = 0;
		kernQuad[1][0] = sqrt((matchQuad[0][0]-matchQuad[1][0])*(matchQuad[0][0]-matchQuad[1][0]) + (matchQuad[0][1] - matchQuad[1][1])*(matchQuad[0][1] - matchQuad[1][1]));
		kernQuad[1][1] = 0;

		double coeffsFine[2][3] ;
		GetRotateOffTransform( kernQuad, matchQuad, coeffsFine );
		double quad[4][2];
		IppiRect kernRect;
		kernRect.x = 0;
		kernRect.y = 0;
		kernRect.width = int(sqrt((matchQuad[0][0]-matchQuad[1][0])*(matchQuad[0][0]-matchQuad[1][0]) + (matchQuad[0][1] - matchQuad[1][1])*(matchQuad[0][1] - matchQuad[1][1])));
		kernRect.height = int(kernRect.width*7/8);

		ippiGetAffineQuad(kernRect, quad, coeffsFine);

		for(int i = 0; i < 4; i++)
		{
            m_hResult.m_ptAnchor[i].x = int(quad[i][0]);
			m_hResult.m_ptAnchor[i].y = int(quad[i][1]);
		}

	}

	if (bModel)
	{
		if (nImgType)
		{
			IppiSize  siz = {nWidth, nHeight};
			kxCImageBuf CopyImage;
			CopyImage.Init(nWidth, nHeight, 3);
			ippiCopy_8u_C3R(buf, nPitch, CopyImage.buf, CopyImage.nPitch, siz);


			IppiRect  srcRoi = { 0, 0, nWidth, nHeight };
			double  warpSrcQuad[4][2];
			if (bRoteCard)
			{
				for (int i = 0; i < 4; i++)
				{
					warpSrcQuad[i][0] = m_hResult.m_ptAnchor[(i+3)%4].x;
					warpSrcQuad[i][1] = m_hResult.m_ptAnchor[(i+3)%4].y;
				}

			}
			else
			{
				for (int i = 0; i < 4; i++)
				{
					warpSrcQuad[i][0] = m_hResult.m_ptAnchor[i].x;
					warpSrcQuad[i][1] = m_hResult.m_ptAnchor[i].y;
				}

			}


			int nDstWidth = m_nWarpWidth;
			int nDstHeight = m_nWarpHeight;

			m_ModelImg.Init(nDstWidth, nDstHeight, 3);

			IppiRect DstRect = {0, 0, nDstWidth, nDstHeight};

			double warpDstQuad[4][2] = {{ 0, 0 }, { (DstRect.width-1), 0 }, { (DstRect.width-1), (DstRect.height-1) }, { 0, (DstRect.height-1) }};


            m_hBaseFun.KxWarpPerspectiveLinearQuadImgae(CopyImage, m_ModelImg, warpSrcQuad, warpDstQuad);
			//ippiWarpPerspectiveQuad_8u_C3R(buf, siz, nPitch, srcRoi, warpSrcQuad,  m_ModelImg.buf, m_ModelImg.nPitch, DstRect, warpDstQuad, IPPI_INTER_LINEAR);
		}
		else
		{
			IppiSize  siz = {nWidth, nHeight};
			IppiRect  srcRoi = { 0, 0, nWidth, nHeight };

			kxCImageBuf CopyImage;
			CopyImage.Init(nWidth, nHeight, 1);
			ippiCopy_8u_C1R(buf, nPitch, CopyImage.buf, CopyImage.nPitch, siz);

			double  warpSrcQuad[4][2];
			if (bRoteCard)
			{
				for (int i = 0; i < 4; i++)
				{
					warpSrcQuad[i][0] = m_hResult.m_ptAnchor[(i+3)%4].x;
					warpSrcQuad[i][1] = m_hResult.m_ptAnchor[(i+3)%4].y;
				}

			}
			else
			{
				for (int i = 0; i < 4; i++)
				{
					warpSrcQuad[i][0] = m_hResult.m_ptAnchor[i].x;
					warpSrcQuad[i][1] = m_hResult.m_ptAnchor[i].y;
				}

			}
			//int nDstWidth = abs(warpSrcQuad[1][0] - warpSrcQuad[0][0]);
			//int nDstHeight = abs(warpSrcQuad[2][1] - warpSrcQuad[0][1]);

			int nDstWidth = m_nWarpWidth;
			int nDstHeight = m_nWarpHeight;

			m_ModelImg.Init(nDstWidth, nDstHeight, 1);

			IppiRect DstRect = {0, 0, nDstWidth, nDstHeight};

			double warpDstQuad[4][2] = {{ 0, 0 }, { (DstRect.width-1), 0 }, { (DstRect.width-1), (DstRect.height-1) }, { 0, (DstRect.height-1) }};

			//ippiWarpPerspectiveQuad_8u_C1R(buf, siz, nPitch, srcRoi, warpSrcQuad,  m_ModelImg.buf, m_ModelImg.nPitch, DstRect, warpDstQuad, IPPI_INTER_LINEAR);

			m_hBaseFun.KxWarpPerspectiveLinearQuadImgae(CopyImage, m_ModelImg, warpSrcQuad, warpDstQuad);
		}

	}

	return true;

}

