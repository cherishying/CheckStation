
#ifndef _KXLEARNHHHH
#define _KXLEARNHHHH

//add by lyl 2016/6/13
//学习算法进行归纳、汇总、并增加三通道的最大值，最小值学习算法
//该类现有功能：
//1、C1/C3极大值极小值学习算法
//2、完全打包学习算法

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
		_Learn_Max_Layer = 3,  //最大学习层数
	};

protected:
	int                m_nLearnHighPage;       //学习高张数
	int                m_nLearnLowPage;       //学习低张数
	int                m_nHaveLearnHigh;      //已经学习高模板的张数
	int                m_nHaveLearnLow;       //已经学习低模板的张数

	//学习高模板需要申请的空间
	kxCImageBuf        m_LeftMaxbuf;
	kxCImageBuf        m_TempMaxbuf;
	kxCImageBuf        m_PyramidMaxBuf[_Learn_Max_Layer];

	//学习低模板需要申请的空间
	kxCImageBuf        m_LeftMinbuf;
	kxCImageBuf        m_TempMinbuf;
	kxCImageBuf        m_PyramidMinBuf[_Learn_Max_Layer];

	//临时申请空间
	kxCImageBuf        m_TmpBuf;

	CKxBaseFunction    m_hBaseFun;

	//内部调用函数
private:
	void Init( int nWidth, int nHeight, int nChannel, int nMode);
	void InitParallel(int nWidth, int nHeight, int nChannel, int nMode);

	//外部调用函数	
public:
    //输入：一张待学习的图像和学习的张数
	//返回值：为1时，表示学习完成！即可获得模板
	int LearnLow( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel,  int nLearnPage, KxCallStatus& hCall);	
	
	//输入：一张待学习的图像和学习的张数
	//返回值：为1时，表示学习完成！即可获得模板
	int LearnHigh( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel, int nLearnPage, KxCallStatus& hCall);

	//输入：一张待学习的图像和学习的张数
	//返回值：为1时，表示学习完成！即可获得模板
	int LearnLow( kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall);
	int LearnLow(kxCImageBuf& LearnImg, int nLearnPage = 50);

	//输入：一张待学习的图像和学习的张数
	//返回值：为1时，表示学习完成！即可获得模板
	int LearnHigh( kxCImageBuf& LearnImg, int nLearnPage, KxCallStatus& hCall);
	int LearnHigh(kxCImageBuf& LearnImg, int nLearnPage = 50);

	//获取学习模板后处理的高模板图像
	unsigned char* GetHighTemplateBuf( int nIndex,  int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess = _3X3);
	int GetHighTemplateBuf( int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess  = _3X3);
	
	//获取学习模板后处理的低模板图像
	unsigned char* GetLowTemplateBuf( int nIndex,  int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess = _3X3);
    int GetLowTemplateBuf( int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess  = _3X3);

	int GetTemplateBuf(int nIndex, int nLearnMode, kxCImageBuf& TemplateBuf, int nModelProcess);
	int LearnImage(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage, KxCallStatus& hCall);
	int LearnImage(kxCImageBuf& LearnImg, int nHighLowMode, int nLearnPage);
	//清空已学习张数
	void Clear();

	unsigned char* GetHighTemplateBufParallel(int nIndex, int& nWidth, int& nHeight, int& nPitch, int& nChannel, int nModelProcess = _3X3);
	int GetHighTemplateBufParallel(int nIndex, kxCImageBuf& TemplateBuf, int nModelProcess = _3X3);

	//获取学习模板后处理的低模板图像
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
//       m_hLearn.GetHighTemplateBuf(……);
//    }

#endif


