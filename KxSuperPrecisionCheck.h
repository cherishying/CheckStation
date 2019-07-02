//add by lyl 2017/5/20
//This class is a super precision's surface inspect class.


#ifndef _KXSUPERPRECISIONCHECKH
#define _KXSUPERPRECISIONCHECKH

//#include "KxAlogrithm.h"

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


class CKxSuperPrecisionCheck
{
public:
	CKxSuperPrecisionCheck();
	~CKxSuperPrecisionCheck();

public:
	enum
	{
		_Valid_Layer = 2,
		_Max_Layer = 10,


		_SAMLLDOTS_INDEX_END = 2,
		_LINE_INDEX_END = 6,
		_SCRATCH_INDEX_END = 7,
		_GRAY_INDEX_END = 9, //�Ҷȼ�
		_LOSS_INDEX_END = 10,

		_CheckModeNum = 2,

		_Max_BlobCount = 4,

		_MAX_OFFSET = 10,
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
			memcpy(m_szVersion,  "CheckControl1.0", _VersionNameLen);
			m_nIsOpenSurfaceCheck = 1;
			m_nAreaNumber = 0;
			m_nCheckMethod = 0;
			m_nModelProcessSize = _3X3;
			m_nIsLearn = 0;
			m_nOffset = 5;
			m_nSmallDotOffset = 2;
			m_nLineOffset = 2;
			m_nLossOffset = 5;
			m_nScratchOffset = 2;
			m_nGrayOffset = 5;
			m_nGrayLowOffset = 10;
			m_rcEdgeRect.setup(0, 0, 1399, 899);
		}
		char              m_szVersion[_VersionNameLen];  //��¼�����İ汾��Ϣ
		int               m_nIsOpenSurfaceCheck;  //�Ƿ���������
		int               m_nAreaNumber;          //�����
		int               m_nCheckMethod;         //��ⷽ��
		int               m_nModelProcessSize;    //ģ�崦���ߴ�
		int               m_nIsLearn;             //�Ƿ�ѧϰ
		int               m_nOffset;
		int               m_nSmallDotOffset;      //��С��ƫ��
		int               m_nLineOffset;          //��ϸ��ƫ��
		int               m_nScratchOffset;       //�컮��ƫ��
		int               m_nLossOffset;          //��ȱʧƫ��
		int               m_nGrayOffset;          //��Ҷ�ƫ�� (��ʱ��ʾ�Ҷȼ��)
		int               m_nGrayLowOffset;       //��Ҷȵ�ƫ��
		kxRect<int>       m_rcEdgeRect;           //��Ե��
	};

	struct Result
	{
		Result()
		{
			memset(m_hBlobInfo, 0, sizeof(BlobInfo)*_Max_BlobCount);
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
			int                 m_nLeftSmall;
			int                 m_nTopSmall;
			int                 m_nBlobWidth;
			int                 m_nBlobHeight;
			int                 m_nWHRatio;
			int                 m_nAvgEnergy;
			int                 m_nAvgArea;
			int                 m_nMinRectWidth;
			int                 m_nMinRectHeight;
			kxCImageBuf         m_ErrImg;
		};

		BlobInfo      m_hBlobInfo[_Max_BlobCount];   //ȱ������
		int           m_nCount;                      //ʵ��ȱ�ݸ���
		int           m_nStatus;                     //���״̬��Ϣ
		int           m_nKernIndex;                  //���ƶȴ���λ�˱��
		int           m_nDistanceX;                  //������󷵻�ʵ�ʾ���
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
			for (int k = 0; k < _Max_BlobCount; k++)
			{
				m_hBlobInfo[k].m_ErrImg.Release();
			}
		}
	};

#pragma pack(pop)


public:
	kxCImageBuf              m_ImgTemplate[_Max_Layer + 1];  //ģ��+һ���ϳ�ģ��
	kxCImageBuf              m_ImgCompose;
	kxCImageBuf              m_ImgLearnTemplate[_Max_Layer + 1];  //�ֲ�ѧϰģ��+һ���ϳ�ģ��

public:
	CKxBaseFunction          m_hFun;
	CKxBaseFunction          m_hBaseFun[_Max_Layer];
	CKxColorDiff             m_hColorDiff;          //ɫ����
	CKxCalDistance           m_hCalDistance;        //������
	CKxWarpStretch           m_hWarpStrech;
	CKxLightAdjust           m_hLightAdjust;
	CKxGradientProcess       m_hGradient[_Max_Layer];
	CKxLearn                 m_hLearn[_Max_Layer];  //�ĸ�ѧϰ�����Ӧ�ĸ�����ͼ
	CKxSliderCc              m_hSlider[_Max_Layer];
	CKxImageMask             m_hImgMask;            //ͼ����Ĥ
	CkxPolygonMask		     m_hPolygonMask;					//�������Ĥ
	CKxBlobAnalyse           m_hBlobAnaly;          //Blob����

	Parameter                m_hParameter;
	kxCImageBuf              m_BaseImg; //��ģʱ�õĵװ壬���ڹ�ӰУ�����Զ���Ĥ

	int                      m_nModelNumber;                 //ģ������
	Result                   m_hResult;                      //���

	int                      m_nLearnPage;                     //ѧϰ����
	bool                     m_bIsLearnCompleted[_Max_Layer];  //ѧϰ�Ƿ�����
	bool                     m_bAllCompleted;

	int                      m_nAlgorithmStatus[_Max_Layer];               //8���������״̬
	int                      m_nLearnStatus;

	int                      m_nLargeSize;
	int                      m_nSpeckleMaxSize;                 //�������С
	int                      m_nHighOffset;
	int                      m_nLowOffset;

