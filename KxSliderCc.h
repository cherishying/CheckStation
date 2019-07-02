#ifndef _KXSLIDERCCHHHHHHH
#define _KXSLIDERCCHHHHHHH

//add by lyl 2016/6/16
//������ɺͲ���ͼ�񻬶��в�ģ��
#include "kxDef.h"
#include "gxMath.h"
#include "ippi.h"
#include "KxCommonDef.h"          
#include "KxBaseFunction.h" 
//#include "KxAlogrithm.h"

class CKxSliderCc
{
public:
	CKxSliderCc();
	~CKxSliderCc();

	enum
	{
		_Check_Low  = 0, //��ʾ��໹�Ǽ���
		_Check_High = 1,
		_Dixy = 20,
	};

	struct Parameter
	{
		Parameter()
		{
			m_nStartX = -2;
			m_nEndX = 2;
			m_nStepX = 2;

			m_nStartY = -2;
			m_nEndY = 2;
			m_nStepY = 2;
			//[-3:2:3]
			m_nOffset = 0;
		}
		int       m_nStartX;  //������ʼƫ��
		int       m_nEndX;    //������ֹƫ��
		int       m_nStepX;   //��������

		int       m_nStartY;  //������ʼƫ��
		int       m_nEndY;    //������ֹƫ��
		int       m_nStepY;   //��������
		int       m_nOffset;
	};

private:
	Parameter        m_hParameter;
	kxCImageBuf      m_ImgCCBig;
	kxCImageBuf      m_ImgSlider;
	kxCImageBuf      m_GridImg;
	kxCImageBuf      m_ExtendImg;
	kxCImageBuf      m_MinImg;

	kxCImageBuf      m_MinImgRGB[3];

protected:
	void GetExImgWidthAndHight(int nSrcW, int nSrcH,int &nDstW,int &nDstH, int GridX, int GridY);
	
	//ͼ��C1�Ļ����в��㷨
	int SilderCcC1( const unsigned char* srcTestBuf, int srcTestPitch,
		            const unsigned char* srcModelBuf, int srcModelPitch,
		            unsigned char* dstBuf, int dstPitch,
		            int width, int height, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall );

    //ͼ��C3�Ļ����в��㷨
	int SilderCcC3( const unsigned char* srcTestBuf, int srcTestPitch,
		             const unsigned char* srcModelBuf, int srcModelPitch,
		             unsigned char* dstBuf, int dstPitch,
		             int width, int height, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall);

	//�����㷨
	//ͼ��C1�Ļ����в��㷨
	int SilderCcC1Parallel(const unsigned char* srcTestBuf, int srcTestPitch,
		const unsigned char* srcModelBuf, int srcModelPitch,
		unsigned char* dstBuf, int dstPitch,
		int width, int height, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall );

	//ͼ��C3�Ļ����в��㷨
	int SilderCcC3Parallel(const unsigned char* srcTestBuf, int srcTestPitch,
		const unsigned char* srcModelBuf, int srcModelPitch,
		unsigned char* dstBuf, int dstPitch,
		int width, int height, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall);


public:
    //ͼ��C1/C3�Ļ����в��㷨
	//���룺���в��ͼ��ģ��ͼ�񣬽��ͼ��
	//nStatus ��ʾ��ࣨ1�����Ǽ��٣�0��
	//nGridX, nGridY ��ʾ��������Ĵ�С
	int  SilderCc( const unsigned char* srcTestBuf, int srcTestPitch,
		           const unsigned char* srcModelBuf, int srcModelPitch,
		           unsigned char* dstBuf, int dstPitch,
		           int width, int height, int nChannel, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall);
	int  SilderCc(const unsigned char* srcTestBuf, int srcTestPitch,
		const unsigned char* srcModelBuf, int srcModelPitch,
		unsigned char* dstBuf, int dstPitch,
		int width, int height, int nChannel, int nStatus, int nGridX = 20, int nGridY = 20);

	
	int  SilderCc(const kxCImageBuf& SrcBuf,  const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf,
		           int nStatus, int nGridX , int nGridY,  KxCallStatus& hCall );
	int  SilderCc(const kxCImageBuf& SrcBuf, const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf,
		int nStatus, int nGridX = 20, int nGridY = 20);
    
	//���û�����Χ
	void SetSliderRange(int nStart, int nStep, int nEnd)
	{
		m_hParameter.m_nStartX= nStart;
		m_hParameter.m_nStepX = nStep;
		m_hParameter.m_nEndX = nEnd;


		m_hParameter.m_nStartY = nStart;
		m_hParameter.m_nStepY = nStep;
		m_hParameter.m_nEndY = nEnd;
	}

	void SetOffset(int nOffset)
	{
		m_hParameter.m_nOffset = nOffset;
	}

	void SetSliderRange(int nStartX, int nStepX, int nEndX, int nStartY, int nStepY, int nEndY)
	{
		m_hParameter.m_nStartX = nStartX;
		m_hParameter.m_nStepX = nStepX;
		m_hParameter.m_nEndX = nEndX;


		m_hParameter.m_nStartY = nStartY;
		m_hParameter.m_nStepY = nStepY;
		m_hParameter.m_nEndY = nEndY;
	}


	int  SilderCcParallel(const unsigned char* srcTestBuf, int srcTestPitch,
		const unsigned char* srcModelBuf, int srcModelPitch,
		unsigned char* dstBuf, int dstPitch,
		int width, int height, int nChannel, int nStatus, int nGridX, int nGridY, KxCallStatus& hCall);
	int  SilderCcParallel(const unsigned char* srcTestBuf, int srcTestPitch,
		const unsigned char* srcModelBuf, int srcModelPitch,
		unsigned char* dstBuf, int dstPitch,
		int width, int height, int nChannel, int nStatus, int nGridX = 20, int nGridY = 20);


	int  SilderCcParallel(const kxCImageBuf& SrcBuf, const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf,
		int nStatus, int nGridX, int nGridY, KxCallStatus& hCall);
	int  SilderCcParallel(const kxCImageBuf& SrcBuf, const kxCImageBuf& ModelBuf, kxCImageBuf& DstBuf,
		int nStatus, int nGridX = 20, int nGridY = 20);


};

#endif