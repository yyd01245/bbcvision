#include "D3D9VideoPlayer.h"
#include <process.h>
#include <conio.h>
#include "..\ipplib\ipps.h"
#include <Windows.h>

//FILE *g_DisplayLog = NULL;
//FILE *g_fpDiaplayLog = NULL;
//FILE *g_fp = NULL;

CD3DDisplay::CD3DDisplay()
:m_bUsed(false)
{
	//m_nWidth = 0;
	//m_nHeight = 0;//保存视频宽高信息
	m_pD3D = NULL;
	m_pd3dDevice = NULL;
	m_pd3dSurface = NULL;//D3D绘图用变量
	m_pd3dFont = NULL;
	//InitConvertTable();
	//memset(m_strSubTitle,0,sizeof(m_strSubTitle));
	m_strSubTitle = "";
	m_bVideoScale = false;

	m_pRgbbuf = NULL;
	m_hWnd = NULL;
	m_xLen = 0;
	m_yLen = 0;
	m_pOutBuffer = NULL;
	m_bDisplayFlag = false;
	m_ThreadId = 0;
	InitializeCriticalSection(&m_csDDraw);
	m_dwStartTime = 0;
	//if(g_fpDiaplayLog == NULL)
	//{
	//	g_fpDiaplayLog = fopen("D:\\amplesky\\software_md\\web\\log\\Display.log","w+");
	//}
	//if(g_fp == NULL)
	//{
	//	g_fp = fopen("D:\\amplesky\\software_md\\web\\log\\DisplayTime.txt","w+");
	//}
}

CD3DDisplay::~CD3DDisplay()
{

}
// <summary>初始化DirectDraw</summary >
// <param name="hWnd">播放视频的窗口句柄 / IN</param>
// <returns>true表示成功，false表示失败</returns>
bool CD3DDisplay::InitDraw(HWND hWnd, int iMutilDisplay,DWORD dwPosx)
{
	EnterCriticalSection(&m_csDDraw);
	m_hWnd = hWnd;
	m_posx = dwPosx;

	if(m_pD3D != NULL)    
	{        
		m_pD3D->Release();        
		m_pD3D = NULL;   
	}    
	if(m_pd3dDevice != NULL)    
	{        
		m_pd3dDevice->Release();       
		m_pd3dDevice = NULL;    
	}    
	if(m_pd3dSurface != NULL)    
	{        
		m_pd3dSurface->Release();        
		m_pd3dSurface = NULL;   
	}   
	if(m_pd3dFont != NULL)
	{
		m_pd3dFont->Release();
		m_pd3dFont = NULL;
	}
	m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );    
	if( m_pD3D == NULL ) 
	{
		LeaveCriticalSection(&m_csDDraw);
		return FALSE;
	}
	m_iDisplayID = iMutilDisplay;

	LeaveCriticalSection(&m_csDDraw);
	return true;

}

