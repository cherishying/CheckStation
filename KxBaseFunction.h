#ifndef _KXBASEFUNCTIONHHHH
#define _KXBASEFUNCTIONHHHH

//add by lyl 2016/6/6
//比较常用的基础函数放在一起
//1.C1/C3相似度定位
//2.C1/C3/C4的图像区域拷贝
//3.存图/取图函数
//4. C1/C3图像的不同尺寸的腐蚀膨胀操作
//5. C1/C3图像层面转换
//6. 数据转换（图像转化为字符串，整形转化为字符串等）
//7. C1/C3最小值或最大值
//8. C1/C3一个定位核计算图像偏移
//9. C1/C3图像Resize功能

#include "kxDef.h"
#include "gxMath.h"
#include "ippcc.h"
#include "ippi.h"
#include "ipps.h"
//#include "ippm.h"
#include "mkl.h"
#include <sstream>

#include <opencv2/opencv.hpp>
#include "opencv2/features2d/features2d.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/calib3d/calib3d.hpp" 
#include "opencv2/imgproc/imgproc_c.h" 
#include "opencv2/imgproc/imgproc.hpp"   


class CKxBaseFunction
{
public:
	CKxBaseFunction();
	~CKxBaseFunction();


private:
	kxCImageBuf    m_ImgHSV;
	kxCImageBuf    m_ImgBGR;
	kxCImageBuf    m_ImgLAB;
	kxCImageBuf    m_TempImg;
	kxCImageBuf    m_WriteTemp;
	kxCImageBuf    m_ImgTemp[3]; 
	kxCImageBuf    m_ImgHighThresh;
	kxCImageBuf    m_ImgLowThresh;
	kxCImageBuf    m_ImgGray;
	kxCImageBuf    m_ImgRotate;
	kxCImageBuf    m_ImgSave;
	//kxImg32f       m_CorrImage32f;  //求相似度存中间变量
	//kxImg32f       m_Result32f;
	kxCImageBuf       m_CorrImage32f;  //求相似度存中间变量
	kxCImageBuf       m_Result32f;
	kxCImageBuf       m_SplitImg[3];
	kxCImageBuf       m_SplitTpl[3];
	kxCImageBuf       m_PyramidBigImg;
	kxCImageBuf       m_PyramidSmallImg;
	kxCImageBuf       m_NormBigImg;
	kxCImageBuf       m_NormSmallImg;
	kxCImageBuf       m_NormBigImgLittle;
	//SobelFilter
	kxCImageBuf   m_ImageFilter;
	kxCImageBuf   m_ImageConvet;
	kxCImageBuf   m_ImageTemp;

	//GeneralFilter
	kxCImageBuf   m_Image16s;
	kxCImageBuf   m_Image16sAbs;
	cv::Mat       m_Mat16s;
	cv::Mat       m_Mat16sAbs;

	//ZhuGuangCheck
	kxCImageBuf   m_Img16s, m_ImgLapFilter, m_ImgDilate, m_ImgResize;

	//FFT
	kxImg<Ipp32f> m_ImgCopy32f, m_ImgFFt, m_ImgFFtFilter, m_ImgFFtResult;
	kxImg<Ipp32fc> m_ImgFilter32fc;

	//FilterSpeck
	kxCImageBuf  m_MarkImg;

	//GaussThresh
	kxCImageBuf  m_ImgGaussGray;


protected:
	//bool SaveBMPImage(const char* path, MV_IMAGE* bmpImg);
	//bool SaveBMPImage_h(const char* path, MV_IMAGE* bmpImg);

	int  KxResizeNearestImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall);
	int  KxResizeLinearImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall);
	int  KxResizeCubicImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall);
	int  KxResizeLanczosImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall);
	int  KxResizeSuperImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, KxCallStatus& hCall);

