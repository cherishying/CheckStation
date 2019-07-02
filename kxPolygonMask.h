#ifndef _KXPOLYGONMASKH
#define _KXPOLYGONMASKH

//add by CXH 2017/02/07
//整理、归纳和增加图像(C1/C3)掩膜模块
//包括以下功能：
// 0. 按定位核进行掩膜；
// 1. 按坐标直接掩膜

//参数初始版本名为：PolygonMask1.0

#include "kxDef.h"
#include "gxMath.h"
#include "KxBaseFunction.h"
#include "KxGradientProcess.h"


class CkxPolygonMask
{
public:
	CkxPolygonMask();
	~CkxPolygonMask();

	enum
	{
		_Max_Mask = 8,  //最大掩膜个数
	};

#pragma pack(push, 1)

	struct Parameter
	{
		Parameter()
		{
			memcpy(m_szVersion,  "PolygonMask1.0", _VersionNameLen);

			m_nKernelEx = 40;
			m_nMaskCount = 0;
			memset(m_nEdgeCount, 0, sizeof(int)*_Max_Mask);
			memset(m_rcMask, 0, sizeof(kxRect<int>)*_Max_Mask);
			memset(m_rcMaskKernel, 0, sizeof(kxRect<int>)*_Max_Mask);
			memset(m_nIsMaskKernel, 0, sizeof(int)*_Max_Mask);
			memset(m_nMaskKernelDirect, 0, sizeof(int)*_Max_Mask);

			m_nIsOpenAutoMask = 0;
			m_nCreateMaskLayer = 0;
			m_nMaskPreProcess = 0;
			m_nOpenGradient = 0;
			m_nThresh = 0;
			m_nComplement = 0;
		}

		char              m_szVersion[_VersionNameLen];  //记录参数的版本信息
		int               m_nKernelEx;                   //定位核搜索扩张距离
		int               m_nMaskCount;                  //掩模个数
		int				  m_nEdgeCount[_Max_Mask];		//多边形边数
		kxPoint<int>	  m_rcMask[_Max_Mask][16];		//掩模区域数据，_MaskCount = 8
		int               m_nIsMaskKernel[_Max_Mask];     //掩模是否需要定位。
		kxRect<int>		  m_rcMaskKernel[_Max_Mask]; 	  //掩模定位核位置, 底板坐标
		int               m_nMaskKernelDirect[_Max_Mask]; //0默认 1是y方向 2是x方向

		int               m_nIsOpenAutoMask;              //是否开启自动掩膜功能
		int               m_nCreateMaskLayer;             //掩膜层面
		int               m_nMaskPreProcess;              //处理方法
		int               m_nOpenGradient;                //是否采用梯度方法
		int               m_nThresh;                      //二值化系数
		int               m_nComplement;                  //取反

	};
#pragma pack(pop)


protected:
	Parameter                m_hParameter;

	kxCImageBuf              m_BaseImg;                //建模时的底板图像，用于扣定位核和做自动掩膜处理图


	kxCImageBuf		         m_TmpBuf;       //临时空间
	kxCImageBuf		         m_imgSearch;    //搜索区域图像，动态分配
	kxCImageBuf              m_Img;
	kxCImageBuf              m_Img1;

	kxCImageBuf              m_AutoMaskImg;  //自动掩膜图像
	CKxBaseFunction          m_hBaseFun;
	CKxGradientProcess       m_hGradient;
	kxCImageBuf              m_ImgPre0;
	kxCImageBuf              m_ImgPre1;
	kxCImageBuf              m_ImgPre2;
	kxCImageBuf              m_ImgPre3;
	kxCImageBuf              m_ImgTemp;
	kxCImageBuf              m_PolygonMaskImg[_Max_Mask];  //多边形掩膜图像
	kxRect<int>              m_rcPolyMask[_Max_Mask];
	kxCImageBuf              m_ResizeMaskImg;


private:
	int GetImgOffset(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, int nImgType, int nIndex, int& nDx, int& nDy);
	void VerticesToImage(kxPoint<int> *Vertex, int nVertices, unsigned char *pBuf, int nWidth, int nHeight, int nPitch);

protected:
	//参数版本1的读写
	bool ReadVesion1Para(FILE* fp);
	bool WriteVesion1Para(FILE* fp);

	//外部调用函数
public:
	kxCImageBuf		         m_ImgMaskKern[_Max_Mask]; //掩膜定位核图像
													   //参数的文件读写操作
	bool Read(FILE*);
	bool Write(FILE*);
	bool ReadParaFromNet(unsigned char*& point);

	//生成自动掩膜图像
	int AutoCreateMask(kxCImageBuf& Srcimg);
	//用于外部调用显示生成的Mask
	int ShowMaskImage(kxCImageBuf& BaseImg, kxRect<int>& rcPos);

	unsigned char* GetAutoMaskImage(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		return m_AutoMaskImg.GetImageBuf(nWidth, nHeight, nPitch, nChannel);
	}


	Parameter& GetParameter()
	{
		return m_hParameter;
	}



	//输入：待掩膜图像和一副用于定位的图像（一般是精确校正后的图像）、填充值
	//SrcImg 待Mask图像，一般是残差图，为灰度图
	//KernImg 为定位图像，一般为校正后的彩色图
	//rcCheckArea 偏移区域
	int Mask(kxCImageBuf& SrcImg, kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal, KxCallStatus& hCall);
	int Mask(kxCImageBuf& SrcImg, kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal);
};

#endif