bool CD3DDisplay::SetVideoParam(int lWidth, int lHeight)
{
	if(lWidth != m_xLen || lHeight != m_yLen || m_pd3dSurface == NULL)
	{
		EnterCriticalSection(&m_csDDraw);
		m_xLen = lWidth;
		m_yLen = lHeight;
		if(m_pD3D == NULL)
		{
			LeaveCriticalSection(&m_csDDraw);
			return false;
		}
		if(m_pd3dDevice != NULL)
		{
			m_pd3dDevice->Release();
			m_pd3dDevice = NULL;
		}
		D3DPRESENT_PARAMETERS d3dpp;    
		ZeroMemory( &d3dpp, sizeof(d3dpp) );   

		d3dpp.Windowed = TRUE;    
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;  
		if(!m_bVideoScale)
		{
		d3dpp.BackBufferHeight = lHeight;
		d3dpp.BackBufferWidth = lWidth;
		}
		d3dpp.PresentationInterval = D3DPRESENT_DONOTWAIT;
		d3dpp.BackBufferCount = 1;
		d3dpp.hDeviceWindow = m_hWnd;
			//char txt[128];
		if(m_pd3dDevice == NULL)
		{
			D3DCAPS9 cap9;
			m_pD3D->GetDeviceCaps(m_iDisplayID,D3DDEVTYPE_HAL/*|D3DDEVTYPE_REF*/,&cap9);
			D3DDEVTYPE CurrentType = D3DDEVTYPE_HAL;
			if(!(cap9.DeviceType & D3DDEVTYPE_HAL))
			{
				m_pD3D->GetDeviceCaps(m_iDisplayID,D3DDEVTYPE_REF,&cap9);
				if(cap9.DeviceType & D3DDEVTYPE_REF)
				{
					CurrentType = D3DDEVTYPE_REF;
				}

			}
			if( FAILED( m_pD3D->CreateDevice( m_iDisplayID, CurrentType, m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice ) ) )
			{

				LeaveCriticalSection(&m_csDDraw);
				return FALSE;

			}
		}
		if(m_pd3dSurface != NULL)
		{
			m_pd3dSurface->Release();
			m_pd3dSurface = NULL;
		}

		//if(m_pd3dDevice == NULL)
		//{
		//	LeaveCriticalSection(&m_csDDraw);
		//	return false;
		//}
		
		memset(&m_rcRectSrc,0,sizeof(m_rcRectSrc));
		SetRect(&m_rcRectSrc, m_xLen/40, m_yLen/44, m_xLen - m_xLen/40, m_yLen - m_yLen/44);
		InitD3DFont();
 		if( FAILED(m_pd3dDevice->CreateOffscreenPlainSurface(lWidth,lHeight, (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'), D3DPOOL_DEFAULT, &m_pd3dSurface, NULL)))   
 		{
			if( FAILED(m_pd3dDevice->CreateOffscreenPlainSurface(lWidth,lHeight, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_pd3dSurface, NULL))) 
			{
				LeaveCriticalSection(&m_csDDraw);
				return FALSE;
			}
			else
			{
				if(m_pRgbbuf == NULL)
				{
					m_pRgbbuf = new unsigned char[MAX_VIDEOWIDTH*MAX_VIDEOHEIGHT*4];
				}
				m_nRGBorYUV = 2;
			}
  		}
  		else
  		{
  			m_nRGBorYUV = 1;
  		}
		LeaveCriticalSection(&m_csDDraw);
	}

	if(m_ThreadId == 0)
	{
		StartPlay();
	}
	return true;
}

bool CD3DDisplay::DisplayFunc(unsigned char* pobuf)
{
	EnterCriticalSection(&m_csDDraw);
	if(m_pd3dSurface == NULL)
	{
		//_cprintf("diaplayfunc failed \n");
		LeaveCriticalSection(&m_csDDraw);
		return false;  
	}
	unsigned char *pVideo[3];
	pVideo[0] = pobuf;
	pVideo[1] = pobuf + m_xLen*m_yLen;
	pVideo[2] = pobuf + m_xLen*m_yLen*5/4;

	D3DLOCKED_RECT d3d_rect;    
	if( FAILED(m_pd3dSurface->LockRect(&d3d_rect,NULL,0)))
	{
		LeaveCriticalSection(&m_csDDraw);
		return false;  
	}

	const int w = m_xLen,h = m_yLen;   

	LPBYTE pOverlay = (LPBYTE)d3d_rect.pBits;

	LPBYTE lpSrcY = pVideo[0];
	LPBYTE lpSrcU = pVideo[1];
	LPBYTE lpSrcV = pVideo[2];

	LPBYTE lpDstY = pOverlay;
	LPBYTE lpDstV = pOverlay + d3d_rect.Pitch * m_yLen;
	LPBYTE lpDstU = pOverlay + d3d_rect.Pitch * m_yLen * 5 / 4;
 
	int i = 0; 
	// fill Y data
	for(int i=0; i<m_yLen; i++)
	{
		ippsCopy_64s((Ipp64s *)lpSrcY, (Ipp64s *)lpDstY, m_xLen / 8);
		lpSrcY += m_xLen;
		lpDstY += d3d_rect.Pitch;
	}
	// fill U data
	for(int i=0; i<m_yLen/2; i++)
	{
		ippsCopy_64s((Ipp64s *)lpSrcU, (Ipp64s *)lpDstU, m_xLen / 16);
		lpSrcU += m_xLen / 2;
		lpDstU += d3d_rect.Pitch / 2;
	}

	// fill V data
	for(int i=0; i<m_yLen/2; i++)
	{
		ippsCopy_64s((Ipp64s *)lpSrcV, (Ipp64s *)lpDstV, m_xLen / 16);
		lpSrcV += m_xLen / 2;
		lpDstV += d3d_rect.Pitch / 2;
	}

	if( FAILED(m_pd3dSurface->UnlockRect()))    
	{       
		LeaveCriticalSection(&m_csDDraw);
		return false;   
	}    
	DrawImage();
	LeaveCriticalSection(&m_csDDraw);
	return true;
}

void CD3DDisplay::DrawImage()
{    
	if (m_pd3dDevice != NULL)   
	{       
		//int itempTime = GetTickCount();

		m_pd3dDevice->Present( NULL, NULL, NULL, NULL );  

 		//int iTempTime2 = GetTickCount();
 		//if(iTempTime2 - itempTime > 35)
 		//{
 		//	char txt[20];
 		//	sprintf(txt,"Present Time = %d\n",iTempTime2-itempTime);
 		//	fwrite(txt,1,strlen(txt),g_fp);
 		//}

		m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 );     

		m_pd3dDevice->BeginScene();        
		IDirect3DSurface9 * pBackBuffer = NULL;       
		m_pd3dDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer); 

		if(m_bVideoScale)
		{
			RECT rtclient;
			GetClientRect(m_hWnd,&rtclient);
			m_rtViewPort    = rtclient;   
			double    dbAspect    = (double)m_xLen / m_yLen; 
			int iWidth = rtclient.right - rtclient.left;
			int iHeight = rtclient.bottom - rtclient.top;
			if(iWidth > iHeight * dbAspect)   
			{        
				//width lager than height,adjust the width        
				int                    nValidW(static_cast<int>(iHeight * dbAspect));       
				int                    nLost(iWidth - nValidW);      
				m_rtViewPort.left    += nLost / 2;   
				m_rtViewPort.right    = m_rtViewPort.left + nValidW;   
			}    
			else   
			{        
				//height lager than width,adjust the height        
				int                    nValidH(static_cast<int>(iWidth / dbAspect));        
				int                    nLost(iHeight - nValidH);       
				m_rtViewPort.top    += nLost / 2;       				
				m_rtViewPort.bottom    = m_rtViewPort.top + nValidH;   
			}  
			m_pd3dDevice->StretchRect(m_pd3dSurface,NULL,pBackBuffer,&m_rtViewPort,D3DTEXF_NONE);      

			if(m_strSubTitle != "" && m_pd3dFont != NULL)
			{
				DWORD dwFomat = 0;
				switch(m_FontPlace)
				{
				case Left_Up:
					{
						dwFomat = DT_LEFT | DT_TOP;
						//SetRect(&rect,10,10,)
					}
					break;
				case Left_Down:
					{
						dwFomat = DT_LEFT | DT_BOTTOM;
					}
					break;
				case Right_Up:
					{
						dwFomat = DT_RIGHT | DT_TOP;
					}
					break;
				case Right_Down:
					{
						dwFomat = DT_RIGHT | DT_BOTTOM;
					}
					break;
				default:
					break;
				}

				DWORD dwColor = m_bIsClarity ? 0xFFFFFFFF : 0xff0f0f0f;
				//LPCSTR
				int iRt = m_pd3dFont->DrawText(NULL,m_strSubTitle.c_str(),-1,&m_rtViewPort,dwFomat,dwColor);
			}

		}
		else
		{
		m_pd3dDevice->StretchRect(m_pd3dSurface,NULL,pBackBuffer,NULL,D3DTEXF_NONE);      

		if(m_strSubTitle != "" && m_pd3dFont != NULL)
		{
			DWORD dwFomat = 0;
			switch(m_FontPlace)
			{
			case Left_Up:
				{
					dwFomat = DT_LEFT | DT_TOP;
					//SetRect(&rect,10,10,)
				}
				break;
			case Left_Down:
				{
					dwFomat = DT_LEFT | DT_BOTTOM;
				}
				break;
			case Right_Up:
				{
					dwFomat = DT_RIGHT | DT_TOP;
				}
				break;
			case Right_Down:
				{
					dwFomat = DT_RIGHT | DT_BOTTOM;
				}
				break;
			default:
				break;
			}

			DWORD dwColor = m_bIsClarity ? 0xFFFFFFFF : 0xff0f0f0f;
			//LPCSTR
			int iRt = m_pd3dFont->DrawText(NULL,m_strSubTitle.c_str(),-1,&m_rcRectSrc,dwFomat,dwColor);
		}

		}
		
		//RECT rect = {100, 0, 300, 100};//Width,Height为窗口的宽高，可以自己设定想要的大小
		//D3DCOLOR color;
		//int iRet = m_pd3dFont->DrawText(NULL,  
		//	"Hello World\n2343534", // 要绘制的文字，可以是中文 
		//	-1, //字符串中特征字符的数量
		//	&rect, // 绘制文字的范围
		//	DT_TOP | DT_LEFT, // Draw in top-left corner of rect. 
		//	0xFFFF0000 //文字颜色
		//	);
		
		m_pd3dDevice->EndScene();  
 		//if(m_dwTotalFrame >= 100)
 		//{
 		//	double Totaltime = GetTickCount() - m_dwStartTime;
 		//	double iFps = 0;
 		//	iFps = m_dwTotalFrame / (Totaltime/1000);
 		//	//printf("fps = %2f \n",iFps);
 
 		//	if(iFps < 20)
 		//	{
 		//		char txt[20];
 		//		sprintf(txt,"fps = %2f\n",iFps);
 		//		fwrite(txt,1,strlen(txt),g_fpDiaplayLog);
 		//	}
 
 		//	m_dwTotalFrame = 0;
 		//	m_dwStartTime = GetTickCount();
 		//}
		pBackBuffer->Release();
			
	}
#ifdef _DEBUG
	else
	{
		_cprintf("Drawimage Failed Device = NULL\n");
	}
#endif
}
void CD3DDisplay::YUV2RGB(unsigned char *src0,unsigned char *src1,unsigned char *src2,unsigned char *dst_ori,
						  int width,int height)
{
	int y1,y2,u,v; 
	unsigned char *py1,*py2;
	int i,j, c1, c2, c3, c4;
	unsigned char *d1, *d2;

	py1=src0;
	py2=py1+width;
	d1=dst_ori;
	d2=d1+3*width;
	for (j = 0; j < height; j += 2) { 
		for (i = 0; i < width; i += 2) {

			u = *src1++;
			v = *src2++;

			c1 = crv_tab[v];
			c2 = cgu_tab[u];
			c3 = cgv_tab[v];
			c4 = cbu_tab[u];

			//up-left
			y1 = tab_76309[*py1++];	
			*d1++ = clp[384+((y1 + c1)>>16)];  
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];
			*d1++ = clp[384+((y1 + c4)>>16)];

			//down-left
			y2 = tab_76309[*py2++];
			*d2++ = clp[384+((y2 + c1)>>16)];  
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];
			*d2++ = clp[384+((y2 + c4)>>16)];

			//up-right
			y1 = tab_76309[*py1++];
			*d1++ = clp[384+((y1 + c1)>>16)];  
			*d1++ = clp[384+((y1 - c2 - c3)>>16)];
			*d1++ = clp[384+((y1 + c4)>>16)];

			//down-right
			y2 = tab_76309[*py2++];
			*d2++ = clp[384+((y2 + c1)>>16)];  
			*d2++ = clp[384+((y2 - c2 - c3)>>16)];
			*d2++ = clp[384+((y2 + c4)>>16)];
		}
		d1 += 3*width;
		d2 += 3*width;
		py1+=   width;
		py2+=   width;
	}       

}

