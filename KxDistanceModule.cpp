
#include "KxDistanceModule.h"
#include "KxReadXml.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/partitioner.h"
using namespace tbb;

CKxDistanceModule::CKxDistanceModule()
{

}

CKxDistanceModule::~CKxDistanceModule()
{

}

bool CKxDistanceModule::ReadXml(const char* filePath)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	//载入底板图像
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "全局", "底板路径", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	std::string szDir = KxXmlFun::UTF8_To_string(szResult);
	m_hBaseFun[0].LoadBMPImage_h(szDir.c_str(), m_ImgBase);
	if (m_ImgBase.nWidth == 0 && m_ImgBase.nHeight == 0)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "相机分辨率标定", "标定物水平像素数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fProcessWidth);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "相机分辨率标定", "标定物水平实际大小", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fActualWidth);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "相机分辨率标定", "标定物垂直像素数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fProcessHeight);
	if (!nStatus)
	{
		return false;
	}
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "相机分辨率标定", "标定物垂直实际大小", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fActualHeight);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "距离测量参数", "测量组数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nGroup);
	if (!nStatus)
	{
		return false;
	}


	if (m_hParameter.m_nGroup == 0)
	{
		return true;
	}

	std::string MainTitle = "距离测量参数";

	for (int i = 0; i < m_hParameter.m_nGroup; i++)
	{
		//-------读取是否打开检查-------//
		char szTittle[128];
		sprintf_s(szTittle, 128, "测量%d", i + 1);

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "打开", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_hMeasure[i].m_nIsOpen);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "名称", szResult);
		if (!nSearchStatus)
		{
			return false;
		}


		std::string szName = KxXmlFun::UTF8_To_string(szResult);
		m_hParameter.m_hMeasure[i].m_szName = szName;


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "参考点搜索定位类型", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nRefSearchType);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "参考点位置信息", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToMulKxRect(szResult, 2, m_hParameter.m_hMeasure[i].m_rcRefInfo);
		if (!nStatus)
		{
			return false;
		}

		//自动确定参考点的搜索方向
		kxRect<int> rc1, rc2;
		kxRect<int> rcSearch;
		rcSearch = m_hParameter.m_hMeasure[i].m_rcRefInfo[0];
		if (m_hParameter.m_hMeasure[i].m_nRefSearchType == _SEARCH_VERTICAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Width()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nRefSearchType, i, pTemplateProject);

			rc1.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[1].left,m_hParameter.m_hMeasure[i].m_rcRefInfo[0].bottom);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[1].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[0].right,m_hParameter.m_hMeasure[i].m_rcRefInfo[0].bottom);

			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcRefInfo[1].left - m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left;
			ippsSum_32f(pTemplateProject, rc1.Width(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Width(), &sum2, ippAlgHintFast);

			m_hParameter.m_hMeasure[i].m_nRefSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Width() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nRefSearchThresh = (int)sum;


			delete[] pTemplateProject;
		}
		if (m_hParameter.m_hMeasure[i].m_nRefSearchType == _SEARCH_HORIZONTAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Height()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nRefSearchType, i, pTemplateProject);
			
			rc1.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[0].right, m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[0].right,m_hParameter.m_hMeasure[i].m_rcRefInfo[0].bottom);

			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top - m_hParameter.m_hMeasure[i].m_rcRefInfo[0].top;

			ippsSum_32f(pTemplateProject, rc1.Height(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Height(), &sum2, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nRefSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Height() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nRefSearchThresh = (int)sum;

			delete[] pTemplateProject;
		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "测量点搜索定位类型", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nMeasureSearchType);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "测量点位置信息", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToMulKxRect(szResult, 2, m_hParameter.m_hMeasure[i].m_rcMeasureInfo);
		if (!nStatus)
		{
			return false;
		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "测量方向", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nMeasureDirection);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "固定参考点", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToBool(szResult, m_hParameter.m_hMeasure[i].m_nFixed);
		if (!nStatus)
		{
			return false;
		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "搜索范围", szResult);
		if (nSearchStatus)
		{
			nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nSearchExtend);
			if (!nStatus)
			{
				return false;
			}

		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "标准值", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_hMeasure[i].m_fStandardValue);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "误差值", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_hMeasure[i].m_fErrorValue);
		if (!nStatus)
		{
			return false;
		}


		//自动确定测量点的搜索方向
		//kxRect<int> rc1, rc2;
		//kxRect<int> rcSearch;
		rcSearch = m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0];
		if (m_hParameter.m_hMeasure[i].m_nMeasureSearchType == _SEARCH_VERTICAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Width()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nMeasureSearchType, i, pTemplateProject);

			rc1.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].bottom);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].right, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].bottom);


			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].left - m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left;

			ippsSum_32f(pTemplateProject, rc1.Width(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Width(), &sum2, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Width() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchThresh = (int)sum;


			delete[] pTemplateProject;
		}
		if (m_hParameter.m_hMeasure[i].m_nMeasureSearchType == _SEARCH_HORIZONTAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Height()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nMeasureSearchType, i, pTemplateProject);



			rc1.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].right, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].right, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].bottom);

			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top - m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].top;

			ippsSum_32f(pTemplateProject, rc1.Height(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Height(), &sum2, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Height() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchThresh = (int)sum;

			delete[] pTemplateProject;
		}


		


	}

	return true;

}

