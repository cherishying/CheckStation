
#include "KxSignatureCheckBeta.h"
#include "KxReadXml.h"

CKxSignatureCheckBeta::CKxSignatureCheckBeta()
{

	memset(m_nLearnHighLowMode, 0, sizeof(int)*_Max_Layer);

	for (int i = 0; i < _Max_Layer; i++)
	{
		m_bIsLearnCompleted[i] = false;
	}
	m_bAllCompleted = false;

	m_nLearnPage = 50;

	memset(m_nAlgorithmStatus, 0, sizeof(int)* _Max_Layer);

	m_nLearnStatus = 0;

	m_nSignatureEdgeThresh = 60;  //用于搜边
	m_nSignatureBorderCheckThresh = 60;

	m_nSearchEdgeThresh = 50;

	m_nSmoothSize = 9;

	m_nSignatureSizeThresh = 10;

	m_nSignatureExtendLen = 10;

	m_nUseXml = 0;

	m_nSignatureModelWidth = 0;
	m_nSignatureModelHeight = 0;

	m_nOpenVerticalSize = 1;
}

CKxSignatureCheckBeta::~CKxSignatureCheckBeta()
{
}


bool CKxSignatureCheckBeta::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
{
	m_nUseXml = 1;
	//检测控制参数
	if (!ReadParaXml(filePath))
		return false;
	//距离标定参数
	if (!m_hCalDistance.ReadXml(filePath, BaseImg))
	{
		return false;
	}
	//图像校正参数
	if (!m_hWarpStrech.ReadXml(filePath, BaseImg))
	{
		return false;
	}
	//色差检查参数
	if (!m_hColorDiff.ReadXml(filePath, BaseImg))
	{
		return false;
	}
	//掩膜参数
	if (!m_hImgMask.ReadXml(filePath, BaseImg))
	{
		return false;
	}

	kxRect<int> rcCopy;
	rcCopy.setup(0, 0, BaseImg.nWidth - 1, BaseImg.nHeight - 1);
	m_BaseImg.Init(rcCopy.Width(), rcCopy.Height(), BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(BaseImg, m_BaseImg, rcCopy);

	//生成自动Mask
	kxRect<int>& rc = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_CreateMask.Init(rc.Width(), rc.Height(), m_BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(m_BaseImg, m_CreateMask, rc);
	m_hImgMask.AutoCreateMask(m_CreateMask);

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

	AnalysisCheckStatus();

	if (m_nIsCreateBaseImg)
	{

		for (int i = 0; i < 2; i++)
		{
			m_Offset[i].x = m_hParameter.m_Point[i].x - m_hParameter.m_Point[0].x;
			m_Offset[i].y = m_hParameter.m_Point[i].y - m_hParameter.m_Point[0].y;
		}
		m_nOffsetCount = m_hParameter.m_nPointCount;
		m_OffsetMin = m_OffsetMax = m_Offset[0];
		for (int i = 1; i < m_nOffsetCount; i++)
		{
			if (m_Offset[i].x < m_OffsetMin.x)
				m_OffsetMin.x = m_Offset[i].x;
			if (m_Offset[i].y < m_OffsetMin.y)
				m_OffsetMin.y = m_Offset[i].y;
			if (m_Offset[i].x > m_OffsetMax.x)
				m_OffsetMax.x = m_Offset[i].x;
			if (m_Offset[i].y > m_OffsetMax.y)
				m_OffsetMax.y = m_Offset[i].y;
		}


		kxRect<int>& rcCheckArea = m_hWarpStrech.GetParameter().m_rcCheckArea;
		m_WarpImg.Init(rcCheckArea.Width(), rcCheckArea.Height(), m_BaseImg.nChannel);
		//m_hBaseFun.SaveBMPImage_h("d:\\Base.bmp", m_BaseImg);
		KxCallStatus hCall;
		hCall.Clear();
		int nStatus = m_hWarpStrech.Check(m_BaseImg, m_WarpImg, hCall);
		if (nStatus)
		{
			return 0;
		}
		Matting(m_WarpImg, true);

	}


	return true;

}

bool CKxSignatureCheckBeta::ReadParaXml(const char* filePath)
{
	//重要参数
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "学习", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsLearn);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "点灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nSmallDotOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "缺失灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nLossOffset);
	if (!nStatus)
	{
		return false;
	}

	//非重要参数
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "区域号", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nAreaNumber);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "签名条设置", "搜边阈值", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nSearchEdgeThresh);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "签名条设置", "白条检查", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsOpenWhiteArea);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "签名条设置", "白条位置信息", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcWhiteArea);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "签名条设置", "白条检查阈值", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nWhiteAreaCheckThresh);
		if (!nStatus)
		{
			return false;
		}
	}



	m_hParameter.m_nPointCount = 2;
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "签名条设置", "底板创建", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToBool(szResult, m_nIsCreateBaseImg);
	if (!nStatus)
	{
		return false;
	}

	if (m_nIsCreateBaseImg)
	{
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "签名条设置", "定位点", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToMulKxPoint(szResult, m_hParameter.m_nPointCount, m_hParameter.m_Point);
		if (!nStatus)
		{
			return false;
		}

	}




	return true;

}


bool CKxSignatureCheckBeta::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
{
	m_nUseXml = 1;
	//检测控制参数
	if (!ReadParaXmlinEnglish(filePath))
		return false;
	//距离标定参数
	if (!m_hCalDistance.ReadXmlinEnglish(filePath, BaseImg))
	{
		return false;
	}
	//图像校正参数
	if (!m_hWarpStrech.ReadXmlinEnglish(filePath, BaseImg))
	{
		return false;
	}
	//色差检查参数
	if (!m_hColorDiff.ReadXmlinEnglish(filePath, BaseImg))
	{
		return false;
	}
	//掩膜参数
	if (!m_hImgMask.ReadXmlinEnglish(filePath, BaseImg))
	{
		return false;
	}

	kxRect<int> rcCopy;
	rcCopy.setup(0, 0, BaseImg.nWidth - 1, BaseImg.nHeight - 1);
	m_BaseImg.Init(rcCopy.Width(), rcCopy.Height(), BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(BaseImg, m_BaseImg, rcCopy);

	//生成自动Mask
	kxRect<int>& rc = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_CreateMask.Init(rc.Width(), rc.Height(), m_BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(m_BaseImg, m_CreateMask, rc);
	m_hImgMask.AutoCreateMask(m_CreateMask);

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

	AnalysisCheckStatus();

	if (m_nIsCreateBaseImg)
	{

		for (int i = 0; i < 2; i++)
		{
			m_Offset[i].x = m_hParameter.m_Point[i].x - m_hParameter.m_Point[0].x;
			m_Offset[i].y = m_hParameter.m_Point[i].y - m_hParameter.m_Point[0].y;
		}
		m_nOffsetCount = m_hParameter.m_nPointCount;
		m_OffsetMin = m_OffsetMax = m_Offset[0];
		for (int i = 1; i < m_nOffsetCount; i++)
		{
			if (m_Offset[i].x < m_OffsetMin.x)
				m_OffsetMin.x = m_Offset[i].x;
			if (m_Offset[i].y < m_OffsetMin.y)
				m_OffsetMin.y = m_Offset[i].y;
			if (m_Offset[i].x > m_OffsetMax.x)
				m_OffsetMax.x = m_Offset[i].x;
			if (m_Offset[i].y > m_OffsetMax.y)
				m_OffsetMax.y = m_Offset[i].y;
		}


		kxRect<int>& rcCheckArea = m_hWarpStrech.GetParameter().m_rcCheckArea;
		m_WarpImg.Init(rcCheckArea.Width(), rcCheckArea.Height(), m_BaseImg.nChannel);
		//m_hBaseFun.SaveBMPImage_h("d:\\Base.bmp", m_BaseImg);
		KxCallStatus hCall;
		hCall.Clear();
		int nStatus = m_hWarpStrech.Check(m_BaseImg, m_WarpImg, hCall);
		if (nStatus)
		{
			return 0;
		}
		Matting(m_WarpImg, true);

	}


	return true;

}

bool CKxSignatureCheckBeta::ReadParaXmlinEnglish(const char* filePath)
{
	//重要参数
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "Learn", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsLearn);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "DotSensitivity", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nSmallDotOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "LoseSensitivity", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nLossOffset);
	if (!nStatus)
	{
		return false;
	}

	//非重要参数
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "AreaNum", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nAreaNumber);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Signature", "SearchEdgeThresh", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToInt(szResult, m_nSearchEdgeThresh);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Signature", "OpenWhiteStripesCheck", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsOpenWhiteArea);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Signature", "WhiteStripesAreaInfo", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcWhiteArea);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Signature", "WhiteStripesCheckThresh", szResult);
	if (nSearchStatus)
	{
		int nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nWhiteAreaCheckThresh);
		if (!nStatus)
		{
			return false;
		}
	}



	m_hParameter.m_nPointCount = 2;
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Signature", "BaseImgCreate", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToBool(szResult, m_nIsCreateBaseImg);
	if (!nStatus)
	{
		return false;
	}

	if (m_nIsCreateBaseImg)
	{
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Signature", "LocatePoint", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToMulKxPoint(szResult, m_hParameter.m_nPointCount, m_hParameter.m_Point);
		if (!nStatus)
		{
			return false;
		}

	}

	return true;

}



int CKxSignatureCheckBeta::LoadTemplateImg(const char* lpszFile)
{
	FILE*   fp;


#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "rb") != 0)
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
	int b = ReadImg(fp);
	fclose(fp);
	return b;
}

int CKxSignatureCheckBeta::SaveTemplateImg(const char* lpszFile)
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

int CKxSignatureCheckBeta::SaveLearnTemplateImg(const char* lpszFile)
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

	int b = WriteLocalLearnImg(fp);
	fclose(fp);
	return b;

}

int CKxSignatureCheckBeta::WriteLocalLearnImg(FILE* fp)
{
	if (fwrite(&m_nModelNumber, sizeof(int), 1, fp) != 1)
		return FALSE;

	switch (m_nModelNumber)
	{
	case _Max_Layer + 1:
		for (int i = 0; i < m_nModelNumber; i++)
		if (FALSE == m_ImgLearnTemplate[i].Write(fp))
			return FALSE;
		break;
	default:
		return FALSE;
	}

	if (m_nUseXml)
	{
		if (fwrite(&m_nSignatureModelWidth, sizeof(int), 1, fp) != 1)
			return FALSE;
		if (fwrite(&m_nSignatureModelHeight, sizeof(int), 1, fp) != 1)
			return FALSE;
	}


	//...............................................
	return TRUE;
}

int CKxSignatureCheckBeta::LoadLearnTemplateImg(const char* lpszFile)
{
	FILE*   fp;

#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "rb") != 0)
	{
		m_nModelNumber = 0;
		for (int k = 0; k < _Max_Layer + 1; k++)
		{
			m_ImgLearnTemplate[k].Init(0, 0, 1);
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
			m_ImgLearnTemplate[k].Init(0, 0, 1);
		}
		return FALSE;
	}

#endif

	int b = ReadLocalLearnImg(fp);
	fclose(fp);
	return b;
}


int CKxSignatureCheckBeta::ReadLocalLearnImg(FILE* fp)
{
	if (fread(&m_nModelNumber, sizeof(int), 1, fp) != 1)
		return FALSE;

	switch (m_nModelNumber)
	{
	case _Max_Layer + 1:
		for (int i = 0; i < m_nModelNumber; i++)
		if (FALSE == m_ImgLearnTemplate[i].Read(fp))
			return FALSE;
		break;
	default:
		return FALSE;
	}

	if (m_nUseXml)
	{
		if (!m_nIsCreateBaseImg)
		{
			if (fread(&m_nSignatureModelWidth, sizeof(int), 1, fp) != 1)
				return FALSE;
			if (fread(&m_nSignatureModelHeight, sizeof(int), 1, fp) != 1)
				return FALSE;
		}
	}

	//...............................................
	return TRUE;
}



int CKxSignatureCheckBeta::ReadImg(FILE* fp)
{
	if (fread(&m_nModelNumber, sizeof(int), 1, fp) != 1)
		return FALSE;

	switch (m_nModelNumber)
	{
	case _Max_Layer + 1:
		for (int i = 0; i < m_nModelNumber; i++)
		if (FALSE == m_ImgTemplate[i].Read(fp))
			return FALSE;
		break;
	default:
		return FALSE;
	}

	for (int k = 0; k < _Max_Layer + 1; k++)
	{
		m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
		m_hBaseFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
	}


	if (m_nUseXml)
	{
		if (!m_nIsCreateBaseImg)
		{
			//把第_Check_Layer张模板图转为定位图
			m_ImagePosSignature.Init(m_ImgTemplate[_Check_Layer].nWidth, m_ImgTemplate[_Check_Layer].nHeight, m_ImgTemplate[_Check_Layer].nChannel);
			kxRect<int> rcCopy;
			rcCopy.setup(0, 0, m_ImgTemplate[_Check_Layer].nWidth - 1, m_ImgTemplate[_Check_Layer].nHeight - 1);
			m_hBaseFun.KxCopyImage(m_ImgTemplate[_Check_Layer], m_ImagePosSignature, rcCopy);

			if (fread(&m_nSignatureModelWidth, sizeof(int), 1, fp) != 1)
				return FALSE;
			if (fread(&m_nSignatureModelHeight, sizeof(int), 1, fp) != 1)
				return FALSE;

		}
	}

	//...............................................
	return TRUE;
}