void CD3DDisplay::InitConvertTable()
{
	long int crv,cbu,cgu,cgv;
	int i,ind;   

	crv = 104597; cbu = 132201;  /* fra matrise i global.h */
	cgu = 25675;  cgv = 53279;

	for (i = 0; i < 256; i++) {
		crv_tab[i] = (i-128) * crv;
		cbu_tab[i] = (i-128) * cbu;
		cgu_tab[i] = (i-128) * cgu;
		cgv_tab[i] = (i-128) * cgv;
		tab_76309[i] = 76309*(i-16);
	}

	for (i=0; i<384; i++)
		clp[i] =0;
	ind=384;
	for (i=0;i<256; i++)
		clp[ind++]=i;
	ind=640;
	for (i=0;i<384;i++)
		clp[ind++]=255;
}

bool CD3DDisplay::DisplayRGBFunc(unsigned char* pbof)
{
	//YUV2RGB(pbof,pbof+m_xLen*m_yLen,pbof+m_xLen*m_yLen*5/4,m_pRgbbuf,m_xLen,m_yLen);
	YUV2RGB(pbof,m_pRgbbuf,m_xLen,m_yLen,24);

	BYTE *pSampleBuffer = m_pRgbbuf;
	EnterCriticalSection(&m_csDDraw);
	if(m_pd3dSurface == NULL)
	{
		LeaveCriticalSection(&m_csDDraw);
		return false;   
	}
	D3DLOCKED_RECT d3d_rect;    
	if( FAILED(m_pd3dSurface->LockRect(&d3d_rect,NULL,0)))       
	{
		LeaveCriticalSection(&m_csDDraw);
		return false;    
	}
	BYTE * pTextureBuffer = static_cast<byte *>(d3d_rect.pBits);   
	const int stride = d3d_rect.Pitch;    

	for(int i = 0; i < m_yLen; i++ ) 
	{
		BYTE *pBmpBufferOld = pSampleBuffer;
		BYTE *pTxtBufferOld = pTextureBuffer;   

		for (int j = 0; j < m_xLen; j++) 
		{
			pTextureBuffer[0] = pSampleBuffer[0];
			pTextureBuffer[1] = pSampleBuffer[1];
			pTextureBuffer[2] = pSampleBuffer[2];
			pTextureBuffer[3] = 0xFF;

			pTextureBuffer += 4;
			pSampleBuffer  += 3;
		}

		pSampleBuffer  = pBmpBufferOld + m_xLen *3;
		pTextureBuffer = pTxtBufferOld + stride;
	}   
	if( FAILED(m_pd3dSurface->UnlockRect()))    
	{       
		LeaveCriticalSection(&m_csDDraw);
		return false;   
	}    
	DrawImage();
	LeaveCriticalSection(&m_csDDraw);
	return true;
}

