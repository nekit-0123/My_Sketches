#include "stdafx.h"
#include "CChangeIp.h"
#include "..\..\res\Phrases.h"
#include "..\..\res\Definitions.h"

IMPLEMENT_DYNAMIC(CChangeIp, CDialogEx)

CChangeIp::CChangeIp(const CString& CurrentIp, const CString& Ip, const CString& NetMask)
	: CDialogEx(IDD_CHANGEIP, nullptr),
	currentIP(CurrentIp),
	resultIP(Ip),
	resultMask(NetMask),
	type(L""),
	m_scanner(nullptr),
	checkIPisBusy(-1)
{}

CChangeIp::~CChangeIp()
{}

void CChangeIp::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_GIF_CONTROL, WaitGif);
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChangeIp, CDialogEx)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS2, &CChangeIp::OnIpnFieldchangedIpaddress2)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_MASK,		&CChangeIp::OnIpnFieldchangedMask)
	ON_BN_CLICKED(IDOK,							&CChangeIp::OnBnClickedOk)
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CChangeIp::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText(g_sChangeIp + L" "+ type);

	GetDlgItem( IDC_LABEL_OLDIP   )	->SetWindowText (g_sCurrentIp + L":");
	GetDlgItem( IDC_OLDIP		  )	->SetWindowText (currentIP);
	GetDlgItem( IDC_OLDIP		  )	->EnableWindow  (FALSE);
	GetDlgItem( IDC_LABEL_NEWIP   )	->SetWindowText (g_sNewIp + L":");
	GetDlgItem( IDC_NEWIP		  ) ->SetWindowText (resultIP);
	GetDlgItem( IDC_LABEL_NEWMASK ) ->SetWindowText (g_sSubnetMask + L":");
	GetDlgItem( IDC_NEWMASK		  ) ->SetWindowText (resultMask);
	GetDlgItem( IDC_ERROR		  ) ->SetWindowText (L"");
	GetDlgItem( IDC_CHECKBUSY	  ) ->ShowWindow	(SW_HIDE);
	GetDlgItem( IDC_CHECKBUSY	  ) ->SetWindowText (g_sBusyIpAdress);
	return TRUE;
}

CString CChangeIp::getIP()
{
	return resultIP;
}

CString CChangeIp::getMask()
{
	return resultMask;
}

void CChangeIp::OnIpnFieldchangedIpaddress2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	GetDlgItem(IDC_IPADDRESS2)->GetWindowText(resultIP);
	*pResult = 0;
}

void CChangeIp::OnIpnFieldchangedMask(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	GetDlgItem(IDC_MASK)->GetWindowText(resultMask);
	*pResult = 0;
}

void CChangeIp::OnBnClickedOk()
{
	if (m_scanner != nullptr)
	{
		GetDlgItem(IDC_ERROR)->SetWindowText(L"");
		std::string checkIp = CT2A(resultIP.GetString());
		std::string checkMask = CT2A(resultMask.GetString());
		if (!m_scanner->ValidateAddress(checkIp, checkMask))
		{
			GetDlgItem(IDC_ERROR)->SetWindowText(g_sInCorrect);
			return;
		}

		ShowComponents(SW_HIDE);
		WaitGif.Load(MAKEINTRESOURCE(IDR_GIF1), _T("GIF"));
		WaitGif.Draw();
		SetTimer(1, 10, NULL);

		auto result = [this](const std::string&, const std::string&, bool avail) {
			checkIPisBusy = avail ? 1 : 0;
		};
		ZdcpScanner::CheckAddressAvailable("0.0.0.0", std::string(CW2A(resultIP)), result);
	}
}

void CChangeIp::ShowComponents(int param)
{
	if (param == SW_SHOW)
	{
		GetDlgItem( IDC_CHECKBUSY   ) ->ShowWindow(SW_HIDE);
		GetDlgItem( IDC_GIF_CONTROL ) ->ShowWindow(SW_HIDE);
	}
	else
	{
		GetDlgItem( IDC_CHECKBUSY  ) ->ShowWindow(SW_SHOW);
		GetDlgItem( IDC_GIF_CONTROL) ->ShowWindow(SW_SHOW);
	}

	GetDlgItem( IDC_NEWIP		 )	->ShowWindow(param);
	GetDlgItem( IDC_NEWMASK		 )	->ShowWindow(param);
	GetDlgItem( IDC_OLDIP		 )	->ShowWindow(param);
	GetDlgItem( IDC_LABEL_OLDIP	 )	->ShowWindow(param);
	GetDlgItem( IDC_LABEL_NEWIP	 )	->ShowWindow(param);
	GetDlgItem( IDC_LABEL_NEWMASK)	->ShowWindow(param);
}

void CChangeIp::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		if (checkIPisBusy == 0)
		{
			KillTimer(1);
			ShowComponents(SW_SHOW);
			WaitGif.UnLoad();
			GetDlgItem(IDC_ERROR)->SetWindowText(g_sIpIsBusy);
			checkIPisBusy = -1;
		}
		else if (checkIPisBusy != -1 && checkIPisBusy != 0)
		{
			KillTimer(1);
			WaitGif.UnLoad();
			CDialogEx::OnOK();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}
