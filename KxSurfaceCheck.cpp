

#include "KxSurfaceCheck.h"
#include "KxReadXml2.h"
#include "global.h"
//#define ColorImageAlign

CKxSurfaceCheck::CKxSurfaceCheck()
{
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_bIsLearnCompleted[i] = false;
	}
	m_bAllCompleted = false;

	m_nLearnPage = 50;

	memset(m_nAlgorithmStatus, 0, sizeof(int)* _Max_Layer);

	m_nLearnStatus = 0;


	m_nLargeSize = 5;

	m_nSpeckleMaxSize = 1;

	m_nHighOffset = 10;
	m_nLowOffset = 10;

	m_nIsDefectLearn = 0;

	m_nIsLoadParamsSuccess = 0;
	m_nIsStandardChange = 0;

	m_nResizeFactorX = 2;
	m_nResizeFactorY = 2;
	m_nBestLocationLayer = 3;

	m_nGrayModelSize = _3X3;

	m_nOriginalX = 0;

	m_nLowTemplateThreshold = 128;

	m_nSlider = 3;
}

CKxSurfaceCheck::~CKxSurfaceCheck()
{

}


bool CKxSurfaceCheck::ReadXml(tinyxml2::XMLDocument& xmlHandle, int nCameraId, int nIdx, const kxCImageBuf& BaseImg)
{
	m_nIsLoadParamsSuccess = 0;
	m_nIsStandardChange = 0;
	//参数
	if (!ReadParaXml(xmlHandle, nCameraId, nIdx, BaseImg))
	{
		m_nIsLoadParamsSuccess = 0;
		return false;
	}




	//清空已学习张数
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_hLearn[i].Clear();
	}
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_bIsLearnCompleted[i] = false;
	}
	m_bAllCompleted = false;


	memset(m_nAlgorithmStatus, 0, sizeof(int)* _Max_Layer);

	m_nLearnStatus = 0;

	AnalysisCheckStatus(m_BaseImg.nChannel);

	m_nIsDefectLearn = 0;

	m_nIsLoadParamsSuccess = 1;

	for (int i = 0; i < 3; i++)
	{
		m_nStandard[i] = 100;
	}

	return true;

}

void CKxSurfaceCheck::convertcolorimage2gray(kxCImageBuf& img, kxCImageBuf& grayimg, int nlayer)
{

	int nLayerType;
	if (nlayer == 0)
	{
		nLayerType = RGB_R;
	}
	else if (nlayer == 1)
	{
		nLayerType = RGB_G;
	}
	else if (nlayer == 2)
	{
		nLayerType = RGB_B;
	}
	else
	{
		nLayerType = RGB_GRAY;
	}
	m_hFun.KxConvertImageLayer(img, grayimg, nLayerType);

}

void CKxSurfaceCheck::GetBestLocationLayer(kxCImageBuf& img, int& nLayer)
{
	Ipp64f maxDev = -1;
	for (int i = 0; i < 4; i++)
	{
		Ipp64f mean, dev;
		convertcolorimage2gray(img, m_ImgTransfer, i);
		IppiSize roiSize = { m_ImgTransfer.nWidth, m_ImgTransfer.nHeight };
		ippiMean_StdDev_8u_C1R(m_ImgTransfer.buf, m_ImgTransfer.nPitch, roiSize, &mean, &dev);
		if (dev > maxDev)
		{
			maxDev = dev;
			nLayer = i;
		}
	}
}


bool CKxSurfaceCheck::ReadParaXml(tinyxml2::XMLDocument& xmlHandle, int nCameraId, int nIdx, const kxCImageBuf& BaseImg)
{
	//重要参数
	std::string szResult;

	//定位参数设置
	char sz[128];
	sprintf_s(sz, 128, "分块%d", nIdx);

	char szCamera[128];
	sprintf_s(szCamera, 128, "相机%d", nCameraId+1);

	int nSearchStatus =  SearchXmlGetValue(xmlHandle, "分块设置", szCamera, sz, "检查区域", "pos", szResult);
	if (!nSearchStatus)
	{
		return false;
	}

	int nStatus =  FromStringToKxRect(szResult, m_hParameter.m_rcCheck);
	if (!nStatus)
	{
		return false;
	}

	m_BaseImg.Init(m_hParameter.m_rcCheck.Width(), m_hParameter.m_rcCheck.Height(), BaseImg.nChannel);
	m_hFun.KxCopyImage(BaseImg, m_BaseImg, m_hParameter.m_rcCheck);



	nSearchStatus =  SearchXmlGetValue(xmlHandle, "分块设置", szCamera, sz, "块号", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nBlockIdx);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus =  SearchXmlGetValue(xmlHandle, "分块设置", szCamera, sz, "定位核区域", "pos", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToKxRect(szResult, m_hParameter.m_rcKernel);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "分块设置", szCamera, sz, "扩展大小", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nExpand);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "分块设置", szCamera, sz, "搜索方向", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nSearchDir);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "分块设置", szCamera, sz, "水平偏移", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nOffsetX);
	if (!nStatus)
	{
		return false;
	}




#ifdef ColorImageAlign
	m_ImgKernel.Init(m_hParameter.m_rcKernel.Width()/m_nResizeFactor, m_hParameter.m_rcKernel.Height()/m_nResizeFactor, BaseImg.nChannel);
	m_ImgBigKernel.SetImageBuf(BaseImg.buf + m_hParameter.m_rcKernel.top*BaseImg.nPitch + m_hParameter.m_rcKernel.left * BaseImg.nChannel,
		m_hParameter.m_rcKernel.Width(), m_hParameter.m_rcKernel.Height(), BaseImg.nPitch, BaseImg.nChannel, false);
	m_hFun.KxResizeImage(m_ImgBigKernel, m_ImgKernel, KxSuper);
#else
	//首先确认最佳定位层面（利用方差判断）
	kxCImageBuf imgtest;
	imgtest.SetImageBuf(BaseImg.buf + m_hParameter.m_rcKernel.top*BaseImg.nPitch + m_hParameter.m_rcKernel.left * BaseImg.nChannel, m_hParameter.m_rcKernel.Width(),
		m_hParameter.m_rcKernel.Height(), BaseImg.nPitch, BaseImg.nChannel, false);

	if (BaseImg.nChannel == _Type_G24)
	{
		GetBestLocationLayer(imgtest, m_nBestLocationLayer);
		m_ImgBigKernel.Init(m_hParameter.m_rcKernel.Width(), m_hParameter.m_rcKernel.Height());
		convertcolorimage2gray(imgtest, m_ImgBigKernel, m_nBestLocationLayer);

		m_ImgKernel.Init(m_hParameter.m_rcKernel.Width() / 2, m_hParameter.m_rcKernel.Height() / 2, m_ImgBigKernel.nChannel);
		m_hFun.KxResizeImage(m_ImgBigKernel, m_ImgKernel, KxSuper);
	}
	else
	{
		//铜箔检查先二值化
		m_ImgBigKernel.Init(m_hParameter.m_rcKernel.Width(), m_hParameter.m_rcKernel.Height());
		if (m_nIsBinary && BaseImg.nChannel == _Type_G8)
		{
			IppiSize RoiSize = { imgtest.nWidth, imgtest.nHeight };
			ippiCompareC_8u_C1R(imgtest.buf, imgtest.nPitch, (Ipp8u)m_nThreshold, m_ImgBigKernel.buf, m_ImgBigKernel.nPitch, RoiSize, ippCmpGreaterEq);
		}
		else
		{
			m_ImgBigKernel.SetImageBuf(imgtest.buf, imgtest.nWidth, imgtest.nHeight, imgtest.nPitch, imgtest.nChannel, false);
		}

		//m_ImgKernel.Init(m_hParameter.m_rcKernel.Width() / 2, m_hParameter.m_rcKernel.Height() / 2, m_ImgBigKernel.nChannel);
		//m_hFun.KxResizeImage(m_ImgBigKernel, m_ImgKernel, KxSuper);

		if (m_hParameter.m_nSearchDir == _Horiz_Vertical_Dir)   //水平垂直搜索
		{
			m_ImgKernel.Init(m_hParameter.m_rcKernel.Width() / 2, m_hParameter.m_rcKernel.Height() / 2, m_ImgBigKernel.nChannel);
			m_hFun.KxResizeImage(m_ImgBigKernel, m_ImgKernel, KxLinear);
		}
		else if (m_hParameter.m_nSearchDir == _Vertical_Dir)
		{
			m_ImgKernel.Init(1, m_hParameter.m_rcKernel.Height(), m_ImgBigKernel.nChannel);
			m_hFun.KxResizeImage(m_ImgBigKernel, m_ImgKernel, KxSuper);
		}
		else if (m_hParameter.m_nSearchDir == _Horizontal_Dir)
		{
			m_ImgKernel.Init(m_hParameter.m_rcKernel.Width(), 1, m_ImgBigKernel.nChannel);
			m_hFun.KxResizeImage(m_ImgBigKernel, m_ImgKernel, KxSuper);
		}
		else
		{
			//do nothing
		}
	}



