
#include "KxCalculateDistance.h"
#include "KxReadXml.h"


CKxCalDistance::CKxCalDistance()
{
	m_nHorDistance = 0;
	m_nVerDistance = 0;
}

CKxCalDistance::~CKxCalDistance()
{

}

bool CKxCalDistance::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "距离标定", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nIsOpenCalPosition);
	if (!nStatus)
	{
		return false;
	}

	if (m_hParameter.m_nIsOpenCalPosition == 0)
	{
		return true;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "真实卡片宽度um", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nActualWidth);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "真实卡片高度um", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nActualHeight);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "卡片采图宽度pixel", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nProcessCardWidth);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "卡片采图高度pixel", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nProcessCardHeight);
	if (!nStatus)
	{
		return false;

	}

	if (m_hParameter.m_nIsOpenCalPosition == 1 || m_hParameter.m_nIsOpenCalPosition == 3)
	{
		/////
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "垂直测量数据", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		std::vector<std::string> v;
		KxXmlFun::SplitString(szResult, v, ",");
		if (v.size() != 12)
		{
			return false;
		}
		else
		{
			m_hParameter.m_ptVerRefCalPosition.x = atoi(v[0].c_str());
			m_hParameter.m_ptVerRefCalPosition.y = atoi(v[1].c_str());

			m_hParameter.m_ptVerCalPosition.x = atoi(v[2].c_str());
			m_hParameter.m_ptVerCalPosition.y = atoi(v[3].c_str());

			m_hParameter.m_rcVerRefKernPostion.left = atoi(v[4].c_str());
			m_hParameter.m_rcVerRefKernPostion.top = atoi(v[5].c_str());
			m_hParameter.m_rcVerRefKernPostion.right = atoi(v[6].c_str());
			m_hParameter.m_rcVerRefKernPostion.bottom = atoi(v[7].c_str());

			m_hParameter.m_rcVerKernPostion.left = atoi(v[8].c_str());
			m_hParameter.m_rcVerKernPostion.top = atoi(v[9].c_str());
			m_hParameter.m_rcVerKernPostion.right = atoi(v[10].c_str());
			m_hParameter.m_rcVerKernPostion.bottom = atoi(v[11].c_str());

		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "垂直测量方向", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nVerSearchDir);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "垂直参考方向", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nVerRefKernSearchDir);
		if (!nStatus)
		{
			return false;
		}
		int nVerStandard, nVerDiff;

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "垂直标准值", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nVerStandard);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "垂直变化范围", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nVerDiff);
		if (!nStatus)
		{
			return false;
		}

		m_hParameter.m_nVerCardEdgeMinHeight = nVerStandard - nVerDiff;
		m_hParameter.m_nVerCardEdgeMaxHeight = nVerStandard + nVerDiff;

		//截取定位核
		m_VerKernImg.Init(m_hParameter.m_rcVerKernPostion.Width(), m_hParameter.m_rcVerKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_VerKernImg, m_hParameter.m_rcVerKernPostion);
		m_VerRefKernImg.Init(m_hParameter.m_rcVerRefKernPostion.Width(), m_hParameter.m_rcVerRefKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion);

		//////////////////////////////////////////////////////////////////////////

	}

	if (m_hParameter.m_nIsOpenCalPosition == 2 || m_hParameter.m_nIsOpenCalPosition == 3)
	{
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "水平测量数据", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		std::vector<std::string> v1;
		KxXmlFun::SplitString(szResult, v1, ",");
		if (v1.size() != 12)
		{
			return false;
		}
		else
		{
			m_hParameter.m_ptHorRefCalPosition.x = atoi(v1[0].c_str());
			m_hParameter.m_ptHorRefCalPosition.y = atoi(v1[1].c_str());

			m_hParameter.m_ptHorCalPosition.x = atoi(v1[2].c_str());
			m_hParameter.m_ptHorCalPosition.y = atoi(v1[3].c_str());

			m_hParameter.m_rcHorRefKernPostion.left = atoi(v1[4].c_str());
			m_hParameter.m_rcHorRefKernPostion.top = atoi(v1[5].c_str());
			m_hParameter.m_rcHorRefKernPostion.right = atoi(v1[6].c_str());
			m_hParameter.m_rcHorRefKernPostion.bottom = atoi(v1[7].c_str());

			m_hParameter.m_rcHorKernPostion.left = atoi(v1[8].c_str());
			m_hParameter.m_rcHorKernPostion.top = atoi(v1[9].c_str());
			m_hParameter.m_rcHorKernPostion.right = atoi(v1[10].c_str());
			m_hParameter.m_rcHorKernPostion.bottom = atoi(v1[11].c_str());

		}



		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "水平参考方向", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nHorRefKernSearchDir);
		if (!nStatus)
		{
			return false;
		}
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "水平测量方向", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nHorSearchDir);
		if (!nStatus)
		{
			return false;
		}

		int nHorStandard, nHorDiff;

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "水平标准值", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nHorStandard);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离设置", "水平变化范围", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nHorDiff);
		if (!nStatus)
		{
			return false;
		}

		m_hParameter.m_nHorCardEdgeMinWidth = nHorStandard - nHorDiff;
		m_hParameter.m_nHorCardEdgeMaxWidth = nHorStandard + nHorDiff;

		//截取定位核
		m_HorKernImg.Init(m_hParameter.m_rcHorKernPostion.Width(), m_hParameter.m_rcHorKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_HorKernImg, m_hParameter.m_rcHorKernPostion);
		m_HorRefKernImg.Init(m_hParameter.m_rcHorRefKernPostion.Width(), m_hParameter.m_rcHorRefKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion);
	}


	if (gTest)
	{
		m_hBaseFun.SaveBMPImage_h("d:\\Test\\m_HorKernImg.bmp", m_HorKernImg);
		m_hBaseFun.SaveBMPImage_h("d:\\Test\\m_HorRefKernImg.bmp", m_HorRefKernImg);
	}
	return true;

}

