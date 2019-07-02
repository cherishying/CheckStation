#ifndef _KXCOMMONDEFH
#define _KXCOMMONDEFH


#include "ipp.h"
#include <cstring>
#include <stdio.h>


const int    gTest = 0;               //测试模式
const float  gMinSimilarity = 0.4f;   //最小相似度系数

enum
{
	_Type_G8  = 1,           //黑白图像
	_Type_G24 = 3,           //三通道彩色图像
	_Type_G32 = 4,           //四通道彩色图像

};

//异常类型
enum
{
	kxImageTypeErr                 = -512, //图像类型错误
	kxImageAlignImageSizeErr       = -256, //相似度定位尺寸错误

	//
	//【-234， 0）Ipp错误
	//（0， 52】  Ipp警告

	kxSearchCardImageSizeErr       = 512, //自动搜卡图像大小错误
	kxSearchCardSearchDotNotEnough = 513, //拟合直线的搜点数不足
	kxSearchCardLineErr            = 514, //上下两侧的卡边垂直错误
	kxImageAlignRatioLow           = 515, //相似度太低
	kxSliderCcModeSizeErr          = 1024, //滑动模块模板大小不一样
	kxBlobAnalyseCountErr          = 2048, //连通域分析模块Blob个数太多

	kxSearchSignatureErr           = 4096, //签名条定位失败
};

enum
{
	_VersionNameLen = 64,   //版本名最大长度
};

//投影方向
enum
{
	_Vertical_Project_Dir   = 0,  //垂直投影
	_Horizontal_Project_Dir = 1,  //水平投影

};

//模板处理大小
enum
{
	_1X1 = -1,
	_3X3 = 0,
	_5X5 = 1,
	_7X7 = 2,

};

//定位核的搜索方向
enum
{
	_Horiz_Vertical_Dir = 0,       //双方向搜索
	_Vertical_Dir       = 1,       //定位核搜索垂直方向
	_Horizontal_Dir     = 2,       //定位核搜索水平方向
};

//逻辑判断标记
enum
{
	KxLess     = ippCmpLess,
	KxLessEq   = ippCmpLessEq,
	KxEq       = ippCmpEq,
	kxGreateEq = ippCmpGreaterEq,
	kxGreate   = ippCmpGreater,
};


//颜色层面转化
enum
{
	RGB_R			= 0,
	RGB_G,
	RGB_B,
	RGB_GRAY,
	HSV_H,
	HSV_S,
	HSV_V,
	LAB_L,
	LAB_A,
	LAB_B,
	BGR_GRAY,

};

//插值方式
enum
{
	KxNearest = 0,
	KxLinear = 1,
	KxCubic = 2,
	KxLanczos = 3,
	KxSuper = 4
};

enum
{
	_Call_Ok    = 0,  //调用成功
	_Call_Err   = 1,  //调用失败
};


#pragma pack(push , 1)

//struct MV_IMAGE_FMT
//{
//	int width , height;
//	int type;
//};
//
//struct MV_IMAGE:public MV_IMAGE_FMT
//{
//	int flag;
//	int pitch;
//	void * data;
//};

//about bmp read and write struct
typedef struct
{
	//unsigned short    bfType;
#if defined( _WIN32 ) || defined ( _WIN64 )
	unsigned long    bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned long    bfOffBits;
#else
	unsigned int    bfSize;
	unsigned short    bfReserved1;
	unsigned short    bfReserved2;
	unsigned int    bfOffBits;
#endif


} ClBitMapFileHeader;

typedef struct
{
#if defined( _WIN32 ) || defined ( _WIN64 )
	unsigned long  biSize;
	long   biWidth;
	long   biHeight;
	unsigned short   biPlanes;
	unsigned short   biBitCount;
	unsigned long  biCompression;
	unsigned long  biSizeImage;
	long   biXPelsPerMeter;
	long   biYPelsPerMeter;
	unsigned long   biClrUsed;
	unsigned long   biClrImportant;
#else
	unsigned int  biSize;
	int   biWidth;
	int   biHeight;
	unsigned short   biPlanes;
	unsigned short   biBitCount;
	unsigned int  biCompression;
	unsigned int  biSizeImage;
	int   biXPelsPerMeter;
	int   biYPelsPerMeter;
	unsigned int   biClrUsed;
	unsigned int   biClrImportant;
#endif


} ClBitMapInfoHeader;

typedef struct
{
	unsigned char rgbBlue; //该颜色的蓝色分量
	unsigned char rgbGreen; //该颜色的绿色分量
	unsigned char rgbRed; //该颜色的红色分量
	unsigned char rgbReserved; //保留值
} ClRgbQuad;


//用于判断Ipp调用是否正常！
//函数，类调用状态结构体
struct KxCallStatus
{
	KxCallStatus()
	{
		nCallStatus = _Call_Ok;
		memset(szErrInfo, 0, sizeof(char)*1024);
	}
	void Clear()
	{
		nCallStatus = _Call_Ok;
		memset(szErrInfo, 0, sizeof(char)*1024);
	}

	int    nCallStatus;     //调用状态
	char   szErrInfo[1024]; //错误信息说明

};


inline bool static check_sts(int status, char* szInfo, KxCallStatus& hCall)
{
	if (IppStatus(status) != ippStsNoErr)
	{
		hCall.nCallStatus = IppStatus(status);

#if defined( _WIN32 ) || defined ( _WIN64 )
		sprintf_s(hCall.szErrInfo, 1024, "%s_%d", szInfo, int(status));
#else
		snprintf(hCall.szErrInfo, 1024, "%s_%d", szInfo, int(status));
#endif


		if (IppStatus(status) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


inline bool static check_sts(IppStatus status, char* szInfo, KxCallStatus& hCall)
{
	if (status != ippStsNoErr)
	{
		hCall.nCallStatus = status;

#if defined( _WIN32 ) || defined ( _WIN64 )
		sprintf_s(hCall.szErrInfo, 1024, "%s_%d", szInfo, int(status));
#else
		snprintf(hCall.szErrInfo, 1024, "%s_%d", szInfo, int(status));
#endif

		if (IppStatus(status) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

inline bool static check_sts(KxCallStatus& hSrcCall, char* szInfo, KxCallStatus& hDstCall)
{
	if (hSrcCall.nCallStatus != ippStsNoErr)
	{
		hDstCall.nCallStatus = hSrcCall.nCallStatus;
#if defined( _WIN32 ) || defined ( _WIN64 )
		sprintf_s(hDstCall.szErrInfo, 1024, "%s_%s", szInfo, hSrcCall.szErrInfo);
#else
		snprintf(hDstCall.szErrInfo, 1024, "%s_%s", szInfo, hSrcCall.szErrInfo);
#endif
		if (IppStatus(hSrcCall.nCallStatus) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}




#pragma pack(pop)


#endif
