
#include "KxQuanxiBiaoCheck.h"
#include "KxReadXml.h"

CKxQuanxiBiaoCheck::CKxQuanxiBiaoCheck()
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

	m_nSegmentThresh = 50;

	m_nSegmentLayer = 0;


}

CKxQuanxiBiaoCheck::~CKxQuanxiBiaoCheck()
{

}

bool CKxQuanxiBiaoCheck::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
{

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
	m_hFun.KxCopyImage(BaseImg, m_BaseImg, rcCopy);

	//生成自动Mask
	kxRect<int>& rc = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_CreateMask.Init(rc.Width(), rc.Height(), m_BaseImg.nChannel);
	m_hBaseFun[0].KxCopyImage(m_BaseImg, m_CreateMask, rc);
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

	AnalysisCheckStatus(m_BaseImg.nChannel);

	//kxRect<int> rcCopy;
	//rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;

	m_WarpImg.Init(rc.Width(), rc.Height(), m_BaseImg.nChannel);
	m_hFun.KxCopyImage(m_BaseImg, m_WarpImg, rc);

	m_hParameter.m_rcEdgeRect.left = (std::max)(m_hParameter.m_rcEdgeRect.left, 0);
	m_hParameter.m_rcEdgeRect.top = (std::max)(m_hParameter.m_rcEdgeRect.top, 0);
	m_hParameter.m_rcEdgeRect.right = (std::min)(m_hParameter.m_rcEdgeRect.right, m_BaseImg.nWidth-1);
	m_hParameter.m_rcEdgeRect.bottom = (std::min)(m_hParameter.m_rcEdgeRect.bottom, m_BaseImg.nHeight-1);

	if (m_hParameter.m_nIsSearchEdge)
	{
		m_ImgEdge.Init(m_hParameter.m_rcEdgeRect.Width(), m_hParameter.m_rcEdgeRect.Height(), m_BaseImg.nChannel);
		m_hFun.KxCopyImage(m_BaseImg, m_ImgEdge, m_hParameter.m_rcEdgeRect);

		KxCallStatus hCallInfo, hCall;
		KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, true, hCallInfo);
		if (check_sts(hCallInfo, "底板检查出多个全息标_", hCall))
		{
			return false;
		}
		else
		{
			m_ImgTemplateArea.Init(m_ImgQuanxiBiaoArea.nWidth, m_ImgQuanxiBiaoArea.nHeight);
			kxRect<int> rc1;
			rc1.setup(0, 0, m_ImgQuanxiBiaoArea.nWidth - 1, m_ImgQuanxiBiaoArea.nHeight - 1);
			m_hFun.KxCopyImage(m_ImgQuanxiBiaoArea, m_ImgTemplateArea, rc1);
		}

		m_ImgAddEdge.Init(m_ImgQuanxiBiaoArea.nWidth, m_ImgQuanxiBiaoArea.nHeight);
		ippsSet_8u(0, m_ImgAddEdge.buf, m_ImgAddEdge.nPitch * m_ImgAddEdge.nHeight);
	}



	//m_hFun.KxDilateImage(m_ImgQuanxiBiaoArea, m_ImgTemplateArea);


	return true;

}

bool CKxQuanxiBiaoCheck::ReadParaXml(const char* filePath)
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

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "线灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nLineOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "灰度高灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nGrayOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "灰度低灵敏度", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nLossOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "全息标设置", "阈值", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_nSegmentThresh);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "全息标设置", "分割层面", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToInt(szResult, m_nSegmentLayer);
		if (!nStatus)
		{
			return false;
		}
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "边缘框", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcEdgeRect);
		if (!nStatus)
		{
			return false;
		}
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "全息标设置", "轮廓检查", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsContoursCheck);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "全息标设置", "边缘定位", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsSearchEdge);
		if (!nStatus)
		{
			return false;
		}
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "全息标设置", "忽略白点", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsOpencheckWhiteDot);
		if (!nStatus)
		{
			return false;
		}
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


	return true;
}



bool CKxQuanxiBiaoCheck::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
{

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
	m_hFun.KxCopyImage(BaseImg, m_BaseImg, rcCopy);

	//生成自动Mask
	kxRect<int>& rc = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_CreateMask.Init(rc.Width(), rc.Height(), m_BaseImg.nChannel);
	m_hBaseFun[0].KxCopyImage(m_BaseImg, m_CreateMask, rc);
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

	AnalysisCheckStatus(m_BaseImg.nChannel);


	m_WarpImg.Init(rc.Width(), rc.Height(), m_BaseImg.nChannel);
	m_hFun.KxCopyImage(m_BaseImg, m_WarpImg, rc);

	m_hParameter.m_rcEdgeRect.left = (std::max)(m_hParameter.m_rcEdgeRect.left, 0);
	m_hParameter.m_rcEdgeRect.top = (std::max)(m_hParameter.m_rcEdgeRect.top, 0);
	m_hParameter.m_rcEdgeRect.right = (std::min)(m_hParameter.m_rcEdgeRect.right, m_BaseImg.nWidth - 1);
	m_hParameter.m_rcEdgeRect.bottom = (std::min)(m_hParameter.m_rcEdgeRect.bottom, m_BaseImg.nHeight - 1);

	if (m_hParameter.m_nIsSearchEdge)
	{
		m_ImgEdge.Init(m_hParameter.m_rcEdgeRect.Width(), m_hParameter.m_rcEdgeRect.Height(), m_BaseImg.nChannel);
		m_hFun.KxCopyImage(m_BaseImg, m_ImgEdge, m_hParameter.m_rcEdgeRect);

		KxCallStatus hCallInfo, hCall;
		KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, true, hCallInfo);
		if (check_sts(hCallInfo, "全息标预处理_", hCall))
		{
			return false;
		}

		m_ImgTemplateArea.Init(m_ImgQuanxiBiaoArea.nWidth, m_ImgQuanxiBiaoArea.nHeight);
		kxRect<int> rc1;
		rc1.setup(0, 0, m_ImgQuanxiBiaoArea.nWidth - 1, m_ImgQuanxiBiaoArea.nHeight - 1);
		m_hFun.KxCopyImage(m_ImgQuanxiBiaoArea, m_ImgTemplateArea, rc1);
	}



	return true;

}

bool CKxQuanxiBiaoCheck::ReadParaXmlinEnglish(const char* filePath)
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

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "LineSensitivity", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nLineOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "GrayHighSensitivity", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nGrayOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "GrayLowSensitivity", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nLossOffset);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Holographic", "ThresholdValue", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_nSegmentThresh);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Holographic", "Layer", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToInt(szResult, m_nSegmentLayer);
		if (!nStatus)
		{
			return false;
		}
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Holographic", "OutlineCheck", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsContoursCheck);
		if (!nStatus)
		{
			return false;
		}
	}



	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "EdgeBox", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcEdgeRect);
		if (!nStatus)
		{
			return false;
		}
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "EdgeLocation", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsSearchEdge);
		if (!nStatus)
		{
			return false;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "Ignorewhitedot", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_nIsOpencheckWhiteDot);
		if (!nStatus)
		{
			return false;
		}
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


	return true;
}