int CKxSignatureCheckBeta::WriteImg(FILE* fp)
{
	if (fwrite(&m_nModelNumber, sizeof(int), 1, fp) != 1)
		return FALSE;

	switch (m_nModelNumber)
	{
	case _Max_Layer + 1:
		for (int i = 0; i < m_nModelNumber; i++)
		if (FALSE == m_ImgTemplate[i].Write(fp))
			return FALSE;
		break;
	default:
		return FALSE;
	}

	for (int k = 0; k < _Max_Layer + 1; k++)
	{
		m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
		m_hBaseFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
	}

	if (m_nUseXml)
	{
		if (fwrite(&m_nSignatureModelWidth, sizeof(int), 1, fp) != 1)
			return FALSE;
		if (fwrite(&m_nSignatureModelHeight, sizeof(int), 1, fp) != 1)
			return FALSE;
	}

	//...............................................
	return TRUE;
}





int  CKxSignatureCheckBeta::Matting(const kxCImageBuf& SrcImg, bool bModel)
{
	KxCallStatus hCall;
	return Matting(SrcImg, bModel, hCall);
}


int  CKxSignatureCheckBeta::Matting(const kxCImageBuf& SrcImg, bool bModel, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTmpCall;
	hTmpCall.Clear();

	bool bSaveImg = true;
	//搜边内括的偏移值
	int dx = 10;
	int dy = 10;

	const int nSearchCountThresh = 30;

	//判断白条位置(靠左还是靠右)
	int nLeft = -1;
	if (m_hParameter.m_nIsOpenWhiteArea)
	{
		int nDiff1 = abs(m_hParameter.m_rcWhiteArea.left - m_hWarpStrech.GetParameter().m_rcCheckArea.left);
		int nDiff2 = abs(m_hParameter.m_rcWhiteArea.right - m_hWarpStrech.GetParameter().m_rcCheckArea.right);	
		if (nDiff1 <= nDiff2)
		{
			nLeft = 1;
		}
		else
		{
			nLeft = 0;
		}
	}


	//梯度
	{
		//检查：水平+垂直方向梯度检多、合成梯度检缺
		//首先生成三幅梯度图
		m_ImgCreateGradientTmp.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
		memset(m_ImgCreateGradientTmp.buf, 0, m_ImgCreateGradientTmp.nPitch * m_ImgCreateGradientTmp.nHeight);
		for (int i = 0; i < _Check_Layer - 1; i++)
		{
			if (i < 2)
			{
				m_hGradient.SingleDirGradientSame(SrcImg, m_ImgCreateGradient[i], i, 1, m_nSmoothSize, 255);
			}
			else
			{
				kxRect<int> rc;
				rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
				m_ImgCreateGradient[i].Init(rc.Width(), rc.Height(), SrcImg.nChannel);
				m_hBaseFun.KxCopyImage(SrcImg, m_ImgCreateGradient[i], rc);
			}
			
			//m_hGradient.SingleDirGradientSameWithBigSmooth(SrcImg, m_ImgCreateGradient[i], i, 1, m_nSmoothSize, 255);
			//增加Mask，利于搜边
			//m_hImgMask.Mask(m_ImgCreateGradient[i], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
			if  (i < 2)
				m_hBaseFun.KxParallelMaxEvery(m_ImgCreateGradient[i], m_ImgCreateGradientTmp);
		}
		m_hBaseFun.KxDilateImage(m_ImgCreateGradientTmp, m_ImgCreateGradient[_Check_Layer - 1]);

		if (check_sts(hTmpCall, "Matting_", hCall))
		{
			return 0;
		}
		//然后，合成图进行搜边定位
		m_hBaseFun.KxImageMaxEvery(m_ImgCreateGradient[_Check_Layer - 1], m_ImgGradient, hTmpCall);
		if (check_sts(hTmpCall, "Matting_", hCall))
		{
			return 0;
		}


		//均值滤波图上
		m_hBaseFun.KxAverageFilterImage(SrcImg, m_ImgAverage, 5, 5);
		Ipp32f* pProject[2];
		int nLen[2] = { m_ImgGradient.nWidth, m_ImgGradient.nHeight };
		for (int k = 0; k < 2; k++)
		{
			pProject[k] = new Ipp32f[nLen[k]];
			memset(pProject[k], 0, sizeof(Ipp32f)*nLen[k]);
		}

		IppiSize size1[2];
		size1[0] = { 1, 21 };
		size1[1] = { 21, 1 };
		IppiSize size2[2];
		size2[0] = { 1, 3 };
		size2[1] = { 61, 1 };
		IppiSize size3[2];
		size3[0] = { 2, 1 };
		size3[1] = { 1, 2 };

		for (int i = 0; i < 2; i++)
		{
			m_hGradient.SingleDirGradientSame(m_ImgAverage, m_ImgGradientPos[i], i, 1, m_nSmoothSize, 0);
			m_hImgMask.Mask(m_ImgGradientPos[i], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
		}

		parallel_for(blocked_range<int>(0, 2),
			[&](const blocked_range<int>& range)
		{
			for (int i = range.begin(); i < range.end(); i++)
			{
				m_hFun[i].KxImageMaxEvery(m_ImgGradientPos[i], m_ImgGradientMaxPos[i]);
				m_hFun[i].KxThreshImage(m_ImgGradientMaxPos[i], m_ImgThreshPos[i], m_hParameter.m_nSearchEdgeThresh, 255);
				m_hFun[i].KxCloseImage(m_ImgThreshPos[i], m_ImgClosePos[i], size1[i].width, size1[i].height);
				m_hFun[i].KxOpenImage(m_ImgClosePos[i], m_ImgThreshPos[i], size2[i].width, size2[i].height);
				m_hFun[i].KxOpenImage(m_ImgThreshPos[i], m_ImgFilterSpeckle[i], size3[i].width, size3[i].height);
			}
		}, auto_partitioner());

		//for (int i = 0; i < 2; i++)
		//{			
		//	m_hGradient.SingleDirGradientSame(m_ImgAverage, m_ImgGradientPos[i], i, 1, m_nSmoothSize, 0);
		//	m_hImgMask.Mask(m_ImgGradientPos[i], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
		//	m_hBaseFun.KxImageMaxEvery(m_ImgGradientPos[i], m_ImgGradientMaxPos[i]);
		//	m_hBaseFun.KxThreshImage(m_ImgGradientMaxPos[i], m_ImgThreshPos[i], m_hParameter.m_nSearchEdgeThresh, 255);
		//	m_hBaseFun.KxCloseImage(m_ImgThreshPos[i], m_ImgClosePos[i], size1[i].width, size1[i].height);
		//	//m_hBaseFun.KxFilterSpeckles(m_ImgClosePos[i], m_ImgFilterSpeckle[i], 50);
		//	m_hBaseFun.KxOpenImage(m_ImgClosePos[i], m_ImgThreshPos[i], size2[i].width, size2[i].height);
		//	m_hBaseFun.KxOpenImage(m_ImgThreshPos[i], m_ImgFilterSpeckle[i], size3[i].width, size3[i].height);
		//}


		//确定白条位置
		if (nLeft >= 0)
		{
			m_hBaseFun.KxProjectImage(m_ImgFilterSpeckle[0], 0, pProject[0], 255, hTmpCall);
			int nX1 = 0, nX2 = 0;
			bool bFind = false;
			int nCount = 0;
			int nStart = (nLeft == 1) ? 3 : (nLen[0] - 3);
			int nEnd = (nLeft == 1) ? (nLen[0]-1) : 3;
			int nStep = (nLeft == 1) ? 1 : -1;
			int k = nStart;
			while (k != nEnd)
			{
				if (pProject[0][k] > nSearchCountThresh)
				{
					if (!bFind)
					{
						if (nCount == 0)
						{
							nX1 = k;
						}
						if (nCount == 1)
						{
							if (abs(k - nX1) < 20)  //白条至少20个像素宽
							{
								k += nStep;
								continue;
							}
							else
							{
								nX2 = k;
								break;
							}

						}
						bFind = true;
						nCount++;
					}
				}
				else
				{
					bFind = false;
				}
				k += nStep;
			}

			int nLeftPos, nRightPos;
			if (nLeft == 1){
				nLeftPos = (std::min)((std::min)(nX1, nX2) + 10, m_ImgAverage.nWidth - 1);
				nRightPos = (std::max)(0, (std::max)(nX1, nX2) - 2);
			}
			else {
				nLeftPos = (std::min)((std::min)(nX1, nX2) + 2, m_ImgAverage.nWidth - 1);
				nRightPos = (std::max)(0, (std::max)(nX1, nX2) - 10);
			}
				
	

			if (nLeftPos == 0 && nRightPos == 0)
			{
				m_rcWhiteArea.setup(0, 0, 0, 0);
			}
			else
			{
				kxRect<int> rc;
				rc.setup(nLeftPos, 0, nRightPos, m_ImgAverage.nHeight - 1);
				m_ImgWhiteArea.SetImageBuf(m_ImgFilterSpeckle[1].buf + rc.left * m_ImgFilterSpeckle[1].nChannel + rc.top * m_ImgFilterSpeckle[1].nPitch,
					rc.Width(), rc.Height(), m_ImgFilterSpeckle[1].nPitch, m_ImgFilterSpeckle[1].nChannel, false);
				m_hBaseFun.KxProjectImage(m_ImgWhiteArea, 1, pProject[1], 255, hTmpCall);
				bool bNotFind[2] = { true, true };
				int nTopPos = 0, nBottomPos = 0;
				for (int k = 0; k < m_ImgWhiteArea.nHeight; k++)
				{
					if ((pProject[1][k] > nSearchCountThresh) && (bNotFind[0]))
					{
						nTopPos = k + 10;
						bNotFind[0] = false;
					}
					if ((pProject[1][m_ImgWhiteArea.nHeight - 1 - k] > nSearchCountThresh) && (bNotFind[1]))
					{
						nBottomPos = m_ImgWhiteArea.nHeight - 1 - k - 10;
						bNotFind[1] = false;
					}
					if (!bNotFind[1] && !bNotFind[0])
					{
						break;
					}
				}
				m_rcWhiteArea.setup(nLeftPos, nTopPos, nRightPos, nBottomPos);
			}

			if (m_rcWhiteArea.Width() > 0 && m_rcWhiteArea.Height() > 0)
			{
				kxRect<int> rcCopy;
				if (nLeft == 1){
					rcCopy.left = (std::max)(m_rcWhiteArea.left - 14, 0);
					rcCopy.top = (std::max)(m_rcWhiteArea.top - 14, 0);
					rcCopy.right = (std::min)(m_rcWhiteArea.right, m_ImgAverage.nWidth - 1);
					rcCopy.bottom = (std::min)(m_rcWhiteArea.bottom + 14, m_ImgAverage.nHeight - 1);
				}
				else
				{
					rcCopy.left = (std::max)(m_rcWhiteArea.left, 0);
					rcCopy.top = (std::max)(m_rcWhiteArea.top - 14, 0);
					rcCopy.right = (std::min)(m_rcWhiteArea.right + 14, m_ImgAverage.nWidth - 1);
					rcCopy.bottom = (std::min)(m_rcWhiteArea.bottom + 14, m_ImgAverage.nHeight - 1);
				}
				
				for (int k = 0; k < 2; k++)
				{
					IppiSize roiSize = { rcCopy.Width(), rcCopy.Height() };
					ippiSet_8u_C1R(0, m_ImgFilterSpeckle[k].buf + rcCopy.left* m_ImgFilterSpeckle[k].nChannel + rcCopy.top * m_ImgFilterSpeckle[k].nPitch,
						m_ImgFilterSpeckle[k].nPitch, roiSize);
				}
			}

			
		}

		//双方向投影,先垂直后水平
		
		int nPos[2][2] = { 0 };
		int nSearchStatus[2] = { 1, 1 };
		//m_hBaseFun.SaveBMPImage_h("d:\\SrcImg.bmp", SrcImg);
		//m_hBaseFun.SaveBMPImage_h("d:\\m_ImgGradient.bmp", m_ImgGradient);

		for (int nDir = 0; nDir < 2; nDir++)
		{
			m_hBaseFun.KxProjectImage(m_ImgFilterSpeckle[nDir], nDir, pProject[nDir], 255, hTmpCall);

			bool bNotFind[2] = { true, true };
			for (int i = 3; i < nLen[nDir]; i++)
			{
				if ((pProject[nDir][i] > nSearchCountThresh) && (bNotFind[0]))
				{
					nPos[nDir][0] = i + 4;
					bNotFind[0] = false;
				}
				if ((pProject[nDir][nLen[nDir] - 1 - i]  > nSearchCountThresh) && (bNotFind[1]))
				{	
					nPos[nDir][1] = nLen[nDir] - 1 - i - 4;
					bNotFind[1] = false;			
				}
				if (!bNotFind[1] && !bNotFind[0])
				{
					break;
				}
			}
			if (bNotFind[0] || bNotFind[1])
			{
				nSearchStatus[nDir] = 0;
			}
		}

		for (int k = 0; k < 2; k++)
		{
			delete[] pProject[k];
			pProject[k] = NULL;
		}
		if (nSearchStatus[0] == 0 || nSearchStatus[1] == 0)
		{
			int nStatus = kxSearchSignatureErr;
			if (check_sts(nStatus, "Not_Find_Signature_Edge", hCall))
			{
				return 0;
			}
		}


		kxRect<int> pos;

		pos.left = (std::min)(nPos[0][0] + dx, SrcImg.nWidth - 1);
		pos.right = (std::max)(0, nPos[0][1] - dx);
		pos.top = (std::min)(nPos[1][0] + dy, SrcImg.nHeight - 1);
		pos.bottom = (std::max)(0, nPos[1][1] - dy);


		if (pos.left >= pos.right || pos.top >= pos.bottom)
		{
			int nStatus = kxSearchSignatureErr;
			if (check_sts(nStatus, "Not_Find_Signature_Edge", hCall))
			{
				return 0;
			}
		}

		//拷贝边缘图像
		m_EdgeImg.Init(SrcImg.nWidth, SrcImg.nHeight);
		ippsSet_8u(0, m_EdgeImg.buf, m_EdgeImg.nPitch * m_EdgeImg.nHeight);

		if (m_hParameter.m_nIsOpenWhiteArea)
		{
			m_rcWhiteArea.left = (std::max)(m_rcWhiteArea.left, 0);
			m_rcWhiteArea.top = (std::max)(m_rcWhiteArea.top, 0);
			m_rcWhiteArea.right = (std::min)(m_rcWhiteArea.right, m_ImgGradient.nWidth-1);
			m_rcWhiteArea.bottom = (std::min)(m_rcWhiteArea.bottom, m_ImgGradient.nHeight-1);
			IppiSize roiSize = { m_rcWhiteArea.Width(), m_rcWhiteArea.Height() };			
			ippiCopy_8u_C1R(m_ImgGradient.buf + m_rcWhiteArea.left * m_ImgGradient.nChannel + m_rcWhiteArea.top * m_ImgGradient.nPitch, m_ImgGradient.nPitch,
				m_EdgeImg.buf + m_rcWhiteArea.left * m_EdgeImg.nChannel + m_rcWhiteArea.top * m_EdgeImg.nPitch, m_EdgeImg.nPitch, roiSize);
		}


		//m_ImgEdgeTemp.Init(SrcImg.nWidth, SrcImg.nHeight);
		//kxRect<int> rc;
		//rc.left = (std::max)(nPos[0][0] - dx / 2, 0);
		//rc.right = (std::min)(SrcImg.nWidth - 1, nPos[0][1] + dx / 2);
		//rc.top = (std::max)(nPos[1][0] - dy / 2, 0);
		//rc.bottom = (std::min)(SrcImg.nHeight - 1, nPos[1][1] + dy / 2);

		//IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
		//ippiCopy_8u_C1R(m_ImgGradient.buf, m_ImgGradient.nPitch, m_ImgEdgeTemp.buf, m_ImgEdgeTemp.nPitch, roiSize);
		//roiSize.width = rc.Width();
		//roiSize.height = rc.Height();
		//ippiSet_8u_C1R(0, m_ImgEdgeTemp.buf + rc.top * m_ImgEdgeTemp.nPitch + rc.left, m_ImgEdgeTemp.nPitch, roiSize);

		////去掉签名条紧靠的上下边缘
		//roiSize.width = m_ImgEdgeTemp.nWidth;
		//roiSize.height = (m_ImgEdgeTemp.nHeight - 1 - rc.top) > 10 ? 10 : (m_ImgEdgeTemp.nHeight - 1 - rc.top);
		//ippiSet_8u_C1R(0, m_ImgEdgeTemp.buf + rc.top * m_ImgEdgeTemp.nPitch, m_ImgEdgeTemp.nPitch, roiSize);
		//roiSize.width = m_ImgEdgeTemp.nWidth;
		//roiSize.height = 20;
		//int nIndex = (rc.bottom - 20) > 0 ? (rc.bottom - 20) : 0;
		//ippiSet_8u_C1R(0, m_ImgEdgeTemp.buf + nIndex * m_ImgEdgeTemp.nPitch, m_ImgEdgeTemp.nPitch, roiSize);

		////mask边缘一个像素由于梯度引起的
		//roiSize.width = SrcImg.nWidth - 2;
		//roiSize.height = SrcImg.nHeight - 2;
		//ippiCopy_8u_C1R(m_ImgEdgeTemp.buf + 1 * m_ImgEdgeTemp.nPitch + 1, m_ImgEdgeTemp.nPitch,
		//	m_EdgeImg.buf + 1 * m_EdgeImg.nPitch + 1, m_EdgeImg.nPitch, roiSize);

		m_nLearnHighLowMode[0] = 1;
		m_nLearnHighLowMode[1] = 1;
		m_nLearnHighLowMode[2] = 0;
		m_nLearnHighLowMode[3] = 0;

		m_nLearnHighLowMode[4] = 0;

		//建模模式生成定位图像
		if (bModel)
		{

			for (int i = 0; i < SUBSECTION; i++)
			{
				m_ImgSingleLocation[i].Init(pos.Width() - m_OffsetMin.x + m_OffsetMax.x, pos.Height() - m_OffsetMin.y + m_OffsetMax.y, SrcImg.nChannel);
				memset(m_ImgSingleLocation[i].buf, 0, m_ImgSingleLocation[i].nHeight * m_ImgSingleLocation[i].nPitch);
			}


			for (int i = 0; i < _Check_Layer; i++)
			{
				m_ImgSingleBase[i].Init(m_ImgSingleLocation[0].nWidth, m_ImgSingleLocation[0].nHeight, m_ImgSingleLocation[0].nChannel);
				m_ImgCreateBase[i].Init(m_ImgSingleBase[i].nWidth, m_ImgSingleBase[i].nHeight * SUBSECTION, m_ImgSingleBase[i].nChannel);
				if (m_nLearnHighLowMode[i])
				{
					memset(m_ImgSingleBase[i].buf, 0xff, m_ImgSingleBase[i].nPitch * m_ImgSingleBase[i].nHeight);
					memset(m_ImgCreateBase[i].buf, 0xff, m_ImgCreateBase[i].nPitch * m_ImgCreateBase[i].nHeight);
				}
				else
				{
					memset(m_ImgSingleBase[i].buf, 0, m_ImgSingleBase[i].nPitch * m_ImgSingleBase[i].nHeight);
					memset(m_ImgCreateBase[i].buf, 0, m_ImgCreateBase[i].nPitch * m_ImgCreateBase[i].nHeight);
				}
			}


			//只用2个十字叉
			for (int i = 0; i < m_nOffsetCount; i++)
			{
				IppiSize Size = { pos.Width(), pos.Height() };

				for (int k = 0; k < SUBSECTION; k++)
				{
					int nStart;
					nStart = pos.left;
					IppStatus status = ippiCopy_8u_C3R(SrcImg.buf + pos.top*SrcImg.nPitch + nStart*SrcImg.nChannel, SrcImg.nPitch,
						m_ImgSingleLocation[k].buf + (m_Offset[i].y - m_OffsetMin.y) * m_ImgSingleLocation[k].nPitch + (m_Offset[i].x - m_OffsetMin.x) * m_ImgSingleLocation[k].nChannel, m_ImgSingleLocation[k].nPitch, Size);
				}
				for (int k = 0; k < _Check_Layer; k++)
				{
					IppStatus status = ippiCopy_8u_C3R(m_ImgCreateGradient[k].buf + pos.top*m_ImgCreateGradient[k].nPitch + pos.left*m_ImgCreateGradient[k].nChannel, m_ImgCreateGradient[k].nPitch,
						m_ImgSingleBase[k].buf + (m_Offset[i].y - m_OffsetMin.y) * m_ImgSingleBase[k].nPitch + (m_Offset[i].x - m_OffsetMin.x) * m_ImgSingleBase[k].nChannel, m_ImgSingleBase[k].nPitch, Size);
				}
			}

			m_ImgCreateLocation.Init(m_ImgSingleLocation[0].nWidth, m_ImgSingleLocation[0].nHeight * SUBSECTION, m_ImgSingleLocation[0].nChannel);
			memset(m_ImgCreateLocation.buf, 0, m_ImgCreateLocation.nHeight * m_ImgCreateLocation.nPitch);
			for (int i = 0; i < SUBSECTION; i++)
			{
				IppiSize Size = { m_ImgSingleLocation[i].nWidth, m_ImgSingleLocation[i].nHeight };
				IppStatus status = ippiCopy_8u_C3R(m_ImgSingleLocation[i].buf, m_ImgSingleLocation[i].nPitch,
					m_ImgCreateLocation.buf + (m_ImgSingleLocation[i].nHeight * i)* m_ImgCreateLocation.nPitch, m_ImgCreateLocation.nPitch, Size);

				for (int k = 0; k < _Check_Layer; k++)
				{
					IppStatus status = ippiCopy_8u_C3R(m_ImgSingleBase[k].buf, m_ImgSingleBase[k].nPitch,
						m_ImgCreateBase[k].buf + (m_ImgSingleBase[k].nHeight * i)* m_ImgCreateBase[k].nPitch, m_ImgCreateBase[k].nPitch, Size);
				}
			}

			//拷贝定位底板
			m_ImagePosSignature.Init(m_ImgCreateBase[_Check_Layer - 1].nWidth, m_ImgCreateBase[_Check_Layer - 1].nHeight, m_ImgCreateBase[_Check_Layer-1].nChannel);
			IppiSize roi = { m_ImagePosSignature.nWidth, m_ImagePosSignature.nHeight };
			IppStatus status = ippiCopy_8u_C3R(m_ImgCreateBase[_Check_Layer - 1].buf, m_ImgCreateBase[_Check_Layer-1].nPitch,
				m_ImagePosSignature.buf, m_ImagePosSignature.nPitch, roi);

			//SaveMidImage(true);
			//3个层面分别学进去
			for (int k = 0; k < _Check_Layer; k++)
			{
				//底板连续学习3次，把边缘白学进去
				for (int i = 0; i < 3; i++)
				{
					m_hLearn[k].LearnImage(m_ImgCreateBase[k], m_nLearnHighLowMode[k], m_nLearnPage);
				}
			}

		}
		else  //在线模式，相似度定位确定在模板中位置
		{	
			pos.left = (std::min)(nPos[0][0] + m_nSignatureExtendLen, SrcImg.nWidth - 1);
			pos.right = (std::max)(0, nPos[0][1] - m_nSignatureExtendLen);
			//pos.top = max(nPos[1][0] - 2*dy, 0);
			//pos.bottom = min(SrcImg.nHeight - 1, nPos[1][1] + 2*dy);
			pos.top = (std::min)(nPos[1][0] + m_nSignatureExtendLen, SrcImg.nHeight - 1);
			pos.bottom = (std::max)(0, nPos[1][1] - m_nSignatureExtendLen);

			if (pos.Width() <= 0 || pos.Height() <= 0)
			{
				int nStatus = kxSearchSignatureErr;
				if (check_sts(nStatus, "Not_Find_Signature_Edge", hCall))
				{
					return 0;
				}
			}

			m_rcSignatureArea.setup(pos.left, pos.top, pos.right, pos.bottom);

			//把签名条分水平方向分SUBSECTION段，分别定位，这样的话可以在一定程度上抗水平方向的轻微倾斜
			int nLen = pos.Width() / SUBSECTION;
			int nRectW[SUBSECTION];
			for (int i = 0; i < SUBSECTION - 1; i++)
			{
				nRectW[i] = nLen;
			}
			nRectW[SUBSECTION - 1] = pos.Width() - (SUBSECTION - 1)* nLen;

			for (int i = 0; i < _Check_Layer; i++)
			{
				//m_ImgCheck[i].Init(m_ImgCreateLocation.nWidth, m_ImgCreateLocation.nHeight, m_ImgCreateLocation.nChannel);
				//m_ImgLearn[i].Init(m_ImgCreateLocation.nWidth, m_ImgCreateLocation.nHeight, m_ImgCreateLocation.nChannel);
				m_ImgCheck[i].Init(m_ImagePosSignature.nWidth, m_ImagePosSignature.nHeight, m_ImagePosSignature.nChannel);
				m_ImgLearn[i].Init(m_ImagePosSignature.nWidth, m_ImagePosSignature.nHeight, m_ImagePosSignature.nChannel);
				if (m_nLearnHighLowMode[i])
				{
					memset(m_ImgCheck[i].buf, 0, m_ImgCheck[i].nPitch * m_ImgCheck[i].nHeight);
					memset(m_ImgLearn[i].buf, 0, m_ImgLearn[i].nPitch * m_ImgLearn[i].nHeight);

				}
				else
				{
					memset(m_ImgCheck[i].buf, 0xff, m_ImgCheck[i].nPitch * m_ImgCheck[i].nHeight);
					memset(m_ImgLearn[i].buf, 0xff, m_ImgLearn[i].nPitch * m_ImgLearn[i].nHeight);
				}
			}

			float fRatio[SUBSECTION] = { 0 };
			float fRatio1[SUBSECTION] = { 0 };
			KxCallStatus hCallInfo[SUBSECTION];
			parallel_for(blocked_range<int>(0, SUBSECTION),
				[&](const blocked_range<int>& range)
			{
				for (int i = range.begin(); i < range.end(); i++)
				{

					m_rcSplitSmall[i].setup(pos.left + i * nLen, pos.top, pos.left + i * nLen + nRectW[i] - 1, pos.bottom);
					//Ipp32f* pProject = new Ipp32f[m_ImgFilterSpeckle[1].nHeight];
					if (m_ImgFilterSpeckle[1].nHeight <= 0 || m_ImgFilterSpeckle[1].nWidth <= 0 || m_rcSplitSmall[i].Width() <= 0 || m_rcSplitSmall[i].Height() <= 0)
					{
						hCallInfo[i].nCallStatus = -1001;
						break;
					}
					//重新分段精确搜水平边 2017/11/23, 垂直边不搜索
					m_ImgSplitSearch[i].SetImageBuf(m_ImgFilterSpeckle[1].buf + m_rcSplitSmall[i].left * m_ImgFilterSpeckle[1].nChannel, m_rcSplitSmall[i].Width(), m_ImgFilterSpeckle[1].nHeight, m_ImgFilterSpeckle[1].nPitch, m_ImgFilterSpeckle[1].nChannel, false);
					//Ipp32f* pProject = (Ipp32f*)scalable_aligned_malloc(m_ImgFilterSpeckle[1].nHeight*sizeof(Ipp32f), 32);
					Ipp32f* pProject = (Ipp32f*)ippsMalloc_32f(m_ImgSplitSearch[i].nHeight);
					m_hFun[i].KxProjectImage(m_ImgSplitSearch[i], 1, pProject, 255, hTmpCall);
					bool bNotFind[2] = { true, true };
					int nStart, nEnd;
					for (int j = 3; j < m_ImgSplitSearch[i].nHeight - 3; j++)
					{
						if ((pProject[j] > nSearchCountThresh / SUBSECTION) && (bNotFind[0]))
						{
							nStart = j + 4;
							bNotFind[0] = false;
						}
						if ((pProject[m_ImgSplitSearch[i].nHeight - 3 - j] > nSearchCountThresh / SUBSECTION) && (bNotFind[1]))
						{
							nEnd = m_ImgSplitSearch[i].nHeight - 3 - j - 4;
							bNotFind[1] = false;
						}
						if (!bNotFind[1] && !bNotFind[0])
						{
							break;
						}
					}
					if (!bNotFind[1] && !bNotFind[0] && (nEnd - nStart)*2 > m_rcSplitSmall[i].Height())
					{
						m_rcSplitSmall[i].top = (std::min)(nStart + m_nSignatureExtendLen, m_ImgSplitSearch[i].nHeight - 1);
						m_rcSplitSmall[i].bottom = (std::max)(0, nEnd - m_nSignatureExtendLen);
					}
					//delete[] pProject;
					//scalable_aligned_free((void*)pProject);
					ippsFree(pProject);

					//////////////////////
					if (m_rcSplitSmall[i].Width() <= 0 || m_rcSplitSmall[i].Height() <= 0)
					{
						hCallInfo[i].nCallStatus = -1002;
						break;
					}
					//灰度图像定位
					//m_ImgLocationSmall[i].SetImageBuf(SrcImg.buf + m_rcSplitSmall[i].top*SrcImg.nPitch + m_rcSplitSmall[i].left*SrcImg.nChannel, m_rcSplitSmall[i].Width(), m_rcSplitSmall[i].Height(), SrcImg.nPitch, SrcImg.nChannel, false);
					//m_ImgLocationBig[i].SetImageBuf(m_ImgCreateLocation.buf + i*m_ImgSingleLocation[i].nHeight*m_ImgCreateLocation.nPitch, m_ImgSingleLocation[i].nWidth, m_ImgSingleLocation[i].nHeight, m_ImgCreateLocation.nPitch, m_ImgCreateLocation.nChannel, false);
					//合成梯度图像定位
					//m_ImgLocationSmall[i].SetImageBuf(m_ImgCreateGradient[2].buf + m_rcSplitSmall[i].top*m_ImgCreateGradient[2].nPitch + m_rcSplitSmall[i].left*m_ImgCreateGradient[2].nChannel, m_rcSplitSmall[i].Width(), m_rcSplitSmall[i].Height(), m_ImgCreateGradient[2].nPitch, m_ImgCreateGradient[2].nChannel, false);
					//m_ImgLocationBig[i].SetImageBuf(m_ImgCreateBase[2].buf + i*m_ImgSingleLocation[i].nHeight*m_ImgCreateBase[2].nPitch, m_ImgSingleLocation[i].nWidth, m_ImgSingleLocation[i].nHeight, m_ImgCreateBase[2].nPitch, m_ImgCreateBase[2].nChannel, false);

					int nSingleHeight = m_ImagePosSignature.nHeight / SUBSECTION;
					m_ImgLocationSmall[i].SetImageBuf(m_ImgCreateGradient[_Check_Layer - 1].buf + m_rcSplitSmall[i].top*m_ImgCreateGradient[_Check_Layer - 1].nPitch + m_rcSplitSmall[i].left*m_ImgCreateGradient[_Check_Layer - 1].nChannel, m_rcSplitSmall[i].Width(), m_rcSplitSmall[i].Height(), m_ImgCreateGradient[_Check_Layer - 1].nPitch, m_ImgCreateGradient[_Check_Layer - 1].nChannel, false);
					m_ImgLocationBig[i].SetImageBuf(m_ImagePosSignature.buf + i*nSingleHeight*m_ImagePosSignature.nPitch, m_ImagePosSignature.nWidth, nSingleHeight, m_ImagePosSignature.nPitch, m_ImagePosSignature.nChannel, false);


					kxPoint<float> pt;

					//fRatio[i] = m_hFun[i].kxImageAlign(pt, m_ImgLocationBig[i], m_ImgLocationSmall[i], 1, hCallInfo[i]);
					fRatio[i] = m_hFun[i].kxImageAlignBySSD(pt, m_ImgLocationBig[i], m_ImgLocationSmall[i], 2, hCallInfo[i]);
					if (hCallInfo[i].nCallStatus != 0)
					{
						break;
					}
					//if (fRatio[i] < gMinSimilarity)
					//{
					//	break;
					//}

					IppiSize Size = { m_rcSplitSmall[i].Width(), m_rcSplitSmall[i].Height() };

					pt.x = gMax(pt.x, 0);
					pt.y = gMax(pt.y, 0);
					pt.x = gMin(pt.x, m_ImgLearn[0].nWidth - Size.width);
					//if (m_ImgSingleLocation[i].nHeight < Size.height)
					//{
					//	pt.y = 0;
					//	Size.height = m_ImgSingleLocation[i].nHeight;
					//}
					//else
					//	pt.y = gMin(pt.y, m_ImgSingleLocation[i].nHeight - Size.height);
					if (nSingleHeight < Size.height)
					{
						pt.y = 0;
						Size.height = nSingleHeight;
					}
					else
						pt.y = gMin(pt.y, nSingleHeight - Size.height);

					m_rcLocationPos[i].setup(int(pt.x), int(pt.y), int(pt.x + Size.width - 1), int(pt.y + Size.height - 1));

					if (m_hParameter.m_nIsLearn)
					{
						for (int k = 0; k < _Check_Layer; k++)
						{
							for (int j = 0; j < SUBSECTION; j++)
							{
								IppStatus status = ippiCopy_8u_C3R(m_ImgCreateGradient[k].buf + m_rcSplitSmall[i].top*m_ImgCreateGradient[k].nPitch + m_rcSplitSmall[i].left*m_ImgCreateGradient[k].nChannel, m_ImgCreateGradient[k].nPitch,
									m_ImgLearn[k].buf + (m_rcLocationPos[i].top + j*m_ImgSingleLocation[i].nHeight)* m_ImgLearn[k].nPitch + m_rcLocationPos[i].left * m_ImgLearn[k].nChannel, m_ImgLearn[k].nPitch, Size);
							}
						}
					}


					//m_rcLocationPos[i].offset(0, i*m_ImgSingleLocation[i].nHeight);
					m_rcLocationPos[i].offset(0, i*nSingleHeight);

					m_rcLocationPos[i].left = m_rcLocationPos[i].left > 0 ? m_rcLocationPos[i].left : 0;
					m_rcLocationPos[i].right = m_rcLocationPos[i].right < m_ImgCheck[0].nWidth - 1 ? m_rcLocationPos[i].right : m_ImgCheck[0].nWidth - 1;
					m_rcLocationPos[i].top = m_rcLocationPos[i].top > 0 ? m_rcLocationPos[i].top : 0;
					m_rcLocationPos[i].bottom = m_rcLocationPos[i].bottom < m_ImgCheck[0].nHeight - 1 ? m_rcLocationPos[i].bottom : m_ImgCheck[0].nHeight - 1;

					for (int k = 0; k < _Check_Layer; k++)
					{
						IppStatus status = ippiCopy_8u_C3R(m_ImgCreateGradient[k].buf + m_rcSplitSmall[i].top*m_ImgCreateGradient[k].nPitch + m_rcSplitSmall[i].left*m_ImgCreateGradient[k].nChannel, m_ImgCreateGradient[k].nPitch,
							m_ImgCheck[k].buf + m_rcLocationPos[i].top * m_ImgCheck[k].nPitch + m_rcLocationPos[i].left * m_ImgCheck[k].nChannel, m_ImgCheck[k].nPitch, Size);
					}
				}

			}, auto_partitioner());

			for (int i = 0; i < SUBSECTION; i++)
			{
				if (check_sts(hCallInfo[i], "Matting_", hCall))
				{
					return 0;
				}
				//if (fRatio[i] < gMinSimilarity)
				//{
				//	IppStatus status = IppStatus(kxImageAlignRatioLow);
				//	if (check_sts(status, "Matting_", hCall))
				//	{
				//		return 0;
				//	}
				//}
			}

			if (m_hParameter.m_nIsLearn)  //学习状态,确保重合区域是原始层的数据
			{
				for (int k = 0; k < _Check_Layer; k++)
				{
					for (int i = 0; i < SUBSECTION; i++)
					{
						IppiSize Size = { m_rcLocationPos[i].Width(), m_rcLocationPos[i].Height() };
						IppStatus status = ippiCopy_8u_C3R(m_ImgCreateGradient[k].buf + m_rcSplitSmall[i].top*m_ImgCreateGradient[k].nPitch + m_rcSplitSmall[i].left*m_ImgCreateGradient[k].nChannel, m_ImgCreateGradient[k].nPitch,
							m_ImgLearn[k].buf + m_rcLocationPos[i].top* m_ImgLearn[k].nPitch + m_rcLocationPos[i].left * m_ImgLearn[k].nChannel, m_ImgLearn[k].nPitch, Size);
					}
				}

			}

		}
	}

	return 1;
}



//学习图像
int  CKxSignatureCheckBeta::LearnTemplate(KxCallStatus& hCall)
{
	hCall.Clear();
	if (!m_hParameter.m_nIsLearn)
	{
		return 0;
	}
	//模板操作
	int nModelProcessSz[_Check_Layer];
	memset(nModelProcessSz, 0, sizeof(int)*_Check_Layer);
	for (int i = 0; i < _Check_Layer; i++)
	{
		nModelProcessSz[i] = _5X5;
	}

	parallel_for(blocked_range<int>(0, _Check_Layer),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		{
			if (m_nLearnStatus)
			{
				if (m_hLearn[i].LearnImage(m_ImgLearn[i], m_nLearnHighLowMode[i], m_nLearnPage, hCall))
				{
					m_bIsLearnCompleted[i] = true;
					m_ImgTemplate[i].Init(m_ImgLearn[i].nWidth, m_ImgLearn[i].nHeight, m_ImgLearn[i].nChannel);
					m_hLearn[i].GetTemplateBufParallel(_Valid_Layer, m_nLearnHighLowMode[i], m_ImgTemplate[i], nModelProcessSz[i]);
				}
			}

		}
	}, auto_partitioner());

	bool status = true;
	for (int i = 0; i < _Check_Layer; i++)
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

		m_nModelNumber = _Max_Layer + 1;

		//for (int i = _Check_Layer; i < _Max_Layer; i++)
		//{
		//	m_ImgTemplate[i].Init(m_ImgTemplate[_Check_Layer - 1].nWidth, m_ImgTemplate[_Check_Layer - 1].nHeight, m_ImgTemplate[_Check_Layer - 1].nChannel);
		//	kxRect<int> rc;
		//	rc.setup(0, 0, m_ImgTemplate[_Check_Layer - 1].nWidth - 1, m_ImgTemplate[_Check_Layer - 1].nHeight - 1);
		//	m_hBaseFun.KxCopyImage(m_ImgTemplate[_Check_Layer - 1], m_ImgTemplate[i], rc);
		//}
		//m_ImgTemplate[_Max_Layer].Init(m_ImgTemplate[0].nWidth, m_ImgTemplate[0].nHeight, m_ImgTemplate[0].nChannel);

		//ippsSet_8u(0, m_ImgTemplate[_Max_Layer].buf, m_ImgTemplate[_Max_Layer].nPitch * m_ImgTemplate[_Max_Layer].nHeight);

		//for (int i = 0; i < _Check_Layer-1; i++)
		//{
		//	if (m_nLearnStatus)
		//	{
		//		m_hBaseFun.KxParallelMaxEvery(m_ImgTemplate[i], m_ImgTemplate[_Max_Layer]);
		//	}
		//}
		//m_nLearnStatus = 0;

		//for (int k = 0; k < _Max_Layer + 1; k++)
		//{
		//	m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
		//	kxRect<int> rc;
		//	rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
		//	m_hBaseFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
		//}
		//第_Check_Layer张图为定位图
		m_ImgTemplate[_Check_Layer].Init(m_ImagePosSignature.nWidth, m_ImagePosSignature.nHeight, m_ImagePosSignature.nChannel);
		kxRect<int> rcCopy;
		rcCopy.setup(0, 0, m_ImgTemplate[_Check_Layer].nWidth - 1, m_ImgTemplate[_Check_Layer].nHeight - 1);
		m_hBaseFun.KxCopyImage(m_ImagePosSignature, m_ImgTemplate[_Check_Layer], rcCopy);

		for (int i = _Check_Layer + 1; i < _Max_Layer; i++)
		{
			m_ImgTemplate[i].Init(m_ImgTemplate[_Check_Layer - 1].nWidth, m_ImgTemplate[_Check_Layer - 1].nHeight, m_ImgTemplate[_Check_Layer - 1].nChannel);
			kxRect<int> rc;
			rc.setup(0, 0, m_ImgTemplate[_Check_Layer - 1].nWidth - 1, m_ImgTemplate[_Check_Layer - 1].nHeight - 1);
			m_hBaseFun.KxCopyImage(m_ImgTemplate[_Check_Layer - 1], m_ImgTemplate[i], rc);
		}
		m_ImgTemplate[_Max_Layer].Init(m_ImgTemplate[0].nWidth, m_ImgTemplate[0].nHeight, m_ImgTemplate[0].nChannel);

		ippsSet_8u(0, m_ImgTemplate[_Max_Layer].buf, m_ImgTemplate[_Max_Layer].nPitch * m_ImgTemplate[_Max_Layer].nHeight);

		for (int i = 0; i < 2; i++)
		{
			if (m_nLearnStatus)
			{
				m_hBaseFun.KxParallelMaxEvery(m_ImgTemplate[i], m_ImgTemplate[_Max_Layer]);
			}
		}
		m_nLearnStatus = 0;

		for (int k = 0; k < _Max_Layer + 1; k++)
		{
			m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
			kxRect<int> rc;
			rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
			m_hBaseFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
		}


	}

	return 1;
}

