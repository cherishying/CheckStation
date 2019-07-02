
//add by lyl 2017/7/8
//read paramters from xml, this file which use to read xml parameter

#include <iostream>
#include <vector>

#include <wchar.h> 
#include "gxMath.h"
#include "tinyxml.h"
//using namespace std;

namespace KxXmlFun
{
	std::string string_To_UTF8(const std::string & str);
	std::string UTF8_To_string(const std::string & str);
	//��ȡһ��
	int SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string& szResult);
	//��ȡ����
	int SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string szParameterName, std::string& szResult);
	//��ȡ����
	int SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string& szResult);
	//��ȡ�ļ�
	int SearchXmlGetValue(const char* pXmlDir, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string szParameterName3, std::string& szResult);

	//XML�������ȡ��ʽ��2018/05/02
	//��xml�ļ�
	int OpenXmlFile(const char* pXmlDir, TiXmlDocument& xmlHandle);
	int OpenXmlFile(const char* pXmlDir, TiXmlDocument& xmlHandle, bool isEncrypt, char* key);

	//��ȡһ��
	int SearchXmlGetValue(TiXmlDocument& xmlHandle, std::string szMainModelName, std::string& szResult);
	//��ȡ����
	int SearchXmlGetValue(TiXmlDocument& xmlHandle, std::string szMainModelName, std::string szParameterName, std::string& szResult);
	//��ȡ����
	int SearchXmlGetValue(TiXmlDocument& xmlHandle, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string& szResult);
	//��ȡ�ļ�
	int SearchXmlGetValue(TiXmlDocument& xmlHandle, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string szParameterName3, std::string& szResult);
	//��ȡ�弶
	int SearchXmlGetValue(TiXmlDocument& xmlHandle, std::string szMainModelName, std::string szParameterName1, std::string szParameterName2, std::string szParameterName3, std::string szParameterName4, std::string& szResult);

	//�ر�xml�ļ�
	//int CloseXmlFile(TiXmlDocument& xmlHandle);


	int FromStringToInt(std::string sz, int &n);
	int FromStringToBool(std::string sz, int&n);
	int FromStringToDouble(std::string sz, double& n);
	int FromStringToFloat(std::string sz, float& n);
	void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);
	int FromStringToKxRect(std::string sz, kxRect<int>& pRect);
	int FromStringToMulKxRect(std::string sz, int nCount, kxRect<int>* pRect);
	int FromStringToMulInt(std::string sz, int nCount, int* pInt);
	int FromStringToKxPoint(std::string sz, kxPoint<int> & pt);
	int FromStringToMulKxPoint(std::string sz, int nCount, kxPoint<int>* pt);
}



