#ifndef _KXSIGNATURECHECKBETAH
#define _KXSIGNATURECHECKBETAH

//add by lyl 2017/11/16
//����ӡ������Ҫ��ǩ�������ģ�������汾


#include "kxDef.h"
#include "gxMath.h"
#include "KxCommonDef.h"            //���õı�������
#include "KxBaseFunction.h"         //�Ƚϳ��õĺ���
#include "KxCalculateDistance.h"    //����궨���
#include "KxLightAdjust.h"          //��ӰУ���㷨
#include "KxWarpStretch.h"          //ͼ��У���㷨
#include "KxColorDiff.h"            //ɫ����
#include "KxGradientProcess.h"      //�ݶ��㷨
#include "KxLearn.h"                //ѧϰ�㷨
#include "KxSliderCc.h"             //�����в��㷨
#include "KxImageMask.h"            //ͼ����Ĥ�����㷨
#include "KxBlobAnalyse.h"          //�Ľ������ͨ������㷨
#include "kxPolygonMask.h"

#include "tbb/tbb.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range2d.h"
#include "tbb/scalable_allocator.h"
#include "tbb/partitioner.h"
using namespace tbb;

#define SUBSECTION 3

class CKxSignatureCheckBeta
{
public:
	CKxSignatureCheckBeta();
	~CKxSignatureCheckBeta();

	enum
	{
		_Valid_Layer = 2,
		_Max_Layer = 8,


		_Check_Layer = 3,
		_Max_BlobCount = 8,

		_Edge_Extend = 10,
		_MAX_OFFSET = 10,
	};

	enum
	{
		_Max_Point = 8				//���λ����
	};

	//��������
	enum
	{
		_Check_Ok = 0,
		_Check_Err = 1,
		_ColorDiff_Err = 2,
		_Distance_Err = 3,
		_Barcode2D_Err = 4,
		_Barcode_Err = 5,
		_Number_Err = 6,
		_Similarity_Err = 7,
		_SearchEdge_Err = 8,
		_BlockImage_Err = 9,
		_Identify_Check = 10,
	};


#pragma pack(push, 1)
	//һЩ���Ʋ���
	struct  Parameter
	{
		Parameter()
		{
			for (int i = 0; i < _Max_Point; i++)
				m_Point[i].setup(0, 0);
			m_nPointCount = 0;
			m_nSearchEdgeThresh = 30;
			m_nWhiteAreaCheckThresh = 60;
		}
		kxPoint<int>	  m_Point[_Max_Point];	  //��λ������
		int			      m_nPointCount;	      //��λ�����
		int               m_nAreaNumber;          //�����
		int               m_nIsLearn;             //�Ƿ�ѧϰ
		int               m_nSmallDotOffset;      //��С��ƫ��
		int               m_nLossOffset;          //��ȱʧƫ��
		int               m_nIsOpenWhiteArea;     //�Ƿ�򿪰������
		kxRect<int>       m_rcWhiteArea;          //�����������
		int               m_nWhiteAreaCheckThresh; //���������ֵ
		int               m_nSearchEdgeThresh;    //�ѱ���ֵ
	};

	struct Result
	{
		Result()
		{
			memset(m_hBlobInfo, 0, sizeof(BlobInfo)*_Max_BlobCount*4);
			m_nCount = 0;
			m_nStatus = _Check_Ok;
			m_nKernIndex = -1;
			m_nDistanceX = 0;
			m_nDistanceY = 0;
			m_nColorDiff = 0;
		}

		struct BlobInfo
		{
			int                 m_nAreaType;   //�����
			int                 m_nDots;
			int                 m_nEnergy;
			int                 m_nLeft;
			int                 m_nTop;
			int                 m_nBlobWidth;
			int                 m_nBlobHeight;
			int                 m_nWHRatio;
			int                 m_nAvgEnergy;
			int                 m_nAvgArea;
			int                 m_nMinRectWidth;
			int                 m_nMinRectHeight;
			kxCImageBuf         m_ErrImg;
		};

		BlobInfo      m_hBlobInfo[_Max_BlobCount * 4];   //ȱ������
		int           m_nCount;                      //ʵ��ȱ�ݸ���
		int           m_nStatus;                     //���״̬��Ϣ
		int           m_nKernIndex;                  //���ƶȴ���λ�˱��
		int           m_nDistanceX;                   //������󷵻�ʵ�ʾ���
		int           m_nDistanceY;
		int           m_nColorDiff;                  //ɫ��

