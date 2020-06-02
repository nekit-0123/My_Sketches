#include "stdafx.h"
#include "listCtrl.h"
#include "..\DeviceInfo\CDeviceInfo.h"
#include "..\ChangeIP\CChangeIp.h"

// Всегда используем релизный Shield (нам не нужен его отладочный вывод)
#ifdef DEBUG
#undef DEBUG
#include <Shield\Shield.h>
#define DEBUG
#else
#include <Shield\Shield.h>
#endif


#pragma region _CListCtrl_
CListCtrlDevice::CListCtrlDevice()
	: cstrText(L"")
	, blockUpdate(false)
{}

CListCtrlDevice::~CListCtrlDevice()
{}

BEGIN_MESSAGE_MAP(CListCtrlDevice, CListCtrl)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0,				OnNeedToolTip			)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,			OnListCtrlCustomDraw	)
	ON_NOTIFY_REFLECT(NM_RCLICK,				OnRButtonDown			)
	ON_NOTIFY_REFLECT(NM_DBLCLK,				OnDblclk				)


	ON_COMMAND(en_NumberMenuSelect::NumOnOff,	OnMenuSelectOnOff		)
	ON_COMMAND(en_NumberMenuSelect::ChangeIp,	OnMenuSelectChangeIp	)
	ON_COMMAND(en_NumberMenuSelect::Up,			OnMenuSelectDeviceUp	)
	ON_COMMAND(en_NumberMenuSelect::Down,		OnMenuSelectDeviceDown	)
END_MESSAGE_MAP()

void CListCtrlDevice::InsertDevice(const int nItem, const int Count, const CString name)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = 2147483647;
	lvi.pszText = _T(" ");
	lvi.cchTextMax = 1;
	SetItemText(nItem, Count, name);
}

void CListCtrlDevice::AddColumn()
{
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER );

	InsertColumn(Col_Type,	g_sType, LVCFMT_LEFT,	130, -1);
	InsertColumn(Col_Name, g_sName, LVCFMT_LEFT,	130, -1);
	InsertColumn(Col_Status, g_sStatus, LVCFMT_LEFT, 216, -1);

	LoadItemImages();
}

void CListCtrlDevice::LoadItemImages()
{
	m_imageList.Create(16, 16, ILC_COLOR32, 10, 1);
	auto select_icon = [](int image_index) {
		switch (image_index)
		{
		case en_NumberImage::just_ethernet:		return IDI_ETHERNET;
		case en_NumberImage::connect_ethernet:	return IDI_ETHERNET_CONNECT;
		case en_NumberImage::ethernet_error:	return IDI_ETHERNET_lERROR;
		case en_NumberImage::ethernet_locked:	return IDI_ETHERNET_lOCKED;
		case en_NumberImage::connecting1:		return IDI_CONNECT1;
		case en_NumberImage::connecting2:		return IDI_CONNECT2;
		case en_NumberImage::connecting3:		return IDI_CONNECT3;
		case en_NumberImage::connecting4:		return IDI_CONNECT4;

		}
		return IDI_ETHERNET;
	};
	while (m_imageList.GetImageCount() < en_NumberImage::lastitem)
		m_imageList.Add(LOAD_ICON(select_icon(m_imageList.GetImageCount()), 20, 20));
	SetImageList(&m_imageList, LVSIL_SMALL);
}

