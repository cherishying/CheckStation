
#include "KxMinRectangle.h"
#if defined( _WIN32 ) || defined ( _WIN64 )
#else
#include "limits.h"
#endif // defined
//#include "ImgDef.h"
//#include "ImgLib.h"


CKxMinRect::CKxMinRect()
{
	m_nCenterX = 0;
	m_nCenterY = 0;
}


CKxMinRect::~CKxMinRect()
{

}

int CKxMinRect::GetCenterPosition(Ipp16s* pLocationX, Ipp16s* pLocationY, int nCount, int& nCenterX, int& nCenterY)
{
	Ipp16s nMin, nMax;
	ippsMinMax_16s(pLocationX, nCount, &nMin, &nMax);
	nCenterX = (nMin + nMax)/2;

	ippsMinMax_16s(pLocationY, nCount, &nMin, &nMax);
	nCenterY = (nMin + nMax)/2;

	return 1;

}

int CKxMinRect::TransFormToOrginalPosition(kxPoint<int> pt1, float fAngle, kxPoint<int>& pt2)
{
	pt2.x = int(cos(fAngle)*(pt1.x + m_nCenterX) - sin(fAngle)*(pt1.y + m_nCenterY) + 0.5);
	pt2.y = int(sin(fAngle)*(pt1.x + m_nCenterX) + cos(fAngle)*(pt1.y + m_nCenterY) + 0.5);
	return 1;
}

int CKxMinRect::GetRectangleArea(Ipp16s* pLocationX, Ipp16s* pLocationY, int nCount, float fangle, float& fArea,  kxRect<int>& rect, Ipp16s* pTmpX, Ipp16s* pTmpY)
{
	memset(pTmpX, 0, sizeof(Ipp16s)*nCount);
	memset(pTmpY, 0, sizeof(Ipp16s)*nCount);

	float alph = cos(fangle);
	float beta = sin(fangle);

	for (int i = 0; i < nCount; i++)
	{
		pTmpX[i] = Ipp16s(alph*pLocationX[i] - beta*pLocationY[i] - m_nCenterX);
		pTmpY[i] = Ipp16s(beta*pLocationX[i] + alph*pLocationY[i] - m_nCenterY);
	}

	Ipp16s nXMin, nXMax, nYMin, nYMax;
	ippsMinMax_16s(pTmpX, nCount, &nXMin, &nXMax);
	ippsMinMax_16s(pTmpY, nCount, &nYMin, &nYMax);

	rect.setup(nXMin, nYMin, nXMax, nYMax);
	fArea = float(rect.Width() * rect.Height());

	return 1;

}

int CKxMinRect::Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pTmp)
{
	m_CopyImg.SetImageBuf(buf, nWidth, nHeight,nPitch, 1, false);
	return Check(m_CopyImg, pTmp);
}