#endif

	//将坐标转换成对应相机采集图的坐标
	m_hParameter.m_rcKernel.offset(-m_hParameter.m_nOffsetX, 0);
	m_hParameter.m_rcCheck.offset(-m_hParameter.m_nOffsetX, 0);
	
	//检查参数设置

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "检查设置", "学习", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToBool(szResult, m_hParameter.m_nIsLearn);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "检查设置", "学习张数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}

	nStatus =  FromStringToInt(szResult, m_nLearnPage);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus =  SearchXmlGetValue(xmlHandle, "检查设置", "破洞灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nLineOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "检查设置", "开路灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGrayHighOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "检查设置", "短路灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGrayLowOffset);
	if (!nStatus)
	{
		return false;
	}

    //质量标准
	nSearchStatus =  SearchXmlGetValue(xmlHandle, "质量标准设置", "短路", "能量", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGrayLowSmallEnergy);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "质量标准设置", "短路", "点数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGrayLowSmallDots);
	if (!nStatus)
	{
		return false;
	}

	//2018/11/1
	nSearchStatus =  SearchXmlGetValue(xmlHandle, "质量标准设置", "短路", "过滤", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nIsFilterFakeDefect);
	if (!nStatus)
	{
		return false;
	}



	nSearchStatus =  SearchXmlGetValue(xmlHandle, "质量标准设置", "开路", "能量", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGrayHighSmallEnergy);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "质量标准设置", "开路", "点数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGrayHighSmallDots);
	if (!nStatus)
	{
		return false;
	}



	nSearchStatus =  SearchXmlGetValue(xmlHandle, "质量标准设置", "破洞", "能量", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGradientSmallEnergy);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus =  SearchXmlGetValue(xmlHandle, "质量标准设置", "破洞", "点数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus =  FromStringToInt(szResult, m_hParameter.m_nGradientSmallDots);
	if (!nStatus)
	{
		return false;
	}

	return true;
}




int CKxSurfaceCheck::LoadTemplateImg(const char* lpszFile)
{
	FILE*   fp;
#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "rb") != 0)
	{
		m_nModelNumber = 0;
		for (int k = 0; k < _Max_Layer; k++)
		{
			m_ImgTemplate[k].Init(0, 0, 1);
		}
		return FALSE;
	}
#else
	fp = fopen(lpszFile, "rb");
	if (fp == NULL)
	{
		m_nModelNumber = 0;
		for (int k = 0; k < _Max_Layer + 1; k++)
		{
			m_ImgTemplate[k].Init(0, 0, 1);
		}
		return FALSE;
	}
#endif


	int b = ReadImg(fp);
	fclose(fp);
	return b;
}

int CKxSurfaceCheck::SaveTemplateImg(const char* lpszFile)
{
	FILE*   fp;

#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "wb") != 0)
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


	int b = WriteImg(fp);
	fclose(fp);
	return b;

}


int CKxSurfaceCheck::ReadImg(FILE* fp)
{
	if (fread(&m_nModelNumber, sizeof(int), 1, fp) != 1)
		return FALSE;

	switch (m_nModelNumber)
	{
	case _Max_Layer:
		for (int i = 0; i < m_nModelNumber; i++)
			if (FALSE == m_ImgTemplate[i].Read(fp))
				return FALSE;
		break;
	default:
		return FALSE;
	}
	//for (int k = 0; k < _Max_Layer + 1; k++)
	//{
	//	m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
	//	kxRect<int> rc;
	//	rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
	//	m_hFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
	//}
	//...............................................
	return TRUE;
}

int CKxSurfaceCheck::WriteImg(FILE* fp)
{

	if (fwrite(&m_nModelNumber, sizeof(int), 1, fp) != 1)
		return FALSE;

	switch (m_nModelNumber)
	{
	case _Max_Layer:
		for (int i = 0; i < m_nModelNumber; i++)
		{
			if (i == 1)  //灰度检查高
			{
				Ipp8u Thresh8u;
				IppiSize roiSize = { m_ImgTemplate[i].nWidth, m_ImgTemplate[i].nHeight };
				ippiComputeThreshold_Otsu_8u_C1R(m_ImgTemplate[i].buf, m_ImgTemplate[i].nPitch, roiSize, &Thresh8u);
				ippiThreshold_GTVal_8u_C1IR(m_ImgTemplate[i].buf, m_ImgTemplate[i].nPitch, roiSize, min(254, Thresh8u + 10), 255);
			}
			if (FALSE == m_ImgTemplate[i].Write(fp))
				return FALSE;
/*			char sz[128];
			sprintf_s(sz, 128, "f:\\123\\template_%d_%d.bmp", m_hParameter.m_nBlockIdx, i);
			m_hFun.SaveBMPImage_h(sz, m_ImgTemplate[i]);	*/	
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}


bool CKxSurfaceCheck::SetTemplatesFiles(const kxCImageBuf& srcImg, kxRect<int>& rcCheckArea, int nModelSize)
{
	kxCImageBuf imgArea;
	imgArea.SetImageBuf(srcImg.buf + srcImg.nPitch * rcCheckArea.top + rcCheckArea.left * srcImg.nChannel, rcCheckArea.Width(), 
		rcCheckArea.Height(), srcImg.nPitch, srcImg.nChannel, false);
	kxCImageBuf imgThreshold;
	int nmasksizex = (m_nResizeFactorX == 1 ? 1 : (m_nResizeFactorX + 1));
	int nmasksizey = (m_nResizeFactorY == 1 ? 1 : (m_nResizeFactorY + 1));	
	{
		m_hFun.KxErodeImage(imgArea, m_ImgOffset[0], nmasksizex, nmasksizey);
		m_ImgCheck[0].Init(imgArea.nWidth / m_nResizeFactorX, imgArea.nHeight / m_nResizeFactorY, imgArea.nChannel);
		m_hFun.KxResizeImage(m_ImgOffset[0], m_ImgCheck[0], KxSuper);
		imgThreshold.Init(m_ImgCheck[0].nWidth, m_ImgCheck[0].nHeight, m_ImgCheck[0].nChannel);
		IppiSize roiSize = { m_ImgCheck[0].nWidth, m_ImgCheck[0].nHeight };
		ippiCompareC_8u_C1R(m_ImgCheck[0].buf, m_ImgCheck[0].nPitch, 1, imgThreshold.buf, imgThreshold.nPitch, roiSize, ippCmpGreaterEq);
		m_hFun.KxErodeImage(imgThreshold, m_ImgTemplate[0], nModelSize, nModelSize);
	}
	{
		m_hFun.KxDilateImage(imgArea, m_ImgOffset[1], nmasksizex, nmasksizey);
		m_ImgCheck[1].Init(imgArea.nWidth / m_nResizeFactorX, imgArea.nHeight / m_nResizeFactorY, imgArea.nChannel);
		m_hFun.KxResizeImage(m_ImgOffset[1], m_ImgCheck[1], KxSuper);
		imgThreshold.Init(m_ImgCheck[1].nWidth, m_ImgCheck[1].nHeight, m_ImgCheck[1].nChannel);
		IppiSize roiSize = { m_ImgCheck[1].nWidth, m_ImgCheck[1].nHeight };
		ippiCompareC_8u_C1R(m_ImgCheck[1].buf, m_ImgCheck[1].nPitch, 1, imgThreshold.buf, imgThreshold.nPitch, roiSize, ippCmpGreaterEq);
		m_hFun.KxDilateImage(imgThreshold, m_ImgTemplate[1], nModelSize, nModelSize);
	}

	return true;
}

//处理精确拉伸后的图像
int  CKxSurfaceCheck::ProcessSurfaceWarpImages(KxCallStatus& hCall)
{
	hCall.Clear();
	int nMode[_Max_Layer];
	memset(nMode, 0, sizeof(int)*_Max_Layer);

	////检低二值化
	//Ipp8u Thresh8u;
	//IppiSize roiSize = { m_ImgWarpGray.nWidth, m_ImgWarpGray.nHeight };
	//ippiComputeThreshold_Otsu_8u_C1R(m_ImgWarpGray.buf, m_ImgWarpGray.nPitch, roiSize, &Thresh8u);
	//Thresh8u = Thresh8u - 10;
	//IppiSize Roi = { m_WarpResizeImg.nWidth, m_WarpResizeImg.nHeight };
	//m_ImgCheck[0].Init(m_WarpResizeImg.nWidth, m_WarpResizeImg.nHeight);
	//ippiCompareC_8u_C1R(m_WarpResizeImg.buf, m_WarpResizeImg.nPitch, Thresh8u, m_ImgCheck[0].buf, m_ImgCheck[0].nPitch, Roi, ippCmpGreaterEq);
	////检高原图
	//m_ImgCheck[1].SetImageBuf(m_WarpResizeImg.buf, m_WarpResizeImg.nWidth, m_WarpResizeImg.nHeight, m_WarpResizeImg.nPitch, m_WarpResizeImg.nChannel, false);
	//for (int i = 0; i < 2; i++)
	//{
	//	m_ImgCheck[i].SetImageBuf(m_WarpResizeImg.buf, m_WarpResizeImg.nWidth, m_WarpResizeImg.nHeight, m_WarpResizeImg.nPitch, m_WarpResizeImg.nChannel, false);
	//}
	int nAlgorithmOpen[_Max_Layer];
	KxCallStatus hCallInfo[_Max_Layer];
	for (int i = 0; i < _Max_Layer; i++)
	{
		nAlgorithmOpen[i] = m_nAlgorithmStatus[i];
		hCallInfo[i].Clear();
	}
	
	int nmasksizex = (m_nResizeFactorX == 1 ? 1 : (m_nResizeFactorX + 1));
	int nmasksizey = (m_nResizeFactorY == 1 ? 1 : (m_nResizeFactorY + 1));

	parallel_for(blocked_range<int>(0, _Max_Layer),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		//for (int i = 2; i < _Max_Layer; i++)
		{
			if (nAlgorithmOpen[i] || m_nLearnStatus)
			{
				if (i == 0)
				{
					m_hFun.KxErodeImage(m_WarpImg, m_ImgOffset[i], nmasksizex, nmasksizey);
					m_ImgCheck[0].Init(m_WarpImg.nWidth / m_nResizeFactorX, m_WarpImg.nHeight / m_nResizeFactorY, m_WarpImg.nChannel);
					m_hFun.KxResizeImage(m_ImgOffset[i], m_ImgCheck[0], KxSuper);
				}
				else if (i == 1)
				{
					m_hFun.KxDilateImage(m_WarpImg, m_ImgOffset[i], nmasksizex, nmasksizey);
					m_ImgCheck[1].Init(m_WarpImg.nWidth / m_nResizeFactorX, m_WarpImg.nHeight / m_nResizeFactorY, m_WarpImg.nChannel);
					m_hFun.KxResizeImage(m_ImgOffset[i], m_ImgCheck[1], KxSuper);
				}
				else
				{
					int nStatus = m_hGradient[i].SingleDirGradient(m_WarpResizeImg, m_ImgCheck[i], i - 2, 4, 255, 5, hCallInfo[i]);
					if (nStatus == 0)
					{
						break;
					}
				}				
			}
		}
	}, auto_partitioner());

	for (int i = 0; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i] || m_nLearnStatus)
		{
			if (check_sts(hCallInfo[i], "", hCall))
			{
				return 0;
			}
		}
	}


	return 1;
}