		void Clear()
		{
			m_nCount = 0;
			m_nStatus = _Check_Ok;
			m_nKernIndex = -1;
			m_nDistanceX = 0;
			m_nDistanceY = 0;
			m_nColorDiff = 0;
			for (int k = 0; k < _Max_BlobCount * 4; k++)
			{
				m_hBlobInfo[k].m_ErrImg.Release();
			}
		}
	};

#pragma pack(pop)

public:
	kxCImageBuf			m_ImgTemplate[_Max_Layer + 1];	//ģ��
	kxCImageBuf         m_ImgLearnTemplate[_Max_Layer + 1];  //�ֲ�ѧϰģ��+һ���ϳ�ģ��

protected:

	CKxBaseFunction		m_hBaseFun;
	CKxBaseFunction     m_hFun[_Max_Layer];
	CKxCalDistance		m_hCalDistance;					//������
	CKxWarpStretch		m_hWarpStrech;
	CKxColorDiff        m_hColorDiff;          //ɫ����
	CKxGradientProcess	m_hGradient;
	CKxLearn			m_hLearn[_Max_Layer];			//�ĸ�ѧϰ�����Ӧ�ĸ�����ͼ
	CKxSliderCc			m_hSlider[_Max_Layer];
	CKxImageMask		m_hImgMask;						//ͼ����Ĥ
	CkxPolygonMask		m_hPolygonMask;					//�������Ĥ
	CKxBlobAnalyse		m_hBlobAnaly;					//Blob����

	Parameter			m_hParameter;
	kxCImageBuf			m_BaseImg;						//��ģʱ�õĵװ壬���ڹ�ӰУ�����Զ���Ĥ
	int					m_nModelNumber;					//ģ������
	Result				m_hResult;						//���

	kxPoint<int>		m_Offset[_Max_Point];			//���ƫ��ֵ
	int					m_nOffsetCount;					//���ƫ�Ƹ���
	kxPoint<int>		m_OffsetMin;					//��Сƫ��
	kxPoint<int>		m_OffsetMax;					//���ƫ��

	kxRect<int>			m_rcMattingPos;					//ǩ�����ѱ�ƫ��
	kxRect<int>         m_rcSignaturePos;               //ǩ�����ڿ����λ�ã����ڶ�λ
	kxRect<int>         m_rcBaseSignaturePos;          //�װ�ǩ������λ��

protected:
	//�����������ʱͼ��ռ�
	kxCImageBuf			m_OriginalImg;					//ԭʼͼ��
	kxCImageBuf			m_WarpImg;						//��ȷУ�����ͼ��

	kxCImageBuf			m_ImgCheck[_Max_Layer];			//����ͼ��,�ݺ�/�ݰ׶�Ӧ90,0,45 135����ͼ��
	kxCImageBuf         m_ImgLearn[_Max_Layer];
	kxCImageBuf			m_ImgSlidCc[_Max_Layer];
	kxCImageBuf			m_ImgOffset[_Max_Layer];
	kxCImageBuf         m_ImgSlidCcMax[_Max_Layer];
	kxCImageBuf         m_ImgCompose[_Max_Layer];
	kxCImageBuf         m_ImgOpen[_Max_Layer];
	kxCImageBuf         m_ImgFilterEdge[_Max_Layer];


	kxCImageBuf			m_ImgTemp;						//
	kxCImageBuf			m_ImgCc;						//�ϲ��в�ͼ(�Ҷ�ͼ)

	//kxCImageBuf			m_StdImg;					//ɫ�����׼
	//kxCImageBuf			m_TestImg;						//ɫ���鱻����ͼ
	//kxCImageBuf			m_ColorCheckImg;
	kxCImageBuf			m_CreateMask;

	kxCImageBuf         m_EdgeImg;
	kxCImageBuf			m_ImageTemp;					//�����м�ͼ��
	kxCImageBuf         m_ImgGradient;                  //�ѱ��ݶ�ͼ��

	kxCImageBuf         m_ImgGradient0;
	kxCImageBuf         m_ImgGradient1;
	kxCImageBuf         m_ImgSignature;                 //ǩ��������
	kxCImageBuf         m_ImgCreateTemp;
	kxCImageBuf         m_ImgCreateLocation;
	kxCImageBuf         m_ImgSingleLocation[SUBSECTION];
	kxCImageBuf         m_ImgCreateGradient[_Max_Layer];
	kxCImageBuf         m_ImgCreateGradientTmp;
	kxCImageBuf         m_ImgCreateBase[_Max_Layer];
	kxCImageBuf         m_ImgSingleBase[_Max_Layer];

	kxCImageBuf         m_ImgLocationSmall[SUBSECTION];
	kxCImageBuf         m_ImgLocationBig[SUBSECTION];
	kxCImageBuf         m_ImgSplitSearch[SUBSECTION];
	//�����ɲ�ɫ�в�ͼ��ʾ�йر���
	kxCImageBuf         m_ColorImg;             //��ɫͼ��
	kxCImageBuf         m_ResizeImg;            //ͼ��
	kxCImageBuf         m_ResidImg[3];          //��ɫ\��ɫ\��ɫ�в�ͼ
	kxCImageBuf         m_CompositionImg[3];    //��ɫ\��ɫ\��ɫ�ϳɲв�ͼ
	kxCImageBuf         m_AddImage;
	kxCImageBuf         m_BinaryImg;
	kxCImageBuf         m_DupImg;
	kxCImageBuf         m_ShowImg;
	kxCImageBuf         m_ResizeImg1;

	kxCImageBuf			m_ImageEdge;					//��Եͼ��
	kxCImageBuf         m_ImgEdgeTemp;
	kxCImageBuf			m_ImageCc;						//�в�ͼ��
	kxCImageBuf			m_ImageLocation[SUBSECTION];	//�ѱ��м�ͼ��

	kxCImageBuf			m_ImageLocationTmp;
	kxCImageBuf         m_ImagePosSignature;

	kxCImageBuf         m_ImgAverage;
	kxCImageBuf         m_ImgGradientPos[2];
	kxCImageBuf         m_ImgGradientMaxPos[2];
	kxCImageBuf         m_ImgThreshPos[2];
	kxCImageBuf         m_ImgFilterSpeckle[2];
	kxCImageBuf         m_ImgClosePos[2];
	kxCImageBuf         m_ImgWhiteArea;
	kxCImageBuf         m_ImgPos;

	int                 m_nLearnHighLowMode[_Max_Layer];
	kxRect<int>			m_rcLocationPos[SUBSECTION];	//ǩ������λƫ��
	kxRect<int>         m_rcSplitSmall[SUBSECTION];

	int                 m_nLearnPage;                     //ѧϰ����
	bool                m_bIsLearnCompleted[_Max_Layer];  //ѧϰ�Ƿ�����
	bool                m_bAllCompleted;

	int                 m_nAlgorithmStatus[_Max_Layer];               //8���������״̬
	int                 m_nLearnStatus;

	int                m_nSignatureEdgeThresh;
	int                m_nSearchEdgeThresh;
	int                m_nSmoothSize;                //�����ݶ�ƽ���ߴ�

	int                m_nSignatureModelWidth;            //ǩ�����װ��
	int                m_nSignatureModelHeight;


	int                m_nSignatureWidth;            //ǩ������
	int                m_nSignatureHeight;
	int                m_nSignatureSizeThresh;

	int                m_nSignatureExtendLen;   //ǩ�����ѱ���������

	int                m_nSignatureBorderCheckThresh;

	int                m_nIsCreateBaseImg;      //�Ƿ񴴽��װ�
	int                m_nUseXml;
	kxRect<int>        m_rcWhiteArea;
	kxRect<int>        m_rcSignatureArea;

	int                m_nOpenVerticalSize;

protected:
	int ReadImg(FILE* fp);
	int WriteImg(FILE* fp);

	int Matting(const kxCImageBuf& SrcImg, bool bModel, KxCallStatus& hCall);
	int Matting(const kxCImageBuf& SrcImg, bool bModel = false);
	int LearnTemplate(KxCallStatus& hCall);
	int ResidualsAnalys(KxCallStatus& hCall);
	int ConnectAnalysis(const kxCImageBuf& SrcImg, KxCallStatus& hCall);
	int SaveMidImage(bool bSave);
	int AnalysisCheckStatus();
	int SetCheckExceptionStatus(const kxCImageBuf& ImgCheck);
	int SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate);
	int AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg);
