#include "gxMath.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int c_Token1 = 456543278;
const int c_Token2 = 123442189;
const int c_Token3 = 976234523;

int IsImgErr( const unsigned char* buf)
{
	int*  pInt = ( int*)(const_cast<unsigned char*>(buf));
	if( *pInt != c_Token1 )
		return false;
	pInt++;
	if( *pInt != c_Token2 )
		return false;
	pInt++;
	if( *pInt != c_Token3 )
		return false;
	pInt++;
	return *pInt;
}
void SetImgErr( unsigned char* buf, int nMark)
{
	int*  pInt = ( int*)buf;
	*pInt = c_Token1;
	pInt++;
	*pInt = c_Token2;
	pInt++;
	*pInt = c_Token3;
	pInt++;
	*pInt = nMark;
}

//float kxImageAlign( kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts )
//{// ��ͼ�� srcBuf�� sw X sh ��Χ  ���� ͼ��tplBuf[ tw X th ]�� ����ֵ��������ƶ�[0--1]�� tplBuf�� srcBuf�е����λ��
//	assert( sw >= tw && sh >= th );
//	IppiSize srcRoiSize;
//	srcRoiSize.width = sw;
//	srcRoiSize.height = sh;
//
//	IppiSize tplRoiSize;
//	tplRoiSize.width = tw;
//	tplRoiSize.height = th;
//
//	int cw = sw - tw + 1;
//	int ch = sh - th + 1;
//
//	int corrSize = cw * ch;
//	int cs = cw * sizeof(float);
//
//	float * corrImage = (float *)malloc(cs * ch);
//	IppiSize corrRoiSize;
//	corrRoiSize.width = cw;
//	corrRoiSize.height = ch;
//
//	ippiCrossCorrValid_NormLevel_8u32f_C1R( srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, corrImage, cs );
//	float maxCorr = 0;
//	IppiPoint maxPos;
//	ippiMaxIndx_32f_C1R(corrImage , cw * sizeof(float) , corrRoiSize ,  &maxCorr , &maxPos.x , &maxPos.y);
//	pos.x = (float)maxPos.x;
//	pos.y = (float)maxPos.y;
//	free( corrImage );
//	return maxCorr;
//}
//float kxImageAlign( kxPoint<float>& pos, const Ipp16u* srcBuf, int sw, int sh, int ss, const Ipp16u* tplBuf, int tw, int th, int ts )
//{// ��ͼ�� srcBuf�� sw X sh ��Χ  ���� ͼ��tplBuf[ tw X th ]�� ����ֵ��������ƶ�[0--1]�� tplBuf�� srcBuf�е����λ��
//	assert( sw >= tw && sh >= th );
//	IppiSize srcRoiSize;
//	srcRoiSize.width = sw;
//	srcRoiSize.height = sh;
//
//	IppiSize tplRoiSize;
//	tplRoiSize.width = tw;
//	tplRoiSize.height = th;
//
//	int cw = sw - tw + 1;
//	int ch = sh - th + 1;
//
//	int corrSize = cw * ch;
//	int cs = cw * sizeof(float);
//
//	float * corrImage = (float *)malloc(cs * ch);
//	IppiSize corrRoiSize;
//	corrRoiSize.width = cw;
//	corrRoiSize.height = ch;
//
//	ippiCrossCorrValid_NormLevel_16u32f_C1R( srcBuf, ss, srcRoiSize, tplBuf, ts, tplRoiSize, corrImage, cs );
//	float maxCorr = 0;
//	IppiPoint maxPos;
//	ippiMaxIndx_32f_C1R(corrImage , cw * sizeof(float) , corrRoiSize ,  &maxCorr , &maxPos.x , &maxPos.y);
//	pos.x = (float)maxPos.x;
//	pos.y = (float)maxPos.y;
//	free( corrImage );
//	return maxCorr;
//}

void kxSetRegion( unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char val )
{//�� buf �� �������� => val
	IppiSize roiSize;
	roiSize.height = nHeight;
	roiSize.width = nWidth;
	ippiSet_8u_C1R( val, buf, nPitch, roiSize );
}

void kxCopyImageRegion( const unsigned char* pSrc, int nWidth, int nHeight, int SrcPitch, unsigned char* pDst, int DstPitch )
{//�� pSrc�� Copy nWidth X nHeight ===> pDst
	IppiSize roiSize;
	roiSize.width = nWidth;
	roiSize.height = nHeight;
	ippiCopy_8u_C1R( pSrc, SrcPitch, pDst, DstPitch, roiSize );
}