//残差分析并且合并为一张残差图
int CKxSignatureCheckBeta::ResidualsAnalys(KxCallStatus& hCall)
{
	hCall.Clear();
	for (int i = 0; i < _Check_Layer; i++)
	{
		m_ImgSlidCc[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight, m_ImgCheck[i].nChannel);
		m_ImgSlidCcMax[i].Init(m_ImgSlidCc[i].nWidth, m_ImgSlidCc[i].nHeight);
		m_ImgOffset[i].Init(m_ImgSlidCc[i].nWidth, m_ImgSlidCc[i].nHeight, m_ImgSlidCc[i].nChannel);
		m_ImgCompose[i].Init(m_WarpImg.nWidth, m_WarpImg.nHeight);
		ippsSet_8u(0, m_ImgCompose[i].buf, m_ImgCompose[i].nPitch * m_ImgCompose[i].nHeight);

		m_ImgOpen[i].Init(m_WarpImg.nWidth, m_WarpImg.nHeight);
		ippsSet_8u(0, m_ImgOpen[i].buf, m_ImgOpen[i].nPitch * m_ImgOpen[i].nHeight);

		m_ImgFilterEdge[i].Init(m_ImgCompose[i].nWidth, m_ImgCompose[i].nHeight);
	}

	int nCheckHighLowMode[_Check_Layer];
	memset(nCheckHighLowMode, 0, sizeof(int)*_Check_Layer);
	KxCallStatus hCallInfo[_Check_Layer];
	//设置去噪卷积核的尺寸
	IppiSize sizeMaskFilter[_Check_Layer];
	sizeMaskFilter[0] = { 1, 3 };
	sizeMaskFilter[1] = { 3, 1 };
	sizeMaskFilter[2] = { 3, 3 };
	//sizeMaskFilter[3] = { 3, 3 };
	//sizeMaskFilter[4] = { 3, 3 };
	

	IppiSize sizeMaskEdgeFilter[_Check_Layer];
	sizeMaskEdgeFilter[0] = { 3, 1 };
	sizeMaskEdgeFilter[1] = { 1, 3 };



	for (int i = 0; i < _Check_Layer - 1; i++)
	{
		nCheckHighLowMode[i] = 1;  //检高
		hCallInfo[i].Clear();
	}

	int nGridXY[_Check_Layer];
	int nOffset[_Max_Layer];
	for (int i = 0; i < _Check_Layer - 1; i++)
	{
		nGridXY[i] = 80;
		nOffset[i] = m_hParameter.m_nSmallDotOffset;
	}

	nOffset[_Check_Layer - 1] = m_hParameter.m_nLossOffset;
	nGridXY[_Check_Layer - 1] = 80;

	parallel_for(blocked_range<int>(0, _Check_Layer),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				int nStatus = m_hSlider[i].SilderCcParallel(m_ImgCheck[i], m_ImgTemplate[i], m_ImgSlidCc[i], nCheckHighLowMode[i], nGridXY[i], nGridXY[i], hCallInfo[i]);
				if (nStatus == 0)
				{
					break;
				}
				IppiSize roiSize = { m_ImgSlidCc[i].nWidth, m_ImgSlidCc[i].nHeight };
				m_hFun[i].KxSubCImage(m_ImgSlidCc[i], m_ImgOffset[i], nOffset[i]);
				m_hFun[i].KxImageMaxEvery(m_ImgOffset[i], m_ImgSlidCcMax[i]);

				for (int k = 0; k < SUBSECTION; k++)
				{
					IppiSize roi = { m_rcSplitSmall[k].Width(), m_rcSplitSmall[k].Height() };
					ippiCopy_8u_C1R(m_ImgSlidCcMax[i].buf + m_rcLocationPos[k].top * m_ImgSlidCcMax[i].nPitch + m_rcLocationPos[k].left, m_ImgSlidCcMax[i].nPitch,
						m_ImgOpen[i].buf + m_rcSplitSmall[k].top * m_ImgOpen[i].nPitch + m_rcSplitSmall[k].left, m_ImgOpen[i].nPitch, roi);
				}


				//采用单向开操作去噪声

				Ipp8u pMask[3] = { 1, 1, 1 };
				m_hFun[i].KxOpenImage(m_ImgOpen[i], m_ImgCompose[i], sizeMaskFilter[i].width, sizeMaskFilter[i].height, pMask, hCallInfo[i]);
				if (check_sts(hCallInfo[i], "", hCall))
				{
					break;
				}
				////采用单向开操作去噪声
				//if (i < 4)
				//{
				//	if (i < 2)
				//	{
				//		Ipp8u pMask[3] = { 1, 1, 1 };
				//		m_hFun[i].KxOpenImage(m_ImgOpen[i], m_ImgCompose[i], sizeMaskFilter[i].width, sizeMaskFilter[i].height, pMask, hCallInfo[i]);
				//	}
				//	else if (i == 2)
				//	{
				//		Ipp8u pMask[3 * 3] = { 0, 0, 1,
				//			0, 1, 0,
				//			1, 0, 0 };
				//		m_hFun[i].KxOpenImage(m_ImgOpen[i], m_ImgCompose[i], sizeMaskFilter[i].width, sizeMaskFilter[i].height, pMask, hCallInfo[i]);
				//	}
				//	else
				//	{
				//		Ipp8u pMask[3 * 3] = { 1, 0, 0,
				//			0, 1, 0,
				//			0, 0, 1 };
				//		m_hFun[i].KxOpenImage(m_ImgOpen[i], m_ImgCompose[i], sizeMaskFilter[i].width, sizeMaskFilter[i].height, pMask, hCallInfo[i]);
				//	}

				//	if (check_sts(hCallInfo[i], "", hCall))
				//	{
				//		break;
				//	}
				//}
				//else
				//{
				//	kxRect<int> rc;
				//	rc.setup(0, 0, m_ImgOpen[i].nWidth - 1, m_ImgOpen[i].nHeight - 1);
				//	m_hFun[i].KxCopyImage(m_ImgOpen[i], m_ImgCompose[i], rc);
				//}

				//边缘部分
				if (i < 2)
				{
					m_hFun[i].KxOpenImage(m_ImgCompose[i], m_ImgFilterEdge[i], sizeMaskEdgeFilter[i].width, sizeMaskEdgeFilter[i].height);
					kxRect<int> rc;
					rc.setup(m_rcSignatureArea.left + 10, m_rcSignatureArea.top + 10, m_rcSignatureArea.right - 10, m_rcSignatureArea.bottom - 10);
					IppiSize roi = { rc.Width(), rc.Height() };
					ippiCopy_8u_C1R(m_ImgCompose[i].buf + rc.top * m_ImgCompose[i].nPitch + rc.left, m_ImgCompose[i].nPitch,
						m_ImgFilterEdge[i].buf + rc.top * m_ImgFilterEdge[i].nPitch + rc.left, m_ImgFilterEdge[i].nPitch, roi);
				}
				else
				{
					kxRect<int> rc;
					rc.setup(0, 0, m_ImgCompose[i].nWidth - 1, m_ImgCompose[i].nHeight - 1);	
					m_hFun[i].KxCopyImage(m_ImgCompose[i], m_ImgFilterEdge[i], rc);
				}
				

			}

		}
	}, auto_partitioner());

	for (int i = 0; i < _Check_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			if (check_sts(hCallInfo[i], "", hCall))
			{
				return 0;
			}
		}
	}

	//加边缘检查
	nOffset[_Check_Layer] = m_hParameter.m_nWhiteAreaCheckThresh;
	m_hBaseFun.KxSubCImage(m_EdgeImg, m_ImgOffset[_Check_Layer], nOffset[_Check_Layer]);

	m_ImgTemp.Init(m_EdgeImg.nWidth, m_EdgeImg.nHeight);
	ippsSet_8u(0, m_ImgTemp.buf, m_ImgTemp.nPitch*m_ImgTemp.nHeight);

	m_ImageCc.Init(m_EdgeImg.nWidth, m_EdgeImg.nHeight);
	ippsSet_8u(0, m_ImageCc.buf, m_ImageCc.nPitch*m_ImageCc.nHeight);

	static int n = 0;
	for (int i = 0; i < _Check_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hFun[i].KxMaxEvery(m_ImgFilterEdge[i], m_ImageCc);
		}
	}

	m_hBaseFun.KxAddImage(m_ImgOffset[_Check_Layer], m_ImageCc);

	KxCallStatus hCallInfo1;
	//hCallInfo1.Clear();
	////图像掩膜
	//m_hImgMask.Mask(m_ImageCc, m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0, hCallInfo1);
	//if (check_sts(hCallInfo1, "", hCall))
	//{
	//	return 0;
	//}

	////多边形掩膜
	//m_hPolygonMask.Mask(m_ImageCc, m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0, hCallInfo1);
	//if (check_sts(hCallInfo1, "", hCall))
	//{
	//	return 0;
	//}

	//去掉边缘的线，误报

	Ipp8u pOpenMask[1] = { 1 };
	m_ImgCc.Init(m_ImageCc.nWidth, m_ImageCc.nHeight);
	if (m_nOpenVerticalSize == 1)
	{
		kxRect<int> rc;
		rc.setup(0, 0, m_ImageCc.nWidth - 1, m_ImageCc.nHeight - 1);
		m_hBaseFun.KxCopyImage(m_ImageCc, m_ImgTemp, rc);
	}
	else
	{
		m_hBaseFun.KxOpenImage(m_ImageCc, m_ImgTemp, 1, m_nOpenVerticalSize);
	}
	
	//m_hBaseFun.KxOpenImage(m_ImgCc, m_ImageCc, 1, 2);
	//m_hBaseFun.KxOpenImage(m_ImageCc, m_ImgTemp, 2, 1);

	if (check_sts(hCallInfo1, "", hCall))
	{
		return 0;
	}

	return 1;

}