//学习图像
int  CKxSurfaceCheck::LearnTemplate(KxCallStatus& hCall)
{
	//if (m_nIsDefectLearn)  //如果开启了缺陷学习,更新模板
	//{
	//	for (int i = 0; i < _Max_Layer; i++)
	//	{
	//		m_ImgTemplate[i].Init(m_ImgLearnTemplate[i].nWidth, m_ImgLearnTemplate[i].nHeight, m_ImgLearnTemplate[i].nChannel);
	//		IppiSize roiSize = { m_ImgTemplate[i].nWidth, m_ImgTemplate[i].nHeight };
	//		ippiCopy_8u_C3R(m_ImgLearnTemplate[i].buf, m_ImgLearnTemplate[i].nPitch, m_ImgTemplate[i].buf, m_ImgTemplate[i].nPitch, roiSize);
	//	}
	//	m_nIsDefectLearn = 0;
	//}

	hCall.Clear();
	if (!m_hParameter.m_nIsLearn)
	{
		return 0;
	}

	int nLearnHighLowMode[_Max_Layer];
	memset(nLearnHighLowMode, 0, sizeof(int)*_Max_Layer);
	for (int i = 1; i < _Max_Layer; i++)
	{
		nLearnHighLowMode[i] = 1;  //学高
	}


	//模板操作
	int nModelProcessSz[_Max_Layer];
	for (int i = 0; i < 2; i++)
	{
		nModelProcessSz[i] = m_nGrayModelSize ;
	}
	for (int i = 2; i < _Max_Layer; i++)
	{
		nModelProcessSz[i] = _5X5;
	}


	parallel_for(blocked_range<int>(0, _Max_Layer),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		//for (int i = 0; i < _Max_Layer; i++)
		{
			if (m_nLearnStatus)
			{
				if (m_hLearn[i].LearnImage(m_ImgCheck[i], nLearnHighLowMode[i], m_nLearnPage, hCall))
				{
					m_bIsLearnCompleted[i] = true;
					m_ImgTemplate[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight, m_ImgCheck[i].nChannel);
					m_hLearn[i].GetTemplateBufParallel(_Valid_Layer, nLearnHighLowMode[i], m_ImgTemplate[i], nModelProcessSz[i]);
				}
			}

		}
	}, auto_partitioner());

	bool status = true;
	for (int i = 0; i < _Max_Layer; i++)
	{
		if (m_nLearnStatus)
		{
			status = (status && m_bIsLearnCompleted[i]);
		}
	}
	if (status)
	{
		m_bAllCompleted = true;
		m_hParameter.m_nIsLearn = 0;
		m_nModelNumber = _Max_Layer;
		m_nLearnStatus = 0;

		for (int i = 0; i < _Max_Layer; i++)
		{
			m_bIsLearnCompleted[i] = false;
			m_hLearn[i].Clear();
		}
		

	}

	return 1;
}


//残差分析并且合并为一张残差图
int CKxSurfaceCheck::ResidualsAnalys(KxCallStatus& hCall)
{

	if (m_hParameter.m_nIsLearn)
	{
		return 0;
	}

	hCall.Clear();
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_ImgSlidCc[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight, m_ImgCheck[i].nChannel);
		//m_ImgSliderNorm[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight);
		//m_ImgOffset[i].Init(m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight);
		m_hSlider[i].SetSliderRange(-2, 1, 2, -m_nSlider, 1, m_nSlider);
		 
	}

	int nCheckHighLowMode[_Max_Layer];
	memset(nCheckHighLowMode, 0, sizeof(int)*_Max_Layer);
	KxCallStatus hCallInfo[_Max_Layer];
	for (int i = 1; i < _Max_Layer; i++)
	{
		nCheckHighLowMode[i] = 1;  //检高
		hCallInfo[i].Clear();
	}

	int nGridXY[_Max_Layer];
	int nOffset[_Max_Layer];

	nGridXY[0] = 40;
	nGridXY[1] = 40;
	nOffset[0] = m_hParameter.m_nGrayLowOffset;
	nOffset[1] = m_hParameter.m_nGrayHighOffset;

	
	for (int i = 2; i < _Max_Layer; i++)
	{
		nGridXY[i] = 40;
		nOffset[i] = m_hParameter.m_nLineOffset;
	}


	parallel_for(blocked_range<int>(0, _Max_Layer),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		//for (int i = 0; i < _Max_Layer; i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				m_hSlider[i].SetOffset(nOffset[i]);
				int nStatus = m_hSlider[i].SilderCc(m_ImgCheck[i], m_ImgTemplate[i], m_ImgSlidCc[i], nCheckHighLowMode[i], nGridXY[i], nGridXY[i], hCallInfo[i]);
				if (nStatus == 0)
				{
					break;
				}

				//if (m_ImgSlidCc[i].nChannel == _Type_G24)
				//{
				//	IppiSize roiSize = { m_ImgSlidCc[i].nWidth, m_ImgSlidCc[i].nHeight };
				//	ippiGradientColorToGray_8u_C3C1R(m_ImgSlidCc[i].buf, m_ImgSlidCc[i].nPitch, m_ImgSliderNorm[i].buf, m_ImgSliderNorm[i].nPitch, roiSize, ippiNormInf);
				//}
				//else
				//{
				//	IppiSize roiSize = { m_ImgSlidCc[i].nWidth, m_ImgSlidCc[i].nHeight };
				//	ippiCopy_8u_C1R(m_ImgSlidCc[i].buf, m_ImgSlidCc[i].nPitch, m_ImgSliderNorm[i].buf, m_ImgSliderNorm[i].nPitch, roiSize);
				//}

				//IppiSize roiSize = { m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight };
				//m_hBaseFun[i].KxSubCImage(m_ImgSliderNorm[i], m_ImgOffset[i], nOffset[i]);

			}
		}
	}, auto_partitioner());

	for (int i = 0; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			if (check_sts(hCallInfo[i], "", hCall))
			{
				return 0;
			}
		}
	}

	m_ImgTemp.Init(m_ImgSlidCc[0].nWidth, m_ImgSlidCc[0].nHeight);
	ippsSet_8u(0, m_ImgTemp.buf, m_ImgTemp.nPitch*m_ImgTemp.nHeight);
	//m_ImgCc.Init(m_ImgOffset[0].nWidth, m_ImgOffset[0].nHeight);

	//边缘5个像素不捡,有重叠区不怕
	const int nExtend = 5;
	m_ImgCc.Init(m_ImgSlidCc[0].nWidth, m_ImgSlidCc[0].nHeight);
	ippsSet_8u(0, m_ImgCc.buf, m_ImgCc.nPitch*m_ImgCc.nHeight);

	if (m_nAlgorithmStatus[2])
	{
		m_ImgLineMax.Init(m_ImgSlidCc[2].nWidth, m_ImgSlidCc[2].nHeight);
		ippsSet_8u(0, m_ImgLineMax.buf, m_ImgLineMax.nPitch*m_ImgLineMax.nHeight);
	}

	for (int i = 0; i < 2; i++)
	{
		if (m_nAlgorithmStatus[i])
			m_hBaseFun[0].KxMaxEvery(m_ImgSlidCc[i], m_ImgCc);
	}

	m_hBaseFun[0].KxOpenImage(m_ImgCc, m_ImgSliderNorm[0], 2, 1);
	m_hBaseFun[0].KxOpenImage(m_ImgSliderNorm[0], m_ImgCc, 1, 2);

	IppiSize roiSize = { m_ImgTemp.nWidth - 2 * nExtend, m_ImgTemp.nHeight - 2 * nExtend };
	ippiCopy_8u_C1R(m_ImgCc.buf + nExtend * m_ImgCc.nPitch + nExtend, m_ImgCc.nPitch,
		m_ImgTemp.buf + nExtend * m_ImgTemp.nPitch + nExtend, m_ImgTemp.nPitch, roiSize);


	for (int i = 2; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[0].KxMaxEvery(m_ImgSlidCc[i], m_ImgLineMax);
		}	
	}

	return 1;

}

