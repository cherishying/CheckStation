#ifndef _KXCOLORDIFFHHHHHHH
#define _KXCOLORDIFFHHHHHHH
//add by lyl 2016/6/22
//it use to check the different color
//it use CIEDE2000 rule
#include "kxDef.h"
#include "KxBaseFunction.h"
//������ʼ�汾��Ϊ��ColorDiff1.0

class CKxColorDiff
{
public:
	CKxColorDiff();
	~CKxColorDiff();

	#pragma pack(push, 1)
	//ɫ�������
	struct Parameter
	{
		Parameter()
		{
			memcpy(m_szVersion, "ColorDiff1.0", _VersionNameLen);
			m_nOpenColorDiff = 0;
			m_rcColorDiff.setup(0,0,1,1);
			m_nColorDiffThresh = 0;

		}
        char              m_szVersion[_VersionNameLen];  //��¼�����İ汾��Ϣ
		int               m_nOpenColorDiff;     //�Ƿ���ɫ����
		kxRect<int>       m_rcColorDiff;        //ɫ��������
		int               m_nColorDiffThresh;   //����ɫ��

	};

	#pragma pack(pop)

private:
	kxCImageBuf    m_ImgStdLab;
	kxCImageBuf    m_ImgSampLab;
	kxCImageBuf    m_Lab[3];

	Parameter       m_hParameter;
	kxCImageBuf     m_StdImg;  //��׼ͼ��
	kxCImageBuf     m_TmpImg;
	CKxBaseFunction m_hBaseFun;

	int            m_nColorDiff;

private:
	double ComputeDeltaE2000(double LabStd[3], double LabSample[3], double nKl = 1.0, double nKc = 1.0, double nKh = 1.0);

protected:
	//�����汾1�Ķ�д
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;
public:

	//�������ļ���д����
	bool Read( FILE* );
	bool Write( FILE* );
	bool ReadParaFromNet( unsigned char*& point );
	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);
    //ɫ���麯�� ���ò���
    //���룺StdImg��׼ͼ��
	//      TestImg������ͼ��
	//����ͼ���Сһ��
	//����ֵ��1 ɫ����� 0 ɫ������
	int Check(const kxCImageBuf& TestImg, double nKl = 2.0, double nKc = 1.0, double nKh = 0.5);

    //ɫ���麯���������ò��������ⲿֱ�ӵ���
	//���룺StdImg��׼ͼ��
	//      TestImg������ͼ��
	//����ͼ���Сһ��
	//����ֵ��ɫ��ֵ
	double Check(const kxCImageBuf& StdImg, const kxCImageBuf& TestImg, double nKl = 2.0, double nKc  = 1.0, double nKh  = 0.5);

	//��ȡɫ��ֵ
	void GetColorDiff(int& nColorDiff)
	{
		nColorDiff = m_nColorDiff;
	}
	//��ȡ����
	Parameter& GetParameter()
	{
		return m_hParameter;
	}


};

#endif
