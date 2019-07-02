
#include "KxLearn.h"
#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
using namespace tbb;

CKxLearn::CKxLearn()
{
	m_nLearnHighPage = 50;
	m_nLearnLowPage = 50;

	m_nHaveLearnHigh = 0;
	m_nHaveLearnLow = 0;

}

CKxLearn::~CKxLearn()
{

}

void CKxLearn::Clear()
{
	m_nHaveLearnHigh = 0;
	m_nHaveLearnLow = 0;
}



void CKxLearn::Init( int nWidth, int nHeight, int nChannel, int nMode)
{
	if (_Learn_Low == nMode)
	{
		m_LeftMinbuf.Init(nWidth, nHeight, nChannel);
		m_TempMinbuf.Init(nWidth, nHeight, nChannel);

		ippsSet_8u( 0xff, m_LeftMinbuf.buf, m_LeftMinbuf.nHeight*m_LeftMinbuf.nPitch );
		ippsSet_8u( 0xff, m_TempMinbuf.buf, m_TempMinbuf.nHeight*m_TempMinbuf.nPitch );
	}
	else
	{
		m_LeftMaxbuf.Init(nWidth, nHeight, nChannel);
		m_TempMaxbuf.Init(nWidth, nHeight, nChannel);
		ippsSet_8u( 0, m_LeftMaxbuf.buf, m_LeftMaxbuf.nHeight*m_LeftMaxbuf.nPitch );
		ippsSet_8u( 0, m_TempMaxbuf.buf, m_TempMaxbuf.nHeight*m_TempMaxbuf.nPitch );
	}

	for (int i = 0; i <_Learn_Max_Layer; i++)
	{	
		if (_Learn_Low == nMode)
		{
			m_PyramidMinBuf[i].Init(nWidth, nHeight, nChannel);
			ippsSet_8u( 0xff, m_PyramidMinBuf[i].buf, m_PyramidMinBuf[i].nHeight*m_PyramidMinBuf[i].nPitch );
		}
		else
		{
			m_PyramidMaxBuf[i].Init(nWidth, nHeight, nChannel);
			ippsSet_8u( 0, m_PyramidMaxBuf[i].buf, m_PyramidMaxBuf[i].nHeight*m_PyramidMaxBuf[i].nPitch );
		}
	}



}

int CKxLearn::LearnHigh(kxCImageBuf& LearnImg, int nLearnPage)
{
	KxCallStatus hCall;
	return LearnHigh(LearnImg.buf, LearnImg.nWidth, LearnImg.nHeight, LearnImg.nPitch, LearnImg.nChannel, nLearnPage, hCall);
}



int CKxLearn::LearnHigh( kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall)
{
	return LearnHigh(LearnImg.buf, LearnImg.nWidth, LearnImg.nHeight, LearnImg.nPitch, LearnImg.nChannel, nLearnPage, hCall);
}



int CKxLearn::LearnHigh(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	IppStatus status1, status2, status3, status4;

	if (NULL == buf && 0 == nWidth && 0 == nHeight)
	{
		return 0;
	}

	m_nLearnHighPage = nLearnPage;
	if (0 == m_nHaveLearnHigh)
	{
		Init(nWidth, nHeight, nChannel,  _Learn_High);
	}

    m_nHaveLearnHigh++;

	if (m_nHaveLearnHigh > m_nLearnHighPage)
	{
		//m_nHaveLearnHigh = 0;
		return 1;
	}


	IppiSize siz ={nWidth, nHeight};
	if (_Type_G8 == nChannel)
	{
		status = ippiCopy_8u_C1R(buf, nPitch, m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, siz);
	}
	else
	{
		status = ippiCopy_8u_C3R(buf, nPitch, m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, siz);
	}

	if (check_sts(status, "LearnHigh_ippiCopy_first", hCall))
	{
		return 0;
	}
	

	for (int i = 0;i < _Learn_Max_Layer;i++)
	{
		if (_Type_G8 == nChannel)
		{
			status1 = ippiCopy_8u_C1R(m_PyramidMaxBuf[i].buf, m_PyramidMaxBuf[i].nPitch, m_TempMaxbuf.buf, m_TempMaxbuf.nPitch, siz);
			status2 = ippiMaxEvery_8u_C1IR(m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, m_PyramidMaxBuf[i].buf, m_PyramidMaxBuf[i].nPitch, siz);
			status3 = ippiMinEvery_8u_C1IR(m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, m_TempMaxbuf.buf, m_TempMaxbuf.nPitch, siz);
			status4 = ippiCopy_8u_C1R(m_TempMaxbuf.buf,m_TempMaxbuf.nPitch, m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, siz);
		}
		else
		{
			status1 = ippiCopy_8u_C3R(m_PyramidMaxBuf[i].buf, m_PyramidMaxBuf[i].nPitch, m_TempMaxbuf.buf, m_TempMaxbuf.nPitch, siz);
			status2 = ippiMaxEvery_8u_C3IR(m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, m_PyramidMaxBuf[i].buf, m_PyramidMaxBuf[i].nPitch, siz);
			status3 = ippiMinEvery_8u_C3IR(m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, m_TempMaxbuf.buf, m_TempMaxbuf.nPitch, siz);
			status4 = ippiCopy_8u_C3R(m_TempMaxbuf.buf,m_TempMaxbuf.nPitch, m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, siz);
		}
	}

	if (check_sts(int(status1 + status2 + status3 + status4), "LearnHigh_for_Learn_Max_Layer", hCall))
	{
		return 0;
	}
	
	

	return 0;

}

