#ifndef _KXCOMMONDEFH
#define _KXCOMMONDEFH


#include "ipp.h"
#include <cstring>
#include <stdio.h>


const int    gTest = 0;               //����ģʽ
const float  gMinSimilarity = 0.4f;   //��С���ƶ�ϵ��

enum
{
	_Type_G8  = 1,           //�ڰ�ͼ��
	_Type_G24 = 3,           //��ͨ����ɫͼ��
	_Type_G32 = 4,           //��ͨ����ɫͼ��

};

//�쳣����
enum
{
	kxImageTypeErr                 = -512, //ͼ�����ʹ���
	kxImageAlignImageSizeErr       = -256, //���ƶȶ�λ�ߴ����

	//
	//��-234�� 0��Ipp����
	//��0�� 52��  Ipp����

	kxSearchCardImageSizeErr       = 512, //�Զ��ѿ�ͼ���С����
	kxSearchCardSearchDotNotEnough = 513, //���ֱ�ߵ��ѵ�������
	kxSearchCardLineErr            = 514, //��������Ŀ��ߴ�ֱ����
	kxImageAlignRatioLow           = 515, //���ƶ�̫��
	kxSliderCcModeSizeErr          = 1024, //����ģ��ģ���С��һ��
	kxBlobAnalyseCountErr          = 2048, //��ͨ�����ģ��Blob����̫��

	kxSearchSignatureErr           = 4096, //ǩ������λʧ��
};

enum
{
	_VersionNameLen = 64,   //�汾����󳤶�
};

//ͶӰ����
enum
{
	_Vertical_Project_Dir   = 0,  //��ֱͶӰ
	_Horizontal_Project_Dir = 1,  //ˮƽͶӰ

};

//ģ�崦���С
enum
{
	_1X1 = -1,
	_3X3 = 0,
	_5X5 = 1,
	_7X7 = 2,

};

//��λ�˵���������
enum
{
	_Horiz_Vertical_Dir = 0,       //˫��������
	_Vertical_Dir       = 1,       //��λ��������ֱ����
	_Horizontal_Dir     = 2,       //��λ������ˮƽ����
};

//�߼��жϱ��
enum
{
	KxLess     = ippCmpLess,
	KxLessEq   = ippCmpLessEq,
	KxEq       = ippCmpEq,
	kxGreateEq = ippCmpGreaterEq,
	kxGreate   = ippCmpGreater,
};


//��ɫ����ת��
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

//��ֵ��ʽ
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
	_Call_Ok    = 0,  //���óɹ�
	_Call_Err   = 1,  //����ʧ��
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
	unsigned char rgbBlue; //����ɫ����ɫ����
	unsigned char rgbGreen; //����ɫ����ɫ����
	unsigned char rgbRed; //����ɫ�ĺ�ɫ����
	unsigned char rgbReserved; //����ֵ
} ClRgbQuad;


//�����ж�Ipp�����Ƿ�������
//�����������״̬�ṹ��
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

	int    nCallStatus;     //����״̬
	char   szErrInfo[1024]; //������Ϣ˵��

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