// <summary>显示线程</summary >
// <param name="pPara">this指针 / IN</param>
// <returns></returns>
unsigned __stdcall CD3DDisplay::DisplayThread(LPVOID pPara)
{
	CD3DDisplay* this0 = (CD3DDisplay *)pPara;

	//UINT_PTR idTimer = SetTimer(NULL, 0, 10, NULL);
	MSG Msg;
	PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE);
	BOOL bRet;
	while ((bRet = GetMessage(&Msg, NULL, 0, 0)) != 0)
	{
		if((bRet == -1))
		{
			continue;
		}

		switch(Msg.message)
		{
		case MSG_DISPLAY_FRAME:
			{
				//if(this0->m_pOutBuffer != NULL/* && this0->m_bDisplayFlag*/)
				//{
				CRITICAL_SECTION *csLock = (CRITICAL_SECTION *)Msg.wParam;
				if(csLock != NULL)
				EnterCriticalSection(csLock);
				unsigned char *pobuf = this0->m_pOutBuffer;

				//int iTime1 = GetTickCount();
				if (this0->m_nRGBorYUV == 1)
				{
					if(!this0->DisplayFunc(pobuf))
					{
					}
				}
				else if (this0->m_nRGBorYUV == 2)
				{
					if(!this0->DisplayRGBFunc(pobuf))
					{
					}
				}
				//int iTime2 = GetTickCount() - iTime1;
				//if(iTime2 > 15)
				//{
				//	if(NULL == g_DisplayLog)
				//	{
				//		g_DisplayLog = fopen("D:\\amplesky\\software_wp\\web\\log\\Display.log","w+");
				//	}
				//	fprintf(g_DisplayLog,"DisPlay TIme = %d \n",iTime2);
				//	fflush(g_DisplayLog);
				//}
				if(csLock != NULL)
				LeaveCriticalSection(csLock);
				//}
				//清空消息队列
				while(PeekMessage(&Msg, NULL , 0, 0, PM_REMOVE))
				{
					if(Msg.message == MSG_THREADQUITE)
					{
						this0->m_ThreadId = 0;
						_endthreadex(0);
						return 0;
					}
				}
			}
			break;
		case MSG_THREADQUITE:
			{
				//KillTimer(NULL, idTimer);
				this0->m_ThreadId = 0;
				_endthreadex(0);
				return 0;
			}
			break;
		default:
			break;
		}
	}
	this0->m_ThreadId = 0;
	return 0;
}