//只返回一个缺陷信息
int CKxSurfaceCheck::ConnectAnalysis(const kxCImageBuf& SrcImg, kxCImageBuf& DefectImg, KxCallStatus& hCall)
{
	if (m_hParameter.m_nIsLearn)
	{
		return 0;
	}

	m_hResult.Clear();
	int nMergeSize = 1; //Blob连接尺寸为1
	const int nExtend = 50;

	if (m_nAlgorithmStatus[0] || m_nAlgorithmStatus[1])
	{
		//if (m_hResult.nDefectCount == 0)
		{
			int nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, nMergeSize, 0, hCall);
			if (nStatus == 0)
			{
				m_hResult.nStatus = _FATAL_Err;
				m_hResult.szErrInfo = "Layer1BlobErr";
				return 0;
			}
			int nCount = (std::min)((int)_Max_BlobCount, m_hBlobAnaly.GetBlobCount());

			for (int i = 0; i < nCount; i++)
			{
				CKxBlobAnalyse::SingleBlobInfo& pObj = m_hBlobAnaly.GetSortSingleBlob(i);

				//判断一下到底是灰度低检出还是灰度高检出
				int nType = 0;
				if (m_nAlgorithmStatus[0] == 1 && m_nAlgorithmStatus[1] == 0)
				{
					nType = 0;
				}
				else if (m_nAlgorithmStatus[0] == 0 && m_nAlgorithmStatus[1] == 1)
				{
					nType = 1;
				}
				else
				{
					Ipp64f pSumTotal, pSumGraylow, pSumGrayhigh;
					IppiSize roiSize = { pObj.m_rc.Width(), pObj.m_rc.Height() };
					ippiSum_8u_C1R(m_ImgTemp.buf + m_ImgTemp.nPitch * pObj.m_rc.top + pObj.m_rc.left, m_ImgTemp.nPitch, roiSize, &pSumTotal);
					ippiSum_8u_C1R(m_ImgSlidCc[0].buf + m_ImgSlidCc[0].nPitch * pObj.m_rc.top + pObj.m_rc.left, m_ImgSlidCc[0].nPitch, roiSize, &pSumGraylow);
					ippiSum_8u_C1R(m_ImgSlidCc[1].buf + m_ImgSlidCc[1].nPitch * pObj.m_rc.top + pObj.m_rc.left, m_ImgSlidCc[1].nPitch, roiSize, &pSumGrayhigh);
					if (abs(pSumTotal - pSumGraylow) > abs(pSumTotal - pSumGrayhigh))
					{
						nType = 1;
					}
					else
					{
						nType = 0;
					}
				}

				if ((nType == 0) && (pObj.m_nDots*m_nResizeFactorX * m_nResizeFactorY > m_hParameter.m_nGrayLowSmallDots && pObj.m_nEnergy > m_hParameter.m_nGrayLowSmallEnergy))
				{
					if (m_hParameter.m_nIsFilterFakeDefect)  //过滤假开路
					{
						int nlabel = pObj.m_nLabel;
						kxRect<int> rcAnalyse = pObj.m_rc;
						//rcAnalyse.mulC(m_nResizeFactorX, m_nResizeFactorY);
						rcAnalyse.left = gMax(rcAnalyse.left - nExtend, 0);
						rcAnalyse.top = gMax(rcAnalyse.top - nExtend, 0);
						rcAnalyse.right = gMin(rcAnalyse.right + nExtend, m_ImgTemp.nWidth - 1);
						rcAnalyse.bottom = gMin(rcAnalyse.bottom + nExtend, m_ImgTemp.nHeight - 1);
						
						//统计模板对应区域连通域个数
						m_ImgAnalyse.SetImageBuf(m_ImgTemplate[0].buf + rcAnalyse.left*m_ImgTemplate[0].nChannel + m_ImgTemplate[0].nPitch * rcAnalyse.top, rcAnalyse.Width(), rcAnalyse.Height(),
							m_ImgTemplate[0].nPitch, m_ImgTemplate[0].nChannel, false);		
						m_hFun.KxThreshImage(m_ImgAnalyse, m_ImgAnalyseBinary, m_nLowTemplateThreshold, 255);

						m_hBlobAnaly.GetBlobImage(nlabel, rcAnalyse, m_ImgAnalyseCc);
						m_hFun.KxThreshImage(m_ImgAnalyseCc, m_ImgAnalyseCc1, 1, 255);
						if (m_hParameter.m_nIsFilterFakeDefect == 3)      //强过滤
						{
							kxRect<int> rcCopy;
							rcCopy.setup(0, 0, m_ImgAnalyseCc.nWidth - 1, m_ImgAnalyseCc.nHeight - 1);
							m_ImgAnalyseCcDilate.Init(m_ImgAnalyseCc.nWidth , m_ImgAnalyseCc.nHeight );
							m_hFun.KxCopyImage(m_ImgAnalyseCc1, m_ImgAnalyseCcDilate, rcCopy);
						}
						else if (m_hParameter.m_nIsFilterFakeDefect == 1)  //弱过滤
						{
							m_hFun.KxDilateImage(m_ImgAnalyseCc1, m_ImgAnalyseCcDilate, 5, 5);
						}
						else                                          //2     //过滤
						{
							m_hFun.KxDilateImage(m_ImgAnalyseCc1, m_ImgAnalyseCcDilate, 3, 3);
						}

						IppiSize Roi = { m_ImgAnalyseBinary.nWidth, m_ImgAnalyseBinary.nHeight };
						m_ImgSubCc.Init(Roi.width, Roi.height);
						ippiSub_8u_C1RSfs(m_ImgAnalyseCcDilate.buf, m_ImgAnalyseCcDilate.nPitch, m_ImgAnalyseBinary.buf, m_ImgAnalyseBinary.nPitch, m_ImgSubCc.buf, m_ImgSubCc.nPitch, Roi, 0);

						int nTemplateCount = 0;
						int nSubCount = 0;
						int  nBufferSize;
						IppStatus status = ippiLabelMarkersGetBufferSize_8u_C1R(Roi, &nBufferSize);
						Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
						status = ippiLabelMarkers_8u_C1IR(m_ImgAnalyseBinary.buf, m_ImgAnalyseBinary.nPitch, Roi, 1, 254,
							ippiNormInf, &nTemplateCount, pBuffer);
						status = ippiLabelMarkers_8u_C1IR(m_ImgSubCc.buf, m_ImgSubCc.nPitch, Roi, 1, 254,
							ippiNormInf, &nSubCount, pBuffer);
						ippsFree(pBuffer);

						if (nTemplateCount == nSubCount)  //假缺陷
						{
							continue;
						}
					}


					m_hResult.mapFeaturelists[0][0] = std::make_pair("点数", pObj.m_nDots*m_nResizeFactorX * m_nResizeFactorY *1.0f);
					m_hResult.mapFeaturelists[0][1] = std::make_pair("能量", pObj.m_nEnergy*1.0f);
					kxRect<int> rc = pObj.m_rc;
					rc.mulC(m_nResizeFactorX, m_nResizeFactorY);
					rc.left = gMax(rc.left - 10, 0);
					rc.top = gMax(rc.top - 10, 0);
					rc.right = gMin(rc.right + 10, m_WarpImg.nWidth - 1);
					rc.bottom = gMin(rc.bottom + 10, m_WarpImg.nHeight - 1);

					kxRect<int> rcdefect;			
					rcdefect.left = max(0, rc.left - nExtend);
					rcdefect.top = max(0, rc.top - nExtend);
					rcdefect.right = min(m_WarpImg.nWidth - 1, rcdefect.left + rc.Width() + 2 * nExtend);
					rcdefect.bottom = min(m_WarpImg.nHeight - 1, rcdefect.top + rc.Height() + 2 * nExtend);
					//DefectImg.Init(rcdefect.Width(), rcdefect.Height(), m_WarpImg.nChannel);
					//m_hFun.KxCopyImage(m_ImgWarpGray, DefectImg, rcdefect);

					DefectImg.Init(rcdefect.Width(), rcdefect.Height() * 2, m_ImgWarpGray.nChannel);
					IppiSize roiSize = { rcdefect.Width(), rcdefect.Height() };
					ippiCopy_8u_C1R(m_ImgWarpGray.buf + rcdefect.top * m_ImgWarpGray.nPitch + rcdefect.left*m_ImgWarpGray.nChannel, m_ImgWarpGray.nPitch,
						DefectImg.buf, DefectImg.nPitch, roiSize);
					kxRect<int> rcCopy = rcdefect;
					rcCopy.divC(m_nResizeFactorX, m_nResizeFactorY);
					kxCImageBuf m_ImgPyramidcc, m_ImgCcResize, m_ImgThresholdCc;
					m_ImgPyramidcc.Init(rcCopy.Width(), rcCopy.Height());
					m_hFun.KxCopyImage(m_ImgSlidCc[0], m_ImgPyramidcc, rcCopy);
					m_ImgCcResize.Init(rcdefect.Width(), rcdefect.Height());
					m_hFun.KxResizeImage(m_ImgPyramidcc, m_ImgCcResize);
					m_ImgThresholdCc.Init(rcdefect.Width(), rcdefect.Height());
					ippiCompareC_8u_C1R(m_ImgCcResize.buf, m_ImgCcResize.nPitch, 1, m_ImgThresholdCc.buf, m_ImgThresholdCc.nPitch, roiSize, ippCmpGreaterEq);
					ippiCopy_8u_C1R(m_ImgThresholdCc.buf, m_ImgThresholdCc.nPitch, DefectImg.buf + rcdefect.Height() * DefectImg.nPitch, DefectImg.nPitch, roiSize);




					m_hResult.mapFeaturelists[0][2] = std::make_pair("X坐标", (rc.left-rcdefect.left)*1.0f);
					m_hResult.mapFeaturelists[0][3] = std::make_pair("Y坐标", (rc.top-rcdefect.top)*1.0f);
					m_hResult.mapFeaturelists[0][4] = std::make_pair("缺陷宽", rc.Width()*1.0f);
					m_hResult.mapFeaturelists[0][5] = std::make_pair("缺陷高", rc.Height()*1.0f);
					m_hResult.mapFeaturelists[0][6] = std::make_pair("类型", 0.0f);
					m_hResult.mapFeaturelists[0][7] = std::make_pair("块号", 1.0f * m_hParameter.m_nBlockIdx);
					
					m_hResult.mapFeaturelists[0][8] = std::make_pair("大图坐标X", m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX + rcdefect.left);
					m_hResult.mapFeaturelists[0][9] = std::make_pair("大图坐标Y", m_hParameter.m_rcCheck.top*1.0f + rcdefect.top);
					m_hResult.mapFeaturelists[0][10] = std::make_pair("块宽", rcdefect.Width()*1.0f);
					m_hResult.mapFeaturelists[0][11] = std::make_pair("块高", rcdefect.Height()*1.0f);

					rc.offset(m_hParameter.m_rcCheck.left, m_hParameter.m_rcCheck.top);
					m_hResult.mapFeaturelists[0][12] = std::make_pair("X原坐标", rc.left*1.0f + m_hParameter.m_nOffsetX);
					m_hResult.mapFeaturelists[0][13] = std::make_pair("Y原坐标", rc.top*1.0f);

					m_hResult.mapFeaturelists[0][14] = std::make_pair("块X坐标", rc.left*1.0f);
					m_hResult.mapFeaturelists[0][15] = std::make_pair("块Y坐标", rc.top*1.0f);

					kxRect<int> rcPos = pObj.m_rc;
					rcPos.mulC(m_nResizeFactorX, m_nResizeFactorY);

					float fy = m_hParameter.m_rcCheck.top*1.0f + rcPos.top + rcPos.Height() *1.0f / 2;
					float fx = m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX + rcPos.left + rcPos.Width() *1.0f / 2;

					float fdefectmetersy = __int64(fy + m_nStartline) * m_nCameraYResolution * 0.001 ;
					m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmetersy);
					float fdefectmetersx = (fx - m_nOriginalX) * m_nCameraXResolution * 0.001 + 1000;
					m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);

					float fdefectlines = __int64(fy + m_nStartline) * 1;
					m_hResult.mapFeaturelists[0][18] = std::make_pair("行号", fdefectlines);


					m_hResult.nDefectCount++;


					break;
				}

				if ((nType == 1) && (pObj.m_nDots*m_nResizeFactorX * m_nResizeFactorY  > m_hParameter.m_nGrayHighSmallDots && pObj.m_nEnergy > m_hParameter.m_nGrayHighSmallEnergy))
				{
					m_hResult.mapFeaturelists[0][0] = std::make_pair("点数", pObj.m_nDots*m_nResizeFactorX * m_nResizeFactorY *1.0f);
					m_hResult.mapFeaturelists[0][1] = std::make_pair("能量", pObj.m_nEnergy*1.0f);
					kxRect<int> rc = pObj.m_rc;
					rc.mulC(m_nResizeFactorX, m_nResizeFactorY);
					rc.left = gMax(rc.left - 10, 0);
					rc.top = gMax(rc.top - 10, 0);
					rc.right = gMin(rc.right + 10, m_WarpImg.nWidth - 1);
					rc.bottom = gMin(rc.bottom + 10, m_WarpImg.nHeight - 1);

					kxRect<int> rcdefect;
					rcdefect.left = max(0, rc.left - nExtend);
					rcdefect.top = max(0, rc.top - nExtend);
					rcdefect.right = min(m_WarpImg.nWidth - 1, rcdefect.left + rc.Width() + 2 * nExtend);
					rcdefect.bottom = min(m_WarpImg.nHeight - 1, rcdefect.top + rc.Height() + 2 * nExtend);
					//m_ImgDefect.SetImageBuf(m_WarpImg.buf + rcdefect.left*m_WarpImg.nChannel + rcdefect.top*m_WarpImg.nPitch, rcdefect.Width(), rcdefect.Height(), m_WarpImg.nPitch,
					//	m_WarpImg.nChannel, false);
					//DefectImg.Init(rcdefect.Width(), rcdefect.Height(), m_WarpImg.nChannel);
					//m_hFun.KxCopyImage(m_ImgWarpGray, DefectImg, rcdefect);
					DefectImg.Init(rcdefect.Width(), rcdefect.Height() * 2, m_ImgWarpGray.nChannel);
					IppiSize roiSize = { rcdefect.Width(), rcdefect.Height() };
					ippiCopy_8u_C1R(m_ImgWarpGray.buf + rcdefect.top * m_ImgWarpGray.nPitch + rcdefect.left*m_ImgWarpGray.nChannel, m_ImgWarpGray.nPitch,
						DefectImg.buf, DefectImg.nPitch, roiSize);
					kxRect<int> rcCopy = rcdefect;
					rcCopy.divC(m_nResizeFactorX, m_nResizeFactorY);
					kxCImageBuf m_ImgPyramidcc, m_ImgCcResize, m_ImgThresholdCc;
					m_ImgPyramidcc.Init(rcCopy.Width(), rcCopy.Height());
					m_hFun.KxCopyImage(m_ImgSlidCc[1], m_ImgPyramidcc, rcCopy);
					m_ImgCcResize.Init(rcdefect.Width(), rcdefect.Height());
					m_hFun.KxResizeImage(m_ImgPyramidcc, m_ImgCcResize);
					m_ImgThresholdCc.Init(rcdefect.Width(), rcdefect.Height());
					ippiCompareC_8u_C1R(m_ImgCcResize.buf, m_ImgCcResize.nPitch, 1, m_ImgThresholdCc.buf, m_ImgThresholdCc.nPitch, roiSize, ippCmpGreaterEq);
					ippiCopy_8u_C1R(m_ImgThresholdCc.buf, m_ImgThresholdCc.nPitch, DefectImg.buf + rcdefect.Height() * DefectImg.nPitch, DefectImg.nPitch, roiSize);


					//rc.offset(m_hParameter.m_rcCheck.left + m_nDx, m_hParameter.m_rcCheck.top + m_nDy);
					m_hResult.mapFeaturelists[0][2] = std::make_pair("X坐标", (rc.left-rcdefect.left)*1.0f);
					m_hResult.mapFeaturelists[0][3] = std::make_pair("Y坐标", (rc.top-rcdefect.top)*1.0f);
					m_hResult.mapFeaturelists[0][4] = std::make_pair("缺陷宽", rc.Width()*1.0f);
					m_hResult.mapFeaturelists[0][5] = std::make_pair("缺陷高", rc.Height()*1.0f);
					m_hResult.mapFeaturelists[0][6] = std::make_pair("类型", 1.0f);
					m_hResult.mapFeaturelists[0][7] = std::make_pair("块号", 1.0f * m_hParameter.m_nBlockIdx);
					
					m_hResult.mapFeaturelists[0][8] = std::make_pair("大图坐标X", m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX +rcdefect.left);
					m_hResult.mapFeaturelists[0][9] = std::make_pair("大图坐标Y", m_hParameter.m_rcCheck.top*1.0f + rcdefect.top);
					m_hResult.mapFeaturelists[0][10] = std::make_pair("块宽", rcdefect.Width()*1.0f);
					m_hResult.mapFeaturelists[0][11] = std::make_pair("块高", rcdefect.Height()*1.0f);

					rc.offset(m_hParameter.m_rcCheck.left, m_hParameter.m_rcCheck.top);
					m_hResult.mapFeaturelists[0][12] = std::make_pair("X原坐标", rc.left*1.0f + m_hParameter.m_nOffsetX);
					m_hResult.mapFeaturelists[0][13] = std::make_pair("Y原坐标", rc.top*1.0f);

					m_hResult.mapFeaturelists[0][14] = std::make_pair("块X坐标", rc.left*1.0f);
					m_hResult.mapFeaturelists[0][15] = std::make_pair("块Y坐标", rc.top*1.0f);

					//float fdefectmeters = (m_hResult.mapFeaturelists[0][9].second + m_nStartline) * m_nCameraYResolution * 0.001 ;
					//m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmeters);

					//float fdefectmetersx = (m_hResult.mapFeaturelists[0][8].second - m_nOriginalX) * m_nCameraYResolution * 0.001 + 1000;
					//m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);

					kxRect<int> rcPos = pObj.m_rc;
					rcPos.mulC(m_nResizeFactorX, m_nResizeFactorY);

					float fy = m_hParameter.m_rcCheck.top*1.0f + rcPos.top + rcPos.Height() *1.0f / 2;
					float fx = m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX + rcPos.left + rcPos.Width() *1.0f / 2;

					float fdefectmetersy = __int64(fy + m_nStartline) * m_nCameraYResolution * 0.001;
					m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmetersy);
					float fdefectmetersx = (fx - m_nOriginalX) * m_nCameraXResolution * 0.001 + 1000;
					m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);

					float fdefectlines = __int64(fy + m_nStartline) * 1;
					m_hResult.mapFeaturelists[0][18] = std::make_pair("行号", fdefectlines);


					m_hResult.nDefectCount++;
					break;
				}

			}
		}
	}

	//if (m_nAlgorithmStatus[2])
	//{
	//	if (m_hResult.nDefectCount == 0)
	//	{
	//		int nStatus = m_hGradBlobAnaly.ToBlobParallel(m_ImgLineMax, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, nMergeSize, 0, hCall);
	//		if (nStatus == 0)
	//		{
	//			m_hResult.nStatus = _FATAL_Err;
	//			m_hResult.szErrInfo = "Layer2BlobErr";
	//			return 0;
	//		}
	//		int nCount = (std::min)((int)_Max_BlobCount, m_hGradBlobAnaly.GetBlobCount());

	//		for (int i = 0; i < nCount; i++)
	//		{
	//			CKxBlobAnalyse::SingleBlobInfo& pObj = m_hGradBlobAnaly.GetSortSingleBlob(i);
	//			if (pObj.m_nDots*m_nResizeFactorX * m_nResizeFactorY / 2 > m_hParameter.m_nGradientSmallDots && pObj.m_nEnergy > m_hParameter.m_nGradientSmallEnergy)
	//			{
	//				m_hResult.mapFeaturelists[0][0] = std::make_pair("点数", pObj.m_nDots*m_nResizeFactorX * m_nResizeFactorY / 2.0f);
	//				m_hResult.mapFeaturelists[0][1] = std::make_pair("能量", pObj.m_nEnergy*1.0f);
	//				kxRect<int> rc = pObj.m_rc;
	//				rc.mulC(m_nResizeFactorX * 4, m_nResizeFactorY*4);
	//				rc.left = gMax(rc.left - 10, 0);
	//				rc.top = gMax(rc.top - 10, 0);
	//				rc.right = gMin(rc.right + 10, m_WarpImg.nWidth - 1);
	//				rc.bottom = gMin(rc.bottom + 10, m_WarpImg.nHeight - 1);

	//				kxRect<int> rcdefect;
	//				rcdefect.left = max(0, rc.left - nExtend);
	//				rcdefect.top = max(0, rc.top - nExtend);
	//				rcdefect.right = min(m_WarpImg.nWidth - 1, rcdefect.left + rc.Width() + 2 * nExtend);
	//				rcdefect.bottom = min(m_WarpImg.nHeight - 1, rcdefect.top + rc.Height() + 2 * nExtend);
	//				//m_ImgDefect.SetImageBuf(m_WarpImg.buf + rcdefect.left*m_WarpImg.nChannel + rcdefect.top*m_WarpImg.nPitch, rcdefect.Width(), rcdefect.Height(), m_WarpImg.nPitch,
	//				//	m_WarpImg.nChannel, false);
	//				DefectImg.Init(rcdefect.Width(), rcdefect.Height(), m_WarpImg.nChannel);
	//				m_hFun.KxCopyImage(m_ImgWarpGray, DefectImg, rcdefect);


	//				//rc.offset(m_hParameter.m_rcCheck.left + m_nDx, m_hParameter.m_rcCheck.top + m_nDy);
	//				m_hResult.mapFeaturelists[0][2] = std::make_pair("X坐标", (rc.left - rcdefect.left)*1.0f);
	//				m_hResult.mapFeaturelists[0][3] = std::make_pair("Y坐标", (rc.top - rcdefect.top)*1.0f);
	//				m_hResult.mapFeaturelists[0][4] = std::make_pair("缺陷宽", rc.Width()*1.0f);
	//				m_hResult.mapFeaturelists[0][5] = std::make_pair("缺陷高", rc.Height()*1.0f);
	//				m_hResult.mapFeaturelists[0][6] = std::make_pair("类型", 2.0f);
	//				m_hResult.mapFeaturelists[0][7] = std::make_pair("块号", 1.0f * m_hParameter.m_nBlockIdx);
	//				
	//				m_hResult.mapFeaturelists[0][8] = std::make_pair("大图坐标X", m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX + rcdefect.left);
	//				m_hResult.mapFeaturelists[0][9] = std::make_pair("大图坐标Y", m_hParameter.m_rcCheck.top*1.0f + rcdefect.top);
	//				m_hResult.mapFeaturelists[0][10] = std::make_pair("块宽", rcdefect.Width()*1.0f);
	//				m_hResult.mapFeaturelists[0][11] = std::make_pair("块高", rcdefect.Height()*1.0f);

	//				rc.offset(m_hParameter.m_rcCheck.left, m_hParameter.m_rcCheck.top);
	//				m_hResult.mapFeaturelists[0][12] = std::make_pair("X原坐标", rc.left*1.0f + m_hParameter.m_nOffsetX);
	//				m_hResult.mapFeaturelists[0][13] = std::make_pair("Y原坐标", rc.top*1.0f);

	//				m_hResult.mapFeaturelists[0][14] = std::make_pair("块X坐标", rc.left*1.0f);
	//				m_hResult.mapFeaturelists[0][15] = std::make_pair("块Y坐标", rc.top*1.0f);

	//				//float fdefectmeters = (m_hResult.mapFeaturelists[0][9].second + m_nStartline) * m_nCameraYResolution * 0.001 ;
	//				//m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmeters);

	//				//float fdefectmetersx = (m_hResult.mapFeaturelists[0][8].second - m_nOriginalX) * m_nCameraYResolution * 0.001 + 1000;
	//				//m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);

	//				kxRect<int> rcPos = pObj.m_rc;
	//				rcPos.mulC(m_nResizeFactorX * 4, m_nResizeFactorY * 4);

	//				float fy = m_hParameter.m_rcCheck.top*1.0f + rcPos.top + rcPos.Height() *1.0f / 2;
	//				float fx = m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX + rcPos.left + rcPos.Width() *1.0f / 2;

	//				float fdefectmetersy = __int64(fy + m_nStartline) * m_nCameraYResolution * 0.001;
	//				m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmetersy);
	//				float fdefectmetersx = (fx - m_nOriginalX) * m_nCameraXResolution * 0.001 + 1000;
	//				m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);


	//				float fdefectlines = __int64(fy + m_nStartline) * 1;
	//				m_hResult.mapFeaturelists[0][18] = std::make_pair("行号", fdefectlines);



	//				m_hResult.nDefectCount++;
	//				break;
	//			}
	//		}
	//	}
	//}



	if (m_hResult.nDefectCount == 0)
	{
		m_hResult.nStatus = _Check_Ok;
	}
	else
	{
		m_hResult.nStatus = _Check_Err;
	}

	return 1;
}