int CKxSignatureCheckBeta::ConnectAnalysis(const kxCImageBuf& SrcImg, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	int nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, 5, 1, hCallInfo);
	if (check_sts(hCallInfo, "ConnectAnalysis_", hCall))
	{
		return 0;
	}

	int nCount = (std::min)((int)_Max_BlobCount, m_hBlobAnaly.GetBlobCount());

	

	if (nCount == 0)
	{
		m_hResult.m_nCount = 0;
		m_hResult.m_nStatus = _Check_Ok;
		return 1;
	}

	kxRect<int> rcWarp = m_hWarpStrech.GetParameter().m_rcCheckArea;
	int i = 0;
	for (int k = 0; k < nCount; k++)
	{
		CKxBlobAnalyse::SingleBlobInfo& pObj = m_hBlobAnaly.GetSortSingleBlob(k);
		float fRatio = (std::max)(pObj.m_rc.Width(), pObj.m_rc.Height()) *1.0f / (std::min)(pObj.m_rc.Width(), pObj.m_rc.Height());
		//if (((pObj.m_rc.Width() <= 8|| pObj.m_rc.Height() <= 8)) && fRatio > 20)
		if (0)
		{
			continue;
		}
		if (i < 4)
		{
			m_hResult.m_hBlobInfo[i].m_nAreaType = m_hParameter.m_nAreaNumber;
			m_hResult.m_hBlobInfo[i].m_nDots = pObj.m_nDots;
			m_hResult.m_hBlobInfo[i].m_nEnergy = pObj.m_nEnergy / 25;

			kxRect<int> rc;
			kxRect<int> rcDst;
			rc.setup(pObj.m_rc.left * 1, pObj.m_rc.top * 1, pObj.m_rc.right * 1, pObj.m_rc.bottom * 1);
			//m_hWarpStrech.GetRightPos(rc, rcDst);
			rcDst = rc;
			if (rcDst.left + rcDst.Width() > SrcImg.nWidth - 1 || rcDst.top + rcDst.Height() > SrcImg.nHeight - 1 || rcDst.left < 0 || rcDst.top < 0)
			{
				rcDst = rc;
			}

			rcDst.offset(rcWarp.left, rcWarp.top);

			m_hResult.m_hBlobInfo[i].m_nLeft = rcDst.left;
			m_hResult.m_hBlobInfo[i].m_nTop = rcDst.top;
			m_hResult.m_hBlobInfo[i].m_nBlobWidth = rcDst.Width();
			m_hResult.m_hBlobInfo[i].m_nBlobHeight = rcDst.Height();
			//m_hResult.m_hBlobInfo[i].m_nWHRatio = int(max(m_hResult.m_hBlobInfo[i].m_nBlobWidth / m_hResult.m_hBlobInfo[i].m_nBlobHeight, m_hResult.m_hBlobInfo[i].m_nBlobHeight / m_hResult.m_hBlobInfo[i].m_nBlobWidth));
			//m_hResult.m_hBlobInfo[i].m_nMinRectWidth = int(pObj.m_nSize / 25);
			//m_hResult.m_hBlobInfo[i].m_nAvgEnergy = pObj.m_nEnergy / pObj.m_nDots;
			//m_hResult.m_hBlobInfo[i].m_nAvgArea = int(pObj.m_fSumEnergy / 25);  //总能量

			m_hResult.m_hBlobInfo[i].m_nWHRatio = int(pObj.m_fRatio + 0.5);
			m_hResult.m_hBlobInfo[i].m_nMinRectWidth = int(pObj.m_nMinRectWidth);
			m_hResult.m_hBlobInfo[i].m_nMinRectHeight = int(pObj.m_nMinRectHeight);

			m_hResult.m_hBlobInfo[i].m_nAvgEnergy = pObj.m_nEnergy / pObj.m_nDots;
			m_hResult.m_hBlobInfo[i].m_nAvgArea = int(pObj.m_fSumEnergy / 25);  //总能量

			i++;
		}
		else
		{
			break;
		}


	}

	
	if (i > 0)
	{
		m_hResult.m_nCount = i;
		m_hResult.m_nStatus = _Check_Err;
	}
	

	return 1;
}

