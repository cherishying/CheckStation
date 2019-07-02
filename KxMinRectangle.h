#ifndef _KXMINRECTANGLEHHH
#define _KXMINRECTANGLEHHH
//add by lyl 2016/2/20
//this algorithm is used to search the image region's minimum enclosing rectangle
#include "ippcv.h"
#include "ippi.h"
#include "ipps.h"
#include "kxDef.h"
#include "gxMath.h"
#include "KxBaseFunction.h"

class CKxMinRect
{
public:
	CKxMinRect();
	~CKxMinRect();

	//90度分30分，精度为3度
	struct Parameter
	{
		Parameter()
		{
			m_nParts = 30;
		}
		int  m_nParts;

	};

	struct Result	
	{
		Result()
		{
			memset(m_Pt, 0, sizeof(kxPoint<int>)*4);
			m_fAngle = 0.0;
			m_fMinArea = 0.0;
			m_nMinRectWidth = 0;
			m_nMinRectHeight = 0;
			m_fRatio = 0.0;
			m_nCircumference = 0;
		}
		void Clear()
		{
			memset(m_Pt, 0, sizeof(kxPoint<int>)*4);
			m_fAngle = 0.0;
			m_fMinArea = 0.0;
			m_nMinRectWidth = 0;
			m_nMinRectHeight = 0;
			m_fRatio = 0.0;
			m_nCircumference = 0;
		}

		kxPoint<int>   m_Pt[4];
		float          m_fAngle;
		float          m_fMinArea;
		int            m_nMinRectWidth;
		int            m_nMinRectHeight;
	    float          m_fRatio;  //aspect ratio
		int            m_nCircumference; //circumference
		
	};



public:
    int TransFormToOrginalPosition(kxPoint<int> pt1, float fAngle, kxPoint<int>& pt2);
	int GetCenterPosition(Ipp16s* pLocationX, Ipp16s* pLocationY, int nCount, int& nCenterX, int& nCenterY);
	int GetRectangleArea(Ipp16s* pLocationX, Ipp16s* pLocationY, int nCount, float fangle, float& fArea,  kxRect<int>& rect, Ipp16s* pTmpX, Ipp16s* pTmpY);
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch ,unsigned char* pTmp = NULL);
	int Check(const kxCImageBuf& SrcImg, unsigned char* pTmp = NULL);
	Result& GetResult()
	{
		return m_Res;
	}
	void setParts(int n)
	{
		m_Para.m_nParts = n;
	}

private:
	kxCImageBuf   m_TmpImg;
	kxCImageBuf   m_CopyImg;
	Parameter     m_Para;
	Result        m_Res;
	int           m_nCenterX;
	int           m_nCenterY;
	CKxBaseFunction m_hBaseFun;


};

#endif