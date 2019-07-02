
#include "KxSuperPrecisionCheck.h"
#include "KxReadXml.h"


CKxSuperPrecisionCheck::CKxSuperPrecisionCheck()
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
}

CKxSuperPrecisionCheck::~CKxSuperPrecisionCheck()
{

}

bool CKxSuperPrecisionCheck::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
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


	return true;

}

bool CKxSuperPrecisionCheck::ReadParaXml(const char* filePath)
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

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "检查设置", "边缘框", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcEdgeRect);
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



bool CKxSuperPrecisionCheck::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
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


	return true;

}

bool CKxSuperPrecisionCheck::ReadParaXmlinEnglish(const char* filePath)
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

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CheckSetting", "EdgeBox", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcEdgeRect);
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



bool CKxSuperPrecisionCheck::ReadVesion1Para(FILE* fp)    //读取版本1参数
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

bool CKxSuperPrecisionCheck::ReadVesion2Para(FILE* fp)    //读取版本2参数
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

bool CKxSuperPrecisionCheck::ReadVesion3Para(FILE* fp)    //读取版本3参数
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




bool CKxSuperPrecisionCheck::ReadPara(FILE*  fp)
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


bool CKxSuperPrecisionCheck::Read(FILE*  fp)
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

bool CKxSuperPrecisionCheck::WriteVesion1Para(FILE* fp)    //写入版本1参数
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



bool CKxSuperPrecisionCheck::WritePara(FILE*  fp)
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

bool CKxSuperPrecisionCheck::Write(FILE*  fp)
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
int CKxSuperPrecisionCheck::ReadParaFromNet(unsigned char*& point)
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

int CKxSuperPrecisionCheck::LoadTemplateImg(const char* lpszFile)
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

int CKxSuperPrecisionCheck::SaveTemplateImg(const char* lpszFile)
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


int CKxSuperPrecisionCheck::SaveLearnTemplateImg(const char* lpszFile)
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

int CKxSuperPrecisionCheck::LoadLearnTemplateImg(const char* lpszFile)
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


int CKxSuperPrecisionCheck::ReadLocalLearnImg(FILE* fp)
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



int CKxSuperPrecisionCheck::WriteLocalLearnImg(FILE* fp)
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

int CKxSuperPrecisionCheck::ReadImg(FILE* fp)
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

