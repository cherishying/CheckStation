
#include "KxZhuGuangCheck.h"
#include "KxReadXml.h"


CKxZhuGuangCheck::CKxZhuGuangCheck()
{
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_bIsLearnCompleted[i] = false;
	}
	m_bAllCompleted = false;

	m_nLearnPage = 50;

	memset(m_nAlgorithmStatus, 0, sizeof(int)* _Max_Layer);

	m_nLearnStatus = 0;

}

CKxZhuGuangCheck::~CKxZhuGuangCheck()
{

}

bool CKxZhuGuangCheck::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
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

	AnalysisCheckStatus();


	return true;

}

bool CKxZhuGuangCheck::ReadParaXml(const char* filePath)
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
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nGrayLowOffset);
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

	return true;

}


bool CKxZhuGuangCheck::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
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

	AnalysisCheckStatus();


	return true;

}

bool CKxZhuGuangCheck::ReadParaXmlinEnglish(const char* filePath)
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
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nGrayLowOffset);
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

	return true;

}



bool CKxZhuGuangCheck::ReadVesion1Para(FILE* fp)    //读取版本1参数
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

bool CKxZhuGuangCheck::ReadVesion2Para(FILE* fp)    //读取版本2参数
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

	if (m_hParameter.m_nGrayOffset < _MAX_OFFSET)
	{
		m_hParameter.m_nGrayLowOffset = m_hParameter.m_nGrayOffset;
		m_hParameter.m_nLossOffset = _MAX_OFFSET;
	}

	return true;
}


bool CKxZhuGuangCheck::ReadVesion3Para(FILE* fp)    //读取版本3参数
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
	if (fread(&m_hParameter.m_nGrayLowOffset, sizeof(int), 1, fp) != 1)  //增加灰度减少参数
	{
		return false;
	}


	return true;
}


bool CKxZhuGuangCheck::ReadPara(FILE*  fp)
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
	else if (strcmp(m_hParameter.m_szVersion, "CheckControl3.0") == 0)
	{
		return ReadVesion3Para(fp);
	}
	else
	{
		return false;
	}
}


bool CKxZhuGuangCheck::Read(FILE*  fp)
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

	AnalysisCheckStatus();


	return true;
}

bool CKxZhuGuangCheck::WriteVesion1Para(FILE* fp)    //写入版本1参数
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



bool CKxZhuGuangCheck::WritePara(FILE*  fp)
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

bool CKxZhuGuangCheck::Write(FILE*  fp)
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
int CKxZhuGuangCheck::ReadParaFromNet(unsigned char*& point)
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

