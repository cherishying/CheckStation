
#include "KxWarpStretch.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
using namespace tbb;

#include "KxReadXml.h"
//#include <fstream>
//using namespace std;

CKxWarpStretch::CKxWarpStretch()
{
	m_nKernX = 0;
	m_nKernY = 0;
}

CKxWarpStretch::~CKxWarpStretch()
{

}

bool CKxWarpStretch::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "检测框", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcCheckArea);

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "定位核个数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nKernCount);

	if (m_hParameter.m_nKernCount == 0)
	{
		return true;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "定位核框", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToMulKxRect(szResult, m_hParameter.m_nKernCount, m_hParameter.m_rcKern);


	////新增加的读定位核接口
	//nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "定位核", szResult);

	//一般参数
	int nSearchDir;
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "搜索方向", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, nSearchDir);
		for (int i = 0; i < _Max_Kern; i++)
		{
			m_hParameter.m_nKernelDirect[i] = nSearchDir;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "扩展数", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nSearchExpand);
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "算法类型", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nAlgorithmType);
	}

	//截取定位核
	for (int i = 0; i < m_hParameter.m_nKernCount; i++)
	{
		m_KernBuf[i].Init(m_hParameter.m_rcKern[i].Width(), m_hParameter.m_rcKern[i].Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_KernBuf[i], m_hParameter.m_rcKern[i]);

//		if (gTest)
//		{
//			char sz[128];
//			sprintf_s(sz, 128, "d:\\Test\\m_KernBuf_%d.bmp", i);
//			m_hBaseFun.SaveBMPImage_h(sz, m_KernBuf[i]);
//		}

	}

	return true;

}

bool CKxWarpStretch::ReadXmlStandard(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "检测框",  "pos", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcCheckArea);

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "定位核个数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nKernCount);

	if (m_hParameter.m_nKernCount == 0)
	{
		return true;
	}

	////新增加的读定位核接口
	for (int i = 1; i <= m_hParameter.m_nKernCount; i++)
	{
		char sz[128];
		memset(sz, 0, 128);
		sprintf_s(sz, 128, "定位核%d", i);
		nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "定位核", sz, "pos", szResult);
		if (!nSearchStatus)
		{
			return false;
		}
		KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcKern[i-1]);
	}


	//一般参数
	int nSearchDir;
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "搜索方向", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, nSearchDir);
		for (int i = 0; i < _Max_Kern; i++)
		{
			m_hParameter.m_nKernelDirect[i] = nSearchDir;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "搜索范围", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nSearchExpand);
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "定位设置", "算法类型", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nAlgorithmType);
	}

	//截取定位核
	for (int i = 0; i < m_hParameter.m_nKernCount; i++)
	{
		m_KernBuf[i].Init(m_hParameter.m_rcKern[i].Width(), m_hParameter.m_rcKern[i].Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_KernBuf[i], m_hParameter.m_rcKern[i]);

		//		if (gTest)
		//		{
		//			char sz[128];
		//			sprintf_s(sz, 128, "d:\\Test\\m_KernBuf_%d.bmp", i);
		//			m_hBaseFun.SaveBMPImage_h(sz, m_KernBuf[i]);
		//		}

	}

	return true;

}



bool CKxWarpStretch::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "LocateSetting", "DetectBox", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcCheckArea);

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "LocateSetting", "NumofLocKernel", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nKernCount);

	if (m_hParameter.m_nKernCount == 0)
	{
		return true;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "LocateSetting", "LocKernelBox", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToMulKxRect(szResult, m_hParameter.m_nKernCount, m_hParameter.m_rcKern);



	//一般参数
	int nSearchDir;
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "LocateSetting", "SearchDirection", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, nSearchDir);
		for (int i = 0; i < _Max_Kern; i++)
		{
			m_hParameter.m_nKernelDirect[i] = nSearchDir;
		}
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "LocateSetting", "SearchRange", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nSearchExpand);
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "LocateSetting", "AlgorithmType", szResult);
	if (nSearchStatus)
	{
		KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nAlgorithmType);
	}

	//截取定位核
	for (int i = 0; i < m_hParameter.m_nKernCount; i++)
	{
		m_KernBuf[i].Init(m_hParameter.m_rcKern[i].Width(), m_hParameter.m_rcKern[i].Height(), BaseImg.nChannel);
		m_hBaseFun.KxCopyImage(BaseImg, m_KernBuf[i], m_hParameter.m_rcKern[i]);

//		if (gTest)
//		{
//			char sz[128];
//			sprintf_s(sz, 128, "d:\\Test\\m_KernBuf_%d.bmp", i);
//			m_hBaseFun.SaveBMPImage_h(sz, m_KernBuf[i]);
//		}

	}




	return true;

}


//从网络中获取参数
bool CKxWarpStretch::ReadParaFromNet( unsigned char*& point )
{
	memcpy(&m_hParameter, point, sizeof(Parameter));
	point += sizeof(Parameter);
	CKxBaseFunction hBaseFun;

	for (int i = 0; i < _Max_Kern; i++)
	{
		if(!hBaseFun.readImgBufFromMemory(m_KernBuf[i], point))
		{
			return false;
		}
	}

	return true;
}

bool CKxWarpStretch::ReadVesion1Para( FILE* fp)    //读取版本1参数
{
	if (fread(&m_hParameter.m_rcCheckArea, sizeof(kxRect<int>), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nKernCount, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcKern, sizeof(kxRect<int>), _Max_Kern, fp) != _Max_Kern)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nSearchExpand, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nKernelDirect, sizeof(int), _Max_Kern, fp) != _Max_Kern)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nAlgorithmType, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	for (int i = 0; i < _Max_Kern; i++)
	{
		if ( !m_KernBuf[i].Read(fp))
		{
			return false;
		}
	}

	return true;
}



bool CKxWarpStretch::Read( FILE*  fp)
{
	if (fread(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_hParameter.m_szVersion, "WarpStretch1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}


}

bool CKxWarpStretch::WriteVesion1Para( FILE* fp)    //写入版本1参数
{
	if (fwrite(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcCheckArea, sizeof(kxRect<int>), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nKernCount, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcKern, sizeof(kxRect<int>), _Max_Kern, fp) != _Max_Kern)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nSearchExpand, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nKernelDirect, sizeof(int), _Max_Kern, fp) != _Max_Kern)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nAlgorithmType, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	for (int i = 0; i < _Max_Kern; i++)
	{
		if ( !m_KernBuf[i].Write(fp))
		{
			return false;
		}
	}

	return true;


}
bool CKxWarpStretch::Write( FILE*  fp)
{
	if (strcmp(m_hParameter.m_szVersion, "WarpStretch1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return WriteVesion1Para(fp);
	}
	else
	{
		return false;
	}

}


int CKxWarpStretch::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall)
{
	m_WarpImg.Init( m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height(), nChannel);
	return Check( buf, nWidth, nHeight, nPitch, nChannel, m_WarpImg.GetBuf(), m_WarpImg.GetWidth(), m_WarpImg.GetHeight(),  m_WarpImg.GetPitch(), hCall);
}

