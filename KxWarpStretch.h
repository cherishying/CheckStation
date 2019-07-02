#ifndef _KXWARPSTRETCHHHHH
#define _KXWARPSTRETCHHHHH

//add by lyl 2016/6/6
//整理归纳和补充图像校正模块
//包括以下功能：
// 0. 0个定位核的拷贝功能；
// 1. 1个定位核的平移变换校正；
// 2. 2个定位核的平移旋转变换校正；
// 3. 3个定位核的仿射变换校正；
// 4. 4个到8个定位核的透视投影变换校正；
// 5. 4个到8个定位核的双线性变换校正；

////参数初始版本名为：WarpStretch1.0

#include "KxBaseFunction.h"
#include "kxDef.h"
#include "gxMath.h"
//#include "ippm.h"
#include "ippi.h"
#include "mkl.h"


class CKxWarpStretch
{
public:
	CKxWarpStretch();
	~CKxWarpStretch();

	enum
	{
		_Max_Kern = 8,           //最大定位核个数
		_Perspective_Trans = 0,  //透视变换
		_Bilinear_Trans = 1,     //双线性变换
	};

    #pragma pack(push, 1)

	struct Parameter
	{
		Parameter()
		{
			memcpy(m_szVersion,  "WarpStretch1.0", _VersionNameLen);
			m_rcCheckArea.setup(0,0,0,0);
			m_nKernCount = 0;
			memset((void*)m_rcKern, 0, sizeof(kxRect<int>)*_Max_Kern);
			m_nSearchExpand = 30;
			memset(m_nKernelDirect, _Horiz_Vertical_Dir, sizeof(int)*_Max_Kern);
			m_nAlgorithmType = _Perspective_Trans;

		}
		char         m_szVersion[_VersionNameLen];    //记录参数的版本信息
		kxRect<int>  m_rcCheckArea;                   //检测区域
		int          m_nKernCount;                    //定位核个数
		kxRect<int>  m_rcKern[_Max_Kern];             //定位核区域
		int          m_nSearchExpand;                 //定位核搜索扩充范围
		int          m_nKernelDirect[_Max_Kern];      //定位核搜索方向
		int          m_nAlgorithmType;                //算法类型，用来区别透视变换还是双线性变换,只有定位核个数超过3有效

	};

	#pragma pack(pop)

protected:
	Parameter        m_hParameter;         //检测参数
	kxCImageBuf      m_KernBuf[_Max_Kern]; //定位图像
	kxCImageBuf      m_WarpImg;            //拉伸后的图像

	CKxBaseFunction  m_hBaseFun;
	CKxBaseFunction  m_hFun[_Max_Kern]; //用于并行


	kxCImageBuf      m_SrcImg;
	kxCImageBuf      m_DstImg;

	int              m_nKernX;             //第一个定位核的实际位置，作为结果数据返回出去，作为距离标定参照
	int              m_nKernY;

	double           m_matchQuad[_Max_Kern][2]; //匹配到的点，坐标已变换到待检测图上
	double           m_kernQuad[_Max_Kern][2];  //模板图像上定位核的LeftTop点
	double           m_coeffsFinePerspective[3][3];  //透视变换系数
	double           m_coeffsFineBilinear[2][4];     //双线性变换系数
	double           m_coeffsFineAffine[2][3];       //仿射变换系数
	kxPoint<int>     m_ptOffset;                     //偏移变换


protected:
	//参数版本1的读写
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;

	//内部调用函数
protected:
	//一个定位核的平移变换校正
	int GetImgOffset(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, int nImgType, int& nDx, int& nDy,  KxCallStatus& hCall);

	//二个定位核的旋转偏移变换校正
	int RotateOffsetTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall);

	//旋转偏移变换计算系数
	int GetRotateOffTransform( const double srcQuad[2][2], const double dstQuad[2][2], double coeffs[2][3], KxCallStatus& hCall);

	//三个定位核的仿射变换校正
	int AffineTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall);
    int GetAffineTransform( const double srcQuad[3][2], const double dstQuad[3][2], double coeffs[2][3], KxCallStatus& hCall);

	//四个到八个定位核的透视投影变换校正
	int  PerspectiveTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall );
    bool GetPerspectiveTransform_8(const double srcQuad[8][2], const double dstQuad[8][2], double coeffs[3][3], KxCallStatus& hCall);
    bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3], KxCallStatus& hCall );

	//四个到八个定位核的双线性插值变换校正
	int BilinearTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall );
	int GetBilinearTransform( const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4], KxCallStatus& hCall  );
	int GetBilinearTransform_8( const double srcQuad[8][2], const double dstQuad[8][2], double coeffs[2][4], KxCallStatus& hCall ) ;


	//并行算法
	int RotateOffsetTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall , int nRowGrain = 120, int nColGrain = 120);
	int AffineTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int PerspectiveTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	//int BilinearTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, int nRowGrain = 120, int nColGrain = 120, KxCallStatus& hCall = KxCallStatus());



	//外部调用函数
public:
	//参数的文件读写操作
	bool Read( FILE* );
	bool Write( FILE* );
	//从网络中获取参数
	bool ReadParaFromNet( unsigned char*& point );

	//通过Xml读参数
	//filePath      为xml路径名
	//szModuleName  为模块名,根据界面设置，譬如本模块 szModuleName = “定位核设置”
	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);

	//最新增加的读参数接口，ROI读方式有区别
	bool ReadXmlStandard(const char* filePath, const kxCImageBuf& BaseImg);

	//将校正图上的残差分析结果校正
	int GetRightPos(kxRect<int> & rcSrc, kxRect<int>& rcDst);

	//输入：一张待校正的图像
	//输出：校正图像存放在 m_WarpImg
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);


	//输入：一张待校正的图像
	//输出：一张校正后的图像
	//返回值：0-8 1-8表示拉伸出错，定位核的标号，0表示正常拉伸。
	//         -1 表示函数调用异常，一般指Ipp异常
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
		       unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall);

	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);
	//获取拉伸后的图像
	const kxCImageBuf& GetWarpImg() const { return m_WarpImg; }
	const unsigned char * GetWarpBuf( int& nWidth, int& nHeight, int& nPitch, int& nChannel )
	{
		return m_WarpImg.GetImageBuf( nWidth, nHeight, nPitch, nChannel );
	}

	//获取参数
	Parameter& GetParameter()  {return  m_hParameter;}

	//获取第一个定位核的实际位置偏移,用于距离标定
	bool GetKernPosition(int& nX, int& nY) const
	{
		if (m_hParameter.m_nKernCount > 0)
		{
			nX = m_nKernX;
			nY = m_nKernY;
			return true;
		}
		else
		{
			nX = 0;
			nY = 0;
			return false;
		}
	}


	//并行算法
	//输入：一张待校正的图像
	//输出：校正图像存放在 m_WarpImg
	int CheckParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);

	//输入：一张待校正的图像
	//输出：一张校正后的图像
	//返回值：0-8 1-8表示拉伸出错，定位核的标号，0表示正常拉伸。
	//         -1 表示函数调用异常，一般指Ipp异常
	int CheckParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
		unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall , int nRowGrain = 120, int nColGrain = 120);

	int CheckParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int CheckParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);
};

#endif