int CKxSuperPrecisionCheck::WriteImg(FILE* fp)
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
int  CKxSuperPrecisionCheck::ProcessSurfaceWarpImages(KxCallStatus& hCall)
{
	hCall.Clear();
	//2张原始分辨率大小的水平垂直梯度图、4张4*4压缩分辨率下的梯度图、1张压缩分辨率下合成梯度图
	int nAlgorithmOpen[_Max_Layer];
	KxCallStatus hCallInfo[_Max_Layer];
	for (int i = 0; i < _Max_Layer; i++)
	{
		nAlgorithmOpen[i] = m_nAlgorithmStatus[i];
		hCallInfo[i].Clear();
	}
	if (m_nLearnStatus)
	{
		nAlgorithmOpen[_SCRATCH_INDEX_END] = 1;
		nAlgorithmOpen[_SCRATCH_INDEX_END + 1] = 1;
	}

	//原图水平垂直的单向梯度\压缩图上的黑白模式下4个方向的单向梯度
	parallel_for(blocked_range<int>(0, _Max_Layer - 1),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		{
			if (nAlgorithmOpen[i])
			{
				m_ImgCheck[i].Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight, m_ExtendWarpImg.nChannel);
				kxRect<int> rc;
				rc.setup(0, 0, m_ExtendWarpImg.nWidth - 1, m_ExtendWarpImg.nHeight - 1);
				m_hBaseFun[i].KxCopyImage(m_ExtendWarpImg, m_ImgCheck[i], rc, hCallInfo[i]);
				break;
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



	return 1;
}

//学习图像
int  CKxSuperPrecisionCheck::LearnTemplate(KxCallStatus& hCall)
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
	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		nModelProcessSz[i] = _1X1;  //灰度检查
	}

	int nAlgorithmOpen[_Max_Layer];
	for (int i = 0; i < _Max_Layer; i++)
	{
		nAlgorithmOpen[i] = m_nAlgorithmStatus[i];
	}
	if (m_nLearnStatus)
	{
		nAlgorithmOpen[_SCRATCH_INDEX_END] = 1;
		nAlgorithmOpen[_SCRATCH_INDEX_END + 1] = 1;
	}


	parallel_for(blocked_range<int>(0, _Max_Layer),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		{
			if (nAlgorithmOpen[i])
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
		if (nAlgorithmOpen[i])
		{
			status = (status && m_bIsLearnCompleted[i]);
		}
		//else
		//{
		//	m_ImgTemplate[i].Init(1, 1);
		//	memset(m_ImgTemplate[i].buf, 0, m_ImgTemplate[i].nPitch * m_ImgTemplate[i].nHeight);
		//}
	}

	//m_ImgTemplate[_Max_Layer].Init(1, 1);
	//memset(m_ImgTemplate[_Max_Layer].buf, 0, m_ImgTemplate[_Max_Layer].nPitch * m_ImgTemplate[_Max_Layer].nHeight);

	if (status)
	{
		m_bAllCompleted = true;
		m_hParameter.m_nIsLearn = 0;

		m_nModelNumber = _Max_Layer + 1;

		for (int k = 0; k < _Max_Layer + 1; k++)
		{
			if (m_ImgTemplate[k].nWidth == 0 || m_ImgTemplate[k].nHeight == 0)
			{

			}
			else
			{
				m_ImgLearnTemplate[k].Init(m_ImgTemplate[k].nWidth, m_ImgTemplate[k].nHeight, m_ImgTemplate[k].nChannel);
				kxRect<int> rc;
				rc.setup(0, 0, m_ImgTemplate[k].nWidth - 1, m_ImgTemplate[k].nHeight - 1);
				m_hFun.KxCopyImage(m_ImgTemplate[k], m_ImgLearnTemplate[k], rc);
			}
		}

		m_nLearnStatus = 0;
	}

	return 1;
}



//残差分析并且合并为一张残差图
int CKxSuperPrecisionCheck::ResidualsAnalys(KxCallStatus& hCall)
{
	hCall.Clear();
	for (int i = 0; i < _Max_Layer; i++)
	{
		if (m_nAlgorithmStatus[i])
		{
			m_ImgSlidCc[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight, m_ImgCheck[i].nChannel);
			m_ImgSliderNorm[i].Init(m_ImgCheck[i].nWidth, m_ImgCheck[i].nHeight);
			m_ImgOffset[i].Init(m_ImgSliderNorm[i].nWidth, m_ImgSliderNorm[i].nHeight);
			m_ImgSlidCcMax[i].Init(m_ExtendWarpImg.nWidth, m_ExtendWarpImg.nHeight);
		}
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

	for (int i = _SCRATCH_INDEX_END; i < _GRAY_INDEX_END; i++)
	{
		nGridXY[i] = 40;
		nSpeckleMaxSize[i] = 1;
	}
	nOffset[_SCRATCH_INDEX_END] = m_nHighOffset;
	nOffset[_SCRATCH_INDEX_END + 1] = m_nLowOffset;


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
				m_hBaseFun[i].KxSubCImage(m_ImgSliderNorm[i], m_ResImg[i], nOffset[i]);


				//kxRect<int> rc;
				//rc.setup(0, 0, m_ImgOffset[i].nWidth - 1, m_ImgOffset[i].nHeight - 1);
				//m_hBaseFun[i].KxCopyImage(m_ImgOffset[i], m_ImgSlidCcMax[i], rc);
				//去掉噪点
				//m_hBaseFun[i].KxFilterSpeckles(m_ImgSlidCcMax[i], m_ResImg[i], nSpeckleMaxSize[i], hCallInfo[i]);

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

	//图像掩膜
	m_hImgMask.Mask(m_ImgTemp, m_LargeImg, m_hParameter.m_rcEdgeRect, 0, hCallInfo[0]);

	if (check_sts(hCallInfo[0], "", hCall))
	{
		return 0;
	}

	//多边形掩膜
	m_hPolygonMask.Mask(m_ImgTemp, m_LargeImg, m_hParameter.m_rcEdgeRect, 0, hCallInfo[0]);

	if (check_sts(hCallInfo[0], "", hCall))
	{
		return 0;
	}


	return 1;

}


int CKxSuperPrecisionCheck::ConnectAnalysis(const kxCImageBuf& SrcImg, KxCallStatus& hCall)
{
	int nMergeSize = 1;

	int nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, nMergeSize, 0, hCall);

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
		m_hResult.m_hBlobInfo[i].m_nDots = pObj.m_nDots;
		m_hResult.m_hBlobInfo[i].m_nEnergy = pObj.m_nEnergy / 10;
		kxRect<int> rc;
		kxRect<int> rcDst;
		rc.setup(pObj.m_rc.left/2, pObj.m_rc.top/2, pObj.m_rc.right/2, pObj.m_rc.bottom/2);
		m_hResult.m_hBlobInfo[i].m_nLeftSmall = rc.left;
		m_hResult.m_hBlobInfo[i].m_nTopSmall = rc.top;

		rc.offset(rcEdge.left, rcEdge.top);
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


		m_hResult.m_hBlobInfo[i].m_nWHRatio = int(pObj.m_fRatio + 0.5);
		m_hResult.m_hBlobInfo[i].m_nMinRectWidth = int(pObj.m_nMinRectWidth);
		m_hResult.m_hBlobInfo[i].m_nMinRectHeight = int(pObj.m_nMinRectHeight);

		m_hResult.m_hBlobInfo[i].m_nAvgEnergy = pObj.m_nEnergy / pObj.m_nDots;
		m_hResult.m_hBlobInfo[i].m_nAvgArea = int(pObj.m_fSumEnergy / 25);  //总能量

	}

	m_hResult.m_nStatus = _Check_Err;

	return 1;
}

