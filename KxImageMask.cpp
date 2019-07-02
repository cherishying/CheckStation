

#include "KxImageMask.h"
#include "KxReadXml.h"


CKxImageMask::CKxImageMask()
{

}

CKxImageMask::~CKxImageMask()
{

}

bool CKxImageMask::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "掩模设置", "个数", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus;
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nMaskCount);
	if (!nStatus)
	{
		return false;
	}

	if (m_hParameter.m_nMaskCount == 0)
	{
		return true;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "掩模设置", "掩模位置", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulKxRect(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_rcMask);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "掩模设置", "掩模定位框使用情况", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulInt(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_nIsMaskKernel);
	if (!nStatus)
	{
		return false;
	}


	KxXmlFun::SearchXmlGetValue(filePath, "掩模设置", "掩模定位框位置", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulKxRect(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_rcMaskKernel);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "掩模设置", "掩模定位框方向", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulInt(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_nMaskKernelDirect);
	if (!nStatus)
	{
		return false;
	}


	//一般参数
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "掩模设置", "扩展数", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nKernelEx);
		if (!nStatus)
		{
			return false;
		}
	}


	//截取定位核
	for (int i = 0; i < m_hParameter.m_nMaskCount; i++)
	{
		if (m_hParameter.m_nIsMaskKernel[i])
		{
			m_ImgMaskKern[i].Init(m_hParameter.m_rcMaskKernel[i].Width(), m_hParameter.m_rcMaskKernel[i].Height(), BaseImg.nChannel);
			m_hBaseFun.KxCopyImage(BaseImg, m_ImgMaskKern[i], m_hParameter.m_rcMaskKernel[i]);

//			if (gTest)
//			{
//				char sz[128];
//				sprintf_s(sz, 128, "d:\\Test\\m_ImgMaskKern_%d.bmp", i);
//				m_hBaseFun.SaveBMPImage_h(sz, m_ImgMaskKern[i]);
//			}

		}
	}

	return true;

}


bool CKxImageMask::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "MaskSetting", "Num", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	int nStatus;
	nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nMaskCount);
	if (!nStatus)
	{
		return false;
	}

	if (m_hParameter.m_nMaskCount == 0)
	{
		return true;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "MaskSetting", "MaskLocation", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulKxRect(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_rcMask);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "MaskSetting", "MaskLocateBoxUseTime", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulInt(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_nIsMaskKernel);
	if (!nStatus)
	{
		return false;
	}


	KxXmlFun::SearchXmlGetValue(filePath, "MaskSetting", "MaskLocateBoxPos", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulKxRect(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_rcMaskKernel);
	if (!nStatus)
	{
		return false;
	}


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "MaskSetting", "MaskLocateBoxDirection", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	nStatus = KxXmlFun::FromStringToMulInt(szResult, m_hParameter.m_nMaskCount, m_hParameter.m_nMaskKernelDirect);
	if (!nStatus)
	{
		return false;
	}


	//一般参数
	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "MaskSetting", "SearchRange", szResult);
	if (nSearchStatus)
	{
		nStatus = KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nKernelEx);
		if (!nStatus)
		{
			return false;
		}
	}


	//截取定位核
	for (int i = 0; i < m_hParameter.m_nMaskCount; i++)
	{
		if (m_hParameter.m_nIsMaskKernel[i])
		{
			m_ImgMaskKern[i].Init(m_hParameter.m_rcMaskKernel[i].Width(), m_hParameter.m_rcMaskKernel[i].Height(), BaseImg.nChannel);
			m_hBaseFun.KxCopyImage(BaseImg, m_ImgMaskKern[i], m_hParameter.m_rcMaskKernel[i]);

//			if (gTest)
//			{
//				char sz[128];
//				sprintf_s(sz, 128, "d:\\Test\\m_ImgMaskKern_%d.bmp", i);
//				m_hBaseFun.SaveBMPImage_h(sz, m_ImgMaskKern[i]);
//			}

		}
	}

	return true;

}


//从网络中获取参数
bool CKxImageMask::ReadParaFromNet( unsigned char*& point )
{
	memcpy(&m_hParameter, point, sizeof(Parameter));
	point += sizeof(Parameter);
	CKxBaseFunction hBaseFun;

	for (int i = 0; i < _Max_Mask; i++)
	{
		if( !hBaseFun.readImgBufFromMemory(m_ImgMaskKern[i], point) )
		{
			return false;
		}
	}

	return true;
}


