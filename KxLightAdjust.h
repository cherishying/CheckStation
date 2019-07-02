#ifndef _KXLIGHTADJUSTHHHH
#define _KXLIGHTADJUSTHHHH
//光强修正
//add by lyl

//参数初始版本名为：LightAdjust1.0
#include "kxDef.h"
#include "gxMath.h"
#include "ippi.h"
#include "KxBaseFunction.h"


class CKxLightAdjust
{
public:
	CKxLightAdjust();
	~CKxLightAdjust();



   #pragma pack(push, 1)

   struct Parameter
   {
	   Parameter()
	   {
		   memcpy(m_szVersion,  "LightAdjust1.0", _VersionNameLen);
		   m_nSkipBlackPercent = 5;
		   m_nBlackPercent = 30;
		   m_nSkipWhitePercent = 5;
		   m_nWhitePercent = 30;

	   }
	   char   m_szVersion[_VersionNameLen];  //记录参数的版本信息
	   int    m_nSkipBlackPercent;
	   int    m_nBlackPercent;
	   int    m_nSkipWhitePercent;
	   int    m_nWhitePercent;

   };

   #pragma pack(pop)

   struct Result
   {
	   Result()
	   {
		   m_nModelBlackAvg = 0;
		   m_nModelWhiteAvg = 255;

		   memset(m_nModelBlackAvgC3, 0, sizeof(int)*3);
		   for (int i = 0; i < 3; i++)
		   {
			   m_nModelWhiteAvgC3[i] = 255;
		   }


		   memset(m_nHistogramArray, 0, sizeof(int)*256);
		   m_nGrayAverage_BlackPart = 0;
		   m_nGrayAverage_WhitePart = 255;
		   memset(m_nLutArray, 0, sizeof(int)*256);

		   memset(m_nGrayAverage_BlackPartC3, 0, sizeof(int)*3);
		   for (int i = 0; i < 3; i++)
		   {
			   m_nGrayAverage_WhitePartC3[i] = 255;
		   }
		   memset(m_nLutArrayC3, 0, sizeof(int)*256*3);


	   }
	   float  m_nModelBlackAvg;
	   float  m_nModelWhiteAvg;

	   float  m_nModelBlackAvgC3[3];
	   float  m_nModelWhiteAvgC3[3];


	   int m_nHistogramArray[256];//测试图的灰阶直方图
	   float m_nGrayAverage_BlackPart;
	   float m_nGrayAverage_WhitePart;
	   Ipp32s m_nLutArray[256];

	   float m_nGrayAverage_BlackPartC3[3];
	   float m_nGrayAverage_WhitePartC3[3];
	   int m_nLutArrayC3[3][256];


   };
   kxImageBuf    m_ModelImg;
   kxImageBuf    m_LightAdjustImg;

   Parameter     m_hParamter;
   Result        m_hResult;
   CKxBaseFunction m_hBaseFun;



protected:
	//参数版本1的读写
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;

protected:

	int LightAdjustC1(const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall);
	int GetOneImageHistogramCharacter(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int* pnHist, float* pnBlackAvg, float* pnWhiteAvg, KxCallStatus& hCall);
	int GetOneImageHistogramCharacterC3(const unsigned char* buf, int nWidth, int nHeight, int nPitch, float pnBlackAvg[3], float pnWhiteAvg[3], KxCallStatus& hCall);
    int LightAdjustC3(const unsigned char* buf, int nWidth, int nHeight, int nPitch, unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall );

	unsigned char* GetLightAdjustImage(int& nWidth, int nHeight, int nPitch, int nChannel)
	{
		return m_LightAdjustImg.GetImageBuf(nWidth, nHeight, nPitch, nChannel);
	}

	//设置校正的参考图像
	bool SetModelImage(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel)
	{
		m_ModelImg.SetImageBuf(buf, nWidth, nHeight, nPitch, nChannel, false);
		return 1;
	}
	bool SetModelImage(kxCImageBuf& Img)
	{
		m_ModelImg.SetImageBuf(Img.buf, Img.nWidth, Img.nHeight, Img.nPitch, Img.nChannel, false);
		return 1;
	}

public:
	//参数的文件读写操作
	bool Read( FILE* );
	bool Write( FILE* );

	//光影校正函数
	//输入：SrcImg 待校正的图像
	//      ReferImg 参考图像
	//输出：目标图像
	int LightAdjust(kxCImageBuf& SrcImg, kxCImageBuf& ReferImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
    int LightAdjust(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
		             unsigned char* pRefer, int nReferPitch, unsigned char* pDst,  int nDstPitch, KxCallStatus& hCall);


};

#endif