int CKxSuperPrecisionCheck::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex)
{
	KxCallStatus hCall;
	return SingleDefectLearn(ImgCheck, nBlobIndex, hCall);
}
//单个缺陷学习
int CKxSuperPrecisionCheck::SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_WarpImg, hCallInfo);
	if (check_sts(hCallInfo, "SingleDefectLearn_", hCall) || nStatus != 0)
	{
		return 0;
	}


	CopyEdges(ImgCheck, m_WarpImg, m_LargeImg, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);


	//发大图像两倍
	m_ExtendWarpImg.Init(m_LargeImg.nWidth * 2, m_LargeImg.nHeight * 2, m_LargeImg.nChannel);
	m_hFun.KxResizeImage(m_LargeImg, m_ExtendWarpImg, 1, hCallInfo);
	if (check_sts(hCallInfo, "SingleDefectLearn_", hCall))
	{
		return 0;
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

	int nMergeSize = 1;

	nStatus = m_hBlobAnaly.ToBlobParallel(m_ImgTemp, CKxBlobAnalyse::_SORT_BYENERGY, _Max_BlobCount, nMergeSize, 0);
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
	rcBig.setup(pObj.m_rc.left, pObj.m_rc.top, pObj.m_rc.right, pObj.m_rc.bottom);


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
			if (rcLearn[i].top < 0 || rcLearn[i].left < 0 || rcLearn[i].right > m_ImgCheck[i].nWidth - 1 || rcLearn[i].bottom > m_ImgCheck[i].nHeight - 1
				|| m_ImgLearnTemplate[i].nWidth <= 0 || m_ImgLearnTemplate[i].nHeight <= 0 || m_ImgLearnTemplate[i].buf == NULL)
			{
				return 0;
			}

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
			if (rcLearn[i].top < 0 || rcLearn[i].left < 0 || rcLearn[i].right > m_ImgCheck[i].nWidth - 1 || rcLearn[i].bottom > m_ImgCheck[i].nHeight - 1
				|| m_ImgLearnTemplate[i].nWidth <= 0 || m_ImgLearnTemplate[i].nHeight <= 0 || m_ImgLearnTemplate[i].buf == NULL)
			{
				return 0;
			}

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
	//m_ImgLearnTemplate[_Max_Layer].Init(m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nWidth, m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nHeight, m_ImgLearnTemplate[_SAMLLDOTS_INDEX_END].nChannel);
	//ippsSet_8u(0, m_ImgLearnTemplate[_Max_Layer].buf, m_ImgLearnTemplate[_Max_Layer].nPitch * m_ImgLearnTemplate[_Max_Layer].nHeight);
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


int CKxSuperPrecisionCheck::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6])
{
	KxCallStatus hCall;
	return ShowResidualsImage(SrcImg, DstImg, nOffset, hCall);
}
//待检测图像的残差图显示
//残差图始终是一张彩色图像,
//蓝色表示检小点残差、绿色表示检细线残差、红色表示检划伤残差、黑色表示检缺失残差、白色表示
int CKxSuperPrecisionCheck::ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6], KxCallStatus& hCall)
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

	AnalysisCheckStatus(SrcImg.nChannel);

	int nStatus = m_hWarpStrech.CheckParallel(SrcImg, m_WarpImg, hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage_", hCall) || nStatus != 0)
	{
		return 0;
	}
	CopyEdges(SrcImg, m_WarpImg, m_LargeImg, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);


	//发大图像两倍
	m_ExtendWarpImg.Init(m_LargeImg.nWidth * 2, m_LargeImg.nHeight * 2, m_LargeImg.nChannel);
	m_hFun.KxResizeImage(m_LargeImg, m_ExtendWarpImg, 1, hCallInfo);
	if (check_sts(hCallInfo, "ShowResidualsImage__", hCall))
	{
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
	m_ResizeImg.Init(m_ColorImg.nWidth / 1, m_ColorImg.nHeight / 1, m_ColorImg.nChannel);
	m_hFun.KxResizeImage(m_ColorImg, m_ResizeImg, KxSuper);

	SetMask(m_ResizeImg, m_ImgTemp, 1, 255);
	////首先获得检小点的残差图
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
		m_hImgMask.Mask(m_ResidImg[i], m_LargeImg, m_hParameter.m_rcEdgeRect, 0);
		m_hPolygonMask.Mask(m_ResidImg[i], m_LargeImg, m_hParameter.m_rcEdgeRect, 0);
		m_CompositionImg[i].Init(m_ResizeImg.nWidth, m_ResizeImg.nHeight, m_ResizeImg.nChannel);
		AddResidualImage(m_ResidImg[i], m_ResizeImg, i, m_CompositionImg[i]);
		m_hFun.KxAddImage(m_CompositionImg[i], m_AddImage);


	}

	//保留残差处
	SetMask(m_AddImage, m_ImgTemp, 0, 0);
	//保留背景处
	SetMask(m_CompositionImg[2], m_ImgTemp, 1, 255);
	m_hFun.KxAddImage(m_CompositionImg[2], m_AddImage);


	//检缺失残差图，贴为黑色
	//m_ResizeImg1.Init(SrcImg.nWidth, SrcImg.nHeight, 3);
	//IppiSize roi = { SrcImg.nWidth, SrcImg.nHeight };
	//if (_Type_G8 == SrcImg.nChannel)
	//{
	//	ippiDup_8u_C1C3R(SrcImg.buf, SrcImg.nPitch, m_ResizeImg1.buf, m_ResizeImg1.nPitch, roi);
	//}
	//else
	//{
	//	ippiCopy_8u_C3R(SrcImg.buf, SrcImg.nPitch, m_ResizeImg1.buf, m_ResizeImg1.nPitch, roi);
	//}
	//DstImg.Init(m_ResizeImg1.nWidth*2, m_ResizeImg1.nHeight*2, m_ResizeImg1.nChannel);
	//m_hFun.KxResizeImage(m_ResizeImg1, DstImg);


	IppiSize Roi = { m_AddImage.nWidth, m_AddImage.nHeight };

	if (m_nAlgorithmStatus[_Max_Layer - 1])
	{
		m_hImgMask.Mask(m_ResidImg[3], m_LargeImg, m_hParameter.m_rcEdgeRect, 0);
		m_hPolygonMask.Mask(m_ResidImg[3], m_LargeImg, m_hParameter.m_rcEdgeRect, 0);
		SetMask(m_AddImage, m_ResidImg[3], 1, 255);
	}

	//kxRect<int> rcDst;
	//m_hWarpStrech.GetRightPos(m_hParameter.m_rcEdgeRect, rcDst);

	//if (rcDst.left + rcDst.Width() > DstImg.nWidth /2- 1 || rcDst.top + rcDst.Height() > DstImg.nHeight/2 - 1 || rcDst.left < 0 || rcDst.top < 0)
	//{
	//	rcDst = m_hParameter.m_rcEdgeRect;
	//}

	//Roi.width = min(DstImg.nWidth - rcDst.left*2 - 1, Roi.width);
	//Roi.height = min(DstImg.nHeight - rcDst.top*2 - 1, Roi.height);


	DstImg.Init(m_AddImage.nWidth, m_AddImage.nHeight, m_AddImage.nChannel);
	kxRect<int> rc;
	rc.setup(0, 0, m_AddImage.nWidth - 1, m_AddImage.nHeight - 1);
	m_hFun.KxCopyImage(m_AddImage, DstImg, rc);


	return 1;
}

