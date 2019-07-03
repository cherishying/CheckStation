#ifndef _KXBASEFUNCTIONHHHH
#define _KXBASEFUNCTIONHHHH

//add by lyl 2016/6/6
//�Ƚϳ��õĻ�����������һ��
//1.C1/C3���ƶȶ�λ
//2.C1/C3/C4��ͼ�����򿽱�
//3.��ͼ/ȡͼ����
//4. C1/C3ͼ��Ĳ�ͬ�ߴ�ĸ�ʴ���Ͳ���
//5. C1/C3ͼ�����ת��
//6. ����ת����ͼ��ת��Ϊ�ַ���������ת��Ϊ�ַ����ȣ�
//7. C1/C3��Сֵ�����ֵ
//8. C1/C3һ����λ�˼���ͼ��ƫ��
//9. C1/C3ͼ��Resize����

/*
	date: 2019.06.29
	description: change to opencv struct, abandon kxImagebuf
*/

//#include "kxDef.h"
//#include "gxMath.h"
#include "KxCommonDef.h"
#include "ippcc.h"
#include "ippi.h"
#include "ipps.h"
#include "mkl.h"
#include <sstream>

#include <opencv2/opencv.hpp>
#include "opencv2/features2d/features2d.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/calib3d/calib3d.hpp" 
#include "opencv2/imgproc/imgproc_c.h" 
#include "opencv2/imgproc/imgproc.hpp"   

using namespace cv;

class CKxBaseFunction
{
public:
	CKxBaseFunction();
	~CKxBaseFunction();


private:
	Mat    m_ImgHSV;
	Mat    m_ImgBGR;
	Mat    m_ImgLAB;
	Mat    m_TempImg;
	Mat    m_WriteTemp;
	Mat    m_ImgTemp[3];
	Mat    m_ImgHighThresh;
	Mat    m_ImgLowThresh;
	Mat    m_ImgGray;
	Mat    m_ImgRotate;
	Mat    m_ImgSave;
	Mat       m_CorrImage32f;  //�����ƶȴ��м����
	Mat       m_Result32f;
	Mat       m_SplitImg[3];
	Mat       m_SplitTpl[3];
	Mat       m_PyramidBigImg;
	Mat       m_PyramidSmallImg;
	Mat       m_NormBigImg;
	Mat       m_NormSmallImg;
	Mat       m_NormBigImgLittle;
	//SobelFilter
	Mat   m_ImageFilter;
	Mat   m_ImageConvet;
	Mat   m_ImageTemp;

	//GeneralFilter
	Mat   m_Image16s;
	Mat   m_Image16sAbs;
	cv::Mat       m_Mat16s;
	cv::Mat       m_Mat16sAbs;
	//ZhuGuangCheck
	Mat   m_Img16s, m_ImgLapFilter, m_ImgDilate, m_ImgResize;

	//FFT
//	kxImg<Ipp32f> m_ImgCopy32f, m_ImgFFt, m_ImgFFtFilter, m_ImgFFtResult;
//	kxImg<Ipp32fc> m_ImgFilter32fc;

	//FilterSpeck
	Mat  m_MarkImg;

	//GaussThresh
	Mat  m_ImgGaussGray;


protected:
	//bool SaveBMPImage(const char* path, MV_IMAGE* bmpImg);
	//bool SaveBMPImage_h(const char* path, MV_IMAGE* bmpImg);

	int  KxResizeNearestImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int  KxResizeLinearImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int  KxResizeCubicImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int  KxResizeLanczosImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int  KxResizeSuperImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);

