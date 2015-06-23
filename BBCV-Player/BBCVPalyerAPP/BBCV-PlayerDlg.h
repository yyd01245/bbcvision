// BBCV-PlayerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "../Decoder/TSDecoder.h"
#include "afxcmn.h"
//#include "../AMPDisplaySDK/AMPDisplaySDK.h"
//#include <string>
//#include "Rtsp.h"
#include "RTSPclient.h"
#include "RSMJson.h"




// CBBCVPlayerDlg 对话框
class CBBCVPlayerDlg : public CDialog
{
// 构造
public:
	CBBCVPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_BBCVPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	static unsigned int _stdcall PlayerStatusCheck(void* param);

	unsigned udp_recv_thread;
	CRTSPclient* m_rtsptheClient;
	URLTYPE m_ifileType;
	TSDecoder_t* m_pInstance;
	bool m_bPlaying;
	FILE *m_fpLog;
	int m_iCurrentLineNum;
	bool m_bAutoMatch;
	bool m_bSmoothPlay;
//	LONG m_lDisplayport;
	bool m_bPauseStatus; //true 为暂停 false为恢复

	bool m_bNeedLog;
	bool m_bSavevideodata;
	bool m_bDelayTimeModel;


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CStatic m_cDisplay_Control;
	CEdit m_cUDPPort;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButton1();

	BOOL PreTranslateMessage(MSG* pMsg);

	void show_Log();

	CBitmapButton m_btnhelp;

	CEdit m_showLogWnd;
	CStatic m_showInfo;
	CComboBox m_cbExamble;
	int m_iLastRate;
	RSMJsonObject m_rsmjsonobj;

	char m_strRsmIP[256];
	int  m_iRSMPort;
	int  m_iLoginNumber;
	int  m_iStreamPortBegin;
	int  m_iControlStreamindex;
	char m_strRecvTSIP[256];
	int  m_vncmsPort;
	bool m_bStreamtoClient;
	CIPAddressCtrl m_cIPRSMctrl;
	CEdit m_cPortRSMctrl;
	CEdit m_cLoginNumberctrl;
	CEdit m_cStreamPortbeginctrl;
	CEdit m_cControlStreamIndexctrl;
	CEdit m_cControlVNCportctrl;
	CButton m_cStreamtoClientctrl;
	CButton m_cStreamtoLocalctrl;
	CIPAddressCtrl m_cRecvTSIPtctrl;
	CEdit m_cURLctrl;
	CButton m_cSendKeyctrl;

	bool m_bSendKey;

	afx_msg void OnTimer(UINT nIDEvent);
	CSliderCtrl m_cAudioVolctrl;
	afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRadio1();
	CButton m_MediaMatch;
	CButton m_cCheckMatch;
	afx_msg void OnBnClickedCheck1();
	CButton m_cSmooth;
	afx_msg void OnBnClickedCheck2();
	CButton m_cPlayButton;
	CButton m_cStopButton;
	afx_msg void OnBnClickedButton2();
	CButton m_cPauseStatus;
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedCheck3();
	CButton m_cNeedLog;
	CButton m_cSaveVideoData;
	CButton m_cDelayTimeModel;
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelendokCombo1();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedCheck6();
	afx_msg void OnBnClickedCheck7();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedRadioKey();
	afx_msg void OnBnClickedButton7();
	int SendKey(int iKeyID);
	afx_msg void OnBnClickedButton10();
};