bool CKxQuanxiBiaoCheck::ReadVesion1Para(FILE* fp)    //读取版本1参数
{
	if (fread(&m_hParameter.m_nIsOpenSurfaceCheck, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nAreaNumber, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nCheckMethod, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	if (fread(&m_hParameter.m_nModelProcessSize, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nIsLearn, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}


	return true;
}

bool CKxQuanxiBiaoCheck::ReadVesion2Para(FILE* fp)    //读取版本2参数
{
	if (fread(&m_hParameter.m_nIsOpenSurfaceCheck, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nAreaNumber, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nCheckMethod, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	if (fread(&m_hParameter.m_nModelProcessSize, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nIsLearn, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nSmallDotOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nLineOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nScratchOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nLossOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nGrayOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcEdgeRect, sizeof(kxRect<int>), 1, fp) != 1)
	{
		return false;
	}


	return true;
}


bool CKxQuanxiBiaoCheck::ReadPara(FILE*  fp)
{
	if (fread(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_hParameter.m_szVersion, "CheckControl1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else if (strcmp(m_hParameter.m_szVersion, "CheckControl2.0") == 0)
	{
		return ReadVesion2Para(fp);
	}
	else
	{
		return false;
	}
}


bool CKxQuanxiBiaoCheck::Read(FILE*  fp)
{
	//检测控制参数
	if (!ReadPara(fp))
		return false;
	//距离标定参数
	if (!m_hCalDistance.Read(fp))
	{
		return false;
	}
	//图像校正参数
	if (!m_hWarpStrech.Read(fp))
	{
		return false;
	}
	//色差检查参数
	if (!m_hColorDiff.Read(fp))
	{
		return false;
	}
	//掩膜参数
	if (!m_hImgMask.Read(fp))
	{
		return false;
	}
	//多边形掩膜参数
	if (!m_hPolygonMask.Read(fp))
		return false;

	//载入底板图像
	if (!m_BaseImg.Read(fp))
	{
		return false;
	}
	//生成自动Mask
	kxRect<int>& rc = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_CreateMask.Init(rc.Width(), rc.Height(), m_BaseImg.nChannel);
	m_hBaseFun[0].KxCopyImage(m_BaseImg, m_CreateMask, rc);
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

	AnalysisCheckStatus(m_BaseImg.nChannel);


	return true;
}

bool CKxQuanxiBiaoCheck::WriteVesion1Para(FILE* fp)    //写入版本1参数
{
	if (fwrite(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nIsOpenSurfaceCheck, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nAreaNumber, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nCheckMethod, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nModelProcessSize, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nIsLearn, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nOffset, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	return true;
}



bool CKxQuanxiBiaoCheck::WritePara(FILE*  fp)
{
	if (strcmp(m_hParameter.m_szVersion, "CheckControl1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return WriteVesion1Para(fp);
	}
	////else if (strcmp(m_hParameter.m_szVersion, "CheckControl2.0") == 0)
	////{
	////	return WriteVesion2Para(fp);
	////}
	else
	{
		return false;
	}
}

bool CKxQuanxiBiaoCheck::Write(FILE*  fp)
{
	if (!WritePara(fp))
		return false;

	if (!m_hCalDistance.Write(fp))
	{
		return false;
	}

	if (!m_hWarpStrech.Write(fp))
	{
		return false;
	}

	if (!m_hColorDiff.Write(fp))
	{
		return false;
	}

	if (!m_hImgMask.Write(fp))
	{
		return false;
	}
	//多边形掩膜参数
	if (!m_hPolygonMask.Write(fp))
		return false;

	if (!m_BaseImg.Write(fp))
	{
		return false;
	}

	return true;

}

//从网络中获取参数
int CKxQuanxiBiaoCheck::ReadParaFromNet(unsigned char*& point)
{
	CKxBaseFunction hFun;
	memcpy(&m_hParameter, point, sizeof(Parameter));
	point += sizeof(Parameter);

	if (!m_hCalDistance.ReadParaFromNet(point))
	{
		return false;
	}
	if (!m_hWarpStrech.ReadParaFromNet(point))
	{
		return false;
	}
	if (!m_hColorDiff.ReadParaFromNet(point))
	{
		return false;
	}
	if (!m_hImgMask.ReadParaFromNet(point))
	{
		return false;
	}
	if (!hFun.readImgBufFromMemory(m_BaseImg, point))
	{
		return false;
	}
	return true;
}

int CKxQuanxiBiaoCheck::LoadTemplateImg(const char* lpszFile)
{
	FILE*   fp;
#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "rb") != 0)
	{
		m_nModelNumber = 0;
		for (int k = 0; k < _Max_Layer + 1; k++)
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

int CKxQuanxiBiaoCheck::SaveTemplateImg(const char* lpszFile)
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


int CKxQuanxiBiaoCheck::SaveLearnTemplateImg(const char* lpszFile)
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

int CKxQuanxiBiaoCheck::LoadLearnTemplateImg(const char* lpszFile)
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


int CKxQuanxiBiaoCheck::ReadLocalLearnImg(FILE* fp)
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

	//...............................................
	return TRUE;
}



int CKxQuanxiBiaoCheck::WriteLocalLearnImg(FILE* fp)
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

	//...............................................
	return TRUE;
}

int CKxQuanxiBiaoCheck::ReadImg(FILE* fp)
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
		m_hFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
	}


	//...............................................
	return TRUE;
}

int CKxQuanxiBiaoCheck::WriteImg(FILE* fp)
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
		m_hFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
	}

	//...............................................
	return TRUE;
}

//处理精确拉伸后的图像
int  CKxQuanxiBiaoCheck::ProcessSurfaceWarpImages(KxCallStatus& hCall)
{
	hCall.Clear();
	//2张原始分辨率大小的水平垂直梯度图、4张4*4压缩分辨率下的梯度图、1张压缩分辨率下合成梯度图
	int nMode[_Max_Layer];
	memset(nMode, 0, sizeof(int)*_Max_Layer);
	for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
	{
		nMode[i] = 1;
	}
	for (int i = _LINE_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	{
		nMode[i] = 2;
	}
	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		nMode[i] = 3;   //灰度检查
	}

	int nAlgorithmOpen[_Max_Layer];
	KxCallStatus hCallInfo[_Max_Layer];
	for (int i = 0; i < _Max_Layer; i++)
	{
		nAlgorithmOpen[i] = m_nAlgorithmStatus[i];
		hCallInfo[i].Clear();
	}
	if (nAlgorithmOpen[_Max_Layer - 1])
	{
		for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
		{
			nAlgorithmOpen[i] = 1;
		}
	}

	int nLargeSize;
	if (m_ExtendWarpImg.nChannel == _Type_G8)
	{
		nLargeSize = m_nLargeSize;
	}
	else
	{
		nLargeSize = 5;
	}

	//原图水平垂直的单向梯度\压缩图上的黑白模式下4个方向的单向梯度
	parallel_for(blocked_range<int>(0, _Max_Layer - 1),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		{
			if (nAlgorithmOpen[i] || m_nLearnStatus)
			{
				switch (nMode[i])
				{
				case 0: //检点
				{
							m_hGradient[i].ParallelSingleDirGradientSame(m_ExtendWarpImg, m_ImgCheck[i], i, CKxGradientProcess::_AverageFilter, 9, 255, hCallInfo[i]);
							break;
				}
				case 1:  //检线
				{
							 m_ImgCheck[i].Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight, m_ExtendWarpImg.nChannel);
							 ippsSet_8u(0, m_ImgCheck[i].buf, m_ImgCheck[i].nPitch * m_ImgCheck[i].nHeight);
							 for (int j = 0; j < 1; j++)
							 {
								// bool bWhite = ((j == 1) ? 1 : 0);
								 int nStatus = m_hGradient[i].SingleDirGradientSameWithBigSmooth(m_ExtendWarpImg, m_MidImg[i], i - _SAMLLDOTS_INDEX_END, 1, 15, 0,  hCallInfo[i]);
								 if (nStatus == 0)
								 {
									 break;
								 }
								 m_hBaseFun[i].KxMaxEvery(m_MidImg[i], m_ImgCheck[i]);
							 }
							 break;
				}
				case 2:   //检查划伤
				{
							  m_ImgCheck[i].Init(m_ExtendWarpImg.nWidth / 4, m_ExtendWarpImg.nHeight / 4, m_ExtendWarpImg.nChannel);
							  ippsSet_8u(0, m_ImgCheck[i].buf, m_ImgCheck[i].nPitch * m_ImgCheck[i].nHeight);
							  for (int j = 0; j < _CheckModeNum; j++)
							  {
								  int nStatus = m_hGradient[i].SingleDirGradientLV(m_ExtendWarpImg, m_MidImg[i], j, 255, hCallInfo[i]);
								  if (nStatus == 0)
								  {
									  break;
								  }
								  m_hBaseFun[i].KxAddImage(m_MidImg[i], m_ImgCheck[i]);
							  }
							  break;
				}
				default: //灰度检，精细检查
				{
							 m_ImgCheck[i].Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight, m_ExtendWarpImg.nChannel);
							 kxRect<int> rc;
							 rc.setup(0, 0, m_ExtendWarpImg.nWidth - 1, m_ExtendWarpImg.nHeight - 1);
							 m_hBaseFun[i].KxCopyImage(m_ExtendWarpImg, m_ImgCheck[i], rc, hCallInfo[i]);
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

	if (m_nAlgorithmStatus[_Max_Layer - 1] || m_nLearnStatus)
	{
		m_ImgCheck[_Max_Layer - 1].Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight, m_ExtendWarpImg.nChannel);

		ippsSet_8u(0, m_ImgCheck[_Max_Layer - 1].buf, m_ImgCheck[_Max_Layer - 1].nPitch * m_ImgCheck[_Max_Layer - 1].nHeight);
		for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxParallelMaxEvery(m_ImgCheck[i], m_ImgCheck[_Max_Layer - 1]);
		}
	}

	return 1;
}

//学习图像
int  CKxQuanxiBiaoCheck::LearnTemplate(KxCallStatus& hCall)
{
	hCall.Clear();
	if (!m_hParameter.m_nIsLearn)
	{
		return 0;
	}

	int nLearnHighLowMode[_Max_Layer];
	memset(nLearnHighLowMode, 0, sizeof(int)*_Max_Layer);
	for (int i = 0; i <= _SCRATCH_INDEX_END; i++)
	{
		nLearnHighLowMode[i] = 1;  //学高
	}


	//模板操作
	int nModelProcessSz[_Max_Layer];
	for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	{
		nModelProcessSz[i] = _5X5;
	}
	for (int i = _SAMLLDOTS_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	{
		nModelProcessSz[i] = _5X5;
	}
	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		nModelProcessSz[i] = _3X3;  //灰度检查
	}
	for (int i = _GRAY_INDEX_END; i < _Max_Layer; i++)
	{
		nModelProcessSz[i] = _5X5;  //整体检缺
	}

	parallel_for(blocked_range<int>(0, _Max_Layer),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
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

		m_nModelNumber = _Max_Layer + 1;
		m_ImgTemplate[_Max_Layer].Init(m_ImgTemplate[_SAMLLDOTS_INDEX_END].nWidth, m_ImgTemplate[_SAMLLDOTS_INDEX_END].nHeight, m_ImgTemplate[_SAMLLDOTS_INDEX_END].nChannel);
		ippsSet_8u(0, m_ImgTemplate[_Max_Layer].buf, m_ImgTemplate[_Max_Layer].nPitch * m_ImgTemplate[_Max_Layer].nHeight);
		for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
		{
			if (m_nLearnStatus)
			{
				m_hFun.KxParallelMaxEvery(m_ImgTemplate[i], m_ImgTemplate[_Max_Layer]);
			}
		}
		for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
		{
			if (m_nLearnStatus)
			{
				m_hFun.KxParallelMaxEvery(m_ImgTemplate[i], m_ImgTemplate[_Max_Layer]);
			}
		}

		for (int k = 0; k < _Max_Layer + 1; k++)
		{
			m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
			kxRect<int> rc;
			rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
			m_hFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
		}

		m_nLearnStatus = 0;
	}

	return 1;
}




//残差分析并且合并为一张残差图
int CKxQuanxiBiaoCheck::ResidualsAnalys(KxCallStatus& hCall)
{
	hCall.Clear();
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_ImgSlidCc[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight, m_ImgCheck[i].nChannel);
		m_ImgSliderNorm[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight);
		m_ImgOffset[i].Init(m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight);
		m_ImgSlidCcMax[i].Init(m_ImgOffset[i].nWidth, m_ImgOffset[i].nHeight);
		m_ResImg[i].Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight);
	}

	int nCheckHighLowMode[_Max_Layer];
	memset(nCheckHighLowMode, 0, sizeof(int)*_Max_Layer);
	KxCallStatus hCallInfo[_Max_Layer];
	for (int i = 0; i <= _SCRATCH_INDEX_END; i++)
	{
		nCheckHighLowMode[i] = 1;  //检高
		hCallInfo[i].Clear();
	}

	int nGridXY[_Max_Layer];
	int nOffset[_Max_Layer];
	int nSpeckleMaxSize[_Max_Layer];
	//设置去噪卷积核的尺寸
	IppiSize sizeMaskFilter[_Max_Layer];
	sizeMaskFilter[0] = { 1, 3 };
	sizeMaskFilter[1] = { 3, 1 };
	sizeMaskFilter[2] = { 1, 3 };
	sizeMaskFilter[3] = { 3, 1 };
	sizeMaskFilter[4] = { 3, 3 };
	sizeMaskFilter[5] = { 3, 3 };


	for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	{
		nGridXY[i] = 20;
		nOffset[i] = m_hParameter.m_nSmallDotOffset;
		nSpeckleMaxSize[i] = 4;  //一个点的噪声从原图压缩到压缩图上会变成4个
		m_hSlider[i].SetSliderRange(-4, 1, 4);
	}
	for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
	{
		nGridXY[i] = 20;
		nOffset[i] = m_hParameter.m_nLineOffset;
		nSpeckleMaxSize[i] = 2;
		m_hSlider[i].SetSliderRange(-4, 1, 4);
	}
	for (int i = _LINE_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	{
		nGridXY[i] = 20;
		nOffset[i] = m_hParameter.m_nScratchOffset;
		nSpeckleMaxSize[i] = 2;
		m_hSlider[i].SetSliderRange(-4, 1, 4);
	}
	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		nGridXY[i] = 20;
		//nOffset[i] = m_hParameter.m_nGrayOffset;
		nSpeckleMaxSize[i] = 2;
		m_hSlider[i].SetSliderRange(-4, 1, 4);
	}

	nOffset[_SCRATCH_INDEX_END] = m_nHighOffset;
	nOffset[_SCRATCH_INDEX_END + 1] = m_nLowOffset;


	nOffset[_Max_Layer - 1] = m_hParameter.m_nLossOffset;
	nGridXY[_Max_Layer - 1] = 20;

	parallel_for(blocked_range<int>(0, _Max_Layer),
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

				if (m_ImgSlidCc[i].nChannel == _Type_G24)
				{
					IppiSize roiSize = { m_ImgSlidCc[i].nWidth, m_ImgSlidCc[i].nHeight };
					ippiGradientColorToGray_8u_C3C1R(m_ImgSlidCc[i].buf, m_ImgSlidCc[i].nPitch, m_ImgSliderNorm[i].buf, m_ImgSliderNorm[i].nPitch, roiSize, ippiNormInf);
				}
				else
				{
					IppiSize roiSize = { m_ImgSlidCc[i].nWidth, m_ImgSlidCc[i].nHeight };
					ippiCopy_8u_C1R(m_ImgSlidCc[i].buf, m_ImgSlidCc[i].nPitch, m_ImgSliderNorm[i].buf, m_ImgSliderNorm[i].nPitch, roiSize);
				}



				IppiSize roiSize = { m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight };
				m_hBaseFun[i].KxSubCImage(m_ImgSliderNorm[i], m_ImgOffset[i], nOffset[i]);

				//采用单向开操作去噪声
				if (i < _LINE_INDEX_END)
				{
					if (i < 4)
					{
						Ipp8u pMask[3] = { 1, 1, 1 };
						m_hBaseFun[i].KxOpenImage(m_ImgOffset[i], m_ImgSlidCcMax[i], sizeMaskFilter[i].width, sizeMaskFilter[i].height, pMask, hCallInfo[i]);
					}
					else if (i == 4)
					{
						Ipp8u pMask[3 * 3] = { 0, 0, 1,
							0, 1, 0,
							1, 0, 0 };
						m_hBaseFun[i].KxOpenImage(m_ImgOffset[i], m_ImgSlidCcMax[i], sizeMaskFilter[i].width, sizeMaskFilter[i].height, pMask, hCallInfo[i]);
					}
					else
					{
						Ipp8u pMask[3 * 3] = { 1, 0, 0,
							0, 1, 0,
							0, 0, 1 };
						m_hBaseFun[i].KxOpenImage(m_ImgOffset[i], m_ImgSlidCcMax[i], sizeMaskFilter[i].width, sizeMaskFilter[i].height, pMask, hCallInfo[i]);
					}

					if (check_sts(hCallInfo[i], "", hCall))
					{
						break;
					}
				}
				else if (i >= _SCRATCH_INDEX_END && i <= _SCRATCH_INDEX_END+1)
				{
					m_hBaseFun[i].KxDilateImage(m_ImgOffset[i], m_ImgSlidCcMax[i], 3, 3); //膨胀一下让灰度点数 能量变大和梯度相当
					ippiMulC_8u_C1IRSfs(4, m_ImgSlidCcMax[i].buf, m_ImgSlidCcMax[i].nPitch, roiSize, 0);
				}
				else
				{
					kxRect<int> rc;
					rc.setup(0, 0, m_ImgOffset[i].nWidth - 1, m_ImgOffset[i].nHeight - 1);
					m_hBaseFun[i].KxCopyImage(m_ImgOffset[i], m_ImgSlidCcMax[i], rc);
				}

				////对于原始分辨先压4倍
				//if ((i < _SAMLLDOTS_INDEX_END) || (i >= _SCRATCH_INDEX_END && i < _GRAY_INDEX_END))
				//{
				//	ConvertSliderCc(m_ImgSlidCcMax[i], m_ImgTransform[i], i, hCallInfo[i]);
				//	if (check_sts(hCallInfo[i], "", hCall))
				//	{
				//		break;
				//	}
				//	//再去掉能量弱的噪点
				//	m_hBaseFun[i].KxFilterSpeckles(m_ImgTransform[i], m_ResImg[i], nSpeckleMaxSize[i], hCallInfo[i]);

				//	if (check_sts(hCallInfo[i], "", hCall))
				//	{
				//		break;
				//	}

				//}
				//else
				{
					kxRect<int> rc;
					rc.setup(0, 0, m_ImgSlidCcMax[i].nWidth - 1, m_ImgSlidCcMax[i].nHeight - 1);
					m_hBaseFun[i].KxCopyImage(m_ImgSlidCcMax[i], m_ResImg[i], rc);
				}






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

	m_ImgTemp.Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight);
	ippsSet_8u(0, m_ImgTemp.buf, m_ImgTemp.nPitch*m_ImgTemp.nHeight);



	for (int i = 0; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxParallelMaxEvery(m_ResImg[i], m_ImgTemp);
		}
	}

	//if (m_hParameter.m_nIsContoursCheck)
	//{
	//	//m_hBaseFun[_SAMLLDOTS_INDEX_END].KxParallelMaxEvery(m_ImgDiff, m_ImgTemp);
	//	//边缘部分单独分析
	//}
	

	//图像掩膜
	//m_hImgMask.Mask(m_ImgTemp, m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
	m_hImgMask.Mask(m_ImgTemp, m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, 0, hCallInfo[0]);
	if (check_sts(hCallInfo[0], "", hCall))
	{
		return 0;
	}



	//多边形掩膜
	m_hPolygonMask.Mask(m_ImgTemp, m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, 0, hCallInfo[0]);
	if (check_sts(hCallInfo[0], "", hCall))
	{
		return 0;
	}


	return 1;

}

int CKxQuanxiBiaoCheck::ConnectAnalysis(const kxCImageBuf& SrcImg, int nTestmode)
{
	KxCallStatus hCall;
	return ConnectAnalysis(SrcImg, nTestmode, hCall);
}

int CKxQuanxiBiaoCheck::ConnectAnalysis(const kxCImageBuf& SrcImg, int nTestmode, KxCallStatus& hCall)
{
	int nMergeSize;
	if (SrcImg.nChannel == _Type_G24)
	{
		nMergeSize = 3;
	}
	else
	{
		nMergeSize = 1;
	}

	int nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYDOTS, 8, nMergeSize, 1, hCall);

	if (nStatus == 0)
	{
		return 0;
	}
	int nCount = (std::min)((int)8, m_hBlobAnaly.GetBlobCount());

	m_hResult.m_nCount = nCount;

	if (nCount == 0)
	{
		m_hResult.m_nStatus = _Check_Ok;
		return 1;
	}
	kxRect<int> rcEdge = m_hParameter.m_rcEdgeRect;
	kxRect<int> rcWarp = m_hWarpStrech.GetParameter().m_rcCheckArea;
	for (int i = 0; i < nCount; i++)
	{
		CKxBlobAnalyse::SingleBlobInfo& pObj = m_hBlobAnaly.GetSortSingleBlob(i);

		m_hResult.m_hBlobInfo[i].m_nAreaType = m_hParameter.m_nAreaNumber;
		m_hResult.m_hBlobInfo[i].m_nDots = pObj.m_nDots;
		m_hResult.m_hBlobInfo[i].m_nEnergy = pObj.m_nEnergy / 25;
		kxRect<int> rc;
		kxRect<int> rcDst;
		rc.setup(pObj.m_rc.left * 1, pObj.m_rc.top * 1, pObj.m_rc.right * 1, pObj.m_rc.bottom * 1);
		rc.offset(rcWarp.left, rcWarp.top);
		if (rc.left > rcWarp.left && rc.top > rcWarp.top && rc.bottom < rcWarp.bottom && rc.right < rcWarp.right)
		{
			//m_hWarpStrech.GetRightPos(rc, rcDst);
			rcDst = rc;
			if (nTestmode == 0)
			{
				rcDst.offset(-m_nOffsetX, -m_nOffsetY);
			}

			if (rcDst.left + rcDst.Width() > SrcImg.nWidth - 1 || rcDst.top + rcDst.Height() > SrcImg.nHeight - 1 || rcDst.left < 0 || rcDst.top < 0)
			{
				rcDst = rc;
			}
			//rcDst = rc;
		}
		else //边缘检测不变换坐标
		{
			rcDst = rc;
		}
		//rc.offset(m_hWarpStrech.GetParameter().m_rcCheckArea.left, m_hWarpStrech.GetParameter().m_rcCheckArea.top);

		//rcDst.offset(rcEdge.left, rcEdge.top);

		m_hResult.m_hBlobInfo[i].m_nLeft = rcDst.left;
		m_hResult.m_hBlobInfo[i].m_nTop = rcDst.top;
		m_hResult.m_hBlobInfo[i].m_nBlobWidth = rcDst.Width();
		m_hResult.m_hBlobInfo[i].m_nBlobHeight = rcDst.Height();

		m_hResult.m_hBlobInfo[i].m_nWHRatio = int(pObj.m_fRatio + 0.5);
		m_hResult.m_hBlobInfo[i].m_nMinRectWidth = int(pObj.m_nMinRectWidth);
		m_hResult.m_hBlobInfo[i].m_nMinRectHeight = int(pObj.m_nMinRectHeight);

		m_hResult.m_hBlobInfo[i].m_nAvgEnergy = pObj.m_nEnergy / pObj.m_nDots;
		m_hResult.m_hBlobInfo[i].m_nAvgArea = int(pObj.m_fSumEnergy / 25);  //总能量

		m_hResult.m_hBlobInfo[i].m_nBlobType = 0;  //表面质量检查

	}

	m_hResult.m_nStatus = _Check_Err;

	if (m_hParameter.m_nIsContoursCheck)
	{
		//边缘部分单独分析
		nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgDiff, CKxBlobAnalyse::_SORT_BYDOTS, 8, 3, 1, hCall);
		if (nStatus == 0)
		{
			return 1;
		}
		int nEdgeCount = (std::min)((int)8, m_hBlobAnaly.GetBlobCount());
		
		if (nEdgeCount == 0)
		{
			return 1;
		}

		for (int i = 0; i < nEdgeCount; i++)
		{
			CKxBlobAnalyse::SingleBlobInfo& pObj = m_hBlobAnaly.GetSortSingleBlob(i);

			m_hResult.m_hBlobInfo[nCount + i].m_nAreaType = m_hParameter.m_nAreaNumber;
			m_hResult.m_hBlobInfo[nCount + i].m_nDots = pObj.m_nDots;
			m_hResult.m_hBlobInfo[nCount + i].m_nEnergy = pObj.m_nEnergy / 25;
			kxRect<int> rc;
			kxRect<int> rcDst;
			rc.setup(pObj.m_rc.left * 1, pObj.m_rc.top * 1, pObj.m_rc.right * 1, pObj.m_rc.bottom * 1);
			rc.offset(rcWarp.left, rcWarp.top);
			if (rc.left > rcWarp.left && rc.top > rcWarp.top && rc.bottom < rcWarp.bottom && rc.right < rcWarp.right)
			{
				//m_hWarpStrech.GetRightPos(rc, rcDst);
				rcDst = rc;
				if (nTestmode == 0)
				{
					rcDst.offset(-m_nOffsetX, -m_nOffsetY);
				}

				if (rcDst.left + rcDst.Width() > SrcImg.nWidth - 1 || rcDst.top + rcDst.Height() > SrcImg.nHeight - 1 || rcDst.left < 0 || rcDst.top < 0)
				{
					rcDst = rc;
				}
				//rcDst = rc;
			}
			else //边缘检测不变换坐标
			{
				rcDst = rc;
			}
			//rc.offset(m_hWarpStrech.GetParameter().m_rcCheckArea.left, m_hWarpStrech.GetParameter().m_rcCheckArea.top);

			//rcDst.offset(rcEdge.left, rcEdge.top);

			m_hResult.m_hBlobInfo[nCount + i].m_nLeft = rcDst.left;
			m_hResult.m_hBlobInfo[nCount + i].m_nTop = rcDst.top;
			m_hResult.m_hBlobInfo[nCount + i].m_nBlobWidth = rcDst.Width();
			m_hResult.m_hBlobInfo[nCount + i].m_nBlobHeight = rcDst.Height();

			m_hResult.m_hBlobInfo[nCount + i].m_nWHRatio = int(pObj.m_fRatio + 0.5);
			m_hResult.m_hBlobInfo[nCount + i].m_nMinRectWidth = int(pObj.m_nMinRectWidth);
			m_hResult.m_hBlobInfo[nCount + i].m_nMinRectHeight = int(pObj.m_nMinRectHeight);

			m_hResult.m_hBlobInfo[nCount + i].m_nAvgEnergy = pObj.m_nEnergy / pObj.m_nDots;
			m_hResult.m_hBlobInfo[nCount + i].m_nAvgArea = int(pObj.m_fSumEnergy / 25);  //总能量

			m_hResult.m_hBlobInfo[nCount + i].m_nBlobType = 1;  //边缘质量检查

		}

		m_hResult.m_nCount = nEdgeCount + nCount;
	}


	m_hResult.m_nStatus = _Check_Err;

	return 1;
}

int CKxQuanxiBiaoCheck::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex)
{
	KxCallStatus hCall;
	return SingleDefectLearn(ImgCheck, nBlobIndex, hCall);
}


//单个缺陷学习
int CKxQuanxiBiaoCheck::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_ImgPos, hCallInfo);
	if (check_sts(hCallInfo, "SingleImageLearn_", hCall) || nStatus != 0)
	{
		return 0;
	}


	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;
	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), ImgCheck.nChannel);
	m_hFun.KxCopyImage(ImgCheck, m_WarpImg, rcCopy);

	//CopyEdges(ImgCheck, m_WarpImg, m_ExtendWarpImg1, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);
	m_ImgEdge.Init(m_hParameter.m_rcEdgeRect.Width(), m_hParameter.m_rcEdgeRect.Height(), ImgCheck.nChannel);
	m_hFun.KxCopyImage(ImgCheck, m_ImgEdge, m_hParameter.m_rcEdgeRect);


	//KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, false, hCall);
	//if (check_sts(hCallInfo, "SingleDefectLearn_", hCall))
	//{
	//	return 0;
	//}
	//if (m_hParameter.m_nIsSearchEdge)
	//{
	//	KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, false, hCallInfo);
	//	if (check_sts(hCallInfo, "全息标预处理_", hCall))
	//	{
	//		//SetCheckExceptionStatus(ImgCheck);
	//		return 0;
	//	}
	//}
	//else
	//{
	//	m_ExtendWarpImg.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, m_WarpImg.nChannel);
	//	kxRect<int> rc;
	//	rc.setup(0, 0, m_WarpImg.nWidth - 1, m_WarpImg.nHeight - 1);
	//	m_hFun.KxCopyImage(m_WarpImg, m_ExtendWarpImg, rc);
	//	m_nOffsetX = 0;
	//	m_nOffsetY = 0;
	//}

	if (m_hParameter.m_nIsSearchEdge)
	{
		KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg1, false, hCallInfo);
		if (check_sts(hCallInfo, "全息标预处理_", hCall))
		{
			return 0;
		}
	}
	else
	{
		m_ExtendWarpImg1.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, m_WarpImg.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_WarpImg.nWidth - 1, m_WarpImg.nHeight - 1);
		m_hFun.KxCopyImage(m_WarpImg, m_ExtendWarpImg1, rc);
		m_nOffsetX = 0;
		m_nOffsetY = 0;
	}

	if (m_hParameter.m_nIsOpencheckWhiteDot)
	{
		m_hFun.KxErodeImage(m_ExtendWarpImg1, m_ExtendWarpImg);
	}
	else
	{
		m_ExtendWarpImg.Init(m_ExtendWarpImg1.nWidth, m_ExtendWarpImg1.nHeight, m_ExtendWarpImg1.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_ExtendWarpImg1.nWidth - 1, m_ExtendWarpImg1.nHeight - 1);
		m_hFun.KxCopyImage(m_ExtendWarpImg1, m_ExtendWarpImg, rc);
	}



	ProcessSurfaceWarpImages(hCallInfo);
	if (check_sts(hCallInfo, "SingleDefectLearn_", hCall))
	{
		return 0;
	}

	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "SingleDefectLearn_", hCall))
	{
		return 0;
	}

	int nMergeSize;
	if (ImgCheck.nChannel == _Type_G24)
	{
		nMergeSize = 1;
	}
	else
	{
		nMergeSize = 1;
	}


	nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYDOTS, _Max_BlobCount, nMergeSize, 1);
	if (nStatus == 0)
	{
		return 0;
	}
	int nCount = (std::min)((int)_Max_BlobCount, m_hBlobAnaly.GetBlobCount());

	if (nCount < nBlobIndex + 1)
	{
		return 0;
	}
	kxRect<int>  rcSmall, rcBig;
	CKxBlobAnalyse::SingleBlobInfo& pObj = m_hBlobAnaly.GetSortSingleBlob(nBlobIndex);
	rcSmall = pObj.m_rc;
	rcBig.setup(pObj.m_rc.left * 1, pObj.m_rc.top * 1, pObj.m_rc.right *1, pObj.m_rc.bottom * 1);


	//SaveMidImage(true);

	kxRect<int> rcLearn[_Max_Layer];
	kxRect<int> rcTemp;
	for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			//坐标由于压缩的缘故有+-1的误差
			rcTemp = rcBig;
			rcTemp.left = gMax(rcTemp.left - 4, 0);
			rcTemp.top = gMax(rcTemp.top - 4, 0);
			rcTemp.right = gMin(rcTemp.right + 4, m_ImgCheck[i].nWidth - 1);
			rcTemp.bottom = gMin(rcTemp.bottom + 4, m_ImgCheck[i].nHeight - 1);
			rcLearn[i] = rcTemp;
		}

	}


	for (int i = _SAMLLDOTS_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			rcTemp = rcSmall;
			//此处扩大学习范围是由于mask边缘学不进去，强行缩进
			rcTemp.left = gMax(rcTemp.left - 20, 0);
			rcTemp.top = gMax(rcTemp.top - 20, 0);
			rcTemp.right = gMin(rcTemp.right + 20, m_ImgCheck[i].nWidth - 1);
			rcTemp.bottom = gMin(rcTemp.bottom + 20, m_ImgCheck[i].nHeight - 1);
			rcLearn[i] = rcTemp;
		}
	}

	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			rcTemp = rcBig;
			//坐标由于压缩的缘故有+-1的误差
			rcTemp.left = gMax(rcTemp.left - 4, 0);
			rcTemp.top = gMax(rcTemp.top - 4, 0);
			rcTemp.right = gMin(rcTemp.right + 4, m_ImgCheck[i].nWidth - 1);
			rcTemp.bottom = gMin(rcTemp.bottom + 4, m_ImgCheck[i].nHeight - 1);
			rcLearn[i] = rcTemp;
		}
	}
	rcLearn[_Max_Layer - 1] = rcSmall;


	//学多
	for (int i = 0; i <= _SCRATCH_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			IppiSize roiSize = { rcLearn[i].Width(), rcLearn[i].Height() };
			if (m_ImgCheck[i].nChannel == _Type_G24)
			{
				ippiMaxEvery_8u_C3IR(m_ImgCheck[i].buf + rcLearn[i].top*m_ImgCheck[i].nPitch + rcLearn[i].left * 3, m_ImgCheck[i].nPitch,
					m_ImgLearnTemplate[i].buf + rcLearn[i].top*m_ImgLearnTemplate[i].nPitch + rcLearn[i].left * 3, m_ImgLearnTemplate[i].nPitch, roiSize);
			}
			else
			{
				ippiMaxEvery_8u_C1IR(m_ImgCheck[i].buf + rcLearn[i].top*m_ImgCheck[i].nPitch + rcLearn[i].left, m_ImgCheck[i].nPitch,
					m_ImgLearnTemplate[i].buf + rcLearn[i].top*m_ImgLearnTemplate[i].nPitch + rcLearn[i].left, m_ImgLearnTemplate[i].nPitch, roiSize);
			}
		}
	}
	//学少
	for (int i = _GRAY_INDEX_END - 1; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			IppiSize roiSize = { rcLearn[i].Width(), rcLearn[i].Height() };
			if (m_ImgCheck[i].nChannel == _Type_G24)
			{
				ippiMinEvery_8u_C3IR(m_ImgCheck[i].buf + rcLearn[i].top*m_ImgCheck[i].nPitch + rcLearn[i].left * 3, m_ImgCheck[i].nPitch,
					m_ImgLearnTemplate[i].buf + rcLearn[i].top*m_ImgLearnTemplate[i].nPitch + rcLearn[i].left * 3, m_ImgLearnTemplate[i].nPitch, roiSize);
			}
			else
			{
				ippiMinEvery_8u_C1IR(m_ImgCheck[i].buf + rcLearn[i].top*m_ImgCheck[i].nPitch + rcLearn[i].left, m_ImgCheck[i].nPitch,
					m_ImgLearnTemplate[i].buf + rcLearn[i].top*m_ImgLearnTemplate[i].nPitch + rcLearn[i].left, m_ImgLearnTemplate[i].nPitch, roiSize);
			}
		}
	}


	m_nModelNumber = _Max_Layer + 1;
	m_ImgLearnTemplate[_Max_Layer].Init(m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nWidth, m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nHeight, m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nChannel);
	ippsSet_8u(0, m_ImgLearnTemplate[_Max_Layer].buf, m_ImgLearnTemplate[_Max_Layer].nPitch * m_ImgLearnTemplate[_Max_Layer].nHeight);
	for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hFun.KxMaxEvery(m_ImgLearnTemplate[i], m_ImgLearnTemplate[_Max_Layer]);
		}
	}
	for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hFun.KxMaxEvery(m_ImgLearnTemplate[i], m_ImgLearnTemplate[_Max_Layer]);
		}
	}

	return 1;
}