bool CKxCalDistance::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "DistanceCalibration", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nIsOpenCalPosition);
	if (!nStatus)
	{
		return false;
	}

	if (m_hParameter.m_nIsOpenCalPosition == 0)
	{
		return true;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "RealCardWidth", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nActualWidth);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "RealCardHeight", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nActualHeight);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "ImgCardWidth", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nProcessCardWidth);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "ImgCardHeight", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nProcessCardHeight);
	if (!nStatus)
	{
		return false;

	}

	if (m_hParameter.m_nIsOpenCalPosition == 1 || m_hParameter.m_nIsOpenCalPosition == 3)
	{
		/////
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "VerticalMeasureData", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		std::vector<std::string> v;
		KxXmlFun::SplitString(szResult, v, ",");
		if (v.size() != 12)
		{
			return false;
		}
		else
		{
			m_hParameter.m_ptVerRefCalPosition.x = atoi(v[0].c_str());
			m_hParameter.m_ptVerRefCalPosition.y = atoi(v[1].c_str());

			m_hParameter.m_ptVerCalPosition.x = atoi(v[2].c_str());
			m_hParameter.m_ptVerCalPosition.y = atoi(v[3].c_str());

			m_hParameter.m_rcVerRefKernPostion.left = atoi(v[4].c_str());
			m_hParameter.m_rcVerRefKernPostion.top = atoi(v[5].c_str());
			m_hParameter.m_rcVerRefKernPostion.right = atoi(v[6].c_str());
			m_hParameter.m_rcVerRefKernPostion.bottom = atoi(v[7].c_str());

			m_hParameter.m_rcVerKernPostion.left = atoi(v[8].c_str());
			m_hParameter.m_rcVerKernPostion.top = atoi(v[9].c_str());
			m_hParameter.m_rcVerKernPostion.right = atoi(v[10].c_str());
			m_hParameter.m_rcVerKernPostion.bottom = atoi(v[11].c_str());

		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "VerticalMeasureDirection", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nVerSearchDir);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "VerticalReferDirection", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nVerRefKernSearchDir);
		if (!nStatus)
		{
			return false;
		}
		int nVerStandard, nVerDiff;

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "VerticalStandardValue", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nVerStandard);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "VerticalChangeRange", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nVerDiff);
		if (!nStatus)
		{
			return false;
		}

		m_hParameter.m_nVerCardEdgeMinHeight = nVerStandard - nVerDiff;
		m_hParameter.m_nVerCardEdgeMaxHeight = nVerStandard + nVerDiff;

		//截取定位核
		m_VerKernImg.Init(m_hParameter.m_rcVerKernPostion.Width(), m_hParameter.m_rcVerKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_VerKernImg, m_hParameter.m_rcVerKernPostion);
		m_VerRefKernImg.Init(m_hParameter.m_rcVerRefKernPostion.Width(), m_hParameter.m_rcVerRefKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion);

		//////////////////////////////////////////////////////////////////////////

	}

	if (m_hParameter.m_nIsOpenCalPosition == 2 || m_hParameter.m_nIsOpenCalPosition == 3)
	{
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "HorizontalMeasureData", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		std::vector<std::string> v1;
		KxXmlFun::SplitString(szResult, v1, ",");
		if (v1.size() != 12)
		{
			return false;
		}
		else
		{
			m_hParameter.m_ptHorRefCalPosition.x = atoi(v1[0].c_str());
			m_hParameter.m_ptHorRefCalPosition.y = atoi(v1[1].c_str());

			m_hParameter.m_ptHorCalPosition.x = atoi(v1[2].c_str());
			m_hParameter.m_ptHorCalPosition.y = atoi(v1[3].c_str());

			m_hParameter.m_rcHorRefKernPostion.left = atoi(v1[4].c_str());
			m_hParameter.m_rcHorRefKernPostion.top = atoi(v1[5].c_str());
			m_hParameter.m_rcHorRefKernPostion.right = atoi(v1[6].c_str());
			m_hParameter.m_rcHorRefKernPostion.bottom = atoi(v1[7].c_str());

			m_hParameter.m_rcHorKernPostion.left = atoi(v1[8].c_str());
			m_hParameter.m_rcHorKernPostion.top = atoi(v1[9].c_str());
			m_hParameter.m_rcHorKernPostion.right = atoi(v1[10].c_str());
			m_hParameter.m_rcHorKernPostion.bottom = atoi(v1[11].c_str());

		}



		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "HorizontalReferDirection", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nHorRefKernSearchDir);
		if (!nStatus)
		{
			return false;
		}
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "HorizontalMeasureDirection", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nHorSearchDir);
		if (!nStatus)
		{
			return false;
		}

		int nHorStandard, nHorDiff;

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "HorizontalStandardValue", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nHorStandard);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceSetting", "HorizontalChangeRange", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, nHorDiff);
		if (!nStatus)
		{
			return false;
		}

		m_hParameter.m_nHorCardEdgeMinWidth = nHorStandard - nHorDiff;
		m_hParameter.m_nHorCardEdgeMaxWidth = nHorStandard + nHorDiff;

		//截取定位核
		m_HorKernImg.Init(m_hParameter.m_rcHorKernPostion.Width(), m_hParameter.m_rcHorKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_HorKernImg, m_hParameter.m_rcHorKernPostion);
		m_HorRefKernImg.Init(m_hParameter.m_rcHorRefKernPostion.Width(), m_hParameter.m_rcHorRefKernPostion.Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion);
	}


	if (gTest)
	{
		m_hBaseFun.SaveBMPImage_h("d:\\Test\\m_HorKernImg.bmp", m_HorKernImg);
		m_hBaseFun.SaveBMPImage_h("d:\\Test\\m_HorRefKernImg.bmp", m_HorRefKernImg);
	}
	return true;

}




