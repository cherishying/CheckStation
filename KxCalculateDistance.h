#ifndef _KXCALCULATEDISTANCEHHHH
#define _KXCALCULATEDISTANCEHHHH

//���ڱ궨����
//�����ĸ�ʮ�ּܼ���λ�˵ķ�ʽ����ɾ���궨
//add  by  lyl
//������ʼ�汾Ϊ��Distance1.0


#include "KxBaseFunction.h"
#include "kxDef.h"
#include "gxMath.h"
#include "KxGradientProcess.h"

class CKxCalDistance
{
public:
	CKxCalDistance();
	~CKxCalDistance();

private:

	#pragma pack(push, 1)

	struct Parameter
	{
		Parameter()
		{
			memcpy(m_szVersion, "Distance1.0", _VersionNameLen);
		

			m_nIsOpenCalPosition = 0;
			m_ptVerCalPosition.setup(0,0);
			m_rcVerKernPostion.setup(0,0,1,1);
			m_ptVerRefCalPosition.setup(0,0);
			m_rcVerRefKernPostion.setup(0,0,1,1);
			m_nVerRefKernSearchDir = _Horiz_Vertical_Dir;
			m_nVerCardEdgeMinHeight = 0;
			m_nVerCardEdgeMaxHeight = 0;

			m_ptHorCalPosition.setup(0, 0);
			m_rcHorKernPostion.setup(0, 0, 1, 1);
            m_nHorSearchDir = _Horiz_Vertical_Dir;

			m_ptHorRefCalPosition.setup(0, 0);
			m_rcHorRefKernPostion.setup(0, 0, 1, 1);
			m_nHorRefKernSearchDir = _Horiz_Vertical_Dir;

			m_nHorCardEdgeMinWidth = 0;
			m_nHorCardEdgeMaxWidth = 0;

			m_nSearchExpand = 40;

			m_nActualWidth = 1;
			m_nActualHeight = 1;
			m_nProcessCardWidth = 1;
			m_nProcessCardHeight = 1;

		}
		//����궨���ܲ���
		char              m_szVersion[_VersionNameLen];  //��¼�����İ汾��Ϣ

		int               m_nIsOpenCalPosition;  //��������궨��0�����򿪣�1����ֱ��2����ˮƽ��3����ֱˮƽ
		kxPoint<int>      m_ptVerCalPosition;      //�궨λ��
		kxRect<int>       m_rcVerKernPostion;      //��Ӧ��λ��λ��
		int               m_nVerSearchDir;         //��Ӧ����������
		kxPoint<int>      m_ptVerRefCalPosition;   //��ֱ�ο��ߵ�λ��
		kxRect<int>       m_rcVerRefKernPostion;   //��Ӧ��λ��λ��
		int               m_nVerRefKernSearchDir;   //��Ӧ����������
		int               m_nVerCardEdgeMinHeight; //���뿨���������
		int               m_nVerCardEdgeMaxHeight;

		kxPoint<int>      m_ptHorCalPosition;      //�궨λ��
		kxRect<int>       m_rcHorKernPostion;      //��Ӧ��λ��λ��
		int               m_nHorSearchDir;         //��Ӧ����������
		kxPoint<int>      m_ptHorRefCalPosition;   //ˮƽ�ο��ߵ�λ��
		kxRect<int>       m_rcHorRefKernPostion;   //��Ӧ��λ��λ��
		int               m_nHorRefKernSearchDir;
		int               m_nHorCardEdgeMinWidth;
		int               m_nHorCardEdgeMaxWidth;

		int               m_nSearchExpand;       //������չ��Χ

		int               m_nActualWidth;      //ʵ�ʿ�ʵ�ʳ��� ΢�ף�
		int               m_nActualHeight;     //ʵ�ʸ�
		int               m_nProcessCardWidth;  //����ͼ������Ӧ��������
		int               m_nProcessCardHeight; //����ͼ���ߣ���Ӧ��������

	};
	#pragma pack(pop)

private:
    //����
	Parameter             m_hParameter;
	//��λ��ͼ��
	kxCImageBuf           m_VerKernImg;
	kxCImageBuf           m_VerRefKernImg;
	kxCImageBuf           m_HorKernImg;
	kxCImageBuf           m_HorRefKernImg;
	//��ʱ����ͼ��
	kxCImageBuf           m_TmpImg;

	//������
	int                   m_nVerDistance;
	int                   m_nHorDistance;

	CKxBaseFunction       m_hBaseFun;

	//�ѱ�
	CKxGradientProcess    m_hGradient;
	kxCImageBuf           m_ImgCopy;
	kxCImageBuf           m_ImgGradientDir;
	kxCImageBuf           m_ImgMaxGradient;
	kxCImageBuf           m_ImgBinary;


protected:
	//�����汾1�Ķ�д
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;

public:
	//�������ļ���д����
	bool Read( FILE* );
	bool Write( FILE* );

	//�������л�ȡ����
	bool ReadParaFromNet( unsigned char*& point );

	//��xml�ж�ȡ�ļ�
	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);

    //������궨
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch,  int nChannel, KxCallStatus& hCall);
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);
	//������궨
	//���룺һ�ű궨��ͼ��
	//����ֵ��0��ʾ�������
	//        1��ʾ�����㶨λ�����ƶȴ���
	//        2��ʾ��ֱ��λ�����ƶȴ���
	//        3��ʾˮƽ��λ�����ƶȴ���
    //        4��ʾ�������
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);
	//���������궨,����ǩ�����궨
	//���룺һ�ű궨��ͼ�񡢱궨λ��
	//����ֵ��0��ʾ�������
	//        1��ʾ�����㶨λ�����ƶȴ���
	//        2��ʾ��ֱ��λ�����ƶȴ���
	//        3��ʾˮƽ��λ�����ƶȴ���
	//        4��ʾ�������
	int CheckSpecial(const kxCImageBuf& ImgCheck, kxRect<int>& rcPos, kxRect<int>& rcBasePos, KxCallStatus& hCall);
	int CheckSpecial(const kxCImageBuf& ImgCheck, kxRect<int>& rcPos, kxRect<int>& rcBasePos);

	//�����������ȫϢ��궨���ο������С��Χ�ѱߵķ�ʽ��λ
	int CheckSpecialWithSearchEdge(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int CheckSpecialWithSearchEdge(const kxCImageBuf& ImgCheck);
	int SearchEdge(const kxCImageBuf& ImgCheck, const kxCImageBuf & ReferImg, kxRect<int>& rcSearch, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall);
	int SearchEdge(const kxCImageBuf& ImgCheck, const kxCImageBuf & ReferImg, kxRect<int>& rcSearch, int nSearchDir, int& nDx, int& nDy);
	//��ȡ�궨��ˮƽ��ֱ����
	//����ֵ����0,0��       ���Ե㶨λ�����ƶȴ���
	//         (1,0)        ˮƽ��λ�˴���
	//         (0,1)        ��ֱ��λ�˴���
	//         (4535,4324)  ����궨�����ʵ�ʲ���ֵ
	void GetHorVerDistance(int& nHorDistance, int& nVerDistance)
	{
		nVerDistance = m_nVerDistance;
		nHorDistance = m_nHorDistance;
	}

	Parameter& GetParameter()
	{
		return m_hParameter;
	}

};

#endif
