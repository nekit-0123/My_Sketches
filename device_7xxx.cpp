#include "stdafx.h"
#include "device_7xxx.h"
#include "CMainTabDlg.h"
#include <ZETFile\include\ZETFile.h>
#include <ZET7xxx\include\base\ZET7xxxDefines.h>
#include <boost/bind.hpp>

IMPLEMENT_DYNAMIC(device_7xxx, CDialogEx)

device_7xxx::device_7xxx(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_7XXX_TAB, pParent)
	, cfgpath(L"")
	, m_bExitFromThread(false)
{}

device_7xxx::~device_7xxx()
{
	thread.interrupt();
	if (m_config7xxx != nullptr)
		m_config7xxx->stop();
}

//------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(device_7xxx, CDialogEx)
	ON_MESSAGE(WM_UPDATE_7xxxDevice, &device_7xxx::OnMessageThread)
END_MESSAGE_MAP()

//------------------------------------------------------------------------------
void device_7xxx::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

//------------------------------------------------------------------------------
BOOL device_7xxx::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_list = std::make_unique<Cdevice_7xxxist>();
	CRect rectNull;
	m_list->Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, rectNull, this, ID_LIST_STATE);
	SetWindowTheme(m_list->m_hWnd, L"explorer", 0);
	m_list->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_list->AddColumn();
	m_list->SetNumTab(en_NumberTab::TabSensorsDevice);

	thread = boost::thread(&device_7xxx::Check7xxxDevice, this);
	m_list->onReactionChooseDevice(boost::bind(&device_7xxx::ReactionChoosingDevice, this, _1));
	m_list->onReactionChangeIPadress(boost::bind(&device_7xxx::ReactionChangeIP, this, _1, _2));
	m_list->onInsertDevice(boost::bind(&device_7xxx::FormingNewDevice, this, _1));
	m_list->EnableGroupView(TRUE);

	m_config7xxx = std::make_unique<zet76::config>();

	BSTR bstrKey(L"DirConfig");
	CMainTabDlg* pParent = dynamic_cast<CMainTabDlg*>(GetParent());
	if (pParent != nullptr && pParent->GetPath() != nullptr)
			cfgpath = pParent->GetPath()->ZetPath(&bstrKey);

	m_config7xxx->start(cfgpath + L"zet76.xml", std::bind(&device_7xxx::followChangeConfig, this, std::placeholders::_1));

	return TRUE;
}

//------------------------------------------------------------------------------
long device_7xxx::Check7xxxDevice()
{
	auto duration = boost::posix_time::milliseconds(100);
	long count = 100 < 1000 ? (1000 / 100) : 1;

	try
	{
		while (true)
		{
			if (m_bExitFromThread)
				break;

			bool bflag(false);
			m_list->getCommonClass().GetScanner().GetDeviceList(&m_devices, ZdcpScanner::Device::Type::ZETSENSOR);
			m_list->getCommonClass().GetScanner().GetInterfaceList(&m_interfaces);
			// Проверка на добавление/отключение устройства
			if (m_lFindCountDevices != m_devices.size() + m_list->GetCountFiction())
			{
				m_lFindCountDevices = m_devices.size() + m_list->GetCountFiction();
				bflag = true;
			}

			if (!bflag)
			{
				for (const auto& device : m_devices)
				{
					device_Id _deviceId;
					_deviceId.first = device.first.type.value;
					_deviceId.second = device.first.serial;

					device_ptr find = GetItem(_deviceId);
					if (find != vecDevice7xxx.end())
					{
						if (m_list->getCommonClass().CheckStruct(find, device.second) == true)
							bflag = true;
					}
				}
			}

			if (bflag)
				PostMessage(WM_UPDATE_7xxxDevice);


			for (long ticks = 0; ticks < count; ++ticks)
			{
				boost::this_thread::sleep(duration);
			}
		}
	}
	catch (boost::thread_interrupted&)
	{}

	thread.joinable();
	m_queueExit7xxx.notify_one();

	return 0;
}

//------------------------------------------------------------------------------
LRESULT device_7xxx::OnMessageThread(WPARAM, LPARAM)
{
	m_list->SetBlockUpdate(true);
	m_list->SetRedraw(FALSE);

	m_list->EndConnecting();
	m_list->ClearDevice();
	for (const auto& device : m_devices)
	{
		strCommonDevice _device;
		m_list->getCommonClass().GenerateNameForList(device, m_interfaces, &_device);
		m_list->pushDevice(_device);
	}

	m_list->setDataToList();
	m_list->SetRedraw(TRUE);
	m_list->Invalidate();
	m_list->SetBlockUpdate(false);

	return 0;
}

