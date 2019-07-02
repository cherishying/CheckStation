#ifndef GXMATHHHHHH_
#define GXMATHHHHHH_


#include "kxDef.h"

	int IsImgErr( const unsigned char* buf );//  [10/14/2014 HZZ]
	void SetImgErr( unsigned char* buf, int nMark);

    //---  所有的pitch都是 字节单位 
	//float kxImageAlign( kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts );
	// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置 
	//float kxImageAlign( kxPoint<float>& pos, const Ipp16u* srcBuf, int sw, int sh, int ss, const Ipp16u* tplBuf, int tw, int th, int ts );
	// 在图像 srcBuf的 sw X sh 范围  搜索 图像tplBuf[ tw X th ]， 返回值是最大相似度[0--1]， tplBuf在 srcBuf中的相对位置 
	void kxSetRegion( unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char val );
	//将 buf 的 区域设置 => val   [*** IPPs]
	void kxCopyImageRegion( const unsigned char* pSrc, int nWidth, int nHeight, int SrcPitch, unsigned char* pDst, int DstPitch );
    //从 pSrc中 Copy nWidth X nHeight ===> pDst
	//void kxResizeImage( const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int SrcPitch, unsigned char* pDst, int nDstWidth, int nDstHeight, int DstPitch, int oper= IPPI_INTER_LINEAR );
    // 将图像 pSrc[ nSrcWidth X nSrcHeight ] 拉伸/压缩 ==〉pDst[ nDstWidth X nDstHeight ]   nType = [IPPI_INTER_LINEAR/IPPI_INTER_NN/IPPI_INTER_CUBIC]
    //    oper = IPPI_INTER_NN or IPPI_INTER_LINEAR or = IPPI_INTER_CUBIC or IPPI_INTER_LANCZOS or IPPI_INTER_SUPER[压缩时 选该项， 图像真实] 

	//void kxResizeImage_RGB( const unsigned char* pSrc[], int nSrcWidth, int nSrcHeight, int SrcPitch, unsigned char* pDst[], int nDstWidth, int nDstHeight, int DstPitch, int oper= IPPI_INTER_NN ) ;

	void kxHistogram( const unsigned char* buf, int nWidth, int nHeight, int nPitch, Ipp32s nHist[] );
	//直方图统计-------->
	void kxHistogramRegulation( const unsigned char* pSrc, int nWidth, int nHeight, int nSrcPitch, unsigned char* pDst, int nDstPitch, Ipp32s* pHist );
	//直方图规定化 图像 pSrc按 pHist 进行规定化
	void kxHistogramRegulation( const unsigned char* pSrc1, int nWidth, int nHeight, int nSrcPitch1, unsigned char* pDst, int nDstPitch, const unsigned char* pSrc2, int nSrcPitch2  );
	//直方图规定化 图像 pSrc1按 pSrc2 进行规定化

	void kxHistogramEqual( const unsigned char* pSrc, int nWidth, int nHeight, int nSrcPitch, unsigned char* pDst, int nDstPitch, Ipp32s* pHist = NULL );
	//直方图均衡化  pHist 指向 pSrc 已经 统计的直方图 
    unsigned char kxOTSU( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch ); 
    
	unsigned char kxGateLow( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, float percent ); 
	//留下百分比percent[如 0.30]小；即留下百分比 percent 黑 
	unsigned char kxGateHigh( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, float percent ); 
	//留下百分比percent[如 0.30]大；即留下百分比 percent 亮 
	
	void kxBinaryImage( const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, unsigned char gate, IppCmpOp oper );
    //-二值化---oper = ippCmpLessEq / ippCmpLess / ippCmpEq / ippCmpGreaterEq / ippCmpGreater
    //------------------------------------------------------------------------------------------------------------------------------------ 
	void kxPrejection_X( const unsigned char* buf, int nWidth, int nHeight, int Pitch, int nSum[] );//X方向投影[注意nSum 调用者 自己清零]
	void kxPrejection_Y( const unsigned char* buf, int nWidth, int nHeight, int Pitch, int nSum[] );//Y方向投影[注意nSum 调用者 自己清零]
	
    void kxThinImage( const unsigned char* buf, int w, int h, int nPitch, unsigned char* pDstBuf, int nDstPitch );//细化=> pDstBuf
    
    int kxFindBlocks( int nSum[], int nLen );   //在nSum[]中的 非0 连续块数
    int kxFindMaxBlocks_L( int nSum[], int nLen, int& nStart, int& nEnd ); //在nSum[]中找一个最大【长度】的连续块, 返回点数 
	int kxFindMaxBlocks_D( int nSum[], int nLen, int& nStart, int& nEnd ); //在nSum[]中找一个最大【点数最多】的连续块, 返回点数 

    void FindImagePosGt( const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, kxPoint<int>& pt, unsigned char gate1, unsigned char gate2  );
	//在 图像 pBuf : nWidth X nHeight X nPitch 上 找到满足条件的区域, 即图像灰度满足   gate1<=   <= gate2
	//入口:( pt.x, pt.y ) 要查找的大小; 出口:( pt.x, pt.y )找到的左上角坐标
    //--------------------------------------------------------------------------------------------------------- 
	void kxCopyImgData( unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch );
	void kxCopyImgRGBData( unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, const unsigned char* pSrc[], int nSrcWidth, int nSrcHeight, int nSrcPitch );

	//------- 从 pSrc  Copy data to pDst, 两个图像可能不一样大， 1：如果pSrc大， 则copy pSrc中间=>pDst; 2：如果pDst大，则Copy pSrc =>pDst中间
	enum
	{
	    _SearchTop = 1,     //搜上边  从上 往 下
        _SearchLeft = 1,    //搜左边  从左 往 右
		_SearchBottom = -1, //搜下边  从下 往 上
		_SearchRight = -1,  //搜右边  从右 往 左
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
    //--统计 满足条件的点数
    int UDNoiseEdgeDetect( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int Sdirect, int Gate, int op, 
	    int movePitch, int Limit, int Depth, int MaxLimit); //搜上/下 边
    //---在图像 buf : nWidth X nHeight X nPitch 从上往下[从下往上]，连续找到Depth个>= Limit的 水平线，或者某次找到>= MaxLimit的水平线，即认为找到边
	//----Sdirect = _SearchTop / _SearchBottom
	//----op = _CmpGt / _CmpGe / ....
    int LRNoiseEdgeDetect( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int Sdirect, int Gate, int op, 
	    int movePitch, int Limit, int Depth, int MaxLimit); //搜左/右 边
    //---在图像 buf : nWidth X nHeight X nPitch 从左往右[从右往左]，连续找到Depth个>= Limit的 垂直线，或者某次找到>= MaxLimit的垂直线，即认为找到边
	//----Sdirect = _SearchLeft / SearchRight
	//----op = _CmpGt / _CmpGe / ....

    //-----------------------------------------------------------------------------------------------------------------------
    void kxImageSub( const unsigned char* buf1, int nWidth, int nHeight, int nPitch1, const unsigned char* buf2, int nPitch2, unsigned char* buf, int nPitch );
    //--图像饱和 减 buf1 - buf2 => buf

    void kxMakeRingMask( unsigned char* buf, int width, int height, int pitch, float x0, float y0, float a, float b, unsigned char nMask = 0xFF );
    //-----在图像 buf: width X height X pitch上 中心（x0,y0）[左上角是坐标原点，向下是Y轴方向]，X轴a,Y轴b的椭圆 
    //-----注意 背景灰度 由调用者 自己管理
    void kxFindPosition_UD( const unsigned char* buf, int width, int height, int pitch, int& top, int& bottom, unsigned char gate = 0 );    
    //根据gate 在 图像buf 上  精确定位 上 下 边界
	void kxFindPosition_LR( const unsigned char* buf, int width, int height, int pitch, int& left, int& right, unsigned char gate = 0 );    
	//根据gate 在 图像buf 上  精确定位 左 右 边界
	enum
	{
		_Angle_0 = 0,      /////////顺时针旋转
		_Angle_90,
		_Angle_180,
		_Angle_270,

	};
	void RotateImg(kxImageBuf& Src,kxImageBuf& Dst,unsigned char* pTemp,int Angle);
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template< class T > void kxSmooth53( T srcData[], T dstData[], int nSize )
	{//-----5点3次平滑
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
	//---计算相关系数
	template < class T >
	float  kxgetGrayRxy( const T* pSrc1, const T*  pSrc2, int nWidth, int nHeight, int Pitch1, int Pitch2 ) 
	{//---计算相关系数
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
	{//计算标准差
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