public:
	//区域图像(C1/C3/C4)拷贝函数
	int KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel,
		unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
		const kxRect<int>& rc);

	int KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel, 
		            unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
		            const kxRect<int>& rc, KxCallStatus& hCall);
	//区域图像(C1/C3/C4)拷贝函数
	int KxCopyImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, const kxRect<int>& rc, KxCallStatus& hCall);
	int KxCopyImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, const kxRect<int>& rc);
	//图像(C1/C3)相似度定位函数
	float kxImageAlign(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	float kxImageAlign( kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall) ;
	//图像(C1/C3)相似度定位函数
	//nPyramidLayer 金字塔压缩匹配为加速
	//nPyramidLayer = 1 不压缩
	//nPyramidLayer = 2 压缩一倍
	//nPyramidLayer = 3 压缩两倍
	float kxImageAlign(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer, KxCallStatus& hCall);
	float kxImageAlign(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer = 1);
	float kxImageAlignBySSD(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer, KxCallStatus & hCall);
	float kxImageAlignBySSD(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg );

	float kxImageAlignBySSD(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall);
	float kxImageAlignBySSD(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	//彩色图像定位
	float kxImageAlignColor(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	float kxImageAlignColor(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall);	
	float kxImageAlignColor(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg,
		float fcompressFactorx = 2.0, float fcompressFactory = 2.0);

	//图像(C1/C3)一个定位核的定位过程
	//输入：SrcImg 搜索图像  KernImg定位核图像
	//      rcKern         定位核区域  
	//      nSearchExpand  扩充范围   nSearchDir 搜索方向
	//输出：图像的偏移 nDx, nDy
	int GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg,  kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall);
	int GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg, kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy);

	//对图像（C1/C3)进行形态学操作
	int KxErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);
	
	int KxDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall);
	int KxDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);
   
	int KxOpenImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxOpenImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);

	int KxCloseImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxCloseImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);


	//对图像(C1/C3)进行层面转化
	//黑白图直接拷贝
	//彩色图则按照参数实现层面的转化
	int KxConvertImageLayer(const unsigned char* srcBuf, int srcPitch, int nImgType, 
		                    unsigned char* dstBuf, int dstPitch, int width, int height, int nChangeType, KxCallStatus& hCall);
	int KxConvertImageLayer(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nChangeType, KxCallStatus& hCall);
	int KxConvertImageLayer(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nChangeType);

    //从网络中读取或者写入图像
	int readImgBufFromMemory(kxCImageBuf& imgBuf, unsigned char *&point);
	int writeImgBufToMemory(kxCImageBuf& imgBuf, unsigned char *&point);
    
	//将图像转化为字符串
	std::string FormatImageToString(const kxCImageBuf& hImageBuffer);
	std::string FormatIntToString(const int& n);

    //读取Bmp图像
	//path       为待读取图片的位置
	//readImg    为读取图像结果体
	//hCall      为记录函数调用状态
	int LoadBMPImage_h(const char* path, kxCImageBuf& readImg, KxCallStatus& hCall);
	int LoadBMPImage_h(const char* path, kxCImageBuf& readImg);
	//MV_IMAGE* LoadBMPImage_h(const char* path);
	//void FreeImage_h(MV_IMAGE * img);

    //保存Bmp图像
	//writeImg        为待存图像
	//path            为存图路径
	bool SaveBMPImage_h(const char* path, const kxCImageBuf& writeImg, KxCallStatus& hCall);
	bool SaveBMPImage_h(const char* path, const kxCImageBuf& writeImg);
	bool SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);	
	bool SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel);

	//取两幅图像(C1/C3)对应位置的最大值
	//SrcImg        为源图像1
	//SrcDstImg     为源图像2、目标图像
	int KxMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall);
	int KxMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg);
    //取两幅图像(C1/C3)对应位置的最小值
	//SrcImg        为源图像1
	//SrcDstImg     为源图像2、目标图像
	int KxMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall);
	int KxMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg);

	//变换图像大小
	//SrcImg    为输入图像   nInterpolationMode 插值方式
	//ResizeImg 为变换后的图像
	//注：调用前申请好图像内存
	//  	KxNearest = 0,
	//      KxLinear = 1,
	//	    KxCubic = 2,
	//	    KxLanczos = 3,
	//	    KxSuper = 4
	int KxResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode, KxCallStatus& hCall);
	int KxResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode = KxLinear);
	
	//图像均值滤波,边界采用保留原始图像方式
	//SrcImg     为输入图像
	//DstImg     为滤波后的图像  
	//nMaskWidth 为卷积核宽
	//nMaskHeight为卷积核高
	int KxAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,  Ipp16s* pMask, KxCallStatus& hCall);
	int KxAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);
	int KxAverageFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, int* pMask, KxCallStatus& hCall);
	int KxAverageFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, cv::Mat Mask, KxCallStatus& hCall);

	//图像中值滤波,边界采用保留原始图像方式
	//SrcImg     为源图像
	//DstImg     为目标图像 
	//nMaskWidth 为卷积核宽
	//nMaskHeight为卷积核高
	int KxMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall);
	int KxMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight);

	//图像滤波
	//保留负值
	//SrcImg     为源图像
	//DstImg     为目标图像 
	//nMaskWidth 为卷积核宽
	//nMaskHeight为卷积核高
	//pMask      为卷积核
	//scale      为除以系数
	//8u->16s->abs(16s)->8u
	int KxGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, Ipp16s scale, KxCallStatus& hCall);
	int KxGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL, Ipp16s scale = 1);
	int KxGeneralFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, int* pMask, int scale, KxCallStatus& hCall);

	//8位图像滤波
	//忽略负值
	//SrcImg     为源图像
	//DstImg     为目标图像 
	//nMaskWidth 为卷积核宽
	//nMaskHeight为卷积核高
	//pMask      为卷积核
	int KxGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall);
	int KxGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	//二值化图像
	//SrcImg		为输入图像
	//nLowThresh	为低阈值
	//nHighThresh   为高阈值
	//nThreshLayer  为二值化图像层面
	//DstImg		为滤波后的图像  
	//【nLowThresh， nHighThresh】
	int KxThreshImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nLowThresh, int nHighThresh, int nThreshLayer, KxCallStatus& hCall);
	int KxThreshImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nLowThresh, int nHighThresh, int nThreshLayer = RGB_R);

	//多项式拟合
	//pX，pY 为拟合数据
	//nDotCount 为点数
	//nPoly 为拟合多项式的阶数
	//pCoeff 为计算的结果 y = pCoeff[0]*x^(nPoly)+pCoeff[1]*x^(nPoly-1)+……
	//pCoeff的大小为 nPoly+1
	int KxPolyFit(Ipp32f* pX, Ipp32f* pY, int nDotCount, int nPoly, Ipp32f* pCoeff, KxCallStatus& hCall);
	int KxPolyFit(Ipp32f* pX, Ipp32f* pY, int nDotCount, int nPoly, Ipp32f* pCoeff);

	//n点最小二乘法直线拟合 （n >= 3）
	//方程形式为：ax + by + c = 0
	//对应系数 pCoeff[0], pCoeff[1], pCoeff[2]
	int KxFitLine(Ipp32f* pX, Ipp32f* pY, int nDotCount, Ipp32f* pCoeff, KxCallStatus& hCall);
	int KxFitLine(Ipp32f* pX, Ipp32f* pY, int nDotCount, Ipp32f* pCoeff);
	//图像旋转特殊角度，为0度，90度，180度，270度
	//SrcImg            为输入图像
	//DstImg            为目标图像
	//nAngle            为输入角度，取值为
	//                  0   ------ _Angle_0
	//                  90  ------ _Angle_90
	//                  180 ------ _Angle_180
	//                  270 ------ _Angle_270
	int KxRotateImageSpecialAngle(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg,  int nAngle, KxCallStatus& hCall);
	int KxRotateImageSpecialAngle(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nAngle);

	//采用线性插值的透视投影变换的图像四角校正
	//SrcImg 为输入图像
	//DstImg 为目标图像
	//warpSrcQuad 为源四角
	//warpDstQuad 为目标四角
	int KxWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//采用线性插值的仿射变换的图像四角校正
	//SrcImg 为输入图像
	//DstImg 为目标图像
	//warpSrcQuad 为源四角
	//warpDstQuad 为目标四角
	int KxWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//采用双线性插值的双线性变换的图像四角校正
	//SrcImg 为输入图像
	//DstImg 为目标图像
	//warpSrcQuad 为源四角
	//warpDstQuad 为目标四角
	int KxWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);


    //图像Sobel滤波
	//SrcImg    为源图像
	//DstImg    为目标图像
	//nMaskSize 为滤波尺寸 参数为_3X3 /_5X5
	int KxFilterSobelImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskSize, KxCallStatus& hCall);
	int KxFilterSobelImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskSize);

	//统计图像的灰度直方图
	//SrcImg 为源图像
	//nBins  为分块数
	//nHist  为直方图统计
	//注：彩色图像的直方图输出一个层面一个层面放,nHist大小为3*nBins
	int  KxHistogram( const kxCImageBuf& SrcImg, int nBins, Ipp32u nHist[], KxCallStatus& hCall);
	int  KxHistogram(const kxCImageBuf& SrcImg, int nBins, Ipp32u nHist[]);

	//水平垂直投影
	//SrcImg    为源图像,单通道图像
	//nDir      为投影方向，_Horizontal_Project_Dir = 1 代表水平，_Vertical_Project_Dir   = 0 代表垂直
	//fProject  为计算结果指针，内存需要预先分配
	int KxProjectImage(const kxCImageBuf& SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale, KxCallStatus& hCall);
	int KxProjectImage(const kxCImageBuf& SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale = 1.0f);

	//两幅图像相加
	//SrcImg    为源图像1
	//DstImg    为源图像2、目标图像
	int KxAddImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxAddImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	//图像减一个常数
	//SrcImg    为源图像1
	//DstImg    为目标图像
	//value     为待减常数值
	int KxSubCImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Ipp8u value, KxCallStatus& hCall);
	int KxSubCImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Ipp8u value);


	//将一张彩色图像对应位置求最大值转换为一张黑白图， 黑白图直接拷贝操作
	//SrcImg    为源图像
	//DstImg    为目的图像，为单通道图像
	int KxImageMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxImageMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	//珠光检查预处理函数(黑白图像)
	//SrcImg    为源图像
	//DstImg    为目的图像，为单通道图像
	int KxZhuGuangCardFilter(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxZhuGuangCardFilter(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);


	//傅里叶变换,周期性条纹检查相关函数
	int KxFFtCheckPeriodic(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxFFtCheckPeriodic(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg);
	int KxFFtCheckPeriodic1(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxFFtCheckPeriodic1(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg);


	//滤除小于指定点数的邻通域
	//只支持单通道
	int KxFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, KxCallStatus& hCall);
	int KxFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize = 1);

	//填补空洞
	int KxFillHoles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxFillHoles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);


	bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3], KxCallStatus& hCall);
	bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3]);