// <summary>开始播放，启动显示线程</summary >
// <returns>无</returns>
void CD3DDisplay::StartPlay()
{
	HANDLE ThreadHandle;
	ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, &DisplayThread, this, 0, &m_ThreadId);
	CloseHandle (ThreadHandle);
	m_dwStartTime = GetTickCount();
	m_dwTotalFrame = 0;
}

// <summary>停止播放，结束显示线程</summary >
// <returns>无</returns>
void CD3DDisplay::StopPlay()
{
	int iRepeatTime = 100;
	m_dwStartTime = 0;
	m_dwTotalFrame = 0;
	while((m_ThreadId > 0) && (PostThreadMessage(m_ThreadId, MSG_THREADQUITE, 0, 0)) && (iRepeatTime--))
	{
		Sleep(5);
	}
}

// <summary>设置输出Buffer</summary >
// <param name="pOutBuffer">YUV数据 / IN</param>
// <returns>无</returns>
void CD3DDisplay::SetOutBuffer(unsigned char * pOutBuffer, CRITICAL_SECTION *csLockYUV)
{
	m_pOutBuffer = pOutBuffer;
	PostThreadMessage(m_ThreadId, MSG_DISPLAY_FRAME, (WPARAM)csLockYUV, NULL);
}
// <summary>设置输出显示标志位，为解决新打开图像先显示上次残余图像</summary >
// <param name="pOutBuffer">YUV数据 / IN</param>
// <returns>无</returns>
void CD3DDisplay::SetDisplayFlag(bool bDisplay)
{
	m_bDisplayFlag = bDisplay;
}