bool CKxDistanceModule::ReadXmlinEnglish(const char* filePath)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	//载入底板图像
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "Global", "BaseImgPath", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	std::string szDir = KxXmlFun::UTF8_To_string(szResult);
	m_hBaseFun[0].LoadBMPImage_h(szDir.c_str(), m_ImgBase);
	if (m_ImgBase.nWidth == 0 && m_ImgBase.nHeight == 0)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CameraCalibration", "HorizontalPixels", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fProcessWidth);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CameraCalibration", "HorizontalActual", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fActualWidth);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CameraCalibration", "VerticalPixels", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fProcessHeight);
	if (!nStatus)
	{
		return false;
	}
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "CameraCalibration", "VerticalActual", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_fActualHeight);
	if (!nStatus)
	{
		return false;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "DistanceMeasureParams", "Groups", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nGroup);
	if (!nStatus)
	{
		return false;
	}


	if (m_hParameter.m_nGroup == 0)
	{
		return true;
	}

	std::string MainTitle = "DistanceMeasureParams";

	for (int i = 0; i < m_hParameter.m_nGroup; i++)
	{
		//-------读取是否打开检查-------//
		char szTittle[128];
		sprintf_s(szTittle, 128, "Measure%d", i + 1);

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "IsOpen", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nIsOpen);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "Name", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		m_hParameter.m_hMeasure[i].m_szName = szResult;

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "ReferenceSearchType", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nRefSearchType);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "ReferencePosInfo", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToMulKxRect(szResult, 2, m_hParameter.m_hMeasure[i].m_rcRefInfo);
		if (!nStatus)
		{
			return false;
		}


		//自动确定参考点的搜索方向
		kxRect<int> rc1, rc2;
		kxRect<int> rcSearch;
		rcSearch = m_hParameter.m_hMeasure[i].m_rcRefInfo[0];
		if (m_hParameter.m_hMeasure[i].m_nRefSearchType == _SEARCH_VERTICAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Width()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nRefSearchType, i, pTemplateProject);

			rc1.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[1].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[0].bottom);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[1].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[0].right, m_hParameter.m_hMeasure[i].m_rcRefInfo[0].bottom);

			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcRefInfo[1].left - m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left;
			ippsSum_32f(pTemplateProject, rc1.Width(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Width(), &sum2, ippAlgHintFast);

			m_hParameter.m_hMeasure[i].m_nRefSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Width() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nRefSearchThresh = (int)sum;


			delete[] pTemplateProject;
		}
		if (m_hParameter.m_hMeasure[i].m_nRefSearchType == _SEARCH_HORIZONTAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Height()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nRefSearchType, i, pTemplateProject);

			rc1.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[0].right, m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcRefInfo[0].left, m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcRefInfo[0].right, m_hParameter.m_hMeasure[i].m_rcRefInfo[0].bottom);

			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcRefInfo[1].top - m_hParameter.m_hMeasure[i].m_rcRefInfo[0].top;

			ippsSum_32f(pTemplateProject, rc1.Height(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Height(), &sum2, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nRefSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Height() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nRefSearchThresh = (int)sum;

			delete[] pTemplateProject;
		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "MeasureSearchType", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nMeasureSearchType);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "MeasurePosInfo", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToMulKxRect(szResult, 2, m_hParameter.m_hMeasure[i].m_rcMeasureInfo);
		if (!nStatus)
		{
			return false;
		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "MeasureDirection", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nMeasureDirection);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "IsReferencePosFixed", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nFixed);
		if (!nStatus)
		{
			return false;
		}


		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "SearchArea", szResult);
		if (nSearchStatus)
		{
			nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_hMeasure[i].m_nSearchExtend);
			if (!nStatus)
			{
				return false;
			}

		}



		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "StandardValue", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_hMeasure[i].m_fStandardValue);
		if (!nStatus)
		{
			return false;
		}

		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, MainTitle, szTittle, "ErrorValue", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		nStatus = KxXmlFun::FromStringToFloat(szResult, m_hParameter.m_hMeasure[i].m_fErrorValue);
		if (!nStatus)
		{
			return false;
		}



		//自动确定测量点的搜索方向
		//kxRect<int> rc1, rc2;
		//kxRect<int> rcSearch;
		rcSearch = m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0];
		if (m_hParameter.m_hMeasure[i].m_nMeasureSearchType == _SEARCH_VERTICAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Width()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nMeasureSearchType, i, pTemplateProject);

			rc1.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].bottom);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].right, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].bottom);


			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].left - m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left;

			ippsSum_32f(pTemplateProject, rc1.Width(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Width(), &sum2, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Width() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchThresh = (int)sum;


			delete[] pTemplateProject;
		}
		if (m_hParameter.m_hMeasure[i].m_nMeasureSearchType == _SEARCH_HORIZONTAL_LINE)
		{
			Ipp32f* pTemplateProject = new Ipp32f[rcSearch.Height()];
			PreAnalyseImage(m_ImgBase, rcSearch, m_hParameter.m_hMeasure[i].m_nMeasureSearchType, i, pTemplateProject);



			rc1.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].right, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top);
			rc2.setup(m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].left, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top,
				m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].right, m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].bottom);

			Ipp32f sum1, sum2, sum;
			int nidx = m_hParameter.m_hMeasure[i].m_rcMeasureInfo[1].top - m_hParameter.m_hMeasure[i].m_rcMeasureInfo[0].top;

			ippsSum_32f(pTemplateProject, rc1.Height(), &sum1, ippAlgHintFast);
			ippsSum_32f(pTemplateProject + nidx, rc2.Height(), &sum2, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchDir = (sum1 > sum2 ? 1 : 0);
			int nIdx = (std::max)(0, nidx - 1);
			int nLen = (std::min)(rcSearch.Height() - nIdx, (int)_SEARCH_LEN);
			ippsSum_32f(pTemplateProject + nIdx, nLen, &sum, ippAlgHintFast);
			m_hParameter.m_hMeasure[i].m_nMeasureSearchThresh = (int)sum;

			delete[] pTemplateProject;
		}


	}
	return true;

}



