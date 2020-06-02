#pragma once
#include "stdafx.h"
#include "NetWizard.h"
#include "CMainTabDlg.h"
#include <ZETFile\include\ZETFile.h>

IMPLEMENT_DYNAMIC(CMainTabDlg, CDialogEx)

CMainTabDlg::CMainTabDlg(CWnd* pParent)
	: CDialogEx(IDD_MAINTAB, pParent)
	, m_unit_reg(false)
	, m_bEnableInterface(true)
	, m_ppath(nullptr)

{
	InitZetDictionary();
	Phrases::getInstance();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ztInitCreateDumpWhenException();
	m_rectOld.left = m_rectOld.right = m_rectOld.top = m_rectOld.bottom = 0;
}

CMainTabDlg::~CMainTabDlg()
{}

BEGIN_MESSAGE_MAP(CMainTabDlg, CDialogEx)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_SIZING()
	ON_WM_SETFONT()
	ON_WM_SHOWWINDOW()
	ON_WM_QUERYDRAGICON()
	ON_WM_WINDOWPOSCHANGING()
	ON_NOTIFY(NM_CLICK, IDC_TABS, &CMainTabDlg::OnNMClickTabs)
	ON_COMMAND(ID_HELP,			  &CMainTabDlg::OnHelp)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CMainTabDlg, CDialogEx)
	ON_EVENT(CMainTabDlg, IDC_UNITCTRL1, 1, CMainTabDlg::ReadyUnitctrl1, VTS_I4)
END_EVENTSINK_MAP()

//------------------------------------------------------------------------------
void CMainTabDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TABS, m_tabs);
	DDX_Control(pDX, IDC_UNITCTRL1, m_unit);
	DDX_Control(pDX, IDC_WINDOWSLOG1, m_windowsLog);
}

//------------------------------------------------------------------------------
BOOL CMainTabDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_autoScale		= std::make_unique<CAutoScaleXY>	();
	m_wndStatusBar  = std::make_unique<CStatusBarCtrl>	();
	wizardDlg		= std::make_unique<CNetWizardDlg>	();
	ZetSensorDlg	= std::make_unique<device_7xxx>		();

	SetIcon(m_hIcon, TRUE);			
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	m_ppath = nullptr;
	m_ppath = std::make_unique<CDZetPath>();
	if (m_ppath != nullptr)
	{
		m_ppath->Create(L"", WS_CHILD, CRect(0, 0, 0, 0), this, 1701);
		m_OnHelp.Create(m_ppath.get());
	}

	CString mystr = g_sConnectingDevicesEthernet;

#ifdef DEBUG
	mystr = L"!!!DEBUG!!! " + mystr;
#endif
	SetWindowText(mystr);

	m_unit_reg = FALSE;
	if (m_unit.UnitReg((ULONG)m_hWnd) == 0)//	return FALSE;
	{
			m_unit_reg = TRUE;
			m_bEnableInterface = false;
			SetTimer(7, 1000, NULL);
	}

	m_wndStatusBar->Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM, CRect(), this, 10011);
	int widths(-1);
	m_wndStatusBar->SetParts(1, &widths);

	SetTimer(WM_UPDATE_STATUS_BAR, 1000, NULL);
	wizardDlg->Create(IDD_NETWIZARD_DIALOG, this);
	wizardDlg->ReadAndCheckCfgNetServer();
	ZetSensorDlg->Create(IDD_7XXX_TAB, this);

	SetAutoScale();
	if (m_unit_reg == FALSE)
		ReadCfgFile();

	InsertTabs();

	myFont = std::make_unique<CFont>();
	myFont->CreatePointFont(94, (L"ZETLab"));
	wizardDlg->GetList()->SetFont(myFont.get());
	ZetSensorDlg->GetList()->SetFont(myFont.get());
	m_wndStatusBar->SetFont(myFont.get());
	m_tabs.SetFont(myFont.get());

	return TRUE;
}

//------------------------------------------------------------------------------
// Заполняем вкладки анализаторов и 7xxx
void CMainTabDlg::InsertTabs()
{
	TCITEM item1;
	item1.mask = TCIF_TEXT | TCIF_PARAM;

	// Анализатор
	item1.lParam = (LPARAM)& wizardDlg;
	item1.pszText = (LPWSTR)(LPCTSTR)g_sAnalizators;
	m_tabs.InsertItem(0, &item1);
	CRect rcItem;
	m_tabs.GetItemRect(0, &rcItem);
	wizardDlg->SetWindowPos(NULL, rcItem.left, rcItem.bottom + 3, NULL, NULL, SWP_FRAMECHANGED | SWP_NOSIZE);
	wizardDlg->ShowWindow(SW_SHOW);

	// 7xxx
	item1.lParam = (LPARAM)& ZetSensorDlg;
	item1.pszText = _T("ZETSENSOR");
	m_tabs.InsertItem(1, &item1);
	CRect rcItem2;
	m_tabs.GetItemRect(0, &rcItem2);
	ZetSensorDlg->SetWindowPos(NULL, rcItem2.left, rcItem2.bottom + 3, NULL, NULL, SWP_FRAMECHANGED | SWP_NOSIZE);
	ZetSensorDlg->ShowWindow(SW_HIDE);

	if (mysafeproc.numTab == 1)
	{
		m_tabs.SetCurSel(1);
		wizardDlg->ShowWindow(SW_HIDE);
		ZetSensorDlg->ShowWindow(SW_SHOW);
	}
	else
		m_tabs.SetCurSel(0);

	RedrawWindow();
}

