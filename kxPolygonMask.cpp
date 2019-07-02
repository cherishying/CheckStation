
#include "kxPolygonMask.h"
#include <limits.h>
CkxPolygonMask::CkxPolygonMask()
{
}

CkxPolygonMask::~CkxPolygonMask()
{
}

//从网络中获取参数
bool CkxPolygonMask::ReadParaFromNet(unsigned char*& point)
{
	memcpy(&m_hParameter, point, sizeof(Parameter));
	point += sizeof(Parameter);
	CKxBaseFunction hBaseFun;

	for (int i = 0; i < _Max_Mask; i++)
	{
		if (!hBaseFun.readImgBufFromMemory(m_ImgMaskKern[i], point))
		{
			return false;
		}
	}

	return true;
}


bool CkxPolygonMask::ReadVesion1Para(FILE* fp)    //读取版本1参数
{
	if (fread(&m_hParameter.m_nKernelEx, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nMaskCount, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fread(&m_hParameter.m_nEdgeCount, sizeof(int), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcMask, sizeof(kxPoint<int>), _Max_Mask*16, fp) != _Max_Mask * 16)
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

	//if (fread(&m_hParameter.m_nIsOpenAutoMask, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fread(&m_hParameter.m_nCreateMaskLayer, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fread(&m_hParameter.m_nMaskPreProcess, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fread(&m_hParameter.m_nOpenGradient, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fread(&m_hParameter.m_nThresh, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fread(&m_hParameter.m_nComplement, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}

	for (int i = 0; i < _Max_Mask; i++)
	{
		if (!m_ImgMaskKern[i].Read(fp))
		{
			return false;
		}
	}

	for(int i = 0;i < m_hParameter.m_nMaskCount; i++)
	{
		int maxX = 0;
		int maxY = 0;
		int minX = INT_MAX;
		int minY = INT_MAX;
		for(int j = 0; j < m_hParameter.m_nEdgeCount[i]; j++)
		{
			if (m_hParameter.m_rcMask[i][j].x > maxX)
			{
				maxX = m_hParameter.m_rcMask[i][j].x;
			}				
			if (m_hParameter.m_rcMask[i][j].y > maxY)
			{
				maxY = m_hParameter.m_rcMask[i][j].y;
			}				
			if (m_hParameter.m_rcMask[i][j].x < minX)
			{
				minX = m_hParameter.m_rcMask[i][j].x;
			}
			if (m_hParameter.m_rcMask[i][j].y < minY)
			{
				minY = m_hParameter.m_rcMask[i][j].y;
			}
		}
		m_rcPolyMask[i].setup(minX, minY, maxX, maxY);

		int w = maxX - minX + 1;
		int h = maxY - minY + 1;
		m_PolygonMaskImg[i].Init(w, h);
		memset(m_PolygonMaskImg[i].buf, 0xff, m_PolygonMaskImg[i].nWidth * m_PolygonMaskImg[i].nHeight);

		//m_hBaseFun.SaveBMPImage_h("d:\\PloyMask1.bmp", m_PolygonMaskImg[i]);
		kxPoint<int> pts[16];
		for (int k = 0; k < m_hParameter.m_nEdgeCount[i]; k++)
		{
			pts[k].x = m_hParameter.m_rcMask[i][k].x - minX;
			pts[k].y = m_hParameter.m_rcMask[i][k].y - minY;

		}
		VerticesToImage(pts, m_hParameter.m_nEdgeCount[i], m_PolygonMaskImg[i].buf, m_PolygonMaskImg[i].nWidth, m_PolygonMaskImg[i].nHeight, m_PolygonMaskImg[i].nWidth);

		//m_hBaseFun.SaveBMPImage_h("d:\\PloyMask.bmp", m_PolygonMaskImg[i]);

	}

	return true;
}


bool CkxPolygonMask::Read(FILE*  fp)
{
	if (fread(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_hParameter.m_szVersion, "PolygonMask1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}

}

bool CkxPolygonMask::WriteVesion1Para(FILE* fp)    //读取版本1参数
{
	if (fwrite(&m_hParameter.m_nKernelEx, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nMaskCount, sizeof(int), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nEdgeCount, sizeof(int), _Max_Mask, fp) != _Max_Mask)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcMask, sizeof(int), _Max_Mask*16*2, fp) != _Max_Mask*16*2)
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

	//if (fwrite(&m_hParameter.m_nIsOpenAutoMask, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fwrite(&m_hParameter.m_nCreateMaskLayer, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fwrite(&m_hParameter.m_nMaskPreProcess, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fwrite(&m_hParameter.m_nOpenGradient, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fwrite(&m_hParameter.m_nThresh, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}
	//if (fwrite(&m_hParameter.m_nComplement, sizeof(int), 1, fp) != 1)
	//{
	//	return false;
	//}

	for (int i = 0; i < _Max_Mask; i++)
	{
		if (!m_ImgMaskKern[i].Write(fp))
		{
			return false;
		}
	}

	return true;

}

bool CkxPolygonMask::Write(FILE*  fp)
{
	if (strcmp(m_hParameter.m_szVersion, "PolygonMask1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return WriteVesion1Para(fp);
	}
	else
	{
		return false;
	}
}

int CkxPolygonMask::Mask(kxCImageBuf& SrcImg, kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal)
{
	KxCallStatus hCall;
	return Mask(SrcImg, KernImg, rcCheckArea, nMaskVal, hCall);
}


//SrcImg 待Mask图像，一般是残差图，为灰度图
//KernImg 为定位图像，一般为校正后的彩色图
//rc为处理区域,由于参数里记录的都是底板的坐标，需用这个区域做偏移
int CkxPolygonMask::Mask(kxCImageBuf& SrcImg, kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal, KxCallStatus& hCall)
{
	hCall.Clear();
	KxCallStatus hTempCall;
	hTempCall.Clear();

	if (m_hParameter.m_nMaskCount > _Max_Mask)
		return 0;
	float  nr = KernImg.nWidth *1.0f/ SrcImg.nWidth;

	for (int i = 0;i < m_hParameter.m_nMaskCount; i++)
	{
		kxRect<int> rcMask = m_rcPolyMask[i];
		rcMask.offset(-rcCheckArea.left, -rcCheckArea.top);

		rcMask.left = (rcMask.left < 0) ? 0 : rcMask.left;
		rcMask.top = (rcMask.top < 0) ? 0 : rcMask.top;
		rcMask.right = (rcMask.right > rcCheckArea.Width() - 1 ? rcCheckArea.Width() - 1 : rcMask.right);
		rcMask.bottom = (rcMask.bottom > rcCheckArea.Height() - 1 ? rcCheckArea.Height() - 1 : rcMask.bottom);

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
			rcKern.left = (rcKern.left < 0) ? 0 : rcKern.left;
			rcKern.top = (rcKern.top < 0) ? 0 : rcKern.top;
			rcKern.right = (rcKern.right > rcCheckArea.Width() - 1 ? rcCheckArea.Width() - 1 : rcKern.right);
			rcKern.bottom = (rcKern.bottom > rcCheckArea.Height() - 1 ? rcCheckArea.Height() - 1 : rcKern.bottom);
			if (rcKern.right  < rcKern.left || rcKern.bottom < rcKern.top)
			{
				continue;
			}
			m_hBaseFun.GetImgOffset(KernImg, m_ImgMaskKern[i], rcKern, m_hParameter.m_nKernelEx, m_hParameter.m_nMaskKernelDirect[i], nDx, nDy, hTempCall);
			if (check_sts(hTempCall, "PolygonMask", hCall))
			{
				return 0;
			}
		}

		IppiSize roi = { int (rcMask.Width() / nr), int(rcMask.Height() / nr) };
		//if (rcMask.left + nDx < 0){ nDx = 0 - rcMask.left; }
		//if (int((rcMask.right + nDx) / nr) > SrcImg.nWidth - 1){ roi.width = SrcImg.nWidth - int((rcMask.left + nDx)/nr); }
		//if (rcMask.top + nDy < 0){ nDy = 0 - rcMask.top; }
		//if (int((rcMask.bottom + nDy) / nr) > SrcImg.nHeight - 1){ roi.height = SrcImg.nHeight - int((rcMask.top + nDy)/nr); }

		m_ResizeMaskImg.Init(int(m_PolygonMaskImg[i].nWidth / nr), int(m_PolygonMaskImg[i].nHeight /nr));
		m_hBaseFun.KxResizeImage(m_PolygonMaskImg[i], m_ResizeMaskImg);

		int nTop = int((rcMask.top + nDy) / nr);
		int nLeft = int((rcMask.left + nDx) / nr);
		nTop = gMax(0, nTop);
		nLeft = gMax(0, nLeft);
		roi.width = gMin(SrcImg.nWidth - nLeft, roi.width);
		roi.height = gMin(SrcImg.nHeight - nTop, roi.height);


		int nOffset = nLeft*SrcImg.nChannel + nTop * SrcImg.nPitch;
		IppStatus status;

		status = ippiAnd_8u_C1IR(m_ResizeMaskImg.buf, m_ResizeMaskImg.nPitch, SrcImg.buf + nOffset, SrcImg.nPitch, roi);
		if (check_sts(status, "PolygonMask_ippiAnd_8u_C1IR", hCall))
		{
			return 0;
		}

		//int offsetX1,offsetX2,offsetY1,offsetY2;
		//if(rcCheckArea.left + nDx >= 0)
		//{
		//	offsetX1 = rcCheckArea.left + nDx;
		//	offsetX2 = 0;
		//}
		//else
		//{
		//	offsetX1 = 0;
		//	offsetX2 = -(rcCheckArea.left + nDx);
		//}
		//if(rcCheckArea.top + nDy >= 0)
		//{
		//	offsetY1 = rcCheckArea.top + nDy;
		//	offsetY2 = 0;
		//}
		//else
		//{
		//	offsetY1 = 0;
		//	offsetY2 = -(rcCheckArea.top + nDy);
		//}
		//int offset1 = offsetY1 * m_PolygonMaskImg[i].nPitch + offsetX1;
		//int offset2 = offsetY2 * SrcImg.nPitch + offsetX2;
		//IppiSize roi = {gMin(m_PolygonMaskImg[i].nWidth - offsetX1, SrcImg.nWidth - offsetX2), gMin(m_PolygonMaskImg[i].nHeight - offsetY1, SrcImg.nHeight - offsetY2)};
		//IppStatus status = ippiAnd_8u_C1IR(m_PolygonMaskImg[i].buf + offset1, m_PolygonMaskImg[i].nPitch, SrcImg.buf + offset2, SrcImg.nPitch, roi);
		//if (check_sts(status, "Mask_ippiAdd_8u_C1IRSfs_2", hCall))
		//	return 0;


	}


	return 1;
}

int CkxPolygonMask::GetImgOffset(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, int nImgType, int nIndex, int& nDx, int& nDy)
{
	kxRect<int> rcSearch;
	int nleft, nright, ntop, nbottom;
	if (m_hParameter.m_nMaskKernelDirect[nIndex] == _Horiz_Vertical_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetLeft() - m_hParameter.m_nKernelEx);
		ntop = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetTop() - m_hParameter.m_nKernelEx);
		nright = gMin(nWidth - 1, m_hParameter.m_rcMaskKernel[nIndex].GetRight() + m_hParameter.m_nKernelEx);
		nbottom = gMin(nHeight - 1, m_hParameter.m_rcMaskKernel[nIndex].GetBottom() + m_hParameter.m_nKernelEx);
	}
	if (m_hParameter.m_nMaskKernelDirect[nIndex] == _Vertical_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetLeft());
		ntop = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetTop() - m_hParameter.m_nKernelEx);
		nright = gMin(nWidth - 1, m_hParameter.m_rcMaskKernel[nIndex].GetRight());
		nbottom = gMin(nHeight - 1, m_hParameter.m_rcMaskKernel[nIndex].GetBottom() + m_hParameter.m_nKernelEx);
	}
	if (m_hParameter.m_nMaskKernelDirect[nIndex] == _Horizontal_Dir)
	{
		nleft = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetLeft() - m_hParameter.m_nKernelEx);
		ntop = gMax(0, m_hParameter.m_rcMaskKernel[nIndex].GetTop());
		nright = gMin(nWidth - 1, m_hParameter.m_rcMaskKernel[nIndex].GetRight() + m_hParameter.m_nKernelEx);
		nbottom = gMin(nHeight - 1, m_hParameter.m_rcMaskKernel[nIndex].GetBottom());
	}

	rcSearch.setup(nleft, ntop, nright, nbottom);
	//-------------------------------------------------------------------------------------------------
	kxPoint<float>  pos;
	int nChannel = (nImgType == 0 ? 1 : 3);

	float fratio = m_hBaseFun.kxImageAlign(pos, pBuf + rcSearch.GetTop()*nPitch + rcSearch.GetLeft()*nChannel, rcSearch.Width(), rcSearch.Height(), nPitch,
		m_ImgMaskKern[nIndex].buf, m_ImgMaskKern[nIndex].nWidth, m_ImgMaskKern[nIndex].nHeight, m_ImgMaskKern[nIndex].nPitch, nImgType);

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