int CKxSurfaceCheck::AnalysisCheckStatus(int nImageType)
{
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_nAlgorithmStatus[i] = 0;
	}

	//灰度检低
	if (m_hParameter.m_nGrayLowOffset < _MAX_OFFSET)
	{
		m_nAlgorithmStatus[0] = 1;
		m_hParameter.m_nGrayLowOffset = 105 + m_hParameter.m_nGrayLowOffset * 15;
	}

	//灰度检高
	if (m_hParameter.m_nGrayHighOffset < _MAX_OFFSET)
	{
		m_nAlgorithmStatus[1] = 1;
		m_hParameter.m_nGrayHighOffset = m_hParameter.m_nGrayHighOffset * 20;
	}

	
	if (m_hParameter.m_nLineOffset < _MAX_OFFSET)
	{
		for (int i = 2; i < _Max_Layer; i++)
		{
			m_nAlgorithmStatus[i] = 1;
		}
		m_hParameter.m_nLineOffset *= 4;	
	}


	if (m_hParameter.m_nIsLearn)
	{
		m_nLearnStatus = 1;
	}

	return 1;
}

int CKxSurfaceCheck::GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg, kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy)
{
	kxRect<int> rcSearch;
	int nleft, nright, ntop, nbottom;
	if (nSearchDir == _Horiz_Vertical_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft() - nSearchExpand);
		ntop = gMax(0, rcKern.GetTop() - nSearchExpand);
		nright = gMin(SrcImg.nWidth - 1, rcKern.GetRight() + nSearchExpand);
		nbottom = gMin(SrcImg.nHeight - 1, rcKern.GetBottom() + nSearchExpand);
	}
	else if (nSearchDir == _Vertical_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft());
		ntop = gMax(0, rcKern.GetTop() - nSearchExpand);
		nright = gMin(SrcImg.nWidth - 1, rcKern.GetRight());
		nbottom = gMin(SrcImg.nHeight - 1, rcKern.GetBottom() + nSearchExpand);
	}
	else if (nSearchDir == _Horizontal_Dir)
	{
		nleft = gMax(0, rcKern.GetLeft() - nSearchExpand);
		ntop = gMax(0, rcKern.GetTop());
		nright = gMin(SrcImg.nWidth - 1, rcKern.GetRight() + nSearchExpand);
		nbottom = gMin(SrcImg.nHeight - 1, rcKern.GetBottom());
	}
	else
	{
		//直接拷贝,不定位
		nDx = 0;
		nDy = 0;
		return 1;
	}

	rcSearch.setup(nleft, ntop, nright, nbottom);
	//-------------------------------------------------------------------------------------------------
	kxPoint<float>  pos;