int CKxSignatureCheckBeta::SingleImageLearn(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return SingleImageLearn(ImgCheck, hCall);
}

//单张学习
int CKxSignatureCheckBeta::SingleImageLearn(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;


	//int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_WarpImg, hCall);
	//if (check_sts(nStatus, "SingleImageLearn_", hCall))
	//{
	//	return 0;
	//}
	//int nStatus;
	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), m_OriginalImg.nChannel);
	m_hBaseFun.KxCopyImage(ImgCheck, m_WarpImg, rcCopy);


	////签名条搜边
	Matting(m_WarpImg, false, hCallInfo);
	if (check_sts(hCallInfo, "Check_", hCall))
	{
		return 0;
	}

	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "SingleImageLearn_", hCall))
	{
		return 0;
	}


	for (int k = 0; k < _Check_Layer - 1; k++)
	{
		if (m_nAlgorithmStatus[k])
		{
			for (int i = 0; i < SUBSECTION; i++)
			{
				IppiSize roiSize = { m_rcLocationPos[i].Width(), m_rcLocationPos[i].Height() };
				if (m_ImgCheck[k].nChannel == _Type_G24)
				{
					ippiMaxEvery_8u_C3IR(m_ImgCheck[k].buf + m_rcLocationPos[i].top * m_ImgCheck[k].nPitch + m_rcLocationPos[i].left * m_ImgCheck[k].nChannel, m_ImgCheck[k].nPitch,
						m_ImgLearnTemplate[k].buf + m_rcLocationPos[i].top * m_ImgLearnTemplate[k].nPitch + m_rcLocationPos[i].left * m_ImgLearnTemplate[k].nChannel, m_ImgLearnTemplate[k].nPitch, roiSize);
				}
				else
				{
					ippiMaxEvery_8u_C1IR(m_ImgCheck[k].buf + m_rcLocationPos[i].top * m_ImgCheck[k].nPitch + m_rcLocationPos[i].left * m_ImgCheck[k].nChannel, m_ImgCheck[k].nPitch,
						m_ImgLearnTemplate[k].buf + m_rcLocationPos[i].top * m_ImgLearnTemplate[k].nPitch + m_rcLocationPos[i].left * m_ImgLearnTemplate[k].nChannel, m_ImgLearnTemplate[k].nPitch, roiSize);
				}

			}

		}


	}




	int k = _Check_Layer - 1;
	if (m_nAlgorithmStatus[k])
	{
		for (int i = 0; i < SUBSECTION; i++)
		{
			IppiSize roiSize = { m_rcLocationPos[i].Width(), m_rcLocationPos[i].Height() };
			if (m_ImgCheck[k].nChannel == _Type_G24)
			{
				ippiMinEvery_8u_C3IR(m_ImgCheck[k].buf + m_rcLocationPos[i].top * m_ImgCheck[k].nPitch + m_rcLocationPos[i].left * m_ImgCheck[k].nChannel, m_ImgCheck[k].nPitch,
					m_ImgLearnTemplate[k].buf + m_rcLocationPos[i].top * m_ImgLearnTemplate[k].nPitch + m_rcLocationPos[i].left * m_ImgLearnTemplate[k].nChannel, m_ImgLearnTemplate[k].nPitch, roiSize);
			}
			else
			{
				ippiMinEvery_8u_C1IR(m_ImgCheck[k].buf + m_rcLocationPos[i].top * m_ImgCheck[k].nPitch + m_rcLocationPos[i].left * m_ImgCheck[k].nChannel, m_ImgCheck[k].nPitch,
					m_ImgLearnTemplate[k].buf + m_rcLocationPos[i].top * m_ImgLearnTemplate[k].nPitch + m_rcLocationPos[i].left * m_ImgLearnTemplate[k].nChannel, m_ImgLearnTemplate[k].nPitch, roiSize);
			}

		}

	}

	m_nModelNumber = _Max_Layer + 1;
	m_ImgLearnTemplate[_Max_Layer].Init(m_ImgLearnTemplate[0].nWidth, m_ImgLearnTemplate[0].nHeight, m_ImgLearnTemplate[0].nChannel);
	ippsSet_8u(0, m_ImgLearnTemplate[_Max_Layer].buf, m_ImgLearnTemplate[_Max_Layer].nPitch * m_ImgLearnTemplate[_Max_Layer].nHeight);
	for (int i = 0; i < _Check_Layer - 1; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun.KxMaxEvery(m_ImgLearnTemplate[i], m_ImgLearnTemplate[_Max_Layer]);
		}
	}

	return 1;
}