//------------------------------------------------------------------------------
// Переключение вкладок
void CMainTabDlg::OnNMClickTabs(NMHDR *pNMHDR, LRESULT *pResult)
{
	switch (m_tabs.GetCurSel())
	{
	case 0: 
		ZetSensorDlg-> ShowWindow(SW_HIDE);
		wizardDlg	-> ShowWindow(SW_SHOW);
		mysafeproc.numTab = 0;
		break;
	case 1:
		wizardDlg	-> ShowWindow(SW_HIDE);
		ZetSensorDlg-> ShowWindow(SW_SHOW);
		mysafeproc.numTab = 1;
		break;
	}
	*pResult = 0;
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnPaint()
{
	int first(0);
	RECT myrect;
	GetClientRect(&myrect);
	if (first == 0)
	{
		PostMessage(WM_SIZE, SIZE_RESTORED, myrect.right - myrect.left + ((myrect.bottom - myrect.top) << 16));
		first++;
	}
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//------------------------------------------------------------------------------
HCURSOR CMainTabDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnHelp()
{
	m_OnHelp.OnHelp();
}

//------------------------------------------------------------------------------
void CMainTabDlg::ReadyUnitctrl1(long param)
{
	long lParametr(-1);
	double dValue(0.);
	m_unit.UnitParam(&lParametr, &dValue);
	switch (lParametr)
	{
	case 0:		//Установка количества подключаемых устройств (от 0 до 1024)
	{
		CString sTemp;
		sTemp.Format(L"%d", long(dValue));
		break;
	}

	default:
		break;
	}
	//Установка IP-адреса первого … тысяча двадцать четвертого устройства, подключаемого по Ethernet (IP-адрес устройства задается в виде строки вида xxx.xxx.xxx.xxx)
	if (lParametr >= 1000 && lParametr < 1000 + MAX_DEVICE)
	{
		CString sTemp;
		BSTR bstrTemp = sTemp.AllocSysString();
		sTemp = bstrTemp;
		SysFreeString(bstrTemp);
		if (wcscmp(sTemp, wizardDlg->ipaddress[lParametr - 1000]) != 0)
		{
			wizardDlg->ipaddress[lParametr - 1000] = sTemp;
		}
	}
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if (m_unit_reg && !m_bEnableInterface)
		lpwndpos->flags &= ~SWP_SHOWWINDOW;

	CDialogEx::OnWindowPosChanging(lpwndpos);
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	static int num = 0;
	static bool bUnitNoFirst = false;
	if (m_unit_reg)
	{
		if (++num > 2)
			bUnitNoFirst = true;

		if (bUnitNoFirst)
		{
			if (bShow)
				m_bEnableInterface = true;
			else
				m_bEnableInterface = false;
		}
	}
}

//------------------------------------------------------------------------------
void CMainTabDlg::WriteCfgFile()
{
	if (m_ppath == nullptr)
		return;

	BSTR bres(L"DirConfig");
	CString tempstr = m_ppath->ZetPath(&bres) + L"netwizardNew.cfg";

	CustomSaveStructure * pcss = (CustomSaveStructure *)&mysafeproc;
	pcss->GetPlacement(m_hWnd);

	bool ret = (0 == zfWriteFileBinary(tempstr, &mysafeproc, sizeof(safeproc)));
}

//------------------------------------------------------------------------------
void CMainTabDlg::ReadCfgFile()
{
	if (m_ppath == nullptr)
		return;

	BSTR bres(L"DirConfig");
	CString sDirConfig = m_ppath->ZetPath(&bres) + L"netwizardNew.cfg";
	DWORD sizeFileCfgInByte(0);
	int m_sizeSaveData = sizeof(safeproc);

	bool ret = (0 == zfGetFileSizeInBytes(sDirConfig, &sizeFileCfgInByte)) &&
		(sizeFileCfgInByte == m_sizeSaveData) &&
		(0 == zfReadFileBinary(sDirConfig, &mysafeproc, m_sizeSaveData));
	if (ret)
		mysafeproc.SetPlacement(m_hWnd);
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnClose()
{
	wizardDlg	->GetList()->EndConnecting();
	ZetSensorDlg->GetList()->EndConnecting();

	wizardDlg	->StopThread();
	ZetSensorDlg->StopThread();

	if (m_unit_reg == FALSE)
	{
		WriteCfgFile();
	}
	if (myFont != nullptr)
	{
		myFont->DeleteObject();
		myFont = nullptr;
	}

	CDialogEx::OnClose();
	EndDialog(IDCANCEL);
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnOK()
{}

//------------------------------------------------------------------------------
void CMainTabDlg::OnCancel()
{}

//------------------------------------------------------------------------------
void CMainTabDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == WM_UPDATE_STATUS_BAR)
	{
		int pType(0);
		auto status = wizardDlg->getInterfaces();
		if (status != m_wndStatusBar->GetText(0, &pType))
			m_wndStatusBar->SetText(status, 0, SBT_POPOUT);
	}
	else if (nIDEvent == WM_UPDATE_WIN_SIZE)
	{
		KillTimer(WM_UPDATE_WIN_SIZE);
		SmartWidthColumn();
	}

	CWnd::OnTimer(nIDEvent);
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnSize(UINT nType, int cx, int cy)
{
	if (m_autoScale)
		m_autoScale->AutoScale(cx, cy);

	if (wizardDlg && ZetSensorDlg)
	{
		CRect lpRect;
		m_tabs.GetItemRect(0, &lpRect);

		CRect lpRectTabs;
		m_tabs.GetWindowRect(lpRectTabs);
		m_tabs.GetParent()->ScreenToClient(lpRectTabs);

		CRect lpRectStatusBar;
		m_wndStatusBar->GetWindowRect(lpRectStatusBar);
		m_wndStatusBar->GetParent()->ScreenToClient(lpRectStatusBar);

		wizardDlg->MoveWindow(lpRect.left, lpRect.bottom + 3, lpRectTabs.right-3, lpRectStatusBar.top - (lpRectTabs.bottom - lpRectStatusBar.top));
		wizardDlg->GetList()->MoveWindow(NULL, NULL, lpRectTabs.right-1, lpRectStatusBar.top - (lpRectTabs.bottom - lpRectStatusBar.top));
		
		ZetSensorDlg->MoveWindow(lpRect.left, lpRect.bottom + 3, lpRectTabs.right - 3, lpRectStatusBar.top - (lpRectTabs.bottom - lpRectStatusBar.top));
		ZetSensorDlg->GetList()->MoveWindow(NULL, NULL, lpRectTabs.right - 1, lpRectStatusBar.top - (lpRectTabs.bottom - lpRectStatusBar.top));

		SetTimer(WM_UPDATE_WIN_SIZE, 100, NULL);
	}

	CDialogEx::OnSize(nType, cx, cy);
}

//------------------------------------------------------------------------------
void CMainTabDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	if (pRect->right - pRect->left < Min_Width)
	{
		pRect->left = m_rectOld.left;
		pRect->right = m_rectOld.right;
	}
	if (pRect->bottom - pRect->top < Min_Height)
	{
		pRect->top = m_rectOld.top;
		pRect->bottom = m_rectOld.bottom;
	}
	m_rectOld = *pRect;
}

//------------------------------------------------------------------------------
void CMainTabDlg::SetAutoScale()
{
	m_autoScale->Create(this);
	m_autoScale->AddControl(m_tabs.m_hWnd, tipReSize, tfTop | tfLeft | tfInvalidate);
	m_autoScale->AddControl(m_wndStatusBar->m_hWnd, tipReSize, tfInvalidate);
	m_autoScale->AddControlFinish();
}

//------------------------------------------------------------------------------
void CMainTabDlg::SmartWidthColumn()
{
	CRect rc;
	wizardDlg->GetList()->GetWindowRect(rc);
	wizardDlg->GetList()->SetColumnWidth(Col_Type, int(rc.Width() * 0.30));
	wizardDlg->GetList()->SetColumnWidth(Col_Name, int(rc.Width() * 0.25));
	wizardDlg->GetList()->SetColumnWidth(Col_Status, int(rc.Width() * 0.45));
	ZetSensorDlg->GetList()->SetColumnWidth(Col_Type, int(rc.Width() * 0.40));
	ZetSensorDlg->GetList()->SetColumnWidth(Col_Name, int(rc.Width() * 0.20));
	ZetSensorDlg->GetList()->SetColumnWidth(Col_Status, int(rc.Width() * 0.40));
}

CDZetPath *CMainTabDlg::GetPath()
{
	return m_ppath.get();
}
//------------------------------------------------------------------------------