//void kxResizeImage( const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int SrcPitch, unsigned char* pDst, int nDstWidth, int nDstHeight, int DstPitch, int nType )
//{// ��ͼ�� pSrc[ nSrcWidth X nSrcHeight ] ����/ѹ�� ==��pDst[ nDstWidth X nDstHeight ]   nType = [IPPI_INTER_LINEAR/IPPI_INTER_NN/IPPI_INTER_CUBIC]
//	IppiSize srcSize = { nSrcWidth, nSrcHeight };
//	IppiRect srcRoi = { 0, 0, nSrcWidth, nSrcHeight };
//	IppiSize dstRoi = { nDstWidth, nDstHeight };
//	double   fx = (double)nDstWidth/nSrcWidth;
//	double   fy = (double)nDstHeight/nSrcHeight;
//	//ippiResize_8u_C1R( pSrc, srcSize, SrcPitch, srcRoi, pDst, DstPitch, dstRoi, fx, fy, nType );
//
//	IppiRect dstRect = {0, 0, nDstWidth, nDstHeight};
//	int nBuffer;
//	ippiResizeGetBufSize(srcRoi, dstRect, 1, nType, &nBuffer);
//	Ipp8u* pBuffer = new Ipp8u[nBuffer];
//	ippiResizeSqrPixel_8u_C1R(pSrc, srcSize, SrcPitch, srcRoi, pDst,
//		DstPitch, dstRect, fx, fy, 0, 0, nType, pBuffer);
//	delete []pBuffer;
//
//}
//void kxResizeImage_RGB( const unsigned char* pSrc[], int nSrcWidth, int nSrcHeight, int SrcPitch, unsigned char* pDst[], int nDstWidth, int nDstHeight, int DstPitch, int nType )
//{// ��ͼ�� pSrc[ nSrcWidth X nSrcHeight ] ����/ѹ�� ==��pDst[ nDstWidth X nDstHeight ]   nType = [IPPI_INTER_LINEAR/IPPI_INTER_NN/IPPI_INTER_CUBIC]
//	IppiSize srcSize = { nSrcWidth, nSrcHeight };
//	IppiRect srcRoi = { 0, 0, nSrcWidth, nSrcHeight };
//	IppiSize dstRoi = { nDstWidth, nDstHeight };
//	double   fx = (double)nDstWidth/nSrcWidth;
//	double   fy = (double)nDstHeight/nSrcHeight;
//	//ippiResize_8u_P3R( pSrc, srcSize, SrcPitch, srcRoi, pDst, DstPitch, dstRoi, fx, fy, nType );
//
//	IppiRect dstRect = {0, 0, nDstWidth, nDstHeight};
//	int nBuffer;
//	ippiResizeGetBufSize(srcRoi, dstRect, 3, nType, &nBuffer);
//	Ipp8u* pBuffer = new Ipp8u[nBuffer];
//	ippiResizeSqrPixel_8u_P3R(pSrc, srcSize, SrcPitch, srcRoi, pDst,
//		DstPitch, dstRect, fx, fy, 0, 0, nType, pBuffer);
//	delete []pBuffer;
//
//}