int CKxSignatureCheckBeta::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex)
{
	KxCallStatus hCall;
	return SingleDefectLearn(ImgCheck, nBlobIndex, hCall);
}

//单个缺陷学习
int CKxSignatureCheckBeta::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;


	//int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_WarpImg, hCall);
	//if (check_sts(nStatus, "SingleDefectLearn_", hCall) || nStatus != 0)
	//{
	//	return 0;
	//}
	int nStatus;
	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), m_OriginalImg.nChannel);
	m_hBaseFun.KxCopyImage(ImgCheck, m_WarpImg, rcCopy);

	////签名条搜边
	Matting(m_WarpImg, false, hCallInfo);
	if (check_sts(hCallInfo, "SingleDefectLearn_", hCall))
	{
		return 0;
	}

	////签名条宽高测量
	//if (abs(m_nSignatureModelWidth - m_nSignatureWidth) > 4 * m_nSignatureSizeThresh || abs(m_nSignatureModelHeight - m_nSignatureHeight) > m_nSignatureSizeThresh)
	//{
	//	hCallInfo.nCallStatus = -1001;
	//	if (check_sts(hCallInfo, "SingleDefectLearn_WidthandHeightSizeDon'tMatch", hCall))
	//	{
	//		return 0;
	//	}
	//}

	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "SingleDefectLearn_", hCall))
	{
		return 0;
	}


	nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, 5);
	if (nStatus == 0)
	{
		return 0;
	}
	int nCount = (std::min)((int)_Max_BlobCount, m_hBlobAnaly.GetBlobCount());

	if (nCount < nBlobIndex + 1)
	{
		return 0;
	}

	int i1 = 0;
	int nFindIndex = 0;
	for (int k = 0; k < nCount; k++)
	{
		CKxBlobAnalyse::SingleBlobInfo& pObj = m_hBlobAnaly.GetSortSingleBlob(k);
		float fRatio = (std::max)(pObj.m_rc.Width(), pObj.m_rc.Height()) *1.0f / (std::min)(pObj.m_rc.Width(), pObj.m_rc.Height());
		if (((pObj.m_rc.Width() <= 8 || pObj.m_rc.Height() <= 8)) && fRatio > 20)
		{
			continue;
		}
		else
		{
			if (nBlobIndex == i1)
			{
				nFindIndex = k;
				break;
			}
			i1++;
		}
	}


	kxRect<int>  rcSmall, rcBig;
	CKxBlobAnalyse::SingleBlobInfo& pObj = m_hBlobAnaly.GetSortSingleBlob(nFindIndex);
	rcSmall = pObj.m_rc;
	rcSmall.offset(-m_rcSplitSmall[0].left, -m_rcSplitSmall[0].top);
	rcSmall.left = (rcSmall.left < 0) ? 0 : rcSmall.left;
	rcSmall.top = (rcSmall.top < 0) ? 0 : rcSmall.top;
	rcSmall.right = rcSmall.left + pObj.m_rc.Width() - 1;
	rcSmall.bottom = rcSmall.top + pObj.m_rc.Height() - 1;

	//SaveMidImage(true);

	kxRect<int> rcSplit[SUBSECTION];

	//缺陷学习，算出分块上缺陷的位置
	kxRect<int> rcLearn[SUBSECTION];
	for (int i = 0; i < SUBSECTION; i++)
	{
		rcSplit[i] = m_rcSplitSmall[i];
		rcSplit[i].offset(-m_rcSplitSmall[0].left, -m_rcSplitSmall[0].top);
		kxRect<int> rcCopy;
		rcCopy.left = (std::max)(rcSmall.left, rcSplit[i].left);
		rcCopy.top = rcSmall.top;
		rcCopy.right = (std::min)(rcSmall.right, rcSplit[i].right);
		rcCopy.bottom = rcSmall.bottom;

		if (rcCopy.Width() <= 0)
		{
			rcLearn[i].setup(0, 0, 0, 0);
		}
		else
		{
			int nOffsetX = 0;
			for (int k = 0; k < i; k++)
			{
				nOffsetX += rcSplit[k].Width();
			}
			rcCopy.offset(-nOffsetX, 0);
			rcCopy.offset(m_rcLocationPos[i].left, m_rcLocationPos[i].top);
			rcLearn[i] = rcCopy;
		}
	}




	for (int i = 0; i < _Check_Layer - 1; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			for (int k = 0; k < SUBSECTION; k++)
			{

				if (rcLearn[k].Width() > 0 || rcLearn[k].Height() > 0)
				{
					if (rcLearn[k].top < 0 || rcLearn[k].left < 0 || rcLearn[k].right > m_ImgCheck[i].nWidth - 1 || rcLearn[k].bottom > m_ImgCheck[i].nHeight - 1
						|| m_ImgLearnTemplate[i].nWidth <= 0 || m_ImgLearnTemplate[i].nHeight <= 0 || m_ImgLearnTemplate[i].buf == NULL)
					{
						return 0;
					}

					IppiSize roiSize = { rcLearn[k].Width(), rcLearn[k].Height() };

					if (m_ImgCheck[i].nChannel == _Type_G24)
					{
						ippiMaxEvery_8u_C3IR(m_ImgCheck[i].buf + rcLearn[k].top * m_ImgCheck[i].nPitch + rcLearn[k].left * 3, m_ImgCheck[i].nPitch,
							m_ImgLearnTemplate[i].buf + rcLearn[k].top * m_ImgLearnTemplate[i].nPitch + rcLearn[k].left * 3, m_ImgLearnTemplate[i].nPitch, roiSize);
					}
					else
					{
						ippiMaxEvery_8u_C1IR(m_ImgCheck[i].buf + rcLearn[k].top * m_ImgCheck[i].nPitch + rcLearn[k].left * 1, m_ImgCheck[i].nPitch,
							m_ImgLearnTemplate[i].buf + rcLearn[k].top * m_ImgLearnTemplate[i].nPitch + rcLearn[k].left * 1, m_ImgLearnTemplate[i].nPitch, roiSize);
					}
				}
			}

		}
	}

	int i = _Check_Layer - 1;
	if (m_nAlgorithmStatus[i])
	{
		for (int k = 0; k < SUBSECTION; k++)
		{
			if (rcLearn[k].Width() > 0 || rcLearn[k].Height() > 0)
			{
				IppiSize roiSize = { rcLearn[k].Width(), rcLearn[k].Height() };
				if (rcLearn[k].top < 0 || rcLearn[k].left < 0 || rcLearn[k].right > m_ImgCheck[i].nWidth - 1 || rcLearn[k].bottom > m_ImgCheck[i].nHeight - 1
					|| m_ImgLearnTemplate[i].nWidth <= 0 || m_ImgLearnTemplate[i].nHeight <= 0 || m_ImgLearnTemplate[i].buf == NULL)
				{
					return 0;
				}

				if (m_ImgCheck[i].nChannel == _Type_G24)
				{
					ippiMinEvery_8u_C3IR(m_ImgCheck[i].buf + rcLearn[k].top * m_ImgCheck[i].nPitch + rcLearn[k].left * 3, m_ImgCheck[i].nPitch,
						m_ImgLearnTemplate[i].buf + rcLearn[k].top * m_ImgLearnTemplate[i].nPitch + rcLearn[k].left * 3, m_ImgLearnTemplate[i].nPitch, roiSize);
				}
				else
				{
					ippiMinEvery_8u_C1IR(m_ImgCheck[i].buf + rcLearn[k].top * m_ImgCheck[i].nPitch + rcLearn[k].left * 1, m_ImgCheck[i].nPitch,
						m_ImgLearnTemplate[i].buf + rcLearn[k].top * m_ImgLearnTemplate[i].nPitch + rcLearn[k].left * 1, m_ImgLearnTemplate[i].nPitch, roiSize);
				}
			}
		}

	}


	m_nModelNumber = _Max_Layer + 1;
	m_ImgLearnTemplate[_Max_Layer].Init(m_ImgLearnTemplate[0].nWidth, m_ImgLearnTemplate[0].nHeight, m_ImgLearnTemplate[0].nChannel);
	ippsSet_8u(0, m_ImgLearnTemplate[_Max_Layer].buf, m_ImgLearnTemplate[_Max_Layer].nPitch * m_ImgLearnTemplate[_Max_Layer].nHeight);
	for (int i = 0; i < _Check_Layer - 1; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun.KxMaxEvery(m_ImgLearnTemplate[i], m_ImgLearnTemplate[_Max_Layer]);
		}
	}

	return 1;
}