public:



	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadParaXml(const char* filePath);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadParaXmlinEnglish(const char* filePath);

	//����ģ��ͼ��
	int SaveTemplateImg(const char* lpszFile);
	//��ȡģ��ͼ��
	int LoadTemplateImg(const char* lpszFile);

	//�ֲ�ѧϰ
	int SaveLearnTemplateImg(const char* lpszFile);
	int WriteLocalLearnImg(FILE* fp);
	int LoadLearnTemplateImg(const char* lpszFile);
	int ReadLocalLearnImg(FILE* fp);


	//��ȡ�����
	const Result&  GetCheckResult() const
	{
		return m_hResult;
	}
	//����ѧϰ����
	void SetLearnPage(int nLearn)
	{
		m_nLearnPage = nLearn;
	}

	void SetEdgeCheckThresh(int nThresh)
	{
		m_nSignatureEdgeThresh = nThresh;
	}

	void SetSignatureBorderCheckThresh(int nThresh)
	{
		m_nSignatureBorderCheckThresh = nThresh;
	}

	void SetSmoothSize(int nSize)
	{
		m_nSmoothSize = nSize;
	}

	void SetSearchEdgeThresh(int nThresh)
	{
		m_nSearchEdgeThresh = nThresh;
	}

	void SetSignatureSizeThresh(int nThresh)
	{
		m_nSignatureSizeThresh = nThresh;
	}

	void SetSignatureExtendLen(int nLen)
	{
		m_nSignatureExtendLen = nLen;
	}

	//���òв����
	void SetCcVerticalOpenSize(int nOpensize = 1)
	{
		m_nOpenVerticalSize = nOpensize;
	}
	//���ѧϰ��״̬
	bool IsLearn()
	{
		return m_hParameter.m_nIsLearn > 0 ? true : false;
	}

	//ͨ�ü��
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);
	int SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall);
	int SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex);
	int ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6], KxCallStatus& hCall);
	int ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6]);
	int SingleImageLearn(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int SingleImageLearn(const kxCImageBuf& ImgCheck);

	//��ȡ����
	Parameter& GetParameter()
	{
		return m_hParameter;
	}
	//�Ƿ��ѧϰ�꣬����ģ������ã��ж�ѧϰ��ɣ���һϵ�еĲ���
	bool IsLearnCompletedStatus() const
	{
		return m_bAllCompleted;
	}
	void SetLearnCompletedStatus(bool bStatus)
	{
		m_bAllCompleted = bStatus;
	}

	//��ȡͼ����Ĥ���������Զ�maskͼ����ʾ
	void  GetImageMaskImage(kxCImageBuf& AutoImg)
	{
		m_hImgMask.ShowMaskImage(m_BaseImg, m_hWarpStrech.GetParameter().m_rcCheckArea);
		AutoImg.buf = m_hImgMask.GetAutoMaskImage(AutoImg.nWidth, AutoImg.nHeight, AutoImg.nPitch, AutoImg.nChannel);
	}

	//��ȡͼ��У�����ͼ��
	unsigned char* GetWarpImage(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		nWidth = m_WarpImg.nWidth;
		nHeight = m_WarpImg.nHeight;
		nPitch = m_WarpImg.nPitch;
		nChannel = m_WarpImg.nChannel;
		return m_WarpImg.buf;
	}
	kxCImageBuf& GetWarpImage() { return m_WarpImg; }

	//��ȡ�в�ͼ��
	unsigned char* GetImageCc(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		nWidth = m_ImgCc.nWidth;
		nHeight = m_ImgCc.nHeight;
		nPitch = m_ImgCc.nPitch;
		nChannel = m_ImgCc.nChannel;
		return m_ImgCc.buf;
	}
	kxCImageBuf& GetImageCc() { return m_ImgCc; }

	//��ȡ�Ƿ��ǩ�������ģ��
	//1 �����  0����ر�
	//int IsOpenSignatureCheck() { return m_hParameter.m_nIsCheckSignature; }
};

#endif
