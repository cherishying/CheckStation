#ifndef _KXPOLYGONMASKH
#define _KXPOLYGONMASKH

//add by CXH 2017/02/07
//�������ɺ�����ͼ��(C1/C3)��Ĥģ��
//�������¹��ܣ�
// 0. ����λ�˽�����Ĥ��
// 1. ������ֱ����Ĥ

//������ʼ�汾��Ϊ��PolygonMask1.0

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
		_Max_Mask = 8,  //�����Ĥ����
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

		char              m_szVersion[_VersionNameLen];  //��¼�����İ汾��Ϣ
		int               m_nKernelEx;                   //��λ���������ž���
		int               m_nMaskCount;                  //��ģ����
		int				  m_nEdgeCount[_Max_Mask];		//����α���
		kxPoint<int>	  m_rcMask[_Max_Mask][16];		//��ģ�������ݣ�_MaskCount = 8
		int               m_nIsMaskKernel[_Max_Mask];     //��ģ�Ƿ���Ҫ��λ��
		kxRect<int>		  m_rcMaskKernel[_Max_Mask]; 	  //��ģ��λ��λ��, �װ�����
		int               m_nMaskKernelDirect[_Max_Mask]; //0Ĭ�� 1��y���� 2��x����

		int               m_nIsOpenAutoMask;              //�Ƿ����Զ���Ĥ����
		int               m_nCreateMaskLayer;             //��Ĥ����
		int               m_nMaskPreProcess;              //������
		int               m_nOpenGradient;                //�Ƿ�����ݶȷ���
		int               m_nThresh;                      //��ֵ��ϵ��
		int               m_nComplement;                  //ȡ��

	};
#pragma pack(pop)


protected:
	Parameter                m_hParameter;

	kxCImageBuf              m_BaseImg;                //��ģʱ�ĵװ�ͼ�����ڿ۶�λ�˺����Զ���Ĥ����ͼ


	kxCImageBuf		         m_TmpBuf;       //��ʱ�ռ�
	kxCImageBuf		         m_imgSearch;    //��������ͼ�񣬶�̬����
	kxCImageBuf              m_Img;
	kxCImageBuf              m_Img1;

	kxCImageBuf              m_AutoMaskImg;  //�Զ���Ĥͼ��
	CKxBaseFunction          m_hBaseFun;
	CKxGradientProcess       m_hGradient;
	kxCImageBuf              m_ImgPre0;
	kxCImageBuf              m_ImgPre1;
	kxCImageBuf              m_ImgPre2;
	kxCImageBuf              m_ImgPre3;
	kxCImageBuf              m_ImgTemp;
	kxCImageBuf              m_PolygonMaskImg[_Max_Mask];  //�������Ĥͼ��
	kxRect<int>              m_rcPolyMask[_Max_Mask];
	kxCImageBuf              m_ResizeMaskImg;


private:
	int GetImgOffset(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, int nImgType, int nIndex, int& nDx, int& nDy);
	void VerticesToImage(kxPoint<int> *Vertex, int nVertices, unsigned char *pBuf, int nWidth, int nHeight, int nPitch);

protected:
	//�����汾1�Ķ�д
	bool ReadVesion1Para(FILE* fp);
	bool WriteVesion1Para(FILE* fp);

	//�ⲿ���ú���
public:
	kxCImageBuf		         m_ImgMaskKern[_Max_Mask]; //��Ĥ��λ��ͼ��
													   //�������ļ���д����
	bool Read(FILE*);
	bool Write(FILE*);
	bool ReadParaFromNet(unsigned char*& point);

	//�����Զ���Ĥͼ��
	int AutoCreateMask(kxCImageBuf& Srcimg);
	//�����ⲿ������ʾ���ɵ�Mask
	int ShowMaskImage(kxCImageBuf& BaseImg, kxRect<int>& rcPos);

	unsigned char* GetAutoMaskImage(int& nWidth, int& nHeight, int& nPitch, int& nChannel)
	{
		return m_AutoMaskImg.GetImageBuf(nWidth, nHeight, nPitch, nChannel);
	}


	Parameter& GetParameter()
	{
		return m_hParameter;
	}



	//���룺����Ĥͼ���һ�����ڶ�λ��ͼ��һ���Ǿ�ȷУ�����ͼ�񣩡����ֵ
	//SrcImg ��Maskͼ��һ���ǲв�ͼ��Ϊ�Ҷ�ͼ
	//KernImg Ϊ��λͼ��һ��ΪУ����Ĳ�ɫͼ
	//rcCheckArea ƫ������
	int Mask(kxCImageBuf& SrcImg, kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal, KxCallStatus& hCall);
	int Mask(kxCImageBuf& SrcImg, kxCImageBuf& KernImg, const kxRect<int>& rcCheckArea, int nMaskVal);
};

#endif
