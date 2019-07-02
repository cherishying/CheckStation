#ifndef _kxMATHHHHHHH_
#define _kxMATHHHHHHH_

#include <string>
#include <list>
#include <vector>
#include <cassert>
#include <float.h>
#include <math.h>
//using namespace std;

#include "ipps.h"
#include "ippi.h"
#include "ippcv.h"
#include "ippcc.h"
#include "mkl.h"
#include "KxCommonDef.h"

#pragma comment(lib, "mkl_intel_lp64.lib")
#pragma comment(lib, "mkl_intel_thread.lib")
#pragma comment(lib, "mkl_core.lib")
#pragma comment(lib, "libiomp5md.lib")
#pragma comment(lib, "mkl_intel_ilp64.lib")

/** 
...Ippnew...
*/
/*! \brief Brief description.
* Brief description continued.
*
* Detailed description starts here.
*/
	//#define OPENCV

	#ifndef PI
	#define PI   3.14159265358979323846
	#endif

	//------kxINT64
	#if defined( _WIN32 ) || defined ( _WIN64 )
	  #define kxINT64     __int64
	  #define kxUINT64    unsigned __int64
	#else
	  #define kxINT64  long long
	  #define kxUINT64 unsigned long long
	#endif

	#ifndef BOOL
	//#define BOOL int
	typedef int BOOL;
	#endif;

	#ifndef TRUE
	#define TRUE  1
	#endif
	#ifndef FALSE
	#define FALSE  0
	#endif

	#ifndef NULL
	#define NULL  0
	#endif

	//#define  _USEIPPMEM

	#ifdef ippNew
	#undef ippNew
	#endif

	#ifdef _USEIPPMEM
	#define   ippNew(x)       (unsigned char*)ippsMalloc_8u(x)
	#else
	#define   ippNew(x)       new unsigned char[x]
	#endif

	#ifdef ippDelete
	#undef ippDelete
	#endif

	#ifdef _USEIPPMEM
	#define   ippDelete(x)    ippsFree(x)
	#else
	#define   ippDelete(x)    delete [](x)
	#endif

	#define    x8Width( x )    (((x)*8+31)/32*4)
	#define    x24Width( x )   (((x)*24+31)/32*4)

	#ifdef gMin
	#undef gMin
	#endif
	#define gMin( x, y )        ((x) < (y) ? (x):(y))

	#ifdef gMax
	#undef gMax
	#endif
	#define gMax( x, y )        ((x) > (y) ? (x):(y))

	enum
	{
		_Version1 = 0,
		_Version2,
		_Version3,
		_Version4,
		_Version5,
		_Version6,
		_Version7,
		_Version8,
		_Version9,
		_Version10,
		_Version11,
		_Version12,
		_Version13,
		_Version14,
		_Version15,
		_Version16,
		_Version17,
		_Version18,
		_Version19,
	};
	enum
	{
		_Bit0 = 1,
		_Bit1 = 2,
		_Bit2 = 4,
		_Bit3 = 8,
		_Bit4 = 32,
		_Bit5 = 64,
		_Bit6 = 128,
		_Bit7 = 256,
		_Bit8 = 512,
		_Bit9 = 1024,
		_Bit10 = 2048,
		_Bit11 = 4096,
		_Bit12 = 8192,
		_Bit13 = (2*_Bit12),
	};

	#pragma pack(push, 1)
	//----��ע�� ��������� kxPOINT û�й��캯���� ��� ������union�� ��ʹ�� kxPOINT kxRECTl
	//----------------------------------------------------------------------------------------------
	template< class T >
	struct kxPoint
	{
		T x;
		T y;
		kxPoint()
		{
		}
		kxPoint( T x1, T y1 ):x(x1),y(y1)
		{
		}
		T GetX() const { return x; }
		T GetY() const { return y; }
		void setup( T x1, T y1 )
		{
			x = x1;
			y = y1;
		}
		kxPoint<T>& operator = ( const kxPoint<int>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPoint<T>& operator = ( const kxPoint<float>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPoint<T>& operator = ( const kxPoint<double>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPoint<T>& operator = ( const kxPoint<long>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPoint<T>& operator = ( const kxPoint<short>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}




	};
	//-------------------------------------------------------------------------------------------
	template< class T >
	struct kxPOINT
	{
		T x;
		T y;
		T GetX() const { return x; }
		T GetY() const { return y; }
		void setup( T x1, T y1 )
		{
			x = x1;
			y = y1;
		}
		kxPOINT<T>& operator = ( const kxPOINT<int>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPOINT<T>& operator = ( const kxPOINT<float>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPOINT<T>& operator = ( const kxPOINT<double>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPOINT<T>& operator = ( const kxPOINT<long>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
		kxPOINT<T>& operator = ( const kxPOINT<short>& pt )
		{
			x = pt.x;
			y = pt.y;
			return *this;
		}
	};
    //------------------------------------------------------------------------------------------------------
	template< class T >
	struct kxRect
	{
		T left, top;
		T right, bottom;
		T Height() const;
		T Width() const;
		T Diagonal() const; //�Խ���
		kxRect()
		{
			left = top = right =bottom =0;
		}
		kxRect( T l, T t, T r, T b ):left(l),top(t),right(r),bottom(b)
		{
		}
		void setup( T l, T t, T r, T b )
		{
			left = l; top = t;
			right = r; bottom = b;
		}
		void offset( T dx, T dy )
		{
			left += dx;
			right += dx;
			top += dy;
			bottom += dy;
		}

		void mulC(T factor)
		{
			int w = Width() * factor;
			int h = Height() * factor;

			left *= factor;		
			top *= factor;
			right = left + w - 1;
			bottom = top + h - 1;

			//right *= factor;
			//bottom *= factor;
		}

		void mulC(T factorX, T factorY)
		{
			int w = Width() * factorX;
			int h = Height() * factorY;

			left *= factorX;
			top *= factorY;
			right = left + w - 1;
			bottom = top + h - 1;

			//right *= factor;
			//bottom *= factor;
		}


		void divC(T factor)
		{
			int w = Width() / factor;
			int h = Height() / factor;

			left /= factor;
			top /= factor;

			right = left + w - 1;
			bottom = top + h - 1;
			//right /= factor;
			//bottom /= factor;
		}

		void divC(T factorX, T factorY)
		{
			int w = Width() / factorX;
			int h = Height() / factorY;

			left /= factorX;
			top /= factorY;

			right = left + w - 1;
			bottom = top + h - 1;
			//right /= factor;
			//bottom /= factor;
		}


		bool Intersect( const kxRect<T>& t )
		{
			if ( left > t.right )
				return false;
			if ( right < t.left )
				return false;
			if ( top > t.bottom )
				return false;
			if ( bottom < t.top )
				return false;
			return true;
		}
		bool IsInclude(kxRect<T>& t)
		{
			if (t.left>=left&&t.top>=top&&t.right<=right&&t.bottom<=bottom)
			{
				return true;
			}
			return false;
		}
		T GetLeft() const { return left; }
		T GetTop() const { return top; }
		T GetRight() const { return right; }
		T GetBottom() const { return bottom; }
		void SetLeft( T l ) { left = l; }
		void SetTop( T t ) { top = t; }
		void SetRight( T r ) { right = r; }
		void SetBottom( T b ) { bottom = b; }

		kxRect<T>& operator = ( const kxRect<int>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRect<T>& operator = ( const kxRect<float>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRect<T>& operator = ( const kxRect<double>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRect<T>& operator = ( const kxRect<long>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRect<T>& operator = ( const kxRect<short>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
	};

	//---------------------------------------------------------------------------------------
	template< class T >
	struct kxRECT
	{
		T left, top;
		T right, bottom;
		T Height() const;
		T Width() const;
		T Diagonal() const; //�Խ���
		void setup( T l, T t, T r, T b )
		{
			left = l; top = t;
			right = r; bottom = b;
		}
		void offset( T dx, T dy )
		{
			left += dx;
			right += dx;
			top += dy;
			bottom += dy;
		}
		bool Intersect( const kxRECT<T>& t )
		{
			if ( left > t.right )
				return false;
			if ( right < t.left )
				return false;
			if ( top > t.bottom )
				return false;
			if ( bottom < t.top )
				return false;
			return true;
		}
		T GetLeft() const { return left; }
		T GetTop() const { return top; }
		T GetRight() const { return right; }
		T GetBottom() const { return bottom; }
		void SetLeft( T l ) { left = l; }
		void SetTop( T t ) { top = t; }
		void SetRight( T r ) { right = r; }
		void SetBottom( T b ) { bottom = b; }
		kxRECT<T>& operator = ( const kxRECT<int>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRECT<T>& operator = ( const kxRECT<float>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRECT<T>& operator = ( const kxRECT<double>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRECT<T>& operator = ( const kxRECT<long>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}
		kxRECT<T>& operator = ( const kxRECT<short>& rc )
		{
			left = rc.left;
			top = rc.top;
			right = rc.right;
			bottom = rc.bottom;
			return *this;
		}

	};

	template< class T >
	inline T kxRect<T>::Height() const { return bottom-top+1; }
	template< class T >
	inline T kxRect<T>::Width() const { return right-left+1; }
	template< class T >
	inline T kxRect<T>::Diagonal() const
	{
		return Width()*Width()+Height()*Height();
	}
	template< class T >
	inline T kxRECT<T>::Height() const { return bottom-top+1; }
	template< class T >
	inline T kxRECT<T>::Width() const { return right-left+1; }
	template< class T >
	inline T kxRECT<T>::Diagonal() const
	{
		return Width()*Width()+Height()*Height();
	}
	//------����֮�����
	template < class T >
	float GetDistanceTwoPt( const kxPoint<T>& pt1, const kxPoint<T>& pt2 )
	{
		float dx = pt1.x - pt2.x;
		float dy = pt1.y - pt2.y;
		return sqrt( dx*dx+dy*dy);
	}
	template < class T >
	float GetDistanceTwoPt( const kxPOINT<T>& pt1, const kxPOINT<T>& pt2 )
	{
		float dx = pt1.x - pt2.x;
		float dy = pt1.y - pt2.y;
		return sqrt( dx*dx+dy*dy);
	}

	//-----��/�� ͨ��ͼ�񻺳���
	//--------------------------------------------------------------------------------------
	struct kxCImageBuf
	{
		kxCImageBuf();
		kxCImageBuf( int width, int height, int nChannel = 1 );
		~kxCImageBuf();
		kxCImageBuf( const kxCImageBuf& Obj );
		kxCImageBuf& operator = ( const kxCImageBuf& Obj );
		int   nWidth;
		int   nHeight;
		int   nPitch;          //�ֽڵ�λ
		unsigned char*  buf;
		bool  bAuto;
		int   nChannel;        //1:��ͨ�� 3��R/G/Bͨ��  4: 4ͨ��

		void Release();
		//��ͼ�� pBuf width X height X pitch copy -> kxImageBuf .���bCopy = false�򣬲�Copy���ݣ� ֻ��ָ��ָ��pBuf
		void SetImageBuf( const unsigned char* pBuf, int width, int height, int pitch, int nChan, bool bCopy, KxCallStatus& hCall );
		void SetImageBuf( const unsigned char* pBuf, int width, int height, int pitch, int nChan, bool bCopy);
		unsigned char* GetImageBuf( int& width, int& height, int& pitch, int& channel) const;
		void Init( int width, int height, int nChan = 1 );
		BOOL Read( FILE* fp );
		BOOL Write( FILE* fp );
		int GetChannel() const { return nChannel; }

		unsigned char* GetBuf() const { return buf;	}
		int GetWidth() const { return nWidth; }
		int GetHeight() const {	return nHeight;	}
		int GetPitch() const { return nPitch; }
		void SetVal( int x, int y, unsigned char val ){	GetBuf()[y*GetPitch()+x] = val;	}
		unsigned char GetVal( int x, int y ) {	return GetBuf()[y*GetPitch()+x]; }
		void Clone( const kxCImageBuf& Obj );
		bool Empty() { return  nWidth == 0 || nHeight == 0 || nPitch == 0; }
	};
#define kxImageBuf  kxCImageBuf

	struct kxImg16u
	{
		kxImg16u();
		kxImg16u( int width, int height);
		~kxImg16u();
		int   nWidth;
		int   nHeight;
		int   nPitch;
		Ipp16u*  buf;
		bool  bAuto;
		void Release();
		void Init( int width, int height);
	};


	struct kxImg32f
	{
		kxImg32f();
		kxImg32f( int width, int height);
		~kxImg32f();
		int   nWidth;
		int   nHeight;
		int   nPitch;
		Ipp32f*  buf;
		bool  bAuto;
		void Release();
		void Init( int width, int height);

	};


	//ģ��ͼ����
	template< class T >
	struct kxImg
	{
		kxImg();
		kxImg(int width, int height);
		~kxImg();
		int   nWidth;
		int   nHeight;
		int   nPitch;
		T*  buf;
		bool  bAuto;
		void Release();
		void Init(int width, int height);
	};


	//ģ��ͼ����
	template<class T>
	kxImg<T>::kxImg()
	{
		buf = NULL;
		nWidth = 0;
		nHeight = 0;
		nPitch = 0;
		bAuto = false;
	}

	template<class T>
	kxImg<T>::kxImg(int width, int height)
	{
		buf = NULL;
		Init(width, height);
	}

	template<class T>
	kxImg<T>::~kxImg()
	{
		Release();
	}

	template<class T>
	void kxImg<T>::Release()
	{
		if (bAuto && buf)
		{
			delete[]buf;
			buf = NULL;
		}
		bAuto = false;
	}

	template<class T>
	void kxImg<T>::Init(int width, int height)
	{
		if (buf && nWidth == width && nHeight == height)
			return;
		Release();

		nWidth = width;
		nHeight = height;
		nPitch = nWidth * sizeof(T);
		bAuto = true;

		buf = new T[nWidth*nHeight];
	}










    //------------------------------------------------------------------------------------------------------------
    /*************************************************************************************************************
    kxCImageBuf    Obj;
	Obj.Init( .... );
	*************************************************************************************************************/
	class kxCImageExtract      //��ͼ�� ���任[ ֻ�Ե�ͨ�� ]
	{
	public:
		enum
		{
			_Type_NULL  = 0,    //�����任
			_Type_2     = 1,    //2�� ȡ 1
			_Type_4     = 2,    //4�� ȡ 1
			_Type_8     = 3,    //8�� ȡ 1
			_Type_16    = 4,   //16�� ȡ 1
			_Type_COUNT = 5,
		};

		kxCImageExtract()
		{
			m_pBuf = NULL;
			m_bAuto = FALSE;
		}
		~kxCImageExtract()
		{
			Release();
		}
		kxCImageExtract( const kxCImageExtract& );
		kxCImageExtract& operator = ( const kxCImageExtract& );

		static int GetExtractTypeCount() { return _Type_COUNT; }
		static std::string GetExtractTypeMsg(int nType);
		unsigned char* GetBuf() const { return m_pBuf; }
		unsigned char* GetBuf( int& nWidth, int& nHeight, int& nPitch ) const
		{
			nWidth = m_nWidth;
			nHeight = m_nHeight;
			nPitch = m_nPitch;
			return m_pBuf;
		}
		int GetWidth() const { return m_nWidth; }
		int GetHeight() const { return m_nHeight; }
		int GetPitch() const { return m_nPitch; }

		void Release();
		void GetExtractMsg( int nType, int nSrcWidth, int nSrcHeight, int nSrcPitch, int& nWidth, int& nHeight, int& nPitch );
		//���ݳ�����ͣ� ���� �����ͼ�� ��С��Ϣ
		void Extract( const unsigned char* buf, int nWidth, int nHeight, int nPitch, int nType, unsigned char* pTMP = NULL );
		//{���任
	private:
		unsigned char*   m_pBuf;
		int  m_nWidth;
		int  m_nHeight;
		int  m_nPitch;
		BOOL m_bAuto;
	};
	/**********************************************************************
	kxImageExtract     Obj;
	Obj.Extract( buf, w, h, pitch, _Type_8 );

	Obj.GetBuf(...);   //�õ������ͼ��
	//-- Ҳ����
	int w1,h1,pitch1;
	Obj.GetImageMsg( _Type_4, w, h, pitch, w1, h1, pitch1 );
	buf1 = new unsigned char[pitch1*h1];
	Obj.Extract( buf, w, h, pitch, _Type_8, buf1 );
	buf1;���� �����ͼ��

	��ע��_Type_NULL ����㣬 ֱ��ָ��Դͼ��
	***********************************************************************/

	//----------------------------------------------------------------------------
	//--����
	template< class T, int nSize >
	class kxCQue
	{
	public:
		kxCQue( const kxCQue& );
		kxCQue& operator =( const kxCQue& );
		kxCQue()
		{
			m_nFront = m_nRear = 0;
		}
		void Clear()
		{
			m_nFront = m_nRear;
		}
		bool IsEmpty() const
		{
			return m_nFront == m_nRear;
		}
		bool IsFull() const
		{
			return ((m_nRear + 1 )%nSize) == m_nFront;
		}
		T Top() { return m_t[m_nFront]; }
		const T Top() const { return m_t[m_nFront]; }

		void Push( const T& t )
		{
			m_t[m_nRear] = t;
			m_nRear = ( m_nRear + 1 )%nSize;
		}
		void Push()
		{
			m_nRear = ( m_nRear + 1 )%nSize;
		}
		void Pop()
		{
			m_nFront = ( m_nFront + 1 ) % nSize;
		}
	private:
		T      m_t[nSize];
		 kxUINT64   m_nFront, m_nRear;
	public:
		long GetFront() const
		{
			return m_nFront;
		}
		long GetRear() const
		{
			return m_nRear;
		}
		int  GetCount()
		{
			return ( m_nRear + nSize - m_nFront ) % nSize;
		}//���ض��е�ǰ ���ݸ���
		int  GetCapability() const { return nSize; }
		T& GetElement( int nIndex )   //ע�� ��Ӧ Front�� nIndex = 0
		{//-----�õ� ������ ��nIndex�����е�Ԫ�ص� ����
			return m_t[(nIndex + m_nFront)%nSize ];
		}
		const T& GetElement( int nIndex ) const  //ע�� ��Ӧ Front�� nIndex = 0
		{//-----�õ� ������ ��nIndex�����е�Ԫ�ص� ����
			return m_t[(nIndex + m_nFront)%nSize ];
		}
		T& GetRearElement() { return m_t[m_nRear]; }
		const T& GetRearElement() const{ return m_t[m_nRear]; }
		T& GetElementAt( int nIndex ) { return m_t[nIndex]; }
		const T& GetElementAt( int nIndex ) const { return m_t[nIndex]; }
	};
	//--------------------------------------------------------------------------------------------------------------------------
	//ͼ�����ݶ���
	class kxCImgQue
	{
	public:
		kxCImgQue( const kxCImgQue& );
		kxCImgQue& operator =( const kxCImgQue& );

		kxCImgQue()
		{
			m_pImg = NULL;
			m_nFront = m_nRear = 0;
			m_nSize = 0;
			m_nBlock = 0;
			m_nTag = 0;
		}
		virtual ~kxCImgQue()
		{
			if( m_pImg )
				//delete []m_pImg;
				free(m_pImg);
		}

		void Init( int nBlock, int nQueSize, int nTag = 1)   //nBlock������ͼ����С��nQueSize�����д�С��nQueSize��ͼ��顿
		{
			assert( nQueSize >=0 );
			m_nTag = nTag;
			if( m_pImg )
			{
				delete []m_pImg;
				m_pImg = NULL;
			}
			m_nFront = m_nRear = 0;

			if( nQueSize )
			{
				assert( nBlock > 0 );
				m_nSize = nQueSize;
				m_nBlock = nBlock;
				if( m_nTag )
					nBlock *= 2;
				m_pImg = new unsigned char[ (kxUINT64)nBlock * (kxUINT64)m_nSize ];
				//m_pImg = (unsigned char *)malloc((__int64)sizeof(unsigned char) * nBlock * m_nSize);
			}
		}
		virtual bool IsEmpty() const { return m_nFront == m_nRear; }
		virtual bool IsFull() const { return ((m_nRear + 1 )%m_nSize) == m_nFront; }
		virtual void Push( /*void* = NULL*/ )
		{
            if( m_nTag )
				CloneData();
			m_nRear = ( m_nRear + 1 )%m_nSize;
		}

		virtual void Push( const unsigned char* pBuf/*, void* = NULL*/ )
		{
			//memcpy( GetRearBuf(), pBuf, m_nBlock );
			ippsCopy_8u( pBuf, GetRearBuf(), int(m_nBlock) );

			if( m_nTag )
				CloneData();

			m_nRear = ( m_nRear + 1 )%m_nSize;
		}
		void CloneData()
		{
			ippsCopy_8u( GetRearBuf(), GetRearBuf() + GetCapability()*GetBlockSize(), int(m_nBlock) );
		}
		virtual void Pop() { m_nFront = ( m_nFront+1)%m_nSize; }

		virtual void Clear() { m_nFront = m_nRear = 0; 	}
		unsigned char* GetImgBuf() const { return m_pImg; }
		void MemInit( unsigned char ch )
		{
			assert( m_pImg );
			//memset( m_pImg, ch, m_nSize*m_nBlock );   // �ڴ��ʼ��
			ippsSet_8u( ch, m_pImg, int(m_nSize*m_nBlock) );
		}
	protected:
		unsigned char*   m_pImg;
	    kxINT64        m_nFront,  m_nRear;
		kxINT64        m_nBlock;  //---һ�����ݿ��С
		kxINT64        m_nSize;   //----���� ��С
		kxINT64   (*fPtr)( long, long );   //�ص������� ��Push(..)�� ���ܻ��
		int        m_nTag;
	public:
		bool  IsCloned() const { return m_nTag != 0; }
		 kxINT64 GetFront() const {	return m_nFront; }
		 kxINT64 GetRear() const { return m_nRear; }

		 kxINT64 GetBlockSize() const { return m_nBlock; }
		//void SetCallBackFun( long (*fun)( long, long ) )
		//{
		//	fPtr = fun;
		//}
		int  GetCount()
		{
			return int(( m_nRear + m_nSize - m_nFront ) % m_nSize);
		}//���ض��е�ǰ ���ݿ� ����

		int GetCapability() const { return int(m_nSize); }

		unsigned char* GetElement( long nIndex )   //ע�� ��Ӧ Front�� nIndex = 0
		{//-----�õ� ������ ��nIndex�����е�Ԫ�ص� ����
			return m_pImg + ( nIndex + m_nFront ) % m_nSize * (kxINT64)(m_nBlock);
		}
		unsigned char* GetElementAt( long nIndex )   //ע�� ��Ӧ ���п�ʼ
		{
			return m_pImg + ( nIndex ) % m_nSize * (kxINT64)(m_nBlock);
		}
		const unsigned char* GetElementAt( long nIndex ) const  //ע�� ��Ӧ ���п�ʼ
		{
			return m_pImg + ( nIndex ) % m_nSize * (kxINT64)(m_nBlock);
		}
		const unsigned char* GetElement( int nIndex ) const  //ע�� ��Ӧ Front�� nIndex = 0
		{//-----�õ� ������ ��nIndex�����е�Ԫ�ص� ����
			return m_pImg + (nIndex + m_nFront)%m_nSize * (kxINT64)(m_nBlock);
		}

		unsigned char* GetFrontBuf() { return m_pImg + m_nFront*m_nBlock; }
		const unsigned char* GetFrontBuf() const { return m_pImg + m_nFront*m_nBlock; }
		unsigned char* GetRearBuf()	{ return m_pImg + m_nRear*m_nBlock;	}
		const unsigned char* GetRearBuf() const { return m_pImg + m_nRear*m_nBlock;	}

		int GetElementSize() const { return int(m_nBlock); }
	};

	#pragma pack(pop)
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	template < class T > T GetMax( const T t[], long nLen, long& nPos )
	{
		T  tMax = t[0];
		nPos = 0;
		for( int i = 1; i < nLen; i++ )
		{
			if( tMax < t[i] )
			{
				tMax = t[i];
				nPos = i;
			}
		}
		return tMax;
	}
	template < class T > T GetMin( const T t[], long nLen, long& nPos )
	{
		T  tMin = t[0];
		nPos = 0;
		for( int i = 1; i < nLen; i++ )
		{
			if( tMin > t[i] )
			{
				tMin = t[i];
				nPos = i;
			}
		}
		return tMin;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class kxCDivRect
	{
	public:
		kxCDivRect()
		{
			m_pRect = NULL;
			m_Row = 0;
			m_Col = 0;
		}
		virtual ~kxCDivRect()
		{
			Release();
		}
		void Release();
		BOOL Check( int nWidth, int nHeight, int nRow, int nCol );
		//--�� nWidth X nHeight  �ֳ� nRow X nCol ������[ ���ȷֲ�  ]

		BOOL Check1( int nWidth, int nHeight, int nRow, int nCol );
		//--�� nWidth X nHeight  �ֳ� nRow X nCol ������[ ����ֲ�������к���  ]
	protected:
		kxRect<int>**  m_pRect;
		int   m_Row, m_Col;

	public:
		kxRect<int>* GetRect( int row, int col )
		{
			if( row < 0 || row > m_Row-1 || col < 0 || col > m_Col-1 )
				return NULL;
			return &(m_pRect[row][col]);
		}

	public:
		int GetRows() const { return m_Row; }
		int GetCols() const { return m_Col; }
		BOOL Read( FILE* );
		BOOL Write( FILE* );
	};




///////=======

#endif