int CKxWarpStretch::Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall)
{
	DstImg.Init(m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height(), SrcImg.nChannel);
	return Check(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel,
		   DstImg.GetBuf(), DstImg.GetWidth(), DstImg.GetHeight(),  DstImg.GetPitch(), hCall);

}

int CKxWarpStretch::Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	DstImg.Init(m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height(), SrcImg.nChannel);
	return Check(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel,
		DstImg.GetBuf(), DstImg.GetWidth(), DstImg.GetHeight(), DstImg.GetPitch(), hCall);

}


int CKxWarpStretch::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
						   unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	switch ( m_hParameter.m_nKernCount )
	{
	//直接复制图像
	case 0:
		{
			IppiSize Roi = {m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height()};
			m_hBaseFun.KxCopyImage(buf, nWidth, nHeight, nPitch, nChannel,  pDstBuf, nDstWidth, nDstHeight, nDstPitch, nChannel, m_hParameter.m_rcCheckArea, hTempCall);

			if (check_sts(hTempCall, "CKxWarpStretch_Check_0", hCall))
			{
				return -1;
			}
			break;
		}
	case 1:
		{
			int nDx, nDy;
			if (GetImgOffset(buf, nWidth, nHeight, nPitch, nChannel, nDx, nDy, hTempCall) == 0)
			{
				return 1;
			}
			if (check_sts(hTempCall, "CKxWarpStretch_Check_1", hCall))
			{
				return -1;
			}


			IppiSize OffSize = {m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height()};
			if (m_hParameter.m_rcCheckArea.left + nDx < 0){ nDx = 0 - m_hParameter.m_rcCheckArea.left;}
			if (m_hParameter.m_rcCheckArea.right + nDx > nWidth-1){ nDx = nWidth-1-m_hParameter.m_rcCheckArea.right;}
			if (m_hParameter.m_rcCheckArea.top + nDy < 0){ nDy = 0 - m_hParameter.m_rcCheckArea.top;}
			if (m_hParameter.m_rcCheckArea.bottom + nDy > nHeight-1){ nDy = nHeight-1-m_hParameter.m_rcCheckArea.bottom;}

			kxRect<int> rc = m_hParameter.m_rcCheckArea;
			rc.offset(nDx, nDy);
			m_ptOffset.x = nDx;
			m_ptOffset.y = nDy;



			m_hBaseFun.KxCopyImage(buf, nWidth, nHeight, nPitch,  nChannel,  pDstBuf, nDstWidth, nDstHeight, nDstPitch, nChannel, rc, hTempCall);

			if (check_sts(hTempCall, "CKxWarpStretch_Check_1", hCall))
			{
				return -1;
			}

			break;

		}
	case 2:
		{
			int nStatus = RotateOffsetTransformWarp(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall);

			if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
			{
				return -1;
			}

			if (nStatus)
			{
				return nStatus;
			}
			break;
		}
	case 3:
		{
			int nStatus = AffineTransformWarp(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall);

			if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
			{
				return -1;
			}

			if (nStatus)
			{
				return  nStatus;
			}
			break;

		}
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		{
			if (_Perspective_Trans == m_hParameter.m_nAlgorithmType )
			{
				int nStatus = PerspectiveTransformWarp(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall);

				if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
				{
					return -1;
				}

				if (nStatus)
				{
					return  nStatus;
				}
			}
			if (_Bilinear_Trans == m_hParameter.m_nAlgorithmType)
			{
				int nStatus = BilinearTransformWarp(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall);

				if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
				{
					return -1;
				}


				if (nStatus)
				{
					return  nStatus;
				}
			}
			break;

		}
	default:
		break;

	}

	return 0;
}


int CKxWarpStretch::GetImgOffset(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, int nChannel, int& nDx, int& nDy, KxCallStatus& hCall)
{
	kxRect<int> rcSearch;
	int nleft,nright,ntop,nbottom;
	if (m_hParameter.m_nKernelDirect[0] == _Horiz_Vertical_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcKern[0].GetLeft() - m_hParameter.m_nSearchExpand);
		ntop  = gMax(0, m_hParameter.m_rcKern[0].GetTop() - m_hParameter.m_nSearchExpand);
		nright = gMin(nWidth-1, m_hParameter.m_rcKern[0].GetRight() + m_hParameter.m_nSearchExpand);
		nbottom = gMin(nHeight-1, m_hParameter.m_rcKern[0].GetBottom() + m_hParameter.m_nSearchExpand);
	}
	if (m_hParameter.m_nKernelDirect[0] == _Vertical_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcKern[0].GetLeft());
		ntop = gMax(0, m_hParameter.m_rcKern[0].GetTop() - m_hParameter.m_nSearchExpand);
		nright = gMin(nWidth-1, m_hParameter.m_rcKern[0].GetRight());
		nbottom = gMin(nHeight-1, m_hParameter.m_rcKern[0].GetBottom()+ m_hParameter.m_nSearchExpand);
	}
	if (m_hParameter.m_nKernelDirect[0] == _Horizontal_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcKern[0].GetLeft()- m_hParameter.m_nSearchExpand);
		ntop = gMax(0, m_hParameter.m_rcKern[0].GetTop());
		nright = gMin(nWidth-1, m_hParameter.m_rcKern[0].GetRight() + m_hParameter.m_nSearchExpand);
		nbottom = gMin(nHeight-1, m_hParameter.m_rcKern[0].GetBottom());
	}

	rcSearch.setup(nleft,ntop,nright,nbottom);
	//-------------------------------------------------------------------------------------------------
	kxPoint<float>  pos;

	KxCallStatus hTempCall;
	hTempCall.Clear();

	//float fratio = m_hBaseFun.kxImageAlign( pos, pBuf + rcSearch.GetTop()*nPitch+rcSearch.GetLeft()*nChannel, rcSearch.Width(), rcSearch.Height(), nPitch,
	//	m_KernBuf[0].buf, m_KernBuf[0].nWidth, m_KernBuf[0].nHeight, m_KernBuf[0].nPitch , nChannel, hTempCall);

	//2018/05/10  芯片定位加速
	kxCImageBuf imgSearch;
	imgSearch.SetImageBuf(pBuf + rcSearch.GetTop()*nPitch + rcSearch.GetLeft()*nChannel, rcSearch.Width(), rcSearch.Height(), nPitch, nChannel, false);
	float fratio = m_hBaseFun.kxImageAlignColor(pos, imgSearch, m_KernBuf[0]);


	//
	//m_hBaseFun.SaveBMPImage_h("d:\\Buf.bmp", pBuf, nWidth, nHeight, nPitch, nChannel);
	//m_hBaseFun.SaveBMPImage_h("d:\\Crop.bmp", pBuf + rcSearch.GetTop()*nPitch + rcSearch.GetLeft()*nChannel, rcSearch.Width(), rcSearch.Height(), nPitch, nChannel);
	//m_hBaseFun.SaveBMPImage_h("d:\\m_KernBuf.bmp", m_KernBuf[0]);

	if (check_sts(hTempCall, "GetImgOffset", hCall))
	{
		return 0;
	}

	if (fratio < gMinSimilarity)
	{
		IppStatus  status = IppStatus(kxImageAlignRatioLow);
		if (check_sts(status, "KxAlign_Low_", hCall))
		{
			nDx = 0;
			nDy = 0;
			return 0;
		}
	}

	nDx = int(rcSearch.GetLeft() + pos.x - m_hParameter.m_rcKern[0].left);
	nDy = int(rcSearch.GetTop() + pos.y - m_hParameter.m_rcKern[0].top);

	m_nKernX = nDx;
	m_nKernY = nDy;


	return 1;
}



