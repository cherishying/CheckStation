
#include "KxSearchCard.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/tick_count.h"
#include "tbb/scalable_allocator.h"
#include "tbb/blocked_range2d.h"
using namespace tbb;

CKxSearchCards::CKxSearchCards()
{
	m_nWarpWidth = 1400;
	m_nWarpHeight = 900;
	m_nExtendLen = 0;
	m_nBeltHorizStart = 500;
	m_nBeltHorizEnd = 1300;
	m_nSampleStep = 15;
}

CKxSearchCards::~CKxSearchCards()
{
}

int CKxSearchCards::Read(unsigned char *&pt)
{
	ippsCopy_8u(pt, (unsigned char*)&m_Parameter, sizeof(Parameter));
	pt += sizeof(Parameter);
	return 1;
}

bool CKxSearchCards::ReadVesion1Para(FILE* fp)    //读取版本1参数
{
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


bool CKxSearchCards::Read(FILE* fp)
{
	if (fread(m_Parameter.m_szVersion, sizeof(m_Parameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_Parameter.m_szVersion, "SearchCard1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}
}

int CKxSearchCards::Load(const char* lpszFile)
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



bool CKxSearchCards::FindMaxGradientPosition(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, kxPoint<int> ptStartPoint,  int nSearchDir, int nDir, int nGradThresh,
	int* pGradValue, kxPoint<int>& ptMaxGrad, int& pMaxGradValue)
{
	//int nSearchLen = nLen;
	int nSearchExtend = 5;

	ptMaxGrad.setup(0, 0);
	bool bFindPoint = false;

	if (nSearchDir == _SEARCH_VER_DIR)
	{
		ptMaxGrad.x = ptStartPoint.x;

		int* nGridValues = new int[nHeight];
		memset(nGridValues, 0, sizeof(int)*nHeight);
		int nTop, nBottom;
		unsigned char* pSrcData;
		if (nDir == _UP_TO_BOTTOM)
		{
			nTop = gMax(4, ptStartPoint.y - 2);
			nBottom = nHeight-3;
		}
		else
		{
			nTop = 4;
			nBottom = gMin(ptStartPoint.y + 2, nHeight - 5);
		}

		int nTwoLinePitch = 2 * nPitch;
		pMaxGradValue = 0;
		//////////////////////////////////////////////////////////////////////////
		int nStart, nEnd, nStep;
		if (nDir == _UP_TO_BOTTOM)
		{
			nStart = nTop;
			nEnd = nBottom;
			nStep = 1;
		}
		else
		{
			nStart = nBottom;
			nEnd = nTop;
			nStep = -1;
		}

		if (nChannel == _Type_G24)
		{
			pSrcData = (unsigned char*)buf + (ptStartPoint.x - nSearchExtend) * 3 + nStart * nPitch;
		}
		else
		{
			pSrcData = (unsigned char*)buf + (ptStartPoint.x - nSearchExtend) + nStart * nPitch;
		}

		int y = nStart;
		int nCurValue, nLastValue;
		nLastValue = -1;
		while (y != nEnd)
		{
			unsigned char * pData = pSrcData;
			//TRACE("%d---%d\n",y, *pData);
			int nGridTemp = 0;
			for (int x = ptStartPoint.x - nSearchExtend; x <= ptStartPoint.x + nSearchExtend; x++)
			{
				if (nChannel == _Type_G24)
				{
					nGridTemp = abs(int(*(pData - nTwoLinePitch)) - int(*(pData + nTwoLinePitch)));
					nGridTemp = gMax(nGridTemp, abs(int(*(pData + 1 - nTwoLinePitch)) - int(*(pData + 1 + nTwoLinePitch))));
					nGridTemp = gMax(nGridTemp, abs(int(*(pData + 2 - nTwoLinePitch)) - int(*(pData + 2 + nTwoLinePitch))));
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
			nGridValues[y] = nGridValues[y] / (2 * nSearchExtend + 1);

			if (nGridValues[y] > nGradThresh)
			{
				nCurValue = nGridValues[y];
				if (nCurValue >= nLastValue)
				{
					nLastValue = nCurValue;
				}
				else
				{
					ptMaxGrad.y = y - 1*nStep;
					pMaxGradValue = nGridValues[y - 1 * nStep];
					bFindPoint = true;
					break;
				}
			}
			else
			{
				if (nLastValue != -1)
				{
					ptMaxGrad.y = y - 1 * nStep;
					pMaxGradValue = nGridValues[y - 1 * nStep];
					bFindPoint = true;
					break;
				}
			}

			pSrcData += nPitch*nStep;

			y += nStep;
		}


		delete[]nGridValues;


	}
	return bFindPoint;

}


bool CKxSearchCards::Check(const kxCImageBuf& SrcImg, bool bModel)
{
	KxCallStatus hCall;
	return Check(SrcImg, bModel, hCall);
}

bool CKxSearchCards::Check(const kxCImageBuf& SrcImg, bool bModel, KxCallStatus& hCall)
{
	//加黑白工位特殊处理，由于黑白工位一张图内可能最多出现3张卡
	//所以先确定当前卡的位置
	if (SrcImg.nChannel == _Type_G8)
	{
		//首先用大津发确定阈值
		//Ipp8u Gate;
		//IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		//ippiComputeThreshold_Otsu_8u_C1R(SrcImg.buf, SrcImg.nPitch, roiSize, &Gate);
		//m_hBaseFun.KxThreshImage(SrcImg, m_BinaryImg, Gate, 255);
		////然后对二值图进行水平方向投影
		//int nLen = SrcImg.nHeight;
		//Ipp32f* fProject = new Ipp32f[nLen];
		//m_hBaseFun.KxProjectImage(m_BinaryImg, _Horizontal_Project_Dir, fProject);
		////搜索最大面积
		//int nCardVerStart = 0, nCardVerEnd = SrcImg.nHeight-1, nMaxArea = -1;
		//int nStart = 0, nArea = 0;
		//bool bFirst = false;
		//for (int i = 0; i < nLen; i++)
		//{
		//	if (fProject[i] > 0)
		//	{
		//		if (!bFirst)
		//		{
		//			bFirst = true;
		//			nStart = i;
		//		}
		//		nArea += int(fProject[i]);
		//	}
		//	else
		//	{
		//		if (bFirst)
		//		{
		//			if (nArea > nMaxArea)
		//			{
		//				nMaxArea = nArea;
		//				nCardVerEnd = i;
		//				nCardVerStart = nStart;
		//			}
		//			bFirst = false;
		//			nArea = 0;
		//		}
		//	}

		//}
		////if (nMaxArea == -1)
		////{
		////	return false;  //表示无卡
		////}

		//delete[]fProject;
		//const int nExtend = 20;
		//m_nCardVerStart = gMax(0, nCardVerStart - nExtend);
		//m_nCardVerEnd = gMin(nCardVerEnd + nExtend, SrcImg.nHeight - 1);
		m_nCardVerStart = 2;
		m_nCardVerEnd = SrcImg.nHeight - 3;
	}
	else
	{
		m_nCardVerStart = 2;
		m_nCardVerEnd = SrcImg.nHeight - 3;
	}

	return Check(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel, bModel, hCall);


}

//use to test a search point algorithm
int CKxSearchCards::MarkPoint(kxCImageBuf& DstImg, kxPoint<int> pts[], int N)
{
	for (int i = 0; i < N; i++)
	{
		int x = pts[i].x;
		int y = pts[i].y;
		for (int k = x - 5; k <= x + 5; k++)
		{
			if (y >0 && y < DstImg.nHeight && k > 0 && k < DstImg.nWidth)
			{
				if (DstImg.nChannel == _Type_G24)
				{
					DstImg.buf[y*DstImg.nPitch + 3 * k] = 0;
					DstImg.buf[y*DstImg.nPitch + 3 * k + 1] = 0;
					DstImg.buf[y*DstImg.nPitch + 3 * k + 2] = 255;
				}
				else
				{
					DstImg.buf[y*DstImg.nPitch + k] = 255;
				}
			}
		}

		for (int k = y - 5; k <= y + 5; k++)
		{
			if (k >0 && k < DstImg.nHeight && x > 0 && x < DstImg.nWidth)
			{
				if (DstImg.nChannel == _Type_G24)
				{
					DstImg.buf[k*DstImg.nPitch + 3 * x] = 0;
					DstImg.buf[k*DstImg.nPitch + 3 * x + 1] = 0;
					DstImg.buf[k*DstImg.nPitch + 3 * x + 2] = 255;
				}
				else
				{
					DstImg.buf[k*DstImg.nPitch + x] = 255;
				}
			}

		}

	}
	return 1;

}


void CKxSearchCards::combine(int* arr, int start, int* result, int count, int num, int& nTotal, int** Result)
{
	for (int i = start; i >= count; i--)
	{
		result[count - 1] = i - 1;
		if (count > 1)
		{
			combine(arr, i - 1, result, count - 1, num, nTotal, Result);
		}
		else
		{
			int n = 0;
			for (int j = num - 1; j >= 0; j--)
			{
				Result[nTotal][n++] = arr[result[j]];
			}
			nTotal++;
		}
	}
}

//y = ax + b
//ax + by + c = 0 ,约束a + b + c = 1000
bool CKxSearchCards::FiltLine(Ipp32f* pX, Ipp32f* pY, int n, float* pCoeff)
{
	Ipp32f pSumX, pSumY, pSumXY, pSumXX;
	ippsSum_32f(pX, n, &pSumX, ippAlgHintFast);
	ippsSum_32f(pY, n, &pSumY, ippAlgHintFast);
	ippsDotProd_32f(pX, pY, n, &pSumXY);
	ippsDotProd_32f(pX, pX, n, &pSumXX);
	if (abs(pSumX*pSumX - n*pSumXX) < 1e-8)
	{
		float fAverageX = pSumX / n;
		if (abs(fAverageX - 1) < 1e-8)
		{
			pCoeff[0] = -1000.0f / (fAverageX - 1);
			pCoeff[1] = 0;
			pCoeff[2] = 1000.0f * fAverageX / (fAverageX - 1);
		}
		else
		{
			pCoeff[0] = 1;
			pCoeff[1] = 0;
			pCoeff[2] = -fAverageX;
		}

		return true;
	}
	else
	{
		//float a, b;
		//b = (pSumXY * pSumX - pSumY * pSumXX) / (pSumX*pSumX - n*pSumXX);
		//a = (pSumY * pSumX - n*pSumXY) / (pSumX*pSumX - n*pSumXX);
		float k = (pSumX*pSumX - n*pSumXX)*1000.f/((pSumXY * pSumX - pSumY * pSumXX) + (pSumY * pSumX - n*pSumXY) - (pSumX*pSumX - n*pSumXX));

		pCoeff[0] = (pSumY * pSumX - n*pSumXY)*1000.f / ((pSumXY * pSumX - pSumY * pSumXX) + (pSumY * pSumX - n*pSumXY) - (pSumX*pSumX - n*pSumXX));
		pCoeff[1] = -k;
		pCoeff[2] = (pSumXY * pSumX - pSumY * pSumXX) *1000.f / ((pSumXY * pSumX - pSumY * pSumXX) + (pSumY * pSumX - n*pSumXY) - (pSumX*pSumX - n*pSumXX));
		return true;
	}



}

//ax + by + c = 0
int  CKxSearchCards::FiltLine2D(kxPoint<int>* pts, int n, float* pCoeff)
{
	Ipp32f* pX = new Ipp32f[n];
	Ipp32f* pY = new Ipp32f[n];

	for (int i = 0; i < n; i++)
	{
		pX[i] = (Ipp32f)pts[i].x;
		pY[i] = (Ipp32f)pts[i].y;
	}
	//int nStatus = m_hBaseFun.KxFitLine(pX, pY, n, pCoeff);
	int nStatus = FiltLine(pX, pY, n, pCoeff);

	delete[]pX;
	delete[]pY;

	return nStatus;
}



float CKxSearchCards::FitLineByInteration(kxPoint<int>* pts, int nLen, int nSampleCount, int nFitLineDots, float* fLineCoeff)
{
	tick_count tbb_start, tbb_end;

	tbb_start = tick_count::now();
	//数据平均采样
	int nDots = nSampleCount;
	if (nLen < nSampleCount)
	{
		nDots = nLen;
	}

	int nStep = nLen / nDots;

	kxPoint<int>* ptFit = new kxPoint<int>[nDots];
	for (int i = 0; i < nDots; i++)
	{
		ptFit[i].x = pts[i*nStep].x;
		ptFit[i].y = pts[i*nStep].y;
	}

	int nIterCount = 0;
	kxPoint<int>* pFitArray = new kxPoint<int>[nFitLineDots];

	int nMaxVoteCount = INT_MIN;
	float fInliersPercentage = 0.0;
	float fMaxSum = 0.0;

	int N = nDots;
	int m = nFitLineDots;
	int* arr = new int[N];
	for (int i = 0; i < N; i++)
	{
		arr[i] = i;
	}
	int nFenmu = 1;
	int nFenZi = 1;
	for (int i = 0; i < m; i++)
	{
		nFenmu *= (N - i);
		nFenZi *= (i + 1);
	}
	int nTotal = nFenmu / nFenZi;

	int** result = new int*[nTotal];
	for (int i = 0; i < nTotal; i++)
	{
		result[i] = new int[m];
	}
	int* pTmp = new int[m];
	int nTotalCount = 0;
	combine(arr, N, pTmp, m, m, nTotalCount, result);

	while ((nIterCount < nTotal) /*&& (fInliersPercentage < 0.9)*/)
	{
		for (int i = 0; i < nFitLineDots; i++)
		{
			int nIndex = result[nIterCount][i];
			pFitArray[i] = ptFit[nIndex];
		}
		int nVoteCount = 0;
		float fSum = 0.0f;
		int nStatus = 1;

		float fCoeff[3];
		memset(fCoeff, 0, 3 * sizeof(float));

		nStatus = FiltLine2D(pFitArray, nFitLineDots, fCoeff);

		if (nStatus)
		{
			for (int k = 0; k < nDots; k++)
			{
				if (abs(fCoeff[0] * ptFit[k].x + ptFit[k].y * fCoeff[1] + fCoeff[2]) < 2*sqrt(fCoeff[0] * fCoeff[0] + fCoeff[1] * fCoeff[1]))
				{
					nVoteCount++;
				}
				fSum += abs(fCoeff[0] * ptFit[k].x + ptFit[k].y * fCoeff[1] + fCoeff[2]) / sqrt(fCoeff[0] * fCoeff[0] + fCoeff[1] * fCoeff[1]);
			}
		}
		if (nVoteCount > nMaxVoteCount)
		{
			nMaxVoteCount = nVoteCount;
			fInliersPercentage = (float)nVoteCount / nDots;
			for (int i = 0; i < 3; i++)
			{
				fLineCoeff[i] = fCoeff[i];
			}
			fMaxSum = fSum;
		}
		else
		{
			if ((nVoteCount == nMaxVoteCount) && (fSum < fMaxSum)) //当投票数相等时并且总距离和小
			{
				nMaxVoteCount = nVoteCount;
				fInliersPercentage = (float)nVoteCount / nDots;
				for (int i = 0; i < 3; i++)
				{
					fLineCoeff[i] = fCoeff[i];
				}
				fMaxSum = fSum;
			}
		}
		nIterCount++;
	}


	delete[]ptFit;
	delete[]pFitArray;
	delete[]arr;
	delete[]pTmp;
	for (int i = 0; i < nTotal; i++)
	{
		delete[]result[i];
		result[i] = NULL;
	}
	result = NULL;


	return fInliersPercentage;
}



bool CKxSearchCards::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, bool bModel, KxCallStatus& hCall)
{
	hCall.Clear();
	//task_scheduler_init init1(-1);
	//tick_count tbb_start, tbb_end;
	//tbb_start = tick_count::now();
	if (nWidth <= 0 || nHeight <= 0 || nPitch <= 0)
	{
		IppStatus status = IppStatus(kxSearchCardImageSizeErr);
		if (check_sts(status, "图像大小错误", hCall))
		{
			return false;
		}
	}
	//bool bFlag;
	//get the card's location, as anchor and angle
	//First, search the vertical direction
	m_hResult.Clear();

	int nVerSearchDir[2] = { _UP_TO_BOTTOM, _BOTTOM_TO_UP };

	kxPoint<int> ptVerBest[2][_HORIZONSEARCH_NUM];

	int nSearchVerPos[2];
	nSearchVerPos[0] = m_nCardVerStart;
	nSearchVerPos[1] = m_nCardVerEnd ;

	//首先搜索卡的上下两边
	Ipp32f pUpDownLineCoeff[2][3];
	for (int k = 0; k < 2; k++)
	{
		//搜上边
		kxPoint<int> nVerStartPoint[_HORIZONSEARCH_NUM];
		for (int i = 0; i < _HORIZONSEARCH_NUM / 2; i++)
		{
			nVerStartPoint[i].x = m_nBeltHorizStart - 10 - m_nSampleStep*i;
			nVerStartPoint[i].y = nSearchVerPos[k];
		}
		for (int i = _HORIZONSEARCH_NUM / 2; i < _HORIZONSEARCH_NUM; i++)
		{
			nVerStartPoint[i].x = m_nBeltHorizEnd + 10 + m_nSampleStep*i;
			nVerStartPoint[i].y = nSearchVerPos[k];
		}

		bool bFlag[_HORIZONSEARCH_NUM];
		parallel_for(blocked_range<int>(0, _HORIZONSEARCH_NUM),
			[&](const blocked_range<int>& range)
		{
			for (int i = range.begin(); i < range.end(); i++)
			{
				bFlag[i] = FindMaxGradientPosition(buf, nWidth, nHeight, nPitch, nChannel, nVerStartPoint[i], _SEARCH_VER_DIR, nVerSearchDir[k], m_Parameter.m_nVerThresh,
					m_hResult.m_nVerGradient[k][i], m_hResult.m_ptVerBestPosition[k][i], m_hResult.m_nVerMaxGradient[k][i]);
				ptVerBest[k][i] = m_hResult.m_ptVerBestPosition[k][i];
			}
		}, auto_partitioner());

		int nDirFactor = (nVerSearchDir[k] == _UP_TO_BOTTOM) ? -1 : 1;
		//根据上下两条边确定两条直线
		//Ipp32f  pX[_HORIZONSEARCH_NUM], pY[_HORIZONSEARCH_NUM];
		int nSum = 0;
		for (int i = 0; i < _HORIZONSEARCH_NUM; i++)
		{
			nSum += (bFlag[i] ? 1 : 0);
			ptVerBest[k][i].x = ptVerBest[k][i].x;
			ptVerBest[k][i].y = ptVerBest[k][i].y + m_nExtendLen * nDirFactor;
		}
		if (nSum < 4)
		{
			IppStatus status = IppStatus(kxSearchCardSearchDotNotEnough);
			if (check_sts(status, "搜点不足", hCall))
			{
				return false;
			}
		}
		// ax + by + c = 0
		FitLineByInteration(ptVerBest[k], _HORIZONSEARCH_NUM, _HORIZONSEARCH_NUM, 3, pUpDownLineCoeff[k]);
	}

	//tbb_end = tick_count::now();
	//printf("Step 1: ----- cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	//tbb_start = tick_count::now();

	//求两条直线分别与卡左右两边的交点
	int nStart, nEnd;
	if (abs(pUpDownLineCoeff[0][1]) < 1e-5 || abs(pUpDownLineCoeff[1][1]) < 1e-5)
	{
		IppStatus status = IppStatus(kxSearchCardLineErr);
		if (check_sts(status, "上下两侧的卡边垂直错误", hCall)) //上下两侧的卡边垂直，异常抛出
		{
			return false;
		}
	}
	else
	{
		float nOffset = 10;
		int nUpActualOffset = int(nOffset * sqrt(pUpDownLineCoeff[0][0] * pUpDownLineCoeff[0][0] + pUpDownLineCoeff[0][1] * pUpDownLineCoeff[0][1]) / abs(pUpDownLineCoeff[0][1]) + 0.5);
		nStart = (int)(-pUpDownLineCoeff[0][2] / pUpDownLineCoeff[0][1]) + nUpActualOffset;
		nStart = gMax(0, nStart);

		int nDownActualOffset = int(nOffset * sqrt(pUpDownLineCoeff[1][0] * pUpDownLineCoeff[1][0] + pUpDownLineCoeff[1][1] * pUpDownLineCoeff[1][1]) / abs(pUpDownLineCoeff[1][1]) + 0.5);
		nEnd = (int)(-pUpDownLineCoeff[1][2] / pUpDownLineCoeff[1][1]) - nDownActualOffset;

		nEnd = gMin(nEnd, nHeight - 1);

	}
	//tbb_end = tick_count::now();
	//printf("Step 2: ----- cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	//tbb_start = tick_count::now();

	int nRange = nEnd - nStart;
	int nStep = nRange / _VERSEARCH_NUM;
	kxPoint<int> pts[2][_VERSEARCH_NUM];
	Ipp32f pLeftRightLineCoeff[2][3];

	//////////////////////////////////////////////////////////////////////////
	parallel_for(blocked_range2d<int>(0, _VERSEARCH_NUM, 0, 2),
		[&](const blocked_range2d<int>& range)
	{
		for (int i = range.rows().begin(); i < range.rows().end(); i++)
		{
			for (int nSide = range.cols().begin(); nSide < range.cols().end(); nSide++)
			{
				int nOffset = nStart + i*nStep;
				//直线方程为 ax+by - b*noffset = 0;
				//采样五步跳法
				int nSetp = 5;
				int nSearchStart, nSearchEnd, nSearchStep, nFlag;
				if (nSide == 0)  //左边
				{
					nSearchStart = 0;
					nSearchEnd = nWidth - 1 - nSetp;
					nSearchStep = 1;
					nFlag = 1;
				}
				else
				{
					nSearchStart = nWidth - 1;
					nSearchEnd = nSetp;
					nSearchStep = -1;
					nFlag = -1;
				}
				//初始化
				int x0 = nSearchEnd;
				int y0 = (int)((pUpDownLineCoeff[0][1] * nOffset - pUpDownLineCoeff[0][0] * x0) / pUpDownLineCoeff[0][1]);
				pts[nSide][i].setup(x0, y0);

				int x = nSearchStart;
				while (x != nSearchEnd)
				{
					//////////////////////////////////////////////////////////////////////////
					int nSumDiff = 0;
					int nS = -2, nE = 2;
					for (int j = nS; j <= nE; j++)
					{
						int y1 = (int)((pUpDownLineCoeff[0][1] * (nOffset + j) - pUpDownLineCoeff[0][0] * x) / pUpDownLineCoeff[0][1]);
						int y2 = (int)((pUpDownLineCoeff[0][1] * (nOffset + j) - pUpDownLineCoeff[0][0] * (x + nSetp*nFlag)) / pUpDownLineCoeff[0][1]);
						y1 = y1 < 0 ? 0 : y1;
						y1 = y1 > nHeight - 1 ? nHeight - 1 : y1;
						y2 = y2 < 0 ? 0 : y2;
						y2 = y2 > nHeight - 1 ? nHeight - 1 : y2;
						int nGridTemp;
						if (nChannel == _Type_G8)
						{
							nGridTemp = abs(buf[y1*nPitch + x] - buf[y2*nPitch + (x + nSetp*nFlag)]);
						}
						if (nChannel == _Type_G24)
						{
							nGridTemp = abs(int(buf[y1*nPitch + 3 * x + 0]) - int(buf[y2*nPitch + 3 * (x + nSetp*nFlag) + 0]));
							nGridTemp = gMax(nGridTemp, abs(int(buf[y1*nPitch + 3 * x + 1]) - int(buf[y2*nPitch + 3 * (x + nSetp*nFlag) + 1])));
							nGridTemp = gMax(nGridTemp, abs(int(buf[y1*nPitch + 3 * x + 2]) - int(buf[y2*nPitch + 3 * (x + nSetp*nFlag) + 2])));
						}
						nSumDiff += nGridTemp;
					}
					int fAverage = nSumDiff / (nE - nS + 1);

					if (fAverage > m_Parameter.m_nHorThresh)
					{
						pts[nSide][i].x = x + nSetp*nFlag - m_nExtendLen*nFlag;
						pts[nSide][i].y = (int)((pUpDownLineCoeff[0][1] * nOffset - pUpDownLineCoeff[0][0] * (x + nSetp*nFlag)) / pUpDownLineCoeff[0][1]);
						break;
					}
					x += nSearchStep;
				}
			}
		}
	}, auto_partitioner());

	//tbb_end = tick_count::now();
	//printf("Step 3: ----- cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	//tbb_start = tick_count::now();

	for (int k = 0; k < 2; k++)
	{
		FitLineByInteration(pts[k], _VERSEARCH_NUM, _VERSEARCH_NUM, 3, pLeftRightLineCoeff[k]);
	}
	//tbb_end = tick_count::now();
	//printf("Step 4: ----- cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	//tbb_start = tick_count::now();

	//求四条直线的交点
	for (int i = 0; i < 2; i++)  //上下两直线
	{
		for (int j = 0; j < 2; j++)  //左右两直线
		{
			float fenmu = pUpDownLineCoeff[i][0] * pLeftRightLineCoeff[j][1] - pUpDownLineCoeff[i][1] * pLeftRightLineCoeff[j][0];
			if (abs(fenmu) < 1e-5)
			{
				m_hResult.m_ptAnchor[2 * i + j].setup(0, 0);
			}
			else
			{
				int x = int((pUpDownLineCoeff[i][1] * pLeftRightLineCoeff[j][2] - pUpDownLineCoeff[i][2] * pLeftRightLineCoeff[j][1]) / fenmu + 0.5);
				int y = int((pUpDownLineCoeff[i][2] * pLeftRightLineCoeff[j][0] - pUpDownLineCoeff[i][0] * pLeftRightLineCoeff[j][2]) / fenmu + 0.5);
				x = x < 0 ? 0 : x;
				x = x > nWidth - 1 ? nWidth - 1 : x;
				y = y < 0 ? 0 : y;
				y = y > nHeight - 1 ? nHeight - 1 : y;
				m_hResult.m_ptAnchor[2 * i + j].x = x;
				m_hResult.m_ptAnchor[2 * i + j].y = y;
			}
		}
	}
	kxPoint<int> tmpPt;
	tmpPt = m_hResult.m_ptAnchor[2];
	m_hResult.m_ptAnchor[2] = m_hResult.m_ptAnchor[3];
	m_hResult.m_ptAnchor[3] = tmpPt;



	//Test
	bool bTest = false;
//	if (bTest)
//	{
//		kxCImageBuf MarkImage;
//		MarkImage.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, true);
//		for (int i = 0; i < 2; i++)
//		{
//			MarkPoint(MarkImage, ptVerBest[i], _HORIZONSEARCH_NUM);
//			MarkPoint(MarkImage, pts[i], _VERSEARCH_NUM);
//		}
//		MarkPoint(MarkImage, m_hResult.m_ptAnchor, 4);
//		static int n = 0;
//		char sz[128];
//		sprintf_s(sz, 128, "D:\\Search\\Mark\\%d.bmp", n++);
//		m_hBaseFun.SaveBMPImage_h(sz, MarkImage);
//	}




	if (bModel)
	{
		IppiSize  siz = { nWidth, nHeight };
		kxCImageBuf CopyImage;

		CopyImage.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, false);

		IppiRect  srcRoi = { 0, 0, nWidth, nHeight };
		double  warpSrcQuad[4][2];

		for (int i = 0; i < 4; i++)
		{
			warpSrcQuad[i][0] = m_hResult.m_ptAnchor[i].x;
			warpSrcQuad[i][1] = m_hResult.m_ptAnchor[i].y;
		}

		int nDstWidth = m_nWarpWidth;
		int nDstHeight = m_nWarpHeight;

		m_ModelImg.Init(nDstWidth, nDstHeight, nChannel);

		IppiRect DstRect = { 0, 0, nDstWidth, nDstHeight };

		double warpDstQuad[4][2] = { { 0, 0 }, { (DstRect.width - 1), 0 }, { (DstRect.width - 1), (DstRect.height - 1) }, { 0, (DstRect.height - 1) } };

		m_hBaseFun.KxWarpPerspectiveLinearQuadImgae(CopyImage, m_ModelImg, warpSrcQuad, warpDstQuad);

	}



	return true;

}