public:
	//����ͼ��(C1/C3/C4)��������
	int KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel,
		unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
		const Rect& rc);

	int KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel, 
		            unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
		            const Rect& rc, KxCallStatus& hCall);
	//����ͼ��(C1/C3/C4)��������
	int KxCopyImage(InputArray SrcImg, OutputArray DstImg, const Rect& rc, KxCallStatus& hCall);
	int KxCopyImage(InputArray SrcImg, OutputArray DstImg, const Rect& rc);
	//ͼ��(C1/C3)���ƶȶ�λ����
	float kxImageAlign(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	float kxImageAlign( Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall) ;
	//ͼ��(C1/C3)���ƶȶ�λ����
	//nPyramidLayer ������ѹ��ƥ��Ϊ����
	//nPyramidLayer = 1 ��ѹ��
	//nPyramidLayer = 2 ѹ��һ��
	//nPyramidLayer = 3 ѹ������
	float kxImageAlign(Point2f& pos, InputArray BigImg, InputArray SamllImg, int nPyramidLayer, KxCallStatus& hCall);
	float kxImageAlign(Point2f& pos, InputArray BigImg, InputArray SamllImg, int nPyramidLayer = 1);
	float kxImageAlignBySSD(Point2f& pos, InputArray BigImg, InputArray SamllImg, int nPyramidLayer, KxCallStatus & hCall);
	float kxImageAlignBySSD(Point2f& pos, InputArray BigImg, InputArray SamllImg );

	float kxImageAlignBySSD(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall);
	float kxImageAlignBySSD(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	//��ɫͼ��λ
	float kxImageAlignColor(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	float kxImageAlignColor(Point2f& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall);	
	float kxImageAlignColor(Point2f& pos, InputArray BigImg, InputArray SmallImg,
		float fcompressFactorx = 2.0, float fcompressFactory = 2.0);

	//ͼ��(C1/C3)һ����λ�˵Ķ�λ����
	//���룺SrcImg ����ͼ��  KernImg��λ��ͼ��
	//      rcKern         ��λ������  
	//      nSearchExpand  ���䷶Χ   nSearchDir ��������
	//�����ͼ���ƫ�� nDx, nDy
	int GetImgOffset(InputArray SrcImg, InputArray KernImg,  Rect& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall);
	int GetImgOffset(InputArray SrcImg, InputArray KernImg, Rect& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy);

	//��ͼ��C1/C3)������̬ѧ����
	int KxErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);
	
	int KxDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall);
	int KxDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);
   
	int KxOpenImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxOpenImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);

	int KxCloseImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxCloseImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);


	//��ͼ��(C1/C3)���в���ת��
	//�ڰ�ͼֱ�ӿ���
	//��ɫͼ���ղ���ʵ�ֲ����ת��
	int KxConvertImageLayer(const unsigned char* srcBuf, int srcPitch, int nImgType, 
		                    unsigned char* dstBuf, int dstPitch, int width, int height, int nChangeType, KxCallStatus& hCall);
	int KxConvertImageLayer(InputArray SrcImg, OutputArray DstImg, int nChangeType, KxCallStatus& hCall);
	int KxConvertImageLayer(InputArray SrcImg, OutputArray DstImg, int nChangeType);

    //�������ж�ȡ����д��ͼ��
	int readImgBufFromMemory(InputArray imgBuf, unsigned char *&point);
	int writeImgBufToMemory(InputArray imgBuf, unsigned char *&point);
    
	//��ͼ��ת��Ϊ�ַ���
	std::string FormatImageToString(InputArray hImageBuffer);
	std::string FormatIntToString(const int& n);

    //��ȡBmpͼ��
	//path       Ϊ����ȡͼƬ��λ��
	//readImg    Ϊ��ȡͼ������
	//hCall      Ϊ��¼��������״̬
	int LoadBMPImage_h(const char* path, InputArray readImg, KxCallStatus& hCall);
	int LoadBMPImage_h(const char* path, InputArray readImg);
	//MV_IMAGE* LoadBMPImage_h(const char* path);
	//void FreeImage_h(MV_IMAGE * img);

    //����Bmpͼ��
	//writeImg        Ϊ����ͼ��
	//path            Ϊ��ͼ·��
	bool SaveBMPImage_h(const char* path, InputArray writeImg, KxCallStatus& hCall);
	bool SaveBMPImage_h(const char* path, InputArray writeImg);
	bool SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);	
	bool SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel);

	//ȡ����ͼ��(C1/C3)��Ӧλ�õ����ֵ
	//SrcImg        ΪԴͼ��1
	//SrcDstImg     ΪԴͼ��2��Ŀ��ͼ��
	int KxMaxEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall);
	int KxMaxEvery(InputArray SrcImg, OutputArray SrcDstImg);
    //ȡ����ͼ��(C1/C3)��Ӧλ�õ���Сֵ
	//SrcImg        ΪԴͼ��1
	//SrcDstImg     ΪԴͼ��2��Ŀ��ͼ��
	int KxMinEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall);
	int KxMinEvery(InputArray SrcImg, OutputArray SrcDstImg);

	//�任ͼ���С
	//SrcImg    Ϊ����ͼ��   nInterpolationMode ��ֵ��ʽ
	//ResizeImg Ϊ�任���ͼ��
	//ע������ǰ�����ͼ���ڴ�
	//  	KxNearest = 0,
	//      KxLinear = 1,
	//	    KxCubic = 2,
	//	    KxLanczos = 3,
	//	    KxSuper = 4
	int KxResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode, KxCallStatus& hCall);
	int KxResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode = KxLinear);
	
	//ͼ���ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg     Ϊ����ͼ��
	//DstImg     Ϊ�˲����ͼ��  
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	int KxAverageFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, int* pMask, KxCallStatus& hCall);
	int KxAverageFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, cv::Mat Mask, KxCallStatus& hCall);

	//ͼ����ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg     ΪԴͼ��
	//DstImg     ΪĿ��ͼ�� 
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	int KxMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall);
	int KxMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight);

	//ͼ���˲�
	//������ֵ
	//SrcImg     ΪԴͼ��
	//DstImg     ΪĿ��ͼ�� 
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	//pMask      Ϊ�����
	//scale      Ϊ����ϵ��
	//8u->16s->abs(16s)->8u
	int KxGeneralFilterImage(cv::InputArray SrcImg, cv::OutputArray DstImg, int nMaskWidth, int nMaskHeight, int* pMask, int scale, KxCallStatus& hCall);
	//8λͼ���˲�
	//���Ը�ֵ
	//SrcImg     ΪԴͼ��
	//DstImg     ΪĿ��ͼ�� 
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	//pMask      Ϊ�����
	int KxGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall);
	int KxGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	//��ֵ��ͼ��
	//SrcImg		Ϊ����ͼ��
	//nLowThresh	Ϊ����ֵ
	//nHighThresh   Ϊ����ֵ
	//nThreshLayer  Ϊ��ֵ��ͼ�����
	//DstImg		Ϊ�˲����ͼ��  
	//��nLowThresh�� nHighThresh��
	int KxThreshImage(InputArray SrcImg, OutputArray DstImg, int nLowThresh, int nHighThresh, int nThreshLayer, KxCallStatus& hCall);
	int KxThreshImage(InputArray SrcImg, OutputArray DstImg, int nLowThresh, int nHighThresh, int nThreshLayer = RGB_R);

	//����ʽ���
	//pX��pY Ϊ�������
	//nDotCount Ϊ����
	//nPoly Ϊ��϶���ʽ�Ľ���
	//pCoeff Ϊ����Ľ�� y = pCoeff[0]*x^(nPoly)+pCoeff[1]*x^(nPoly-1)+����
	//pCoeff�Ĵ�СΪ nPoly+1
	int KxPolyFit(Ipp32f* pX, Ipp32f* pY, int nDotCount, int nPoly, Ipp32f* pCoeff, KxCallStatus& hCall);
	int KxPolyFit(Ipp32f* pX, Ipp32f* pY, int nDotCount, int nPoly, Ipp32f* pCoeff);

	//n����С���˷�ֱ����� ��n >= 3��
	//������ʽΪ��ax + by + c = 0
	//��Ӧϵ�� pCoeff[0], pCoeff[1], pCoeff[2]
	int KxFitLine(Ipp32f* pX, Ipp32f* pY, int nDotCount, Ipp32f* pCoeff, KxCallStatus& hCall);
	int KxFitLine(Ipp32f* pX, Ipp32f* pY, int nDotCount, Ipp32f* pCoeff);
	//ͼ����ת����Ƕȣ�Ϊ0�ȣ�90�ȣ�180�ȣ�270��
	//SrcImg            Ϊ����ͼ��
	//DstImg            ΪĿ��ͼ��
	//nAngle            Ϊ����Ƕȣ�ȡֵΪ
	//                  0   ------ _Angle_0
	//                  90  ------ _Angle_90
	//                  180 ------ _Angle_180
	//                  270 ------ _Angle_270
	int KxRotateImageSpecialAngle(InputArray SrcImg, OutputArray DstImg,  int nAngle, KxCallStatus& hCall);
	int KxRotateImageSpecialAngle(InputArray SrcImg, OutputArray DstImg, int nAngle);

	//�������Բ�ֵ��͸��ͶӰ�任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//�������Բ�ֵ�ķ���任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//����˫���Բ�ֵ��˫���Ա任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);


    //ͼ��Sobel�˲�
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��
	//nMaskSize Ϊ�˲��ߴ� ����Ϊ_3X3 /_5X5
	int KxFilterSobelImage(InputArray SrcImg, OutputArray DstImg, int nMaskSize, KxCallStatus& hCall);
	int KxFilterSobelImage(InputArray SrcImg, OutputArray DstImg, int nMaskSize);

	//ͳ��ͼ��ĻҶ�ֱ��ͼ
	//SrcImg ΪԴͼ��
	//nBins  Ϊ�ֿ���
	//nHist  Ϊֱ��ͼͳ��
	//ע����ɫͼ���ֱ��ͼ���һ������һ�������,nHist��СΪ3*nBins
	int  KxHistogram( InputArray SrcImg, int nBins, Ipp32u nHist[], KxCallStatus& hCall);
	int  KxHistogram(InputArray SrcImg, int nBins, Ipp32u nHist[]);

	//ˮƽ��ֱͶӰ
	//SrcImg    ΪԴͼ��,��ͨ��ͼ��
	//nDir      ΪͶӰ����_Horizontal_Project_Dir = 1 ����ˮƽ��_Vertical_Project_Dir   = 0 ����ֱ
	//fProject  Ϊ������ָ�룬�ڴ���ҪԤ�ȷ���
	int KxProjectImage(InputArray SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale, KxCallStatus& hCall);
	int KxProjectImage(InputArray SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale = 1.0f);

	//����ͼ�����
	//SrcImg    ΪԴͼ��1
	//DstImg    ΪԴͼ��2��Ŀ��ͼ��
	int KxAddImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int KxAddImage(InputArray SrcImg, OutputArray DstImg);

	//ͼ���һ������
	//SrcImg    ΪԴͼ��1
	//DstImg    ΪĿ��ͼ��
	//value     Ϊ��������ֵ
	int KxSubCImage(InputArray SrcImg, OutputArray DstImg, Ipp8u value, KxCallStatus& hCall);
	int KxSubCImage(InputArray SrcImg, OutputArray DstImg, Ipp8u value);


	//��һ�Ų�ɫͼ���Ӧλ�������ֵת��Ϊһ�źڰ�ͼ�� �ڰ�ͼֱ�ӿ�������
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��Ϊ��ͨ��ͼ��
	int KxImageMaxEvery(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int KxImageMaxEvery(InputArray SrcImg, OutputArray DstImg);

	//�����Ԥ������(�ڰ�ͼ��)
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��Ϊ��ͨ��ͼ��
	int KxZhuGuangCardFilter(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int KxZhuGuangCardFilter(InputArray SrcImg, OutputArray DstImg);


	//����Ҷ�任,���������Ƽ����غ���
	int KxFFtCheckPeriodic(InputArray & SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int KxFFtCheckPeriodic(InputArray & SrcImg, OutputArray DstImg);
	int KxFFtCheckPeriodic1(InputArray & SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int KxFFtCheckPeriodic1(InputArray & SrcImg, OutputArray DstImg);


	//�˳�С��ָ����������ͨ��
	//ֻ֧�ֵ�ͨ��
	int KxFilterSpeckles(InputArray SrcImg, OutputArray DstImg, int nMaxSpeckleSize, KxCallStatus& hCall);
	int KxFilterSpeckles(InputArray SrcImg, OutputArray DstImg, int nMaxSpeckleSize = 1);

	//��ն�
	int KxFillHoles(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int KxFillHoles(InputArray SrcImg, OutputArray DstImg);


	bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3], KxCallStatus& hCall);
	bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3]);

#ifdef OPENCV
	//kxIamgeBufת��opencv���ݽṹMat
	int KxImageBufToMat(InputArray SrcImg, cv::Mat& mat, bool bCopy = false);
	//opencv���ݽṹMatת��kxIamgeBuf
	int MatToKxImageBuf(const cv::Mat& mat, InputArray SrcImg, bool bCopy = true);
#endif

	//����Ӧ��˹�ֲ���ֵ��
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��Ϊ��ͨ��ͼ��
	//mskSize   Ϊ�ֲ���ֵ���ڴ�С
	//nDelta    Ϊƫ��
	//nThreshLayer Ϊ��ֵ���棬��ɫͼƬ��Ч
	//���ְױ���Ч����
	int KxThresholdAdaptiveGauss(InputArray SrcImg, OutputArray DstImg, IppiSize mskSize, int nDelta, int nThreshLayer, KxCallStatus& hCall);
	int KxThresholdAdaptiveGauss(InputArray SrcImg, OutputArray DstImg, IppiSize mskSize, int nDelta = 10, int nThreshLayer = RGB_GRAY);


	//ͼ��Ҷ�ȡ��
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��
	int KxInvertImage(InputArray SrcImg, OutputArray DstImg, KxCallStatus& hCall);
	int KxInvertImage(InputArray SrcImg, OutputArray DstImg);


	//�����㷨ģ��
protected:
	//���д�����
	int  KxParallelResizeNearestImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeLinearImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeCubicImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeLanczosImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeSuperImage(InputArray SrcImg, OutputArray DstImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);

public:
	//���ò����㷨���Բ�ֵ�ķ���任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxParalleWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpAffineLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//���ò����㷨���Բ�ֵ��͸��ͶӰ�任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxParalleWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpPerspectiveLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);


	////���ò����㷨���Բ�ֵ��˫���Ա任��ͼ���Ľ�У��
	////SrcImg Ϊ����ͼ��
	////DstImg ΪĿ��ͼ��
	////warpSrcQuad ΪԴ�Ľ�
	////warpDstQuad ΪĿ���Ľ�
	int KxParalleWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpBilinearLinearQuadImgae(InputArray SrcImg, OutputArray DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);
	int GetBilinearTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4]);
	int GetBilinearTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4], KxCallStatus& hCall);

	//���м���һ��ͼ���˲�
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ�� 
	//8u->16s->abs(16s)->8u
	int KxParallelGeneralFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelGeneralFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	int KxParallelGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelGeneralFilterImage8u(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	//���м��㣬ͼ���ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ��  
	int KxParallelAverageFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelAverageFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);


	//���м��㣬ͼ���ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ��  
	int KxParallelBoxFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelBoxFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight);


	//���м��㣬ͼ����ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ��  
	int KxParallelMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelMedianFilterImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight);

	//���м���任ͼ���С
	//SrcImg    Ϊ����ͼ��   nInterpolationMode ��ֵ��ʽ
	//ResizeImg Ϊ�任���ͼ��
	//ע������ǰ�����ͼ���ڴ�
	int KxParallelResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelResizeImage(InputArray SrcImg, OutputArray DstImg, int nInterpolationMode = KxLinear);

	//���м����ͼ��C1/C3)������̬ѧ����
	int KxParallelErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelErodeImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);

	int KxParallelDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelDilateImage(InputArray SrcImg, OutputArray DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);

	int KxParallelMaxEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall);
	int KxParallelMaxEvery(InputArray SrcImg, OutputArray SrcDstImg);
	int KxParallelMinEvery(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall);
	int KxParallelMinEvery(InputArray SrcImg, OutputArray SrcDstImg);

	int KxParallelCopy(InputArray SrcImg, OutputArray SrcDstImg, KxCallStatus& hCall);
	int KxParallelCopy(InputArray SrcImg, OutputArray SrcDstImg);
};


#endif