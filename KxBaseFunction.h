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

#include "kxDef.h"
#include "gxMath.h"
#include "ippcc.h"
#include "ippi.h"
#include "ipps.h"
//#include "ippm.h"
#include "mkl.h"
#include <sstream>

#ifdef OPENCV
#include <opencv2/opencv.hpp>
#include "opencv2/features2d/features2d.hpp" 
#include "opencv2/highgui/highgui.hpp" 
#include "opencv2/calib3d/calib3d.hpp" 
#include "opencv2/imgproc/imgproc_c.h" 
#include "opencv2/imgproc/imgproc.hpp"   
#endif



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
	//kxImg32f       m_CorrImage32f;  //�����ƶȴ��м����
	//kxImg32f       m_Result32f;
	kxCImageBuf       m_CorrImage32f;  //�����ƶȴ��м����
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
	//����ͼ��(C1/C3/C4)��������
	int KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel,
		unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
		const kxRect<int>& rc);

	int KxCopyImage(const unsigned char* pSrc, int nSrcWidth, int nSrcHeight, int nSrcPitch, int nSrcChannel, 
		            unsigned char* pDst, int nDstWidth, int nDstHeight, int nDstPitch, int nDstChannel,
		            const kxRect<int>& rc, KxCallStatus& hCall);
	//����ͼ��(C1/C3/C4)��������
	int KxCopyImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, const kxRect<int>& rc, KxCallStatus& hCall);
	int KxCopyImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, const kxRect<int>& rc);
	//ͼ��(C1/C3)���ƶȶ�λ����
	float kxImageAlign(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	float kxImageAlign( kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall) ;
	//ͼ��(C1/C3)���ƶȶ�λ����
	//nPyramidLayer ������ѹ��ƥ��Ϊ����
	//nPyramidLayer = 1 ��ѹ��
	//nPyramidLayer = 2 ѹ��һ��
	//nPyramidLayer = 3 ѹ������
	float kxImageAlign(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer, KxCallStatus& hCall);
	float kxImageAlign(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer = 1);
	float kxImageAlignBySSD(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg, int nPyramidLayer, KxCallStatus & hCall);
	float kxImageAlignBySSD(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg );

	float kxImageAlignBySSD(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall);
	float kxImageAlignBySSD(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	//��ɫͼ��λ
	float kxImageAlignColor(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType);
	float kxImageAlignColor(kxPoint<float>& pos, const unsigned char* srcBuf, int sw, int sh, int ss, const unsigned char* tplBuf, int tw, int th, int ts, int nImgType, KxCallStatus& hCall);	
	float kxImageAlignColor(kxPoint<float>& pos, const kxCImageBuf& BigImg, const kxCImageBuf& SamllImg,
		float fcompressFactorx = 2.0, float fcompressFactory = 2.0);

	//ͼ��(C1/C3)һ����λ�˵Ķ�λ����
	//���룺SrcImg ����ͼ��  KernImg��λ��ͼ��
	//      rcKern         ��λ������  
	//      nSearchExpand  ���䷶Χ   nSearchDir ��������
	//�����ͼ���ƫ�� nDx, nDy
	int GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg,  kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy, KxCallStatus& hCall);
	int GetImgOffset(const kxCImageBuf& SrcImg, const kxCImageBuf& KernImg, kxRect<int>& rcKern, int nSearchExpand, int nSearchDir, int& nDx, int& nDy);

	//��ͼ��C1/C3)������̬ѧ����
	int KxErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxErodeImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);
	
	int KxDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, unsigned char* pMask, KxCallStatus& hCall);
	int KxDilateImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);
   
	int KxOpenImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxOpenImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);

	int KxCloseImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,   unsigned char* pMask, KxCallStatus& hCall);
	int KxCloseImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth = 3, int nMaskHeight = 3, unsigned char* pMask = NULL);


	//��ͼ��(C1/C3)���в���ת��
	//�ڰ�ͼֱ�ӿ���
	//��ɫͼ���ղ���ʵ�ֲ����ת��
	int KxConvertImageLayer(const unsigned char* srcBuf, int srcPitch, int nImgType, 
		                    unsigned char* dstBuf, int dstPitch, int width, int height, int nChangeType, KxCallStatus& hCall);
	int KxConvertImageLayer(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nChangeType, KxCallStatus& hCall);
	int KxConvertImageLayer(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nChangeType);

    //�������ж�ȡ����д��ͼ��
	int readImgBufFromMemory(kxCImageBuf& imgBuf, unsigned char *&point);
	int writeImgBufToMemory(kxCImageBuf& imgBuf, unsigned char *&point);
    
	//��ͼ��ת��Ϊ�ַ���
	std::string FormatImageToString(const kxCImageBuf& hImageBuffer);
	std::string FormatIntToString(const int& n);

    //��ȡBmpͼ��
	//path       Ϊ����ȡͼƬ��λ��
	//readImg    Ϊ��ȡͼ������
	//hCall      Ϊ��¼��������״̬
	int LoadBMPImage_h(const char* path, kxCImageBuf& readImg, KxCallStatus& hCall);
	int LoadBMPImage_h(const char* path, kxCImageBuf& readImg);
	//MV_IMAGE* LoadBMPImage_h(const char* path);
	//void FreeImage_h(MV_IMAGE * img);

    //����Bmpͼ��
	//writeImg        Ϊ����ͼ��
	//path            Ϊ��ͼ·��
	bool SaveBMPImage_h(const char* path, const kxCImageBuf& writeImg, KxCallStatus& hCall);
	bool SaveBMPImage_h(const char* path, const kxCImageBuf& writeImg);
	bool SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel, KxCallStatus& hCall);	
	bool SaveBMPImage_h(const char* file, const unsigned char* pSrc, int nWidth, int nHeight, int nPitch, int nChannel);

	//ȡ����ͼ��(C1/C3)��Ӧλ�õ����ֵ
	//SrcImg        ΪԴͼ��1
	//SrcDstImg     ΪԴͼ��2��Ŀ��ͼ��
	int KxMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall);
	int KxMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg);
    //ȡ����ͼ��(C1/C3)��Ӧλ�õ���Сֵ
	//SrcImg        ΪԴͼ��1
	//SrcDstImg     ΪԴͼ��2��Ŀ��ͼ��
	int KxMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg, KxCallStatus& hCall);
	int KxMinEvery(const kxCImageBuf& SrcImg, kxCImageBuf& SrcDstImg);

	//�任ͼ���С
	//SrcImg    Ϊ����ͼ��   nInterpolationMode ��ֵ��ʽ
	//ResizeImg Ϊ�任���ͼ��
	//ע������ǰ�����ͼ���ڴ�
	//  	KxNearest = 0,
	//      KxLinear = 1,
	//	    KxCubic = 2,
	//	    KxLanczos = 3,
	//	    KxSuper = 4
	int KxResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode, KxCallStatus& hCall);
	int KxResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode = KxLinear);
	
	//ͼ���ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg     Ϊ����ͼ��
	//DstImg     Ϊ�˲����ͼ��  
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	int KxAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight,  Ipp16s* pMask, KxCallStatus& hCall);
	int KxAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	//ͼ����ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg     ΪԴͼ��
	//DstImg     ΪĿ��ͼ�� 
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	int KxMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall);
	int KxMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight);

	//ͼ���˲�
	//������ֵ
	//SrcImg     ΪԴͼ��
	//DstImg     ΪĿ��ͼ�� 
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	//pMask      Ϊ�����
	//scale      Ϊ����ϵ��
	//8u->16s->abs(16s)->8u
	int KxGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, Ipp16s scale, KxCallStatus& hCall);
	int KxGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL, Ipp16s scale = 1);

	//8λͼ���˲�
	//���Ը�ֵ
	//SrcImg     ΪԴͼ��
	//DstImg     ΪĿ��ͼ�� 
	//nMaskWidth Ϊ����˿�
	//nMaskHeightΪ����˸�
	//pMask      Ϊ�����
	int KxGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall);
	int KxGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	//��ֵ��ͼ��
	//SrcImg		Ϊ����ͼ��
	//nLowThresh	Ϊ����ֵ
	//nHighThresh   Ϊ����ֵ
	//nThreshLayer  Ϊ��ֵ��ͼ�����
	//DstImg		Ϊ�˲����ͼ��  
	//��nLowThresh�� nHighThresh��
	int KxThreshImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nLowThresh, int nHighThresh, int nThreshLayer, KxCallStatus& hCall);
	int KxThreshImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nLowThresh, int nHighThresh, int nThreshLayer = RGB_R);

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
	int KxRotateImageSpecialAngle(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg,  int nAngle, KxCallStatus& hCall);
	int KxRotateImageSpecialAngle(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nAngle);

	//�������Բ�ֵ��͸��ͶӰ�任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//�������Բ�ֵ�ķ���任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//����˫���Բ�ֵ��˫���Ա任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall);
	int KxWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);


    //ͼ��Sobel�˲�
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��
	//nMaskSize Ϊ�˲��ߴ� ����Ϊ_3X3 /_5X5
	int KxFilterSobelImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskSize, KxCallStatus& hCall);
	int KxFilterSobelImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskSize);

	//ͳ��ͼ��ĻҶ�ֱ��ͼ
	//SrcImg ΪԴͼ��
	//nBins  Ϊ�ֿ���
	//nHist  Ϊֱ��ͼͳ��
	//ע����ɫͼ���ֱ��ͼ���һ������һ�������,nHist��СΪ3*nBins
	int  KxHistogram( const kxCImageBuf& SrcImg, int nBins, Ipp32u nHist[], KxCallStatus& hCall);
	int  KxHistogram(const kxCImageBuf& SrcImg, int nBins, Ipp32u nHist[]);

	//ˮƽ��ֱͶӰ
	//SrcImg    ΪԴͼ��,��ͨ��ͼ��
	//nDir      ΪͶӰ����_Horizontal_Project_Dir = 1 ����ˮƽ��_Vertical_Project_Dir   = 0 ����ֱ
	//fProject  Ϊ������ָ�룬�ڴ���ҪԤ�ȷ���
	int KxProjectImage(const kxCImageBuf& SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale, KxCallStatus& hCall);
	int KxProjectImage(const kxCImageBuf& SrcImg, int nDir, Ipp32f* fProject, Ipp32f fScale = 1.0f);

	//����ͼ�����
	//SrcImg    ΪԴͼ��1
	//DstImg    ΪԴͼ��2��Ŀ��ͼ��
	int KxAddImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxAddImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	//ͼ���һ������
	//SrcImg    ΪԴͼ��1
	//DstImg    ΪĿ��ͼ��
	//value     Ϊ��������ֵ
	int KxSubCImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Ipp8u value, KxCallStatus& hCall);
	int KxSubCImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, Ipp8u value);


	//��һ�Ų�ɫͼ���Ӧλ�������ֵת��Ϊһ�źڰ�ͼ�� �ڰ�ͼֱ�ӿ�������
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��Ϊ��ͨ��ͼ��
	int KxImageMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxImageMaxEvery(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);

	//�����Ԥ������(�ڰ�ͼ��)
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��Ϊ��ͨ��ͼ��
	int KxZhuGuangCardFilter(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxZhuGuangCardFilter(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);


	//����Ҷ�任,���������Ƽ����غ���
	int KxFFtCheckPeriodic(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxFFtCheckPeriodic(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg);
	int KxFFtCheckPeriodic1(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxFFtCheckPeriodic1(const kxCImageBuf & SrcImg, kxCImageBuf& DstImg);


	//�˳�С��ָ����������ͨ��
	//ֻ֧�ֵ�ͨ��
	int KxFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize, KxCallStatus& hCall);
	int KxFilterSpeckles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaxSpeckleSize = 1);

	//��ն�
	int KxFillHoles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxFillHoles(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);


	bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3], KxCallStatus& hCall);
	bool GetPerspectiveTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[3][3]);