//------------------------------------------------------------------------------
void device_7xxx::ReactionChoosingDevice(_In_ int enabled)
{
	device_Id _IdDevice;
	m_list->GetDeviceId(&_IdDevice);

	if (helpClass.ShellFindProcesses(L"MODBUSZETLAB.exe") == false)
		ShellExecute(NULL, _T("open"), L"MODBUSZETLAB.exe", L"", _T(""), SW_HIDE);

	zet76::device::id addDevice(_IdDevice.first, _IdDevice.second);
	zet76::device::mode mode;
	mode = enabled ? zet76::device::mode::CONNECT : zet76::device::mode::NOTHING;
	m_config7xxx->set_mode(addDevice, mode);

	device_ptr info = GetItem(_IdDevice);
	if (info == vecDevice7xxx.end())
		return;
	m_config7xxx->set_label(addDevice, info->label.GetString());

	if (mode == zet76::device::mode::NOTHING && info->is_fictional_device)
		m_list->SetCountFiction(m_list->GetCountFiction() - 1);

	PostMessage(WM_UPDATE_7xxxDevice);
}

//------------------------------------------------------------------------------
void device_7xxx::ReactionChangeIP(_In_ const CString& newIP, _In_ const CString& newMask)
{
	device_Id _IdDevice;
	m_list->GetDeviceId(&_IdDevice);

	device_ptr info = GetItem(_IdDevice);
	if (info == vecDevice7xxx.end())
		return;

	ZdcpScanner::Device::Id id;
	id.type = info->Type;
	id.serial = info->IdDevice.second;

	std::string ip_str = CW2A(newIP);
	std::string netmask_str = CW2A(newMask);
	m_list->getCommonClass().GetScanner().RequestChangeAddress(id, ip_str, netmask_str);
}

//------------------------------------------------------------------------------
device_ptr device_7xxx::GetItem(const device_Id& _IdDevice)
{
	m_list->getArrayDevices(vecDevice7xxx);
	auto find = std::find_if(vecDevice7xxx.begin(), vecDevice7xxx.end(), [&_IdDevice](strCommonDevice& deviceId)
	{
		return _IdDevice == deviceId.IdDevice;
	});
	return find;
}

//------------------------------------------------------------------------------
Cdevice_7xxxist *device_7xxx::GetList()
{
	return m_list.get();
}

//------------------------------------------------------------------------------
void device_7xxx::followChangeConfig(void* arg)
{
	m_list->ClearConnectingFromXml();
	zet76::config::mode_db db; zet76::config::mode_db diff;
	m_config7xxx->get_mode_db(&db, &diff);

	for (auto&itt : diff)
		m_list->SetConnectingDevice(itt);
}

//------------------------------------------------------------------------------
void device_7xxx::StopThread()
{
	if (thread.joinable())
	{
		m_bExitFromThread = true;
		std::unique_lock<std::mutex> locker(m_lockExit7xxx);
		m_queueExit7xxx.wait_for(locker, TimeOut);
	}
}

//------------------------------------------------------------------------------
// Создадим вымешленное устройство на основе xml файла
void device_7xxx::FormingNewDevice(const Connecting7xxxptr& itt)
{
	strCommonDevice 	m_strCommonDevice;
	m_strCommonDevice.IdDevice.first = (uint32_t)itt.first.type;
	m_strCommonDevice.IdDevice.second = itt.first.serial;
	m_strCommonDevice.label = itt.second.label.c_str();
	m_strCommonDevice.Connceted = g_sOffdevice;
	m_strCommonDevice.is_Connecting = true;
	m_strCommonDevice.is_fictional_device = true;

	CString type(L"");
	switch (itt.first.type)
	{
	case zet76::device::type::ZET_7176:
		type = L"ZET_7176";
		break;
	case zet76::device::type::ZET_7076:
		type = L"ZET_7076";
		break;
	case zet76::device::type::ZET_7177:
		type = L"ZET_7177";
		break;
	default:
		type = L"UNKNOWN";
		break;
	}

	m_strCommonDevice.name.Format(L"%s %s%016llx",
		type.GetString(),
		g_sNo.GetString(),
		itt.first.serial);


	m_list->SetBlockUpdate(true);
	m_list->pushDevice(m_strCommonDevice);
	m_list->DeleteAllItems();
	m_list->SetBlockCheckxml(false);
	m_list->setDataToList();
	m_list->SetBlockCheckxml(true);
	m_list->SetBlockUpdate(false);
}
//------------------------------------------------------------------------------