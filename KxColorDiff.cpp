#include "KxColorDiff.h"
#include "KxBaseFunction.h"
#include "KxReadXml.h"


CKxColorDiff::CKxColorDiff()
{
	m_nColorDiff = 0;

}

CKxColorDiff::~CKxColorDiff()
{

}

bool CKxColorDiff::ReadXml(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "色差设置", "打开状态", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nOpenColorDiff);

	if (m_hParameter.m_nOpenColorDiff == 0)
	{
		return true;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "色差设置", "检测框", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcColorDiff);


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "色差设置", "阈值", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nColorDiffThresh);


	//截取定位核
	m_StdImg.Init(m_hParameter.m_rcColorDiff.Width(), m_hParameter.m_rcColorDiff.Height(), BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(BaseImg, m_StdImg, m_hParameter.m_rcColorDiff);

	return true;

}

bool CKxColorDiff::ReadXmlinEnglish(const char* filePath, const kxCImageBuf& BaseImg)
{
	//关键参数缺失，返回错误
	//一般参数缺失，采用默认值
	std::string szResult;
	int nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "ColorDiffSetting", "OpenStatus", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nOpenColorDiff);

	if (m_hParameter.m_nOpenColorDiff == 0)
	{
		return true;
	}

	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "ColorDiffSetting", "DetectBox", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToKxRect(szResult, m_hParameter.m_rcColorDiff);


	nSearchStatus = KxXmlFun::SearchXmlGetValue(filePath, "ColorDiffSetting", "ThresholdValue", szResult);
	if (!nSearchStatus)
	{
		return false;
	}
	KxXmlFun::FromStringToInt(szResult, m_hParameter.m_nColorDiffThresh);


	//截取定位核
	m_StdImg.Init(m_hParameter.m_rcColorDiff.Width(), m_hParameter.m_rcColorDiff.Height(), BaseImg.nChannel);
	m_hBaseFun.KxCopyImage(BaseImg, m_StdImg, m_hParameter.m_rcColorDiff);

	return true;

}


bool CKxColorDiff::ReadParaFromNet( unsigned char*& point )
{
	memcpy(&m_hParameter, point, sizeof(Parameter));
	point += sizeof(Parameter);
	CKxBaseFunction hBaseFun;
	if(!hBaseFun.readImgBufFromMemory(m_StdImg, point))
	{
		return false;
	}

	return true;
}