#ifdef ColorImageAlign
	m_ImgSearch.SetImageBuf(SrcImg.buf + rcSearch.GetTop()*SrcImg.nPitch + rcSearch.GetLeft()*SrcImg.nChannel, rcSearch.Width(),
		rcSearch.Height(), SrcImg.nPitch, SrcImg.nChannel, false);
	//m_ImgResizeSuper.Init(m_ImgSearch.nWidth / 2, m_ImgSearch.nHeight / 2, m_ImgSearch.nChannel);
	//m_hFun.KxResizeImage(m_ImgSearch, m_ImgResizeSuper, KxSuper);
#else
	m_ImgSearch.Init(rcSearch.Width(), rcSearch.Height());
	//IppiSize Roi = { rcSearch.Width(), rcSearch.Height() };
	//ippiRGBToGray_8u_C3C1R(SrcImg.buf + rcSearch.GetTop()*SrcImg.nPitch + rcSearch.GetLeft()*SrcImg.nChannel, SrcImg.nPitch, 
	//	m_ImgSearch.buf, m_ImgSearch.nPitch, Roi);

	kxCImageBuf imgtest;
	imgtest.SetImageBuf(SrcImg.buf + rcSearch.top*SrcImg.nPitch + rcSearch.left * SrcImg.nChannel, rcSearch.Width(),
		rcSearch.Height(), SrcImg.nPitch, SrcImg.nChannel, false);
	convertcolorimage2gray(imgtest, m_ImgSearch, m_nBestLocationLayer);

	//m_ImgResizeSuper.Init(m_ImgSearch.nWidth / 2, m_ImgSearch.nHeight / 2, m_ImgSearch.nChannel);
	//m_hFun.KxResizeImage(m_ImgSearch, m_ImgResizeSuper, KxSuper);

	if (m_hParameter.m_nSearchDir == _Horiz_Vertical_Dir)   //水平垂直搜索
	{
		m_ImgResizeSuper.Init(m_ImgSearch.nWidth / 2, m_ImgSearch.nHeight / 2, m_ImgSearch.nChannel);
		m_hFun.KxResizeImage(m_ImgSearch, m_ImgResizeSuper, KxLinear);

	}
	else if (m_hParameter.m_nSearchDir == _Vertical_Dir)
	{
		m_ImgResizeSuper.Init(1, m_ImgSearch.nHeight, m_ImgSearch.nChannel);
		m_hFun.KxResizeImage(m_ImgSearch, m_ImgResizeSuper, KxSuper);
	}
	else if (m_hParameter.m_nSearchDir == _Horizontal_Dir)
	{
		m_ImgResizeSuper.Init(m_ImgSearch.nWidth, 1, m_ImgSearch.nChannel);
		m_hFun.KxResizeImage(m_ImgSearch, m_ImgResizeSuper, KxSuper);
	}

	//m_ImgResizeSuper.Init(m_ImgSearch.nWidth / 2, m_ImgSearch.nHeight / 2, m_ImgSearch.nChannel);
	//m_hFun.KxResizeImage(m_ImgSearch, m_ImgResizeSuper, KxSuper);
#endif

	//float fratio = m_hFun.kxImageAlignColor(pos, m_ImgResizeSuper, KernImg);
	float fratio = m_hFun.kxImageAlign(pos, m_ImgResizeSuper, KernImg);

	if (fratio < gMinSimilarity)
	{
		nDx = 0;
		nDy = 0;
		return 0;
	}
	//nDx = int(rcSearch.GetLeft() + pos.x *2 - rcKern.left );
	//nDy = int(rcSearch.GetTop() + pos.y *2- rcKern.top);

	if (m_hParameter.m_nSearchDir == _Horiz_Vertical_Dir)
	{
		nDx = int(rcSearch.GetLeft() + pos.x * 2 - rcKern.left);
		nDy = int(rcSearch.GetTop() + pos.y * 2 - rcKern.top);
	}
	else if (m_hParameter.m_nSearchDir == _Vertical_Dir)
	{
		nDx = 0;
		nDy = int(rcSearch.GetTop() + pos.y * 1 - rcKern.top);
	}
	else
	{
		nDx = int(rcSearch.GetLeft() + pos.x * 1 - rcKern.left);
		nDy = 0;
	}


	return 1;
}