void CD3DDisplay::GetVideoParam(int *iWidth, int *iHeight)
{
	*iWidth = m_xLen;
	*iHeight = m_yLen;
}

void CD3DDisplay::FreeDraw()
{
	EnterCriticalSection(&m_csDDraw);
	if(m_pD3D != NULL)    
	{        
		m_pD3D->Release();        
		m_pD3D = NULL;   
	}    
	if(m_pd3dDevice != NULL)    
	{        
		m_pd3dDevice->Release();       
		m_pd3dDevice = NULL;    
	}    
	if(m_pd3dSurface != NULL)    
	{        
		m_pd3dSurface->Release();        
		m_pd3dSurface = NULL;   
	} 
	if(m_pd3dFont != NULL)
	{
		m_pd3dFont->Release();
		m_pd3dFont = NULL;
	}
	LeaveCriticalSection(&m_csDDraw);
}
bool CD3DDisplay::YUV2RGB(unsigned char * yuv, unsigned char * rgb, const int nWidth, const int nHeight, const int nBits)
{
	if(yuv==NULL||rgb==NULL)
		return false;
	if(nBits!=24)
		return false;
	unsigned char *y = yuv;
	unsigned char *v = yuv + nWidth * nHeight;
	unsigned char *u = v + nWidth * nHeight / 4;

	for(int h=0; h<nHeight; h+=2)
	{
		for(int w=0; w<nWidth; w+=2)
		{
			int tmp0,tmp1,tmp2,tmp3;
			tmp0 = h * nWidth + w;
			tmp1 = h * nWidth + w + 1;
			tmp2 = (h + 1) * nWidth + w;
			tmp3 = (h + 1) * nWidth + w + 1;

			int t = h / 2 * nWidth / 2 + w / 2;
			int temp;

			temp = (int)(y[tmp0] + 1.14*(v[t]-128));
			rgb[3*tmp0] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp0] - 0.39*(u[t]-128) - 0.58*(v[t]-128));
			rgb[3*tmp0+1] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp0] + 2.03*(u[t]-128));
			rgb[3*tmp0+2] = MIDDLE(temp,0,255);

			temp = (int)(y[tmp1] + 1.14*(v[t]-128));
			rgb[3*tmp1] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp1] - 0.39*(u[t]-128) - 0.58*(v[t]-128));
			rgb[3*tmp1+1] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp1] + 2.03*(u[t]-128));
			rgb[3*tmp1+2] = MIDDLE(temp,0,255);

			temp = (int)(y[tmp2] + 1.14*(v[t]-128));
			rgb[3*tmp2] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp2] - 0.39*(u[t]-128) - 0.58*(v[t]-128));
			rgb[3*tmp2+1] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp2] + 2.03*(u[t]-128));
			rgb[3*tmp2+2] = MIDDLE(temp,0,255);

			temp = (int)(y[tmp3] + 1.14*(v[t]-128));
			rgb[3*tmp3] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp3] - 0.39*(u[t]-128) - 0.58*(v[t]-128));
			rgb[3*tmp3+1] = MIDDLE(temp,0,255);
			temp = (int)(y[tmp3] + 2.03*(u[t]-128));
			rgb[3*tmp3+2] = MIDDLE(temp,0,255);
		}
	}
	return true;
}

