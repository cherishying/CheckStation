
#include "KxLightAdjust.h"


CKxLightAdjust::CKxLightAdjust()
{

}

CKxLightAdjust::~CKxLightAdjust()
{

}

bool CKxLightAdjust::ReadVesion1Para( FILE* fp)    //读取版本1参数
{
	if (fread(&m_hParamter.m_nSkipBlackPercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParamter.m_nBlackPercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParamter.m_nSkipWhitePercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParamter.m_nWhitePercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	return true;
}


bool CKxLightAdjust::Read( FILE*  fp)
{
	if (fread(m_hParamter.m_szVersion, sizeof(m_hParamter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_hParamter.m_szVersion, "LightAdjust1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}
}


bool CKxLightAdjust::WriteVesion1Para( FILE* fp)    //写入版本1参数
{
	if (fwrite(m_hParamter.m_szVersion, sizeof(m_hParamter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParamter.m_nSkipBlackPercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParamter.m_nBlackPercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParamter.m_nSkipWhitePercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParamter.m_nWhitePercent, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	return true;
}


bool CKxLightAdjust::Write( FILE*  fp)
{
	if (strcmp(m_hParamter.m_szVersion, "LightAdjust1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return WriteVesion1Para(fp);
	}
	else
	{
		return false;
	}

}


int CKxLightAdjust::GetOneImageHistogramCharacter(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int* pnHist, float* pnBlackAvg, float* pnWhiteAvg, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	IppiSize roiSize = {nWidth, nHeight};
    //ipp9.0 version
	//* get sizes for spec and buffer */
	const int nBins = 256;
	int nLevels[] = {nBins + 1};
	Ipp32f lowerLevel[] = {0};
	Ipp32f upperLevel[] = {255};
    int nSizeHistObj, nSizeBuffer;
	if (check_sts(status = ippiHistogramGetBufferSize(ipp8u, roiSize, nLevels, 1, 1, &nSizeHistObj, &nSizeBuffer),
		   "GetOneImageHistogramCharacter_ippiHistogramGetBufferSize", hCall))
	{
		return 0;
	}
	IppiHistogramSpec* pHistObj = (IppiHistogramSpec* )ippsMalloc_8u(nSizeHistObj);
	Ipp8u* pBuffer = ippsMalloc_8u(nSizeBuffer);
	//initialize spec */
	status = ippiHistogramUniformInit(ipp8u, lowerLevel, upperLevel, nLevels, 1, pHistObj);
	if (check_sts(status, "GetOneImageHistogramCharacter_ippiHistogramUniformInit", hCall))
	{
		ippsFree(pBuffer);
		ippsFree(pHistObj);
		return 0;
	}
	//calculate histogram
	status = ippiHistogram_8u_C1R(buf, nPitch, roiSize, (Ipp32u*)pnHist, pHistObj, pBuffer);
	if (check_sts(status, "GetOneImageHistogramCharacter_ippiHistogram_8u_C1R", hCall))
	{
		ippsFree(pBuffer);
		ippsFree(pHistObj);
		return 0;
	}

	ippsFree(pBuffer);
	ippsFree(pHistObj);

	//Ipp32s nLevelsArray[257];
	//ippiHistogramEven_8u_C1R(buf,nPitch,roiSize,pnHist,&nLevelsArray[0],257,0,256);

	int nBlackPixelsAmount = 0;
	int nBlackPixelsGrayAmount = 0;
	int nSkipBlackAmount = 0;

	int nSkipBlackThresh = (nWidth * nHeight * m_hParamter.m_nSkipBlackPercent ) / 100;
	int nBlackPartPixelThreshold = (nWidth * nHeight * m_hParamter.m_nBlackPercent ) / 100;

	for(int i = 0; i < 256; i++)
	{
		if (nSkipBlackAmount > nSkipBlackThresh)
		{
			if(nBlackPixelsAmount > nBlackPartPixelThreshold)
				break;
			nBlackPixelsGrayAmount += pnHist[i] * i;
			nBlackPixelsAmount += pnHist[i];
		}
		nSkipBlackAmount += pnHist[i];

	}

	if (nBlackPixelsAmount == 0)
	{
		return 0;
	}
	*pnBlackAvg = float(nBlackPixelsGrayAmount*1.0 / nBlackPixelsAmount);

	int nWhitePixelsAmount = 0;
	int nWhitePixelsGrayAmount = 0;
	int nSkipWhiteAmount = 0;

    int nSkipWhiteThresh = (nWidth * nHeight * m_hParamter.m_nSkipWhitePercent ) / 100;
	int nWhitePartPixelThreshold = (nWidth * nHeight * m_hParamter.m_nWhitePercent ) / 100;  //nPixelAmount - (nPixelAmount * nWhitePercent) / 100;

	for(int i = 254; i >= 0; i--)
	{
		if (nSkipWhiteAmount > nSkipWhiteThresh)
		{
			if(nWhitePixelsAmount > nWhitePartPixelThreshold)
				break;
			nWhitePixelsGrayAmount += pnHist[i] * (i+1);
			nWhitePixelsAmount += pnHist[i];
		}
		nWhitePixelsAmount += pnHist[i];

	}

	*pnWhiteAvg = float(nWhitePixelsGrayAmount*1.0 / nWhitePixelsAmount);

	return 1;
}


int CKxLightAdjust::LightAdjust(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
				 unsigned char* pRefer, int nReferPitch, unsigned char* pDst,  int nDstPitch, KxCallStatus& hCall)
{
	if (nChannel == _Type_G8)
	{
		SetModelImage(pRefer, nWidth, nHeight, nReferPitch, nChannel);
		return LightAdjustC1(buf, nWidth, nHeight, nPitch, pDst, nWidth, nHeight, nDstPitch, hCall);
	}
	if (nChannel == _Type_G24)
	{
		SetModelImage(pRefer, nWidth, nHeight, nReferPitch, nChannel);
		return LightAdjustC3(buf, nWidth, nHeight, nPitch, pDst, nWidth, nHeight, nDstPitch, hCall);
	}
	return 1;

}

int CKxLightAdjust::LightAdjust(kxCImageBuf& SrcImg, kxCImageBuf& ReferImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	assert(SrcImg.nChannel == ReferImg.nChannel);
	return LightAdjust(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel,
		                ReferImg.buf, ReferImg.nPitch, DstImg.buf, DstImg.nPitch, hCall);
}


int CKxLightAdjust::LightAdjustC1(const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	int nModelHistgram[256];

	GetOneImageHistogramCharacter(m_ModelImg.buf, m_ModelImg.nWidth, m_ModelImg.nHeight, m_ModelImg.nPitch, nModelHistgram, &m_hResult.m_nModelBlackAvg, &m_hResult.m_nModelWhiteAvg, hCallInfo);
	if (check_sts(hCallInfo, "LightAdjustC1_GetOneImageHistogramCharacter1", hCall))
	{
		return 0;
	}

	GetOneImageHistogramCharacter(buf, nWidth, nHeight, nPitch, &m_hResult.m_nHistogramArray[0], &m_hResult.m_nGrayAverage_BlackPart, &m_hResult.m_nGrayAverage_WhitePart, hCallInfo);
	if (check_sts(hCallInfo, "LightAdjustC1_GetOneImageHistogramCharacter1", hCall))
	{
		return 0;
	}

	Ipp32s pLevel[256];

	float fK = float(m_hResult.m_nModelWhiteAvg - m_hResult.m_nModelBlackAvg) / float(m_hResult.m_nGrayAverage_WhitePart - m_hResult.m_nGrayAverage_BlackPart);


	for(int i = 0; i < 256; i++)
	{
		int nValue = int(fK * float(i - m_hResult.m_nGrayAverage_BlackPart) + m_hResult.m_nModelBlackAvg);
		if(nValue < 0)
			nValue = 0;
		else if(nValue > 255)
			nValue = 255;

		m_hResult.m_nLutArray[i] = nValue;
		pLevel[i] = i;
	}

	IppiSize Roi = {nWidth, nHeight};

    //ippiLUT_8u_C1R(buf, nPitch, pDst, nDstPitch, Roi, m_hResult.m_nLutArray, pLevel, 256);

	//ipp9.0 version
	//* get sizes for spec and buffer */
	int nSpecSize;
	IppiLUT_Spec* pSepc;
	IppStatus status;
	int nLevels[1] = {256};
	const Ipp32s* ppValues[1] = { m_hResult.m_nLutArray };
	const Ipp32s* ppLevels[1] = { pLevel };
	status = ippiLUT_GetSize(ippLinear, ipp8u, ippC1, Roi, nLevels, &nSpecSize);
	if (check_sts(status, "LightAdjustC1_ippiLUT_GetSize", hCall))
	{
		return 0;
	}
	pSepc = (IppiLUT_Spec*)ippsMalloc_8u(nSpecSize);
	status = ippiLUT_Init_8u(ippLinear, ippC1, Roi, ppValues, ppLevels, nLevels, pSepc);
	if (check_sts(status, "LightAdjustC1_ippiLUT_Init_8u", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}

	status = ippiLUT_8u_C1R(buf, nPitch, pDst, nDstPitch, Roi, pSepc);
	if (check_sts(status, "LightAdjustC1_ippiLUT_8u_C1R", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}
	ippsFree(pSepc);


	return 1;
}


int CKxLightAdjust::GetOneImageHistogramCharacterC3(const unsigned char* buf, int nWidth, int nHeight, int nPitch, float pnBlackAvg[3], float pnWhiteAvg[3], KxCallStatus& hCall)
{
	hCall.Clear();
	IppiSize roiSize = {nWidth, nHeight};
	//Ipp32s* nLevelsArray[3];
	//int nLevels[3] = {257, 257, 257};
	//Ipp32s lowerLevel[3] = {0, 0, 0};
	//Ipp32s upperLevel[3] = {256, 256, 256};
	//Ipp32s nHist[3][256];
	//Ipp32s nLevel[3][257];
	//Ipp32s* pnHist[3];
	//for(int k = 0; k < 3; k++)
	//{
	//	pnHist[k] = nHist[k];
	//	nLevelsArray[k] = nLevel[k];
	//}
	//ippiHistogramEven_8u_C3R(buf,nPitch,roiSize, pnHist, nLevelsArray, nLevels, lowerLevel, upperLevel);
	//ipp9.0 version
	//* get sizes for spec and buffer */
	const int nBins = 255;
	IppStatus status;
	Ipp32u* pnHist[3];
	for (int i = 0; i < 3; i++)
	{
		pnHist[i] = ippsMalloc_32u(nBins);
	}
	int nLevels[] = {nBins + 1, nBins + 1, nBins + 1};
	Ipp32f lowerLevel[] = {0, 0, 0};
	Ipp32f upperLevel[] = {255, 255, 255};
	int nSizeHistObj, nSizeBuffer;
	status = ippiHistogramGetBufferSize(ipp8u, roiSize, nLevels, 3, 1, &nSizeHistObj, &nSizeBuffer);
	if (check_sts(status, "GetOneImageHistogramCharacterC3_ippiHistogramGetBufferSize", hCall))
	{
		for (int i = 0; i < 3; i++)
		{
			ippsFree(pnHist[i]);
			pnHist[i] = NULL;
		}
		return 0;
	}

	IppiHistogramSpec* pHistObj = (IppiHistogramSpec* )ippsMalloc_8u(nSizeHistObj);
	Ipp8u* pBuffer = ippsMalloc_8u(nSizeBuffer);
	//initialize spec */
	status = ippiHistogramUniformInit(ipp8u, lowerLevel, upperLevel, nLevels, 3, pHistObj);
	if (check_sts(status, "GetOneImageHistogramCharacterC3_ippiHistogramUniformInit", hCall))
	{
		for (int i = 0; i < 3; i++)
		{
			ippsFree(pnHist[i]);
			pnHist[i] = NULL;
		}
		ippsFree(pBuffer);
		ippsFree(pHistObj);
		return 0;
	}

	//calculate histogram
	status = ippiHistogram_8u_C3R(buf, nPitch, roiSize, pnHist, pHistObj, pBuffer);
	if (check_sts(status, "GetOneImageHistogramCharacterC3_ippiHistogram_8u_C3R", hCall))
	{
		for (int i = 0; i < 3; i++)
		{
			ippsFree(pnHist[i]);
			pnHist[i] = NULL;
		}
		ippsFree(pBuffer);
		ippsFree(pHistObj);
		return 0;
	}

	ippsFree(pBuffer);
	ippsFree(pHistObj);



	int nBlackPixelsAmount[3] = {0, 0, 0};
	int nBlackPixelsGrayAmount[3] = {0, 0, 0};
	int nSkipBlackAmount[3] = {0, 0, 0};

	int nSkipBlackThresh = (nWidth * nHeight * m_hParamter.m_nSkipBlackPercent ) / 100;
	int nBlackPartPixelThreshold = (nWidth * nHeight * m_hParamter.m_nBlackPercent ) / 100;

	int nWhitePixelsAmount[3] = {0, 0, 0};
	int nWhitePixelsGrayAmount[3] = {0, 0, 0};
	int nSkipWhiteAmount[3] = {0, 0, 0};

	int nSkipWhiteThresh = (nWidth * nHeight * m_hParamter.m_nSkipWhitePercent ) / 100;
	int nWhitePartPixelThreshold = (nWidth * nHeight * m_hParamter.m_nWhitePercent ) / 100;


	for (int k = 0; k < 3; k++)
	{
		for(int i = 0; i < 255; i++)
		{
			if (nSkipBlackAmount[k] > nSkipBlackThresh)
			{
				if(nBlackPixelsAmount[k] > nBlackPartPixelThreshold)
					break;
				nBlackPixelsGrayAmount[k] += pnHist[k][i] * i;
				nBlackPixelsAmount[k] += pnHist[k][i];
			}
			nSkipBlackAmount[k] += pnHist[k][i];


			if (nSkipWhiteAmount[k] > nSkipWhiteThresh)
			{
				if(nWhitePixelsAmount[k] > nWhitePartPixelThreshold)
					break;
				nWhitePixelsGrayAmount[k] += pnHist[k][254-i] * (255-i);
				nWhitePixelsAmount[k] += pnHist[k][254-i];
			}
			nSkipWhiteAmount[k] += pnHist[k][254-i];

		}
	}

	for (int k = 0; k < 3; k++)
	{
		pnBlackAvg[k] = float(nBlackPixelsGrayAmount[k]*1.0 / gMax(1,nBlackPixelsAmount[k]));
		pnWhiteAvg[k] = float(nWhitePixelsGrayAmount[k]*1.0 / gMax(1,nWhitePixelsAmount[k]));
	}

	for (int i = 0; i < 3; i++)
	{
		ippsFree(pnHist[i]);
		pnHist[i] = NULL;
	}
	return 1;

}



int CKxLightAdjust::LightAdjustC3(const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;

	GetOneImageHistogramCharacterC3(m_ModelImg.buf, m_ModelImg.nWidth, m_ModelImg.nHeight, m_ModelImg.nPitch, m_hResult.m_nModelBlackAvgC3, m_hResult.m_nModelWhiteAvgC3, hCallInfo);
	if (check_sts(hCallInfo, "LightAdjustC3_GetOneImageHistogramCharacterC3_1", hCall))
	{
		return 0;
	}

	GetOneImageHistogramCharacterC3(buf, nWidth, nHeight, nPitch,  m_hResult.m_nGrayAverage_BlackPartC3, m_hResult.m_nGrayAverage_WhitePartC3, hCallInfo);
	if (check_sts(hCallInfo, "LightAdjustC3_GetOneImageHistogramCharacterC3_2", hCall))
	{
		return 0;
	}


    Ipp32s level[3][256];

	const Ipp32s* pLevel[3];
	const Ipp32s* pLutArray[3];


	float fK[3];
	for (int k = 0; k < 3; k++)
	{
		fK[k] = (m_hResult.m_nModelWhiteAvgC3[k] - m_hResult.m_nModelBlackAvgC3[k]) / (m_hResult.m_nGrayAverage_WhitePartC3[k] - m_hResult.m_nGrayAverage_BlackPartC3[k]);

		for(int i = 0; i < 256; i++)
		{
			int nValue = int(fK[k] * float(i - m_hResult.m_nGrayAverage_BlackPartC3[k]) + m_hResult.m_nModelBlackAvgC3[k]);
			if(nValue < 0)
				nValue = 0;
			else if(nValue > 255)
				nValue = 255;

			m_hResult.m_nLutArrayC3[k][i] = nValue;

			level[k][i] = i;
		}
	}
	for (int k = 0; k < 3; k++)
	{
		pLutArray[k] = (Ipp32s* )(m_hResult.m_nLutArrayC3[k]);
		pLevel[k] = (Ipp32s* )(level[k]);
	}

	IppiSize Roi = {nWidth, nHeight};
	//int nLevels[3] = {256, 256, 256};
	//ippiLUT_8u_C3R(buf, nPitch, pDst, nDstPitch, Roi, pLutArray, pLevel, nLevels);
	//ipp9.0 version
	//* get sizes for spec and buffer */
	int nSpecSize;
	IppiLUT_Spec* pSepc;
	IppStatus status;
	int nLevels[3] = {256, 256, 256};
	status = ippiLUT_GetSize(ippLinear, ipp8u, ippC3, Roi, nLevels, &nSpecSize);
	if (check_sts(status, "LightAdjustC3_ippiLUT_GetSize", hCall))
	{
		return 0;
	}
	pSepc = (IppiLUT_Spec*)ippsMalloc_8u(nSpecSize);
	status = ippiLUT_Init_8u(ippLinear, ippC3, Roi, pLutArray, pLevel, nLevels, pSepc);
	if (check_sts(status, "LightAdjustC3_ippiLUT_Init_8u", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}

	status = ippiLUT_8u_C3R(buf, nPitch, pDst, nDstPitch, Roi, pSepc);
	if (check_sts(status, "LightAdjustC3_ippiLUT_8u_C3R", hCall))
	{
		ippsFree(pSepc);
		return 0;
	}
	ippsFree(pSepc);

	return 1;
}