#ifdef OPENCV
	//kxIamgeBuf转换opencv数据结构Mat
	int KxImageBufToMat(const kxCImageBuf& SrcImg, cv::Mat& mat, bool bCopy = false);
	//opencv数据结构Mat转换kxIamgeBuf
	int MatToKxImageBuf(const cv::Mat& mat, kxCImageBuf& SrcImg, bool bCopy = true);
#endif

	//自适应高斯局部二值化
	//SrcImg    为源图像
	//DstImg    为目的图像，为单通道图像
	//mskSize   为局部二值窗口大小
	//nDelta    为偏移
	//nThreshLayer 为二值层面，彩色图片有效
	//黑字白背景效果好
	int KxThresholdAdaptiveGauss(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiSize mskSize, int nDelta, int nThreshLayer, KxCallStatus& hCall);
	int KxThresholdAdaptiveGauss(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiSize mskSize, int nDelta = 10, int nThreshLayer = RGB_GRAY);


	//图像灰度取反
	//SrcImg    为源图像
	//DstImg    为目的图像
	int KxInvertImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxInvertImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);


	//并行算法模块
protected:
	//并行处理函数
	int  KxParallelResizeNearestImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeLinearImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeCubicImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeLanczosImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeSuperImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);

public:
	//采用并行算法线性插值的仿射变换的图像四角校正
	//SrcImg 为输入图像
	//DstImg 为目标图像
	//warpSrcQuad 为源四角
	//warpDstQuad 为目标四角
	int KxParalleWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//采用并行算法线性插值的透视投影变换的图像四角校正
	//SrcImg 为输入图像
	//DstImg 为目标图像
	//warpSrcQuad 为源四角
	//warpDstQuad 为目标四角
	int KxParalleWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);


	////采用并行算法线性插值的双线性变换的图像四角校正
	////SrcImg 为输入图像
	////DstImg 为目标图像
	////warpSrcQuad 为源四角
	////warpDstQuad 为目标四角
	int KxParalleWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);
	int GetBilinearTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4]);
	int GetBilinearTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4], KxCallStatus& hCall);

	//并行计算一般图像滤波
	//SrcImg    为输入图像
	//DstImg    为滤波后的图像 
	//8u->16s->abs(16s)->8u
	int KxParallelGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	int KxParallelGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	//并行计算，图像均值滤波,边界采用保留原始图像方式
	//SrcImg    为输入图像
	//DstImg    为滤波后的图像  
	int KxParallelAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);


	//并行计算，图像均值滤波,边界采用保留原始图像方式
	//SrcImg    为输入图像
	//DstImg    为滤波后的图像  
	int KxParallelBoxFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelBoxFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight);


	//并行计算，图像中值滤波,边界采用保留原始图像方式
	//SrcImg    为输入图像
	//DstImg    为滤波后的图像  
	int KxParallelMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight);

	//并行计算变换图像大小
	//SrcImg    为输入图像   nInterpolationMode 插值方式
	//ResizeImg 为变换后的图像
	//注：调用前申请好图像内存
	int KxParallelResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode = KxLinear);

	//并行计算对图像（C1/C3)进行形态学操作
	int KxParallelErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);

	int KxParallelDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);

	int KxParallelMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall);
	int KxParallelMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg);
	int KxParallelMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall);
	int KxParallelMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg);

	int KxParallelCopy(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall);
	int KxParallelCopy(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg);
};


#endif