int CKxSuperPrecisionCheck::SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate)
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
int CKxSuperPrecisionCheck::AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg)
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
int CKxSuperPrecisionCheck::CopyEdges(const kxCImageBuf& SrcImg, const kxCImageBuf CopyImg, kxCImageBuf& DstImg, kxRect<int>& rcEdge, kxRect<int>& rcCopy)
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

int CKxSuperPrecisionCheck::AnalysisCheckStatus(int nImageType)
{
	for (int i = 0; i < _Max_Layer; i++)
	{
		m_nAlgorithmStatus[i] = 0;
	}

	//灰度检低
	if (m_hParameter.m_nGrayLowOffset < _MAX_OFFSET)
	{
		m_nLowOffset = m_hParameter.m_nGrayLowOffset * 2;
		m_nAlgorithmStatus[_SCRATCH_INDEX_END+1] = 1;
	}

	//灰度检高
	if (m_hParameter.m_nGrayOffset < _MAX_OFFSET)
	{
		m_nHighOffset = m_hParameter.m_nGrayOffset * 2;
		m_nAlgorithmStatus[_SCRATCH_INDEX_END] = 1;
	}


	if (m_hParameter.m_nIsLearn)
	{
		m_nLearnStatus = 1;
	}

	return 1;
}

int CKxSuperPrecisionCheck::Check(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return Check(ImgCheck, hCall);
}

