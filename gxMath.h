#ifndef GXMATHHHHHH_
#define GXMATHHHHHH_


#include "kxDef.h"

	int IsImgErr( const unsigned char* buf );//  [10/14/2014 HZZ]
	void SetImgErr( unsigned char* buf, int nMark);

    //---  ���е�pitch���� �ֽڵ�λ 
	//float kxImageAlign( kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts );
	// ��ͼ�� srcBuf�� sw X sh ��Χ  ���� ͼ��tplBuf[ tw X th ]�� ����ֵ��������ƶ�[0--1]�� tplBuf�� srcBuf�е����λ�� 
	//float kxImageAlign( kxPoint<float>& pos, const Ipp16u* srcBuf, int sw, int sh, int ss, const Ipp16u* tplBuf, int tw, int th, int ts );
	// ��ͼ�� srcBuf�� sw X sh ��Χ  ���� ͼ��tplBuf[ tw X th ]�� ����ֵ��������ƶ�[0--1]�� tplBuf�� srcBuf�е����λ�� 
	void kxSetRegion( unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char val );
	//�� buf �� �������� => val   [*** IPPs]
	void kxCopyImageRegion( const unsigned char* pSrc, int nWidth, int nHeight, int SrcPitch, unsigned char* pDst, int DstPitch );
    //�� pSrc�� Copy nWidth X nHeight ===> pDst
	//void kxResizeImage( const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int SrcPitch, unsigned char* pDst, int nDstWidth, int nDstHeight, int DstPitch, int oper= IPPI_INTER_LINEAR );
    // ��ͼ�� pSrc[ nSrcWidth X nSrcHeight ] ����/ѹ�� ==��pDst[ nDstWidth X nDstHeight ]   nType = [IPPI_INTER_LINEAR/IPPI_INTER_NN/IPPI_INTER_CUBIC]
    //    oper = IPPI_INTER_NN or IPPI_INTER_LINEAR or = IPPI_INTER_CUBIC or IPPI_INTER_LANCZOS or IPPI_INTER_SUPER[ѹ��ʱ ѡ��� ͼ����ʵ] 

	//void kxResizeImage_RGB( const unsigned char* pSrc[], int nSrcWidth, int nSrcHeight, int SrcPitch, unsigned char* pDst[], int nDstWidth, int nDstHeight, int DstPitch, int oper= IPPI_INTER_NN ) ;

	void kxHistogram( const unsigned char* buf, int nWidth, int nHeight, int nPitch, Ipp32s nHist[] );
	//ֱ��ͼͳ��-------->
	void kxHistogramRegulation( const unsigned char* pSrc, int nWidth, int nHeight, int nSrcPitch, unsigned char* pDst, int nDstPitch, Ipp32s* pHist );
	//ֱ��ͼ�涨�� ͼ�� pSrc�� pHist ���й涨��
	void kxHistogramRegulation( const unsigned char* pSrc1, int nWidth, int nHeight, int nSrcPitch1, unsigned char* pDst, int nDstPitch, const unsigned char* pSrc2, int nSrcPitch2  );
	//ֱ��ͼ�涨�� ͼ�� pSrc1�� pSrc2 ���й涨��

	void kxHistogramEqual( const unsigned char* pSrc, int nWidth, int nHeight, int nSrcPitch, unsigned char* pDst, int nDstPitch, Ipp32s* pHist = NULL );
	//ֱ��ͼ���⻯  pHist ָ�� pSrc �Ѿ� ͳ�Ƶ�ֱ��ͼ 
    unsigned char kxOTSU( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch ); 
    
	unsigned char kxGateLow( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, float percent ); 
	//���°ٷֱ�percent[�� 0.30]С�������°ٷֱ� percent �� 
	unsigned char kxGateHigh( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, float percent ); 
	//���°ٷֱ�percent[�� 0.30]�󣻼����°ٷֱ� percent �� 
	
	void kxBinaryImage( const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, unsigned char gate, IppCmpOp oper );
    //-��ֵ��---oper = ippCmpLessEq / ippCmpLess / ippCmpEq / ippCmpGreaterEq / ippCmpGreater
    //------------------------------------------------------------------------------------------------------------------------------------ 
	void kxPrejection_X( const unsigned char* buf, int nWidth, int nHeight, int Pitch, int nSum[] );//X����ͶӰ[ע��nSum ������ �Լ�����]
	void kxPrejection_Y( const unsigned char* buf, int nWidth, int nHeight, int Pitch, int nSum[] );//Y����ͶӰ[ע��nSum ������ �Լ�����]
	
    void kxThinImage( const unsigned char* buf, int w, int h, int nPitch, unsigned char* pDstBuf, int nDstPitch );//ϸ��=> pDstBuf
    
    int kxFindBlocks( int nSum[], int nLen );   //��nSum[]�е� ��0 ��������
    int kxFindMaxBlocks_L( int nSum[], int nLen, int& nStart, int& nEnd ); //��nSum[]����һ����󡾳��ȡ���������, ���ص��� 
	int kxFindMaxBlocks_D( int nSum[], int nLen, int& nStart, int& nEnd ); //��nSum[]����һ����󡾵�����ࡿ��������, ���ص��� 

    void FindImagePosGt( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, kxPoint<int>& pt, unsigned char gate1, unsigned char gate2  );
	//�� ͼ�� pBuf : nWidth X nHeight X nPitch �� �ҵ���������������, ��ͼ��Ҷ�����   gate1<=   <= gate2
	//���:( pt.x, pt.y ) Ҫ���ҵĴ�С; ����:( pt.x, pt.y )�ҵ������Ͻ�����
    //--------------------------------------------------------------------------------------------------------- 
	void kxCopyImgData( unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch );
	void kxCopyImgRGBData( unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, const unsigned char* pSrc[], int nSrcWidth, int nSrcHeight, int nSrcPitch );

	//------- �� pSrc  Copy data to pDst, ����ͼ����ܲ�һ���� 1�����pSrc�� ��copy pSrc�м�=>pDst; 2�����pDst����Copy pSrc =>pDst�м�
	enum
	{
	    _SearchTop = 1,     //���ϱ�  ���� �� ��
        _SearchLeft = 1,    //�����  ���� �� ��
		_SearchBottom = -1, //���±�  ���� �� ��
		_SearchRight = -1,  //���ұ�  ���� �� ��
	};
	enum
	{
		_CmpGt = 0,     // >
        _CmpGe = 1,     // >=
		_CmpEq = 2,     // =
		_CmpLt = 3,     // <
		_CmpLe = 4      // <=
	}; 
	int kxCountDots( const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char gate, int oper );//oper = _CmpGt / ....
    //--ͳ�� ���������ĵ���
    int UDNoiseEdgeDetect( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int Sdirect, int Gate, int op, 
	    int movePitch, int Limit, int Depth, int MaxLimit); //����/�� ��
    //---��ͼ�� buf : nWidth X nHeight X nPitch ��������[��������]�������ҵ�Depth��>= Limit�� ˮƽ�ߣ�����ĳ���ҵ�>= MaxLimit��ˮƽ�ߣ�����Ϊ�ҵ���
	//----Sdirect = _SearchTop / _SearchBottom
	//----op = _CmpGt / _CmpGe / ....
    int LRNoiseEdgeDetect( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int Sdirect, int Gate, int op, 
	    int movePitch, int Limit, int Depth, int MaxLimit); //����/�� ��
    //---��ͼ�� buf : nWidth X nHeight X nPitch ��������[��������]�������ҵ�Depth��>= Limit�� ��ֱ�ߣ�����ĳ���ҵ�>= MaxLimit�Ĵ�ֱ�ߣ�����Ϊ�ҵ���
	//----Sdirect = _SearchLeft / SearchRight
	//----op = _CmpGt / _CmpGe / ....

    //-----------------------------------------------------------------------------------------------------------------------
    void kxImageSub( const unsigned char* buf1, int nWidth, int nHeight, int nPitch1, const unsigned char* buf2, int nPitch2, unsigned char* buf, int nPitch );
    //--ͼ�񱥺� �� buf1 - buf2 => buf

    void kxMakeRingMask( unsigned char* buf, int width, int height, int pitch, float x0, float y0, float a, float b, unsigned char nMask = 0xFF );
    //-----��ͼ�� buf: width X height X pitch�� ���ģ�x0,y0��[���Ͻ�������ԭ�㣬������Y�᷽��]��X��a,Y��b����Բ 
    //-----ע�� �����Ҷ� �ɵ����� �Լ�����
    void kxFindPosition_UD( const unsigned char* buf, int width, int height, int pitch, int& top, int& bottom, unsigned char gate = 0 );    
    //����gate �� ͼ��buf ��  ��ȷ��λ �� �� �߽�
	void kxFindPosition_LR( const unsigned char* buf, int width, int height, int pitch, int& left, int& right, unsigned char gate = 0 );    
	//����gate �� ͼ��buf ��  ��ȷ��λ �� �� �߽�
	enum
	{
		_Angle_0 = 0,      /////////˳ʱ����ת
		_Angle_90,
		_Angle_180,
		_Angle_270,

	};
	void RotateImg(kxImageBuf& Src,kxImageBuf& Dst,unsigned char* pTemp,int Angle);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template< class T > void kxSmooth53( T srcData[], T dstData[], int nSize )
	{//-----5��3��ƽ��
		assert( nSize >= 5 );
		dstData[0] = (69.0 * srcData[0] + 4.0 * srcData[1] - 6.0 * srcData[2] + 4.0 * srcData[3] - srcData[4])/70.0;
        dstData[1] = ( 2.0 * srcData[0] + 27.0 * srcData[1] + 12.0 * srcData[2] - 8.0 * srcData[3] + 2.0 * srcData[4]) / 35.0;
		for( int i = 2; i < nSize-2; i++ )
		{
            dstData[i] = ( -3.0 * srcData[i-2] + 12.0 * srcData[i-1] + 17.0 * srcData[i] + 12.0 * srcData[i+1] - 3.0 * srcData[i+2] )/35.0;  
		}
		dstData[nSize-2] = ( 2.0 * srcData[nSize-5] - 8.0 * srcData[nSize-4] + 12.0 * srcData[nSize-3] + 27.0 * srcData[nSize-2] + 2.0 * srcData[nSize-1] )/35.0;
        dstData[nSize-1] = ( -srcData[nSize-5] + 4.0 * srcData[nSize-4] - 6.0 * srcData[nSize-3] + 4.0 * srcData[nSize-2] + 69.0 * srcData[nSize-1]) / 70.0;
	}
	//---�������ϵ��
	template < class T >
	float  kxgetGrayRxy( const T* pSrc1, const T*  pSrc2, int nWidth, int nHeight, int Pitch1, int Pitch2 ) 
	{//---�������ϵ��
		int     offset1 = 0;
		int     offset2 = 0;

		double  sum1 = 0;
		double  sum2 = 0;
		for(int y = 0; y < nHeight; y++ )
		{
			for(int x = 0;  x < nWidth; x++ )
			{
				sum1 += ((double)pSrc1[offset1+x]);
				sum2 += ((double)pSrc2[offset2+x]);
			}
			offset1 += Pitch1;
			offset2 += Pitch2;
		}
		sum1 /= (nWidth*nHeight);
		sum2 /= (nWidth*nHeight);
		double   averGray1 = 0;
		double   averGray2 = 0;
		double   averGray = 0;
		
		offset1 = 0;
		offset2 = 0;
		for(int y = 0;  y < nHeight; y++ )
		{
			for(int x = 0; x < nWidth; x++ )
			{
				///pSrc1[offset1+x] = 10;
				double f1 = pSrc1[offset1+x] - sum1;
				double f2 = pSrc2[offset2+x] - sum2;
				averGray1 += f1 * f1;
				averGray2 += f2 * f2;
				averGray += f1 * f2;
			}
			offset1 += Pitch1;
			offset2 += Pitch2;
		}
		
		float   Rxy = float(averGray / sqrt(averGray1*averGray2));  
		
		return (Rxy);
	}
	
	template < class T >
	float kxgetGraySigma( const T* pSrc, int nWidth, int nHeight, int Pitch )
	{//�����׼��
		int    offset = 0;
		int    x, y;
		float  fSum = 0;
		for( y = 0; y < nHeight; y++)
		{
			for( x = 0; x < nWidth; x++ )
			{
				fSum += pSrc[offset+x];
			}
			offset += Pitch;
		}
		float aver = fSum / ( nWidth*nHeight);
		fSum = 0;
		for( y = 0; y < nHeight; y++)
		{
			for( x = 0; x < nWidth; x++ )
			{
				fSum += (pSrc[offset+x] - aver) * (pSrc[offset+x] - aver);
			}
			offset += Pitch;
		}
		return fSum;
	}
    template< class T >
	float kxgetlinefunction( T pt[], int n, double& alf )
	{
		int  averx = 0, avery = 0;
		for ( int i = 0; i < n; i++ )
		{
			averx += pt[i].x;
			avery += pt[i].y;
		}
		double ax = averx / double(n);
		double ay = avery / double(n);
		
		double lxx = 0;
		double lxy = 0.0;
		double lyy = 0;
		for ( int j = 0; j < n; j++ )
		{
			lxx = lxx + ( pt[j].x-ax ) * ( pt[j].x-ax ); 
			lxy = lxy + ( pt[j].x-ax ) * ( pt[j].y-ay ); 
			lyy = lyy + (pt[j].y-ay) * (pt[j].y-ay);
		}
		float r = 1;
		if( lxx < DBL_MIN )
		{
            alf = PI/2; 
		}
		else
		{
		    alf = atan(lxy / lxx);
		    r = lxy/sqrt( lxx*lyy );
		}
		return fabs(r);
	}


	int kxRound(float value);
	int kxRound(int value);
	int kxFloor(float value);
	int kxFloor(int value);
	int kxCeil(float value);
	int kxCeil(int value);

#endif