bool CKxImageMask::ReadVesion1Para( FILE* fp)    //读取版本1参数
{
	if (fread(&m_hParameter.m_nKernelEx, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nMaskCount, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcMask, sizeof(kxRect<int>), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nIsMaskKernel, sizeof(int), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}

	if (fread(&m_hParameter.m_rcMaskKernel, sizeof(kxRect<int>), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nMaskKernelDirect, sizeof(int), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}

	if (fread(&m_hParameter.m_nIsOpenAutoMask, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nCreateMaskLayer, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nMaskPreProcess, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nOpenGradient, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nThresh, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nComplement, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	for (int i = 0; i < _Max_Mask; i++)
	{
		if ( !m_ImgMaskKern[i].Read(fp))
		{
			return false;
		}
	}

	return true;

}


bool CKxImageMask::Read( FILE*  fp)
{
	if (fread(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_hParameter.m_szVersion, "ImageMask1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}

}

bool CKxImageMask::WriteVesion1Para( FILE* fp)    //读取版本1参数
{
	if (fwrite(&m_hParameter.m_nKernelEx, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nMaskCount, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcMask, sizeof(kxRect<int>), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nIsMaskKernel, sizeof(int), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcMaskKernel, sizeof(kxRect<int>), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nMaskKernelDirect, sizeof(int), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}

	if (fwrite(&m_hParameter.m_nIsOpenAutoMask, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nCreateMaskLayer, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nMaskPreProcess, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nOpenGradient, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nThresh, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nComplement, sizeof(int), 1, fp) != 1)
	{
		return false;
	}

	for (int i = 0; i < _Max_Mask; i++)
	{
		if ( !m_ImgMaskKern[i].Write(fp))
		{
			return false;
		}
	}

	return true;

}

bool CKxImageMask::Write( FILE*  fp)
{
	if (strcmp(m_hParameter.m_szVersion, "ImageMask1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return WriteVesion1Para(fp);
	}
	else
	{
		return false;
	}
}

int CKxImageMask::Mask(kxCImageBuf& SrcImg, kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal)
{
	KxCallStatus hCall;
	return Mask(SrcImg, KernImg, rcCheckArea, nMaskVal, hCall);
}

//SrcImg 待Mask图像，一般是残差图，为灰度图
//KernImg 为定位图像，一般为校正后的彩色图
//rc为处理区域,由于参数里记录的都是底板的坐标，需用这个区域做偏移
int CKxImageMask::Mask(kxCImageBuf& SrcImg,  kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	//是否启动自动Mask
	if (m_hParameter.m_nIsOpenAutoMask)
	{
		if (nMaskVal)
		{
			//check low, mask area should replace 255
			m_Img.Init(m_AutoMaskImg.nWidth, m_AutoMaskImg.nHeight);
			IppiSize Roi = {m_Img.nWidth, m_Img.nHeight};
			IppStatus status = ippiAbsDiffC_8u_C1R(m_AutoMaskImg.buf, m_AutoMaskImg.nPitch, m_Img.buf, m_Img.nPitch, Roi, 255);

			if (check_sts(status, "Mask_ippiAbsDiffC_8u_C1R", hCall))
			{
				return 0;
			}
			status = ippiAdd_8u_C1IRSfs(m_Img.buf, m_Img.nPitch, SrcImg.buf, SrcImg.nPitch, Roi, 0);

			if (check_sts(status, "Mask_ippiAdd_8u_C1IRSfs_1", hCall))
			{
				return 0;
			}
		}
		else
		{
			IppiSize Roi = {m_AutoMaskImg.nWidth, m_AutoMaskImg.nHeight};
			IppStatus status = ippiAnd_8u_C1IR(m_AutoMaskImg.buf, m_AutoMaskImg.nPitch, SrcImg.buf, SrcImg.nPitch, Roi);

			if (check_sts(status, "Mask_ippiAdd_8u_C1IRSfs_2", hCall))
			{
				return 0;
			}
		}
	}

	if (m_hParameter.m_nMaskCount > _Max_Mask)
	{
		return 0;
	}

    float nr = KernImg.nWidth *1.0f / SrcImg.nWidth ;

	for (int i = 0;i < m_hParameter.m_nMaskCount; i++)
	{
		kxRect<int> rcMask = m_hParameter.m_rcMask[i];
		rcMask.offset(-rcCheckArea.left, -rcCheckArea.top);

		rcMask.left = (rcMask.left < 0) ? 0:rcMask.left;
		rcMask.top = (rcMask.top < 0) ? 0:rcMask.top;
		rcMask.right = (rcMask.right > rcCheckArea.Width()-1 ? rcCheckArea.Width()-1:rcMask.right);
		rcMask.bottom = (rcMask.bottom > rcCheckArea.Height()-1 ? rcCheckArea.Height()-1:rcMask.bottom);

		if (rcMask.right  < rcMask.left || rcMask.bottom < rcMask.top)
		{
			continue;
		}

		int nDx = 0;
		int nDy = 0;


		if (m_hParameter.m_nIsMaskKernel[i])
		{
			kxRect<int> rcKern = m_hParameter.m_rcMaskKernel[i];
			rcKern.offset(-rcCheckArea.left, -rcCheckArea.top);
			rcKern.left = (rcKern.left < 0) ? 0:rcKern.left;
			rcKern.top = (rcKern.top < 0) ? 0:rcKern.top;
			rcKern.right = (rcKern.right > rcCheckArea.Width()-1 ? rcCheckArea.Width()-1:rcKern.right);
			rcKern.bottom = (rcKern.bottom > rcCheckArea.Height()-1 ? rcCheckArea.Height()-1:rcKern.bottom);
			if (rcKern.right  < rcKern.left || rcKern.bottom < rcKern.top)
			{
				continue;
			}
			m_hBaseFun.GetImgOffset(KernImg, m_ImgMaskKern[i], rcKern, m_hParameter.m_nKernelEx, m_hParameter.m_nMaskKernelDirect[i], nDx, nDy, hTempCall);
			if (check_sts(hTempCall, "Mask", hCall))
			{
				return 0;
			}
		}

		IppiSize roi = {int(rcMask.Width()/nr), int(rcMask.Height()/nr)};
		//if (rcMask.left + nDx < 0){ nDx = 0 - rcMask.left;}
		//if (int((rcMask.right + nDx)/nr) > SrcImg.nWidth-1){ roi.width = SrcImg.nWidth - int((rcMask.left + nDx)/nr);}
		//if (rcMask.top + nDy < 0){ nDy = 0 - rcMask.top;}
		//if (int((rcMask.bottom + nDy)/nr) > SrcImg.nHeight-1){ roi.height = SrcImg.nHeight - int((rcMask.top + nDy)/nr);}
		//m_hBaseFun.SaveBMPImage_h("d:\\Bmask.bmp", SrcImg);
		//int nOffset = int((rcMask.left + nDx)/nr*SrcImg.nChannel+ (rcMask.top + nDy)/nr * SrcImg.nPitch);
		int nTop = int((rcMask.top + nDy) / nr);
		int nLeft = int((rcMask.left + nDx) / nr);
		nTop = gMax(0, nTop);
		nLeft = gMax(0, nLeft);
		roi.width = gMin(SrcImg.nWidth - nLeft, roi.width);
		roi.height = gMin(SrcImg.nHeight - nTop, roi.height);

		IppStatus status;
		if (SrcImg.nChannel == _Type_G8)
		{
			status = ippiSet_8u_C1R(nMaskVal, SrcImg.buf + nTop*SrcImg.nPitch + nLeft, SrcImg.nPitch, roi);
		}
		else
		{
			Ipp8u value[3];
			for (int i = 0; i < 3; i++)
			{
				value[i] = nMaskVal;
			}
			status = ippiSet_8u_C3R(value, SrcImg.buf + nTop*SrcImg.nPitch + nLeft*3, SrcImg.nPitch, roi);
		}

		if (check_sts(status, "Mask_ippiSet_8u_C1R", hCall))
		{
			return 0;
		}

		//m_hBaseFun.SaveBMPImage_h("d:\\Amask.bmp", SrcImg);
	}

	return 1;


}

int CKxImageMask::GetImgOffset(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, int nImgType, int nIndex, int& nDx, int& nDy)
{
	kxRect<int> rcSearch;
	int nleft,nright,ntop,nbottom;
	if (m_hParameter.m_nMaskKernelDirect[nIndex] == _Horiz_Vertical_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetLeft() - m_hParameter.m_nKernelEx);
		ntop  = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetTop() - m_hParameter.m_nKernelEx);
		nright = gMin(nWidth-1, m_hParameter.m_rcMaskKernel[nIndex].GetRight() + m_hParameter.m_nKernelEx);
		nbottom = gMin(nHeight-1, m_hParameter.m_rcMaskKernel[nIndex].GetBottom() + m_hParameter.m_nKernelEx);
	}
	if (m_hParameter.m_nMaskKernelDirect[nIndex] == _Vertical_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetLeft());
		ntop = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetTop() - m_hParameter.m_nKernelEx);
		nright = gMin(nWidth-1, m_hParameter.m_rcMaskKernel[nIndex].GetRight());
		nbottom = gMin(nHeight-1, m_hParameter.m_rcMaskKernel[nIndex].GetBottom()+ m_hParameter.m_nKernelEx);
	}
	if (m_hParameter.m_nMaskKernelDirect[nIndex] == _Horizontal_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetLeft()- m_hParameter.m_nKernelEx);
		ntop = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetTop());
		nright = gMin(nWidth-1, m_hParameter.m_rcMaskKernel[nIndex].GetRight() + m_hParameter.m_nKernelEx);
		nbottom = gMin(nHeight-1, m_hParameter.m_rcMaskKernel[nIndex].GetBottom());
	}

	rcSearch.setup(nleft,ntop,nright,nbottom);
	//-------------------------------------------------------------------------------------------------
	kxPoint<float>  pos;
	int nChannel = (nImgType == 0 ? 1:3);

	float fratio = m_hBaseFun.kxImageAlign( pos, pBuf + rcSearch.GetTop()*nPitch+rcSearch.GetLeft()*nChannel, rcSearch.Width(), rcSearch.Height(), nPitch,
		m_ImgMaskKern[nIndex].buf, m_ImgMaskKern[nIndex].nWidth, m_ImgMaskKern[nIndex].nHeight, m_ImgMaskKern[nIndex].nPitch , nImgType);

	if (fratio < gMinSimilarity)
	{
		nDx = 0;
		nDy = 0;
		return 0;
	}

	nDx = int(rcSearch.GetLeft() + pos.x - m_hParameter.m_rcMaskKernel[nIndex].left);
	nDy = int(rcSearch.GetTop() + pos.y - m_hParameter.m_rcMaskKernel[nIndex].top);

	return 1;
}




int CKxImageMask::AutoCreateMask(kxCImageBuf& Srcimg)
{
	int nWidth = Srcimg.nWidth;
	int nHeight = Srcimg.nHeight;
	m_ImgPre0.Init(nWidth, nHeight);
	m_hBaseFun.KxConvertImageLayer(Srcimg, m_ImgPre0, m_hParameter.m_nCreateMaskLayer);

	//预处理
	m_ImgPre1.Init(m_ImgPre0.nWidth, m_ImgPre0.nHeight);


	if (m_hParameter.m_nMaskPreProcess == 0)  //均值滤波
	{
		m_hBaseFun.KxMedianFilterImage(m_ImgPre0, m_ImgPre1, 5, 5);
	}
	else if (m_hParameter.m_nMaskPreProcess == 1)
	{
		m_hBaseFun.KxErodeImage(m_ImgPre0, m_ImgPre1, 5, 5);
	}
	else if (m_hParameter.m_nMaskPreProcess == 2)
	{
		m_hBaseFun.KxDilateImage(m_ImgPre0, m_ImgPre1, 5, 5);

	}
	else
	{

	}

	m_ImgPre2.Init(m_ImgPre1.nWidth, m_ImgPre1.nHeight);
	if (m_hParameter.m_nOpenGradient)
	{
		m_hGradient.ComputeGrand(m_ImgPre1,	m_ImgPre2);
	}else
	{
		IppiSize roi = {m_ImgPre1.nWidth, m_ImgPre1.nHeight};
		ippiCopy_8u_C1R(m_ImgPre1.buf, m_ImgPre1.nPitch, m_ImgPre2.buf, m_ImgPre2.nPitch, roi);
	}


	//二值化
	IppiSize SrcRoi = {m_ImgPre2.nWidth, m_ImgPre2.nHeight};
	m_ImgPre3.Init(m_ImgPre2.nWidth, m_ImgPre2.nHeight);
	ippiCompareC_8u_C1R(m_ImgPre2.buf, m_ImgPre2.nPitch, (Ipp8u)m_hParameter.m_nThresh, m_ImgPre3.buf, m_ImgPre3.nPitch, SrcRoi, ippCmpGreaterEq);

	m_AutoMaskImg.Init(m_ImgPre3.nWidth, m_ImgPre3.nHeight);
	//取反
	if (m_hParameter.m_nComplement)
	{
		ippiAbsDiffC_8u_C1R(m_ImgPre3.buf, m_ImgPre3.nPitch, m_AutoMaskImg.buf, m_AutoMaskImg.nPitch, SrcRoi, 255);
	}
	else
	{
		ippiCopy_8u_C1R(m_ImgPre3.buf, m_ImgPre3.nPitch, m_AutoMaskImg.buf, m_AutoMaskImg.nPitch, SrcRoi);
	}

	return 1;

}

int CKxImageMask::ShowMaskImage(kxCImageBuf& BaseImg, kxRect<int>& rcPos)
{
	if (BaseImg.nWidth <= 0 || BaseImg.nHeight <= 0 )
	{
		m_AutoMaskImg.Init(10,10);
		ippsSet_8u(0, m_AutoMaskImg.buf, m_AutoMaskImg.nWidth*m_AutoMaskImg.nPitch);
		return 0;
	}

	kxCImageBuf TmpImg;
	TmpImg.Init(rcPos.Width(), rcPos.Height(), BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(BaseImg, TmpImg, rcPos);
	AutoCreateMask(TmpImg);

	return 1;
}