int CKxQuanxiBiaoCheck::SingleImageLearn(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return SingleImageLearn(ImgCheck, hCall);
}

//单张学习
int CKxQuanxiBiaoCheck::SingleImageLearn(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_ImgPos, hCallInfo);
	if (check_sts(hCallInfo, "SingleImageLearn_", hCall) || nStatus != 0)
	{
		return 0;
	}


	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;

	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), ImgCheck.nChannel);
	m_hFun.KxCopyImage(ImgCheck, m_WarpImg, rcCopy);

	//CopyEdges(ImgCheck, m_WarpImg, m_ExtendWarpImg1, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);
	m_ImgEdge.Init(m_hParameter.m_rcEdgeRect.Width(), m_hParameter.m_rcEdgeRect.Height(), ImgCheck.nChannel);
	m_hFun.KxCopyImage(ImgCheck, m_ImgEdge, m_hParameter.m_rcEdgeRect);

	//KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, false, hCall);
	//if (check_sts(hCallInfo, "SingleImageLearn_", hCall))
	//{
	//	return 0;
	//}

	//if (m_hParameter.m_nIsSearchEdge)
	//{
	//	KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, false, hCallInfo);
	//	if (check_sts(hCallInfo, "全息标预处理_", hCall))
	//	{
	//		//SetCheckExceptionStatus(ImgCheck);
	//		return 0;
	//	}
	//}
	//else
	//{
	//	m_ExtendWarpImg.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, m_WarpImg.nChannel);
	//	kxRect<int> rc;
	//	rc.setup(0, 0, m_WarpImg.nWidth - 1, m_WarpImg.nHeight - 1);
	//	m_hFun.KxCopyImage(m_WarpImg, m_ExtendWarpImg, rc);
	//	m_nOffsetX = 0;
	//	m_nOffsetY = 0;
	//}


	if (m_hParameter.m_nIsSearchEdge)
	{
		KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg1, false, hCallInfo);
		if (check_sts(hCallInfo, "全息标预处理_", hCall))
		{
			return 0;
		}
	}
	else
	{
		m_ExtendWarpImg1.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, m_WarpImg.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_WarpImg.nWidth - 1, m_WarpImg.nHeight - 1);
		m_hFun.KxCopyImage(m_WarpImg, m_ExtendWarpImg1, rc);
		m_nOffsetX = 0;
		m_nOffsetY = 0;
	}

	if (m_hParameter.m_nIsOpencheckWhiteDot)
	{
		m_hFun.KxErodeImage(m_ExtendWarpImg1, m_ExtendWarpImg);
	}
	else
	{
		m_ExtendWarpImg.Init(m_ExtendWarpImg1.nWidth, m_ExtendWarpImg1.nHeight, m_ExtendWarpImg1.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_ExtendWarpImg1.nWidth - 1, m_ExtendWarpImg1.nHeight - 1);
		m_hFun.KxCopyImage(m_ExtendWarpImg1, m_ExtendWarpImg, rc);
	}



	ProcessSurfaceWarpImages(hCallInfo);
	if (check_sts(hCallInfo, "SingleImageLearn_", hCall))
	{
		return 0;
	}


	//学多
	for (int i = 0; i <= _SCRATCH_INDEX_END; i++)
	{
		//if (m_nAlgorithmStatus[i])
		{
			IppiSize roiSize = { m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight };
			if (m_ImgCheck[i].nChannel == _Type_G24)
			{
				ippiMaxEvery_8u_C3IR(m_ImgCheck[i].buf , m_ImgCheck[i].nPitch, m_ImgLearnTemplate[i].buf, m_ImgLearnTemplate[i].nPitch, roiSize);
			}
			else
			{
				ippiMaxEvery_8u_C1IR(m_ImgCheck[i].buf, m_ImgCheck[i].nPitch,	m_ImgLearnTemplate[i].buf , m_ImgLearnTemplate[i].nPitch, roiSize);
			}
		}
	}
	//学少
	for (int i = _GRAY_INDEX_END - 1; i < _Max_Layer; i++)
	{
		//if (m_nAlgorithmStatus[i])
		{
			IppiSize roiSize = { m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight };
			if (m_ImgCheck[i].nChannel == _Type_G24)
			{
				ippiMinEvery_8u_C3IR(m_ImgCheck[i].buf, m_ImgCheck[i].nPitch,
					m_ImgLearnTemplate[i].buf , m_ImgLearnTemplate[i].nPitch, roiSize);
			}
			else
			{
				ippiMinEvery_8u_C1IR(m_ImgCheck[i].buf , m_ImgCheck[i].nPitch,
					m_ImgLearnTemplate[i].buf, m_ImgLearnTemplate[i].nPitch, roiSize);
			}
		}
	}


	m_nModelNumber = _Max_Layer + 1;
	m_ImgLearnTemplate[_Max_Layer].Init(m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nWidth, m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nHeight, m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nChannel);
	ippsSet_8u(0, m_ImgLearnTemplate[_Max_Layer].buf, m_ImgLearnTemplate[_Max_Layer].nPitch * m_ImgLearnTemplate[_Max_Layer].nHeight);
	for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	{
		//if (m_nAlgorithmStatus[i])
		{
			m_hFun.KxMaxEvery(m_ImgLearnTemplate[i], m_ImgLearnTemplate[_Max_Layer]);
		}
	}
	for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
	{
		//if (m_nAlgorithmStatus[i])
		{
			m_hFun.KxMaxEvery(m_ImgLearnTemplate[i], m_ImgLearnTemplate[_Max_Layer]);
		}
	}

	return 1;
}