void kxHistogram( const unsigned char* buf, int nWidth, int nHeight, int nPitch, Ipp32s nHist[] )
{//ֱ��ͼͳ��-------->
	IppiSize roiSize;
	roiSize.height = nHeight;
	roiSize.width = nWidth;
	//Ipp32s nLevels[257];
	//for( int i = 0; i < 257; i++ )
	//	nLevels[i] = i;
	//memset( nHist, 0, sizeof(int)*256 );
	//ippiHistogramRange_8u_C1R( buf, nPitch, roiSize, nHist, nLevels, 257 );
	//ipp9.0 version
	//* get sizes for spec and buffer */
	IppStatus status;
	const int nBins = 256;
	int nLevels[] = {nBins + 1};
	Ipp32f lowerLevel[] = {0};
	Ipp32f upperLevel[] = {255};
	int nSizeHistObj, nSizeBuffer;
	status = ippiHistogramGetBufferSize(ipp8u, roiSize, nLevels, 1, 1, &nSizeHistObj, &nSizeBuffer);
	IppiHistogramSpec* pHistObj = (IppiHistogramSpec* )ippsMalloc_8u(nSizeHistObj);
	Ipp8u* pBuffer = ippsMalloc_8u(nSizeBuffer);
	//initialize spec */
	status = ippiHistogramUniformInit(ipp8u, lowerLevel, upperLevel, nLevels, 1, pHistObj);
	//calculate histogram
	status = ippiHistogram_8u_C1R(buf, nPitch, roiSize, (Ipp32u*)nHist, pHistObj, pBuffer);
	ippsFree(pBuffer);
	ippsFree(pHistObj);

}
void kxHistogramRegulation( const unsigned char* pSrc, int nWidth, int nHeight, int nSrcPitch, unsigned char* pDst, int nDstPitch, Ipp32s* pHist )
{//ֱ��ͼ�涨�� ͼ�� pSrc�� pHist ���й涨��
	int  nSrcHist[256];
	kxHistogram( pSrc, nWidth, nHeight, nSrcPitch, nSrcHist );

	int   nTable[256];
	int   nSum = 0;
	int   nSrcSum = 0;
	int   k = 0;
	for( int i = 0; i < 256; i++ )
	{
        nSrcSum += nSrcHist[i];
		while( nSum < nSrcSum && k < 256 )
		{
            nSum += pHist[k++];
		}
		assert( k < 256 );
		nTable[i] = k;
	}
    for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
            pDst[y*nDstPitch+x] = nTable[ pSrc[y*nSrcPitch+x] ];
		}
	}
}
void kxHistogramRegulation( const unsigned char* pSrc1, int nWidth, int nHeight, int nSrcPitch1, unsigned char* pDst, int nDstPitch, const unsigned char* pSrc2, int nSrcPitch2  )
{//ֱ��ͼ�涨�� ͼ�� pSrc1�� pSrc2 ���й涨��
    Ipp32s   nHist[256];
	kxHistogram( pSrc2, nWidth, nHeight, nSrcPitch2, nHist );
    kxHistogramRegulation( pSrc1, nWidth, nHeight, nSrcPitch1, pDst, nDstPitch, nHist );
}
void kxHistogramEqual( const unsigned char* pSrc, int nWidth, int nHeight, int nSrcPitch, unsigned char* pDst, int nDstPitch, Ipp32s* pHist )
{//ֱ��ͼ���⻯  pHist ָ�� pSrc �Ѿ� ͳ�Ƶ�ֱ��ͼ
    Ipp32s* p;
	if( pHist == NULL )
	{
		Ipp32s   nHist[256];
		kxHistogram( pSrc, nWidth, nHeight, nSrcPitch, nHist );
        p = nHist;
	}
	else
		p = pHist;
    //----------------------------------------------------------------------------------------------------
    int      nTable[256];
	kxINT64  nSum = 0;
    for( int i = 0; i < 256; i++ )
	{
		nSum += p[i];
        nTable[i] = int(255 * nSum /(nWidth*nHeight));
	}
    for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
            pDst[y*nDstPitch+x] = nTable[ pSrc[y*nSrcPitch+x] ];
		}
	}
}
unsigned char kxOTSU( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch )
{
	Ipp8u  gate;
	IppiSize roiSize;
	roiSize.width = nWidth;
	roiSize.height = nHeight;
	ippiComputeThreshold_Otsu_8u_C1R( pBuf, nPitch, roiSize, &gate );
	return gate;
}
unsigned char kxGateLow( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, float percent )
{ //���°ٷֱ�percent С=======
    Ipp32s   nHist[256];
	kxHistogram( pBuf, nWidth, nHeight, nPitch, nHist );
    int  nSum = 0;
	for( int i = 0; i < 256; i++ )
	{
		nSum += nHist[i];
		if( nSum >= percent*(nWidth*nHeight) )
			return i;
	}
	return (unsigned char)(256);
}
unsigned char kxGateHigh( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, float percent )
{ //���°ٷֱ�percent ��=======
	Ipp32s   nHist[256];
	kxHistogram( pBuf, nWidth, nHeight, nPitch, nHist );
	int  nSum = 0;
	for( int i = 256; i >= 0; i++ )
	{
		nSum += nHist[i];
		if( nSum >= percent*(nWidth*nHeight) )
			return i;
	}
	return 0;
}

void kxBinaryImage( const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, unsigned char gate, IppCmpOp oper )
{
	IppiSize roiSize = { nWidth, nHeight };

	ippiCompareC_8u_C1R( buf, nPitch, gate, pDstBuf, nDstPitch, roiSize, oper );
}
void kxPrejection_X( const unsigned char* buf, int nWidth, int nHeight, int Pitch, int nSum[] )
{//X����ͶӰ
    for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			nSum[y] += buf[y*Pitch+x];
		}
	}
}
void kxPrejection_Y( const unsigned char* buf, int nWidth, int nHeight, int Pitch, int nSum[] )
{//Y����ͶӰ
    for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			nSum[x] += buf[y*Pitch+x];
		}
	}
}

void kxImageSub( const unsigned char* buf1, int nWidth, int nHeight, int nPitch1, const unsigned char* buf2, int nPitch2, unsigned char* buf, int nPitch )
{    //--ͼ�񱥺� �� buf1 - buf2 => buf

	IppiSize   roiSize;
	roiSize.height = nHeight;
	roiSize.width = nWidth;

	ippiSub_8u_C1RSfs( buf2, nPitch2, buf1, nPitch1, buf, nPitch, roiSize, 0 );
}