int CKxLearn::LearnLow(kxCImageBuf& LearnImg, int nLearnPage)
{
	KxCallStatus hCall;
	return LearnLow(LearnImg.buf, LearnImg.nWidth, LearnImg.nHeight, LearnImg.nPitch, LearnImg.nChannel, nLearnPage, hCall);
}



int CKxLearn::LearnLow( kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall)
{
	return LearnLow(LearnImg.buf, LearnImg.nWidth, LearnImg.nHeight, LearnImg.nPitch, LearnImg.nChannel, nLearnPage, hCall);
}

int CKxLearn::LearnLow(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage, KxCallStatus& hCall)
{
	hCall.Clear();
	IppStatus status;
	IppStatus status1, status2, status3, status4;

	if (NULL == buf && 0 == nWidth && 0 == nHeight)
	{
		return 0;
	}

	m_nLearnLowPage = nLearnPage;
	if (0 == m_nHaveLearnLow)
	{		
		Init(nWidth, nHeight, nChannel, _Learn_Low);
	}

	m_nHaveLearnLow++;
	if (m_nHaveLearnLow > m_nLearnLowPage)
	{
		//m_nHaveLearnLow = 0;
		return 1;
	}


	IppiSize siz ={nWidth, nHeight};
	if (_Type_G8 == nChannel)
	{
		status = ippiCopy_8u_C1R(buf, nPitch, m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, siz);
	}
	else
	{
		status = ippiCopy_8u_C3R(buf, nPitch, m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, siz);
	}

	if (check_sts(status, "LearnLow_ippiCopy_first", hCall))
	{
		return 0;
	}


	for (int i = 0;i < _Learn_Max_Layer;i++)
	{
		if (_Type_G8 == nChannel)
		{
			status1 = ippiCopy_8u_C1R(m_PyramidMinBuf[i].buf, m_PyramidMinBuf[i].nPitch, m_TempMinbuf.buf, m_TempMinbuf.nPitch, siz);
			status2 = ippiMinEvery_8u_C1IR(m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, m_PyramidMinBuf[i].buf, m_PyramidMinBuf[i].nPitch, siz);
			status3 = ippiMaxEvery_8u_C1IR(m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, m_TempMinbuf.buf, m_TempMinbuf.nPitch, siz);
			status4 = ippiCopy_8u_C1R(m_TempMinbuf.buf,m_TempMinbuf.nPitch, m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, siz);

		}
		else
		{
			status1 = ippiCopy_8u_C3R(m_PyramidMinBuf[i].buf, m_PyramidMinBuf[i].nPitch, m_TempMinbuf.buf, m_TempMinbuf.nPitch, siz);
			status2 = ippiMinEvery_8u_C3IR(m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, m_PyramidMinBuf[i].buf, m_PyramidMinBuf[i].nPitch, siz);
			status3 = ippiMaxEvery_8u_C3IR(m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, m_TempMinbuf.buf, m_TempMinbuf.nPitch, siz);
			status4 = ippiCopy_8u_C3R(m_TempMinbuf.buf,m_TempMinbuf.nPitch, m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, siz);
		}
	}

	if (check_sts(int(status1 + status2 + status3 + status4), "LearnLow_for_Learn_Max_Layer", hCall))
	{
		return 0;
	}

	return 0;

}