int CKxWarpStretch::RotateOffsetTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall)
{
	return 0;
}



int CKxWarpStretch::GetRotateOffTransform( const double srcQuad[2][2], const double dstQuad[2][2], double coeffs[2][3], KxCallStatus& hCall )
{

	return 1;

}



int CKxWarpStretch::AffineTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall)
{

	return 0;
}


int CKxWarpStretch::GetAffineTransform( const double srcQuad[3][2], const double dstQuad[3][2], double coeffs[2][3], KxCallStatus& hCall )
{

	return 1;

}


int CKxWarpStretch::PerspectiveTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall)
{


	return 0;
}


bool CKxWarpStretch::GetPerspectiveTransform_8(const double srcQuad[8][2], const double dstQuad[8][2], double coeffs[3][3], KxCallStatus& hCall)
{

	return true;

}


bool CKxWarpStretch::GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3], KxCallStatus& hCall)
{

	return true;
}



int CKxWarpStretch::BilinearTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTmpCall;
	hTmpCall.Clear();

	IppiSize srcSize = { nWidth, nHeight };     //待检测大图size
	IppiRect srcRoi = { 0, 0, nWidth, nHeight };//待检测大图ROI
	double matchQuad[_Max_Kern][2] = {0}; //匹配到的点，坐标已变换到待检测图上
	double kernQuad[_Max_Kern][2] = {0};  //模板图像上定位核的LeftTop点

	for( int i = 0; i < m_hParameter.m_nKernCount; i++ )
	{
		IppiRect searchRoi;
		int nleft, ntop, nright, nbottom;
		if (m_hParameter.m_nKernelDirect[i] == _Horiz_Vertical_Dir)
		{
			nleft = gMax(0,m_hParameter.m_rcKern[i].GetLeft() - m_hParameter.m_nSearchExpand);
			ntop = gMax(0,m_hParameter.m_rcKern[i].GetTop() - m_hParameter.m_nSearchExpand);
			nright = gMin(nWidth-1,m_hParameter.m_rcKern[i].GetRight() + m_hParameter.m_nSearchExpand);
			nbottom = gMin(nHeight-1,m_hParameter.m_rcKern[i].GetBottom() + m_hParameter.m_nSearchExpand);
		}
		if (m_hParameter.m_nKernelDirect[i] == _Vertical_Dir)
		{
			nleft = gMax(0, m_hParameter.m_rcKern[i].GetLeft());
			ntop = gMax(0, m_hParameter.m_rcKern[i].GetTop() - m_hParameter.m_nSearchExpand);
			nright = gMin(nWidth-1,m_hParameter.m_rcKern[i].GetRight());
			nbottom = gMin(nHeight-1,m_hParameter.m_rcKern[i].GetBottom() + m_hParameter.m_nSearchExpand);
		}
		if (m_hParameter.m_nKernelDirect[i] == _Horizontal_Dir)
		{
			nleft = gMax(0,m_hParameter.m_rcKern[i].GetLeft()- m_hParameter.m_nSearchExpand);
			ntop = gMax(0,m_hParameter.m_rcKern[i].GetTop());
			nright = gMin(nWidth-1,m_hParameter.m_rcKern[i].GetRight() + m_hParameter.m_nSearchExpand);
			nbottom = gMin(nHeight-1,m_hParameter.m_rcKern[i].GetBottom());
		}
		searchRoi.x = nleft;
		searchRoi.y = ntop;
		searchRoi.width = nright - nleft + 1;
		searchRoi.height = nbottom - ntop + 1;

		kxPoint<float>  pos;


		float fratio = m_hBaseFun.kxImageAlign( pos, pBuf + searchRoi.y*nPitch+searchRoi.x*nChannel, searchRoi.width, searchRoi.height, nPitch,
			m_KernBuf[i].buf, m_KernBuf[i].nWidth, m_KernBuf[i].nHeight, m_KernBuf[i].nPitch , nChannel, hTmpCall);

		if (check_sts(hTmpCall, "BilinearTransformWarp", hCall))
		{
			return -1;
		}


		if (fratio < gMinSimilarity)
		{
			return i+1;
		}
		pos.x += searchRoi.x;
		pos.y += searchRoi.y;
		matchQuad[i][0] = pos.x;
		matchQuad[i][1] = pos.y;

		kernQuad[i][0] = m_hParameter.m_rcKern[i].left;
		kernQuad[i][1] = m_hParameter.m_rcKern[i].top;
	}

	//第一个定位核的实际位置
	m_nKernX = int(matchQuad[0][0] - m_hParameter.m_rcKern[0].left);
	m_nKernY = int(matchQuad[0][1] - m_hParameter.m_rcKern[0].top);

	double coeffsFine[2][4];
	GetBilinearTransform_8( kernQuad, matchQuad, coeffsFine, hTmpCall );

	if (check_sts(hTmpCall, "BilinearTransformWarp", hCall))
	{
		return -1;
	}

	IppiRect warpRoi;
	warpRoi.x = m_hParameter.m_rcCheckArea.left;
	warpRoi.y = m_hParameter.m_rcCheckArea.top;
	warpRoi.width = m_hParameter.m_rcCheckArea.Width();
	warpRoi.height = m_hParameter.m_rcCheckArea.Height();
	double warpSrcQuad[4][2];
	IppStatus status = ippiGetBilinearQuad( warpRoi, warpSrcQuad, coeffsFine );

	if (check_sts(status, "BilinearTransformWarp_ippiGetBilinearQuad", hCall))
	{
		return -1;
	}

	IppiRect warpDstRoi = { 0, 0, warpRoi.width, warpRoi.height };
	double warpDstQuad[4][2] = { { 0, 0 }, { warpRoi.width-1, 0 }, { warpRoi.width-1, warpRoi.height-1 }, { 0, warpRoi.height-1 } };

	int nBufferSize;
	status = ippiWarpBilinearQuadGetBufferSize(srcSize, srcRoi, warpSrcQuad, warpDstRoi, warpDstQuad, IPPI_INTER_LINEAR, &nBufferSize);
	if (check_sts(status, "BilinearTransformWarp_ippiWarpBilinearQuadGetBufferSize", hCall))
	{
		return -1;
	}
	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);

	if (nChannel == _Type_G8)
	{
		status = ippiWarpBilinearQuad_8u_C1R( pBuf, srcSize, nPitch, srcRoi, warpSrcQuad,
			pDstBuf, nDstPitch, warpDstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer );

		if (check_sts(status, "BilinearTransformWarp_ippiWarpBilinearQuad_8u_C1R", hCall))
		{
			ippsFree(pBuffer);
			return -1;
		}
	}
	if (nChannel == _Type_G24)
	{
		status = ippiWarpBilinearQuad_8u_C3R( pBuf, srcSize, nPitch, srcRoi, warpSrcQuad,
			pDstBuf, nDstPitch, warpDstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer );

		if (check_sts(status, "BilinearTransformWarp_ippiWarpBilinearQuad_8u_C3R", hCall))
		{
			ippsFree(pBuffer);
			return -1;
		}
	}

	ippsFree(pBuffer);

	return 0;
}