int kxFindBlocks( int nSum[], int nLen )
{   //��nSum[]�е� ��0 ��������
	int   nCount = 0;
	int   nTag = 0;
	for( int i = 0; i < nLen; i++ )
	{
		if( nSum[i] )
		{
			if( nTag == 0 )
			{
				nTag = 1;
				nCount++;
			}
		}
		else
		{
			if( nTag )
			{
				nTag = 0;
			}
		}
	}
	return nCount;
}
int kxFindMaxBlocks_L( int nSum[], int nLen, int& nStart, int& nEnd )
{ //��nSum[]����һ������������, ���ص���
	int   nDot;
	int   nTag = 0;
	int   x1,x2;
	nStart = nEnd = 0;
	int   nMaxDot = 0;     //���ĵ���
	int   nMaxLen = 0;     //��Ŀ鳤
	for( int i = 0; i < nLen; i++ )
	{
		if( nSum[i] )
		{
			if( nTag == 0 )
			{
				nTag = 1;
				x1 = i;
				nDot = nSum[i];
			}
			else
				nDot += nSum[i];
		}
		else
		{
			if( nTag )
			{
				nTag = 0;
				x2 = i-1;
				if( nMaxLen < x2-x1+1 )
				{
					nMaxDot = nDot;
					nStart = x1;
					nEnd = x2;
					nMaxLen = nEnd-nStart+1;
				}
			}
		}
	}
    if( nTag )
	{
		if( nMaxLen < nLen-x1 )
		{
			nMaxDot = nDot;
			nStart = x1;
			nEnd = nLen-1;
			nMaxLen = nEnd-nStart+1;
		}
	}
	return nMaxDot;
}

int kxFindMaxBlocks_D( int nSum[], int nLen, int& nStart, int& nEnd )
{ //��nSum[]����һ������������, ���ص���
	int   nDot;
	int   nTag = 0;
	int   x1,x2;
	nStart = nEnd = 0;
	int   nMaxDot = 0;
	for( int i = 0; i < nLen; i++ )
	{
		if( nSum[i] )
		{
			if( nTag == 0 )
			{
				nTag = 1;
				x1 = i;
				nDot = nSum[i];
			}
			else
				nDot += nSum[i];
		}
		else
		{
			if( nTag )
			{
				nTag = 0;
				x2 = i-1;
				if( nDot > nMaxDot )
				{
					nMaxDot = nDot;
					nStart = x1;
					nEnd = x2;
				}
			}
		}
	}
	if( nTag )
	{
		if( nDot > nMaxDot )
		{
			nMaxDot = nDot;
			nStart = x1;
			nEnd = nLen-1;
		}
	}
	return nMaxDot;
}


int kxCountDots( const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char gate, int oper )
{   //ͳ�����������ĵ���==================================
	assert( oper >= _CmpGt && oper <= _CmpLe );
	Ipp8u lowBound, upBound;
	switch( oper )
	{
	case _CmpGt:
		upBound = 255;
		lowBound = gate+1;
		break;
	case _CmpGe:
		upBound = 255;
		lowBound = gate;
		break;
	case _CmpEq:
		upBound = gate;
		lowBound = gate;
		break;
	case _CmpLt:
		upBound = gate-1;
		lowBound = 0;
		break;
	case _CmpLe:
		upBound = gate;
		lowBound = 0;
		break;
	default:
		upBound = 0;
		lowBound = 0;
		break;
	}
	IppiSize  roiSize = { nWidth, nHeight };
	int  nCount;
	ippiCountInRange_8u_C1R( buf, nPitch, roiSize, &nCount, lowBound, upBound );
	return nCount;
}