protected:
	//�����������ʱͼ��ռ�
	kxCImageBuf      m_OriginalImg;           //ԭʼͼ��
	kxCImageBuf      m_WarpImg;               //��ȷУ�����ͼ��
	kxCImageBuf      m_ExtendWarpImg;         //��ͨ��Եͼ��
	kxCImageBuf      m_LargeImg;
	kxCImageBuf      m_ExtendWarpImg1;        //�߽���0
	kxCImageBuf      m_MidImg[_Max_Layer];                //�м���ͼ��
	kxCImageBuf      m_ImgCheck[_Max_Layer];  //6�Ŵ���ͼ��,�ݺ�/�ݰ׶�Ӧ90,0,45 135����ͼ�� ȫ��2��
	kxCImageBuf      m_ImgSlidCc[_Max_Layer];
	kxCImageBuf      m_ImgSliderNorm[_Max_Layer];
	kxCImageBuf      m_ImgSlidCcMax[_Max_Layer];
	kxCImageBuf      m_ImgOffset[_Max_Layer];
	kxCImageBuf      m_ResImg[_Max_Layer];
	kxCImageBuf      m_ImgCcTemp;
	kxCImageBuf      m_ColorCheckImg;

	kxCImageBuf      m_TmpImg[_SAMLLDOTS_INDEX_END];
	kxCImageBuf      m_TmpImg1[_SAMLLDOTS_INDEX_END];
	kxCImageBuf      m_ImgTemp;               //
	kxCImageBuf      m_ImgCc;                 //�ϲ��в�ͼ(�Ҷ�ͼ)

	kxCImageBuf      m_StdImg;                //ɫ�����׼
	kxCImageBuf      m_TestImg;               //ɫ���鱻����ͼ
	kxCImageBuf      m_CreateMask;

	//�����ɲ�ɫ�в�ͼ��ʾ�йر���
	kxCImageBuf      m_ColorImg;             //��ɫͼ��
	kxCImageBuf      m_ResizeImg;            //ͼ��
	kxCImageBuf      m_ResidImg[4];          //��ɫ\��ɫ\��ɫ�в�ͼ
	kxCImageBuf      m_CompositionImg[3];    //��ɫ\��ɫ\��ɫ�ϳɲв�ͼ
	kxCImageBuf      m_AddImage;
	kxCImageBuf      m_BinaryImg;
	kxCImageBuf      m_DupImg;
	kxCImageBuf      m_ShowImg;
	kxCImageBuf      m_ResizeImg1;
	//��ȱ��ѧϰ
	kxCImageBuf      m_ImgSub[_Max_Layer];



public:

	int ReadImg(FILE* fp);
	int WriteImg(FILE* fp);

protected:
	//�����汾1�Ķ�д
	bool ReadVesion1Para(FILE* fp);
	bool ReadVesion2Para(FILE* fp);
	bool ReadVesion3Para(FILE* fp);

	bool ReadPara(FILE*  fp);
	bool WriteVesion1Para(FILE* fp);

	bool WritePara(FILE*  fp);
	int  CopyEdges(const kxCImageBuf& SrcImg, const kxCImageBuf CopyImg, kxCImageBuf& DstImg,
		kxRect<int>& rcEdge, kxRect<int>& rcCopy);
	int AnalysisCheckStatus(int nImageType);
	int AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg);
	int SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate);
	int SaveMidImage(bool bSave);
	int SetCheckExceptionStatus(const kxCImageBuf& ImgCheck); //�����쳣����Ϊ��Ʒ
public:

	//�������ļ���д����
	bool Read(FILE*);
	bool Write(FILE*);
	//�������л�ȡ����
	int ReadParaFromNet(unsigned char*& point);


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

	//���ѧϰ��״̬
	bool IsLearn()
	{
		return  m_hParameter.m_nIsLearn > 0 ? true : false;;
	}

	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);

	int  ProcessSurfaceWarpImages(KxCallStatus& hCall);
	int  LearnTemplate(KxCallStatus& hCall);
	int  ResidualsAnalys(KxCallStatus& hCall );
	int  ConnectAnalysis(const kxCImageBuf& SrcImg, KxCallStatus& hCall);

	int  SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex, KxCallStatus& hCall);
	int  SingleDefectLearn(const kxCImageBuf& ImgCheck, int nBlobIndex);

	int  ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6], KxCallStatus& hCall);
	int  ShowResidualsImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nOffset[6]);
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

	//���õ����ݶ�Ԥ�������߶�
	void SetLargeSize(int nLargeSize)
	{
		m_nLargeSize = nLargeSize;
	}

	//����������ߴ�
	void SetSpeckleMaxSize(int nSpeckleMaxSize)
	{
		m_nSpeckleMaxSize = nSpeckleMaxSize;
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
		return m_WarpImg.GetImageBuf(nWidth, nHeight, nPitch, nChannel);
	}
	kxCImageBuf& GetWarpImage() { return m_WarpImg; }

	//��ȡ�в�ͼ��
	unsigned char* GetImageCc(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		return m_ImgCc.GetImageBuf(nWidth, nHeight, nPitch, nChannel);
	}
	kxCImageBuf& GetImageCc() { return m_ImgCc; }

	//��ȡ�Ƿ���������ģ��
	//1 ������  0�����ر�
	int IsOpenSurfaceCheck() { return m_hParameter.m_nIsOpenSurfaceCheck; }



};

#endif