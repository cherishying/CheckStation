
#ifndef _KXLEARNHHHH
#define _KXLEARNHHHH

//add by lyl 2016/6/13
//ѧϰ�㷨���й��ɡ����ܡ���������ͨ�������ֵ����Сֵѧϰ�㷨
//�������й��ܣ�
//1��C1/C3����ֵ��Сֵѧϰ�㷨
//2����ȫ���ѧϰ�㷨

#include "kxDef.h"
#include "gxMath.h"
#include "KxBaseFunction.h"
#include "ipps.h"
#include "ippi.h"

class CKxLearn
{
public:
	CKxLearn();
	~CKxLearn();

	enum
	{
		_Learn_Low  = 0,
		_Learn_High = 1,
		_Learn_Max_Layer = 3,  //���ѧϰ����
	};

protected:
	int                m_nLearnHighPage;       //ѧϰ������
	int                m_nLearnLowPage;       //ѧϰ������
	int                m_nHaveLearnHigh;      //�Ѿ�ѧϰ��ģ�������
	int                m_nHaveLearnLow;       //�Ѿ�ѧϰ��ģ�������

	//ѧϰ��ģ����Ҫ����Ŀռ�
	kxCImageBuf        m_LeftMaxbuf;
	kxCImageBuf        m_TempMaxbuf;
	kxCImageBuf        m_PyramidMaxBuf[_Learn_Max_Layer];

	//ѧϰ��ģ����Ҫ����Ŀռ�
	kxCImageBuf        m_LeftMinbuf;
	kxCImageBuf        m_TempMinbuf;
	kxCImageBuf        m_PyramidMinBuf[_Learn_Max_Layer];

	//��ʱ����ռ�
	kxCImageBuf        m_TmpBuf;

	CKxBaseFunction    m_hBaseFun;

	//�ڲ����ú���
private:
	void Init( int nWidth, int nHeight, int nChannel, int nMode);
	void InitParallel(int nWidth, int nHeight, int nChannel, int nMode);

	//�ⲿ���ú���	
public:
    //���룺һ�Ŵ�ѧϰ��ͼ���ѧϰ������
	//����ֵ��Ϊ1ʱ����ʾѧϰ��ɣ����ɻ��ģ��
	int LearnLow( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,  int nLearnPage, KxCallStatus& hCall);	
	
	//���룺һ�Ŵ�ѧϰ��ͼ���ѧϰ������
	//����ֵ��Ϊ1ʱ����ʾѧϰ��ɣ����ɻ��ģ��
	int LearnHigh( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage, KxCallStatus& hCall);

	//���룺һ�Ŵ�ѧϰ��ͼ���ѧϰ������
	//����ֵ��Ϊ1ʱ����ʾѧϰ��ɣ����ɻ��ģ��
	int LearnLow( kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall);
	int LearnLow(kxCImageBuf& LearnImg, int nLearnPage = 50);

	//���룺һ�Ŵ�ѧϰ��ͼ���ѧϰ������
	//����ֵ��Ϊ1ʱ����ʾѧϰ��ɣ����ɻ��ģ��
	int LearnHigh( kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall);
	int LearnHigh(kxCImageBuf& LearnImg, int nLearnPage = 50);

	//��ȡѧϰģ�����ĸ�ģ��ͼ��
	unsigned char* GetHighTemplateBuf( int nIndex,  int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess = _3X3);
	int GetHighTemplateBuf( int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess  = _3X3);
	
	//��ȡѧϰģ�����ĵ�ģ��ͼ��
	unsigned char* GetLowTemplateBuf( int nIndex,  int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess = _3X3);
    int GetLowTemplateBuf( int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess  = _3X3);

	int GetTemplateBuf(int nIndex, int nLearnMode, kxCImageBuf& TemplateBuf, int nModelProcess);
	int LearnImage(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage, KxCallStatus& hCall);
	int LearnImage(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage);
	//�����ѧϰ����
	void Clear();

	unsigned char* GetHighTemplateBufParallel(int nIndex, int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess = _3X3);
	int GetHighTemplateBufParallel(int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess = _3X3);

	//��ȡѧϰģ�����ĵ�ģ��ͼ��
	unsigned char* GetLowTemplateBufParallel(int nIndex, int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess = _3X3);
	int GetLowTemplateBufParallel(int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess = _3X3);

	int LearnLowParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage,  KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int LearnHighParallel(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage, KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);


	int LearnLowParallel(kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);	
	int LearnHighParallel(kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);

	int GetTemplateBufParallel(int nIndex, int nLearnMode, kxCImageBuf& TemplateBuf, int nModelProcess);
	int LearnImageParallel(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage, KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int LearnImageParallel(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage);

};

//   For example:
//   CKxLearn  m_hLearn;
//   int nStatus = m_hLearn.LearnHigh(m_CheckImg.buf, m_CheckImg.nWidth, m_CheckImg.nHeight, m_CheckImg.nPitch, _Type_G24);
//   if (nStatus)
//   {
//       m_hLearn.GetHighTemplateBuf(����);
//    }

#endif