BOOL CD3DDisplay::InitD3DFont(/*std::string strSubTitle*/)
{
	//m_textFONT = CreateFont(m_textHeight, 0, 0, 1, m_textWeight, FALSE, FALSE, FALSE,
	//	GB2312_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_MODERN, "宋体"/*m_typefaceName*/);
	if(m_pd3dDevice == NULL)
	{

		return FALSE;
	}
	if(m_pd3dFont != NULL)
	{
		m_pd3dFont->Release();
		m_pd3dFont = NULL;
	}
	int iFontHeight = 0;
	int iFontWidth = 0;
	//RECT rt;
	//GetClientRect(m_hWnd,&rt);

	int xLen = m_xLen;

	switch(m_FontSize)
	{
	case BigFont:
		{
			iFontHeight = 10*xLen/176*3/2/2;  ///3*2
			iFontWidth = 10*xLen/352*3/2/2;  ///3*2
		}
		break;
	case  FitFont:
		{
			iFontHeight = 10*xLen/176/2;   //2
			iFontWidth = 10*xLen/352/2;
		}
		break;
	case  SmallFont:
		{
			iFontHeight = 10*xLen/176/4;
			iFontWidth = 10*xLen/352/4;  //3*5/4;
		}
		break;
	default:

		break;
	}


	//创建D3DXFont
	D3DXFONT_DESC d3df_d;
	ZeroMemory(&d3df_d, sizeof(D3DXFONT_DESC));

	d3df_d.Height         = iFontHeight;    // in logical units
	d3df_d.Width          = iFontWidth;    // in logical units
	d3df_d.Weight         = 600;   // boldness, range 0(light) - 1000(bold)
	d3df_d.MipLevels	  = 1;
	d3df_d.Italic         = false;   
	d3df_d.CharSet        = GB2312_CHARSET;//DEFAULT_CHARSET;
	d3df_d.OutputPrecision   = 0;              
	d3df_d.Quality        = 0;           
	d3df_d.PitchAndFamily = 0;         
	strcpy(d3df_d.FaceName, "宋体"); // font style
	//strcpy(d3df_d.FaceName, "Times New Roman"); // font style

	//if(FAILED(D3DXCreateFont(m_pd3dDevice,iFontHeight,iFontWidth,500,1,false,DEFAULT_CHARSET,0,0,0,"宋体",&m_pd3dFont)))
	if(FAILED(D3DXCreateFontIndirect(m_pd3dDevice, &d3df_d, &m_pd3dFont)))
	{
		return FALSE;
		
	}
	
	return TRUE;
}

