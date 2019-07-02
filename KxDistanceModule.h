#ifndef _KXDISTANCEMODULEHH
#define _KXDISTANCEMODULEHH

//������ģ��
//2017/11/13
//add  by  lyl
//������ʼ�汾Ϊ��Distance2.0


#include "KxBaseFunction.h"
#include "kxDef.h"
#include "gxMath.h"
#include "KxGradientProcess.h"
#include <algorithm>

class CKxDistanceModule
{
public:
	CKxDistanceModule();
	~CKxDistanceModule();

	enum 
	{
		_SEARCH_VERTICAL_LINE = 0,            //�Ѵ�ֱ��
		_SEARCH_HORIZONTAL_LINE = 1,            //��ˮƽ��	
		_SEARCH_IMAGE = 2,            //��ͼ��
		_MAX_MEASURE_GROUP = 32,           //����������
		_SEARCH_LEN = 3,
	};

//private:

#pragma pack(push, 1)

	struct MeasurePara
	{
		MeasurePara()
		{
			m_nSearchExtend = 30;
		}
		std::string    m_szName;                  //����
		int            m_nIsOpen;                 //�Ƿ��
		int            m_nRefSearchType;          //�ο���������λ���ͣ��Ѵ�ֱ��/��ˮƽ��/��ͼ�ģ�
		kxRect<int>    m_rcRefInfo[2];            //�ο���λ����Ϣ /*(������ + ��λ��)*/
		int            m_nRefSearchDir;              //��������
		int            m_nRefSearchThresh;           //������ֵ

		int            m_nFixed;                  //�ο���λ���Ƿ�̶�
		int            m_nMeasureSearchType;      //������������λ����
		kxRect<int>    m_rcMeasureInfo[2];        //������������λ����
		int            m_nMeasureSearchDir;              //��������
		int            m_nMeasureSearchThresh;           //������ֵ

		int            m_nMeasureDirection;       //��������(��ֱ/ˮƽ)
		int            m_nSearchExtend;           //������Χ
		
		
		float          m_fStandardValue;          //��׼ֵ(mm)
		float          m_fErrorValue;             //���ֵ(mm)
	};

	struct Parameter
	{
		Parameter()
		{
			m_fActualWidth = 1;
			m_fActualHeight = 1;
			m_fProcessWidth = 1;
			m_fProcessHeight = 1;
			m_nGroup = 0;
			memset(m_hMeasure, 0, sizeof(MeasurePara)* _MAX_MEASURE_GROUP);
		}
		int                 m_nGroup;                 //�������������0-16��
		MeasurePara         m_hMeasure[_MAX_MEASURE_GROUP];
		//��������ֱ��ʼ���
		float               m_fActualWidth;      //�궨���mm��
		float               m_fActualHeight;     //�궨���
		float               m_fProcessWidth;     //����ͼ����Ӧ��������
		float               m_fProcessHeight;    //����ͼ�ߣ���Ӧ��������
	};

	struct SingleMeasureResult
	{
		std::string    m_szName;             //����
		std::string    m_szStandardValue;    //��׼ֵ��Χ('6.52-7.58')
		float          m_fMeasureValue;      //����ֵ
		int            m_nIsOk;              //�Ƿ�����

		void clear()
		{
			m_fMeasureValue = -1.0f;
			m_nIsOk = 0;
		}
	};

	struct Result
	{
		Result()
		{
			memset(m_hMeasureResults, 0, sizeof(SingleMeasureResult)* _MAX_MEASURE_GROUP);
		}

		void clear()
		{
			m_nGroup = 0;
			memset(m_hMeasureResults, 0, sizeof(SingleMeasureResult)* _MAX_MEASURE_GROUP);
		}
		int                 m_nGroup;
		SingleMeasureResult m_hMeasureResults[_MAX_MEASURE_GROUP];
	};

#pragma pack(pop)

private:
	//����
	Parameter             m_hParameter;
	Result                m_hResult;
	SingleMeasureResult   m_hSingleRes[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_Imgkern[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgBase;
	kxCImageBuf           m_TmpImg;
	CKxBaseFunction       m_hBaseFun[_MAX_MEASURE_GROUP];
	CKxGradientProcess    m_hGradient[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgCopy[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgGradientDir[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgMaxGradient[_MAX_MEASURE_GROUP];
	kxCImageBuf           m_ImgThreshGradient[_MAX_MEASURE_GROUP];

private:
	int PreAnalyseImage(const kxCImageBuf& SrcImg, kxRect<int>& rcSearch, int nSearchType, int nIndex, Ipp32f* pProject);
	int SingleMeasure(const kxCImageBuf& ImgCheck, const MeasurePara& para, int nIndex, SingleMeasureResult& res, KxCallStatus& hCall);
	int ComputeDistance(const kxCImageBuf& SrcImg, const kxCImageBuf& BaseImg, int nSearchType, int nSearchDir, int nThresh, const kxRect<int> rc[2], int nIndex, int& nDx, int& nDy, KxCallStatus& hCall);

public:
	//��xml�ж�ȡ�ļ�
	bool ReadXml(const char* filePath);
	bool ReadXmlinEnglish(const char* filePath);

	//������궨
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);

	
	
	Parameter& GetParameter()
	{
		return m_hParameter;
	}

	Result& GetResult()
	{
		return m_hResult;
	}
};

#endif