int CKxWarpStretch::GetBilinearTransform( const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4], KxCallStatus& hCall )
{
	hCall.Clear();
	/* Source matrix with widthHeight=3 */
	double pSrc[4 * 4] = { srcQuad[0][0] * srcQuad[0][1], srcQuad[0][0], srcQuad[0][1], 1,
		srcQuad[1][0] * srcQuad[1][1], srcQuad[1][0], srcQuad[1][1], 1,
		srcQuad[2][0] * srcQuad[2][1], srcQuad[2][0], srcQuad[2][1], 1,
		srcQuad[3][0] * srcQuad[3][1], srcQuad[3][0], srcQuad[3][1], 1 };
	double pSrc2[4 * 2] = { dstQuad[0][0], dstQuad[0][1], dstQuad[1][0], dstQuad[1][1],
		dstQuad[2][0], dstQuad[2][1], dstQuad[3][0], dstQuad[3][1] };

	//////////////////////////////////////////////////////////////////////////
	double *ps = new double[4];
	lapack_int rank;
	int info = LAPACKE_dgelsd(LAPACK_ROW_MAJOR, 4, 4, 2, pSrc, 4, pSrc2, 2, ps, -1.0, &rank);
	if (check_sts(info, "GetBilinearTransform_LAPACKE_dgelsd", hCall))
	{
		return 0;
	}
	delete[]ps;

	for (int i = 0; i < 4; i++)
	{
		coeffs[0][i] = pSrc2[2 * i];
		coeffs[1][i] = pSrc2[2 * i + 1];
	}
	return 1;


}

int CKxWarpStretch::GetBilinearTransform_8( const double srcQuad[8][2], const double dstQuad[8][2], double coeffs[2][4], KxCallStatus& hCall )
{
	hCall.Clear();

	/* Source matrix with widthHeight=3 */
	double pSrc[8*4] = { srcQuad[0][0] * srcQuad[0][1], srcQuad[0][0], srcQuad[0][1], 1,
		srcQuad[1][0] * srcQuad[1][1], srcQuad[1][0], srcQuad[1][1], 1,
		srcQuad[2][0] * srcQuad[2][1], srcQuad[2][0], srcQuad[2][1], 1,
		srcQuad[3][0] * srcQuad[3][1], srcQuad[3][0], srcQuad[3][1], 1,

		srcQuad[4][0] * srcQuad[4][1], srcQuad[4][0], srcQuad[4][1], 1,
		srcQuad[5][0] * srcQuad[5][1], srcQuad[5][0], srcQuad[5][1], 1,
		srcQuad[6][0] * srcQuad[6][1], srcQuad[6][0], srcQuad[6][1], 1,
		srcQuad[7][0] * srcQuad[7][1], srcQuad[7][0], srcQuad[7][1], 1};
	for (int x=0;x<8;x++)
	{
		if (srcQuad[x][0]==0 && srcQuad[x][1]==0 && dstQuad[x][0]==0&& dstQuad[x][1]==0)
		{
			pSrc[4*x+3]=0;
		}

	}

	double pSrc2[8*2] = { dstQuad[0][0], dstQuad[0][1], dstQuad[2][0], dstQuad[2][1],dstQuad[3][0], dstQuad[3][1], dstQuad[4][0], dstQuad[4][1],
		dstQuad[5][0], dstQuad[5][1], dstQuad[6][0], dstQuad[6][1],	dstQuad[6][0], dstQuad[6][1], dstQuad[7][0], dstQuad[7][1]};

	double *ps = new double[4];
	lapack_int rank;
	int info = LAPACKE_dgelsd(LAPACK_ROW_MAJOR, 8,4,2,pSrc,4,pSrc2,2,ps,-1.0, &rank);
	if (check_sts(info, "GetBilinearTransform_8_LAPACKE_dgelsd", hCall))
	{
		return 0;
	}
	delete []ps;

	for(int i=0; i<4; i++)
	{
		coeffs[0][i] = pSrc2[2*i];
		coeffs[1][i] = pSrc2[2*i+1];
	}
	return 1;


}
//////////////////////////////////////
//并行算法
int CKxWarpStretch::CheckParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	m_WarpImg.Init(m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height(), nChannel);
	return CheckParallel(buf, nWidth, nHeight, nPitch, nChannel, m_WarpImg.GetBuf(), m_WarpImg.GetWidth(), m_WarpImg.GetHeight(), m_WarpImg.GetPitch(), hCall, nRowGrain, nColGrain);
}

int CKxWarpStretch::CheckParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	DstImg.Init(m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height(), SrcImg.nChannel);
	int nStatus = CheckParallel(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel,
		DstImg.GetBuf(), DstImg.GetWidth(), DstImg.GetHeight(), DstImg.GetPitch(), hCall, nRowGrain, nColGrain);
	return nStatus;

}

int CKxWarpStretch::CheckParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg)
{
	KxCallStatus hCall;
	DstImg.Init(m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height(), SrcImg.nChannel);
	int nStatus = CheckParallel(SrcImg.buf, SrcImg.nWidth, SrcImg.nHeight, SrcImg.nPitch, SrcImg.nChannel,
		DstImg.GetBuf(), DstImg.GetWidth(), DstImg.GetHeight(), DstImg.GetPitch(), hCall);
	return nStatus;

}