int CKxZhuGuangCheck::LoadTemplateImg(const char* lpszFile)
{
	FILE*   fp;


#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "rb") != 0)
		return FALSE;
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

int CKxZhuGuangCheck::SaveTemplateImg(const char* lpszFile)
{
	FILE*   fp;

#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "wb") != 0)
		return FALSE;
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

int CKxZhuGuangCheck::SaveLearnTemplateImg(const char* lpszFile)
{
	FILE*   fp;


#if defined( _WIN32 ) || defined ( _WIN64 )
	if (fopen_s(&fp, lpszFile, "wb") != 0)
		return FALSE;
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

int CKxZhuGuangCheck::WriteLocalLearnImg(FILE* fp)
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

int CKxZhuGuangCheck::LoadLearnTemplateImg(const char* lpszFile)
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


int CKxZhuGuangCheck::ReadLocalLearnImg(FILE* fp)
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


int CKxZhuGuangCheck::ReadImg(FILE* fp)
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

int CKxZhuGuangCheck::WriteImg(FILE* fp)
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
int  CKxZhuGuangCheck::ProcessSurfaceWarpImages(KxCallStatus& hCall)
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
							m_hGradient[i].ParallelSingleDirGradientSame(m_ExtendWarpImg, m_ImgCheck[i], i, CKxGradientProcess::_AverageFilter, 13, 255, hCallInfo[i]);
							break;
				}
				case 1:  //检线
				{
							 m_ImgCheck[i].Init(m_ExtendWarpImg.nWidth / 4, m_ExtendWarpImg.nHeight / 4, m_ExtendWarpImg.nChannel);
							 ippsSet_8u(0, m_ImgCheck[i].buf, m_ImgCheck[i].nPitch * m_ImgCheck[i].nHeight);
							 for (int j = 0; j < _CheckModeNum; j++)
							 {
								 bool bWhite = ((j == 1) ? 1 : 0);
								 int nStatus = m_hGradient[i].SingleDirGradient(m_ExtendWarpImg, m_MidImg[i], i - _SAMLLDOTS_INDEX_END, bWhite, 4, 255, 5, hCallInfo[i]);
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
		for (int i = _SCRATCH_INDEX_END; i < _SCRATCH_INDEX_END+1; i++)
		{
			m_hBaseFun[_SCRATCH_INDEX_END].KxParallelMaxEvery(m_ImgCheck[i], m_ImgCheck[_Max_Layer - 1]);
		}
	}

	return 1;
}

//学习图像
int  CKxZhuGuangCheck::LearnTemplate(KxCallStatus& hCall)
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
		nModelProcessSz[i] = _3X3;
	}
	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		nModelProcessSz[i] = _1X1;  //灰度检查
	}
	for (int i = _GRAY_INDEX_END; i < _Max_Layer; i++)
	{
		nModelProcessSz[i] = _3X3;  //整体检缺
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
		m_ImgTemplate[_Max_Layer].Init(m_ImgTemplate[_SCRATCH_INDEX_END].nWidth, m_ImgTemplate[_SCRATCH_INDEX_END].nHeight, m_ImgTemplate[_SCRATCH_INDEX_END].nChannel);
		ippsSet_8u(0, m_ImgTemplate[_Max_Layer].buf, m_ImgTemplate[_Max_Layer].nPitch * m_ImgTemplate[_Max_Layer].nHeight);
		//for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
		//{
		//	if (m_nLearnStatus)
		//	{
		//		m_ImgCompose.Init(m_ImgTemplate[i].nWidth / 4, m_ImgTemplate[i].nHeight / 4, m_ImgTemplate[i].nChannel);
		//		m_hFun.KxParallelResizeImage(m_ImgTemplate[i], m_ImgCompose);
		//		m_hFun.KxParallelMaxEvery(m_ImgCompose, m_ImgTemplate[_Max_Layer]);
		//	}
		//}
		//for (int i = _SAMLLDOTS_INDEX_END; i < _SCRATCH_INDEX_END; i++)
		//{
		//	if (m_nLearnStatus)
		//	{
		//		m_hFun.KxParallelMaxEvery(m_ImgTemplate[i], m_ImgTemplate[_Max_Layer]);
		//	}
		//}
		m_nLearnStatus = 0;

		for (int k = 0; k < _Max_Layer + 1; k++)
		{
			m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
			kxRect<int> rc;
			rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
			m_hFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
		}


	}

	return 1;
}

//将原分辨率残差图换算到压缩分辨率残差图上
int CKxZhuGuangCheck::ConvertSliderCc(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nDirection, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	IppiSize roiSize = { SrcImg.nWidth, SrcImg.nHeight };

	//5*5膨胀
	Ipp8u pMask1[5][5] =
	{
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
	};
	IppiSize maskSize = { 5, 5 };
	m_TmpImg[nDirection].Init(roiSize.width, roiSize.height, SrcImg.nChannel);

	m_hBaseFun[nDirection].KxDilateImage(SrcImg, m_TmpImg[nDirection], maskSize.width, maskSize.height, (Ipp8u*)pMask1, hCallInfo);

	if (check_sts(hCallInfo, "ConvertSliderCc_KxDilateImage", hCall))
	{
		return 0;
	}

	//4*4压缩
	roiSize.width /= 4;
	roiSize.height /= 4;
	//m_TmpImg1[nDirection].Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	//m_hBaseFun[0].KxParallelResizeImage(m_TmpImg, m_TmpImg1, KxSuper, hCallInfo, 100, 100);
	DstImg.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	m_hBaseFun[nDirection].KxParallelResizeImage(m_TmpImg[nDirection], DstImg, KxSuper, hCallInfo, 100, 100);
	if (check_sts(hCallInfo, "ConvertSliderCc_KxParallelResizeImage", hCall))
	{
		return 0;
	}

	//5*5平滑滤波
	//Ipp16s pKernel3[4][5][5] =
	//{
	//	1, 1, 1, 1, 1,
	//	0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0,

	//	1, 1, 1, 1, 1,
	//	0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0,
	//	0, 0, 0, 0, 0,
	//};
	//IppiSize kernelSize3[2] = { 1, 5, 5, 1 };
	//DstImg.Init(roiSize.width, roiSize.height, SrcImg.nChannel);
	//m_hBaseFun[nDirection].KxParallelAverageFilterImage(m_TmpImg1[nDirection], DstImg, kernelSize3[nDirection].width, kernelSize3[nDirection].height, (Ipp16s*)pKernel3[nDirection], hCallInfo);
	//if (check_sts(hCallInfo, "ConvertSliderCc_KxParallelAverageFilterImage", hCall))
	//{
	//	return 0;
	//}
	return 1;
}