int UDNoiseEdgeDetect( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int Sdirect, int Gate, int op,
	int movePitch, int Limit, int Depth, int MaxLimit)
{//����/�� ��
//---��ͼ�� buf : nWidth X nHeight X nPitch ��������[��������]�������ҵ�Depth��>= Limit�� ˮƽ�ߣ�����ĳ���ҵ�>= MaxLimit��ˮƽ�ߣ�����Ϊ�ҵ���
//----Sdirect = _SearchTop / _SearchBottom
//----op = _CmpGt / _CmpGe / ....
    assert( Sdirect == _SearchTop || Sdirect == _SearchBottom );
	assert( movePitch <= nHeight );
    int  nCount = 0;
    int  nStartY;
	int  nStepY = movePitch * Sdirect; //
	if( Sdirect == _SearchTop )
		nStartY = 0;
	else
		nStartY = nHeight-1;
	while( 1 )
	{
        int   dot = kxCountDots( buf + nStartY*nPitch, nWidth, 1, nPitch, Gate, op );   //ͳ�����������ĵ���
		if( dot >= MaxLimit )
		{
			return nStartY - nCount * nStepY;
		}
		if( dot >= Limit )
		{
			nCount++;
		}
		else
		{
			if( nCount )
				nCount--;
		}
		if( nCount >= Depth )
		{
			return nStartY - nCount * nStepY;
		}
		nStartY += nStepY;
		if( nStartY < 0 || nStartY >nHeight-1 )
			break;
	}
	return nStartY - nCount * nStepY;
}
int LRNoiseEdgeDetect( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int Sdirect, int Gate, int op,
	int movePitch, int Limit, int Depth, int MaxLimit)
{ //����/�� ��
//---��ͼ�� buf : nWidth X nHeight X nPitch ��������[��������]�������ҵ�Depth��>= Limit�� ��ֱ�ߣ�����ĳ���ҵ�>= MaxLimit�Ĵ�ֱ�ߣ�����Ϊ�ҵ���
//----Sdirect = _SearchLeft / SearchRight
//----op = _CmpGt / _CmpGe / ....
    assert( Sdirect == _SearchLeft || Sdirect == _SearchRight );
	assert( movePitch <= nWidth );
    int  nCount = 0;
    int  nStartX;
	int  nStepX = movePitch * Sdirect; //
	if( Sdirect == _SearchLeft )
		nStartX = 0;
	else
		nStartX = nWidth-1;
	while( 1 )
	{
        int   dot = kxCountDots( buf + nStartX, 1, nHeight, nPitch, Gate, op );   //ͳ�����������ĵ���
		if( dot >= MaxLimit )
		{
			return nStartX - nCount * nStepX;
		}
		if( dot >= Limit )
		{
			nCount++;
		}
		else
		{
			if( nCount )
				nCount--;
		}
		if( nCount >= Depth )
		{
			return nStartX - nCount * nStepX;
		}
		nStartX += nStepX;
		if( nStartX < 0 || nStartX >nWidth-1 )
			break;
	}
	return nStartX - nCount * nStepX;
}

void FindImagePosGt( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, kxPoint<int>& pt, unsigned char lowBound, unsigned char upBound )
{//�� ͼ�� pBuf : nWidth X nHeight X nPitch �� �ҵ���������������, ��ͼ��Ҷ�����
//���:( pt.x, pt.y ) Ҫ���ҵĴ�С; ����:( pt.x, pt.y )�ҵ������Ͻ�����
	assert( pt.x < nWidth && pt.y < nHeight );
	Ipp32s*   pSumX = (Ipp32s*)ippNew( sizeof(int)*nHeight );
	Ipp32s*   pSumY = (Ipp32s*)ippNew( sizeof(int)*nWidth );

	ippsZero_32s( pSumX, nHeight );
	ippsZero_32s( pSumY, nWidth );
    //----׼��ͶӰ
	for( int y = 0; y < nHeight; y++ )
	{
		for( int x = 0; x < nWidth; x++ )
		{
			if( pBuf[y*nPitch+x] >= lowBound && pBuf[y*nPitch+x] <= upBound )
			{
			    pSumX[y] += pBuf[y*nPitch+x];

			    pSumY[x] += pBuf[y*nPitch+x];
			}
		}
	}

	int   nSum = 0;
	int   x;
	for( x = 0; x < pt.x; x++)
		nSum += pSumY[x];
	int   nSumMaxX = nSum;
	int   nPosX = 0;

	for( x = 1; x <= nWidth - pt.x; x++ )
	{
		nSum -= pSumY[x-1];
		nSum += pSumY[x-1+pt.x];
		if( nSum > nSumMaxX )
		{
			nSumMaxX = nSum;
			nPosX = x;
		}
	}

	int   y;
	for( y = 0; y < pt.y; y++)
		nSum += pSumX[y];
	int   nSumMaxY = nSum;
	int   nPosY = 0;

	for( y = 1; y <= nHeight - pt.y; y++ )
	{
		nSum -= pSumX[y-1];
		nSum += pSumX[y-1+pt.y];
		if( nSum > nSumMaxY )
		{
			nSumMaxY = nSum;
			nPosY = y;
		}
	}
	ippDelete( pSumX );
	ippDelete( pSumY );
	pt.x = nPosX;
	pt.y = nPosY;
}

