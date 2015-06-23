// D3DDisplay.h: interface for the CD3DDisplay class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_D3DDISPLAY_H__D2B5461A_AF44_411A_8FEA_8439E0FB5628__INCLUDED_)
#define AFX_D3DDISPLAY_H__D2B5461A_AF44_411A_8FEA_8439E0FB5628__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Windows.h>
#include "..\D3DDisplay\\include\d3d9.h"
#include "..\D3DDisplay\include\d3dx9core.h"
#include <string>


#define MAX_VIDEOWIDTH 1920
#define MAX_VIDEOHEIGHT 1088

#define	MSG_THREADQUITE	 WM_USER + 5001
#define	MSG_DISPLAY_FRAME	 WM_USER + 5002

#define MIDDLE(a,b,c)  ((a>b?a:b)<c?(a>b?a:b):c)	//得到中间大小的变量


//typedef struct 
//{
//	std::string stringByName1;
//	std::string stringByName2;
//	std::string stringByName3;
//	std::string stringByName4;
//	int			iFontProperity;
//}SubTitleProp;


typedef enum _SubTitlePlace
{
	Left_Up = 0,
	Left_Down = 1,
	Right_Up = 2,
	Right_Down = 3,
}SubTitlePlace;
typedef enum
{
	BigFont = 0,
	FitFont = 1,
	SmallFont = 2,
}SubTitleSize;
const int MAXSUBTITLELENGHT = 32;
class CD3DDisplay  
{
public:

	CD3DDisplay();
	virtual ~CD3DDisplay();
	//BOOL SetVideoSize(long lWidth,long lHeight, HWND hWnd, int &nRGBorYUV);
	//void SetYUVbuffer(LPBYTE pBuffer,long lLen);
	//void SetRGBbuffer(LPBYTE pSampleBuffer);
	void DrawImage();
	bool YUV2RGB(unsigned char * yuv, unsigned char * rgb, const int nWidth, const int nHeight, const int nBits);
	void YUV2RGB(unsigned char *src0,unsigned char *src1,unsigned char *src2,unsigned char *dst_ori,
		int width,int height);
	void InitConvertTable();

//	bool InitDraw(HWND hWnd, GUID *guid = NULL, DWORD posx = 0);
	bool InitDraw(HWND hWnd, int iMutilDisplay,DWORD dwPosx=0);
	void StartPlay();
	void StopPlay();
	void FreeDraw();
	void ReInitDraw(HWND hWnd, int iMutilDisplay);

	BOOL InitD3DFont(/*std::string strSubTitle*/);

	//bool SetSubTitleProp(unsigned char uFontProp,std::string strSplitName1,std::string strSplitName2,
	//	std::string strSplitName3,std::string strSplitName4);
	bool SetSubTitleProp(unsigned char uFontProp,const char* strSplitName,int iLen);
	bool SetVideoParam(int iWidth, int iHeight);
	void GetVideoParam(int *iWidth, int *iHeight);
	void SetOutBuffer(unsigned char * pOutBuffer, CRITICAL_SECTION *csLockYUV);
	void SetDisplayFlag(bool bDisplay);
	bool DisplayFunc(unsigned char* pVideo);
	bool DisplayRGBFunc(unsigned char* pSampleBuffer);
	bool SetDisplayScale(bool bVideoScale);
	bool SetSubtitleScope(int xLeft,int yTop,int xRight,int yBottom);
	bool IsUsed();
	void SetUsed(bool bUsed);
private:
	//int m_nWidth;
	//int m_nHeight;//保存视频宽高信息
	IDirect3D9 * m_pD3D;
	IDirect3DDevice9 * m_pd3dDevice;
	IDirect3DSurface9 * m_pd3dSurface;//D3D绘图用变量
	DWORD m_dwStartTime;  //帧率计算
	DWORD m_dwTotalFrame; //总帧数
	
	LPD3DXFONT m_pd3dFont;
	HFONT m_textFONT;

	int m_iDisplayID;
//	RECT m_rtViewport;//视频显示
	//HWND m_PresentWnd;
	//YUV===>RGB
	long int crv_tab[256];
	long int cbu_tab[256];
	long int cgu_tab[256];
	long int cgv_tab[256];
	long int tab_76309[256];
	unsigned char clp[1024];
	int m_nRGBorYUV;//用YUV还是RGB
	unsigned char *m_pRgbbuf;
	bool m_bUsed;
	bool m_bVideoScale; //Fase为窗口比例，True为视频比例。
	RECT m_rtViewPort;

private:
	static unsigned __stdcall DisplayThread(LPVOID pPara);

	HWND m_hWnd;
	unsigned m_ThreadId;
	bool m_bDisplayFlag;

	SubTitlePlace m_FontPlace;
	SubTitleSize m_FontSize;
	bool m_bIsClarity;
	std::string m_strSubTitle;
	//char m_strSubTitle[1024];

	unsigned char * m_pOutBuffer;
	int m_xLen;
	int m_yLen;
	int m_posx;
	RECT    m_rcRectSrc;  

	CRITICAL_SECTION m_csDDraw;

};

#endif // !defined(AFX_D3DDISPLAY_H__D2B5461A_AF44_411A_8FEA_8439E0FB5628__INCLUDED_)