int CkxPolygonMask::AutoCreateMask(kxCImageBuf& Srcimg)
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
		m_hGradient.ComputeGrand(m_ImgPre1, m_ImgPre2);
	}
	else
	{
		IppiSize roi = { m_ImgPre1.nWidth, m_ImgPre1.nHeight };
		ippiCopy_8u_C1R(m_ImgPre1.buf, m_ImgPre1.nPitch, m_ImgPre2.buf, m_ImgPre2.nPitch, roi);
	}


	//二值化
	IppiSize SrcRoi = { m_ImgPre2.nWidth, m_ImgPre2.nHeight };
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

int CkxPolygonMask::ShowMaskImage(kxCImageBuf& BaseImg, kxRect<int>& rcPos)
{
	if (BaseImg.nWidth <= 0 || BaseImg.nHeight <= 0)
	{
		m_AutoMaskImg.Init(10, 10);
		ippsSet_8u(0, m_AutoMaskImg.buf, m_AutoMaskImg.nWidth*m_AutoMaskImg.nPitch);
		return 0;
	}

	kxCImageBuf TmpImg;
	TmpImg.Init(rcPos.Width(), rcPos.Height(), BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(BaseImg, TmpImg, rcPos);
	AutoCreateMask(TmpImg);

	return 1;
}

//void CkxPolygonMask::CreatePolyMaskFromVertexs(const kxCImageBuf& SrcImg, kxPoint<int> *Vertex, int nVertexs)
//{
//	//首先算出凸多边形的总面积
//	float s1 = 0, s2 = 0;
//	for (int i = 0; i < nVertexs; i++)
//	{
//		s1 += Vertex[i].x * Vertex[(i + 1) % nVertexs].y * 1.0f;
//		s2 += Vertex[i].x * Vertex[(i + nVertexs - 1) % nVertexs].y * 1.0f;
//	}
//	float fArea = (s1 - s2) / 2;
//
//	for (int i = 0; i < SrcImg.nHeight; i++)
//	{
//		for (int j = 0; j < SrcImg.nWidth; j++)
//		{
//			//计算每个点是否在多边形内部
//			float s = 0;
//			kxPoint<int> nVertexArray[3];
//			nVertexArray[0].setup(j, i); 
//			for (int k = 0; k < nVertexs; k++)
//			{
//				nVertexArray[1] = Vertex[k];
//				nVertexArray[2] = Vertex[(k + 1) % nVertexs];
//				
//			}
//		}
//	}
//}



void CkxPolygonMask::VerticesToImage(kxPoint<int> *Vertex, int nVertices, unsigned char *pBuf, int nWidth, int nHeight, int nPitch)
{
	int NodeX[1<<10], nNodes;

	//循环图像的每一行
	for(int y = 0; y < nHeight; y++)
	{
		//建立节点列表
		nNodes = 0;
		for(int i = 0; i < nVertices; i++)
		{
			int j = (i + nVertices - 1) % nVertices;
			if(Vertex[i].y < y && Vertex[j].y >= y || Vertex[j].y < y && Vertex[i].y >= y)
				NodeX[nNodes++] = (int)(Vertex[i].x+((double)y-Vertex[i].y) / (Vertex[j].y-Vertex[i].y) * (Vertex[j].x-Vertex[i].x));
		}

		//排序节点，通过一个简单的“泡沫”排序
		{
		 int i = 0;
		while(i < nNodes - 1)
		{
			if(NodeX[i] > NodeX[i+1])
			{
				int swap = NodeX[i];
				NodeX[i] = NodeX[i+1];
				NodeX[i+1] = swap;
				if(i)
					i--;
			}
			else
				i++;
		}
		}

		//在节点对之间填充像素
		for(int i = 0; i < nNodes; i += 2)
		{
			if(NodeX[i] >= nWidth)
				break;
			if(NodeX[i+1] > 0)
			{
				if(NodeX[i] < 0)
					NodeX[i] = 0;
				if(NodeX[i+1] > nWidth)
					NodeX[i+1] = nWidth;
				for(int x = NodeX[i]; x < NodeX[i+1]; x++)
					pBuf[y*nPitch+x] = 0;
			}
		}
	}
}