bool CKxColorDiff::ReadVesion1Para( FILE* fp)    //读取版本1参数
{
	if (fread(&m_hParameter.m_nOpenColorDiff, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_rcColorDiff, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fread(&m_hParameter.m_nColorDiffThresh, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	if (!m_StdImg.Read(fp))
	{
		return false;
	}

	return true;
}



bool CKxColorDiff::Read( FILE*  fp)
{
	if (fread(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (strcmp(m_hParameter.m_szVersion, "ColorDiff1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		return ReadVesion1Para(fp);
	}
	else
	{
		return false;
	}
}

bool CKxColorDiff::WriteVesion1Para( FILE* fp)    //写入版本1参数
{
	if (fwrite(m_hParameter.m_szVersion, sizeof(m_hParameter.m_szVersion), 1, fp) != 1)
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nOpenColorDiff, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_rcColorDiff, sizeof(kxRect<int>), 1, fp) != 1)		
	{
		return false;
	}
	if (fwrite(&m_hParameter.m_nColorDiffThresh, sizeof(int), 1, fp) != 1)		
	{
		return false;
	}

	if (!m_StdImg.Write(fp))
	{
		return false;
	}

	return true;

}

bool CKxColorDiff::Write( FILE*  fp)
{
	if (strcmp(m_hParameter.m_szVersion, "ColorDiff1.0") == 0)   //根据版本标识来判断需要调用的函数
	{
		WriteVesion1Para(fp);
	}
	else
	{
		return false;
	}
	return true;
}


int CKxColorDiff::Check(const kxCImageBuf& TestImg, double nKl, double nKc , double nKh )
{
	if (0 == m_hParameter.m_nOpenColorDiff)
	{
		return 0;
	}

	IppiSize Roi = {m_hParameter.m_rcColorDiff.Width(), m_hParameter.m_rcColorDiff.Height()};
	m_ImgStdLab.Init(m_StdImg.nWidth, m_StdImg.nHeight, m_StdImg.nChannel);
	m_ImgSampLab.Init(m_StdImg.nWidth, m_StdImg.nHeight, m_StdImg.nChannel);

    m_TmpImg.Init(m_StdImg.nWidth, m_StdImg.nHeight, m_StdImg.nChannel);
	m_hBaseFun.KxCopyImage(TestImg, m_TmpImg, m_hParameter.m_rcColorDiff);
	
    ippiBGRToLab_8u_C3R(m_StdImg.buf , m_StdImg.nPitch, m_ImgStdLab.buf, m_ImgStdLab.nPitch, Roi);
	ippiBGRToLab_8u_C3R(m_TmpImg.buf , m_TmpImg.nPitch, m_ImgSampLab.buf, m_ImgSampLab.nPitch, Roi);


    Ipp64f std[3];
	Ipp64f sample[3];

	ippiMean_8u_C3R(m_ImgStdLab.buf, m_ImgStdLab.nPitch, Roi, std);
	ippiMean_8u_C3R(m_ImgSampLab.buf, m_ImgSampLab.nPitch, Roi, sample);

	std[0] = std[0] * 100/255;
	std[1] = std[1] - 128;
	std[2] = std[2] - 128;

	sample[0] = sample[0] * 100/255;
	sample[1] = sample[1] - 128;
	sample[2] = sample[2] - 128;

	m_nColorDiff = int(ComputeDeltaE2000(std, sample, nKl, nKc, nKh) * 100);

	if (m_nColorDiff  > m_hParameter.m_nColorDiffThresh)
	{
		return 1;  //色差错误
	}

	return 0;

}

double CKxColorDiff::Check(const kxCImageBuf& StdImg, const kxCImageBuf& TestImg, double nKl, double nKc , double nKh )
{
	m_ImgStdLab.Init(StdImg.nWidth, StdImg.nHeight, StdImg.nChannel);
	m_ImgSampLab.Init(StdImg.nWidth, StdImg.nHeight, StdImg.nChannel);
	IppiSize Roi = {StdImg.nWidth, StdImg.nHeight};
	ippiBGRToLab_8u_C3R(StdImg.buf, StdImg.nPitch, m_ImgStdLab.buf, m_ImgStdLab.nPitch, Roi);
	ippiBGRToLab_8u_C3R(TestImg.buf, TestImg.nPitch, m_ImgSampLab.buf, m_ImgSampLab.nPitch, Roi);
	Ipp64f std[3];
	Ipp64f sample[3];

	ippiMean_8u_C3R(m_ImgStdLab.buf, m_ImgStdLab.nPitch, Roi, std);
	ippiMean_8u_C3R(m_ImgSampLab.buf, m_ImgSampLab.nPitch, Roi, sample);

	std[0] = std[0] * 100/255;
	std[1] = std[1] - 128;
	std[2] = std[2] - 128;

	sample[0] = sample[0] * 100/255;
	sample[1] = sample[1] - 128;
	sample[2] = sample[2] - 128;

	return ComputeDeltaE2000(std, sample, 2, 1, 0.5);

}



double CKxColorDiff::ComputeDeltaE2000(double LabStd[3], double LabSample[3], double nKl /* = 1 */, double nKc /* = 1 */, double nKh /* = 1 */)
{
	double Lstd = LabStd[0];
	double astd = LabStd[1];
	double bstd = LabStd[2];
	double Cabstd = sqrt(astd*astd + bstd*bstd);

	double Lsample = LabSample[0];
	double asample = LabSample[1];
	double bsample = LabSample[2];
	double Cabsample = sqrt(asample*asample + bsample*bsample);

	double Cabarithmean = (Cabsample + Cabstd)/2;

	double G = 0.5*(1 - sqrt( pow(Cabarithmean,7)/(pow(Cabarithmean,7) + pow(25.0, 7))));

	double apstd = (1 + G)*astd;
	double apsample = (1 + G)*asample;

	double Cpsample = sqrt( apsample * apsample + bsample * bsample);
	double Cpstd = sqrt(apstd * apstd + bstd * bstd);

	

	double hpstd = atan2(bstd, apstd);

	if (hpstd < 0)
	{
		hpstd = hpstd + 2*PI;
	}
	if (apstd == 0 && bstd == 0)
	{
		hpstd = 0;
	}

	double hpsample = atan2(bsample, apsample);

	if (hpsample < 0)
	{
		hpsample = hpsample + 2*PI;
	}
	if (apsample == 0 && bsample == 0)
	{
		hpsample = 0;
	}

	double dL = (Lsample - Lstd);
	double dC = (Cpsample - Cpstd);

	double dhp = (hpsample - hpstd);
	double Cpprod = Cpsample * Cpstd;
	if (Cpprod == 0)
	{
		dhp = 0;
	}
	else
	{
		if (dhp > PI)
		{
			dhp = dhp - 2*PI;
		}
		else if (dhp < -PI)
		{
			dhp = dhp + 2*PI;
		}
	}

	double dH = 2*sqrt(Cpprod)*sin(dhp/2);

	double Lp = (Lsample + Lstd)/2;
	double Cp = (Cpstd + Cpsample)/2;

	double hp = (hpstd + hpsample)/2;

	if (Cpprod == 0)
	{
		hp = hpstd + hpsample;
	}
	else
	{
		if (abs(hpstd - hpsample) > PI)
		{
			hp = hp - PI;
		}
		if (hp < 0)
		{
			hp = hp + 2*PI;
		}

	}

	double Lpm502 = (Lp - 50)*(Lp - 50);
	double Sl = 1 + 0.015*Lpm502/sqrt(20 + Lpm502);
	double Sc = 1 + 0.045*Cp;
	double T = 1 - 0.17*cos(hp - PI/6) + 0.24*cos(2*hp) + 0.32*cos(3*hp + PI/30)-0.20*cos(4*hp - 63*PI/180);
	double Sh = 1 + 0.015*Cp*T;
	double delthetarad = (30*PI/180)* exp(-((180/PI*hp-275)/25)*((180/PI*hp-275)/25));
    double Rc = 2*sqrt(pow(Cp, 7)/(pow(Cp, 7) + pow(25.0, 7)));
	double RT = -sin(2*delthetarad)*Rc;


	double klSl = nKl * Sl;
	double kcSc = nKc * Sc;
	double khSh = nKh * Sh;

	double de00 = sqrt( (dL/klSl)*(dL/klSl) + (dC/kcSc)*(dC/kcSc) + (dH/khSh)*(dH/khSh) 
		                 + RT * (dC/kcSc)*(dH/khSh));

	return de00;

}