#ifdef OPENCV
	//kxIamgeBufת��opencv���ݽṹMat
	int KxImageBufToMat(const kxCImageBuf& SrcImg, cv::Mat& mat, bool bCopy = false);
	//opencv���ݽṹMatת��kxIamgeBuf
	int MatToKxImageBuf(const cv::Mat& mat, kxCImageBuf& SrcImg, bool bCopy = true);
#endif

	//����Ӧ��˹�ֲ���ֵ��
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��Ϊ��ͨ��ͼ��
	//mskSize   Ϊ�ֲ���ֵ���ڴ�С
	//nDelta    Ϊƫ��
	//nThreshLayer Ϊ��ֵ���棬��ɫͼƬ��Ч
	//���ְױ���Ч����
	int KxThresholdAdaptiveGauss(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiSize mskSize, int nDelta, int nThreshLayer, KxCallStatus& hCall);
	int KxThresholdAdaptiveGauss(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, IppiSize mskSize, int nDelta = 10, int nThreshLayer = RGB_GRAY);


	//ͼ��Ҷ�ȡ��
	//SrcImg    ΪԴͼ��
	//DstImg    ΪĿ��ͼ��
	int KxInvertImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, KxCallStatus& hCall);
	int KxInvertImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg);


	//�����㷨ģ��
protected:
	//���д�����
	int  KxParallelResizeNearestImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeLinearImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeCubicImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeLanczosImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);
	int  KxParallelResizeSuperImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nRowGrain, int nColGrain, KxCallStatus& hCall);

public:
	//���ò����㷨���Բ�ֵ�ķ���任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxParalleWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpAffineLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);

	//���ò����㷨���Բ�ֵ��͸��ͶӰ�任��ͼ���Ľ�У��
	//SrcImg Ϊ����ͼ��
	//DstImg ΪĿ��ͼ��
	//warpSrcQuad ΪԴ�Ľ�
	//warpDstQuad ΪĿ���Ľ�
	int KxParalleWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpPerspectiveLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);


	////���ò����㷨���Բ�ֵ��˫���Ա任��ͼ���Ľ�У��
	////SrcImg Ϊ����ͼ��
	////DstImg ΪĿ��ͼ��
	////warpSrcQuad ΪԴ�Ľ�
	////warpDstQuad ΪĿ���Ľ�
	int KxParalleWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2], KxCallStatus& hCall, int nRowGrain = 100, int nColGrain = 100);
	int KxParalleWarpBilinearLinearQuadImgae(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, double warpSrcQuad[4][2], double warpDstQuad[4][2]);
	int GetBilinearTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4]);
	int GetBilinearTransform(const double srcQuad[4][2], const double dstQuad[4][2], double coeffs[2][4], KxCallStatus& hCall);

	//���м���һ��ͼ���˲�
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ�� 
	//8u->16s->abs(16s)->8u
	int KxParallelGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelGeneralFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	int KxParallelGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelGeneralFilterImage8u(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);

	//���м��㣬ͼ���ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ��  
	int KxParallelAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelAverageFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, Ipp16s* pMask = NULL);


	//���м��㣬ͼ���ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ��  
	int KxParallelBoxFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelBoxFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight);


	//���м��㣬ͼ����ֵ�˲�,�߽���ñ���ԭʼͼ��ʽ
	//SrcImg    Ϊ����ͼ��
	//DstImg    Ϊ�˲����ͼ��  
	int KxParallelMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight, KxCallStatus& hCall, int nRowGrain = 120, int nColGrain = 120);
	int KxParallelMedianFilterImage(const kxCImageBuf& SrcImg, kxCImageBuf& DstImg, int nMaskWidth, int nMaskHeight);

	//���м���任ͼ���С
	//SrcImg    Ϊ����ͼ��   nInterpolationMode ��ֵ��ʽ
	//ResizeImg Ϊ�任���ͼ��
	//ע������ǰ�����ͼ���ڴ�
	int KxParallelResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode, KxCallStatus& hCall, int nRowGrain = 280, int nColGrain = 280);
	int KxParallelResizeImage(const kxCImageBuf& SrcImg, kxCImageBuf& ResizeImg, int nInterpolationMode = KxLinear);

	//���м����ͼ��C1/C3)������̬ѧ����
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