int CKxLearn::GetHighTemplateBuf( int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess )
{
	assert(nIndex < _Learn_Max_Layer);
	int nModelProcessW = 3, nModelProcessH = 3;
	switch(nModelProcess)
	{
	case _3X3:
		{
			nModelProcessW = 3;
			nModelProcessH = 3;
			break;
		}
	case _5X5:
		{
			nModelProcessW = 5;
			nModelProcessH = 5;
			break;
		}
	case _7X7:
		{
			nModelProcessW = 7;
			nModelProcessH = 7;
			break;
		}
	default:
		break;
	}

	m_TmpBuf.Init(m_PyramidMaxBuf[nIndex].nWidth, m_PyramidMaxBuf[nIndex].nHeight, m_PyramidMaxBuf[nIndex].nChannel);

	if ((nIndex >= m_nHaveLearnHigh) && (m_nHaveLearnHigh <= m_nLearnHighPage))
	{
		ippsSet_8u( 0xff, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxDilateImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH);	
	}

	if (m_nHaveLearnHigh > m_nLearnHighPage)
	{
		m_nHaveLearnHigh = 0;
	}

	TemplateBuf.SetImageBuf(m_TmpBuf.buf, m_TmpBuf.nWidth, m_TmpBuf.nHeight, m_TmpBuf.nPitch, m_TmpBuf.nChannel, true);

	return 1;
}


unsigned char* CKxLearn::GetHighTemplateBuf( int nIndex, int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess )
{
	assert(nIndex < _Learn_Max_Layer);

	m_TmpBuf.Init(m_PyramidMaxBuf[nIndex].nWidth, m_PyramidMaxBuf[nIndex].nHeight, m_PyramidMaxBuf[nIndex].nChannel);

	int nModelProcessW = 3, nModelProcessH = 3;
	switch(nModelProcess)
	{
	case _3X3:
		{
			nModelProcessW = 3;
			nModelProcessH = 3;
			break;
		}
	case _5X5:
		{
			nModelProcessW = 5;
			nModelProcessH = 5;
			break;
		}
	case _7X7:
		{
			nModelProcessW = 7;
			nModelProcessH = 7;
			break;
		}
	default:
		break;
	}
	
	if ((nIndex >= m_nHaveLearnHigh) && (m_nHaveLearnHigh <= m_nLearnHighPage))
	{
		ippsSet_8u( 0xff, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxDilateImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH);	
	}

	if (m_nHaveLearnHigh > m_nLearnHighPage)
	{
		m_nHaveLearnHigh = 0;
	}

	return m_TmpBuf.GetImageBuf(nWidth, nHeight, nPitch, nChannel);

}


