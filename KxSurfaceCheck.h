
//add by lyl 2016/12/27
//This class is a general surface inspect class.
//It use intel's thread building block to parallel computing,
//which speed up the algorithm.
//This class's the interface as:
//input is a image,
//output is the result is some blob analysis results.

#ifndef _KXSURFACECHECKH
#define _KXSURFACECHECKH

#include "kxDef.h"
#include "gxMath.h"
#include "KxCommonDef.h"            //���õı�������
#include "KxBaseFunction.h"         //�Ƚϳ��õĺ���
#include "KxCalculateDistance.h"    //����궨���
#include "KxLightAdjust.h"          //��ӰУ���㷨
#include "KxWarpStretch.h"          //ͼ��У���㷨
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
#include "tinyxml2.h"
using namespace tbb;


class CKxSurfaceCheck
{
public:
	CKxSurfaceCheck();
	~CKxSurfaceCheck();

public:
	enum
	{
		_Valid_Layer = 2,
		_Max_Layer = 6,
		_CheckModeNum = 2,
		_Max_BlobCount = 4,
		_MAX_OFFSET = 10,
		_MAX_DEFECT_COUNT = 2,
	};
	//��������
	enum
	{
		_Check_Ok       = 0,
		_Check_Err      = 1,
		_Similarity_Err = 2,
		_FATAL_Err      = 3,
	};


#pragma pack(push, 1)
	//һЩ���Ʋ���
	struct  Parameter
	{
		Parameter()
		{
			m_nIsLearn        = 0;
			m_nLineOffset     = 2;
			m_nGrayHighOffset = 5;
			m_nGrayLowOffset  = 10;

			m_nGrayLowSmallDots    = 100;
			m_nGrayLowSmallEnergy  = 100;

			m_nGrayHighSmallDots   = 100;
			m_nGrayHighSmallEnergy = 100;

			m_nGradientSmallDots   = 100;
			m_nGradientSmallEnergy = 100;
		}
		//��λ����
		int               m_nBlockIdx;            //���
		kxRect<int>       m_rcCheck;              //�������
		kxRect<int>       m_rcKernel;       
		int               m_nExpand;            
		int               m_nSearchDir;
		int               m_nOffsetX;

		//������
		int               m_nIsLearn;             //�Ƿ�ѧϰ
		int               m_nLineOffset;          //��ϸ��ƫ��
		int               m_nGrayHighOffset;      //��Ҷȸ�ƫ��
		int               m_nGrayLowOffset;       //��Ҷȵ�ƫ��

		//������׼
		int               m_nGrayLowSmallDots;        //�Ҷȵ���С����
		int               m_nGrayLowSmallEnergy;      //�Ҷȵ���С����
		int               m_nGrayHighSmallDots;       //�Ҷȸ���С����
		int               m_nGrayHighSmallEnergy;     //�Ҷȸ���С����

		int               m_nGradientSmallDots;       //�ݶ���С����
		int               m_nGradientSmallEnergy;     //�ݶ���С����

		int               m_nIsFilterFakeDefect;     //�Ƿ�ȥ����ȱ��

	};


