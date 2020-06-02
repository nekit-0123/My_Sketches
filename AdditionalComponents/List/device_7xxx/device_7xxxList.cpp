#include "stdafx.h"
#include "device_7xxxList.h"
#include "..\..\..\ZdcpScanner.h"

#pragma region TabSensorsDevice
Cdevice_7xxxist::Cdevice_7xxxist()
	:_connectingImage(0),
	exitFromtread(false),
	treadIsRunning(false),
	param(true),
	countFictionDevice(0)
{}

BEGIN_MESSAGE_MAP(Cdevice_7xxxist, CListCtrlDevice)
	ON_MESSAGE(WM_UPDATE_7xxxLIST, &Cdevice_7xxxist::OnMessageThread)
END_MESSAGE_MAP()

Cdevice_7xxxist::~Cdevice_7xxxist()
{}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::Sort(_In_ const std::vector<CString>& data)
{
	std::sort(m_Device.begin(), m_Device.end(), [&](const strCommonDevice &i, const strCommonDevice &j)
	{
		return (i.status < j.status);
	});

	std::sort(m_Device.begin(), m_Device.end(), [&](const strCommonDevice &i, const strCommonDevice &j)
	{
		return (i.is_our_device && !j.is_our_device);
	});

	std::sort(m_Device.begin(), m_Device.end(), [&](const strCommonDevice &i, const strCommonDevice &j)
	{
		if (i.is_our_device)
		{
			return (!i.is_fictional_device && j.is_fictional_device);
		}
		else
			return false;
	});
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::SetConnectingDevice(const Connecting7xxxptr& data)
{
	ConnectingFromXml.emplace_back(data);
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::ClearConnectingFromXml()
{
	ConnectingFromXml.clear();
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::setDataToList(_In_ const std::vector<CString>& data)
{
	// Локальная копия объекта ConnectingFromXml"
	vecConnecting7xxx m_ConnectingFromXml(ConnectingFromXml.size());
	std::copy(ConnectingFromXml.begin(), ConnectingFromXml.end(), m_ConnectingFromXml.begin());

	Sort(data);

	bool m_bStartConnecting(false);
	for (auto &itt : m_Device)
	{
		auto find = find_if(m_ConnectingFromXml.begin(), m_ConnectingFromXml.end(), [&](Connecting7xxxptr id)
		{
			return id.first.serial == itt.IdDevice.second && (uint32_t)id.first.type == itt.IdDevice.first;
		});
		if (find != m_ConnectingFromXml.end())
		{
			itt.is_Connecting = true;
			m_bStartConnecting = true;
		}
	}

	if (std::any_of(m_Device.begin(), m_Device.end(), [](strCommonDevice str) {
		return str.is_our_device || str.is_Connecting;
	}))
		InsertGroup(GroupOurDevices, g_sConnectDevice);
	if (std::any_of(m_Device.begin(), m_Device.end(), [](strCommonDevice str) {
		return !str.is_our_device || str.is_Connecting;
	}))
		InsertGroup(GroupOtherDevices, g_sAllowDevice);

	for (auto &itt : m_Device)
	{
		auto find = find_if(m_ConnectingFromXml.begin(), m_ConnectingFromXml.end(), [&](Connecting7xxxptr id)
		{
			return id.first.serial == itt.IdDevice.second && (uint32_t)id.first.type == itt.Type;
		});
		if (find != m_ConnectingFromXml.end())	
			m_ConnectingFromXml.erase(find);

		SetItemTextInList(itt);
	}

	// Кто - то остался в xml файле, но не нашелся в сканнере
	if (!m_ConnectingFromXml.empty() && param)
	{
		countFictionDevice = 0;
		for (auto &itt : m_ConnectingFromXml)
		{
			InsertTurnOffDevice(itt);
			m_bStartConnecting = true;
			++countFictionDevice;
		}
	}

	if (m_bStartConnecting)
		StartConnecting(true);
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::OnRButtonDown(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem == -1)
		return;

	device_ptr info = GetItem(pNMItemActivate->iItem);
	if (info == m_Device.end())
		return;

	infoForMenu forMenu_;
	forMenu_.IdDevice.first = info->IdDevice.first;
	forMenu_.IdDevice.second = info->IdDevice.second;
	forMenu_.status = info->status;
	forMenu_.ip = info->IpDevice;
	forMenu_.iItem = pNMItemActivate->iItem;
	forMenu_.type = info->name;
	menu = en_NumberTab::TabSensorsDevice;
	forMenu_.is_our_device = info->is_our_device;
	forMenu_.isConnecting = info->is_Connecting;
	forMenu_.enaleChangeIp = (info->status != ZdcpScanner::Device::Status::CONNECTED);

	menu.Add(forMenu_);

	ClientToScreen(&pNMItemActivate->ptAction);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pNMItemActivate->ptAction.x, pNMItemActivate->ptAction.y, this);

	*pResult = 0;
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::StartConnecting(bool started, int item /*= -1*/)
{
	device_ptr info = GetItem(item);
	if (info != m_Device.end())
		info->is_Connecting = started ? true : false;
	if (started)
	{
		_connectingImage = en_NumberImage::connecting1;
		if (!updateIco.joinable())
		{
			updateIco = std::thread(&Cdevice_7xxxist::UpdateConnecting, this);
		}
	}
	else if (info != m_Device.end())
		SetItemImage(item, GetState(*info));
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::UpdateConnecting()
{
	exitFromtread = false;
	treadIsRunning = true;
	while (true)
	{	
		if (exitFromtread)
		{
			g_queuecheck7xxx.notify_one();
			break;
		}		
		
		if (blockUpdate)
		{
			Sleep(100);
			continue;
		}

		PostMessage(WM_UPDATE_7xxxLIST);
		++_connectingImage;
		if (_connectingImage > en_NumberImage::connecting4)
			_connectingImage = en_NumberImage::connecting1;

		bool b_checkIsConnecting(false);
		for (auto &itt : m_Device)
			if (itt.is_Connecting && !itt.is_our_device)
			{
				b_checkIsConnecting = true;
				break;
			}

		if (!b_checkIsConnecting)
		{
			break;
		}

		Sleep(100);
	}
	updateIco.detach();
	treadIsRunning = false;
}

//------------------------------------------------------------------------------
LRESULT Cdevice_7xxxist::OnMessageThread(WPARAM, LPARAM)
{
	for (int iItem = 0; iItem < GetItemCount(); ++iItem)
	{
		for (auto &itt : m_Device)
		{
			if ((reinterpret_cast<device_Id*>(GetItemData(iItem))) != nullptr)
			{
				if (itt.IdDevice == *(reinterpret_cast<device_Id*>(GetItemData(iItem))))
				{
					if (itt.is_Connecting && !itt.is_our_device)
					{
						SetItemImage(iItem, _connectingImage);

						if (GetItemText(iItem, Col_Status) != g_sConnected + L"..." && !itt.is_fictional_device)
							SetItemText(iItem, Col_Status, g_sConnected + L"...");

						break;
					}
				}
			}
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::EndConnecting()
{
	if (treadIsRunning)
	{
		exitFromtread = true;
		std::unique_lock<std::mutex> locker(g_lockExit7xxx);
		g_queuecheck7xxx.wait_for(locker, TimeOut);
	}
}

//------------------------------------------------------------------------------
void Cdevice_7xxxist::onInsertDevice(const boost::function< void(const Connecting7xxxptr&) >& f)
{
	InsertTurnOffDevice.connect(f);
}
//------------------------------------------------------------------------------
#pragma endregion