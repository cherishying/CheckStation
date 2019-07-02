#ifndef _KXALOGRITHMHHHHHH
#define _KXALOGRITHMHHHHHH
//-----------------------------------------------
//add by lyl  2016/6/17
//�㷨��������㷨 v1.0 
//����ΪIpp9.0

#include "kxDef.h"
#include "gxMath.h"
#include "KxCommonDef.h"            //���õı�������
#include "KxBaseFunction.h"         //�Ƚϳ��õĺ���
#include "KxRotateCards.h"          //���������㷨
#include "KxCalculateDistance.h"    //����궨���
#include "KxLightAdjust.h"          //��ӰУ���㷨
#include "KxWarpStretch.h"          //ͼ��У���㷨
#include "KxColorDiff.h"            //ɫ����
#include "KxGradientProcess.h"      //�ݶ��㷨
#include "KxLearn.h"                //ѧϰ�㷨
#include "KxSliderCc.h"             //�����в��㷨
#include "KxImageMask.h"            //ͼ����Ĥ�����㷨
#include "KxBlobAnalyse.h"          //�Ľ������ͨ������㷨
#include "KxSearchCard.h"           //�Զ��ѿ��㷨
#include "kxPolygonMask.h"          //�������Ĥ
#include "KxGeneralSurfaceCheck.h"  //ͨ�ü��ģ��
#include "KxSignatureCheck.h"       //ǩ��������㷨


#include "KxZhuGuangCheck.h"        //������㷨
#include "KxSuperPrecisionCheck.h"  //����ϸ���
#include "KxQuanxiBiaoCheck.h"
#include "KxReadXml.h"
#include "KxDistanceModule.h"      //������ģ��
#include "KxReadXml2.h"
//-------------------------------------------------------------------
//������ѧ��mkl
#pragma comment(lib, "mkl_intel_lp64.lib") 
#pragma comment(lib, "mkl_sequential.lib") 
#pragma comment(lib, "mkl_core.lib") 
#pragma comment(lib, "libiomp5md.lib")
#pragma comment(lib, "mkl_intel_ilp64.lib")
#pragma comment(lib, "tinyxml.lib")


#ifdef NDEBUG
	#pragma comment( lib, "KxAlgorithmR.lib" )  
#else
	#pragma comment( lib, "KxAlgorithmD.lib" )
#endif


#endif