void CListCtrlDevice::OnListCtrlCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);

	switch (pNMCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		break;

	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
	{
		if (pNMCD->nmcd.dwItemSpec % 2)
			pNMCD->clrTextBk = RGB(248, 248, 248);
		else
			pNMCD->clrTextBk = RGB(255, 255, 255);

		pNMCD->clrText = RGB(0, 0, 0);
		if (!m_Device.empty() && !blockUpdate)
		{
			for (auto &itt : m_Device)
			{
				if ((reinterpret_cast<device_Id*>(GetItemData(pNMCD->nmcd.dwItemSpec))) != nullptr)
				{
					if (itt.IdDevice == *(reinterpret_cast<device_Id*>(GetItemData(pNMCD->nmcd.dwItemSpec))))
					{
						if (itt.is_fictional_device)
						{
							SetItemState(pNMCD->nmcd.dwItemSpec, 0, LVIS_SELECTED);
							pNMCD->clrTextBk = RGB(255, 35, 0);
						}

						else if (!itt.is_fictional_device && itt.is_Connecting && !itt.is_our_device)
						{
							SetItemState(pNMCD->nmcd.dwItemSpec, 0, LVIS_SELECTED);
							pNMCD->clrTextBk = RGB(255, 136, 0);
						}
						else if (!itt.is_our_device)
						pNMCD->clrText = RGB(128, 128, 128);

						break;
					}
				}
			}
		}
		pNMCD->clrFace = pNMCD->clrTextBk;
		*pResult = CDRF_DODEFAULT;
	}
	break;

	case CDDS_ITEMPOSTPAINT:
		SetItemState(pNMCD->nmcd.dwItemSpec, 0xff, LVIS_SELECTED);

		*pResult = CDRF_DODEFAULT;
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CListCtrlDevice::OnMenuSelectOnOff()
{
	infoForMenu forMenu_;
	menu.OnMenuSelect(&forMenu_);

	if (forMenu_.iItem == -1)
		return;

	bool param(true);

	if (forMenu_.isConnecting == true)
		param = false;
	else
	{
		if (forMenu_.is_our_device)
			param = false;
	}

	if (menu.GetNumTab() == en_NumberTab::TabSensorsDevice && param && !CheckShield())
		return;

	ActivateDevice(param);
}

long CListCtrlDevice::CheckShield()
{
	char lerr = 0;
	if (Shield(24, &lerr) >= 0 && !(lerr & 1))
		return 1;
	else 
		AfxMessageBox(g_sForProgrammableComponents);
	return 0;
}

void CListCtrlDevice::OnMenuSelectChangeIp()
{
	infoForMenu forMenu_;
	menu.OnMenuSelect(&forMenu_);

	if (forMenu_.iItem == -1)
		return;

	ZdcpScanner::Device::Id device_id;
	device_id.serial = forMenu_.IdDevice.second;
	device_id.type = forMenu_.IdDevice.first;
	auto iface = commonWork.GetScanner().FindDeviceInterface(device_id);

	CString newMask(L"");

	union { ULONG ul; BYTE b[4]; } m;
	ConvertLengthToIpv4Mask(iface.netmask_length, &m.ul);
	newMask.Format(L"%u.%u.%u.%u", m.b[0], m.b[1], m.b[2], m.b[3]);

	std::string NextMask = CT2A(newMask.GetString());
	CChangeIp cchangeIp(forMenu_.ip, CString(commonWork.GetScanner().GetNextAddress(iface.addr_str.c_str(), NextMask).c_str()), newMask);
	cchangeIp.SetInfo(forMenu_.type, &commonWork.GetScanner());
	if (cchangeIp.DoModal() == IDOK) 
	{
		if (forMenu_.ip != cchangeIp.getIP() && !cchangeIp.getIP().IsEmpty())
			ChangeIPadress(cchangeIp.getIP(), cchangeIp.getMask());
	}
}

void CListCtrlDevice::ClearDevice()
{
	DeleteAllItems();
	m_Device.clear();
}

void CListCtrlDevice::pushDevice(const strCommonDevice& data)
{
	m_Device.push_back(data);
}

void CListCtrlDevice::SetNumTab(en_NumberTab::Number NumberTab)
{
	this->NumberTab = NumberTab;
}

void CListCtrlDevice::onReactionChooseDevice(const boost::function< void(int) >& f)
{
	ActivateDevice.connect(f);
}

void CListCtrlDevice::onReactionChangeIPadress(const boost::function< void(CString, CString) >& f)
{
	ChangeIPadress.connect(f);
}

void CListCtrlDevice::onReactionChangeOrderNumber(const boost::function< void(long) >& f)
{
	ChangeOrderNumber.connect(f);
}

void CListCtrlDevice::getArrayDevices(device_vector& Device)
{
	Device = m_Device;
}

void CListCtrlDevice::OnMenuSelectDeviceUp()
{
	ChangeOrderNumber(en_NumberMenuSelect::Up);
}

void CListCtrlDevice::OnMenuSelectDeviceDown()
{
	ChangeOrderNumber(en_NumberMenuSelect::Down);
}

BOOL CListCtrlDevice::OnNeedToolTip(UINT nID, NMHDR* pNMHDR, LRESULT* pResult)
{
	LPTOOLTIPTEXT pTTT = reinterpret_cast<LPTOOLTIPTEXT>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	CPoint ptMousePos = (CPoint)GetMessagePos();
	CPaintDC dc(this);
	ScreenToClient(&ptMousePos);
	UINT uFlags(0);
	int iItem(HitTest(ptMousePos, &uFlags));
	if (-1 == iItem)
		return FALSE;

	device_ptr info = GetItem(iItem);
	if (info == m_Device.end())
		return TRUE;

	cstrText = L"";

	CString ipDevice(L"");
	CString peerIp(L"");
	SIZE szDeviceIp; SIZE szPeerIp;
	int maxWidth(0);

	if (!info->IpDevice.IsEmpty() && info->IpDevice != L"0.0.0.0")
		ipDevice.Format(g_sIP_address + L": [%s]", info->IpDevice);

	GetTextExtentPoint32W(dc.GetSafeHdc(), ipDevice, ipDevice.GetLength(), &szDeviceIp);
	if (szDeviceIp.cx > maxWidth)
		maxWidth = szDeviceIp.cx;

	if (!info->peer_ip.IsEmpty() && info->peer_ip != L"0.0.0.0" && !info->is_our_device)
		peerIp.AppendFormat(g_sConnectTo + L": [%s]", info->peer_ip);

	GetTextExtentPoint32W(dc.GetSafeHdc(), peerIp, peerIp.GetLength(), &szPeerIp);
	if (szPeerIp.cx > maxWidth)
		maxWidth = szPeerIp.cx;


	if (szDeviceIp.cx < maxWidth && !ipDevice.IsEmpty())
	{
		auto diff(maxWidth - szDeviceIp.cx);
		for (int i = 0; i <= ceil(diff / 4); ++i)
			ipDevice += L" ";
	}

	if (szPeerIp.cx < maxWidth && !peerIp.IsEmpty())
	{
		auto diff(maxWidth - szPeerIp.cx);
		for (int i = 0; i <= ceil(diff / 4); ++i)
			peerIp += L" ";
	}

	if (!ipDevice.IsEmpty() || !peerIp.IsEmpty())
		cstrText.Format(L"%s %s ", ipDevice, peerIp);

	CToolTipCtrl* pToolTip = GetToolTips();
	if (pToolTip)
		pToolTip->SetMaxTipWidth(maxWidth);

	pTTT->lpszText = (LPTSTR)(LPCTSTR)cstrText;
	return TRUE;
}

long CListCtrlDevice::GetState(const strCommonDevice & itt)
{
	if (itt.is_our_device)
		return en_NumberImage::connect_ethernet;
	switch (itt.status)
	{
	case ZdcpScanner::Device::Status::AVAILABLE:
		return en_NumberImage::just_ethernet;
	case ZdcpScanner::Device::Status::CONNECTED:
		return en_NumberImage::ethernet_locked;
	case ZdcpScanner::Device::Status::CONFLICT:
		return en_NumberImage::ethernet_error;
	default:
		return en_NumberImage::just_ethernet;
	}
	return 0;
}

device_ptr CListCtrlDevice::GetItem(int Item)
{

	device_ptr ret(m_Device.end());

	if (Item < 0)
		return ret;
	
	IdDevice = *(reinterpret_cast<device_Id*>(GetItemData(Item)));
	device_Id _IdDevice = std::make_pair(IdDevice.first, IdDevice.second);
	ret = std::find_if(m_Device.begin(), m_Device.end(), [&_IdDevice](const strCommonDevice & s)
	{
		return s.IdDevice == _IdDevice;
	});

	return ret;
}

void CListCtrlDevice::SetItemImage(int iItem, int iImage)
{
	LVITEM lvi;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.mask = LVIF_IMAGE;
	lvi.iImage = iImage;
	SetItem(&lvi);
}

void CListCtrlDevice::OnDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem == -1)
		return;

	device_ptr info = GetItem(pNMItemActivate->iItem);
	if (info == m_Device.end())
		return;

	CDeviceInfo infoDevice;
	DeviceInfo data;
	data.ip = info->IpDevice;
	data.label = info->label;
	data.name = info->name;
	data.serial = info->IdDevice.second;
	data.type = info->IdDevice.first;
	data.status = info->status;
	data.isOurDevice = info->is_our_device;
	data.is_fictional_device = info->is_fictional_device;

	infoDevice.SetStrInfo(data);
	infoDevice.DoModal();
}