	struct  Result
	{
		Result()
		{
			Clear();
		}
		void Clear()
		{
			nStatus = _Check_Ok;  //��ʾ�������
			szErrInfo = "ok";
			nDefectCount = 0;
			mapFeaturelists.clear();
			mapFeaturelists.resize(_MAX_DEFECT_COUNT);
			for (int i = 0; i < _MAX_DEFECT_COUNT; i++)
			{
				mapFeaturelists[i].push_back(std::pair<std::string, float>("����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("X����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("Y����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("ȱ�ݿ�", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("ȱ�ݸ�", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("���", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("��ͼ����X", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("��ͼ����Y", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("���", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("���", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("Xԭ����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("Yԭ����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("��X����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("��Y����", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("POSY(mm)", 0.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("POSX(mm)", 12222.0f));
				mapFeaturelists[i].push_back(std::pair<std::string, float>("�к�", 0.0f));

			}
		}
		int                                        nStatus;    //���״̬
		std::string                                szErrInfo;       //��ע��Ϣ
		int                                        nDefectCount;    //ȱ�ݸ���
		std::vector<std::vector<std::pair<std::string, float>>>  mapFeaturelists; //ÿ��ȱ�ݵ������б�
	};



#pragma pack(pop)

public:
	kxCImageBuf              m_ImgKernel;     //��λ��ͼ��
	kxCImageBuf              m_ImgBigKernel;

public:
	kxCImageBuf              m_ImgTemplate[_Max_Layer + 1];  //ģ��+һ���ϳ�ģ��
	kxCImageBuf              m_ImgCompose;
	kxCImageBuf              m_ImgLearnTemplate[_Max_Layer + 1];  //�ֲ�ѧϰģ��+һ���ϳ�ģ��

public:
	CKxBaseFunction          m_hFun;
	CKxBaseFunction          m_hBaseFun[_Max_Layer];
	CKxWarpStretch           m_hWarpStrech;
	CKxLightAdjust           m_hLightAdjust;
	CKxGradientProcess       m_hGradient[_Max_Layer];
	CKxLearn                 m_hLearn[_Max_Layer];  //�ĸ�ѧϰ�����Ӧ�ĸ�����ͼ
	CKxSliderCc              m_hSlider[_Max_Layer];
	CKxImageMask             m_hImgMask;            //ͼ����Ĥ
	CkxPolygonMask		     m_hPolygonMask;					//�������Ĥ
	CKxBlobAnalyse           m_hBlobAnaly;          //Blob����
	CKxBlobAnalyse           m_hGradBlobAnaly;
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

	int                      m_nDx, m_nDy;
	int                      m_nIsLoadParamsSuccess;
	int                      m_nStandard[3];
	int                      m_nIsStandardChange;
	int                      m_nResizeFactorX, m_nResizeFactorY;

	int                      m_nBestLocationLayer;

	int                      m_nGrayModelSize; //

	int                      m_nIsBinary;
	int                      m_nThreshold;  //��ֵ����ֵ

	int                      m_nLowTemplateThreshold; //��ģ���ֵ��
	int                      m_nSlider;
protected:
	//�����������ʱͼ��ռ�
	kxCImageBuf      m_OriginalImg;           //ԭʼͼ��
	kxCImageBuf      m_WarpImg;               //��ȷУ�����ͼ��
	kxCImageBuf      m_ExtendWarpImg;         //��ͨ��Եͼ��
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

	//2018/11/1 ����ȥ��FPC�ٶ�·
	kxCImageBuf      m_ImgAnalyse;
	kxCImageBuf      m_ImgAnalyseBinary;
	kxCImageBuf      m_ImgAnalyseCc, m_ImgAnalyseCc1;
	kxCImageBuf      m_ImgAnalyseCcBinary;
	kxCImageBuf      m_ImgSubCc;
	kxCImageBuf      m_ImgAnalyseCcDilate;


	//kxCImageBuf      m_TmpImg[_SAMLLDOTS_INDEX_END];
	//kxCImageBuf      m_TmpImg1[_SAMLLDOTS_INDEX_END];
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

	kxCImageBuf      m_ImgTransform[_Max_Layer];
	kxCImageBuf      m_ImgLineMax;
	kxCImageBuf      m_ImgResizeSuper;
	kxCImageBuf      m_ImgSearch;
	kxCImageBuf      m_WarpResizeImg, m_WarpResizeImg1;
	kxCImageBuf      m_ImgCropBig;
	kxCImageBuf      m_ReferImg;
	kxCImageBuf      m_ImgDefect;
	kxCImageBuf      m_ImgTransfer;
	kxCImageBuf      m_ImgSrc;
	kxCImageBuf      m_ImgWarpGray;
protected:
	int GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg, kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy);
	int  CopyEdges(const kxCImageBuf& SrcImg, const kxCImageBuf CopyImg, kxCImageBuf& DstImg,
		kxRect<int>& rcEdge, kxRect<int>& rcCopy);
	int AnalysisCheckStatus(int nImageType);
	int AddResidualImage(const kxCImageBuf& CcImg, const kxCImageBuf& SrcImg, int nColorMode, kxCImageBuf& DstImg);
	int SetMask(const kxCImageBuf& SrcDstImg, const kxCImageBuf& MaskImg, Ipp8u lowGate, Ipp8u highGate);
	int SaveMidImage(bool bSave);
	int SetCheckExceptionStatus(kxCImageBuf& ImgCheck); //�����쳣����Ϊ��Ʒ
	void convertcolorimage2gray(kxCImageBuf& img, kxCImageBuf& grayimg, int nlayer);
	void GetBestLocationLayer(kxCImageBuf& img, int& nLayer);
public:

	bool ReadXml(tinyxml2::XMLDocument& xmlHandle, int nCameraId, int nIdx, const kxCImageBuf& BaseImg);
	bool ReadParaXml(tinyxml2::XMLDocument& xmlHandle, int nCameraId, int nIdx, const kxCImageBuf& BaseImg);

	int ReadImg(FILE* fp);
	int WriteImg(FILE* fp);

	//����ģ��ͼ��
	int SaveTemplateImg(const char* lpszFile);
	//��ȡģ��ͼ��
	int LoadTemplateImg(const char* lpszFile);


	//��ȡ�����
	Result&  GetCheckResult() 
	{
		return m_hResult;
	}
	//����ѧϰ����
	void SetLearnPage(int nLearn)
	{
		m_nLearnPage = nLearn;
	}

	//���û�����С
	void SetSliderRange(int nSlider)
	{
		m_nSlider = nSlider;
	}

	//���ѧϰ��״̬
	bool IsLearn()
	{
		return  m_hParameter.m_nIsLearn > 0 ? true : false;;
	}

	//����ѹ��ϵ��
	void  SetResizefactor(int nResizeX, int nResizeY)
	{
		m_nResizeFactorX = nResizeX;
		m_nResizeFactorY = nResizeY;
	}

	//���õ�ģ���ֵ��
	void SetLowTemplateThreshold(Ipp8u nThresh)
	{
		m_nLowTemplateThreshold = (int)nThresh;
	}


	int Check(const kxCImageBuf& ImgCheck, kxCImageBuf& DstImg, KxCallStatus& hCall);


	int  ProcessSurfaceWarpImages(KxCallStatus& hCall);
	int  LearnTemplate(KxCallStatus& hCall);
	int  ResidualsAnalys(KxCallStatus& hCall);
	int  ConnectAnalysis(const kxCImageBuf& SrcImg, kxCImageBuf& DefectImg, KxCallStatus& hCall);

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

	//���õ����ݶ�Ԥ������߶�
	void SetLargeSize(int nLargeSize)
	{
		m_nLargeSize = nLargeSize;
	}

	//����������ߴ�
	void SetSpeckleMaxSize(int nSpeckleMaxSize)
	{
		m_nSpeckleMaxSize = nSpeckleMaxSize;
	}

	//����������׼
	void SetCheckStandard(int* pStandard, int n)
	{
		assert(n == 3);
		memcpy(m_nStandard, pStandard, sizeof(int) * 3);
		m_nIsStandardChange = true;
	}

	//�ô�ˮƽƫ��У��ͼ��
	void SetOffsetX(int noffset)
	{
		m_nBigImgOffsetX = noffset;
	}

	void Setonlinestatus(int nstatus)
	{
		m_hParameter.m_nIsLearn = nstatus; 
		m_nLearnStatus = 1;
	}

	void SetLearnModeSize(int nModelsz)
	{
		m_nGrayModelSize = nModelsz;
	}


	//���ж�ֵ��Ԥ�������ڼ��ͭ��
	void SetIsBinary(int nIsBinary, int nThreshold)
	{
		m_nIsBinary = nIsBinary;
		m_nThreshold = nThreshold;
	}

	//���õ�ʱͼ��ʼ�е�����
	void SetStartlines(__int64 nStartline)
	{
		m_nStartline = nStartline;
	}

	void SetCameraResolution(float nCameraXResolution, float nCameraYResolution)
	{
		m_nCameraYResolution = nCameraYResolution;
		m_nCameraXResolution = nCameraXResolution;

	}

	void SetOriginalX(int& nOriginalX)
	{
		m_nOriginalX = nOriginalX;
	}

	//���߾ֲ�ѧϰ
	int SetDefectLearnStatus(const kxCImageBuf& srcImg, kxRect<int>& rcDefect, std::string szModelDir);

	//���ݸ������ģ��
	bool SetTemplatesFiles(const kxCImageBuf& srcImg, kxRect<int>& rcCheckArea, int nModelSize);

	//����ѧϰ��Ҫ��ͼ��
	protected:
		kxCImageBuf m_ImgOnlineDefectLearnGray;
		kxCImageBuf m_ImgOnlineDefectLearnGradient[4];
		int         m_nIsDefectLearn;
		CKxBaseFunction m_hDefectLearnFun;
		CKxGradientProcess m_hDefectLearnGradient;
		kxRect<int> m_rcDefectGray;
		kxRect<int> m_rcDefectGradient;
		int         m_nBigImgOffsetX;

	//��������������������λȱ��
		__int64         m_nStartline;
		float       m_nCameraYResolution, m_nCameraXResolution;  //����ֱ���
		int         m_nOriginalX;   //����ԭ������x


};

#endif