//残差分析并且合并为一张残差图
int CKxZhuGuangCheck::ResidualsAnalys(KxCallStatus& hCall)
{
	hCall.Clear();
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_ImgSlidCc[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight, m_ImgCheck[i].nChannel);
		m_ImgSliderNorm[i].Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight, m_ExtendWarpImg.nChannel);
		m_ImgSlidCcMax[i].Init(m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight);
		m_ImgOffset[i].Init(m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight, m_ImgSliderNorm[i].nChannel);
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
	for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	{
		nGridXY[i] = 40;
		nOffset[i] = m_hParameter.m_nSmallDotOffset;
	}
	for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
	{
		nGridXY[i] = 20;
		nOffset[i] = m_hParameter.m_nLineOffset;
	}
	for (int i = _LINE_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	{
		nGridXY[i] = 20;
		nOffset[i] = m_hParameter.m_nScratchOffset;
	}
	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		nGridXY[i] = 40;
		//nOffset[i] = m_hParameter.m_nGrayOffset;
	}

	nOffset[_SCRATCH_INDEX_END] = m_hParameter.m_nGrayOffset;
	nOffset[_SCRATCH_INDEX_END + 1] = m_hParameter.m_nGrayLowOffset;


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
				//对于原始分辨先压4倍
				if ((i < _SAMLLDOTS_INDEX_END))
				{
					ConvertSliderCc(m_ImgSlidCc[i], m_ImgSliderNorm[i], i, hCallInfo[i]);
					if (check_sts(hCallInfo[i], "", hCall))
					{
						break;
					}
				}
				else
				{
					kxRect<int> rc;
					rc.setup(0, 0, m_ImgSlidCc[i].nWidth - 1, m_ImgSlidCc[i].nHeight - 1);
					m_hBaseFun[i].KxCopyImage(m_ImgSlidCc[i], m_ImgSliderNorm[i], rc);
				}

				IppiSize roiSize = { m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight };
				m_hBaseFun[i].KxSubCImage(m_ImgSliderNorm[i], m_ImgOffset[i], nOffset[i]);

				if (m_ImgOffset[i].nChannel == _Type_G24)
				{
					IppiSize roiSize = { m_ImgOffset[i].nWidth, m_ImgOffset[i].nHeight };
					ippiGradientColorToGray_8u_C3C1R(m_ImgOffset[i].buf, m_ImgOffset[i].nPitch, m_ImgSlidCcMax[i].buf, m_ImgSlidCcMax[i].nPitch, roiSize, ippiNormInf);
				}
				else
				{
					IppiSize roiSize = { m_ImgOffset[i].nWidth, m_ImgOffset[i].nHeight };
					ippiCopy_8u_C1R(m_ImgOffset[i].buf, m_ImgOffset[i].nPitch, m_ImgSlidCcMax[i].buf, m_ImgSlidCcMax[i].nPitch, roiSize);
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

	//边缘检测只用检小点残差，其他方法出现的残差置0
	for (int i = 0; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_ResImg[i].Init(m_ImgSlidCcMax[i].nWidth, m_ImgSlidCcMax[i].nHeight, m_ImgSlidCcMax[i].nChannel);
			kxRect<int> rc;
			rc.setup(0, 0, m_ResImg[i].nWidth - 1, m_ResImg[i].nHeight - 1);
			m_hBaseFun[_SCRATCH_INDEX_END].KxCopyImage(m_ImgSlidCcMax[i], m_ResImg[i], rc);
		}
	}
	//for (int i = _SAMLLDOTS_INDEX_END; i < _Max_Layer; i++)
	//{
	//	kxRect<int> rcWarp = m_hWarpStrech.GetParameter().m_rcCheckArea;
	//	kxRect<int> rcEdge = m_hParameter.m_rcEdgeRect;
	//	if (m_nAlgorithmStatus[i])
	//	{
	//		m_ResImg[i].Init(m_ImgSlidCcMax[i].nWidth, m_ImgSlidCcMax[i].nHeight, m_ImgSlidCcMax[i].nChannel);
	//		ippsSet_8u(0, m_ResImg[i].buf, m_ResImg[i].nHeight*m_ResImg[i].nPitch);
	//		kxRect<int> rc;
	//		rc.left = ((rcWarp.left - rcEdge.left) / 4 + 1) >= 0 ? ((rcWarp.left - rcEdge.left) / 4 + 1) : 0;
	//		rc.top = ((rcWarp.top - rcEdge.top) / 4 + 1) >= 0 ? ((rcWarp.top - rcEdge.top) / 4 + 1) : 0;
	//		rc.right = (rc.left + rcWarp.Width() / 4 - 1) <= m_ResImg[i].nWidth - 1 ? (rc.left + rcWarp.Width() / 4 - 1) : m_ResImg[i].nWidth - 1;
	//		rc.bottom = (rc.top + rcWarp.Height() / 4 - 1) <= m_ResImg[i].nHeight - 1 ? (rc.top + rcWarp.Height() / 4 - 1) : m_ResImg[i].nHeight - 1;;
	//		IppiSize roi = { rc.Width(), rc.Height() };
	//		if (_Type_G24 == m_ResImg[i].nChannel)
	//		{
	//			ippiCopy_8u_C3R(m_ImgSlidCcMax[i].buf + rc.left * 3 + rc.top * m_ImgSlidCcMax[i].nPitch, m_ImgSlidCcMax[i].nPitch,
	//				m_ResImg[i].buf + rc.left * 3 + rc.top*m_ResImg[i].nPitch, m_ResImg[i].nPitch, roi);
	//		}
	//		else
	//		{
	//			ippiCopy_8u_C1R(m_ImgSlidCcMax[i].buf + rc.left * 1 + rc.top * m_ImgSlidCcMax[i].nPitch, m_ImgSlidCcMax[i].nPitch,
	//				m_ResImg[i].buf + rc.left * 1 + rc.top*m_ResImg[i].nPitch, m_ResImg[i].nPitch, roi);
	//		}
	//	}
	//}
	for (int i = 0; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SCRATCH_INDEX_END].KxParallelMaxEvery(m_ResImg[i], m_ImgTemp);
		}
	}

	//图像掩膜
	//m_hImgMask.Mask(m_ImgTemp, m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
	m_hImgMask.Mask(m_ImgTemp, m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0, hCallInfo[0]);

	if (check_sts(hCallInfo[0], "", hCall))
	{
		return 0;
	}


	//多边形掩膜
	m_hPolygonMask.Mask(m_ImgTemp, m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0, hCallInfo[0]);


	if (check_sts(hCallInfo[0], "", hCall))
	{
		return 0;
	}

	return 1;

}


