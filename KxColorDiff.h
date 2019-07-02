#ifndef _KXCOLORDIFFHHHHHHH
#define _KXCOLORDIFFHHHHHHH
//add by lyl 2016/6/22
//it use to check the different color
//it use CIEDE2000 rule
#include "kxDef.h"
#include "KxBaseFunction.h"
//参数初始版本名为：ColorDiff1.0

class CKxColorDiff
{
public:
	CKxColorDiff();
	~CKxColorDiff();

	#pragma pack(push, 1)
	//色差检查参数
	struct Parameter
	{
		Parameter()
		{
			memcpy(m_szVersion, "ColorDiff1.0", _VersionNameLen);
			m_nOpenColorDiff = 0;
			m_rcColorDiff.setup(0,0,1,1);
			m_nColorDiffThresh = 0;

		}
        char              m_szVersion[_VersionNameLen];  //记录参数的版本信息
		int               m_nOpenColorDiff;     //是否开启色差检查
		kxRect<int>       m_rcColorDiff;        //色差检查区域
		int               m_nColorDiffThresh;   //允许色差

	};

	#pragma pack(pop)

private:
	kxCImageBuf    m_ImgStdLab;
	kxCImageBuf    m_ImgSampLab;
	kxCImageBuf    m_Lab[3];

	Parameter       m_hParameter;
	kxCImageBuf     m_StdImg;  //标准图像
	kxCImageBuf     m_TmpImg;
	CKxBaseFunction m_hBaseFun;

	int            m_nColorDiff;

private:
	double ComputeDeltaE2000(double LabStd[3], double LabSample[3], double nKl = 1.0, double nKc = 1.0, double nKh = 1.0);

protected:
	//参数版本1的读写
	bool ReadVesion1Para( FILE* fp);
	bool WriteVesion1Para( FILE* fp) ;
public:

	//参数的文件读写操作
	bool Read( FILE* );
	bool Write( FILE* );
	bool ReadParaFromNet( unsigned char*& point );
	bool ReadXml(const char* filePath, const kxCImageBuf& BaseImg);
	bool ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg);
    //色差检查函数 利用参数
    //输入：StdImg标准图像
	//      TestImg待检测的图像
	//两张图像大小一致
	//返回值：1 色差错误 0 色差正常
	int Check(const kxCImageBuf& TestImg, double nKl = 2.0, double nKc = 1.0, double nKh = 0.5);

    //色差检查函数，不调用参数，供外部直接调用
	//输入：StdImg标准图像
	//      TestImg待检测的图像
	//两张图像大小一致
	//返回值：色差值
	double Check(const kxCImageBuf& StdImg, const kxCImageBuf& TestImg, double nKl = 2.0, double nKc  = 1.0, double nKh  = 0.5);

	//获取色差值
	void GetColorDiff(int& nColorDiff)
	{
		nColorDiff = m_nColorDiff;
	}
	//获取参数
	Parameter& GetParameter()
	{
		return m_hParameter;
	}


};

#endif
