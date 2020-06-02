#include "stdafx.h"
#include "CDeviceInfo.h"
#include "afxdialogex.h"
#include "..\..\res\Phrases.h"

IMPLEMENT_DYNAMIC(CDeviceInfo, CDialogEx)

#define LoadBitmap_(nID) \
    (HBITMAP)::LoadImage( AfxGetInstanceHandle(), \
    MAKEINTRESOURCE( nID ), \
    IMAGE_BITMAP, 64, 64, LR_SHARED | LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS )

CDeviceInfo::CDeviceInfo(CWnd* pParent)
	: CDialogEx(IDD_MINIINFO, pParent)
{}

CDeviceInfo::~CDeviceInfo()
{}

void CDeviceInfo::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_ICON, m_icon);
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDeviceInfo, CDialogEx)
END_MESSAGE_MAP()

void CDeviceInfo::SetStrInfo(const DeviceInfo& info)
{
	this->info = info;
}

BOOL CDeviceInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowTextW(g_sDeviceInfo);

	GetDlgItem(IDC_LabelTypeDevice)->SetWindowTextW(g_sDeviceType + L":");
	GetDlgItem(IDC_LabelSerialNum) ->SetWindowTextW(g_sSerial	  + L":");
	GetDlgItem(IDC_LabelIpAdres)   ->SetWindowTextW(g_sIP_address + L":");
	GetDlgItem(IDC_LabelState)     ->SetWindowTextW(g_sStatus     + L":");

	ZdcpScanner::Device::Type type;
	type = info.type;

	switch (type.GetCategory())
	{
	case ZdcpScanner::Device::Type::ANALYSIS:
		GetDlgItem(IDC_TypeDevice)->SetWindowText(g_sAnaliz);
		break;
	case ZdcpScanner::Device::Type::TENSO:
		GetDlgItem(IDC_TypeDevice)->SetWindowText(g_sTenso);
		break;
	case ZdcpScanner::Device::Type::SEISMO:
		GetDlgItem(IDC_TypeDevice)->SetWindowText(g_sSeismo);
		break;
	case ZdcpScanner::Device::Type::ZETSENSOR:
		GetDlgItem(IDC_TypeDevice)->SetWindowText(g_s7xxx);
		break;
	default:
		GetDlgItem(IDC_TypeDevice)->SetWindowText(type.GetNameW());
		break;
	}

	CString format(L"");
	if (type.GetCategory() == ZdcpScanner::Device::Type::Category::ZETSENSOR 
		|| (info.serial >> 32) != 0ull)
		format.Format(L"%016llx", info.serial);
	else 
		format.Format(L"%llu", info.serial);

	GetDlgItem(IDC_SerialNum)->SetWindowTextW(format);
	GetDlgItem(IDC_IpAdres)->SetWindowTextW(info.ip);

	switch (info.status)
	{
	case ZdcpScanner::Device::Status::AVAILABLE:
		GetDlgItem(IDC_State)->SetWindowTextW(g_sAvailable);
		break;
	case ZdcpScanner::Device::Status::CONNECTED:
		if (info.isOurDevice)
			GetDlgItem(IDC_State)->SetWindowTextW(g_sDeviceConnected);
		else 
			GetDlgItem(IDC_State)->SetWindowTextW(g_sBusyDevice);	
		break;
	case ZdcpScanner::Device::Status::CONFLICT:
		GetDlgItem(IDC_State)->SetWindowTextW(g_sConflict);
		break;
	default:
		GetDlgItem(IDC_State)->SetWindowTextW(g_sUnknown);
		break;
	}

	if (info.is_fictional_device)
		GetDlgItem(IDC_State)->SetWindowTextW(g_sOffdevice);

	if (!info.label.IsEmpty())
		GetDlgItem(IDC_Comment)->SetWindowTextW(info.label);
	else 
		GetDlgItem(IDC_Comment)->SetWindowTextW(type.GetNameW());

	if (type.GetCategory() == ZdcpScanner::Device::Type::Category::ZETSENSOR)
		m_icon.SetBitmap(LoadBitmap_(IDB_SENSORS));
	else
		m_icon.SetBitmap(LoadBitmap_(IDB_ANALIZATOR));

	if (info.ip.IsEmpty())
		NotIP();

	return 0;
}

void CDeviceInfo::NotIP()
{
	GetDlgItem(IDC_IpAdres)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LabelIpAdres)->ShowWindow(SW_HIDE);

	CRect rect(NULL);
	GetDlgItem(IDC_LabelIpAdres)->GetWindowRect(&rect);
	GetDlgItem(IDC_LabelIpAdres)->GetParent()->ScreenToClient(&rect);
	GetDlgItem(IDC_LabelState)->MoveWindow(rect);

	rect = NULL;
	GetDlgItem(IDC_IpAdres)->GetWindowRect(&rect);
	GetDlgItem(IDC_IpAdres)->GetParent()->ScreenToClient(&rect);
	GetDlgItem(IDC_State)->MoveWindow(rect);
}