int CKxSignatureCheckBeta::SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate)
{
	kxCImageBuf  BinaryImg, ColorImg;
	BinaryImg.Init(MaskImg.nWidth, MaskImg.nHeight);
	m_hBaseFun.KxThreshImage(MaskImg, BinaryImg, lowGate, highGate);
	ColorImg.Init(SrcDstImg.nWidth, SrcDstImg.nHeight, SrcDstImg.nChannel);
	IppiSize roiSize = { SrcDstImg.nWidth, SrcDstImg.nHeight };
	ippiDup_8u_C1C3R(BinaryImg.buf, BinaryImg.nPitch, ColorImg.buf, ColorImg.nPitch, roiSize);
	ippiSub_8u_C3IRSfs(ColorImg.buf, ColorImg.nPitch, SrcDstImg.buf, SrcDstImg.nPitch, roiSize, 0);
	return 1;
}

//把一张残差图贴到原图像上，高亮显示对应的颜色
//SrcImg 为彩色图像， CcImg 为黑白图像， nColorMode 为颜色模式
int CKxSignatureCheckBeta::AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg)
{
	kxCImageBuf  BinaryImg, ColorImg, SubImg, AddImg;
	BinaryImg.Init(CcImg.nWidth, CcImg.nHeight);
	m_hBaseFun.KxThreshImage(CcImg, BinaryImg, 1, 255);
	ColorImg.Init(CcImg.nWidth, CcImg.nHeight, SrcImg.nChannel);
	IppiSize roiSize = { CcImg.nWidth, CcImg.nHeight };
	ippiDup_8u_C1C3R(BinaryImg.buf, BinaryImg.nPitch, ColorImg.buf, ColorImg.nPitch, roiSize);
	SubImg.Init(CcImg.nWidth, CcImg.nHeight, SrcImg.nChannel);
	ippiSub_8u_C3RSfs(ColorImg.buf, ColorImg.nPitch, SrcImg.buf, SrcImg.nPitch, SubImg.buf, SubImg.nPitch, roiSize, 0);
	AddImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	ippsSet_8u(0, AddImg.buf, AddImg.nPitch*AddImg.nHeight);
	ippiCopy_8u_C1C3R(BinaryImg.buf, BinaryImg.nPitch, AddImg.buf + nColorMode, AddImg.nPitch, roiSize);
	ippiAdd_8u_C3RSfs(AddImg.buf, AddImg.nPitch, SubImg.buf, SubImg.nPitch, DstImg.buf, DstImg.nPitch, roiSize, 0);

	return 1;
}

int CKxSignatureCheckBeta::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[5])
{
	KxCallStatus hCall;
	return ShowResidualsImage(SrcImg, DstImg, nOffset, hCall);
}

//待检测图像的残差图显示
//残差图始终是一张彩色图像,
//蓝色表示检小点残差、绿色表示检细线残差、红色表示检划伤残差、黑色表示检缺失残差、白色表示
int CKxSignatureCheckBeta::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[5], KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCall.Clear();

	m_hParameter.m_nSmallDotOffset = nOffset[0];
	//m_hParameter.m_nLineOffset = nOffset[1];
	//m_hParameter.m_nScratchOffset = nOffset[2];
	m_hParameter.m_nLossOffset = nOffset[3];


	AnalysisCheckStatus();
	//int nStatus = m_hWarpStrech.CheckParallel(SrcImg, m_WarpImg, hCallInfo);
	//if (check_sts(hCallInfo, "ShowResidualsImage_", hCall) || nStatus != 0)
	//{
	//	return 0;
	//}
	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), m_OriginalImg.nChannel);
	m_hBaseFun.KxCopyImage(SrcImg, m_WarpImg, rcCopy);

	////签名条搜边

	Matting(m_WarpImg, false, hCallInfo);
	if (check_sts(hCallInfo, "Check_", hCall))
	{
		return 0;
	}

	//SaveMidImage(true);

	////签名条宽高测量
	//if (abs(m_nSignatureModelWidth - m_nSignatureWidth) > m_nSignatureSizeThresh || abs(m_nSignatureModelHeight - m_nSignatureHeight) > m_nSignatureSizeThresh)
	//{
	//	SetCheckExceptionStatus(m_WarpImg);
	//	IppStatus status = IppStatus(-10000);
	//	if (check_sts(status, "Signature's width or height is not match", hCall))
	//	{
	//		return 0;
	//	}
	//}


	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage_", hCall))
	{
		return 0;
	}

	//SaveMidImage(true);

	//Blob 分析
	ConnectAnalysis(SrcImg, hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage_", hCall))
	{
		return 0;
	}

	//原图是黑白图，先转化为彩色图像
	IppiSize roiSize = { m_WarpImg.nWidth, m_WarpImg.nHeight };
	m_ColorImg.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, 3);
	if (_Type_G8 == m_WarpImg.nChannel)
	{
		ippiDup_8u_C1C3R(m_WarpImg.buf, m_WarpImg.nPitch, m_ColorImg.buf, m_ColorImg.nPitch, roiSize);
	}
	else
	{
		ippiCopy_8u_C3R(m_WarpImg.buf, m_WarpImg.nPitch, m_ColorImg.buf, m_ColorImg.nPitch, roiSize);
	}

	//彩色图压缩1倍
	m_ResizeImg.Init(m_ColorImg.nWidth / 1, m_ColorImg.nHeight / 1, m_ColorImg.nChannel);
	m_hBaseFun.KxResizeImage(m_ColorImg, m_ResizeImg, KxSuper);
	SetMask(m_ResizeImg, m_ImgTemp, 1, 255);
	//首先获得检小点的残差图
	for (int i = 0; i < 1; i++)
	{
		m_ResidImg[i].Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight);
		ippsSet_8u(0, m_ResidImg[i].buf, m_ResidImg[i].nPitch*m_ResidImg[i].nHeight);
	}

	for (int i = 0; i < _Check_Layer - 1; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun.KxMaxEvery(m_ImgCompose[i], m_ResidImg[0]);
		}
	}
	m_hBaseFun.KxAddImage(m_ImgOffset[_Check_Layer], m_ResidImg[0]);

	//合成三张残差图
	m_AddImage.Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight, m_ResizeImg.nChannel);
	ippsSet_8u(0, m_AddImage.buf, m_AddImage.nPitch*m_AddImage.nHeight);

	for (int i = 0; i < 1; i++)
	{
		//图像掩膜
		m_hImgMask.Mask(m_ResidImg[i], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
		//多边形掩膜
		m_hPolygonMask.Mask(m_ResidImg[i], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);

		m_CompositionImg[i].Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight, m_ResizeImg.nChannel);
		AddResidualImage(m_ResidImg[i], m_ResizeImg, i, m_CompositionImg[i]);
		m_hBaseFun.KxAddImage(m_CompositionImg[i], m_AddImage);

	}

	//保留残差处
	SetMask(m_AddImage, m_ImgTemp, 0, 0);
	//保留背景处
	SetMask(m_CompositionImg[0], m_ImgTemp, 1, 255);
	m_hBaseFun.KxAddImage(m_CompositionImg[0], m_AddImage);


	//检缺失残差图，贴为黑色
	m_ResizeImg1.Init(SrcImg.nWidth, SrcImg.nHeight, 3);
	IppiSize roi = { SrcImg.nWidth, SrcImg.nHeight };
	if (_Type_G8 == SrcImg.nChannel)
	{
		ippiDup_8u_C1C3R(SrcImg.buf, SrcImg.nPitch, m_ResizeImg1.buf, m_ResizeImg1.nPitch, roi);
	}
	else
	{
		ippiCopy_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ResizeImg1.buf, m_ResizeImg1.nPitch, roi);
	}
	DstImg.Init(m_ResizeImg1.nWidth / 1, m_ResizeImg1.nHeight / 1, m_ResizeImg1.nChannel);
	m_hBaseFun.KxResizeImage(m_ResizeImg1, DstImg);


	IppiSize Roi = { m_AddImage.nWidth, m_AddImage.nHeight };

	if (m_nAlgorithmStatus[_Check_Layer - 1])
	{
		SetMask(m_AddImage, m_ImgCompose[_Check_Layer - 1], 1, 255);
	}

	kxRect<int> rcDst;
	//m_hWarpStrech.GetRightPos(m_hWarpStrech.GetParameter().m_rcCheckArea, rcDst);
	rcDst = m_hWarpStrech.GetParameter().m_rcCheckArea;

	rcDst.top = rcDst.top > 0 ? rcDst.top : 0;
	rcDst.top = rcDst.top < DstImg.nHeight - 2 ? rcDst.top : DstImg.nHeight - 2;
	rcDst.left = rcDst.left > 0 ? rcDst.left : 0;
	rcDst.left = rcDst.left < DstImg.nWidth - 2 ? rcDst.left : DstImg.nWidth - 2;

	if (rcDst.left + Roi.width > DstImg.nWidth - 1)
	{
		Roi.width = DstImg.nWidth - 1 - rcDst.left;
	}
	if (rcDst.top + Roi.height > DstImg.nHeight - 1)
	{
		Roi.height = DstImg.nHeight - 1 - rcDst.top;
	}

	ippiCopy_8u_C3R(m_AddImage.buf, m_AddImage.nPitch,
		DstImg.buf + rcDst.top  * DstImg.nPitch + rcDst.left * 3, DstImg.nPitch, Roi);

	//ippiCopy_8u_C3R(m_AddImage.buf, m_AddImage.nPitch,
	//	DstImg.buf + m_hWarpStrech.GetParameter().m_rcCheckArea.top  * DstImg.nPitch + m_hWarpStrech.GetParameter().m_rcCheckArea.left  * 3, DstImg.nPitch, Roi);




	return 1;
}




