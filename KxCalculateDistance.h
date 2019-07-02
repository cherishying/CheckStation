#ifndef _KXCALCULATEDISTANCEHHHH
#define _KXCALCULATEDISTANCEHHHH

//用于标定距离
//采用四个十字架及定位核的方式，完成距离标定
//add  by  lyl
//参数初始版本为：Distance1.0


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
		//距离标定功能参数
		char              m_szVersion[_VersionNameLen];  //记录参数的版本信息

		int               m_nIsOpenCalPosition;  //开启距离标定，0代表不打开，1代表垂直、2代表水平、3代表垂直水平
		kxPoint<int>      m_ptVerCalPosition;      //标定位置
		kxRect<int>       m_rcVerKernPostion;      //对应定位核位置
		int               m_nVerSearchDir;         //对应的搜索方向
		kxPoint<int>      m_ptVerRefCalPosition;   //垂直参考边的位置
		kxRect<int>       m_rcVerRefKernPostion;   //对应定位核位置
		int               m_nVerRefKernSearchDir;   //对应的搜索方向
		int               m_nVerCardEdgeMinHeight; //距离卡边容许距离
		int               m_nVerCardEdgeMaxHeight;

		kxPoint<int>      m_ptHorCalPosition;      //标定位置
		kxRect<int>       m_rcHorKernPostion;      //对应定位核位置
		int               m_nHorSearchDir;         //对应的搜索方向
		kxPoint<int>      m_ptHorRefCalPosition;   //水平参考边的位置
		kxRect<int>       m_rcHorRefKernPostion;   //对应定位核位置
		int               m_nHorRefKernSearchDir;
		int               m_nHorCardEdgeMinWidth;
		int               m_nHorCardEdgeMaxWidth;

		int               m_nSearchExpand;       //搜索扩展范围

		int               m_nActualWidth;      //实际宽（实际长度 微米）
		int               m_nActualHeight;     //实际高
		int               m_nProcessCardWidth;  //处理图卡宽，对应多少像素
		int               m_nProcessCardHeight; //处理图卡高，对应多少像素

	};
	#pragma pack(pop)

private:
    //参数
	Parameter             m_hParameter;
	//定位核图像
	kxCImageBuf           m_VerKernImg;
	kxCImageBuf           m_VerRefKernImg;
	kxCImageBuf           m_HorKernImg;
	kxCImageBuf           m_HorRefKernImg;
	//临时申请图像
	kxCImageBuf           m_TmpImg;

	//距离结果
	int                   m_nVerDistance;
	int                   m_nHorDistance;

	CKxBaseFunction       m_hBaseFun;

	//搜边
	CKxGradientProcess    m_hGradient;
	kxCImageBuf           m_ImgCopy;
	kxCImageBuf           m_ImgGradientDir;
	kxCImageBuf           m_ImgMaxGradient;
	kxCImageBuf           m_ImgBinary;


protected:
	//参数版本1的读写
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;

public:
	//参数的文件读写操作
	bool Read( FILE* );
	bool Write( FILE* );

	//从网络中获取参数
	bool ReadParaFromNet( unsigned char*& point );

	//从xml中读取文件
	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);

    //检测距离标定
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch,  int nChannel, KxCallStatus& hCall);
	int Check(const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nChannel);
	//检测距离标定
	//输入：一张标定的图像
	//返回值：0表示检测正常
	//        1表示测量点定位核相似度错误
	//        2表示垂直定位核相似度错误
	//        3表示水平定位核相似度错误
    //        4表示距离错误
	int Check(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int Check(const kxCImageBuf& ImgCheck);
	//特殊检测距离标定,用于签名条标定
	//输入：一张标定的图像、标定位置
	//返回值：0表示检测正常
	//        1表示测量点定位核相似度错误
	//        2表示垂直定位核相似度错误
	//        3表示水平定位核相似度错误
	//        4表示距离错误
	int CheckSpecial(const kxCImageBuf& ImgCheck, kxRect<int>& rcPos, kxRect<int>& rcBasePos, KxCallStatus& hCall);
	int CheckSpecial(const kxCImageBuf& ImgCheck, kxRect<int>& rcPos, kxRect<int>& rcBasePos);

	//智能相机用于全息标标定，参考点采用小范围搜边的方式点位
	int CheckSpecialWithSearchEdge(const kxCImageBuf& ImgCheck, KxCallStatus& hCall);
	int CheckSpecialWithSearchEdge(const kxCImageBuf& ImgCheck);
	int SearchEdge(const kxCImageBuf& ImgCheck, const kxCImageBuf & ReferImg, kxRect<int>& rcSearch, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall);
	int SearchEdge(const kxCImageBuf& ImgCheck, const kxCImageBuf & ReferImg, kxRect<int>& rcSearch, int nSearchDir, int& nDx, int& nDy);
	//获取标定后水平垂直距离
	//返回值：（0,0）       测试点定位核相似度错误
	//         (1,0)        水平定位核错误
	//         (0,1)        垂直定位核错误
	//         (4535,4324)  距离标定错误的实际测量值
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