int CKxQuanxiBiaoCheck::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6])
{
	KxCallStatus hCall;
	return ShowResidualsImage(SrcImg, DstImg, nOffset, hCall);
}

//待检测图像的残差图显示
//残差图始终是一张彩色图像,
//蓝色表示检小点残差、绿色表示检细线残差、红色表示检划伤残差、黑色表示检缺失残差、白色表示
int CKxQuanxiBiaoCheck::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6], KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCall.Clear();
	int nCompressSize = 1;

	m_hParameter.m_nSmallDotOffset = nOffset[0];
	m_hParameter.m_nLineOffset = nOffset[1];
	m_hParameter.m_nScratchOffset = nOffset[2];
	m_hParameter.m_nLossOffset = nOffset[4];
	m_hParameter.m_nGrayOffset = nOffset[5];

	AnalysisCheckStatus(SrcImg.nChannel);

	int nStatus = m_hWarpStrech.CheckParallel(SrcImg, m_ImgPos, hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage_", hCall) || nStatus != 0)
	{
		return 0;
	}


	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;

	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), SrcImg.nChannel);
	m_hFun.KxCopyImage(SrcImg, m_WarpImg, rcCopy);


	//CopyEdges(SrcImg, m_WarpImg, m_ExtendWarpImg1, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);
	m_ImgEdge.Init(m_hParameter.m_rcEdgeRect.Width(), m_hParameter.m_rcEdgeRect.Height(), SrcImg.nChannel);
	m_hFun.KxCopyImage(SrcImg, m_ImgEdge, m_hParameter.m_rcEdgeRect);

	//KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, false, hCallInfo);
	//if (check_sts(hCallInfo, "ShowResidualsImage__", hCall))
	//{
	//	return 0;
	//}

	//if (m_hParameter.m_nIsSearchEdge)
	//{
	//	KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg, false, hCallInfo);
	//	if (check_sts(hCallInfo, "全息标预处理_", hCall))
	//	{
	//		//SetCheckExceptionStatus(ImgCheck);
	//		return 0;
	//	}
	//}
	//else
	//{
	//	m_ExtendWarpImg.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, m_WarpImg.nChannel);
	//	kxRect<int> rc;
	//	rc.setup(0, 0, m_WarpImg.nWidth - 1, m_WarpImg.nHeight - 1);
	//	m_hFun.KxCopyImage(m_WarpImg, m_ExtendWarpImg, rc);
	//	m_nOffsetX = 0;
	//	m_nOffsetY = 0;
	//}


	if (m_hParameter.m_nIsSearchEdge)
	{
		KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg1, false, hCallInfo);
		if (check_sts(hCallInfo, "全息标预处理_", hCall))
		{
			return 0;
		}
	}
	else
	{
		m_ExtendWarpImg1.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, m_WarpImg.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_WarpImg.nWidth - 1, m_WarpImg.nHeight - 1);
		m_hFun.KxCopyImage(m_WarpImg, m_ExtendWarpImg1, rc);
		m_nOffsetX = 0;
		m_nOffsetY = 0;
	}

	if (m_hParameter.m_nIsOpencheckWhiteDot)
	{
		m_hFun.KxErodeImage(m_ExtendWarpImg1, m_ExtendWarpImg);
	}
	else
	{
		m_ExtendWarpImg.Init(m_ExtendWarpImg1.nWidth, m_ExtendWarpImg1.nHeight, m_ExtendWarpImg1.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_ExtendWarpImg1.nWidth - 1, m_ExtendWarpImg1.nHeight - 1);
		m_hFun.KxCopyImage(m_ExtendWarpImg1, m_ExtendWarpImg, rc);
	}




	ProcessSurfaceWarpImages(hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage_", hCall))
	{
		return 0;
	}

	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage_", hCall))
	{
		return 0;
	}

	//SaveMidImage(true);

	//Blob 分析
	ConnectAnalysis(SrcImg, 1, hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage_", hCall))
	{
		return 0;
	}

	//原图是黑白图，先转化为彩色图像
	IppiSize roiSize = { m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight };
	m_ColorImg.Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight, 3);
	if (_Type_G8 == m_ExtendWarpImg.nChannel)
	{
		ippiDup_8u_C1C3R(m_ExtendWarpImg.buf, m_ExtendWarpImg.nPitch, m_ColorImg.buf, m_ColorImg.nPitch, roiSize);
	}
	else
	{
		ippiCopy_8u_C3R(m_ExtendWarpImg.buf, m_ExtendWarpImg.nPitch, m_ColorImg.buf, m_ColorImg.nPitch, roiSize);
	}
	//彩色图压缩4倍
	m_ResizeImg.Init(m_ColorImg.nWidth / 1, m_ColorImg.nHeight /1, m_ColorImg.nChannel);
	m_hFun.KxResizeImage(m_ColorImg, m_ResizeImg, KxSuper);

	SetMask(m_ResizeImg, m_ImgTemp, 1, 255);
	//首先获得检小点的残差图
	for (int i = 0; i <= 3; i++)
	{
		m_ResidImg[i].Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight);
		ippsSet_8u(0, m_ResidImg[i].buf, m_ResidImg[i].nPitch*m_ResidImg[i].nHeight);
	}

	for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			//ConvertSliderCc(m_ImgSlidCcMax[i], m_ImgCcTemp, i);
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ResImg[i], m_ResidImg[0]);
		}
	}
	//获得检细线的残差图
	for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ResImg[i], m_ResidImg[1]);
		}
	}
	//获得检划伤的残差图
	for (int i = _LINE_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ResImg[i], m_ResidImg[2]);
		}
	}

	//检缺残差图
	for (int i = _SCRATCH_INDEX_END; i < _LOSS_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ResImg[i], m_ResidImg[3]);
		}
	}

	//合成三张残差图
	m_AddImage.Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight, m_ResizeImg.nChannel);
	ippsSet_8u(0, m_AddImage.buf, m_AddImage.nPitch*m_AddImage.nHeight);

	for (int i = 0; i < 3; i++)
	{
		m_hImgMask.Mask(m_ResidImg[i], m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, 0);
		m_hPolygonMask.Mask(m_ResidImg[i], m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, 0);
		m_CompositionImg[i].Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight, m_ResizeImg.nChannel);
		AddResidualImage(m_ResidImg[i], m_ResizeImg, i, m_CompositionImg[i]);
		m_hFun.KxAddImage(m_CompositionImg[i], m_AddImage);

		//char sz[128];
		//sprintf_s(sz, 128, "d:\\Save\\Show\\m_CompositionImg_%d.bmp", i);
		//m_hBaseFun[0].SaveBMPImage_h(sz, m_CompositionImg[i]);
		//
		//sprintf_s(sz, 128, "d:\\Save\\Show\\m_ResidImg_%d.bmp", i);
		//m_hBaseFun[0].SaveBMPImage_h(sz, m_ResidImg[i]);

		//sprintf_s(sz, 128, "d:\\Save\\Show\\Resize_%d.bmp", i);
		//m_hBaseFun[0].SaveBMPImage_h(sz, m_ResidImg[i]);

	}

	//保留残差处
	SetMask(m_AddImage, m_ImgTemp, 0, 0);
	//保留背景处
	SetMask(m_CompositionImg[2], m_ImgTemp, 1, 255);
	m_hFun.KxAddImage(m_CompositionImg[2], m_AddImage);

	//m_hBaseFun[0].SaveBMPImage_h("d:\\Save\\Show\\Defect.bmp", m_AddImage);
	//m_hBaseFun[0].SaveBMPImage_h("d:\\Save\\Show\\Blackgroud.bmp", m_CompositionImg[2]);
	//m_hBaseFun[0].SaveBMPImage_h("d:\\Save\\Show\\hecheng.bmp", m_AddImage);
	//m_hBaseFun[0].SaveBMPImage_h("d:\\Save\\Show\\m_ImgSlidCcMax.bmp", m_ImgSlidCcMax[_Max_Layer - 1]);

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
	m_hFun.KxResizeImage(m_ResizeImg1, DstImg);


	IppiSize Roi = { m_AddImage.nWidth, m_AddImage.nHeight };

	if (m_nAlgorithmStatus[_Max_Layer - 1])
	{
		m_hImgMask.Mask(m_ResidImg[3], m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, 0);
		m_hPolygonMask.Mask(m_ResidImg[3], m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, 0);

		SetMask(m_AddImage, m_ResidImg[3], 1, 255);
	}



	kxRect<int> rcDst;
	//m_hWarpStrech.GetRightPos(m_hParameter.m_rcEdgeRect, rcDst);
	//rcDst = m_hParameter.m_rcEdgeRect;
	rcDst = m_hWarpStrech.GetParameter().m_rcCheckArea;
	//rcDst.offset(-m_nOffsetX, -m_nOffsetY);

	if (rcDst.left + rcDst.Width() > DstImg.nWidth - 1 || rcDst.top + rcDst.Height() > DstImg.nHeight - 1 || rcDst.left < 0 || rcDst.top < 0)
	{
		rcDst = m_hParameter.m_rcEdgeRect;
	}

	Roi.width = (std::min)(DstImg.nWidth - rcDst.left / 1 - 1, Roi.width);
	Roi.height = (std::min)(DstImg.nHeight - rcDst.top / 1 - 1, Roi.height);


	ippiCopy_8u_C3R(m_AddImage.buf, m_AddImage.nPitch,
		DstImg.buf + rcDst.top / 1 * DstImg.nPitch + rcDst.left / 1 * 3, DstImg.nPitch, Roi);




	return 1;
}