int CKxSurfaceCheck::Check(const kxCImageBuf& ImgCheck, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	if (m_nIsLoadParamsSuccess == 0)  //参数加载失败直接不检查
	{
		return 1;
	}

	if (m_nIsStandardChange)   //下次生效
	{
		m_hParameter.m_nGrayLowSmallDots    = m_nStandard[0];
		m_hParameter.m_nGrayHighSmallDots   = m_nStandard[1];
		m_hParameter.m_nGradientSmallDots   = m_nStandard[2];
		m_nIsStandardChange = 0;
	}

	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	hCall.Clear();
	tick_count tbb_start, tbb_end;
	m_hResult.Clear();

	tbb_start = tick_count::now();

	m_ImgSrc.SetImageBuf(ImgCheck.buf, ImgCheck.nWidth, ImgCheck.nHeight, ImgCheck.nPitch, ImgCheck.nChannel, false);
	

	kxRect<int> rckernel = m_hParameter.m_rcKernel;
	rckernel.offset(m_nBigImgOffsetX, 0);
	rckernel.left = (std::max)(rckernel.left, 2 * m_hParameter.m_nExpand + 1);
	rckernel.right = (std::min)(rckernel.right, m_ImgSrc.nWidth - 1 - 2 * m_hParameter.m_nExpand);


	m_nDy = 0;
	m_nDx = 0;
	int nStatus = GetImgOffset(m_ImgSrc, m_ImgKernel, rckernel, 2*m_hParameter.m_nExpand, m_hParameter.m_nSearchDir, m_nDx, m_nDy);
	if (nStatus == 0)
	{

		DstImg.Init(m_hParameter.m_rcCheck.Width(), m_hParameter.m_rcCheck.Height() * 2, ImgCheck.nChannel);
		ippsSet_8u(255, DstImg.buf, DstImg.nPitch * DstImg.nHeight);
		IppiSize roiSize = { m_hParameter.m_rcCheck.Width(), m_hParameter.m_rcCheck.Height() };
		ippiCopy_8u_C1R(ImgCheck.buf + m_hParameter.m_rcCheck.left * ImgCheck.nChannel + m_hParameter.m_rcCheck.top * ImgCheck.nPitch, ImgCheck.nPitch,
			DstImg.buf, DstImg.nPitch, roiSize);
		

		m_hResult.nDefectCount = 1;
		m_hResult.nStatus = _Similarity_Err;
		m_hResult.mapFeaturelists[0][0] = std::make_pair("点数", 1000000 * 1.0f);
		m_hResult.mapFeaturelists[0][1] = std::make_pair("能量", 1000000 * 1.0f);
		kxRect<int> rc;
		rc.setup(0, 0, DstImg.nWidth - 1, DstImg.nHeight - 1);

		m_hResult.mapFeaturelists[0][2] = std::make_pair("X坐标", rc.left*1.0f);
		m_hResult.mapFeaturelists[0][3] = std::make_pair("Y坐标", rc.top*1.0f);
		m_hResult.mapFeaturelists[0][4] = std::make_pair("缺陷宽", rc.Width()*1.0f);
		m_hResult.mapFeaturelists[0][5] = std::make_pair("缺陷高", rc.Height()*1.0f);
		m_hResult.mapFeaturelists[0][6] = std::make_pair("类型", 3.0f);  //定位点丢失缺陷
		m_hResult.mapFeaturelists[0][7] = std::make_pair("块号", 1.0f * m_hParameter.m_nBlockIdx);
		rc.offset(m_hParameter.m_rcCheck.left, m_hParameter.m_rcCheck.top);
		m_hResult.mapFeaturelists[0][8] = std::make_pair("大图坐标X", m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX);
		m_hResult.mapFeaturelists[0][9] = std::make_pair("大图坐标Y", m_hParameter.m_rcCheck.top*1.0f);
		m_hResult.mapFeaturelists[0][10] = std::make_pair("块宽", m_hParameter.m_rcCheck.Width()*1.0f);
		m_hResult.mapFeaturelists[0][11] = std::make_pair("块高", m_hParameter.m_rcCheck.Height()*1.0f);
		m_hResult.mapFeaturelists[0][12] = std::make_pair("X原坐标", rc.left*1.0f + m_hParameter.m_nOffsetX);
		m_hResult.mapFeaturelists[0][13] = std::make_pair("Y原坐标", rc.top*1.0f);
		m_hResult.mapFeaturelists[0][14] = std::make_pair("块X坐标", rc.left*1.0f);
		m_hResult.mapFeaturelists[0][15] = std::make_pair("块Y坐标", rc.top*1.0f);

		float fdefectmeters = ((m_hParameter.m_rcCheck.top + m_hParameter.m_rcCheck.bottom)/2 + m_nStartline) * m_nCameraYResolution * 0.001;
		m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmeters);

		float fdefectmetersx = ((m_hParameter.m_rcCheck.left + m_hParameter.m_rcCheck.right) / 2 * 1.0f + m_hParameter.m_nOffsetX - m_nOriginalX) * m_nCameraXResolution * 0.001 + 1000;
		m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);

		float fdefectlines = __int64((m_hParameter.m_rcCheck.top + m_hParameter.m_rcCheck.bottom) / 2 + m_nStartline) * 1;
		m_hResult.mapFeaturelists[0][18] = std::make_pair("行号", fdefectlines);

		return 0;
	}


	kxRect<int> rcCopyBig = m_hParameter.m_rcCheck;
	rcCopyBig.offset(m_nDx + m_nBigImgOffsetX, m_nDy);

	rcCopyBig.left = (std::max)(rcCopyBig.left, 0);
	rcCopyBig.right = (std::min)(rcCopyBig.right, m_ImgSrc.nWidth-1);
	rcCopyBig.top = (std::max)(rcCopyBig.top, 0);
	rcCopyBig.bottom = (std::min)(rcCopyBig.bottom, m_ImgSrc.nHeight - 1);

	m_WarpImg.SetImageBuf(m_ImgSrc.buf + rcCopyBig.top * m_ImgSrc.nPitch + rcCopyBig.left * m_ImgSrc.nChannel, rcCopyBig.Width(), rcCopyBig.Height(),
		m_ImgSrc.nPitch, m_ImgSrc.nChannel, false);
	m_ImgWarpGray.SetImageBuf(ImgCheck.buf + rcCopyBig.top * ImgCheck.nPitch + rcCopyBig.left * ImgCheck.nChannel, rcCopyBig.Width(), rcCopyBig.Height(),
		ImgCheck.nPitch, ImgCheck.nChannel, false);


	tbb_end = tick_count::now();
	//if (m_hParameter.m_nBlockIdx == 0)
		//printf(" -----copy cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);

	tbb_start = tick_count::now();
	//Process
	ProcessSurfaceWarpImages(hCallInfo);
	if (check_sts(hCallInfo, "处理中_", hCall))
	{
		SetCheckExceptionStatus(DstImg);
		return 0;
	}

	tbb_end = tick_count::now();
	//if (m_hParameter.m_nBlockIdx == 0)
		//printf(" -----process cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);


	//Thirdly, learn a image
	tbb_start = tick_count::now();

	LearnTemplate(hCallInfo);
	if (check_sts(hCallInfo, "学习中_", hCall))
	{
		SetCheckExceptionStatus(DstImg);
		return 0;
	}

	tbb_end = tick_count::now();
	//if (m_hParameter.m_nBlockIdx == 0)
		//printf("-----learn cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);


	//Fourthly,compute a sub image
	tbb_start = tick_count::now();

	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "比较中_", hCall))
	{
		SetCheckExceptionStatus(DstImg);
		return 0;
	}

	tbb_end = tick_count::now();
	//if (m_hParameter.m_nBlockIdx == 0)
		//printf(" -----slidercc cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);


	//fifth,blob Analysis
	tbb_start = tick_count::now();

	ConnectAnalysis(ImgCheck, DstImg, hCallInfo);
	if (check_sts(hCallInfo, "分析中_", hCall))
	{
		SetCheckExceptionStatus(DstImg);
		return 0;
	}


	tbb_end = tick_count::now();
	//if (m_hParameter.m_nBlockIdx == 0)
		//printf(" -----blob cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);
	//SaveMidImage(true);

	//crop a defect image
	//DstImg.SetImageBuf(m_ImgDefect.buf, m_ImgDefect.nWidth, m_ImgDefect.nHeight,
	//	m_ImgDefect.nPitch, m_ImgDefect.nChannel, true);


	return 1;

}