//--------------------------------------------------------------------------------------------------------------------
void kxThinImage( const unsigned char* buf, int w, int h, int nPitch, unsigned char* pDstBuf, int nDstPitch )
{/***************************
				  p1   p2   p3   p4   p5
				  p6   p7   p8   p9   p10
				  p11  p12 [p13] p14  p15
				  p16  p17  p18  p19  p20
				  p21  p22  p23  p24  p25
**************************************/
	enum
	{
		p1 = 0,
		p2 = 1,
		p3 = 2,
		p4 = 3,
		p5 = 4,
		p6 = 5,
		p7 = 6,
		p8 = 7,
		p9 = 8,
		p10 = 9,
		p11 = 10,
		p12 = 11,
		p13 = 12,
		p14 = 13,
		p15 = 14,
		p16 = 15,
		p17 = 16,
		p18 = 17,
		p19 = 18,
		p20 = 19,
		p21 = 20,
		p22 = 21,
		p23 = 22,
		p24 = 23,
		p25 = 24,
	};
#define GetBufGray( x, y, pt )   pThinBuf[nSrcIndex][(y+pt.y)*nThinPitch[nSrcIndex] + x +pt.x ]
	kxPoint<int>   pt[25];   //5 X 5

	for( int i = 0; i < 25; i++ )
	{
		pt[i].x = i % 5 -2;
		pt[i].y = i / 5 -2;
	}
	/*********************** ���� ���ĵ� p13 ��� ����4������ͬʱ���㣬 ���ɾ���õ㣬 ����ֱ����û�е��ɾ��
	//      buf �Ƕ�ֵͼ�� Ҫϸ���ļ��� �ǻҶ�Ϊ0xFF�ĵ�
	1, 2<= N(p13) <= 6, N(p13)��ʾ p13�� 8��������� �������
	2��Z(p13) = 1, Z(p13)  p8, p7, p12, p17, p18, p19, p14, p9, p8 ˳�򣬴�0�䵽0xFF�Ĵ���
	3��P8 * p12 * p14 = 0 �� Z(p8) <> 1
	4��P8 * p12 * p18 = 0 �� Z(p12) <> 1
	*****************************************/
	int   nTmpPitch = w;
    unsigned char*  pTmpBuf = new unsigned char[h*nTmpPitch];
	for( int col = 0; col < 2; col++ )
	{
		memset( pDstBuf + col*nDstPitch, 0, w );
		memset( pDstBuf + (h-1-col)*nDstPitch, 0, w );
		memset( pTmpBuf + col*nTmpPitch, 0, w );
		memset( pTmpBuf + (h-1-col)*nTmpPitch, 0, w );
	}
	for( int row = 2; row < h-2; row++ )
	{
		*((short*)(pDstBuf + row*nDstPitch )) = 0;
		*((short*)(pDstBuf + row*nDstPitch + w-2 )) = 0;
		*((short*)(pTmpBuf + row*nTmpPitch )) = 0;
		*((short*)(pTmpBuf + row*nTmpPitch + w-2 )) = 0;
	}

	unsigned char*  pThinBuf[] = { const_cast<unsigned char*>(buf), pDstBuf };
	int   nThinPitch[] = { nPitch, nDstPitch };
	int   nDelPts = 0;
	int   nLoop = 0;
	do
	{
		int   nSrcIndex = nLoop % 2;
		int   nDstIndex = 1 - nSrcIndex;

		nDelPts = 0;
		for( int y = 2; y < h-2; y++ )
		{
			for( int x = 2; x < w-2; x++ )
			{
				if( pThinBuf[nSrcIndex][y*nThinPitch[nSrcIndex] + x ] == 0 )
				{
					pThinBuf[nDstIndex][y*nThinPitch[nDstIndex] + x ] = 0;
					continue;
				}
				//1
				int  nDots = 0;
				if( GetBufGray( x, y, pt[p7] ) )
					nDots++;
				if( GetBufGray( x, y, pt[p8] ) )
					nDots++;
				if( GetBufGray( x, y, pt[p9] ) )
					nDots++;
				if( GetBufGray( x, y, pt[p12] ) )
					nDots++;
				if( GetBufGray( x, y, pt[p14] ) )
					nDots++;
				if( GetBufGray( x, y, pt[p17] ) )
					nDots++;
				if( GetBufGray( x, y, pt[p18] ) )
					nDots++;
				if( GetBufGray( x, y, pt[p19] ) )
					nDots++;
				if( nDots < 2 || nDots > 6 )
				{
					pThinBuf[nDstIndex][y*nThinPitch[nDstIndex] + x ] = 0xFF;
					continue;
				}
				nDots = 0;
				//2-----------------------------------
				if( GetBufGray( x, y, pt[p8] ) == 0 &&  GetBufGray( x, y, pt[p7] ) != 0 )
					nDots++;
				if( GetBufGray( x, y, pt[p7] ) == 0 &&  GetBufGray( x, y, pt[p12] ) != 0 )
					nDots++;
				if( GetBufGray( x, y, pt[p12] ) == 0 &&  GetBufGray( x, y, pt[p17] ) != 0 )
					nDots++;
				if( GetBufGray( x, y, pt[p17] ) == 0 &&  GetBufGray( x, y, pt[p18] ) != 0 )
					nDots++;
				if( GetBufGray( x, y, pt[p18] ) == 0 &&  GetBufGray( x, y, pt[p19] ) != 0 )
					nDots++;
				if( GetBufGray( x, y, pt[p19] ) == 0 &&  GetBufGray( x, y, pt[p14] ) != 0 )
					nDots++;
				if( GetBufGray( x, y, pt[p14] ) == 0 &&  GetBufGray( x, y, pt[p9] ) != 0 )
					nDots++;
				if( GetBufGray( x, y, pt[p9] ) == 0 &&  GetBufGray( x, y, pt[p8] ) != 0 )
					nDots++;
				if( nDots != 1 )
				{
					pThinBuf[nDstIndex][y*nThinPitch[nDstIndex] + x ] = 0xFF;
					continue;
				}
				//3------------------------------------------------
				int  nX = ((int)GetBufGray(x,y,pt[p8])) * GetBufGray(x,y,pt[p12]) * GetBufGray(x,y,pt[p14]);
				if( nX != 0 )
				{//Z(p8)   p3,p2,p7,p12,p13,p14,p9,p4,p3
					nDots = 0;
					if( GetBufGray( x, y, pt[p3] ) == 0 &&  GetBufGray( x, y, pt[p2] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p2] ) == 0 &&  GetBufGray( x, y, pt[p7] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p7] ) == 0 &&  GetBufGray( x, y, pt[p12] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p12] ) == 0 &&  GetBufGray( x, y, pt[p13] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p13] ) == 0 &&  GetBufGray( x, y, pt[p14] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p14] ) == 0 &&  GetBufGray( x, y, pt[p9] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p9] ) == 0 &&  GetBufGray( x, y, pt[p4] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p4] ) == 0 &&  GetBufGray( x, y, pt[p3] ) != 0 )
						nDots++;
					if( nDots == 1 )
					{
						pThinBuf[nDstIndex][y*nThinPitch[nDstIndex] + x ] = 0xFF;
						continue;
					}
				}
				//4------------------------------------------------
				nX = (int)GetBufGray(x,y,pt[p8]) * GetBufGray(x,y,pt[p12]) * GetBufGray(x,y,pt[p18]);
				if( nX != 0 )
				{//Z(p12)   p7,p6,p11,p16,p17,p18,p13,p8,p7
					nDots = 0;
					if( GetBufGray( x, y, pt[p7] ) == 0 &&  GetBufGray( x, y, pt[p6] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p6] ) == 0 &&  GetBufGray( x, y, pt[p11] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p11] ) == 0 &&  GetBufGray( x, y, pt[p16] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p16] ) == 0 &&  GetBufGray( x, y, pt[p17] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p17] ) == 0 &&  GetBufGray( x, y, pt[p18] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p18] ) == 0 &&  GetBufGray( x, y, pt[p13] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p13] ) == 0 &&  GetBufGray( x, y, pt[p8] ) != 0 )
						nDots++;
					if( GetBufGray( x, y, pt[p8] ) == 0 &&  GetBufGray( x, y, pt[p7] ) != 0 )
						nDots++;
					if( nDots == 1 )
					{
						pThinBuf[nDstIndex][y*nThinPitch[nDstIndex] + x ] = 0xFF;
						continue;
					}
				}
				pThinBuf[nDstIndex][y*nThinPitch[nDstIndex] + x ] = 0x0;
				nDelPts++;
			}
		}
		nLoop++;
        if( nLoop == 1)
		{
			pThinBuf[0] = pTmpBuf;
            nThinPitch[0] = nTmpPitch;
		}
		//memcpy( buf, pDstBuf, h*nPitch );
	}while( nDelPts != 0 );
    if( !(nLoop & 0x1) )
	{
		for( int y = 0; y < h; y++ )
		{
			memcpy( pDstBuf + y*nDstPitch, pTmpBuf + y * nTmpPitch, w );
		}
	}
	delete []pTmpBuf;
}