int CKxQuanxiBiaoCheck::SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate)
{
	kxCImageBuf  BinaryImg, ColorImg;
	BinaryImg.Init(MaskImg.nWidth, MaskImg.nHeight);
	m_hFun.KxThreshImage(MaskImg, BinaryImg, lowGate, highGate);
	ColorImg.Init(SrcDstImg.nWidth, SrcDstImg.nHeight, SrcDstImg.nChannel);
	IppiSize roiSize = { SrcDstImg.nWidth, SrcDstImg.nHeight };
	ippiDup_8u_C1C3R(BinaryImg.buf, BinaryImg.nPitch, ColorImg.buf, ColorImg.nPitch, roiSize);
	ippiSub_8u_C3IRSfs(ColorImg.buf, ColorImg.nPitch, SrcDstImg.buf, SrcDstImg.nPitch, roiSize, 0);
	return 1;
}

//把一张残差图贴到原图像上，高亮显示对应的颜色
//SrcImg 为彩色图像， CcImg 为黑白图像， nColorMode 为颜色模式
int CKxQuanxiBiaoCheck::AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg)
{
	kxCImageBuf  BinaryImg, ColorImg, SubImg, AddImg;
	BinaryImg.Init(CcImg.nWidth, CcImg.nHeight);
	m_hFun.KxThreshImage(CcImg, BinaryImg, 1, 255);
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

//确保 rcEdge 包含 rcCopy
int CKxQuanxiBiaoCheck::CopyEdges(const kxCImageBuf& SrcImg, const kxCImageBuf CopyImg, kxCImageBuf& DstImg, kxRect<int>& rcEdge, kxRect<int>& rcCopy)
{


	rcEdge.left = rcEdge.left < rcCopy.left ? rcEdge.left : rcCopy.left;
	rcEdge.top = rcEdge.top < rcCopy.top ? rcEdge.top : rcCopy.top;
	rcEdge.right = rcEdge.right > rcCopy.right ? rcEdge.right : rcCopy.right;
	rcEdge.bottom = rcEdge.bottom > rcCopy.bottom ? rcEdge.bottom : rcCopy.bottom;

	rcEdge.right = rcEdge.right > SrcImg.nWidth - 1 ? SrcImg.nWidth - 1 : rcEdge.right;
	rcEdge.bottom = rcEdge.bottom > SrcImg.nHeight - 1 ? SrcImg.nHeight - 1 : rcEdge.bottom;


	DstImg.Init(rcEdge.Width(), rcEdge.Height(), SrcImg.nChannel);
	m_hBaseFun[0].KxCopyImage(SrcImg, DstImg, rcEdge);


	kxRect<int> rc;
	rc.left = rcCopy.left - rcEdge.left;
	rc.top = rcCopy.top - rcEdge.top;
	rc.right = rc.left + rcCopy.Width() - 1;
	rc.bottom = rc.top + rcCopy.Height() - 1;
	m_hBaseFun[0].KxCopyImage(CopyImg, DstImg, rc);




	return 1;
}

int CKxQuanxiBiaoCheck::AnalysisCheckStatus(int nImageType)
{
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_nAlgorithmStatus[i] = 0;
	}
	if (m_hParameter.m_nSmallDotOffset < _MAX_OFFSET)
	{
		for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
		{
			m_nAlgorithmStatus[i] = 1;
		}
		m_hParameter.m_nSmallDotOffset *= 2;
	}
	if (m_hParameter.m_nLineOffset < _MAX_OFFSET)
	{
		for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
		{
			m_nAlgorithmStatus[i] = 1;
		}
		if (_Type_G8 == nImageType)
		{
			m_hParameter.m_nLineOffset *= 3;
		}
		else
		{
			m_hParameter.m_nLineOffset *= 1;
		}

	}
	if (m_hParameter.m_nScratchOffset < _MAX_OFFSET)
	{
		for (int i = _LINE_INDEX_END; i < _SCRATCH_INDEX_END; i++)
		{
			m_nAlgorithmStatus[i] = 1;
		}
		m_hParameter.m_nScratchOffset *= 4;
	}

	if (m_hParameter.m_nGrayOffset < _MAX_OFFSET)
	{

		m_nAlgorithmStatus[_SCRATCH_INDEX_END] = 1;
		m_nHighOffset = m_hParameter.m_nGrayOffset * 4;

	}

	//整体检缺
	if (m_hParameter.m_nLossOffset < _MAX_OFFSET)
	{
		m_nAlgorithmStatus[_SCRATCH_INDEX_END + 1] = 1;
		m_nLowOffset = m_hParameter.m_nLossOffset * 4;
	}


	if (m_hParameter.m_nIsLearn)
	{
		m_nLearnStatus = 1;
	}

	return 1;
}