int CKxSurfaceCheck::SetCheckExceptionStatus(kxCImageBuf& ImgCheck)
{
	m_hResult.nDefectCount = 1;
	m_hResult.nStatus = _FATAL_Err;
	m_hResult.mapFeaturelists[0][0] = std::make_pair("点数", 1000000*1.0f);
	m_hResult.mapFeaturelists[0][1] = std::make_pair("能量", 1000000*1.0f);
	kxRect<int> rc;
	rc.setup(0, 0, m_ImgWarpGray.nWidth - 1, m_ImgWarpGray.nHeight - 1);
	//ImgCheck.Init(rc.Width(), rc.Height(), m_WarpImg.nChannel);
	//m_hFun.KxCopyImage(m_WarpImg, ImgCheck, rc);
	//ImgCheck.SetImageBuf(m_ImgWarpGray.buf, m_ImgWarpGray.nWidth, m_ImgWarpGray.nHeight,
	//	m_ImgWarpGray.nPitch, m_ImgWarpGray.nChannel, true);


	ImgCheck.Init(rc.Width(), rc.Height() * 2, m_ImgWarpGray.nChannel);
	ippsSet_8u(255, ImgCheck.buf, ImgCheck.nPitch * ImgCheck.nHeight);
	IppiSize roiSize = { rc.Width(), rc.Height() };
	ippiCopy_8u_C1R(m_ImgWarpGray.buf + rc.left * m_ImgWarpGray.nChannel + rc.top * m_ImgWarpGray.nPitch, m_ImgWarpGray.nPitch,
		ImgCheck.buf, ImgCheck.nPitch, roiSize);


	//rc.offset(m_hParameter.m_rcCheck.left + m_nDx, m_hParameter.m_rcCheck.top + m_nDy);
	m_hResult.mapFeaturelists[0][2] = std::make_pair("X坐标", rc.left*1.0f);
	m_hResult.mapFeaturelists[0][3] = std::make_pair("Y坐标", rc.top*1.0f);
	m_hResult.mapFeaturelists[0][4] = std::make_pair("缺陷宽", rc.Width()*1.0f);
	m_hResult.mapFeaturelists[0][5] = std::make_pair("缺陷高", rc.Height()*1.0f);
	m_hResult.mapFeaturelists[0][6] = std::make_pair("类型", 100.0f);  //致命缺陷

	m_hResult.mapFeaturelists[0][7] = std::make_pair("块号", 1.0f * m_hParameter.m_nBlockIdx);
	rc.offset(m_hParameter.m_rcCheck.left, m_hParameter.m_rcCheck.top);
	m_hResult.mapFeaturelists[0][8] = std::make_pair("大图坐标X", m_hParameter.m_rcCheck.left*1.0f + m_hParameter.m_nOffsetX);
	m_hResult.mapFeaturelists[0][9] = std::make_pair("大图坐标Y", m_hParameter.m_rcCheck.top*1.0f);
	m_hResult.mapFeaturelists[0][10] = std::make_pair("块宽", m_hParameter.m_rcCheck.Width()*1.0f);
	m_hResult.mapFeaturelists[0][11] = std::make_pair("块高", m_hParameter.m_rcCheck.Height()*1.0f);
	m_hResult.mapFeaturelists[0][12] = std::make_pair("X原坐标", rc.left*1.0f + m_hParameter.m_nOffsetX);
	m_hResult.mapFeaturelists[0][13] = std::make_pair("Y原坐标", rc.top*1.0f);
	m_hResult.mapFeaturelists[0][14] = std::make_pair("块X坐标", rc.left*1.0f);
	m_hResult.mapFeaturelists[0][15] = std::make_pair("块Y坐标", rc.top*1.0f);
	//float fdefectmeters = (m_hResult.mapFeaturelists[0][9].second + m_nStartline) * m_nCameraYResolution * 0.001;
	//m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmeters);
	//float fdefectmetersx = (m_hResult.mapFeaturelists[0][8].second - m_nOriginalX) * m_nCameraYResolution * 0.001 + 1000;
	//m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);

	float fdefectmeters = ((m_hParameter.m_rcCheck.top + m_hParameter.m_rcCheck.bottom) / 2 + m_nStartline) * m_nCameraYResolution * 0.001;
	m_hResult.mapFeaturelists[0][16] = std::make_pair("POSY(mm)", fdefectmeters);
	float fdefectmetersx = ((m_hParameter.m_rcCheck.left + m_hParameter.m_rcCheck.right) / 2 * 1.0f + m_hParameter.m_nOffsetX - m_nOriginalX) * m_nCameraXResolution * 0.001 + 1000;
	m_hResult.mapFeaturelists[0][17] = std::make_pair("POSX(mm)", fdefectmetersx);

	float fdefectlines = __int64((m_hParameter.m_rcCheck.top + m_hParameter.m_rcCheck.bottom) / 2 + m_nStartline) * 1;
	m_hResult.mapFeaturelists[0][18] = std::make_pair("行号", fdefectlines);

	return 1;
}


int CKxSurfaceCheck::SetDefectLearnStatus(const kxCImageBuf& srcImg, kxRect<int>& rcDefect, std::string szModelDir)
{
	//assert(srcImg.nChannel == 3);
	//m_ImgOnlineDefectLearnGray.Init(srcImg.nWidth / m_nResizeFactor, srcImg.nHeight / m_nResizeFactor, srcImg.nChannel);
	//m_hDefectLearnFun.KxResizeImage(srcImg, m_ImgOnlineDefectLearnGray, KxSuper);
	//KxCallStatus hCall;
	//for (int i = 2; i < _Max_Layer; i++)
	//{
	//	int nStatus = m_hDefectLearnGradient.SingleDirGradient(m_ImgOnlineDefectLearnGray, m_ImgOnlineDefectLearnGradient[i - 2], i - 2, 4, 255, 5, hCall);
	//	if (nStatus == 0)
	//	{
	//		return -1;
	//	}
	//}
	//
	//m_rcDefectGray = rcDefect;
	//m_rcDefectGray.divC(m_nResizeFactor);

	//m_rcDefectGray.left = gMax(0, m_rcDefectGray.left - 4*m_nResizeFactor);
	//m_rcDefectGray.top = gMax(0, m_rcDefectGray.top - 4*m_nResizeFactor);
	//m_rcDefectGray.right = gMin(m_ImgOnlineDefectLearnGray.nWidth - 1, m_rcDefectGray.right + 4 * m_nResizeFactor);
	//m_rcDefectGray.bottom = gMin(m_ImgOnlineDefectLearnGray.nHeight - 1, m_rcDefectGray.bottom + 4 * m_nResizeFactor);

	//
	//m_rcDefectGradient = m_rcDefectGray;
	//m_rcDefectGradient.divC(4);
	// 

	//m_rcDefectGradient.left = gMax(0, m_rcDefectGradient.left - 16);
	//m_rcDefectGradient.top = gMax(0, m_rcDefectGradient.top - 16);
	//m_rcDefectGradient.right = gMin(m_ImgOnlineDefectLearnGradient[0].nWidth - 1, m_rcDefectGradient.right + 16);
	//m_rcDefectGradient.bottom = gMin(m_ImgOnlineDefectLearnGradient[0].nHeight - 1, m_rcDefectGradient.bottom + 16);


	//if (m_ImgTemplate[0].nWidth == 0 || m_ImgTemplate[1].nHeight == 0)
	//{
	//	return -2;
	//}

	//assert(m_ImgOnlineDefectLearnGray.nChannel == 3 && m_ImgTemplate[0].nChannel == 3);

	//for (int i = 0; i < _Max_Layer; i++)
	//{
	//	m_ImgLearnTemplate[i].Init(m_ImgTemplate[i].nWidth, m_ImgTemplate[i].nHeight, m_ImgTemplate[i].nChannel);
	//	IppiSize roiSize = { m_ImgTemplate[i].nWidth, m_ImgTemplate[i].nHeight };
	//	ippiCopy_8u_C3R(m_ImgTemplate[i].buf, m_ImgTemplate[i].nPitch, m_ImgLearnTemplate[i].buf, m_ImgLearnTemplate[i].nPitch, roiSize);
	//}

	////灰度学低
	//IppiSize GrayroiSize = { m_rcDefectGray.Width(), m_rcDefectGray.Height() };
	//if (m_rcDefectGray.left < 0 || m_rcDefectGray.top < 0 || (m_rcDefectGray.left + GrayroiSize.width - 1 >= m_ImgOnlineDefectLearnGray.nWidth) ||
	//	(m_rcDefectGray.top + GrayroiSize.height - 1 >= m_ImgOnlineDefectLearnGray.nHeight))
	//{
	//	return -3;
	//}
	//ippiMinEvery_8u_C3IR(m_ImgOnlineDefectLearnGray.buf + m_rcDefectGray.top*m_ImgOnlineDefectLearnGray.nPitch + m_rcDefectGray.left * m_ImgOnlineDefectLearnGray.nChannel, m_ImgOnlineDefectLearnGray.nPitch,
	//	m_ImgLearnTemplate[0].buf + m_rcDefectGray.top*m_ImgLearnTemplate[0].nPitch + m_rcDefectGray.left * m_ImgLearnTemplate[0].nChannel, m_ImgLearnTemplate[0].nPitch, GrayroiSize);
	////灰度学高
	//ippiMaxEvery_8u_C3IR(m_ImgOnlineDefectLearnGray.buf + m_rcDefectGray.top*m_ImgOnlineDefectLearnGray.nPitch + m_rcDefectGray.left * m_ImgOnlineDefectLearnGray.nChannel, m_ImgOnlineDefectLearnGray.nPitch,
	//	m_ImgLearnTemplate[1].buf + m_rcDefectGray.top*m_ImgLearnTemplate[1].nPitch + m_rcDefectGray.left * m_ImgLearnTemplate[1].nChannel, m_ImgLearnTemplate[1].nPitch, GrayroiSize);

	////梯度学高
	//IppiSize GradientroiSize = { m_rcDefectGradient.Width(), m_rcDefectGradient.Height() };
	//if (m_rcDefectGradient.left < 0 || m_rcDefectGradient.top < 0 || (m_rcDefectGradient.left + GradientroiSize.width - 1 >= m_ImgOnlineDefectLearnGradient[0].nWidth) ||
	//	(m_rcDefectGradient.top + GradientroiSize.height - 1 >= m_ImgOnlineDefectLearnGradient[0].nHeight))
	//{		
	//	return -4;
	//}

	//for (int i = 2; i < _Max_Layer; i++)
	//{
	//	ippiMaxEvery_8u_C3IR(m_ImgOnlineDefectLearnGradient[i - 2].buf + m_rcDefectGradient.top*m_ImgOnlineDefectLearnGradient[i - 2].nPitch + m_rcDefectGradient.left * m_ImgOnlineDefectLearnGradient[i - 2].nChannel, m_ImgOnlineDefectLearnGradient[i - 2].nPitch,
	//		m_ImgLearnTemplate[i].buf + m_rcDefectGradient.top*m_ImgLearnTemplate[i].nPitch + m_rcDefectGradient.left * m_ImgLearnTemplate[i].nChannel, m_ImgLearnTemplate[i].nPitch, GradientroiSize);
	//}

	//m_nIsDefectLearn = 1;

	//FILE*   fp;
	//if (fopen_s(&fp, szModelDir.c_str(), "wb") != 0)
	//{
	//	return -5;
	//}
	//m_nModelNumber = _Max_Layer;
	//if (fwrite(&m_nModelNumber, sizeof(int), 1, fp) != 1)
	//	return 0;

	//switch (m_nModelNumber)
	//{
	//case _Max_Layer:
	//	for (int i = 0; i < m_nModelNumber; i++)
	//	{
	//		if (FALSE == m_ImgLearnTemplate[i].Write(fp))
	//			return 0;
	//	}
	//	break;
	//default:
	//	return 0;
	//}

	//fclose(fp);

	return 1;
}