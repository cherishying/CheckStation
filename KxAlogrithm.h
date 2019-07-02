#ifndef _KXALOGRITHMHHHHHH
#define _KXALOGRITHMHHHHHH
//-----------------------------------------------
//add by lyl  2016/6/17
//算法库包含的算法 v1.0 
//升级为Ipp9.0

#include "kxDef.h"
#include "gxMath.h"
#include "KxCommonDef.h"            //常用的变量申明
#include "KxBaseFunction.h"         //比较常用的函数
#include "KxRotateCards.h"          //卡边搜索算法
#include "KxCalculateDistance.h"    //距离标定检测
#include "KxLightAdjust.h"          //光影校正算法
#include "KxWarpStretch.h"          //图像校正算法
#include "KxColorDiff.h"            //色差检测
#include "KxGradientProcess.h"      //梯度算法
#include "KxLearn.h"                //学习算法
#include "KxSliderCc.h"             //滑动残差算法
#include "KxImageMask.h"            //图像掩膜设置算法
#include "KxBlobAnalyse.h"          //改进后的连通域分析算法
#include "KxSearchCard.h"           //自动搜卡算法
#include "kxPolygonMask.h"          //多边形掩膜
#include "KxGeneralSurfaceCheck.h"  //通用检查模块
#include "KxSignatureCheck.h"       //签名条检查算法


#include "KxZhuGuangCheck.h"        //珠光检测算法
#include "KxSuperPrecisionCheck.h"  //超精细检测
#include "KxQuanxiBiaoCheck.h"
#include "KxReadXml.h"
#include "KxDistanceModule.h"      //距离检查模块
#include "KxReadXml2.h"
//-------------------------------------------------------------------
//加载数学库mkl
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