int CKxZhuGuangCheck::ConnectAnalysis(const kxCImageBuf& SrcImg, KxCallStatus& hCall)
{
	int nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, 3, 0, hCall);

	if (nStatus == 0)
	{
		return 0;
	}
	int nCount = (std::min)((int)_Max_BlobCount, m_hBlobAnaly.GetBlobCount());

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
		m_hResult.m_hBlobInfo[i].m_nDots = pObj.m_nDots * 4;
		m_hResult.m_hBlobInfo[i].m_nEnergy = pObj.m_nEnergy * 4 / 25;
		kxRect<int> rc;
		kxRect<int> rcDst;
		rc.setup(pObj.m_rc.left * 4, pObj.m_rc.top * 4 , pObj.m_rc.right * 4, pObj.m_rc.bottom * 4 );
		rc.offset(rcWarp.left, rcWarp.top);
		if (rc.left > rcWarp.left && rc.top > rcWarp.top && rc.bottom < rcWarp.bottom && rc.right < rcWarp.right)
		{
			m_hWarpStrech.GetRightPos(rc, rcDst);
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
		m_hResult.m_hBlobInfo[i].m_nWHRatio = int(pObj.m_fRatio);

		m_hResult.m_hBlobInfo[i].m_nAvgEnergy = pObj.m_nEnergy  / pObj.m_nDots;

		m_hResult.m_hBlobInfo[i].m_nAvgArea = int(pObj.m_fSumEnergy * 4 / 25);  //总能量

	}

	m_hResult.m_nStatus = _Check_Err;

	return 1;
}