int CKxDistanceModule::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel)
{
	KxCallStatus hCall;
	return Check(buf, nWidth, nHeight, nPitch, nChannel, hCall);
}

int CKxDistanceModule::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall)
{
	m_TmpImg.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, false);
	return Check(m_TmpImg, hCall);
}

int CKxDistanceModule::Check(const kxCImageBuf& ImgCheck)
{
	KxCallStatus hCall;
	return Check(ImgCheck, hCall);
}

int CKxDistanceModule::PreAnalyseImage(const kxCImageBuf& SrcImg, kxRect<int>& rcSearch, int nSearchType, int nIndex, Ipp32f* pProject)
{
	rcSearch.left = (std::max)(0, rcSearch.left);
	rcSearch.top = (std::max)(0, rcSearch.top);
	rcSearch.right = (std::min)(SrcImg.nWidth - 1, rcSearch.right);
	rcSearch.bottom = (std::min)(SrcImg.nHeight - 1, rcSearch.bottom);

	m_ImgCopy[nIndex].Init(rcSearch.Width(), rcSearch.Height(), SrcImg.nChannel);
	if (SrcImg.nChannel == _Type_G8)
	{
		IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
		ippiCopy_8u_C1R(SrcImg.buf + rcSearch.left + rcSearch.top * SrcImg.nPitch, SrcImg.nPitch, m_ImgCopy[nIndex].buf, m_ImgCopy[nIndex].nPitch, roiSize);
	}
	else
	{
		IppiSize roiSize = { rcSearch.Width(), rcSearch.Height() };
		ippiCopy_8u_C3R(SrcImg.buf + rcSearch.left * 3 + rcSearch.top * SrcImg.nPitch, SrcImg.nPitch, m_ImgCopy[nIndex].buf, m_ImgCopy[nIndex].nPitch, roiSize);
	}

	m_hGradient[nIndex].SingleDirGradientSame(m_ImgCopy[nIndex], m_ImgGradientDir[nIndex], nSearchType, 1, 7, 0);
	m_hBaseFun[nIndex].KxImageMaxEvery(m_ImgGradientDir[nIndex], m_ImgMaxGradient[nIndex]);
	//m_hBaseFun[nIndex].KxThreshImage(m_ImgMaxGradient[nIndex], m_ImgThreshGradient[nIndex], 20, 255);
	m_hBaseFun[nIndex].KxProjectImage(m_ImgMaxGradient[nIndex], nSearchType, pProject);

	return 1;
}