int CKxWarpStretch::CheckParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
	unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	switch (m_hParameter.m_nKernCount)
	{
		//直接复制图像
	case 0:
	{
			  IppiSize Roi = { m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height() };
			  m_hBaseFun.KxCopyImage(buf, nWidth, nHeight, nPitch, nChannel, pDstBuf, nDstWidth, nDstHeight, nDstPitch, nChannel, m_hParameter.m_rcCheckArea, hTempCall);

			  if (check_sts(hTempCall, "CKxWarpStretch_Check_0", hCall))
			  {
				  return -1;
			  }
			  break;
	}
	case 1:
	{
			  int nDx, nDy;
			  int nStatus = GetImgOffset(buf, nWidth, nHeight, nPitch, nChannel, nDx, nDy, hTempCall);
			  if (nStatus == 0)
			  {
				  return 1;
			  }


			  IppiSize OffSize = { m_hParameter.m_rcCheckArea.Width(), m_hParameter.m_rcCheckArea.Height() };
			  if (m_hParameter.m_rcCheckArea.left + nDx < 0){ nDx = 0 - m_hParameter.m_rcCheckArea.left; }
			  if (m_hParameter.m_rcCheckArea.right + nDx > nWidth - 1){ nDx = nWidth - 1 - m_hParameter.m_rcCheckArea.right; }
			  if (m_hParameter.m_rcCheckArea.top + nDy < 0){ nDy = 0 - m_hParameter.m_rcCheckArea.top; }
			  if (m_hParameter.m_rcCheckArea.bottom + nDy > nHeight - 1){ nDy = nHeight - 1 - m_hParameter.m_rcCheckArea.bottom; }

			  kxRect<int> rc = m_hParameter.m_rcCheckArea;
			  rc.offset(nDx, nDy);

			  m_ptOffset.x = nDx;
			  m_ptOffset.y = nDy;


			  m_hBaseFun.KxCopyImage(buf, nWidth, nHeight, nPitch, nChannel, pDstBuf, nDstWidth, nDstHeight, nDstPitch, nChannel, rc, hTempCall);

			  if (check_sts(hTempCall, "CKxWarpStretch_Check_1", hCall))
			  {
				  return -1;
			  }

			  break;

	}
	case 2:
	{
			  int nStatus = RotateOffsetTransformWarpParallel(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall, nRowGrain, nColGrain);

			  if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
			  {
				  return -1;
			  }

			  if (nStatus)
			  {
				  return nStatus;
			  }
			  break;
	}
	case 3:
	{
			  int nStatus = AffineTransformWarpParallel(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall, nRowGrain, nColGrain);

			  if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
			  {
				  return -1;
			  }

			  if (nStatus)
			  {
				  return  nStatus;
			  }
			  break;

	}
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	{
			  if (_Perspective_Trans == m_hParameter.m_nAlgorithmType)
			  {
				  int nStatus = PerspectiveTransformWarpParallel(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall, nRowGrain, nColGrain);

				  if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
				  {
					  return -1;
				  }

				  if (nStatus)
				  {
					  return  nStatus;
				  }
			  }
			  //if (_Bilinear_Trans == m_hParameter.m_nAlgorithmType)
			  //{
				 // int nStatus = BilinearTransformWarp(buf, nWidth, nHeight, nPitch, pDstBuf, nDstPitch, nChannel, hTempCall);

				 // if (check_sts(hTempCall, "CKxWarpStretch_Check", hCall))
				 // {
					//  return -1;
				 // }

				 // if (nStatus)
				 // {
					//  return  nStatus;
				 // }
			  //}
			  break;

	}
	default:
		break;

	}

	return 0;
}


int CKxWarpStretch::RotateOffsetTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	IppiSize srcSize = { nWidth, nHeight };     //待检测大图size
	IppiRect srcRoi = { 0, 0, nWidth, nHeight };//待检测大图ROI
	double matchQuad[2][2] = { 0 }; //匹配到的点，坐标已变换到待检测图上
	double kernQuad[2][2] = { 0 };  //模板图像上定位核的LeftTop点

	int nRowActualGrain = gMin(nRowGrain, nHeight);
	int nColActualGrain = gMin(nColGrain, nWidth);
	float fRatio[_Max_Kern] = { 0 };

	parallel_for(blocked_range<int>(0, m_hParameter.m_nKernCount),
		[&](const blocked_range<int>& range)
	{
		for (int nIndex = range.begin(); nIndex < range.end(); nIndex++)
		{
			IppiRect searchRoi;
			int nleft, ntop, nright, nbottom;
			if (m_hParameter.m_nKernelDirect[nIndex] == _Horiz_Vertical_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[nIndex].GetLeft() - m_hParameter.m_nSearchExpand);
				ntop = gMax(0, m_hParameter.m_rcKern[nIndex].GetTop() - m_hParameter.m_nSearchExpand);
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[nIndex].GetRight() + m_hParameter.m_nSearchExpand);
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[nIndex].GetBottom() + m_hParameter.m_nSearchExpand);
			}
			if (m_hParameter.m_nKernelDirect[nIndex] == _Vertical_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[nIndex].GetLeft());
				ntop = gMax(0, m_hParameter.m_rcKern[nIndex].GetTop() - m_hParameter.m_nSearchExpand);
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[nIndex].GetRight());
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[nIndex].GetBottom() + m_hParameter.m_nSearchExpand);
			}
			if (m_hParameter.m_nKernelDirect[nIndex] == _Horizontal_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[nIndex].GetLeft() - m_hParameter.m_nSearchExpand);
				ntop = gMax(0, m_hParameter.m_rcKern[nIndex].GetTop());
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[nIndex].GetRight() + m_hParameter.m_nSearchExpand);
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[nIndex].GetBottom());
			}

			searchRoi.x = nleft;
			searchRoi.y = ntop;
			searchRoi.width = nright - nleft + 1;
			searchRoi.height = nbottom - ntop + 1;

			kxPoint<float>  pos;

			fRatio[nIndex] = m_hFun[nIndex].kxImageAlign(pos, pBuf + searchRoi.y*nPitch + searchRoi.x*nChannel, searchRoi.width, searchRoi.height, nPitch,
				m_KernBuf[nIndex].buf, m_KernBuf[nIndex].nWidth, m_KernBuf[nIndex].nHeight, m_KernBuf[nIndex].nPitch, nChannel);

			if (pos.x < 0 || pos.x >= searchRoi.width || pos.y < 0 || pos.y >= searchRoi.height)
			{
				fRatio[nIndex] = 0.0f;
			}

			pos.x += searchRoi.x;
			pos.y += searchRoi.y;
			matchQuad[nIndex][0] = pos.x;
			matchQuad[nIndex][1] = pos.y;

			kernQuad[nIndex][0] = m_hParameter.m_rcKern[nIndex].left;
			kernQuad[nIndex][1] = m_hParameter.m_rcKern[nIndex].top;
		}


	}, auto_partitioner());

	for (int i = 0; i < m_hParameter.m_nKernCount; i++)
	{
		if (fRatio[i] < gMinSimilarity)
		{
			return i + 1;
		}
	}

	//第一个定位核的实际位置
	m_nKernX = int(matchQuad[0][0] - m_hParameter.m_rcKern[0].left);
	m_nKernY = int(matchQuad[0][1] - m_hParameter.m_rcKern[0].top);

	//斜切变换对应的系数
	double coeffsFine[2][3];
	GetRotateOffTransform(kernQuad, matchQuad, coeffsFine, hTempCall);

	if (check_sts(hTempCall, "RotateOffsetTransformWarp", hCall))
	{
		return -1;
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_coeffsFineAffine[i][j] = coeffsFine[i][j];
		}
	}

	double quad[4][2];
	IppiRect kernRect;
	kernRect.x = m_hParameter.m_rcCheckArea.left;
	kernRect.y = m_hParameter.m_rcCheckArea.top;
	kernRect.width = m_hParameter.m_rcCheckArea.Width();
	kernRect.height = m_hParameter.m_rcCheckArea.Height();

	IppStatus status = ippiGetAffineQuad(kernRect, quad, coeffsFine);
	if (check_sts(status, "RotateOffsetTransformWarp_ippiGetAffineQuad", hCall))
	{
		return -1;
	}

	IppiRect warpDstRoi = { 0, 0, kernRect.width, kernRect.height };
	double warpDstQuad[4][2] = { { 0, 0 }, { kernRect.width - 1, 0 }, { kernRect.width - 1, kernRect.height - 1 }, { 0, kernRect.height - 1 } };

	m_SrcImg.SetImageBuf(pBuf, nWidth, nHeight, nPitch, nChannel, false);
	m_DstImg.SetImageBuf(pDstBuf, warpDstRoi.width, warpDstRoi.height, nDstPitch, nChannel, false);


	m_hBaseFun.KxParalleWarpAffineLinearQuadImgae(m_SrcImg, m_DstImg, quad, warpDstQuad, hTempCall, nRowGrain, nColGrain);

	if (check_sts(status, "RotateOffsetTransformWarp_KxWarpAffineLinearQuadImgaeC3", hCall))
	{
		return -1;
	}


	return 0;

}