int CKxZhuGuangCheck::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex)
{
	KxCallStatus hCall;
	return SingleDefectLearn(ImgCheck, nBlobIndex, hCall);
}

//单个缺陷学习
int CKxZhuGuangCheck::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;


	int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_WarpImg, hCall);
	if (check_sts(nStatus, "SingleDefectLearn_", hCall) || nStatus != 0)
	{
		return 0;
	}


	m_hFun.KxZhuGuangCardFilter(m_WarpImg, m_ExtendWarpImg, hCallInfo);

	if (check_sts(hCallInfo, "珠光预处理", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	//CopyEdges(ImgCheck, m_WarpImg, m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);

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


	nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, 3, 0);
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
	rcBig.setup(pObj.m_rc.left , pObj.m_rc.top, pObj.m_rc.right, pObj.m_rc.bottom);


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
			rcTemp.left = gMax(rcTemp.left - 2, 0);
			rcTemp.top = gMax(rcTemp.top - 2, 0);
			rcTemp.right = gMin(rcTemp.right + 2, m_ImgCheck[i].nWidth - 1);
			rcTemp.bottom = gMin(rcTemp.bottom + 2, m_ImgCheck[i].nHeight - 1);
			rcLearn[i] = rcTemp;
		}
	}

	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
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
	rcLearn[_Max_Layer - 1] = rcSmall;


	//学多
	for (int i = 0; i <= _SCRATCH_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			if (rcLearn[i].top < 0 || rcLearn[i].left < 0 || rcLearn[i].right > m_ImgCheck[i].nWidth - 1 || rcLearn[i].bottom > m_ImgCheck[i].nHeight - 1
				|| m_ImgLearnTemplate[i].nWidth <= 0 || m_ImgLearnTemplate[i].nHeight <= 0 || m_ImgLearnTemplate[i].buf == NULL)
			{
				return 0;
			}

			IppiSize roiSize = { rcLearn[i].Width(), rcLearn[i].Height() };
			if (m_ImgOffset[i].nChannel == _Type_G24)
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
			if (rcLearn[i].top < 0 || rcLearn[i].left < 0 || rcLearn[i].right > m_ImgCheck[i].nWidth - 1 || rcLearn[i].bottom > m_ImgCheck[i].nHeight - 1
				|| m_ImgLearnTemplate[i].nWidth <= 0 || m_ImgLearnTemplate[i].nHeight <= 0 || m_ImgLearnTemplate[i].buf == NULL)
			{
				return 0;
			}

			IppiSize roiSize = { rcLearn[i].Width(), rcLearn[i].Height() };
			if (m_ImgOffset[i].nChannel == _Type_G24)
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
	m_ImgLearnTemplate[_Max_Layer].Init(m_ImgLearnTemplate[_SCRATCH_INDEX_END].nWidth, m_ImgLearnTemplate[_SCRATCH_INDEX_END].nHeight, m_ImgLearnTemplate[_SCRATCH_INDEX_END].nChannel);
	ippsSet_8u(0, m_ImgLearnTemplate[_Max_Layer].buf, m_ImgLearnTemplate[_Max_Layer].nPitch * m_ImgLearnTemplate[_Max_Layer].nHeight);
	//for (int i = 0; i < _SAMLLDOTS_INDEX_END; i++)
	//{
	//	if (m_nAlgorithmStatus[i])
	//	{
	//		m_ImgCompose.Init(m_ImgLearnTemplate[i].nWidth / 4, m_ImgLearnTemplate[i].nHeight / 4, m_ImgLearnTemplate[i].nChannel);
	//		m_hFun.KxParallelResizeImage(m_ImgLearnTemplate[i], m_ImgCompose);
	//		m_hFun.KxMaxEvery(m_ImgCompose, m_ImgLearnTemplate[_Max_Layer]);
	//	}
	//}
	//for (int i = _SAMLLDOTS_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	//{
	//	if (m_nAlgorithmStatus[i])
	//	{
	//		m_hFun.KxMaxEvery(m_ImgLearnTemplate[i], m_ImgLearnTemplate[_Max_Layer]);
	//	}
	//}

	return 1;
}