int CKxSuperPrecisionCheck::Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	hCall.Clear();
	tick_count tbb_start, tbb_end;
	m_hResult.Clear();
	//task_scheduler_init init;
	//firstly, accurate correction a image
	tbb_start = tick_count::now();

	if (m_hCalDistance.Check(ImgCheck, hCall))
	{
		m_hCalDistance.GetHorVerDistance(m_hResult.m_nDistanceX, m_hResult.m_nDistanceY);
		m_hResult.m_nStatus = _Distance_Err;
		return 0;
	}

	int nStatus = m_hWarpStrech.CheckParallel(ImgCheck, m_WarpImg, hCall);
	if (nStatus)
	{
		m_hResult.m_nStatus = _Similarity_Err;
		m_hResult.m_nKernIndex = nStatus - 1;
		return 0;
	}
	CopyEdges(ImgCheck, m_WarpImg, m_LargeImg, m_hParameter.m_rcEdgeRect, m_hWarpStrech.GetParameter().m_rcCheckArea);

	tbb_end = tick_count::now();
	//printf("-----warp process cost : %f ms\n", (tbb_end - tbb_start).seconds() * 1000);
	//发大图像两倍
	m_ExtendWarpImg.Init(m_LargeImg.nWidth * 2, m_LargeImg.nHeight * 2, m_LargeImg.nChannel);
	m_hFun.KxResizeImage(m_LargeImg, m_ExtendWarpImg, 1, hCallInfo);
	if (check_sts(hCallInfo, "放大中_", hCall))
	{
		SetCheckExceptionStatus(ImgCheck);
		return 0;
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

	ConnectAnalysis(ImgCheck, hCallInfo);
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




int CKxSuperPrecisionCheck::SetCheckExceptionStatus(const kxCImageBuf& ImgCheck)
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


int CKxSuperPrecisionCheck::SaveMidImage(bool bSave)
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
			if (m_nAlgorithmStatus[i])
			{
				sprintf_s(sz, 128, "d:\\Save\\Learn\\%d.bmp", i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemplate[i]);
			}
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
			if (m_nAlgorithmStatus[i])
			{
				snprintf(sz, 128, "d:\\Save\\Learn\\%d.bmp", i);
				m_hBaseFun[0].SaveBMPImage_h(sz, m_ImgTemplate[i]);
			}
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