int CKxDistanceModule::ComputeDistance(const kxCImageBuf& SrcImg, const kxCImageBuf& BaseImg, int nSearchType, int nSearchDir, int nThresh, const kxRect<int> rc[2], int nIndex, int& nDx, int& nDy, KxCallStatus& hCall)
{
	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	//rc[0] 存放搜索框
	//rc[1] 存放定位点
	if (nSearchType == _SEARCH_HORIZONTAL_LINE)
	{
		kxRect<int> rcSearch;
		rcSearch = rc[0];
		const int nExtendLen = 0;
		rcSearch.top = (std::max)(0, rc[0].top - nExtendLen);
		rcSearch.bottom = (std::min)(rc[0].bottom + nExtendLen, SrcImg.nHeight-1);
		Ipp32f* pProject = new Ipp32f[rcSearch.Height()];
		PreAnalyseImage(SrcImg, rcSearch, nSearchType, nIndex, pProject);
		Ipp32f sum;
		int nBestIndex = 0;
		bool bFind = false;
		for (int k = 0; k < rcSearch.Height()-3; k++)
		{
			if (nSearchDir == 0)
			{
				sum = pProject[k] + pProject[k + 1] + pProject[k + 2];
				if (sum >  nThresh*0.6)  //容易提前
				{		
					//精确定位
					int idx;
					Ipp32f Max = -1.0f;
					for (int j = -1; j <= _SEARCH_LEN; j++)
					{
						int nidx = (std::min)((std::max)(k + j, 0), rcSearch.Height() - 1);
						if (pProject[nidx] > Max)
						{
							Max = pProject[nidx];
							idx = j;
						}
					}
					nBestIndex = k + idx;
					bFind = true;
					break;
				}
			}
			else
			{
				sum = pProject[rcSearch.Height() - 1 - k] + pProject[rcSearch.Height() - 2 - k] + pProject[rcSearch.Height() - 3 - k];
				if (sum > nThresh*0.6)
				{
					//精确定位
					int idx;
					Ipp32f Max = -1.0f;
					for (int j = -_SEARCH_LEN; j <= 1; j++)
					{
						int nidx = (std::min)((std::max)(rcSearch.Height() - 3 - k + j, 0), rcSearch.Height() - 1);
						if (pProject[nidx] > Max)
						{
							Max = pProject[nidx];
							idx = j;
						}
					}

					nBestIndex = rcSearch.Height() - 3 - k + idx;
					bFind = true;
					break;
				}
			}
		}


		if (!bFind)
		{
			IppStatus status = IppStatus(10000);
			char sz[128];
			memset(sz, 0, 128);
			sprintf_s(sz, 128, "ComputeDistance_SimilaryErr");
			if (check_sts(status, sz, hCall))
			{
				return 0;
			}
		}
		else
		{
			nDy = nBestIndex - (rc[1].top - rc[0].top);
			nDx = 0;
		}

		delete[]pProject;

	}
	else if (nSearchType == _SEARCH_VERTICAL_LINE)
	{
		kxRect<int> rcSearch;
		rcSearch = rc[0];

		const int nExtendLen = 0;
		rcSearch.left = (std::max)(0, rc[0].left - nExtendLen);
		rcSearch.right = (std::min)(rc[0].right + nExtendLen, SrcImg.nWidth - 1);
		Ipp32f* pProject = new Ipp32f[rcSearch.Width()];
		PreAnalyseImage(SrcImg, rcSearch, nSearchType, nIndex, pProject);

		Ipp32f sum;
		int nBestIndex = 0;
		bool bFind = false;
		for (int k = 0; k < rcSearch.Width() - 3; k++)
		{
			if (nSearchDir == 0)
			{
				sum = pProject[k] + pProject[k + 1] + pProject[k + 2];
				if (sum >  nThresh*0.6)
				{
					//精确定位
					int idx;
					Ipp32f Max = -1.0f;
					for (int j = -1; j <= _SEARCH_LEN; j++)
					{
						int nidx = (std::min)((std::max)(k + j, 0), rcSearch.Width() - 1);
						if (pProject[nidx] > Max)
						{
							Max = pProject[nidx];
							idx = j;
						}
					}
					nBestIndex = k + idx;
					bFind = true;
					break;

				}
			}
			else
			{
				sum = pProject[rcSearch.Width() - 1 - k] + pProject[rcSearch.Width() - 2 - k] + pProject[rcSearch.Width() - 3 - k];
				if (sum > nThresh*0.6)
				{
					//精确定位
					int idx;
					Ipp32f Max = -1.0f;
					for (int j = -_SEARCH_LEN; j <= 1; j++)
					{
						int nidx = (std::min)((std::max)(rcSearch.Width() - 3 - k + j, 0), rcSearch.Height() - 1);
						if (pProject[nidx] > Max)
						{
							Max = pProject[nidx];
							idx = j;
						}
					}
					nBestIndex = rcSearch.Width() - 3 - k + idx;
					bFind = true;
					break;

				}
			}
		}

		if (!bFind)
		{
			IppStatus status = IppStatus(10000);
			char sz[128];
			memset(sz, 0, 128);
			sprintf_s(sz, 128, "ComputeDistance_SimilaryErr");
			if (check_sts(status, sz, hCall))
			{
				return 0;
			}
		}
		else
		{
			nDx = nBestIndex - (rc[1].left - rc[0].left);
			nDy = 0;
		}

		delete[]pProject;
	}
	else
	{
		m_Imgkern[nIndex].Init(rc[1].Width(), rc[1].Height(), SrcImg.nChannel);
		m_hBaseFun[nIndex].KxCopyImage(BaseImg, m_Imgkern[nIndex], rc[1]);
		int nExtendSize1 = (std::max)(rc[1].top - rc[0].top, rc[1].left - rc[0].left);
		int nExtendSize2 = (std::max)(rc[0].bottom - rc[1].bottom, rc[0].right - rc[1].right);
		int nExtendSize = (std::max)(nExtendSize1, nExtendSize2);
		kxRect<int> rcSearch = rc[1];
		int nStatus = m_hBaseFun[nIndex].GetImgOffset(SrcImg, m_Imgkern[nIndex], rcSearch, nExtendSize, 0, nDx, nDy, hCallInfo);
		if (check_sts(hCallInfo, "ComputeDistance_", hCall))
		{
			return 0;
		}
	}
	return 1;
}


