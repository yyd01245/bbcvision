/*
	author yyd
		
*/

#pragma once
#include <Windows.h>
#include <MMSystem.h>
#include "..\D3DDisplay\include\dsound.h"

class AudioPlay
{
public:
	AudioPlay(void);
	~AudioPlay(void);

	bool InitDSound(HWND hWnd,DWORD dwSamplesPerSec);
	void FreeDSound();
	void StartPlay();
	void StopPlay();
	void PlayData(void * pBuffer, int iDataLength);
	bool SetVolume(DWORD nVolume);
	bool GetVolume(DWORD &nVolume);

private:
	LPDIRECTSOUND8 m_lpDS;
	LPDIRECTSOUNDBUFFER8 m_lpDSB;
	DWORD m_dwPlayBufferSize;
	DWORD m_dwNextPlayOffset;

	bool m_bEnableSound;
	bool m_bPlayFlag;
	int m_iSetBufferCount;
	WORD m_wVolume;

	CRITICAL_SECTION m_csDSound;
};