void kxMakeRingMask( unsigned char* buf, int width, int height, int pitch, float x0, float y0, float a, float b, unsigned char nMask )
{//-----��ͼ�� buf: width X height X pitch�� ���ģ�x0,y0��[���Ͻ�������ԭ�㣬������Y�᷽��]��X��a,Y��b����Բ
    assert( a < width + 5 && b < height + 5 );
	assert( a > 0 && b > 0 );
	int   x, y;
	for( x = 0; x < width; x++ )
	{
		double fxy = 1-(x-x0)*(x-x0)/(a*a);
		if( fxy < 0 )
			continue;
        float f = float(b * sqrt(fxy ));
		y = int(y0+f);
		if( y >=0 && y < height )
		{
			buf[y*pitch+x] = nMask;
		}
		y = int(y0-f);
		if( y >=0 && y < height )
		{
			buf[y*pitch+x] = nMask;
		}
	}

	for( y = 0; y < height; y++ )
	{
		double fxy = 1-(y-y0)*(y-y0)/(b*b);
		if( fxy < 0 )
			continue;
        float f = float(a * sqrt(fxy ));
		x = int(x0+f);
		if( x >=0 && x < width )
		{
			buf[y*pitch+x] = nMask;
		}
		x = int(x0-f);
		if( x >=0 && x < width )
		{
			buf[y*pitch+x] = nMask;
		}
	}
}