int CKxSignatureCheckBeta::AnalysisCheckStatus()
{
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_nAlgorithmStatus[i] = 0;
	}
	if (m_hParameter.m_nSmallDotOffset < _MAX_OFFSET)
	{
		for (int i = 0; i < _Check_Layer - 1; i++)
		{
			m_nAlgorithmStatus[i] = 1;
		}
		m_hParameter.m_nSmallDotOffset *= 4;
	}

	if (m_hParameter.m_nLossOffset < _MAX_OFFSET)
	{
		m_nAlgorithmStatus[_Check_Layer - 1] = 1;
		m_hParameter.m_nLossOffset *= 6;
	}

	if (m_hParameter.m_nIsLearn)
	{
		m_nLearnStatus = 1;
	}

	return 1;
}

int CKxSignatureCheckBeta::Check(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return Check(ImgCheck.buf, ImgCheck.nWidth, ImgCheck.nHeight, ImgCheck.nPitch, ImgCheck.nChannel, hCall);
}


int CKxSignatureCheckBeta::Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	return Check(ImgCheck.buf, ImgCheck.nWidth, ImgCheck.nHeight, ImgCheck.nPitch, ImgCheck.nChannel, hCall);
}

int CKxSignatureCheckBeta::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall)
{
	KxCallStatus hTmpCall;
	hTmpCall.Clear();
	hCall.Clear();
	tick_count tbb_start, tbb_end;
	m_hResult.Clear();
	//task_scheduler_init init;
	//firstly, accurate correction a image
	tbb_start = tick_count::now();

	//step 0, 拷贝图像
	m_OriginalImg.Init(nWidth, nHeight, nChannel);
	kxRect<int> rc;
	rc.setup(0, 0, nWidth - 1, nHeight - 1);
	m_hBaseFun.KxCopyImage(buf, nWidth, nHeight, nPitch, nChannel,
		m_OriginalImg.buf, m_OriginalImg.nWidth, m_OriginalImg.nHeight, m_OriginalImg.nPitch, m_OriginalImg.nChannel, rc);


	//step 1, 精确校正图像
	//kxRect<int>& rcCheckArea = m_hWarpStrech.GetParameter().m_rcCheckArea;
	//m_WarpImg.Init(rcCheckArea.Width(), rcCheckArea.Height(), m_OriginalImg.nChannel);
	//int nStatus = m_hWarpStrech.CheckParallel(m_OriginalImg, m_WarpImg);
	//if (nStatus)
	//{
	//	m_hResult.m_nStatus = _Similarity_Err;
	//	m_hResult.m_nKernIndex = nStatus - 1;
	//	return 0;
	//}

	int nStatus = m_hWarpStrech.CheckParallel(m_OriginalImg, m_ImgPos, hCall);
	if (nStatus)
	{
		m_hResult.m_nStatus = _Similarity_Err;
		m_hResult.m_nKernIndex = nStatus - 1;
		return 0;
	}

	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), m_OriginalImg.nChannel);
	m_hBaseFun.KxCopyImage(m_OriginalImg, m_WarpImg, rcCopy);



	tbb_end = tick_count::now();
	//printf("-----warp process cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	tbb_start = tick_count::now();
	////step 2,签名条搜边
	Matting(m_WarpImg, false, hTmpCall);
	if (check_sts(hTmpCall, "Matting_", hCall))
	{
		SetCheckExceptionStatus(m_WarpImg);
		return 0;
	}

	//改由距离模板监控
	////签名条宽高测量
	//if (abs(m_nSignatureModelWidth - m_nSignatureWidth) > m_nSignatureSizeThresh || abs(m_nSignatureModelHeight - m_nSignatureHeight) > m_nSignatureSizeThresh)
	//{
	//	SetCheckExceptionStatus(m_WarpImg);
	//	return 0;
	//}
	//printf("-----w : %d      -----h：%d ms\n", abs(m_nSignatureModelWidth - m_nSignatureWidth), abs(m_nSignatureModelHeight - m_nSignatureHeight));


	//step 3,距离检查
	//if (m_hCalDistance.CheckSpecial(m_OriginalImg, m_rcSignaturePos, m_rcBaseSignaturePos, hTmpCall))
	//{
	//	m_hCalDistance.GetHorVerDistance(m_hResult.m_nDistanceX, m_hResult.m_nDistanceY);
	//	m_hResult.m_nStatus = _Distance_Err;
	//	return 0;
	//}


	tbb_end = tick_count::now();
	//printf("-----search process cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	tbb_start = tick_count::now();


	//step 4,学习
	LearnTemplate(hTmpCall);
	if (check_sts(hTmpCall, "Learn_", hCall))
	{
		SetCheckExceptionStatus(m_WarpImg);
		return 0;
	}

	//step 5,残差对减
	ResidualsAnalys(hTmpCall);
	if (check_sts(hTmpCall, "Sub_", hCall))
	{
		SetCheckExceptionStatus(m_WarpImg);
		return 0;
	}

	//step 6,连通域分析
	ConnectAnalysis(m_OriginalImg, hTmpCall);
	if (check_sts(hTmpCall, "分析中_", hCall))
	{
		SetCheckExceptionStatus(m_WarpImg);
		return 0;
	}


	tbb_end = tick_count::now();
	//printf("-----anlaysis process cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);

	//SaveMidImage(true);

	return 1;
}

int CKxSignatureCheckBeta::SetCheckExceptionStatus(const kxCImageBuf& ImgCheck)
{
	m_hResult.m_nCount = 1;
	m_hResult.m_hBlobInfo[0].m_nAreaType = m_hParameter.m_nAreaNumber;
	m_hResult.m_hBlobInfo[0].m_nDots = ImgCheck.nWidth * ImgCheck.nHeight;
	m_hResult.m_hBlobInfo[0].m_nEnergy = ImgCheck.nWidth * ImgCheck.nHeight;

	m_hResult.m_hBlobInfo[0].m_nLeft = m_hWarpStrech.GetParameter().m_rcCheckArea.left;
	m_hResult.m_hBlobInfo[0].m_nTop = m_hWarpStrech.GetParameter().m_rcCheckArea.top;
	m_hResult.m_hBlobInfo[0].m_nBlobWidth = ImgCheck.nWidth - 1;
	m_hResult.m_hBlobInfo[0].m_nBlobHeight = ImgCheck.nHeight - 1;
	m_hResult.m_hBlobInfo[0].m_nWHRatio = 0;
	m_hResult.m_hBlobInfo[0].m_nMinRectWidth = abs(m_nSignatureModelWidth - m_nSignatureWidth);
	m_hResult.m_hBlobInfo[0].m_nMinRectHeight = abs(m_nSignatureModelHeight - m_nSignatureHeight);
	m_hResult.m_hBlobInfo[0].m_nAvgEnergy = 1;
	m_hResult.m_hBlobInfo[0].m_nAvgArea = 1;
	m_hResult.m_nStatus = _Check_Err;


	return 1;
}


int CKxSignatureCheckBeta::SaveMidImage(bool bSave)
{
#if defined( _WIN32 ) || defined ( _WIN64 )
	if (bSave)
	{
		char sz[128];
		static int n = 0;

		sprintf_s(sz, 128, "D:\\Save\\Signature\\Location\\m_ImgGradient_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgGradient);

		sprintf_s(sz, 128, "D:\\Save\\Signature\\Location\\m_ImgGradient1_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgGradient1);

		sprintf_s(sz, 128, "D:\\Save\\Signature\\Location\\m_ImgGradient0_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgGradient0);


		sprintf_s(sz, 128, "D:\\Save\\Signature\\Base\\m_ImgCreateLocation_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgCreateLocation);

		sprintf_s(sz, 128, "D:\\Save\\Signature\\Edge\\EdgeImg_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_EdgeImg);

		sprintf_s(sz, 128, "D:\\Save\\Signature\\warp\\m_WarpImg_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_WarpImg);

		for (int i = 0; i < 3; i++)
		{
			sprintf_s(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgCreateGradient_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgCreateGradient[i]);

			sprintf_s(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgCreateBase_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgCreateBase[i]);

			sprintf_s(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgCheck_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgCheck[i]);

			sprintf_s(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgLearn_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgLearn[i]);


			sprintf_s(sz, 128, "D:\\Save\\Signature\\Slider\\m_ImgSlider_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgSlidCc[i]);

			sprintf_s(sz, 128, "D:\\Save\\Signature\\Slider\\m_ImgOffset_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgOffset[i]);

			sprintf_s(sz, 128, "D:\\Save\\Signature\\Slider\\m_ImgSlidCcMax_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgSlidCcMax[i]);


			sprintf_s(sz, 128, "D:\\Save\\Signature\\Template\\m_ImgTemplate_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgTemplate[i]);
		}


		sprintf_s(sz, 128, "D:\\Save\\Signature\\Cc\\ImgTemp_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgTemp);

		sprintf_s(sz, 128, "D:\\Save\\Signature\\Edge\\Offset_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgOffset[_Check_Layer]);

		n++;
	}
#else
	if (bSave)
	{
		char sz[128];
		static int n = 0;

		snprintf(sz, 128, "D:\\Save\\Signature\\Location\\m_ImgGradient_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgGradient);

		snprintf(sz, 128, "D:\\Save\\Signature\\Location\\m_ImgGradient1_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgGradient1);

		snprintf(sz, 128, "D:\\Save\\Signature\\Location\\m_ImgGradient0_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgGradient0);


		snprintf(sz, 128, "D:\\Save\\Signature\\Base\\m_ImgCreateLocation_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgCreateLocation);

		snprintf(sz, 128, "D:\\Save\\Signature\\Edge\\EdgeImg_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_EdgeImg);

		snprintf(sz, 128, "D:\\Save\\Signature\\warp\\m_WarpImg_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_WarpImg);

		for (int i = 0; i < 3; i++)
		{
			snprintf(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgCreateGradient_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgCreateGradient[i]);

			snprintf(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgCreateBase_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgCreateBase[i]);

			snprintf(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgCheck_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgCheck[i]);

			snprintf(sz, 128, "D:\\Save\\Signature\\Check\\m_ImgLearn_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgLearn[i]);


			snprintf(sz, 128, "D:\\Save\\Signature\\Slider\\m_ImgSlider_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgSlidCc[i]);

			snprintf(sz, 128, "D:\\Save\\Signature\\Slider\\m_ImgOffset_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgOffset[i]);

			snprintf(sz, 128, "D:\\Save\\Signature\\Slider\\m_ImgSlidCcMax_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgSlidCcMax[i]);


			snprintf(sz, 128, "D:\\Save\\Signature\\Template\\m_ImgTemplate_%d_%d.bmp", n, i);
			m_hBaseFun.SaveBMPImage_h(sz, m_ImgTemplate[i]);
		}


		snprintf(sz, 128, "D:\\Save\\Signature\\Cc\\ImgTemp_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgTemp);

		snprintf(sz, 128, "D:\\Save\\Signature\\Edge\\Offset_%d.bmp", n);
		m_hBaseFun.SaveBMPImage_h(sz, m_ImgOffset[_Check_Layer]);

		n++;
	}

#endif // defined

	return 1;

}