int CKxZhuGuangCheck::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6])
{
	KxCallStatus hCall;
	return ShowResidualsImage(SrcImg, DstImg, nOffset, hCall);
}
//待检测图像的残差图显示
//残差图始终是一张彩色图像,
//蓝色表示检小点残差、绿色表示检细线残差、红色表示检划伤残差、黑色表示检缺失残差、白色表示
int CKxZhuGuangCheck::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6], KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCall.Clear();
	int nCompressSize = 1;

	m_hParameter.m_nSmallDotOffset = nOffset[0];
	m_hParameter.m_nLineOffset = nOffset[1];
	m_hParameter.m_nScratchOffset = nOffset[2];
	m_hParameter.m_nLossOffset = nOffset[3];
	m_hParameter.m_nGrayLowOffset = nOffset[4];
	m_hParameter.m_nGrayOffset = nOffset[5];

	AnalysisCheckStatus();

	int nStatus = m_hWarpStrech.CheckParallel(SrcImg, m_WarpImg, hCall);
	if (check_sts(nStatus, "ShowResidualsImage_", hCall) || nStatus != 0)
	{
		return 0;
	}
	//CopyEdges(SrcImg, m_WarpImg, m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);
	//珠光处理,结果图压缩4倍
	m_hFun.KxZhuGuangCardFilter(m_WarpImg, m_ExtendWarpImg, hCallInfo);

	if (check_sts(hCallInfo, "珠光预处理", hCall))
	{
		SetCheckExceptionStatus(SrcImg);
		return 0;
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
	ConnectAnalysis(SrcImg, hCallInfo);
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
	m_ResizeImg.Init(m_ColorImg.nWidth, m_ColorImg.nHeight, m_ColorImg.nChannel);
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
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ImgSlidCcMax[i], m_ResidImg[0]);
		}
	}
	//获得检细线的残差图
	for (int i = _SAMLLDOTS_INDEX_END; i < _LINE_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ImgSlidCcMax[i], m_ResidImg[1]);
		}
	}
	//获得检划伤的残差图
	for (int i = _LINE_INDEX_END; i < _SCRATCH_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ImgSlidCcMax[i], m_ResidImg[2]);
		}
	}

	//检缺残差图
	for (int i = _SCRATCH_INDEX_END; i < _LOSS_INDEX_END; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_hBaseFun[_SAMLLDOTS_INDEX_END].KxMaxEvery(m_ImgSlidCcMax[i], m_ResidImg[3]);
		}
	}

	//合成三张残差图
	m_AddImage.Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight, m_ResizeImg.nChannel);
	ippsSet_8u(0, m_AddImage.buf, m_AddImage.nPitch*m_AddImage.nHeight);

	for (int i = 0; i < 3; i++)
	{
		m_hImgMask.Mask(m_ResidImg[i], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
		m_hPolygonMask.Mask(m_ResidImg[i], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
		m_CompositionImg[i].Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight, m_ResizeImg.nChannel);
		AddResidualImage(m_ResidImg[i], m_ResizeImg, i, m_CompositionImg[i]);
		m_hFun.KxAddImage(m_CompositionImg[i], m_AddImage);

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
	DstImg.Init(SrcImg.nWidth/4, SrcImg.nHeight/4, 3);
	m_hFun.KxResizeImage(m_ResizeImg1, DstImg);


	IppiSize Roi = { m_AddImage.nWidth, m_AddImage.nHeight };

	//if (m_nAlgorithmStatus[_Max_Layer - 1])
	//{
	//	m_hImgMask.Mask(m_ResidImg[3], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
	//	m_hPolygonMask.Mask(m_ResidImg[3], m_WarpImg, m_hWarpStrech.GetParameter().m_rcCheckArea, 0);
	//	SetMask(m_AddImage, m_ResidImg[3], 1, 255);
	//}


	kxRect<int> rcDst;
	m_hWarpStrech.GetRightPos(m_hWarpStrech.GetParameter().m_rcCheckArea, rcDst);

	if (rcDst.right / 4 > DstImg.nWidth - 1 || rcDst.bottom / 4 > DstImg.nHeight - 1 || rcDst.left < 0 || rcDst.top < 0)
	{
		rcDst = m_hWarpStrech.GetParameter().m_rcCheckArea;
	}

	Roi.width = (std::min)(DstImg.nWidth - rcDst.left / 4 - 1, Roi.width);
	Roi.height = (std::min)(DstImg.nHeight - rcDst.top / 4 - 1, Roi.height);


	ippiCopy_8u_C3R(m_AddImage.buf, m_AddImage.nPitch,
		DstImg.buf + rcDst.top / 4 * DstImg.nPitch + rcDst.left / 4 * 3, DstImg.nPitch, Roi);


	return 1;
}

int CKxZhuGuangCheck::SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate)
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
int CKxZhuGuangCheck::AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg)
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
int CKxZhuGuangCheck::CopyEdges(const kxCImageBuf& SrcImg, const kxCImageBuf CopyImg, kxCImageBuf& DstImg, kxRect<int>& rcEdge, kxRect<int>& rcCopy)
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