//------------------------------------------------------------------------------
void CListCtrlDevice::InsertGroup(const int&  val, const CString& str)
{
	LVGROUP lg = { 0 };
	lg.cbSize = sizeof(lg);
	lg.iGroupId = val;
	lg.state = LVGS_COLLAPSIBLE;
	lg.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
	lg.uAlign = LVGA_HEADER_LEFT;

	CString appendics(L"   ");
	appendics += str;
	lg.pszHeader = (LPWSTR)(LPCTSTR)appendics;
	lg.cchHeader = appendics.GetLength();

	CListCtrl::InsertGroup(val, (PLVGROUP)&lg);
}

//------------------------------------------------------------------------------
void CListCtrlDevice::SetItemTextInList(const strCommonDevice& str)
{
	LVITEM lvi;
	lvi.iItem = 2147483637;
	lvi.iSubItem = 0;
	lvi.pszText = NULL;
	lvi.cchTextMax = 0;
	lvi.mask = LVIF_IMAGE | LVIF_INDENT | LVIF_GROUPID;
	lvi.iIndent = 1;

	if (str.is_our_device || str.is_Connecting)
		lvi.iGroupId = GroupOurDevices;
	else
		lvi.iGroupId = GroupOtherDevices;
	 
	lvi.iImage = GetState(str);
	int nItem = InsertItem(&lvi);

	SetItemText(nItem, Col_Type, str.name);
	SetItemText(nItem, Col_Name, str.label);

	SetItemText(nItem, Col_Status, str.Connceted);
	SetItemData(nItem, reinterpret_cast<DWORD_PTR>(&str.IdDevice));
}

#pragma endregion 