#ifndef _KXWARPSTRETCHHHHH
#define _KXWARPSTRETCHHHHH

//add by lyl 2016/6/6
//������ɺͲ���ͼ��У��ģ��
//�������¹��ܣ�
// 0. 0����λ�˵Ŀ������ܣ�
// 1. 1����λ�˵�ƽ�Ʊ任У����
// 2. 2����λ�˵�ƽ����ת�任У����
// 3. 3����λ�˵ķ���任У����
// 4. 4����8����λ�˵�͸��ͶӰ�任У����
// 5. 4����8����λ�˵�˫���Ա任У����

////������ʼ�汾��Ϊ��WarpStretch1.0

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
		_Max_Kern = 8,           //���λ�˸���
		_Perspective_Trans = 0,  //͸�ӱ任
		_Bilinear_Trans = 1,     //˫���Ա任
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
		char         m_szVersion[_VersionNameLen];    //��¼�����İ汾��Ϣ
		kxRect<int>  m_rcCheckArea;                   //�������
		int          m_nKernCount;                    //��λ�˸���
		kxRect<int>  m_rcKern[_Max_Kern];             //��λ������
		int          m_nSearchExpand;                 //��λ���������䷶Χ
		int          m_nKernelDirect[_Max_Kern];      //��λ����������
		int          m_nAlgorithmType;                //�㷨���ͣ���������͸�ӱ任����˫���Ա任,ֻ�ж�λ�˸�������3��Ч

	};

	#pragma pack(pop)

protected:
	Parameter        m_hParameter;         //������
	kxCImageBuf      m_KernBuf[_Max_Kern]; //��λͼ��
	kxCImageBuf      m_WarpImg;            //������ͼ��

	CKxBaseFunction  m_hBaseFun;
	CKxBaseFunction  m_hFun[_Max_Kern]; //���ڲ���


	kxCImageBuf      m_SrcImg;
	kxCImageBuf      m_DstImg;

	int              m_nKernX;             //��һ����λ�˵�ʵ��λ�ã���Ϊ������ݷ��س�ȥ����Ϊ����궨����
	int              m_nKernY;

	double           m_matchQuad[_Max_Kern][2]; //ƥ�䵽�ĵ㣬�����ѱ任�������ͼ��
	double           m_kernQuad[_Max_Kern][2];  //ģ��ͼ���϶�λ�˵�LeftTop��
	double           m_coeffsFinePerspective[3][3];  //͸�ӱ任ϵ��
	double           m_coeffsFineBilinear[2][4];     //˫���Ա任ϵ��
	double           m_coeffsFineAffine[2][3];       //����任ϵ��
	kxPoint<int>     m_ptOffset;                     //ƫ�Ʊ任


protected:
	//�����汾1�Ķ�д
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;

	//�ڲ����ú���
protected:
	//һ����λ�˵�ƽ�Ʊ任У��
	int GetImgOffset(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, int nImgType, int& nDx, int& nDy,  KxCallStatus& hCall);

	//������λ�˵���תƫ�Ʊ任У��
	int RotateOffsetTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall);

	//��תƫ�Ʊ任����ϵ��
	int GetRotateOffTransform( const double srcQuad[2][2], const double dstQuad[2][2], double coeffs[2][3], KxCallStatus& hCall);

	//������λ�˵ķ���任У��
	int AffineTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall);
    int GetAffineTransform( const double srcQuad[3][2], const double dstQuad[3][2], double coeffs[2][3], KxCallStatus& hCall);

	//�ĸ����˸���λ�˵�͸��ͶӰ�任У��
	int  PerspectiveTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall );
    bool GetPerspectiveTransform_8(const double srcQuad[8][2], const double dstQuad[8][2], double coeffs[3][3], KxCallStatus& hCall);
    bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3], KxCallStatus& hCall );

	//�ĸ����˸���λ�˵�˫���Բ�ֵ�任У��
	int BilinearTransformWarp(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall );
	int GetBilinearTransform( const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4], KxCallStatus& hCall  );
	int GetBilinearTransform_8( const double srcQuad[8][2], const double dstQuad[8][2], double coeffs[2][4], KxCallStatus& hCall ) ;


	//�����㷨
	int RotateOffsetTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall , int nRowGrain = 120, int nColGrain = 120);
	int AffineTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int PerspectiveTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	//int BilinearTransformWarpParallel(const unsigned char* pBuf, int nWidth, int nHeight, int nPitch, unsigned char* pDstBuf, int nDstPitch, int nChannel, int nRowGrain = 120, int nColGrain = 120, KxCallStatus& hCall = KxCallStatus());



	//�ⲿ���ú���
public:
	//�������ļ���д����
	bool Read( FILE* );
	bool Write( FILE* );
	//�������л�ȡ����
	bool ReadParaFromNet( unsigned char*& point );

	//ͨ��Xml������
	//filePath      Ϊxml·����
	//szModuleName  Ϊģ����,���ݽ������ã�Ʃ�籾ģ�� szModuleName = ����λ�����á�
	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);

	//�������ӵĶ������ӿڣ�ROI����ʽ������
	bool ReadXmlStandard(const char* filePath, const kxCImageBuf& BaseImg);

	//��У��ͼ�ϵĲв�������У��
	int GetRightPos(kxRect<int> & rcSrc, kxRect<int>& rcDst);

	//���룺һ�Ŵ�У����ͼ��
	//�����У��ͼ������ m_WarpImg
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);


	//���룺һ�Ŵ�У����ͼ��
	//�����һ��У�����ͼ��
	//����ֵ��0-8 1-8��ʾ���������λ�˵ı�ţ�0��ʾ�������졣
	//         -1 ��ʾ���������쳣��һ��ָIpp�쳣
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
		       unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall);

	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int Check(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);
	//��ȡ������ͼ��
	const kxCImageBuf& GetWarpImg() const { return m_WarpImg; }
	const unsigned char * GetWarpBuf( int& nWidth, int& nHeight, int& nPitch, int& nChannel )
	{
		return m_WarpImg.GetImageBuf( nWidth, nHeight, nPitch, nChannel );
	}

	//��ȡ����
	Parameter& GetParameter()  {return  m_hParameter;}

	//��ȡ��һ����λ�˵�ʵ��λ��ƫ��,���ھ���궨
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


	//�����㷨
	//���룺һ�Ŵ�У����ͼ��
	//�����У��ͼ������ m_WarpImg
	int CheckParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);

	//���룺һ�Ŵ�У����ͼ��
	//�����һ��У�����ͼ��
	//����ֵ��0-8 1-8��ʾ���������λ�˵ı�ţ�0��ʾ�������졣
	//         -1 ��ʾ���������쳣��һ��ָIpp�쳣
	int CheckParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,
		unsigned char* pDstBuf, int nDstWidth, int nDstHeight, int nDstPitch, KxCallStatus& hCall , int nRowGrain = 120, int nColGrain = 120);

	int CheckParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int CheckParallel(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);
};

#endif