int CKxQuanxiBiaoCheck::Check(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return Check(ImgCheck, hCall);
}

int CKxQuanxiBiaoCheck::Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	hCall.Clear();
	tick_count tbb_start, tbb_end;
	m_hResult.Clear();
	//task_scheduler_init init;
	//firstly, accurate correction a image
	tbb_start = tick_count::now();

	//if (m_hCalDistance.CheckSpecialWithSearchEdge(ImgCheck, hCall))
	//{
	//	m_hCalDistance.GetHorVerDistance(m_hResult.m_nDistanceX, m_hResult.m_nDistanceY);
	//	m_hResult.m_nStatus = _Distance_Err;
	//	return 0;
	//}

	int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_ImgPos, hCall);
	if (nStatus)
	{
		m_hResult.m_nStatus = _Similarity_Err;
		m_hResult.m_nKernIndex = nStatus - 1;
		return 0;
	}

	kxRect<int> rcCopy;
	rcCopy = m_hWarpStrech.GetParameter().m_rcCheckArea;

	m_WarpImg.Init(rcCopy.Width(), rcCopy.Height(), ImgCheck.nChannel);
	m_hFun.KxCopyImage(ImgCheck, m_WarpImg, rcCopy);

	m_hParameter.m_rcEdgeRect.left = (std::max)(m_hParameter.m_rcEdgeRect.left, 0);
	m_hParameter.m_rcEdgeRect.top = (std::max)(m_hParameter.m_rcEdgeRect.top, 0);
	m_hParameter.m_rcEdgeRect.right = (std::min)(m_hParameter.m_rcEdgeRect.right, m_BaseImg.nWidth - 1);
	m_hParameter.m_rcEdgeRect.bottom = (std::min)(m_hParameter.m_rcEdgeRect.bottom, m_BaseImg.nHeight - 1);

	m_ImgEdge.Init(m_hParameter.m_rcEdgeRect.Width(), m_hParameter.m_rcEdgeRect.Height(), ImgCheck.nChannel);
	m_hFun.KxCopyImage(ImgCheck, m_ImgEdge, m_hParameter.m_rcEdgeRect);

	//CopyEdges(ImgCheck, m_WarpImg, m_ExtendWarpImg1, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);

	tbb_end = tick_count::now();
	//printf("-----warp process cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);

	if (m_hParameter.m_nIsSearchEdge)
	{
		KxQuanxiBiaoPre(m_WarpImg, m_ImgEdge, m_ExtendWarpImg1, false, hCallInfo);
		if (check_sts(hCallInfo, "全息标预处理_", hCall))
		{
			SetCheckExceptionStatus(ImgCheck);
			return 0;
		}
	}
	else
	{
		m_ExtendWarpImg1.Init(m_WarpImg.nWidth, m_WarpImg.nHeight, m_WarpImg.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_WarpImg.nWidth - 1, m_WarpImg.nHeight - 1);
		m_hFun.KxCopyImage(m_WarpImg, m_ExtendWarpImg1, rc);
		m_nOffsetX = 0;
		m_nOffsetY = 0;
	}

	if (m_hParameter.m_nIsOpencheckWhiteDot)
	{
		m_hFun.KxErodeImage(m_ExtendWarpImg1, m_ExtendWarpImg);
	}
	else
	{
		m_ExtendWarpImg.Init(m_ExtendWarpImg1.nWidth, m_ExtendWarpImg1.nHeight, m_ExtendWarpImg1.nChannel);
		kxRect<int> rc;
		rc.setup(0, 0, m_ExtendWarpImg1.nWidth - 1, m_ExtendWarpImg1.nHeight - 1);
		m_hFun.KxCopyImage(m_ExtendWarpImg1, m_ExtendWarpImg, rc);
	}



	if (_Type_G24 == m_WarpImg.nChannel)
	{
		m_ColorCheckImg.Init(ImgCheck.nWidth, ImgCheck.nHeight, ImgCheck.nChannel);
		ippsSet_8u(0, m_ColorCheckImg.buf, m_ColorCheckImg.nPitch*m_ColorCheckImg.nHeight);
		m_hFun.KxCopyImage(m_WarpImg, m_ColorCheckImg, m_hWarpStrech.GetParameter().m_rcCheckArea);
		if (m_hColorDiff.Check(m_ColorCheckImg))
		{
			m_hResult.m_nStatus = _ColorDiff_Err;
			m_hColorDiff.GetColorDiff(m_hResult.m_nColorDiff);
			return 0;
		}
	}


	//secondly, process a image with some methods
	tbb_start = tick_count::now();

	ProcessSurfaceWarpImages(hCallInfo);
	if (check_sts(hCallInfo, "处理中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	tbb_end = tick_count::now();
	//printf(" -----process cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);


	//Thirdly, learn a image
	tbb_start = tick_count::now();

	LearnTemplate(hCallInfo);
	if (check_sts(hCallInfo, "学习中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	tbb_end = tick_count::now();
	//printf("-----learn cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);


	//Fourthly,compute a sub image
	tbb_start = tick_count::now();

	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "比较中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	tbb_end = tick_count::now();
	//printf(" -----slidercc cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);


	//fifth,blob Analysis
	tbb_start = tick_count::now();

	ConnectAnalysis(ImgCheck, 0, hCallInfo);
	if (check_sts(hCallInfo, "分析中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}


	tbb_end = tick_count::now();
	//printf(" -----blob cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);
	//SaveMidImage(true);

	return 1;

}


//全息标预处理,提取全息标图案，贴到灰度背景为128的图中
int CKxQuanxiBiaoCheck::KxQuanxiBiaoPre(const kxCImageBuf& SrcImg, const kxCImageBuf& EdgeImg, kxCImageBuf& DstImg, bool bModel, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	IppStatus status;
	//函数入口是一张彩色图像
	if (SrcImg.nChannel != _Type_G24)
	{
		status = IppStatus(kxImageTypeErr);
		if (check_sts(status, "KxQuanxiBiaoPre_Image_TypeErr", hCall))
		{
			return 0;
		}
	}

	//需要的图像中间变量


	//1、彩色图转灰度图,采用简单的rgb2gray算法
	m_ImgGray.Init(SrcImg.nWidth, SrcImg.nHeight);
	m_ImgGray1.Init(EdgeImg.nWidth, EdgeImg.nHeight);
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };
	if (m_nSegmentLayer == 0)//RGB_GRAY
	{
		m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, RGB_GRAY);
		m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, RGB_GRAY);
	}
	else if (m_nSegmentLayer == 1)
	{
		m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, RGB_R);
		m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, RGB_R);
	}
	else if (m_nSegmentLayer == 2)
	{
		m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, RGB_G);
		m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, RGB_G);
	}
	else if (m_nSegmentLayer == 3)
	{
		m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, RGB_B);
		m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, RGB_B);
	}
	else if (m_nSegmentLayer == 4)
	{
		m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, HSV_H);
		m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, HSV_H);
	}
	else if (m_nSegmentLayer == 5)
	{
		m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, HSV_S);
		m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, HSV_S);
	}
	else if (m_nSegmentLayer == 6)
	{
		//m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, HSV_V);
		//m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, HSV_V);
		m_hFun.KxMedianFilterImage(SrcImg, m_ImgMedian, 3, 3);
		m_hFun.KxConvertImageLayer(m_ImgMedian, m_ImgB, RGB_B);
		m_hFun.KxConvertImageLayer(m_ImgMedian, m_ImgR, RGB_R);
		ippiAbsDiff_8u_C1R(m_ImgR.buf, m_ImgR.nPitch, m_ImgB.buf, m_ImgB.nPitch, m_ImgGray.buf, m_ImgGray.nPitch, roiSize);

		m_hFun.KxMedianFilterImage(EdgeImg, m_ImgMedianEdge, 3, 3);
		m_hFun.KxConvertImageLayer(m_ImgMedianEdge, m_ImgBEdge, RGB_B);
		m_hFun.KxConvertImageLayer(m_ImgMedianEdge, m_ImgREdge, RGB_R);
		IppiSize roi = { EdgeImg.nWidth, EdgeImg.nHeight };
		ippiAbsDiff_8u_C1R(m_ImgREdge.buf, m_ImgREdge.nPitch, m_ImgBEdge.buf, m_ImgBEdge.nPitch, m_ImgGray1.buf, m_ImgGray1.nPitch, roi);


	}
	else
	{
		m_hFun.KxConvertImageLayer(SrcImg, m_ImgGray, RGB_GRAY);
		m_hFun.KxConvertImageLayer(EdgeImg, m_ImgGray1, RGB_GRAY);
	}

	//char sz[128];
	//sprintf_s(sz, 128, "D:\\m_ImgGray.bmp");
	//m_hFun.SaveBMPImage_h(sz, m_ImgGray);


	//2、将灰度图像利用大井法二值化
	Ipp8u Gate;
	m_ImgBinary.Init(SrcImg.nWidth, SrcImg.nHeight);
	//ippiComputeThreshold_Otsu_8u_C1R(m_ImgGray.buf, m_ImgGray.nPitch, roiSize, &Gate);
	Gate = Ipp8u(m_nSegmentThresh);
	//Gate = max(10, Gate - 10);
	ippiCompareC_8u_C1R(m_ImgGray.buf, m_ImgGray.nPitch, Gate, m_ImgBinary.buf, m_ImgBinary.nPitch, roiSize, ippCmpGreaterEq);

	//char sz[128];
	//sprintf_s(sz, 128, "D:\\2_Mask.bmp");
	//m_hFun.SaveBMPImage_h(sz, m_ImgBinary);


	m_hFun.KxCloseImage(m_ImgBinary, m_ImgClose, 1, 1);
	//3、填充空洞
	m_hFun.KxFillHoles(m_ImgClose, m_ImgFillHoles, hCallInfo);


	//做一次Blob分析，用于判断重贴全息标
	m_ImgBinary1.Init(EdgeImg.nWidth, EdgeImg.nHeight);
	IppiSize roi = { EdgeImg.nWidth, EdgeImg.nHeight };
	ippiCompareC_8u_C1R(m_ImgGray1.buf, m_ImgGray1.nPitch, Gate, m_ImgBinary1.buf, m_ImgBinary1.nPitch, roi, ippCmpGreaterEq);
	m_hFun.KxCloseImage(m_ImgBinary1, m_ImgClose1, 5, 5);
	m_hFun.KxFillHoles(m_ImgClose1, m_ImgFillHoles1, hCallInfo);
	m_hSelectBlobAnalyse.ToBlobParallel(m_ImgFillHoles1, 0, 4, 1, 0, hCallInfo);
	if (check_sts(hCallInfo, "KxQuanxiBiaoPre_", hCall))
	{
		return 0;
	}
	int nCount = m_hSelectBlobAnalyse.GetBlobCount();

	if (nCount == 0)
	{
		status = IppStatus(10000);
		if (check_sts(status, "没有检测出全息标", hCall))
		{
			return 0;
		}
	}
	if (nCount >= 2)
	{
		int nCount1 = m_hSelectBlobAnalyse.GetSortSingleBlob(0).m_nDots;
		int nCount2 = m_hSelectBlobAnalyse.GetSortSingleBlob(1).m_nDots;
		if (nCount2 > nCount1/2)
		{
			status = IppStatus(10000);
			if (check_sts(status, "检测出多个全息标", hCall))
			{
				return 0;
			}
		}
		int nH1 = m_hSelectBlobAnalyse.GetSortSingleBlob(0).m_rc.Height();
		int nH2 = m_hSelectBlobAnalyse.GetSortSingleBlob(1).m_rc.Height();
		if (nH2 > nH1*2 / 3 && nCount2 > 100)
		{
			status = IppStatus(10000);
			if (check_sts(status, "检测出多个全息标", hCall))
			{
				return 0;
			}
		}
	}

	if (nCount == 1)
	{
		int nW = m_hSelectBlobAnalyse.GetSortSingleBlob(0).m_rc.Width();
		int nH = m_hSelectBlobAnalyse.GetSortSingleBlob(0).m_rc.Height();
		if (nW*nH == m_ImgFillHoles1.nWidth*m_ImgFillHoles1.nHeight)
		{
			status = IppStatus(10000);
			if (check_sts(status, "分割阈值过低", hCall))
			{
				return 0;
			}
		}
	}


	//4、挑选最大连通域
	//CKxBlobAnalyse m_hSelectBlobAnalyse;
	m_hSelectBlobAnalyse.SelectMaxRegionByDots(m_ImgFillHoles, m_ImgBigArea, hCallInfo);
	if (check_sts(hCallInfo, "KxQuanxiBiaoPre_", hCall))
	{
		return 0;
	}


	//5、去掉背景，留下前景
	ippiDivC_8u_C1IRSfs(0xff, m_ImgBigArea.buf, m_ImgBigArea.nPitch, roiSize, 0);
	m_ImgMask.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	ippiDup_8u_C1C3R(m_ImgBigArea.buf, m_ImgBigArea.nPitch, m_ImgMask.buf, m_ImgMask.nPitch, roiSize);
	ippiMul_8u_C3IRSfs(SrcImg.buf, SrcImg.nPitch, m_ImgMask.buf, m_ImgMask.nPitch, roiSize, 0);



	//6、校正位置
	int nIndexX1, nIndexX2, nIndexY1, nIndexY2;
	Ipp32f* fProjectY = new Ipp32f[m_ImgBigArea.nHeight];
	m_hFun.KxProjectImage(m_ImgBigArea, 1, fProjectY);
	bool bNotFindY[2] = { true, true };
	for (int i = 0; i < m_ImgBigArea.nHeight; i++)
	{
		if ((fProjectY[i] > 5) && (bNotFindY[0]))
		{
			nIndexY1 = i;
			bNotFindY[0] = false;
		}
		if ((fProjectY[m_ImgBigArea.nHeight - i - 1] > 5) && (bNotFindY[1]))
		{
			nIndexY2 = m_ImgBigArea.nHeight - i - 1;
			bNotFindY[1] = false;
		}
		if (!bNotFindY[0] && !bNotFindY[1])
		{
			break;
		}

	}

	Ipp32f* fProjectX = new Ipp32f[m_ImgBigArea.nWidth];
	m_hFun.KxProjectImage(m_ImgBigArea, 0, fProjectX);
	bool bNotFindX[2] = { true, true };
	for (int i = 0; i < m_ImgBigArea.nWidth; i++)
	{
		if ((fProjectX[i] > 5) && (bNotFindX[0]))
		{
			nIndexX1 = i;
			bNotFindX[0] = false;
		}
		if ((fProjectX[m_ImgBigArea.nWidth - i - 1] > 5) && (bNotFindX[1]))
		{
			nIndexX2 = m_ImgBigArea.nWidth - i - 1;
			bNotFindX[1] = false;
		}
		if (!bNotFindX[0] && !bNotFindX[1])
		{
			break;
		}
	}

	delete[] fProjectX;
	delete[] fProjectY;

	DstImg.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
	if (bNotFindX[0] || bNotFindX[1] || bNotFindY[0] || bNotFindY[1] || nIndexX2 <= nIndexX1 || nIndexY2 <= nIndexY1)
	{
		kxRect<int> rc;
		rc.setup(0, 0, SrcImg.nWidth - 1, SrcImg.nHeight - 1);
		m_hFun.KxCopyImage(SrcImg, DstImg, rc);
	}
	else
	{
		IppiSize Roi = { nIndexX2 - nIndexX1, nIndexY2 - nIndexY1 };
		//Ipp64f mean[3];
		//Ipp8u* pSrc[3];
		//ippiMean_8u_C3R(SrcImg.buf + nIndexX1 * 3 + nIndexY1 * SrcImg.nPitch, SrcImg.nPitch, Roi, mean);
		//for (int i = 0; i < 3; i++)
  //      {
		//	m_ImgSingle[i].Init(SrcImg.nWidth, SrcImg.nHeight);
		//	ippsSet_8u(Ipp8u(mean[i]), m_ImgSingle[i].buf, m_ImgSingle[i].nPitch * m_ImgSingle[i].nHeight);
		//	pSrc[i] = m_ImgSingle[i].buf;
  //      }
		m_ImgColor.Init(SrcImg.nWidth, SrcImg.nHeight,3);
		//ippiCopy_8u_P3C3R(pSrc, m_ImgSingle[0].nPitch, m_ImgColor.buf, m_ImgColor.nPitch, roiSize);
		ippsSet_8u(64, m_ImgColor.buf, m_ImgColor.nPitch * m_ImgColor.nHeight);
		ippiCopy_8u_C3R(m_ImgColor.buf, m_ImgColor.nPitch, DstImg.buf, DstImg.nPitch, roiSize);

		m_ImgWhite.Init(m_ImgBigArea.nWidth, m_ImgBigArea.nHeight);
		ippsSet_8u(1, m_ImgWhite.buf, m_ImgWhite.nPitch * m_ImgWhite.nHeight);
		ippiSub_8u_C1IRSfs(m_ImgBigArea.buf, m_ImgBigArea.nPitch, m_ImgWhite.buf, m_ImgWhite.nPitch, roiSize, 0);
		m_ImgMask1.Init(SrcImg.nWidth, SrcImg.nHeight, SrcImg.nChannel);
		ippiDup_8u_C1C3R(m_ImgWhite.buf, m_ImgWhite.nPitch, m_ImgMask1.buf, m_ImgMask1.nPitch, roiSize);
		ippiMul_8u_C3IRSfs(m_ImgMask1.buf, m_ImgMask1.nPitch, m_ImgColor.buf, m_ImgColor.nPitch, roiSize, 0);
		ippiAdd_8u_C3IRSfs(m_ImgColor.buf, m_ImgColor.nPitch, m_ImgMask.buf, m_ImgMask.nPitch, roiSize, 0);

		int nDiffX = 20;
		int nDiffY = 20;


		Roi.width = (std::min)(Roi.width, DstImg.nWidth - nDiffX);
		Roi.height = (std::min)(Roi.height, DstImg.nHeight - nDiffY);
		ippiCopy_8u_C3R(m_ImgMask.buf + nIndexX1 * 3 + nIndexY1 * m_ImgMask.nPitch, m_ImgMask.nPitch, DstImg.buf + nDiffX * 3 + nDiffY * DstImg.nPitch, DstImg.nPitch, Roi);

		m_ImgQuanxiBiaoArea.Init(SrcImg.nWidth, SrcImg.nHeight);
		ippsSet_8u(0, m_ImgQuanxiBiaoArea.buf, m_ImgQuanxiBiaoArea.nPitch * m_ImgQuanxiBiaoArea.nHeight);
		ippiCopy_8u_C1R(m_ImgBigArea.buf + nIndexX1 * 1 + nIndexY1 * m_ImgBigArea.nPitch, m_ImgBigArea.nPitch, m_ImgQuanxiBiaoArea.buf + nDiffX * 1 + nDiffY * m_ImgQuanxiBiaoArea.nPitch, m_ImgQuanxiBiaoArea.nPitch, Roi);

		if (bModel == false)
		{
			roi.width = m_ImgQuanxiBiaoArea.nWidth;
			roi.height = m_ImgQuanxiBiaoArea.nHeight;	
			m_ImgDiff.Init(SrcImg.nWidth, SrcImg.nHeight);
			//ippiAbsDiff_8u_C1R(m_ImgQuanxiBiaoArea.buf, m_ImgQuanxiBiaoArea.nPitch, m_ImgTemplateArea.buf, m_ImgTemplateArea.nPitch, m_ImgDiff.buf, m_ImgDiff.nPitch, roi);
			//ippiMulC_8u_C1IRSfs(255, m_ImgDiff.buf, m_ImgDiff.nPitch, roi, 0);
			//m_hFun.KxOpenImage(m_ImgDiff, m_ImgOpen, 3, 1);
			//m_hFun.KxOpenImage(m_ImgOpen, m_ImgDiffSpeckle, 1, 3);
			//m_hFun.KxFilterSpeckles(m_ImgDiffSpeckle, m_ImgDiff, 10);
			//改为滑动对减的方式
			int  nDstW, nDstH;
			int GridX = m_ImgQuanxiBiaoArea.nWidth / 1;
			int GridY = m_ImgQuanxiBiaoArea.nHeight / 1;
			nDstH = ((m_ImgQuanxiBiaoArea.nHeight + (GridY - 1)) / GridY)*GridY;
			nDstW = ((m_ImgQuanxiBiaoArea.nWidth + (GridX - 1)) / GridX)*GridX;

			//m_ImgDiff1.Init(SrcImg.nWidth, SrcImg.nHeight);
			m_ImgCCBig.Init(nDstW, nDstH);
			enum 
			{
				_Dixy = 20,
			};
			m_ExtendImg.Init(nDstW + 2 * _Dixy, nDstH + 2 * _Dixy);
			IppiSize Roi = { m_ExtendImg.nWidth, m_ExtendImg.nHeight };
			status = ippiSet_8u_C1R(0, m_ExtendImg.buf, m_ExtendImg.nPitch, Roi);
			IppiSize roiSize = { m_ImgCCBig.nWidth, m_ImgCCBig.nHeight };
			status = ippiSet_8u_C1R(0, m_ImgCCBig.buf, m_ImgCCBig.nPitch, roiSize);

			//IppiSize sizBig = { width, height };
			status = ippiCopy_8u_C1R(m_ImgTemplateArea.buf, m_ImgTemplateArea.nPitch, m_ExtendImg.buf + _Dixy*m_ExtendImg.nPitch + _Dixy * 1, m_ExtendImg.nPitch, roi);
			status = ippiCopy_8u_C1R(m_ImgQuanxiBiaoArea.buf, m_ImgQuanxiBiaoArea.nPitch, m_ImgCCBig.buf, m_ImgCCBig.nPitch, roi);

			kxCImageBuf m_ImgTemplateAreaEdge, m_ImgQuanxiBiaoAreaEdge;
			m_hFun.KxErodeImage(m_ExtendImg, m_ImgTemplateAreaEdge);
			ippiSub_8u_C1RSfs(m_ImgTemplateAreaEdge.buf, m_ImgTemplateAreaEdge.nPitch, m_ExtendImg.buf, m_ExtendImg.nPitch,
				m_ImgTemplateAreaEdge.buf, m_ImgTemplateAreaEdge.nPitch, Roi, 0);

			m_hFun.KxErodeImage(m_ImgCCBig, m_ImgQuanxiBiaoAreaEdge);
			ippiSub_8u_C1RSfs(m_ImgQuanxiBiaoAreaEdge.buf, m_ImgQuanxiBiaoAreaEdge.nPitch, m_ImgCCBig.buf, m_ImgCCBig.nPitch,
				m_ImgQuanxiBiaoAreaEdge.buf, m_ImgQuanxiBiaoAreaEdge.nPitch, roiSize,0);

			int  widthCC = nDstW / GridX;
			int  heightCC = nDstH / GridY;

			m_GridImg.Init(nDstW, nDstH);
			m_ImgSlider.Init(nDstW, nDstH);
			m_ImgSliderEdge.Init(nDstW, nDstH);

			int nEnd = 5, nStart = -5, nStep = 1;
			int n = (nEnd - nStart) / nStep + 1;
			int nNum = n > 0 ? n : 0;
			int* pIndex = new int[nNum];
			memset(pIndex, 0, sizeof(int)*nNum);

			for (int i = 0; i < nNum; i++)
			{
				pIndex[i] = nStart + i*nStep;
			}

			parallel_for(blocked_range2d<int, int>(0, heightCC, 0, widthCC),
				[&](const blocked_range2d<int, int>& range)
			{
				for (int j = range.rows().begin(); j < range.rows().end(); j++)
				{
					for (int i = range.cols().begin(); i < range.cols().end(); i++)
					{
						IppStatus status;
						Ipp64f nMin = INT_MAX;
						IppiSize  Size = { GridX, GridY };
						int idx = 0, idy = 0;
						for (int y = 0; y < nNum; y++)
						{
							for (int x = 0; x < nNum; x++)
							{
								status = ippiAbsDiff_8u_C1R(m_ImgTemplateAreaEdge.buf + (_Dixy + j*GridY + pIndex[y])*m_ImgTemplateAreaEdge.nPitch + _Dixy + i*GridX + pIndex[x], m_ImgTemplateAreaEdge.nPitch, m_ImgQuanxiBiaoAreaEdge.buf + j*GridY*m_ImgQuanxiBiaoAreaEdge.nPitch + i*GridX, m_ImgQuanxiBiaoAreaEdge.nPitch,
									m_GridImg.buf + j*GridY*m_GridImg.nPitch + i*GridX, m_GridImg.nPitch, Size);
								Ipp64f  nSum = 0;
								status = ippiSum_8u_C1R(m_GridImg.buf + j*GridY*m_GridImg.nPitch + i*GridX, m_GridImg.nPitch, Size, &nSum);
								if (nMin > nSum)
								{
									nMin = nSum;
									//status = ippiCopy_8u_C1R(m_GridImg.buf + j*GridY*m_GridImg.nPitch + i*GridX, m_GridImg.nPitch, m_ImgSlider.buf + j*GridY*m_ImgSlider.nPitch + i*GridX, m_ImgSlider.nPitch, Size);
									idx = x;
									idy = y;
								}
							}
						 }
						//ippiAbsDiff_8u_C1R(m_ExtendImg.buf + (_Dixy + j*GridY + pIndex[idy])*m_ExtendImg.nPitch + _Dixy + i*GridX + pIndex[idx], m_ExtendImg.nPitch, m_ImgCCBig.buf + j*GridY*m_ImgCCBig.nPitch + i*GridX, m_ImgCCBig.nPitch,
						//	m_ImgSlider.buf + j*GridY*m_ImgSlider.nPitch + i*GridX, m_ImgSlider.nPitch, Size);
						//
						status = ippiCopy_8u_C1R(m_ExtendImg.buf + (_Dixy + j*GridY + pIndex[idy])*m_ExtendImg.nPitch + _Dixy + i*GridX + pIndex[idx], m_ExtendImg.nPitch, 
							m_ImgSlider.buf + j*GridY*m_ImgSlider.nPitch + i*GridX, m_ImgSlider.nPitch, Size);

						status = ippiCopy_8u_C1R(m_ImgTemplateAreaEdge.buf + (_Dixy + j*GridY + pIndex[idy])*m_ImgTemplateAreaEdge.nPitch + _Dixy + i*GridX + pIndex[idx], m_ImgTemplateAreaEdge.nPitch,
							m_ImgSliderEdge.buf + j*GridY*m_ImgSliderEdge.nPitch + i*GridX, m_ImgSliderEdge.nPitch, Size);


					}
				}
			}, auto_partitioner());

	
			ippiAbsDiff_8u_C1R(m_ImgSlider.buf, m_ImgSlider.nPitch, m_ImgCCBig.buf, m_ImgCCBig.nPitch,
				m_ImgDiff.buf, m_ImgDiff.nPitch, roi);

			//ippiAdd_8u_C1IRSfs(m_ImgSliderEdge.buf, m_ImgSliderEdge.nPitch, m_ImgAddEdge.buf, m_ImgAddEdge.nPitch, roi, 0);
			//static int n1 = 0;
			//if (n1 == 50)
			//{
			//	char sz[128];
			//	sprintf_s(sz, 128, "D:\\m_ImgAddEdge.bmp");
			//	m_hFun.SaveBMPImage_h(sz, m_ImgAddEdge);
			//}
			//n1++;

			//status = ippiCopy_8u_C1R(m_ImgSlider.buf, m_ImgSlider.nPitch, m_ImgDiff.buf, m_ImgDiff.nPitch, roi);
			m_hFun.KxOpenImage(m_ImgDiff, m_ImgOpen, 3, 1);
			m_hFun.KxOpenImage(m_ImgOpen, m_ImgDiffSpeckle, 1, 3);
			m_hFun.KxFilterSpeckles(m_ImgDiffSpeckle, m_ImgDiff, 10);
			ippiMulC_8u_C1IRSfs(255, m_ImgDiff.buf, m_ImgDiff.nPitch, roi, 0);

			delete[] pIndex;
		}
		//kxRect<int> rc;
		//rc.setup(0, 0, m_ImgMask.nWidth - 1, m_ImgMask.nHeight - 1);
		//m_hFun.KxCopyImage(m_ImgMask, DstImg, rc);
		m_nOffsetX = (nDiffX - nIndexX1);
		m_nOffsetY = (nDiffY - nIndexY1);
	}


	//if (1)
	//{
	//	char sz[128];
	//	//sprintf_s(sz, 128, "D:\\2_Mask.bmp");
	//	//hFun.SaveBMPImage_h(sz, m_ImgMask);
	//	//sprintf_s(sz, 128, "D:\\2_m_ImgBigArea.bmp");
	//	//hFun.SaveBMPImage_h(sz, m_ImgBigArea);
	//	//sprintf_s(sz, 128, "D:\\2_m_ImgFillHoles.bmp");
	//	//hFun.SaveBMPImage_h(sz, m_ImgFillHoles);
	//	static int n = 0;
	//	sprintf_s(sz, 128, "D:\\123\\m_ImgDiff_%d.bmp", n++);
	//	m_hFun.SaveBMPImage_h(sz, m_ImgDiff);
	//}

	return 1;
}