int CKxZhuGuangCheck::AnalysisCheckStatus()
{
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_nAlgorithmStatus[i] = 0;
	}

	//灰度检
	if (m_hParameter.m_nGrayOffset < _MAX_OFFSET)
	{

		m_nAlgorithmStatus[_SCRATCH_INDEX_END] = 1;
		m_hParameter.m_nGrayOffset *= 8;

	}
	if (m_hParameter.m_nGrayLowOffset < _MAX_OFFSET)
	{
		m_nAlgorithmStatus[_SCRATCH_INDEX_END + 1] = 1;
		m_hParameter.m_nGrayLowOffset *= 8;
	}


	if (m_hParameter.m_nIsLearn)
	{
		m_nLearnStatus = 1;
	}

	return 1;
}

int CKxZhuGuangCheck::Check(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return Check(ImgCheck, hCall);
}

int CKxZhuGuangCheck::Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	hCall.Clear();
	tick_count tbb_start, tbb_end;
	m_hResult.Clear();
	//task_scheduler_init init;
	//firstly, accurate correction a image
	tbb_start = tick_count::now();




	//if (m_hCalDistance.Check(ImgCheck, hCall))
	//{
	//	m_hCalDistance.GetHorVerDistance(m_hResult.m_nDistanceX, m_hResult.m_nDistanceY);
	//	m_hResult.m_nStatus = _Distance_Err;
	//	return 0;
	//}

	int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_WarpImg, hCall);
	if (nStatus)
	{
		m_hResult.m_nStatus = _Similarity_Err;
		m_hResult.m_nKernIndex = nStatus - 1;
		return 0;
	}
	//CopyEdges(ImgCheck, m_WarpImg, m_ExtendWarpImg, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);



	tbb_end = tick_count::now();
	//printf("-----warp process cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);



	//if (_Type_G24 == m_WarpImg.nChannel)
	//{
	//	m_ColorCheckImg.Init(ImgCheck.nWidth, ImgCheck.nHeight, ImgCheck.nChannel);
	//	ippsSet_8u(0, m_ColorCheckImg.buf, m_ColorCheckImg.nPitch*m_ColorCheckImg.nHeight);
	//	m_hFun.KxCopyImage(m_WarpImg, m_ColorCheckImg, m_hWarpStrech.GetParameter().m_rcCheckArea);

	//	if (m_hColorDiff.Check(m_ColorCheckImg))
	//	{
	//		m_hResult.m_nStatus = _ColorDiff_Err;
	//		m_hColorDiff.GetColorDiff(m_hResult.m_nColorDiff);
	//		return 0;
	//	}
	//}

	//珠光处理,结果图压缩4倍
	m_hFun.KxZhuGuangCardFilter(m_WarpImg, m_ExtendWarpImg, hCallInfo);

	if (check_sts(hCallInfo, "珠光预处理", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	//secondly, process a image with some methods

	tbb_start = tick_count::now();

	ProcessSurfaceWarpImages(hCallInfo);
	if (check_sts(hCallInfo, "珠光处理中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	tbb_end = tick_count::now();
	//printf(" -----process cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);


	//Thirdly, learn a image
	tbb_start = tick_count::now();

	LearnTemplate(hCallInfo);
	if (check_sts(hCallInfo, "珠光学习中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	tbb_end = tick_count::now();
	//printf("-----learn cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);


	//Fourthly,compute a sub image
	tbb_start = tick_count::now();

	ResidualsAnalys(hCallInfo);
	if (check_sts(hCallInfo, "珠光比较中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}

	tbb_end = tick_count::now();
	//printf(" -----slidercc cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);


	//fifth,blob Analysis
	tbb_start = tick_count::now();

	ConnectAnalysis(ImgCheck,  hCallInfo);
	if (check_sts(hCallInfo, "珠光分析中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
	}


	tbb_end = tick_count::now();
	//printf(" -----blob cost : %f ms\n",  (tbb_end - tbb_start).seconds() * 1000);

	//SaveMidImage(true);

	return 1;

}


int CKxZhuGuangCheck::SetCheckExceptionStatus(const kxCImageBuf& ImgCheck)
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


int CKxZhuGuangCheck::SaveMidImage(bool bSave)
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
	}

	if (bSave)
	{
		char sz[128];
		for (int i = _GRAY_INDEX_END - 1; i < _GRAY_INDEX_END; i++)
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
		for (int i = _GRAY_INDEX_END - 1; i < _GRAY_INDEX_END; i++)
		{
			sprintf_s(sz, 128, "d:\\Save\\Learn\\%d.bmp", i);
			m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemplate[i]);
		}

	}

	if (bSave)
	{
		char sz[128];
		for (int i = _GRAY_INDEX_END - 1; i < _GRAY_INDEX_END; i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				sprintf_s(sz, 128, "d:\\Save\\SliderCc\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgOffset[i]);

				sprintf_s(sz, 128, "d:\\Save\\Slider\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCc[i]);


				sprintf_s(sz, 128, "d:\\Save\\SliderNorm\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCcMax[i]);

				sprintf_s(sz, 128, "d:\\Save\\ResImg\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ResImg[i]);


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
	}

	if (bSave)
	{
		char sz[128];
		for (int i = _GRAY_INDEX_END - 1; i < _GRAY_INDEX_END; i++)
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
		for (int i = _GRAY_INDEX_END - 1; i < _GRAY_INDEX_END; i++)
		{
			snprintf(sz, 128, "d:\\Save\\Learn\\%d.bmp", i);
			m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemplate[i]);
		}

	}

	if (bSave)
	{
		char sz[128];
		for (int i = _GRAY_INDEX_END - 1; i < _GRAY_INDEX_END; i++)
		{
			if (m_nAlgorithmStatus[i])
			{
				snprintf(sz, 128, "d:\\Save\\SliderCc\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgOffset[i]);

				snprintf(sz, 128, "d:\\Save\\Slider\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCc[i]);


				snprintf(sz, 128, "d:\\Save\\SliderNorm\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgSlidCcMax[i]);

				snprintf(sz, 128, "d:\\Save\\ResImg\\%d_%d.bmp", n, i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ResImg[i]);


			}

		}
		snprintf(sz, 128, "d:\\Save\\Cc\\%d.bmp", n);
		m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemp);
		n++;
	}


#endif // defined
	return 1;

}