int CKxDistanceModule::SingleMeasure(const kxCImageBuf& ImgCheck, const MeasurePara& para, int nIndex, SingleMeasureResult& res, KxCallStatus& hCall)
{
	if (para.m_nIsOpen == 0)  //关闭
	{
		return 1;
	}

	KxCallStatus hCallInfo;
	hCallInfo.Clear();
	int nRefdx, nRefdy, nMeasuredx, nMeasuredy;
	
	ComputeDistance(ImgCheck, m_ImgBase, para.m_nMeasureSearchType, para.m_nMeasureSearchDir, para.m_nMeasureSearchThresh, para.m_rcMeasureInfo, nIndex, nMeasuredx, nMeasuredy, hCallInfo);
	if (check_sts(hCallInfo, "SingleMeasure_Measure_", hCall))
	{
		res.m_fMeasureValue = -1.0f;
		res.m_szName = para.m_szName;
		char sz[128];
		memset(sz, 0, 128);
		sprintf_s(sz, 128, "%.2f - %.2f", para.m_fStandardValue - para.m_fErrorValue, para.m_fStandardValue + para.m_fErrorValue);
		res.m_szStandardValue = sz;
		res.m_nIsOk = 0;
		return 0;
	}


	if (para.m_nFixed == 1)
	{
		nRefdx = 0;
		nRefdy = 0;
	}
	else
	{
		ComputeDistance(ImgCheck, m_ImgBase, para.m_nRefSearchType, para.m_nRefSearchDir, para.m_nRefSearchThresh, para.m_rcRefInfo, nIndex, nRefdx, nRefdy, hCallInfo);
		if (check_sts(hCallInfo, "SingleMeasure_Reference_", hCall))
		{
			res.m_fMeasureValue = -1.0f;
			res.m_szName = para.m_szName;
			char sz[128];
			memset(sz, 0, 128);
			sprintf_s(sz, 128, "%.2f - %.2f", para.m_fStandardValue - para.m_fErrorValue, para.m_fStandardValue + para.m_fErrorValue);
			res.m_szStandardValue = sz;
			res.m_nIsOk = 0;
			return 0;
		}

	}

	float fMeasure;
	if (para.m_nMeasureSearchType == _SEARCH_IMAGE)
	{
		if (para.m_nMeasureDirection == 0) //垂直测量
		{
			fMeasure = abs(((para.m_rcMeasureInfo[1].top + para.m_rcMeasureInfo[1].bottom) / 2 + nMeasuredy) - ((para.m_rcRefInfo[1].top + para.m_rcRefInfo[1].bottom)/2 + nRefdy))*m_hParameter.m_fActualHeight*1.0f / m_hParameter.m_fProcessHeight + 0.005f;
		}
		else
		{
			fMeasure = abs(((para.m_rcMeasureInfo[1].left + para.m_rcMeasureInfo[1].right) / 2 + nMeasuredx) - ((para.m_rcRefInfo[1].left + para.m_rcRefInfo[1].right)/2 + nRefdx))*m_hParameter.m_fActualWidth*1.0f / m_hParameter.m_fProcessWidth + 0.005f;
		}

	}
	else
	{
		if (para.m_nMeasureDirection == 0) //垂直测量
		{
			fMeasure = abs((para.m_rcMeasureInfo[1].top + nMeasuredy) - (para.m_rcRefInfo[1].top + nRefdy))*m_hParameter.m_fActualHeight*1.0f / m_hParameter.m_fProcessHeight + 0.005f;
		}
		else
		{
			fMeasure = abs((para.m_rcMeasureInfo[1].left + nMeasuredx) - (para.m_rcRefInfo[1].left + nRefdx))*m_hParameter.m_fActualWidth*1.0f / m_hParameter.m_fProcessWidth + 0.005f;
		}
	}

	res.m_fMeasureValue = fMeasure;
	res.m_szName = para.m_szName;
	char sz[128];
	memset(sz, 0, 128);
	sprintf_s(sz, 128, "%.2f - %.2f", para.m_fStandardValue - para.m_fErrorValue, para.m_fStandardValue + para.m_fErrorValue);
	res.m_szStandardValue = sz;

	if (fMeasure <= (para.m_fStandardValue + para.m_fErrorValue) && fMeasure >= (para.m_fStandardValue - para.m_fErrorValue))
	{
		res.m_nIsOk = 1;
	}
	else
	{
		res.m_nIsOk = 0;
	}
	return 1;
}