int CKxWarpStretch::AffineTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	IppiSize srcSize = { nWidth, nHeight };     //待检测大图size
	IppiRect srcRoi = { 0, 0, nWidth, nHeight };//待检测大图ROI
	double matchQuad[3][2] = { 0 }; //匹配到的点，坐标已变换到待检测图上
	double kernQuad[3][2] = { 0 };  //模板图像上定位核的LeftTop点
	float fRatio[_Max_Kern] = { 0 };

	parallel_for(blocked_range<int>(0, m_hParameter.m_nKernCount),
		[&](const blocked_range<int>& range)
	{
		for (int nIndex = range.begin(); nIndex < range.end(); nIndex++)
		{
			IppiRect searchRoi;
			int nleft, ntop, nright, nbottom;
			if (m_hParameter.m_nKernelDirect[nIndex] == _Horiz_Vertical_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[nIndex].GetLeft() - m_hParameter.m_nSearchExpand);
				ntop = gMax(0, m_hParameter.m_rcKern[nIndex].GetTop() - m_hParameter.m_nSearchExpand);
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[nIndex].GetRight() + m_hParameter.m_nSearchExpand);
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[nIndex].GetBottom() + m_hParameter.m_nSearchExpand);
			}
			if (m_hParameter.m_nKernelDirect[nIndex] == _Vertical_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[nIndex].GetLeft());
				ntop = gMax(0, m_hParameter.m_rcKern[nIndex].GetTop() - m_hParameter.m_nSearchExpand);
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[nIndex].GetRight());
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[nIndex].GetBottom() + m_hParameter.m_nSearchExpand);
			}
			if (m_hParameter.m_nKernelDirect[nIndex] == _Horizontal_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[nIndex].GetLeft() - m_hParameter.m_nSearchExpand);
				ntop = gMax(0, m_hParameter.m_rcKern[nIndex].GetTop());
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[nIndex].GetRight() + m_hParameter.m_nSearchExpand);
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[nIndex].GetBottom());
			}

			searchRoi.x = nleft;
			searchRoi.y = ntop;
			searchRoi.width = nright - nleft + 1;
			searchRoi.height = nbottom - ntop + 1;

			kxPoint<float>  pos;

			fRatio[nIndex] = m_hFun[nIndex].kxImageAlign(pos, pBuf + searchRoi.y*nPitch + searchRoi.x*nChannel, searchRoi.width, searchRoi.height, nPitch,
				m_KernBuf[nIndex].buf, m_KernBuf[nIndex].nWidth, m_KernBuf[nIndex].nHeight, m_KernBuf[nIndex].nPitch, nChannel);

			if (pos.x < 0 || pos.x >= searchRoi.width || pos.y < 0 || pos.y >= searchRoi.height)
			{
				fRatio[nIndex] = 0.0f;
			}

			pos.x += searchRoi.x;
			pos.y += searchRoi.y;
			matchQuad[nIndex][0] = pos.x;
			matchQuad[nIndex][1] = pos.y;

			kernQuad[nIndex][0] = m_hParameter.m_rcKern[nIndex].left;
			kernQuad[nIndex][1] = m_hParameter.m_rcKern[nIndex].top;
		}


	},auto_partitioner());

	for (int i = 0; i < m_hParameter.m_nKernCount; i++)
	{
		if (fRatio[i] < gMinSimilarity)
		{
			return i + 1;
		}
	}
	//第一个定位核的实际位置
	m_nKernX = int(matchQuad[0][0] - m_hParameter.m_rcKern[0].left);
	m_nKernY = int(matchQuad[0][1] - m_hParameter.m_rcKern[0].top);

	//仿射变换对应的系数
	double coeffsFine[2][3];
	GetAffineTransform(kernQuad, matchQuad, coeffsFine, hTempCall);

	if (check_sts(hTempCall, "AffineTransformWarp", hCall))
	{
		return -1;
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_coeffsFineAffine[i][j] = coeffsFine[i][j];
		}
	}

	double quad[4][2];
	IppiRect kernRect;
	kernRect.x = m_hParameter.m_rcCheckArea.left;
	kernRect.y = m_hParameter.m_rcCheckArea.top;
	kernRect.width = m_hParameter.m_rcCheckArea.Width();
	kernRect.height = m_hParameter.m_rcCheckArea.Height();

	IppStatus status = ippiGetAffineQuad(kernRect, quad, coeffsFine);
	if (check_sts(status, "AffineTransformWarp_ippiGetAffineQuad", hCall))
	{
		return -1;
	}


	IppiRect warpDstRoi = { 0, 0, kernRect.width, kernRect.height };
	double warpDstQuad[4][2] = { { 0, 0 }, { kernRect.width - 1, 0 }, { kernRect.width - 1, kernRect.height - 1 }, { 0, kernRect.height - 1 } };

	m_SrcImg.SetImageBuf(pBuf, nWidth, nHeight, nPitch, nChannel, false);
	m_DstImg.SetImageBuf(pDstBuf, warpDstRoi.width, warpDstRoi.height, nDstPitch, nChannel, false);


	m_hBaseFun.KxParalleWarpAffineLinearQuadImgae(m_SrcImg, m_DstImg, quad, warpDstQuad, hTempCall, nRowGrain, nColGrain);
	if (check_sts(status, "AffineTransformWarp_KxParalleWarpAffineLinearQuadImgae", hCall))
	{
		return -1;
	}


	return 0;

}