void kxCopyImgData( unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch )
{
	unsigned char*  srcBuf = const_cast<unsigned char*>(pSrc);
	unsigned char*  dstBuf = pDst;
	int  width;
	int  height;
    int  dy = nDstHeight - nSrcHeight;
	if( dy > 0 )
	{
		height = nSrcHeight;
		dstBuf += ( dy/2 )*nDstPitch;
	}
	else
	{
		height = nDstHeight;
		srcBuf -= ( dy/2 )*nSrcPitch;
	}
    int  dx = nDstWidth - nSrcWidth;
    if( dx > 0 )
	{
		width = nSrcWidth;
		dstBuf += ( dx/2 );
	}
	else
	{
		width = nDstWidth;
		srcBuf -= ( dx/2 );
	}
    for( int y = 0; y < height; y++ )
	{
		memcpy( dstBuf+y*nDstPitch, srcBuf+y*nSrcPitch, width );
	}
}
void kxCopyImgRGBData( unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, const unsigned char* pSrc[], int nSrcWidth, int nSrcHeight, int nSrcPitch )
{
	unsigned char*  srcBufR = const_cast<unsigned char*>(pSrc[2]);
	unsigned char*  srcBufG = const_cast<unsigned char*>(pSrc[1]);
	unsigned char*  srcBufB = const_cast<unsigned char*>(pSrc[0]);

	unsigned char*  dstBuf = pDst;
	int  width;
	int  height;
	int  dy = nDstHeight - nSrcHeight;
	if( dy > 0 )
	{
		height = nSrcHeight;
		dstBuf += ( dy/2 )*nDstPitch;
	}
	else
	{
		height = nDstHeight;
		srcBufR -= ( dy/2 )*nSrcPitch;
		srcBufG -= ( dy/2 )*nSrcPitch;
		srcBufB -= ( dy/2 )*nSrcPitch;
	}
	int  dx = nDstWidth - nSrcWidth;
	if( dx > 0 )
	{
		width = nSrcWidth;
		dstBuf += ( dx/2 )*4;
	}
	else
	{
		width = nDstWidth;
		srcBufR -= ( dx/2 );
		srcBufG -= ( dx/2 );
		srcBufB -= ( dx/2 );
	}
	const unsigned char * pS[4] ={srcBufB,srcBufG,srcBufR,srcBufR};
	IppiSize CpySize ={width,height};
    ippiCopy_8u_P4C4R(pS,nSrcPitch,dstBuf,nDstPitch,CpySize);
}
void kxFindPosition_UD( const unsigned char* buf, int width, int height, int pitch, int& top, int& bottom, unsigned char gate )
{//����gate �� ͼ��buf ��  ��ȷ��λ �� �� �߽�




}

void kxFindPosition_LR( const unsigned char* buf, int width, int height, int pitch, int& left, int& right, unsigned char gate )
{//����gate �� ͼ��buf ��  ��ȷ��λ �� �� �߽�


}
void  RotateImg(kxImageBuf& Src,kxImageBuf& Dst,unsigned char* pTemp,int Angle)
{
	int  n_w = Src.nWidth;  int n_h = Src.nHeight;  int n_p = Src.nPitch;
	IppiSize  Siz={n_w,n_h};
	switch (Angle)
	{
	case  _Angle_0:
		Dst.Release();
		Dst.Clone(Src);
		break;
	case _Angle_90:
		Dst.Init(n_h,n_w);
		//m_RotateMid.Init(n_w,n_h);
		ippiMirror_8u_C1R(Src.buf,n_p,pTemp,Src.nPitch,Siz,ippAxsHorizontal);
		ippiTranspose_8u_C1R(pTemp,Src.nPitch,Dst.buf,Dst.nPitch,Siz);
		break;
	case _Angle_180:
		Dst.Init(n_w,n_h);
		ippiMirror_8u_C1R(Src.buf,n_p,Dst.buf,Dst.nPitch,Siz,ippAxsBoth);
		break;
	case _Angle_270:
		Dst.Init(n_h,n_w);
		//m_RotateMid.Init(n_w,n_h);
		ippiMirror_8u_C1R(Src.buf,n_p,pTemp,Src.nPitch,Siz,ippAxsVertical);
		ippiTranspose_8u_C1R(pTemp,Src.nPitch,Dst.buf,Dst.nPitch,Siz);
		break;
	default:
		break;
	}
}

int kxRound(float value)
{
    return (int)(value + (value >= 0 ? 0.5f : -0.5f));
}


int kxRound(int value)
{
	return value;
}

int kxFloor(float value)
{
	int i = (int)value;
	return i - (i > value);
}

int kxFloor(int value)
{
	return value;
}

int kxCeil(float value)
{
	int i = (int)value;
	return i + (i < value);
}

int kxCeil(int value)
{
	return value;
}