int CKxMinRect::Check(const kxCImageBuf& SrcImg, unsigned char* pTmp)
{
	//first, get the edge's points of the image region
	int nWidth = SrcImg.nWidth;
	int nHeight = SrcImg.nHeight;

    m_Res.Clear();
	if (NULL == pTmp)
	{
		m_TmpImg.Init(nWidth, nHeight);
	}
	//else
	//{
	//	m_TmpImg.buf = pTmp;
	//	m_TmpImg.nPitch = nWidth;
	//}
	IppiSize Roi = {nWidth, nHeight};
    if (nWidth > 10 && nHeight > 10)
    {
		Ipp8u pMask[3*3] = {0,1,0,1,1,1,0,1,0};
		IppiSize MaskSize = {3, 3};
		//IppiPoint anchor;
		//anchor.x = 1;
		//anchor.y = 1;
		//IppiMorphState* pState;
		//ippiMorphologyInitAlloc_8u_C1R(Roi.width, pMask,MaskSize, anchor, &pState);
		//ippiErodeBorderReplicate_8u_C1R(buf, nPitch, m_TmpImg.buf, m_TmpImg.nPitch, Roi, ippBorderRepl, pState);
		//ippiMorphologyFree(pState);
		m_hBaseFun.KxErodeImage(SrcImg, m_TmpImg, 3, 3, pMask);
		ippiXor_8u_C1IR(SrcImg.buf, SrcImg.nPitch, m_TmpImg.buf, m_TmpImg.nPitch, Roi);
    }
	else
	{
		ippiCopy_8u_C1R(SrcImg.buf, SrcImg.nPitch, m_TmpImg.buf, m_TmpImg.nPitch, Roi);
	}


	//MV_IMAGE hImg;
	//hImg.type = MV_IMAGE_TYPE_G8;
	//hImg.data = m_TmpImg.GetImageBuf(hImg.width, hImg.height, hImg.pitch);
	//SaveBMPImage("d:\\qq.bmp", &hImg);
	//CKxBaseFunction hFun;
	//hFun.SaveBMPImage_h("d:\\123.bmp", m_TmpImg);

    int  nCount;
    ippiCountInRange_8u_C1R(m_TmpImg.buf, m_TmpImg.nPitch, Roi, &nCount, 1, 255);
	m_Res.m_nCircumference = nCount;

	Ipp16s* pLocationX, *pLocationY;
	pLocationX = new Ipp16s[nCount];
	pLocationY = new Ipp16s[nCount];
	memset(pLocationX, 0, sizeof(Ipp16s)*nCount);
	memset(pLocationY, 0, sizeof(Ipp16s)*nCount);
	int k = 0;
	for (int i = 0; i < nHeight; i++)
	{
		for (int j = 0; j < nWidth; j++)
		{
			if (*(m_TmpImg.buf+i*nWidth+j))
			{
				pLocationX[k] = j;
				pLocationY[k] = i;
				k++;
			}
		}
	}

	//second, search the min area of all postions
	float fArea, fMinArea;
	fMinArea = float(INT_MAX);
	kxRect<int> rect;
	float ftheta = float(PI/(2*m_Para.m_nParts));
	float fBestAngle;
	kxRect<int> BestRect;
	GetCenterPosition(pLocationX, pLocationY, nCount, m_nCenterX, m_nCenterY);

	Ipp16s* pTmpX = new Ipp16s[nCount];
	Ipp16s* pTmpY = new Ipp16s[nCount];
	for (int i = 0; i < m_Para.m_nParts; i++)
	{
		GetRectangleArea(pLocationX, pLocationY, nCount, ftheta*i, fArea, rect, pTmpX, pTmpY);
		if (fArea < fMinArea)
		{
			fMinArea = fArea;
			fBestAngle = ftheta*i;
			BestRect = rect;
		}

	}

	//thirdly, get the position of min close rectangle
	m_Res.m_fAngle = float(fBestAngle*180/PI);
	m_Res.m_fMinArea = fMinArea;
	kxPoint<int> pt;
	pt.setup(BestRect.left, BestRect.top);
	TransFormToOrginalPosition(pt, -fBestAngle, m_Res.m_Pt[0]);

	pt.setup(BestRect.right, BestRect.top);
	TransFormToOrginalPosition(pt, -fBestAngle, m_Res.m_Pt[1]);

	pt.setup(BestRect.right, BestRect.bottom);
	TransFormToOrginalPosition(pt, -fBestAngle, m_Res.m_Pt[2]);

	pt.setup(BestRect.left, BestRect.bottom);
	TransFormToOrginalPosition(pt, -fBestAngle, m_Res.m_Pt[3]);

	m_Res.m_nMinRectWidth = BestRect.Width();
	m_Res.m_nMinRectHeight = BestRect.Height();
	m_Res.m_fRatio = float(gMax(m_Res.m_nMinRectWidth, m_Res.m_nMinRectHeight)/gMax(1, gMin(m_Res.m_nMinRectWidth, m_Res.m_nMinRectHeight)));



	delete []pLocationX;
	delete []pLocationY;
	delete []pTmpX;
	delete []pTmpY;

	return 1;


}