int CKxWarpStretch::PerspectiveTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	IppiSize srcSize = { nWidth, nHeight };     //待检测大图size
	IppiRect srcRoi = { 0, 0, nWidth, nHeight };//待检测大图ROI
	double matchQuad[_Max_Kern][2] = { 0 }; //匹配到的点，坐标已变换到待检测图上
	double kernQuad[_Max_Kern][2] = { 0 };  //模板图像上定位核的LeftTop点
	float fRatio[_Max_Kern] = { 0 };
	parallel_for(blocked_range<int>(0, m_hParameter.m_nKernCount),
		[&](const blocked_range<int>& range)
	{
		for (int i = range.begin(); i < range.end(); i++)
		//for (int i = 0; i < m_hParameter.m_nKernCount; i++)
		{
			IppiRect searchRoi;
			int nleft, ntop, nright, nbottom;
			if (m_hParameter.m_nKernelDirect[i] == _Horiz_Vertical_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[i].GetLeft() - m_hParameter.m_nSearchExpand);
				ntop = gMax(0, m_hParameter.m_rcKern[i].GetTop() - m_hParameter.m_nSearchExpand);
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[i].GetRight() + m_hParameter.m_nSearchExpand);
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[i].GetBottom() + m_hParameter.m_nSearchExpand);
			}
			if (m_hParameter.m_nKernelDirect[i] == _Vertical_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[i].GetLeft());
				ntop = gMax(0, m_hParameter.m_rcKern[i].GetTop() - m_hParameter.m_nSearchExpand);
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[i].GetRight());
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[i].GetBottom() + m_hParameter.m_nSearchExpand);
			}
			if (m_hParameter.m_nKernelDirect[i] == _Horizontal_Dir)
			{
				nleft = gMax(0, m_hParameter.m_rcKern[i].GetLeft() - m_hParameter.m_nSearchExpand);
				ntop = gMax(0, m_hParameter.m_rcKern[i].GetTop());
				nright = gMin(nWidth - 1, m_hParameter.m_rcKern[i].GetRight() + m_hParameter.m_nSearchExpand);
				nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[i].GetBottom());
			}
			searchRoi.x = nleft;
			searchRoi.y = ntop;
			searchRoi.width = nright - nleft + 1;
			searchRoi.height = nbottom - ntop + 1;

			kxPoint<float>  pos;

			//2018/05/10
			//fRatio[i] = m_hFun[i].kxImageAlign(pos, pBuf + searchRoi.y*nPitch + searchRoi.x*nChannel, searchRoi.width, searchRoi.height, nPitch,
			//	m_KernBuf[i].buf, m_KernBuf[i].nWidth, m_KernBuf[i].nHeight, m_KernBuf[i].nPitch, nChannel, hTempCall);
			//2018/05/10  芯片定位加速
			kxCImageBuf imgSearch;
			imgSearch.SetImageBuf(pBuf + searchRoi.y*nPitch + searchRoi.x*nChannel, searchRoi.width, searchRoi.height, nPitch, nChannel, false);
			fRatio[i] = m_hFun[i].kxImageAlignColor(pos, imgSearch, m_KernBuf[i]);

			if (pos.x < 0 || pos.x >= searchRoi.width || pos.y < 0 || pos.y >= searchRoi.height)
			{
				fRatio[i] = 0.0f;
			}

			pos.x += searchRoi.x;
			pos.y += searchRoi.y;
			matchQuad[i][0] = pos.x;
			matchQuad[i][1] = pos.y;

			kernQuad[i][0] = m_hParameter.m_rcKern[i].left;
			kernQuad[i][1] = m_hParameter.m_rcKern[i].top;
		}
	}, auto_partitioner());

	for (int i = 0; i < m_hParameter.m_nKernCount; i++)
	{
		if (fRatio[i] < gMinSimilarity)
		{
			return i + 1;
		}
	}
	//第一个定位核的实际位置
	m_nKernX = int(matchQuad[0][0] - m_hParameter.m_rcKern[0].left);
	m_nKernY = int(matchQuad[0][1] - m_hParameter.m_rcKern[0].top);

	double coeffsFine[3][3];
	GetPerspectiveTransform_8(kernQuad, matchQuad, coeffsFine, hTempCall);

	if (check_sts(hTempCall, "PerspectiveTransformWarp", hCall))
	{
		return -1;
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			m_coeffsFinePerspective[i][j] = coeffsFine[i][j];
		}
	}

	IppiRect warpRoi;
	warpRoi.x = m_hParameter.m_rcCheckArea.left;
	warpRoi.y = m_hParameter.m_rcCheckArea.top;
	warpRoi.width = m_hParameter.m_rcCheckArea.Width();
	warpRoi.height = m_hParameter.m_rcCheckArea.Height();
	double warpSrcQuad[4][2];
	IppStatus status = ippiGetPerspectiveQuad(warpRoi, warpSrcQuad, coeffsFine);

	if (check_sts(status, "PerspectiveTransformWarp_ippiGetPerspectiveQuad", hCall))
	{
		return -1;
	}


	IppiRect warpDstRoi = { 0, 0, warpRoi.width, warpRoi.height };
	double warpDstQuad[4][2] = { { 0, 0 }, { warpRoi.width - 1, 0 }, { warpRoi.width - 1, warpRoi.height - 1 }, { 0, warpRoi.height - 1 } };

	m_SrcImg.SetImageBuf(pBuf, nWidth, nHeight, nPitch, nChannel, false);
	m_DstImg.SetImageBuf(pDstBuf, warpDstRoi.width, warpDstRoi.height, nDstPitch, nChannel, false);


	if (nChannel == _Type_G8)
	{
		m_hBaseFun.KxParalleWarpPerspectiveLinearQuadImgae(m_SrcImg, m_DstImg, warpSrcQuad, warpDstQuad, hTempCall, nRowGrain, nColGrain);
		if (check_sts(hTempCall, "PerspectiveTransformWarp_KxWarpPerspectiveLinearQuadImgaeC1", hCall))
		{
			return -1;
		}
	}
	if (nChannel == _Type_G24)
	{
		m_hBaseFun.KxParalleWarpPerspectiveLinearQuadImgae(m_SrcImg, m_DstImg, warpSrcQuad, warpDstQuad, hTempCall, nRowGrain, nColGrain);
		if (check_sts(hTempCall, "PerspectiveTransformWarp_KxWarpPerspectiveLinearQuadImgaeC3", hCall))
		{
			return -1;
		}
	}

	return 0;
}


