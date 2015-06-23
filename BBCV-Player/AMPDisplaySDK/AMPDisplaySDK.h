#ifndef _AMPDISPLAYSDK_H_
#define _AMPDISPLAYSDK_H_

#pragma once
#include <Windows.h>
#include <string>

//typedef struct 
//{
//	std::string stringByName1;
//	std::string stringByName2;
//	std::string stringByName3;
//	std::string stringByName4;
//	int			iFontProperity;
//}SubTitleProp;

//#ifdef AMPDISPLAYSDK_EXPORTS
#if defined _WINDLL
#define AMPDISPLAYSDK_API extern "C"  _declspec(dllexport)
#else
#define AMPDISPLAYSDK_API extern "C"  _declspec(dllimport)
#endif

#define AMP_PLAY_MAX_SUPPORTS 128

#define AMP_PLAY_NOERROR 0
#define AMP_PLAY_NOINIT	1						//句柄已存在，或为归0
#define AMP_PLAY_HANDLE_ALREADY_USED	2		//该Handle已经被使用
#define AMP_PLAY_PARA_ERROR				3		//参数错误
#define AMP_PLAY_ORDER_ERROR			4		//调用顺序错误
#define AMP_PLAY_HEADERBUF_ERROR		5		//数据头有误
#define AMP_PLAY_OVER_CHANNEL			6		//超出了最大支持路数
#define AMP_PLAY_OPENFILE_ERROR			7		//打开文件失败
#define AMP_PLAY_UNKNOWN				8		//未知错误
#define AMP_PLAY_OBJNOTFIND				9		//对象不存在

//<summary> 获取播放句柄
//<param >
//<return> true为成功，false 失败
AMPDISPLAYSDK_API bool __stdcall AMP_Display_GetHandle(LONG &lHandle);

//<summary> 释放播放句柄
//<param >
//<return> true为成功，false 失败
AMPDISPLAYSDK_API bool __stdcall AMP_Display_FreeHandle(LONG lHandle);

//<summary> 初始化显示对象，绑定窗口
//<param >
//<return> true为成功，false 失败
AMPDISPLAYSDK_API bool __stdcall AMP_Display_InitDisplay(LONG lHandle,HWND hWnd,int iAdpterNum=0);

//<summary> 释放显示对象
//<param >
//<return> true为成功，false 失败
AMPDISPLAYSDK_API bool __stdcall AMP_Display_FreeDisplay(LONG lHandle);

//<summary> 设置显示参数
//<param >
//<return> true为成功，false 失败
AMPDISPLAYSDK_API bool __stdcall AMP_Display_SetParam(LONG lHandle,long lWidth,long lHeight);

//<summary> 重新初始化显示对象
//<param >
//<return> true为成功，false 失败
AMPDISPLAYSDK_API bool __stdcall AMP_Display_reInitDisplay(LONG lHandle,HWND hWnd,int iAdpterNum);

//<summary> 设置字幕信息
//<param name= "lHandle"> 播放句柄 /IN</param>
//<returns>返回成功 TRUE，失败 FALSE
AMPDISPLAYSDK_API bool __stdcall AMP_Display_SetSubTitleProp(LONG lHandle,const char* subTitleProperity,int iLen,unsigned char iFontProp);

//<summary> 获取播放参数信息
//<param name= "lHandle"> 播放句柄 /IN</param>
//<returns>返回成功 TRUE，失败 FALSE
AMPDISPLAYSDK_API bool __stdcall AMP_Display_GetDisplayParam(LONG lHandle,int &iWidth,int &iHeight);

//<summary> 显示YUV图像
//<param name= "lHandle"> 播放句柄 /IN</param>
//<returns>返回成功 TRUE，失败 FALSE
AMPDISPLAYSDK_API bool __stdcall AMP_Display_DisplayYUV(LONG lHandle,unsigned char * pOutBuffer, void *csLockYUV);

//<summary> 停止显示图像
//<param name= "lHandle"> 播放句柄 /IN</param>
//<returns>返回成功 TRUE，失败 FALSE
AMPDISPLAYSDK_API bool __stdcall AMP_Display_StopDisplay(LONG lHandle);

//<summary> 显示图像比例
//<param name= "lHandle"> 播放句柄 /IN</param>
//<param name= "bVideoScale"> /IN False为窗口比例，True为视频图像比例。
//<returns>返回成功 TRUE，失败 FALSE
AMPDISPLAYSDK_API bool __stdcall AMP_Display_SetScale(LONG lHandle,bool bVideoScale);

//<summary> 设置字幕显示范围
//<param name= "lHandle"> 播放句柄 /IN</param>
//<returns>返回成功 TRUE，失败 FALSE
AMPDISPLAYSDK_API bool __stdcall AMP_Display_SetSubtitleScope(LONG lHandle,int xLeft,int yTop,int xRight,int yBottom);

#endif