//bool CD3DDisplay::SetSubTitleProp(unsigned char uFontProp,std::string strSplitName1,std::string strSplitName2,
//										 std::string strSplitName3,std::string strSplitName4)
//{
//
//	//uFontProp = 2;
//	//uFontProp = 10;
//	//uFontProp = 18;
//	//int bDisFlag = uFontProp & 0X01;
//	int bIsClarity = uFontProp & 0X01;
//	//int bIsBatch = (uFontProp>>2) & 0X01;
//	int iPlace = (uFontProp>>1) & 0X03;
//	int iSize = (uFontProp>>3) & 0X03;
//
//	std::string strSplitSubtitle = strSplitName1 + "\n" + strSplitName2 + "\n" +strSplitName3 + "\n" + strSplitName4;
//	if(m_FontPlace == iPlace && m_FontSize == iSize && m_bIsClarity == bIsClarity && m_strSubTitle == strSplitSubtitle)
//	{
//		return false;
//	}
//
//	m_FontPlace = (SubTitlePlace)iPlace;
//	m_FontSize = (SubTitleSize)iSize;
//	m_bIsClarity = bIsClarity;
//
//	m_strSubTitle = strSplitSubtitle;
//
//
//	EnterCriticalSection(&m_csDDraw);
//	InitD3DFont();
//	LeaveCriticalSection(&m_csDDraw);
//	return true;
//}

void CD3DDisplay::ReInitDraw(HWND hWnd, int iMutilDisplay)
{
	StopPlay();
	FreeDraw();
	InitDraw(hWnd,iMutilDisplay);
}
bool CD3DDisplay::SetSubTitleProp(unsigned char uFontProp,const char* strSplitName,int iLen)
{

	//uFontProp = 2;
	//uFontProp = 10;
	//uFontProp = 18;
	//int bDisFlag = uFontProp & 0X01;
	int bIsClarity = uFontProp & 0X01;
	//int bIsBatch = (uFontProp>>2) & 0X01;
	int iPlace = (uFontProp>>1) & 0X03;
	int iSize = (uFontProp>>3) & 0X03;

	m_FontPlace = (SubTitlePlace)iPlace;
	m_FontSize = (SubTitleSize)iSize;
	m_bIsClarity = bIsClarity;

	//m_strSubTitle = strSplitName;
	m_strSubTitle.assign(strSplitName);
	//int x = sizeof(m_strSubTitle);
	//memset(m_strSubTitle,0,sizeof(m_strSubTitle));
	//memcpy(m_strSubTitle,strSplitName,iLen);

	EnterCriticalSection(&m_csDDraw);
	InitD3DFont();
	LeaveCriticalSection(&m_csDDraw);
	return true;
}

bool CD3DDisplay::IsUsed()
{
	return m_bUsed;
}

void CD3DDisplay::SetUsed(bool bUsed)
{
	m_bUsed = bUsed;
}
bool CD3DDisplay::SetDisplayScale(bool bVideoScale)
{
	m_bVideoScale = bVideoScale;
	return true;
}
bool CD3DDisplay::SetSubtitleScope(int xLeft,int yTop,int xRight,int yBottom)
{
	//memset(&m_rcRectSrc,0,sizeof(m_rcRectSrc));
	//SetRect(&m_rcRectSrc, xLeft, yTop, xRight, yBottom);
	return true;
}