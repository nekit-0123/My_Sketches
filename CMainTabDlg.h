#pragma once
#include <CDUnit.h>
#include <OnHelp.h>
#include <Dialog_ZET\Translate.h>
#include <Dialog_ZET\CDWindowsLog.h> 
#include "NetWizardDlg.h"
#include "device_7xxx.h"


class CMainTabDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMainTabDlg)
public:
	CMainTabDlg(CWnd* pParent = nullptr);   
	virtual ~CMainTabDlg();
	enum { IDD = IDD_MAINTAB };

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	afx_msg void OnHelp();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMClickTabs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();

	virtual void DoDataExchange(CDataExchange* pDX);    
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	std::unique_ptr <CAutoScaleXY>	 m_autoScale;
	std::unique_ptr <CStatusBarCtrl> m_wndStatusBar;
	std::unique_ptr <CNetWizardDlg>	 wizardDlg;
	std::unique_ptr <device_7xxx>    ZetSensorDlg;
	std::unique_ptr <CDZetPath>		 m_ppath;
	std::unique_ptr <CFont>			 myFont;

	CTabCtrl      m_tabs;
	HICON		  m_hIcon;
	RECT		  m_rectOld;
	COnHelp		  m_OnHelp;
	CDUnit		  m_unit;
	CDWindowslog  m_windowsLog;
	safeproc	  mysafeproc;

	void InsertTabs();
	void ReadyUnitctrl1(long param);
	void WriteCfgFile();
	void ReadCfgFile();
	void SetAutoScale();
	void SmartWidthColumn();

	bool m_unit_reg;
	bool m_bEnableInterface;
public:
	CDZetPath *GetPath();
};