int CKxDistanceModule::Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall)
{
	hCall.Clear();

	KxCallStatus hCallInfo[_MAX_MEASURE_GROUP];
	
	//检查
	parallel_for(blocked_range<int>(0, m_hParameter.m_nGroup),
		[&](const blocked_range<int>& range)
	{
		for (int index = range.begin(); index != range.end(); index++)
		{
			m_hSingleRes[index].clear();
			SingleMeasure(ImgCheck, m_hParameter.m_hMeasure[index], index, m_hSingleRes[index], hCallInfo[index]);
		}
	}, auto_partitioner());

	//for (int index = 0; index != m_hParameter.m_nGroup; index++)
	//{
	//	m_hSingleRes[index].clear();
	//	SingleMeasure(ImgCheck, m_hParameter.m_hMeasure[index], index, m_hSingleRes[index], hCallInfo[index]);
	//}

	//拷贝结果 
	m_hResult.clear();
	int nCount = 0;
	for (int i = 0; i < m_hParameter.m_nGroup; i++)
	{
		if (m_hParameter.m_hMeasure[i].m_nIsOpen)
		{
			memcpy(&m_hResult.m_hMeasureResults[nCount], &m_hSingleRes[i], sizeof(SingleMeasureResult));
			nCount++;
		}
	}

	
	m_hResult.m_nGroup = nCount;

	for (int i = 0; i < m_hParameter.m_nGroup; i++)
	{
		if (check_sts(hCallInfo[i], "DistanceModuleCheck_", hCall))
		{
			return 0;
		}
	}


	return 1;
}