//从网络中获取参数
bool CKxCalDistance::ReadParaFromNet( unsigned char*& point )
{
	memcpy(&m_hParameter, point, sizeof(Parameter));
	point += sizeof(Parameter);
	CKxBaseFunction hBaseFun;
	if (!hBaseFun.readImgBufFromMemory(m_VerKernImg,    point))
	{
		return false;
	}
	if (!hBaseFun.readImgBufFromMemory(m_VerRefKernImg, point))
	{
		return false;
	}
	if (!hBaseFun.readImgBufFromMemory(m_HorKernImg, point))
	{
		return false;
	}
	if (!hBaseFun.readImgBufFromMemory(m_HorRefKernImg, point))
	{
		return false;
	}

	return true;
}
bool CKxCalDistance::WriteVesion1Para( FILE* fp)    //写入版本1参数
{
	if (fwrite(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nIsOpenCalPosition, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	//垂直参数
	if (fwrite(&m_hParameter.m_ptVerCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcVerKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nVerSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_ptVerRefCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcVerRefKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nVerRefKernSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nVerCardEdgeMinHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nVerCardEdgeMaxHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	//水平参数
	if (fwrite(&m_hParameter.m_ptHorCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcHorKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nHorSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_ptHorRefCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcHorRefKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nHorRefKernSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nHorCardEdgeMinWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nHorCardEdgeMaxWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nSearchExpand, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nActualWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nActualHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nProcessCardWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nProcessCardHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	if ( !m_VerKernImg.Write(fp))
	{
		return false;
	}
	if ( !m_VerRefKernImg.Write(fp))
	{
		return false;
	}
	if ( !m_HorKernImg.Write(fp))
	{
		return false;
	}
	if ( !m_HorRefKernImg.Write(fp))
	{
		return false;
	}

	return true;

}

bool CKxCalDistance::ReadVesion1Para( FILE* fp)    //读取版本1参数
{
	if (fread(&m_hParameter.m_nIsOpenCalPosition, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
    //垂直参数
	if (fread(&m_hParameter.m_ptVerCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcVerKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nVerSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	m_hParameter.m_nVerSearchDir = abs(2 - m_hParameter.m_nVerSearchDir);

	if (fread(&m_hParameter.m_ptVerRefCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcVerRefKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nVerRefKernSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	m_hParameter.m_nVerRefKernSearchDir = abs(2 - m_hParameter.m_nVerRefKernSearchDir);


	if (fread(&m_hParameter.m_nVerCardEdgeMinHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nVerCardEdgeMaxHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	//水平参数
	if (fread(&m_hParameter.m_ptHorCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcHorKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nHorSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	//m_hParameter.m_nHorSearchDir = abs(2 - m_hParameter.m_nHorSearchDir);


	if (fread(&m_hParameter.m_ptHorRefCalPosition, sizeof(kxPoint<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcHorRefKernPostion, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nHorRefKernSearchDir, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	//m_hParameter.m_nHorRefKernSearchDir = abs(2 - m_hParameter.m_nHorRefKernSearchDir);


	if (fread(&m_hParameter.m_nHorCardEdgeMinWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nHorCardEdgeMaxWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	if (fread(&m_hParameter.m_nSearchExpand, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nActualWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nActualHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nProcessCardWidth, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nProcessCardHeight, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	if ( !m_VerKernImg.Read(fp))
	{
		return false;
	}

	if ( !m_VerRefKernImg.Read(fp))
	{
		return false;
	}

	if ( !m_HorKernImg.Read(fp))
	{
		return false;
	}
	if ( !m_HorRefKernImg.Read(fp))
	{
		return false;
	}

	return true;

}


bool CKxCalDistance::Read( FILE*  fp)
{
	if (fread(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_hParameter.m_szVersion, "Distance1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}
}

bool CKxCalDistance::Write( FILE*  fp)
{
	if (strcmp(m_hParameter.m_szVersion, "Distance1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return WriteVesion1Para(fp);
	}
	else
	{
		return false;
	}

}

int CKxCalDistance::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel)
{
	KxCallStatus hCall;
	return Check(buf, nWidth, nHeight, nPitch, nChannel, hCall);
}

int CKxCalDistance::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch,  int nChannel, KxCallStatus& hCall)
{
    m_TmpImg.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, false);
	return Check(m_TmpImg, hCall);
}

int CKxCalDistance::Check(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return Check(ImgCheck, hCall);
}


int CKxCalDistance::Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();
   
	switch( m_hParameter.m_nIsOpenCalPosition )
	{
	case 0:  //未开启
		break;
	case 1:  //垂直标定
		{
			double fFactorY = m_hParameter.m_nActualHeight*1.0/m_hParameter.m_nProcessCardHeight;

			int nDx, nDy;
			kxPoint<int> pt;
			if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerKernImg, m_hParameter.m_rcVerKernPostion, m_hParameter.m_nSearchExpand,
				                        m_hParameter.m_nVerSearchDir, nDx, nDy, hTempCall))
			{
				if (check_sts(hTempCall, "CKxCalDistance_Check_1_m_VerKernImg", hCall))
				{
					return 1;
				}
				pt.setup(m_hParameter.m_ptVerCalPosition.x + nDx, m_hParameter.m_ptVerCalPosition.y + nDy);
			}
			else
			{
				m_nVerDistance = 1;
				return 1; //定位核相似度错误
			}

			kxPoint<int> Verpt;
			if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nSearchExpand,
				                        m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
			{	
				if (check_sts(hTempCall, "CKxCalDistance_Check_1_m_VerRefKernImg", hCall))
				{
					return 2;
				}

				Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
			}
			else
			{
				m_nVerDistance = 1;
				return 2;  //垂直定位核相似度错误
			}

			m_nVerDistance = int(abs(Verpt.y - pt.y) * fFactorY);

			if (m_nVerDistance > m_hParameter.m_nVerCardEdgeMaxHeight || m_nVerDistance < m_hParameter.m_nVerCardEdgeMinHeight)
			{
				return 4;   //距离标定错误
			}
			break;
		}
	case 2: //水平标定
		{
			double fFactorX = m_hParameter.m_nActualWidth*1.0/m_hParameter.m_nProcessCardWidth;

			int nDx, nDy;
			kxPoint<int> pt;
			if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorKernImg, m_hParameter.m_rcHorKernPostion, m_hParameter.m_nSearchExpand,
				m_hParameter.m_nHorSearchDir, nDx, nDy, hTempCall))
			{
				if (check_sts(hTempCall, "CKxCalDistance_Check_2_m_HorKernImg", hCall))
				{
					return 1;
				}

				pt.setup(m_hParameter.m_ptHorCalPosition.x + nDx, m_hParameter.m_ptHorCalPosition.y + nDy);
			}
			else
			{
				m_nHorDistance = 1;
				return 1; //定位核相似度错误
			}

			kxPoint<int> Horpt;
			if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nSearchExpand,
				m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
			{
				if (check_sts(hTempCall, "CKxCalDistance_Check_2_m_HorRefKernImg", hCall))
				{
					return 3;
				}

				Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
			}
			else
			{
				m_nHorDistance = 1;
				return 3;  //水平定位核相似度错误
			}

			m_nHorDistance = int(abs(Horpt.x - pt.x) * fFactorX);

			if (m_nHorDistance > m_hParameter.m_nHorCardEdgeMaxWidth || m_nHorDistance < m_hParameter.m_nHorCardEdgeMinWidth)
			{
				return 4;   //距离标定错误
			}

			break;

		}

	case 3:
		{
			double fFactorY = m_hParameter.m_nActualHeight*1.0/m_hParameter.m_nProcessCardHeight;

			int nDx, nDy;
			kxPoint<int> pt;
			if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerKernImg, m_hParameter.m_rcVerKernPostion, m_hParameter.m_nSearchExpand,
				m_hParameter.m_nVerSearchDir, nDx, nDy, hTempCall))
			{
				if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_VerKernImg", hCall))
				{
					return 1;
				}

				pt.setup(m_hParameter.m_ptVerCalPosition.x + nDx, m_hParameter.m_ptVerCalPosition.y + nDy);
			}
			else
			{
				m_nVerDistance = 1;
				return 1; //定位核相似度错误
			}

			kxPoint<int> Verpt;
			if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nSearchExpand,
				m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
			{
				if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_VerRefKernImg", hCall))
				{
					return 2;
				}

				Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
			}
			else
			{
				m_nVerDistance = 1;
				return 2;  //垂直定位核相似度错误
			}

			m_nVerDistance = int(abs(Verpt.y - pt.y) * fFactorY); 
			
			double fFactorX = m_hParameter.m_nActualWidth*1.0/m_hParameter.m_nProcessCardWidth;

			if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorKernImg, m_hParameter.m_rcHorKernPostion, m_hParameter.m_nSearchExpand,
				m_hParameter.m_nHorSearchDir, nDx, nDy, hTempCall))
			{
				if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_HorKernImg", hCall))
				{
					return 1;
				}

				pt.setup(m_hParameter.m_ptHorCalPosition.x + nDx, m_hParameter.m_ptHorCalPosition.y + nDy);
			}
			else
			{
				m_nHorDistance = 1;
				return 1; //定位核相似度错误
			}

			kxPoint<int> Horpt;
			if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nSearchExpand,
				m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
			{
				if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_HorRefKernImg", hCall))
				{
					return 3;
				}

				Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
			}
			else
			{
				m_nHorDistance = 1;
				return 3;  //水平定位核相似度错误
			}

			m_nHorDistance = int(abs(Horpt.x - pt.x) * fFactorX);
            

			if (m_nHorDistance > m_hParameter.m_nHorCardEdgeMaxWidth || m_nHorDistance < m_hParameter.m_nHorCardEdgeMinWidth
				||m_nVerDistance > m_hParameter.m_nVerCardEdgeMaxHeight || m_nVerDistance < m_hParameter.m_nVerCardEdgeMinHeight)
			{
				return 4;   //距离标定错误
			}

			break;

		}
	default:
		break;

	}

	return 0;
}

int CKxCalDistance::CheckSpecial(const kxCImageBuf& ImgCheck, kxRect<int>& rcPos, kxRect<int>& rcBasePos)
{
	KxCallStatus hCall;
	return CheckSpecial(ImgCheck, rcPos, rcBasePos, hCall);
}


int CKxCalDistance::CheckSpecial(const kxCImageBuf& ImgCheck, kxRect<int>& rcPos, kxRect<int>& rcBasePos, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	switch (m_hParameter.m_nIsOpenCalPosition)
	{
	case 0:  //未开启
		break;
	case 1:  //垂直标定
	{
				 double fFactorY = m_hParameter.m_nActualHeight*1.0 / m_hParameter.m_nProcessCardHeight;

				 int nDx, nDy;
				 kxPoint<int> pt;
				 //if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerKernImg, m_hParameter.m_rcVerKernPostion, m_hParameter.m_nSearchExpand,
					// m_hParameter.m_nVerSearchDir, nDx, nDy, hTempCall))
				 //{
					// if (check_sts(hTempCall, "CKxCalDistance_Check_1_m_VerKernImg", hCall))
					// {
					//	 return 1;
					// }
					// pt.setup(m_hParameter.m_ptVerCalPosition.x + nDx, m_hParameter.m_ptVerCalPosition.y + nDy);
				 //}
				 //else
				 //{
					// return 1; //定位核相似度错误
				 //}
				 int nCloseTop = abs(m_hParameter.m_ptVerCalPosition.y - rcBasePos.top);
				 int nCloseBottom = abs(m_hParameter.m_ptVerCalPosition.y - rcBasePos.bottom);
				 pt.y = nCloseTop < nCloseBottom ? rcPos.top : rcPos.bottom;

				 //kxPoint<int> Verpt;
				 //if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nSearchExpand,
					// m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
				 //{
					// if (check_sts(hTempCall, "CKxCalDistance_Check_1_m_VerRefKernImg", hCall))
					// {
					//	 return 2;
					// }

					// Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
				 //}
				 //else
				 //{
					// m_nVerDistance = 1;
					// return 2;  //垂直定位核相似度错误
				 //}
				 //改成搜边的方式确定参考点位置
				 kxPoint<int> Verpt;
				 if (SearchEdge(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
				 {
					 Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
				 }
				 else
				 {
					 m_nVerDistance = 1;
					 return 2;  //垂直定位核相似度错误
				 }


				 m_nVerDistance = int(abs(Verpt.y - pt.y) * fFactorY);

				 if (m_nVerDistance > m_hParameter.m_nVerCardEdgeMaxHeight || m_nVerDistance < m_hParameter.m_nVerCardEdgeMinHeight)
				 {
					 return 4;   //距离标定错误
				 }
				 break;
	}
	case 2: //水平标定
	{
				double fFactorX = m_hParameter.m_nActualWidth*1.0 / m_hParameter.m_nProcessCardWidth;

				int nDx, nDy;
				kxPoint<int> pt;
				//if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorKernImg, m_hParameter.m_rcHorKernPostion, m_hParameter.m_nSearchExpand,
				//	m_hParameter.m_nHorSearchDir, nDx, nDy, hTempCall))
				//{
				//	if (check_sts(hTempCall, "CKxCalDistance_Check_2_m_HorKernImg", hCall))
				//	{
				//		return 1;
				//	}

				//	pt.setup(m_hParameter.m_ptHorCalPosition.x + nDx, m_hParameter.m_ptHorCalPosition.y + nDy);
				//}
				//else
				//{
				//	return 1; //定位核相似度错误
				//}

				int nCloseLeft = abs(m_hParameter.m_ptHorCalPosition.x - rcBasePos.left);
				int nCloseRight = abs(m_hParameter.m_ptHorCalPosition.x - rcBasePos.right);
				pt.x = nCloseLeft < nCloseRight ? rcPos.left : rcPos.right;


				//kxPoint<int> Horpt;
				//if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nSearchExpand,
				//	m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
				//{
				//	if (check_sts(hTempCall, "CKxCalDistance_Check_2_m_HorRefKernImg", hCall))
				//	{
				//		return 3;
				//	}

				//	Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
				//}
				//else
				//{
				//	m_nHorDistance = 1;
				//	return 3;  //水平定位核相似度错误
				//}
				kxPoint<int> Horpt;
				if (SearchEdge(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
				{
					Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
				}
				else
				{
					m_nHorDistance = 1;
					return 3;  //水平定位核相似度错误
				}


				m_nHorDistance = int(abs(Horpt.x - pt.x) * fFactorX);

				if (m_nHorDistance > m_hParameter.m_nHorCardEdgeMaxWidth || m_nHorDistance < m_hParameter.m_nHorCardEdgeMinWidth)
				{
					return 4;   //距离标定错误
				}

				break;

	}

	case 3:
	{
			  double fFactorY = m_hParameter.m_nActualHeight*1.0 / m_hParameter.m_nProcessCardHeight;

			  int nDx, nDy;
			  kxPoint<int> pt;
			  //if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerKernImg, m_hParameter.m_rcVerKernPostion, m_hParameter.m_nSearchExpand,
				 // m_hParameter.m_nVerSearchDir, nDx, nDy, hTempCall))
			  //{
				 // if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_VerKernImg", hCall))
				 // {
					//  return 1;
				 // }

				 // pt.setup(m_hParameter.m_ptVerCalPosition.x + nDx, m_hParameter.m_ptVerCalPosition.y + nDy);
			  //}
			  //else
			  //{
				 // return 1; //定位核相似度错误
			  //}
			  int nCloseTop = abs(m_hParameter.m_ptVerCalPosition.y - rcBasePos.top);
			  int nCloseBottom = abs(m_hParameter.m_ptVerCalPosition.y - rcBasePos.bottom);
			  pt.y = nCloseTop < nCloseBottom ? rcPos.top : rcPos.bottom;


			  //kxPoint<int> Verpt;
			  //if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nSearchExpand,
				 // m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
			  //{
				 // if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_VerRefKernImg", hCall))
				 // {
					//  return 2;
				 // }

				 // Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
			  //}
			  //else
			  //{
				 // m_nVerDistance = 1;
				 // return 2;  //垂直定位核相似度错误
			  //}
			  //改成搜边的方式确定参考点位置
			  kxPoint<int> Verpt;
			  if (SearchEdge(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
			  {
				  Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
			  }
			  else
			  {
				  m_nVerDistance = 1;
				  return 2;  //垂直定位核相似度错误
			  }


			  m_nVerDistance = int(abs(Verpt.y - pt.y) * fFactorY);




			  double fFactorX = m_hParameter.m_nActualWidth*1.0 / m_hParameter.m_nProcessCardWidth;

			  //if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorKernImg, m_hParameter.m_rcHorKernPostion, m_hParameter.m_nSearchExpand,
				 // m_hParameter.m_nHorSearchDir, nDx, nDy, hTempCall))
			  //{
				 // if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_HorKernImg", hCall))
				 // {
					//  return 1;
				 // }

				 // pt.setup(m_hParameter.m_ptHorCalPosition.x + nDx, m_hParameter.m_ptHorCalPosition.y + nDy);
			  //}
			  //else
			  //{
				 // return 1; //定位核相似度错误
			  //}
			  int nCloseLeft = abs(m_hParameter.m_ptHorCalPosition.x - rcBasePos.left);
			  int nCloseRight = abs(m_hParameter.m_ptHorCalPosition.x - rcBasePos.right);
			  pt.x = nCloseLeft < nCloseRight ? rcPos.left : rcPos.right;

			  //kxPoint<int> Horpt;
			  //if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nSearchExpand,
				 // m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
			  //{
				 // if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_HorRefKernImg", hCall))
				 // {
					//  return 3;
				 // }

				 // Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
			  //}
			  //else
			  //{
				 // m_nHorDistance = 1;
				 // return 3;  //水平定位核相似度错误
			  //}
			  kxPoint<int> Horpt;
			  if (SearchEdge(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
			  {
				  Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
			  }
			  else
			  {
				  m_nHorDistance = 1;
				  return 3;  //水平定位核相似度错误
			  }


			  m_nHorDistance = int(abs(Horpt.x - pt.x) * fFactorX);


			  if (m_nHorDistance > m_hParameter.m_nHorCardEdgeMaxWidth || m_nHorDistance < m_hParameter.m_nHorCardEdgeMinWidth
				  || m_nVerDistance > m_hParameter.m_nVerCardEdgeMaxHeight || m_nVerDistance < m_hParameter.m_nVerCardEdgeMinHeight)
			  {
				  return 4;   //距离标定错误
			  }

			  break;

	}
	default:
		break;

	}

	return 0;
}

int CKxCalDistance::CheckSpecialWithSearchEdge(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return CheckSpecialWithSearchEdge(ImgCheck, hCall);
}

int CKxCalDistance::CheckSpecialWithSearchEdge(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	switch (m_hParameter.m_nIsOpenCalPosition)
	{
	case 0:  //未开启
		break;
	case 1:  //垂直标定
	{
				 double fFactorY = m_hParameter.m_nActualHeight*1.0 / m_hParameter.m_nProcessCardHeight;

				 int nDx, nDy;
				 kxPoint<int> pt;
				 if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerKernImg, m_hParameter.m_rcVerKernPostion, m_hParameter.m_nSearchExpand,
					 m_hParameter.m_nVerSearchDir, nDx, nDy, hTempCall))
				 {
					 if (check_sts(hTempCall, "CKxCalDistance_Check_1_m_VerKernImg", hCall))
					 {
						 return 1;
					 }
					 pt.setup(m_hParameter.m_ptVerCalPosition.x + nDx, m_hParameter.m_ptVerCalPosition.y + nDy);
				 }
				 else
				 {
					 m_nVerDistance = 1;
					 return 1; //定位核相似度错误
				 }


				 //改成搜边的方式确定参考点位置
				 kxPoint<int> Verpt;
				 if (SearchEdge(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
				 {
					  Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
				 }
				 else
				 {
					  m_nVerDistance = 1;
					  return 2;  //垂直定位核相似度错误
				 }
				

				 
				 m_nVerDistance = int(abs(Verpt.y - pt.y) * fFactorY);

				 if (m_nVerDistance > m_hParameter.m_nVerCardEdgeMaxHeight || m_nVerDistance < m_hParameter.m_nVerCardEdgeMinHeight)
				 {
					 return 4;   //距离标定错误
				 }
				 break;
	}
	case 2: //水平标定
	{
				double fFactorX = m_hParameter.m_nActualWidth*1.0 / m_hParameter.m_nProcessCardWidth;

				int nDx, nDy;
				kxPoint<int> pt;
				if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorKernImg, m_hParameter.m_rcHorKernPostion, m_hParameter.m_nSearchExpand,
					m_hParameter.m_nHorSearchDir, nDx, nDy, hTempCall))
				{
					if (check_sts(hTempCall, "CKxCalDistance_Check_2_m_HorKernImg", hCall))
					{
						return 1;
					}

					pt.setup(m_hParameter.m_ptHorCalPosition.x + nDx, m_hParameter.m_ptHorCalPosition.y + nDy);
				}
				else
				{
					m_nHorDistance = 1;
					return 1; //定位核相似度错误
				}

				kxPoint<int> Horpt;


				if (SearchEdge(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
				{
					Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
				}
				else
				{
					m_nHorDistance = 1;
					return 3;  //水平定位核相似度错误
				}

				m_nHorDistance = int(abs(Horpt.x - pt.x) * fFactorX);

				if (m_nHorDistance > m_hParameter.m_nHorCardEdgeMaxWidth || m_nHorDistance < m_hParameter.m_nHorCardEdgeMinWidth)
				{
					return 4;   //距离标定错误
				}

				break;

	}

	case 3:
	{
			  double fFactorY = m_hParameter.m_nActualHeight*1.0 / m_hParameter.m_nProcessCardHeight;

			  int nDx, nDy;
			  kxPoint<int> pt;
			  if (m_hBaseFun.GetImgOffset(ImgCheck, m_VerKernImg, m_hParameter.m_rcVerKernPostion, m_hParameter.m_nSearchExpand,
				  m_hParameter.m_nVerSearchDir, nDx, nDy, hTempCall))
			  {
				  if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_VerKernImg", hCall))
				  {
					  return 1;
				  }

				  pt.setup(m_hParameter.m_ptVerCalPosition.x + nDx, m_hParameter.m_ptVerCalPosition.y + nDy);
			  }
			  else
			  {
				  m_nVerDistance = 1;
				  return 1; //定位核相似度错误
			  }

			  kxPoint<int> Verpt;

			  if (SearchEdge(ImgCheck, m_VerRefKernImg, m_hParameter.m_rcVerRefKernPostion, m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hTempCall))
			  {
				  Verpt.setup(m_hParameter.m_ptVerRefCalPosition.x + nDx, m_hParameter.m_ptVerRefCalPosition.y + nDy);
			  }
			  else
			  {
				  m_nVerDistance = 1;
				  return 2;  //垂直定位核相似度错误
			  }

			  m_nVerDistance = int(abs(Verpt.y - pt.y) * fFactorY);

			  double fFactorX = m_hParameter.m_nActualWidth*1.0 / m_hParameter.m_nProcessCardWidth;

			  if (m_hBaseFun.GetImgOffset(ImgCheck, m_HorKernImg, m_hParameter.m_rcHorKernPostion, m_hParameter.m_nSearchExpand,
				  m_hParameter.m_nHorSearchDir, nDx, nDy, hTempCall))
			  {
				  if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_HorKernImg", hCall))
				  {
					  return 1;
				  }

				  pt.setup(m_hParameter.m_ptHorCalPosition.x + nDx, m_hParameter.m_ptHorCalPosition.y + nDy);
			  }
			  else
			  {
				  m_nHorDistance = 1;
				  return 1; //定位核相似度错误
			  }

			  kxPoint<int> Horpt;

			  if (SearchEdge(ImgCheck, m_HorRefKernImg, m_hParameter.m_rcHorRefKernPostion, m_hParameter.m_nHorRefKernSearchDir, nDx, nDy, hTempCall))
			  {
				  if (check_sts(hTempCall, "CKxCalDistance_Check_3_m_HorRefKernImg", hCall))
				  {
					  return 3;
				  }

				  Horpt.setup(m_hParameter.m_ptHorRefCalPosition.x + nDx, m_hParameter.m_ptHorRefCalPosition.y + nDy);
			  }
			  else
			  {
				  m_nHorDistance = 1;
				  return 3;  //水平定位核相似度错误
			  }

			  m_nHorDistance = int(abs(Horpt.x - pt.x) * fFactorX);


			  if (m_nHorDistance > m_hParameter.m_nHorCardEdgeMaxWidth || m_nHorDistance < m_hParameter.m_nHorCardEdgeMinWidth
				  || m_nVerDistance > m_hParameter.m_nVerCardEdgeMaxHeight || m_nVerDistance < m_hParameter.m_nVerCardEdgeMinHeight)
			  {
				  return 4;   //距离标定错误
			  }

			  break;

	}
	default:
		break;

	}

	return 0;
}


int CKxCalDistance::SearchEdge(const kxCImageBuf& ImgCheck, const kxCImageBuf & ReferImg, kxRect<int>& rcSearch, int nSearchDir, int& nDx, int& nDy)
{
	KxCallStatus hCall;
	return SearchEdge(ImgCheck, ReferImg, rcSearch, nSearchDir, nDx, nDy, hCall);
}


int CKxCalDistance::SearchEdge(const kxCImageBuf& ImgCheck, const kxCImageBuf & ReferImg, kxRect<int>& rcSearch, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();

	if (nSearchDir == _Vertical_Dir)
	{
		m_ImgCopy.Init(rcSearch.Width(), rcSearch.Height(), ImgCheck.nChannel);
		if (ImgCheck.nChannel == _Type_G8)
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C1R(ImgCheck.buf + rcSearch.left + rcSearch.top * ImgCheck.nPitch, ImgCheck.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);
		}
		else
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C3R(ImgCheck.buf + rcSearch.left*3 + rcSearch.top * ImgCheck.nPitch, ImgCheck.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);			
		}
		//m_hBaseFun.SaveBMPImage_h("d:\\123_1.bmp", m_ImgCopy);


		m_hGradient.SingleDirGradientSame(m_ImgCopy, m_ImgGradientDir, 1, 1, 7, 0, hCallInfo);
		if (check_sts(hCallInfo, "SearchEdge__Vertical_Dir_1", hCall))
		{
			return 0;
		}
		m_hBaseFun.KxImageMaxEvery(m_ImgGradientDir, m_ImgMaxGradient);

		//m_hBaseFun.SaveBMPImage_h("d:\\123.bmp", m_ImgMaxGradient);

		m_hBaseFun.KxThreshImage(m_ImgMaxGradient, m_ImgBinary, 20, 255);
		int nLen = rcSearch.Height();
		Ipp32f* pProject = new Ipp32f[nLen];
		m_hBaseFun.KxProjectImage(m_ImgBinary, 1, pProject);
		ippsDivC_32f_I(Ipp32f(rcSearch.Width()), pProject, nLen);
		int nIndy;
		Ipp32f pMax;
		ippsMaxIndx_32f(pProject, nLen, &pMax, &nIndy);


		if (ReferImg.nChannel == _Type_G8)
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C1R(ReferImg.buf , ReferImg.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);
		}
		else
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C3R(ReferImg.buf, ReferImg.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);
		}
		m_hGradient.SingleDirGradientSame(m_ImgCopy, m_ImgGradientDir, 1, 1, 7, 0, hCallInfo);
		if (check_sts(hCallInfo, "SearchEdge__Vertical_Dir_2", hCall))
		{
			return 0;
		}
		//m_hBaseFun.SaveBMPImage_h("d:\\123_2.bmp", m_ImgCopy);

		m_hBaseFun.KxImageMaxEvery(m_ImgGradientDir, m_ImgMaxGradient);
		m_hBaseFun.KxThreshImage(m_ImgMaxGradient, m_ImgBinary, 20, 255);
		m_hBaseFun.KxProjectImage(m_ImgBinary, 1, pProject);
		ippsDivC_32f_I(Ipp32f(rcSearch.Width()), pProject, nLen);
		int nIndy1;
		ippsMaxIndx_32f(pProject, nLen, &pMax, &nIndy1);

		nDx = 0;
		nDy = (  nIndy - nIndy1);

		delete []pProject;
		
	}
	else if (nSearchDir == _Horizontal_Dir)
	{
		m_ImgCopy.Init(rcSearch.Width(), rcSearch.Height(), ImgCheck.nChannel);
		if (ImgCheck.nChannel == _Type_G8)
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C1R(ImgCheck.buf + rcSearch.left + rcSearch.top * ImgCheck.nPitch, ImgCheck.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);
		}
		else
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C3R(ImgCheck.buf + rcSearch.left * 3 + rcSearch.top * ImgCheck.nPitch, ImgCheck.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);
		}

		//m_hBaseFun.SaveBMPImage_h("d:\\1234_1.bmp", m_ImgCopy);

		m_hGradient.SingleDirGradientSame(m_ImgCopy, m_ImgGradientDir, 0, 1, 7, 0, hCallInfo);
		if (check_sts(hCallInfo, "SearchEdge__Horizontal_Dir_1", hCall))
		{
			return 0;
		}

		m_hBaseFun.KxImageMaxEvery(m_ImgGradientDir, m_ImgMaxGradient);
		m_hBaseFun.KxThreshImage(m_ImgMaxGradient, m_ImgBinary, 20, 255);
		int nLen = rcSearch.Width();
		Ipp32f* pProject = new Ipp32f[nLen];
		m_hBaseFun.KxProjectImage(m_ImgBinary, 0, pProject);
		ippsDivC_32f_I(Ipp32f(rcSearch.Height()), pProject, nLen);
		int nIndx;
		Ipp32f pMax;
		ippsMaxIndx_32f(pProject, nLen, &pMax, &nIndx);


		if (ReferImg.nChannel == _Type_G8)
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C1R(ReferImg.buf, ReferImg.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);
		}
		else
		{
			IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
			ippiCopy_8u_C3R(ReferImg.buf, ReferImg.nPitch, m_ImgCopy.buf, m_ImgCopy.nPitch, roiSize);
		}
		m_hGradient.SingleDirGradientSame(m_ImgCopy, m_ImgGradientDir, 0, 1, 7, 0, hCallInfo);
		if (check_sts(hCallInfo, "SearchEdge__Horizontal_Dir_2_", hCall))
		{
			return 0;
		}

		//m_hBaseFun.SaveBMPImage_h("d:\\1234_2.bmp", m_ImgCopy);


		m_hBaseFun.KxImageMaxEvery(m_ImgGradientDir, m_ImgMaxGradient);
		m_hBaseFun.KxThreshImage(m_ImgMaxGradient, m_ImgBinary, 20, 255);
		m_hBaseFun.KxProjectImage(m_ImgBinary, 0, pProject);
		ippsDivC_32f_I(Ipp32f(rcSearch.Height()), pProject, nLen);
		int nIndx1;
		ippsMaxIndx_32f(pProject, nLen, &pMax, &nIndx1);

		nDx = (nIndx - nIndx1);
		nDy = 0;

		delete[]pProject;


	}
	else
	{
		int nStatus = m_hBaseFun.GetImgOffset(ImgCheck, ReferImg, rcSearch, 40, m_hParameter.m_nVerRefKernSearchDir, nDx, nDy, hCallInfo);
		if (check_sts(hCallInfo, "SearchEdge_", hCall))
		{
			return 0;
		}
	}
	return 1;

}