//int CKxWarpStretch::BilinearTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, int nRowGrain, int nColGrain, KxCallStatus& hCall)
//{
//	hCall.Clear();
//	KxCallStatus hTmpCall;
//	hTmpCall.Clear();
//
//	IppiSize srcSize = { nWidth, nHeight };     //待检测大图size
//	IppiRect srcRoi = { 0, 0, nWidth, nHeight };//待检测大图ROI
//	double matchQuad[_Max_Kern][2] = { 0 }; //匹配到的点，坐标已变换到待检测图上
//	double kernQuad[_Max_Kern][2] = { 0 };  //模板图像上定位核的LeftTop点
//
//	for (int i = 0; i < m_hParameter.m_nKernCount; i++)
//	{
//		IppiRect searchRoi;
//		int nleft, ntop, nright, nbottom;
//		if (m_hParameter.m_nKernelDirect[i] == _Horiz_Vertical_Dir)
//		{
//			nleft = gMax(0, m_hParameter.m_rcKern[i].GetLeft() - m_hParameter.m_nSearchExpand);
//			ntop = gMax(0, m_hParameter.m_rcKern[i].GetTop() - m_hParameter.m_nSearchExpand);
//			nright = gMin(nWidth - 1, m_hParameter.m_rcKern[i].GetRight() + m_hParameter.m_nSearchExpand);
//			nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[i].GetBottom() + m_hParameter.m_nSearchExpand);
//		}
//		if (m_hParameter.m_nKernelDirect[i] == _Vertical_Dir)
//		{
//			nleft = gMax(0, m_hParameter.m_rcKern[i].GetLeft());
//			ntop = gMax(0, m_hParameter.m_rcKern[i].GetTop() - m_hParameter.m_nSearchExpand);
//			nright = gMin(nWidth - 1, m_hParameter.m_rcKern[i].GetRight());
//			nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[i].GetBottom() + m_hParameter.m_nSearchExpand);
//		}
//		if (m_hParameter.m_nKernelDirect[i] == _Horizontal_Dir)
//		{
//			nleft = gMax(0, m_hParameter.m_rcKern[0].GetLeft() - m_hParameter.m_nSearchExpand);
//			ntop = gMax(0, m_hParameter.m_rcKern[0].GetTop());
//			nright = gMin(nWidth - 1, m_hParameter.m_rcKern[i].GetRight() + m_hParameter.m_nSearchExpand);
//			nbottom = gMin(nHeight - 1, m_hParameter.m_rcKern[i].GetBottom());
//		}
//		searchRoi.x = nleft;
//		searchRoi.y = ntop;
//		searchRoi.width = nright - nleft + 1;
//		searchRoi.height = nbottom - ntop + 1;
//
//		kxPoint<float>  pos;
//
//
//		float fratio = m_hBaseFun.kxImageAlign(pos, pBuf + searchRoi.y*nPitch + searchRoi.x*nChannel, searchRoi.width, searchRoi.height, nPitch,
//			m_KernBuf[i].buf, m_KernBuf[i].nWidth, m_KernBuf[i].nHeight, m_KernBuf[i].nPitch, nChannel, hTmpCall);
//
//		if (check_sts(hTmpCall, "BilinearTransformWarp", hCall))
//		{
//			return -1;
//		}
//
//
//		if (fratio < gMinSimilarity)
//		{
//			return i + 1;
//		}
//		pos.x += searchRoi.x;
//		pos.y += searchRoi.y;
//		matchQuad[i][0] = pos.x;
//		matchQuad[i][1] = pos.y;
//
//		kernQuad[i][0] = m_hParameter.m_rcKern[i].left;
//		kernQuad[i][1] = m_hParameter.m_rcKern[i].top;
//	}
//
//	//第一个定位核的实际位置
//	m_nKernX = int(matchQuad[0][0] > 0 ? matchQuad[0][0] : 0);
//	m_nKernY = int(matchQuad[0][1] > 0 ? matchQuad[0][1] : 0);
//
//	double coeffsFine[2][4];
//	GetBilinearTransform_8(kernQuad, matchQuad, coeffsFine, hTmpCall);
//
//	if (check_sts(hTmpCall, "BilinearTransformWarp", hCall))
//	{
//		return -1;
//	}
//
//for (int i = 0; i < 2; i++)
//{
//	for (int j = 0; j < 4; j++)
//	{
//		m_coeffsFineBilinear[i][j] = coeffsFine[i][j];
//	}
//}
//	IppiRect warpRoi;
//	warpRoi.x = m_hParameter.m_rcCheckArea.left;
//	warpRoi.y = m_hParameter.m_rcCheckArea.top;
//	warpRoi.width = m_hParameter.m_rcCheckArea.Width();
//	warpRoi.height = m_hParameter.m_rcCheckArea.Height();
//	double warpSrcQuad[4][2];
//	IppStatus status = ippiGetBilinearQuad(warpRoi, warpSrcQuad, coeffsFine);
//
//	if (check_sts(status, "BilinearTransformWarp_ippiGetBilinearQuad", hCall))
//	{
//		return -1;
//	}
//
//	IppiRect warpDstRoi = { 0, 0, warpRoi.width, warpRoi.height };
//	double warpDstQuad[4][2] = { { 0, 0 }, { warpRoi.width - 1, 0 }, { warpRoi.width - 1, warpRoi.height - 1 }, { 0, warpRoi.height - 1 } };
//
//	int nBufferSize;
//	status = ippiWarpBilinearQuadGetBufferSize(srcSize, srcRoi, warpSrcQuad, warpDstRoi, warpDstQuad, IPPI_INTER_LINEAR, &nBufferSize);
//	if (check_sts(status, "BilinearTransformWarp_ippiWarpBilinearQuadGetBufferSize", hCall))
//	{
//		return -1;
//	}
//	Ipp8u* pBuffer = ippsMalloc_8u(nBufferSize);
//
//	if (nChannel == _Type_G8)
//	{
//		status = ippiWarpBilinearQuad_8u_C1R(pBuf, srcSize, nPitch, srcRoi, warpSrcQuad,
//			pDstBuf, nDstPitch, warpDstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
//
//		if (check_sts(status, "BilinearTransformWarp_ippiWarpBilinearQuad_8u_C1R", hCall))
//		{
//			ippsFree(pBuffer);
//			return -1;
//		}
//	}
//	if (nChannel == _Type_G24)
//	{
//		status = ippiWarpBilinearQuad_8u_C3R(pBuf, srcSize, nPitch, srcRoi, warpSrcQuad,
//			pDstBuf, nDstPitch, warpDstRoi, warpDstQuad, IPPI_INTER_LINEAR, pBuffer);
//
//		if (check_sts(status, "BilinearTransformWarp_ippiWarpBilinearQuad_8u_C3R", hCall))
//		{
//			ippsFree(pBuffer);
//			return -1;
//		}
//	}
//
//	ippsFree(pBuffer);
//
//	return 0;
//}

int CKxWarpStretch::GetRightPos(kxRect<int> & rcSrc, kxRect<int>& rcDst)
{
	double quad[4][2];
	IppiRect kernRect;
	bool bFlag = true;
	switch (m_hParameter.m_nKernCount)
	{
	case 0:
	{
			rcDst = rcSrc;
			bFlag = false;
			break;
	}
	case 1:
	{
			rcDst = rcSrc;
			rcDst.offset(m_ptOffset.x, m_ptOffset.y);
			bFlag = false;
			break;
	}
	case 2:
	case 3:
	{
			  kernRect.x = rcSrc.left;
			  kernRect.y = rcSrc.top;
			  kernRect.width = rcSrc.Width();
			  kernRect.height = rcSrc.Height();
			  IppStatus status = ippiGetAffineQuad(kernRect, quad, m_coeffsFineAffine);
			  break;

	}
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	{
			  if (_Perspective_Trans == m_hParameter.m_nAlgorithmType)
			  {
				  kernRect.x = rcSrc.left;
				  kernRect.y = rcSrc.top;
				  kernRect.width = rcSrc.Width();
				  kernRect.height = rcSrc.Height();
				  IppStatus status = ippiGetPerspectiveQuad(kernRect, quad, m_coeffsFinePerspective);

			  }
			  if (_Bilinear_Trans == m_hParameter.m_nAlgorithmType)
			  {
				  kernRect.x = rcSrc.left;
				  kernRect.y = rcSrc.top;
				  kernRect.width = rcSrc.Width();
				  kernRect.height = rcSrc.Height();
				  IppStatus status = ippiGetBilinearQuad(kernRect, quad, m_coeffsFineBilinear);
			  }
			  break;

	}
	default:
		break;

	}

	if (bFlag)
	{
		double xMin = INT_MAX, xMax = -INT_MAX, yMin = INT_MAX, yMax = -INT_MAX;
		for (int i = 0; i < 4; i++)
		{
			if (quad[i][0] < xMin)
			{
				xMin = quad[i][0];
			}
			if (quad[i][0] > xMax)
			{
				xMax = quad[i][0];
			}
			if (quad[i][1] < yMin)
			{
				yMin = quad[i][1];
			}
			if (quad[i][1] > yMax)
			{
				yMax = quad[i][1];
			}

		}

		rcDst.setup((int)xMin, (int)yMin, (int)xMax, (int)yMax);
	}


	return 1;
}