unsigned char* CKxLearn::GetLowTemplateBuf( int nIndex, int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess )
{
	assert(nIndex < _Learn_Max_Layer);

	int nModelProcessW = 3, nModelProcessH = 3;
	switch(nModelProcess)
	{
	case _3X3:
		{
			nModelProcessW = 3;
			nModelProcessH = 3;
			break;
		}
	case _5X5:
		{
			nModelProcessW = 5;
			nModelProcessH = 5;
			break;
		}
	case _7X7:
		{
			nModelProcessW = 7;
			nModelProcessH = 7;
			break;
		}
	default:
		break;
	}


	m_TmpBuf.Init(m_PyramidMinBuf[nIndex].nWidth, m_PyramidMinBuf[nIndex].nHeight, m_PyramidMinBuf[nIndex].nChannel);
	if ((nIndex >= m_nHaveLearnLow) && (m_nHaveLearnLow < m_nLearnLowPage))
	{
		ippsSet_8u( 0, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxErodeImage(m_PyramidMinBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH);
	}

	if (m_nHaveLearnLow > m_nLearnLowPage)
	{
		m_nHaveLearnLow = 0;
	}



	return m_TmpBuf.GetImageBuf(nWidth, nHeight, nPitch, nChannel);

}

int CKxLearn::GetLowTemplateBuf( int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess )
{
	assert(nIndex < _Learn_Max_Layer);

	int nModelProcessW = 3, nModelProcessH = 3;
	switch(nModelProcess)
	{
	case _3X3:
		{
			nModelProcessW = 3;
			nModelProcessH = 3;
			break;
		}
	case _5X5:
		{
			nModelProcessW = 5;
			nModelProcessH = 5;
			break;
		}
	case _7X7:
		{
			nModelProcessW = 7;
			nModelProcessH = 7;
			break;
		}
	default:
		break;
	}


	m_TmpBuf.Init(m_PyramidMinBuf[nIndex].nWidth, m_PyramidMinBuf[nIndex].nHeight, m_PyramidMinBuf[nIndex].nChannel);
	if ((nIndex >= m_nHaveLearnLow) && (m_nHaveLearnLow < m_nLearnLowPage))
	{
		ippsSet_8u( 0, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxErodeImage(m_PyramidMinBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH);
	}

	if (m_nHaveLearnLow > m_nLearnLowPage)
	{
		m_nHaveLearnLow = 0;
	}

	TemplateBuf.SetImageBuf(m_TmpBuf.buf, m_TmpBuf.nWidth, m_TmpBuf.nHeight, m_TmpBuf.nPitch, m_TmpBuf.nChannel, true);

	return 1;

}

int CKxLearn::LearnImage(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage)
{
	KxCallStatus hCall;
	if (nHighLowMode == _Learn_High)
	{
		return LearnHigh(LearnImg, nLearnPage, hCall);
	}
	else
	{
		return LearnLow(LearnImg, nLearnPage, hCall);
	}
}


int CKxLearn::LearnImage(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage, KxCallStatus& hCall)
{
	if (nHighLowMode == _Learn_High)
	{
		return LearnHigh(LearnImg, nLearnPage, hCall);
	}
	else
	{
		return LearnLow(LearnImg, nLearnPage, hCall);
	}
}

int CKxLearn::GetTemplateBuf(int nIndex, int nLearnMode, kxCImageBuf& TemplateBuf, int nModelProcess)
{
	assert(nIndex < _Learn_Max_Layer);
	int nModelProcessW = 1, nModelProcessH = 1;
	switch (nModelProcess)
	{
	case _3X3:
	{
				 nModelProcessW = 3;
				 nModelProcessH = 3;
				 break;
	}
	case _5X5:
	{
				 nModelProcessW = 5;
				 nModelProcessH = 5;
				 break;
	}
	case _7X7:
	{
				 nModelProcessW = 7;
				 nModelProcessH = 7;
				 break;
	}
	default:
		break;
	}



	if (_Learn_High == nLearnMode)
	{
		m_TmpBuf.Init(m_PyramidMaxBuf[nIndex].nWidth, m_PyramidMaxBuf[nIndex].nHeight, m_PyramidMaxBuf[nIndex].nChannel);
		if ((nIndex >= m_nHaveLearnHigh) && (m_nHaveLearnHigh <= m_nLearnHighPage))
		{
			ippsSet_8u(0xff, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
		}
		else
		{
			if (nModelProcessW == 1 && nModelProcessH == 1)
			{
				kxRect<int> rc;
				rc.setup(0, 0, m_TmpBuf.nWidth - 1, m_TmpBuf.nHeight - 1);
				m_hBaseFun.KxCopyImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, rc);
			}
			else
			{
				m_hBaseFun.KxDilateImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH, NULL);
			}

		}

		if (m_nHaveLearnHigh > m_nLearnHighPage)
		{
			m_nHaveLearnHigh = 0;
		}
	}
	else
	{
		m_TmpBuf.Init(m_PyramidMinBuf[nIndex].nWidth, m_PyramidMinBuf[nIndex].nHeight, m_PyramidMinBuf[nIndex].nChannel);
		if ((nIndex >= m_nHaveLearnLow) && (m_nHaveLearnLow < m_nLearnLowPage))
		{
			ippsSet_8u(0, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
		}
		else
		{
			if (nModelProcessW == 1 && nModelProcessH == 1)
			{
				kxRect<int> rc;
				rc.setup(0, 0, m_TmpBuf.nWidth - 1, m_TmpBuf.nHeight - 1);
				m_hBaseFun.KxCopyImage(m_PyramidMinBuf[nIndex], m_TmpBuf, rc);
			}
			else
			{
				m_hBaseFun.KxErodeImage(m_PyramidMinBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH);
			}

		}

		if (m_nHaveLearnLow > m_nLearnLowPage)
		{
			m_nHaveLearnLow = 0;
		}
	}

	TemplateBuf.SetImageBuf(m_TmpBuf.buf, m_TmpBuf.nWidth, m_TmpBuf.nHeight, m_TmpBuf.nPitch, m_TmpBuf.nChannel, true);

	return 1;
}



int CKxLearn::GetTemplateBufParallel(int nIndex, int nLearnMode, kxCImageBuf& TemplateBuf, int nModelProcess)
{
	assert(nIndex < _Learn_Max_Layer);
	int nModelProcessW = 1, nModelProcessH = 1;
	switch (nModelProcess)
	{
	case _3X3:
	{
				 nModelProcessW = 3;
				 nModelProcessH = 3;
				 break;
	}
	case _5X5:
	{
				 nModelProcessW = 5;
				 nModelProcessH = 5;
				 break;
	}
	case _7X7:
	{
				 nModelProcessW = 7;
				 nModelProcessH = 7;
				 break;
	}
	default:
		break;
	}

	

	if (_Learn_High == nLearnMode)
	{
		m_TmpBuf.Init(m_PyramidMaxBuf[nIndex].nWidth, m_PyramidMaxBuf[nIndex].nHeight, m_PyramidMaxBuf[nIndex].nChannel);
		if ((nIndex >= m_nHaveLearnHigh) && (m_nHaveLearnHigh <= m_nLearnHighPage))
		{
			ippsSet_8u(0xff, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
		}
		else
		{
			if (nModelProcessW == 1 && nModelProcessH == 1)
			{
				kxRect<int> rc;
				rc.setup(0, 0, m_TmpBuf.nWidth - 1, m_TmpBuf.nHeight - 1);
				m_hBaseFun.KxCopyImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, rc);
			}
			else
			{
				m_hBaseFun.KxDilateImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH, NULL);
			}
			
		}

		if (m_nHaveLearnHigh > m_nLearnHighPage)
		{
			m_nHaveLearnHigh = 0;
		}
	}
	else
	{
		m_TmpBuf.Init(m_PyramidMinBuf[nIndex].nWidth, m_PyramidMinBuf[nIndex].nHeight, m_PyramidMinBuf[nIndex].nChannel);
		if ((nIndex >= m_nHaveLearnLow) && (m_nHaveLearnLow < m_nLearnLowPage))
		{
			ippsSet_8u(0, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
		}
		else
		{
			if (nModelProcessW == 1 && nModelProcessH == 1)
			{
				kxRect<int> rc;
				rc.setup(0, 0, m_TmpBuf.nWidth - 1, m_TmpBuf.nHeight - 1);
				m_hBaseFun.KxCopyImage(m_PyramidMinBuf[nIndex], m_TmpBuf, rc);
			}
			else
			{
				m_hBaseFun.KxErodeImage(m_PyramidMinBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH);
			}
			
		}

		if (m_nHaveLearnLow > m_nLearnLowPage)
		{
			m_nHaveLearnLow = 0;
		}
	}

	TemplateBuf.SetImageBuf(m_TmpBuf.buf, m_TmpBuf.nWidth, m_TmpBuf.nHeight, m_TmpBuf.nPitch, m_TmpBuf.nChannel, true);

	return 1;
}


int CKxLearn::GetHighTemplateBufParallel(int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess)
{
	assert(nIndex < _Learn_Max_Layer);
	int nModelProcessW = 3, nModelProcessH = 3;
	switch (nModelProcess)
	{
	case _3X3:
	{
				 nModelProcessW = 3;
				 nModelProcessH = 3;
				 break;
	}
	case _5X5:
	{
				 nModelProcessW = 5;
				 nModelProcessH = 5;
				 break;
	}
	case _7X7:
	{
				 nModelProcessW = 7;
				 nModelProcessH = 7;
				 break;
	}
	default:
		break;
	}

	m_TmpBuf.Init(m_PyramidMaxBuf[nIndex].nWidth, m_PyramidMaxBuf[nIndex].nHeight, m_PyramidMaxBuf[nIndex].nChannel);

	if ((nIndex >= m_nHaveLearnHigh) && (m_nHaveLearnHigh <= m_nLearnHighPage))
	{
		ippsSet_8u(0xff, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxDilateImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH, NULL);
	}

	if (m_nHaveLearnHigh > m_nLearnHighPage)
	{
		m_nHaveLearnHigh = 0;
	}

	TemplateBuf.SetImageBuf(m_TmpBuf.buf, m_TmpBuf.nWidth, m_TmpBuf.nHeight, m_TmpBuf.nPitch, m_TmpBuf.nChannel, true);

	return 1;
}


unsigned char* CKxLearn::GetHighTemplateBufParallel(int nIndex, int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess)
{
	assert(nIndex < _Learn_Max_Layer);

	m_TmpBuf.Init(m_PyramidMaxBuf[nIndex].nWidth, m_PyramidMaxBuf[nIndex].nHeight, m_PyramidMaxBuf[nIndex].nChannel);

	int nModelProcessW = 3, nModelProcessH = 3;
	switch (nModelProcess)
	{
	case _3X3:
	{
				 nModelProcessW = 3;
				 nModelProcessH = 3;
				 break;
	}
	case _5X5:
	{
				 nModelProcessW = 5;
				 nModelProcessH = 5;
				 break;
	}
	case _7X7:
	{
				 nModelProcessW = 7;
				 nModelProcessH = 7;
				 break;
	}
	default:
		break;
	}

	if ((nIndex >= m_nHaveLearnHigh) && (m_nHaveLearnHigh <= m_nLearnHighPage))
	{
		ippsSet_8u(0xff, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxDilateImage(m_PyramidMaxBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH, NULL);
	}

	if (m_nHaveLearnHigh > m_nLearnHighPage)
	{
		m_nHaveLearnHigh = 0;
	}

	return m_TmpBuf.GetImageBuf(nWidth, nHeight, nPitch, nChannel);

}


unsigned char* CKxLearn::GetLowTemplateBufParallel(int nIndex, int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess)
{
	assert(nIndex < _Learn_Max_Layer);

	int nModelProcessW = 3, nModelProcessH = 3;
	switch (nModelProcess)
	{
	case _3X3:
	{
				 nModelProcessW = 3;
				 nModelProcessH = 3;
				 break;
	}
	case _5X5:
	{
				 nModelProcessW = 5;
				 nModelProcessH = 5;
				 break;
	}
	case _7X7:
	{
				 nModelProcessW = 7;
				 nModelProcessH = 7;
				 break;
	}
	default:
		break;
	}


	m_TmpBuf.Init(m_PyramidMinBuf[nIndex].nWidth, m_PyramidMinBuf[nIndex].nHeight, m_PyramidMinBuf[nIndex].nChannel);
	if ((nIndex >= m_nHaveLearnLow) && (m_nHaveLearnLow < m_nLearnLowPage))
	{
		ippsSet_8u(0, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxErodeImage(m_PyramidMinBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH, NULL);
	}

	if (m_nHaveLearnLow > m_nLearnLowPage)
	{
		m_nHaveLearnLow = 0;
	}



	return m_TmpBuf.GetImageBuf(nWidth, nHeight, nPitch, nChannel);

}

int CKxLearn::GetLowTemplateBufParallel(int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess)
{
	assert(nIndex < _Learn_Max_Layer);

	int nModelProcessW = 3, nModelProcessH = 3;
	switch (nModelProcess)
	{
	case _3X3:
	{
				 nModelProcessW = 3;
				 nModelProcessH = 3;
				 break;
	}
	case _5X5:
	{
				 nModelProcessW = 5;
				 nModelProcessH = 5;
				 break;
	}
	case _7X7:
	{
				 nModelProcessW = 7;
				 nModelProcessH = 7;
				 break;
	}
	default:
		break;
	}


	m_TmpBuf.Init(m_PyramidMinBuf[nIndex].nWidth, m_PyramidMinBuf[nIndex].nHeight, m_PyramidMinBuf[nIndex].nChannel);
	if ((nIndex >= m_nHaveLearnLow) && (m_nHaveLearnLow < m_nLearnLowPage))
	{
		ippsSet_8u(0, m_TmpBuf.buf, m_TmpBuf.nPitch*m_TmpBuf.nHeight);
	}
	else
	{
		m_hBaseFun.KxErodeImage(m_PyramidMinBuf[nIndex], m_TmpBuf, nModelProcessW, nModelProcessH, NULL);
	}

	if (m_nHaveLearnLow > m_nLearnLowPage)
	{
		m_nHaveLearnLow = 0;
	}

	TemplateBuf.SetImageBuf(m_TmpBuf.buf, m_TmpBuf.nWidth, m_TmpBuf.nHeight, m_TmpBuf.nPitch, m_TmpBuf.nChannel, true);

	return 1;

}



void CKxLearn::InitParallel(int nWidth, int nHeight, int nChannel, int nMode)
{
	if (_Learn_Low == nMode)
	{
		m_LeftMinbuf.Init(nWidth, nHeight, nChannel);
		m_TempMinbuf.Init(nWidth, nHeight, nChannel);

		ippsSet_8u(0xff, m_LeftMinbuf.buf, m_LeftMinbuf.nHeight*m_LeftMinbuf.nPitch);
		ippsSet_8u(0xff, m_TempMinbuf.buf, m_TempMinbuf.nHeight*m_TempMinbuf.nPitch);
	}
	else
	{
		m_LeftMaxbuf.Init(nWidth, nHeight, nChannel);
		m_TempMaxbuf.Init(nWidth, nHeight, nChannel);
		ippsSet_8u(0, m_LeftMaxbuf.buf, m_LeftMaxbuf.nHeight*m_LeftMaxbuf.nPitch);
		ippsSet_8u(0, m_TempMaxbuf.buf, m_TempMaxbuf.nHeight*m_TempMaxbuf.nPitch);
	}


	for (int i = 0; i < _Learn_Max_Layer; i++)
	{
		if (_Learn_Low == nMode)
		{
			m_PyramidMinBuf[i].Init(nWidth, nHeight, nChannel);
			ippsSet_8u(0xff, m_PyramidMinBuf[i].buf, m_PyramidMinBuf[i].nHeight*m_PyramidMinBuf[i].nPitch);
		}
		else
		{
			m_PyramidMaxBuf[i].Init(nWidth, nHeight, nChannel);
			ippsSet_8u(0, m_PyramidMaxBuf[i].buf, m_PyramidMaxBuf[i].nHeight*m_PyramidMaxBuf[i].nPitch);
		}
	}

}

int CKxLearn::LearnHighParallel(kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	return LearnHighParallel(LearnImg.buf, LearnImg.nWidth, LearnImg.nHeight, LearnImg.nPitch, LearnImg.nChannel, nLearnPage, hCall, nRowGrain, nColGrain);
}



int CKxLearn::LearnHighParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	IppStatus status;
	//IppStatus status1, status2, status3, status4;

	if (NULL == buf && 0 == nWidth && 0 == nHeight)
	{
		return 0;
	}

	m_nLearnHighPage = nLearnPage;
	if (0 == m_nHaveLearnHigh)
	{		
		InitParallel(nWidth, nHeight, nChannel, _Learn_High);
	}

	m_nHaveLearnHigh++;

	if (m_nHaveLearnHigh > m_nLearnHighPage)
	{
		//m_nHaveLearnHigh = 0;
		return 1;
	}


	IppiSize siz = { nWidth, nHeight };
	if (_Type_G8 == nChannel)
	{
		status = ippiCopy_8u_C1R(buf, nPitch, m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, siz);
	}
	else
	{
		status = ippiCopy_8u_C3R(buf, nPitch, m_LeftMaxbuf.buf, m_LeftMaxbuf.nPitch, siz);
	}

	if (check_sts(status, "LearnHigh_ippiCopy_first", hCall))
	{
		return 0;
	}

	int nRowActualGrain = gMin(nRowGrain, nHeight);
	int nColActualGrain = gMin(nColGrain, nHeight);
	parallel_for(blocked_range2d<int,int>(0, nHeight, 0, nWidth),
		[&](const blocked_range2d<int, int>& range)
	{
			for (int i = 0; i < _Learn_Max_Layer; i++)
			{
				IppiSize dstSize = {(int)range.cols().size(), (int)range.rows().size()};
				int nY = range.rows().begin();
				int nX = range.cols().begin();
				Ipp8u* pyramidMax = m_PyramidMaxBuf[i].buf + nY * m_PyramidMaxBuf[i].nPitch + nX * m_PyramidMaxBuf[i].nChannel;
				Ipp8u* pTempMaxbuf = m_TempMaxbuf.buf + nY * m_TempMaxbuf.nPitch + nX * m_TempMaxbuf.nChannel;
				Ipp8u* pLeftMaxbuf = m_LeftMaxbuf.buf + nY * m_LeftMaxbuf.nPitch + nX * m_LeftMaxbuf.nChannel;

				if (_Type_G8 == m_PyramidMaxBuf[i].nChannel)
				{
					ippiCopy_8u_C1R(pyramidMax, m_PyramidMaxBuf[i].nPitch, pTempMaxbuf, m_TempMaxbuf.nPitch, dstSize);
					ippiMaxEvery_8u_C1IR(pLeftMaxbuf, m_LeftMaxbuf.nPitch, pyramidMax, m_PyramidMaxBuf[i].nPitch, dstSize);
					ippiMinEvery_8u_C1IR(pLeftMaxbuf, m_LeftMaxbuf.nPitch, pTempMaxbuf, m_TempMaxbuf.nPitch, dstSize);
					ippiCopy_8u_C1R(pTempMaxbuf, m_TempMaxbuf.nPitch, pLeftMaxbuf, m_LeftMaxbuf.nPitch, dstSize);
				}
				else
				{
					ippiCopy_8u_C3R(pyramidMax, m_PyramidMaxBuf[i].nPitch, pTempMaxbuf, m_TempMaxbuf.nPitch, dstSize);
					ippiMaxEvery_8u_C3IR(pLeftMaxbuf, m_LeftMaxbuf.nPitch, pyramidMax, m_PyramidMaxBuf[i].nPitch, dstSize);
					ippiMinEvery_8u_C3IR(pLeftMaxbuf, m_LeftMaxbuf.nPitch, pTempMaxbuf, m_TempMaxbuf.nPitch, dstSize);
					ippiCopy_8u_C3R(pTempMaxbuf, m_TempMaxbuf.nPitch, pLeftMaxbuf, m_LeftMaxbuf.nPitch, dstSize);
				}
			}
	}, auto_partitioner());

	return 0;

}

int CKxLearn::LearnImageParallel(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	if (nHighLowMode == _Learn_High)
	{
		return LearnHighParallel(LearnImg, nLearnPage, hCall, nRowGrain, nColGrain);
	}
	else
	{
		return LearnLowParallel(LearnImg, nLearnPage, hCall, nRowGrain, nColGrain);
	}
}

int CKxLearn::LearnImageParallel(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage)
{
	KxCallStatus hCall;
	if (nHighLowMode == _Learn_High)
	{
		return LearnHighParallel(LearnImg, nLearnPage, hCall);
	}
	else
	{
		return LearnLowParallel(LearnImg, nLearnPage, hCall);
	}
}




int CKxLearn::LearnLowParallel(kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	return LearnLowParallel(LearnImg.buf, LearnImg.nWidth, LearnImg.nHeight, LearnImg.nPitch, LearnImg.nChannel, nLearnPage, hCall, nRowGrain, nColGrain);
}

int CKxLearn::LearnLowParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage, KxCallStatus& hCall, int nRowGrain, int nColGrain)
{
	hCall.Clear();
	IppStatus status;
	//IppStatus status1, status2, status3, status4;

	if (NULL == buf && 0 == nWidth && 0 == nHeight)
	{
		return 0;
	}

	m_nLearnLowPage = nLearnPage;
	if (0 == m_nHaveLearnLow)
	{		
		InitParallel(nWidth, nHeight, nChannel, _Learn_Low);
	}

	m_nHaveLearnLow++;
	if (m_nHaveLearnLow > m_nLearnLowPage)
	{
		//m_nHaveLearnLow = 0;
		return 1;
	}


	IppiSize siz = { nWidth, nHeight };
	if (_Type_G8 == nChannel)
	{
		status = ippiCopy_8u_C1R(buf, nPitch, m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, siz);
	}
	else
	{
		status = ippiCopy_8u_C3R(buf, nPitch, m_LeftMinbuf.buf, m_LeftMinbuf.nPitch, siz);
	}

	if (check_sts(status, "LearnLow_ippiCopy_first", hCall))
	{
		return 0;
	}


	int nRowActualGrain = gMin(nRowGrain, nHeight);
	int nColActualGrain = gMin(nColGrain, nHeight);
	parallel_for(blocked_range2d<int, int>(0, nHeight, 0, nWidth),
		[&](const blocked_range2d<int, int>& range)
	{
		for (int i = 0; i < _Learn_Max_Layer; i++)
		{ 
			IppiSize dstSize = { (int)range.cols().size(), (int)range.rows().size() };
			Ipp8u* pyramidMin = m_PyramidMinBuf[i].buf + range.rows().begin() * m_PyramidMinBuf[i].nPitch + range.cols().begin() * m_PyramidMinBuf[i].nChannel;
			Ipp8u* pTempMinbuf = m_TempMinbuf.buf + range.rows().begin() * m_TempMinbuf.nPitch + range.cols().begin() * m_TempMinbuf.nChannel;
			Ipp8u* pLeftMinbuf = m_LeftMinbuf.buf + range.rows().begin() * m_LeftMinbuf.nPitch + range.cols().begin() * m_LeftMinbuf.nChannel;

			if (_Type_G8 == m_PyramidMaxBuf[i].nChannel)
			{
				ippiCopy_8u_C1R(pyramidMin, m_PyramidMinBuf[i].nPitch, pTempMinbuf, m_TempMinbuf.nPitch, dstSize);
				ippiMinEvery_8u_C1IR(pLeftMinbuf, m_LeftMinbuf.nPitch, pyramidMin, m_PyramidMinBuf[i].nPitch, dstSize);
				ippiMaxEvery_8u_C1IR(pLeftMinbuf, m_LeftMinbuf.nPitch, pTempMinbuf, m_TempMinbuf.nPitch, dstSize);
				ippiCopy_8u_C1R(pTempMinbuf, m_TempMinbuf.nPitch, pLeftMinbuf, m_LeftMinbuf.nPitch, dstSize);
			}
			else
			{
				ippiCopy_8u_C3R(pyramidMin, m_PyramidMinBuf[i].nPitch, pTempMinbuf, m_TempMinbuf.nPitch, dstSize);
				ippiMinEvery_8u_C3IR(pLeftMinbuf, m_LeftMinbuf.nPitch, pyramidMin, m_PyramidMinBuf[i].nPitch, dstSize);
				ippiMaxEvery_8u_C3IR(pLeftMinbuf, m_LeftMinbuf.nPitch, pTempMinbuf, m_TempMinbuf.nPitch, dstSize);
				ippiCopy_8u_C3R(pTempMinbuf, m_TempMinbuf.nPitch, pLeftMinbuf, m_LeftMinbuf.nPitch, dstSize);
			}
		}
	}, auto_partitioner());

	return 0;

}