int CKxQuanxiBiaoCheck::SetCheckExceptionStatus(const kxCImageBuf& ImgCheck)
{
	m_hResult.m_nCount = 1;
	m_hResult.m_hBlobInfo[0].m_nAreaType = m_hParameter.m_nAreaNumber;
	m_hResult.m_hBlobInfo[0].m_nDots = ImgCheck.nWidth * ImgCheck.nHeight;
	m_hResult.m_hBlobInfo[0].m_nEnergy = ImgCheck.nWidth * ImgCheck.nHeight;

	m_hResult.m_hBlobInfo[0].m_nLeft = 0;
	m_hResult.m_hBlobInfo[0].m_nTop = 0;
	m_hResult.m_hBlobInfo[0].m_nBlobWidth = ImgCheck.nWidth - 1;
	m_hResult.m_hBlobInfo[0].m_nBlobHeight = ImgCheck.nHeight - 1;
	m_hResult.m_hBlobInfo[0].m_nWHRatio = 0;

	m_hResult.m_hBlobInfo[0].m_nAvgEnergy = 1;
	m_hResult.m_hBlobInfo[0].m_nAvgArea = 1;
	m_hResult.m_nStatus = _Check_Err;


	return 1;
}


int CKxQuanxiBiaoCheck::SaveMidImage(bool bSave)
{
#if defined( _WIN32 ) || defined ( _WIN64 )
	static int n = 0;
	if (bSave)
	{
		char sz[128];
		sprintf_s(sz, 128, "d:\\Save\\Warp\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_WarpImg);

		sprintf_s(sz, 128, "d:\\Save\\WarpExtend\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_ExtendWarpImg);

		sprintf_s(sz, 128, "d:\\Save\\WarpExtend1\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_ExtendWarpImg1);


	}

	if (bSave)
	{
		char sz[128];
		for (int i = 0; i < _Max_Layer - 1; i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				sprintf_s(sz, 128, "d:\\Save\\Process\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgCheck[i]);
			}

		}

	}

	if (bSave)
	{
		char sz[128];
		for (int i = 0; i < _Max_Layer - 1; i++)
		{
			sprintf_s(sz, 128, "d:\\Save\\Learn\\%d.bmp", i);
			m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemplate[i]);
		}

	}

	if (bSave)
	{
		char sz[128];
		for (int i = 0; i < _Max_Layer - 1; i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				sprintf_s(sz, 128, "d:\\Save\\Offset\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgOffset[i]);

				sprintf_s(sz, 128, "d:\\Save\\SliderCc\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCc[i]);


				sprintf_s(sz, 128, "d:\\Save\\SliderNorm\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSliderNorm[i]);

				sprintf_s(sz, 128, "d:\\Save\\ResImg\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ResImg[i]);


				sprintf_s(sz, 128, "d:\\Save\\SlidCcMax\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCcMax[i]);


			}

		}
		sprintf_s(sz, 128, "d:\\Save\\Cc\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemp);
		n++;
	}
#else

	static int n = 0;
	if (bSave)
	{
		char sz[128];
		snprintf(sz, 128, "d:\\Save\\Warp\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_WarpImg);

		snprintf(sz, 128, "d:\\Save\\WarpExtend\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_ExtendWarpImg);

		snprintf(sz, 128, "d:\\Save\\WarpExtend1\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_ExtendWarpImg1);


	}

	if (bSave)
	{
		char sz[128];
		for (int i = 0; i < _Max_Layer - 1; i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				snprintf(sz, 128, "d:\\Save\\Process\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgCheck[i]);
			}

		}

	}

	if (bSave)
	{
		char sz[128];
		for (int i = 0; i < _Max_Layer - 1; i++)
		{
			snprintf(sz, 128, "d:\\Save\\Learn\\%d.bmp", i);
			m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemplate[i]);
		}

	}

	if (bSave)
	{
		char sz[128];
		for (int i = 0; i < _Max_Layer - 1; i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				snprintf(sz, 128, "d:\\Save\\Offset\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgOffset[i]);

				snprintf(sz, 128, "d:\\Save\\SliderCc\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCc[i]);


				snprintf(sz, 128, "d:\\Save\\SliderNorm\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSliderNorm[i]);

				snprintf(sz, 128, "d:\\Save\\ResImg\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ResImg[i]);


				snprintf(sz, 128, "d:\\Save\\SlidCcMax\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCcMax[i]);


			}

		}
		snprintf(sz, 128, "d:\\Save\\Cc\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemp);
		n++;
	}
#endif // defined


	return 1;

}
