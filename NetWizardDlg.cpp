#pragma once
#include "stdafx.h"
#include "NetWizard.h"
#include "NetWizardDlg.h"
#include "CMainTabDlg.h"
#include <ZETFile/include/ZETFile.h>
#include <ADCINFO.h>
#include <Zadc.h>

//------------------------------------------------------------------------------
CNetWizardDlg::CNetWizardDlg(CWnd* pParent)
	: CDialog(CNetWizardDlg::IDD, pParent)
	, m_lFindCountDevices(0)
	, m_bExitFromThread(false)
{
	std::fill(typeDevice, typeDevice + MAX_DEVICE,			 KDU8500);
	std::fill(numberDSP,  numberDSP + MAX_DEVICE,			 FIRST_NUMBER_DSP);
	std::fill(open_timeout, open_timeout + MAX_DEVICE,		 60000);
	std::fill(command_timeout, command_timeout + MAX_DEVICE, 1000);
	std::fill(ipaddress, ipaddress + MAX_DEVICE,			 L"0.0.0.0");
	std::fill(port_number, port_number + MAX_DEVICE,		 1808);

	ZeroMemory(serial, sizeof(serial));
	ZeroMemory(type, sizeof(type));
}

CNetWizardDlg::~CNetWizardDlg()
{
	threadAnaliz.interrupt();
}

void CNetWizardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNetWizardDlg, CDialog)
	ON_MESSAGE(WM_UPDATE_AnalizDevice, &CNetWizardDlg::OnMessageThread)
END_MESSAGE_MAP()

//------------------------------------------------------------------------------
BOOL CNetWizardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_list = std::make_unique<Cdevice_analizatorList>();
	CRect rectNull;
	m_list->Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL, rectNull, this, ID_LIST_STATE);
	SetWindowTheme(m_list->m_hWnd, L"explorer", 0);
	m_list->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_list->AddColumn();
	m_list->SetNumTab(en_NumberTab::TabAnalizDevice);

	BSTR bstrKey(L"DirConfig");
	CMainTabDlg* pParent = dynamic_cast<CMainTabDlg*>(GetParent());
	if (pParent != nullptr && pParent->GetPath()!= nullptr)
		cfgpath = pParent->GetPath()->ZetPath(&bstrKey);

	m_list->onReactionChooseDevice(boost::bind(&CNetWizardDlg::ReactionChoosingDevice, this, _1));
	m_list->onReactionChangeIPadress(boost::bind(&CNetWizardDlg::ReactionChangeIP, this, _1, _2));
	m_list->onReactionChangeOrderNumber(boost::bind(&CNetWizardDlg::ReactionChangeOrderNum, this,_1));
	m_list->onInsertDevice(boost::bind(&CNetWizardDlg::FormingNewDevice, this, _1));
	m_list->EnableGroupView(TRUE);
	threadAnaliz = boost::thread(&CNetWizardDlg::CheckDevices, this);

	return TRUE;
}

//------------------------------------------------------------------------------
void CNetWizardDlg::ReadAndCheckCfgNetServer()
{
	long m_index(0);
	ReadNetServCfgFile(m_index);
	ReadAndCheckCfg(m_index);
	ReadAllDeviceAtXml();
	if (helpClass.ShellFindProcesses(L"NetServer.exe") == false)
		ShellExecute(NULL, _T("open"), L"NetServer.exe", L"", _T(""), SW_HIDE);
}

//------------------------------------------------------------------------------
void CNetWizardDlg::WriteAndCheckCfgNetServer()
{
	WriteNetServCfgFile();
	ReadAllDeviceAtXml();
	if (helpClass.ShellFindProcesses(L"NetServer.exe") == false)
		ShellExecute(NULL, _T("open"), L"NetServer.exe", L"", _T(""), SW_HIDE);
}

//------------------------------------------------------------------------------
void CNetWizardDlg::WriteNetServCfgFile()
{
	pugi::xml_document xmlDoc; pugi::xml_node xmlNode;

	zfDeleteFileW(cfgpath + L"netserverNew.cfg");

	if (!xmlDoc.load_file(cfgpath + L"netserverNew.cfg", parse_default, encoding_utf8))
	{
		xmlDoc.load(_T(""));
		xmlDoc.append_child(_T("Config"));
	}
	xmlNode = xmlDoc.first_child();

	if (xmlNode.attribute(_T("version")).as_double() < CONFIG_VERSION)
	{
		xmlDoc.remove_child(xmlNode);
		xmlNode = xmlDoc.append_child(_T("Config"));
	}

	if (!xmlNode.attribute(_T("version")))
		xmlNode.append_attribute(_T("version"));
	xmlNode.attribute(_T("version")).set_value(CONFIG_VERSION);

	pugi::xml_node xmlNodeDevice;

	for (long i = 0; i < MAX_DEVICE; i++)
	{
		if (!ipaddress[i].IsEmpty() && ipaddress[i] != L"0.0.0.0")
		{
			xmlNodeDevice = xmlNode.append_child(_T("Device"));

			if (CheckCfg(i, xmlNode))
			{
				xmlNode.remove_child(xmlNodeDevice);
			}

			append_child_attr(xmlNodeDevice, Type, typeDevice[i]);
			append_child_attr(xmlNodeDevice, Dsp, numberDSP[i]);
			append_child_attr(xmlNodeDevice, Open_timeout, open_timeout[i]);
			append_child_attr(xmlNodeDevice, Command_timeout, command_timeout[i]);
			append_child_attr(xmlNodeDevice, IpAddress, ipaddress[i]);
			append_child_attr(xmlNodeDevice, Port_number, port_number[i]);
			append_child_attr(xmlNodeDevice, Serial, serial[i]);
			append_child_attr(xmlNodeDevice, Label, label[i]);
			append_child_attr(xmlNodeDevice, ScanType, type[i]);


		}
	}
	xmlDoc.save_file(cfgpath + L"netserverNew.cfg", _T("    "), format_default, encoding_utf8);
}

//------------------------------------------------------------------------------
bool CNetWizardDlg::CheckCfg(_In_ const long i, _In_ const pugi::xml_node& parent)
{
	pugi::xml_node node;
	for (node = parent.child(_T("Device")); node; node = node.next_sibling(_T("Device")))
	{
		if (ipaddress[i] == node.attribute(_T("IpAddress")).value())
			return true;
	}
	return false;
}

//------------------------------------------------------------------------------
void CNetWizardDlg::ReadNetServCfgFile(long& index)
{
	pugi::xml_document xmlDoc; pugi::xml_node xmlNode;
	pugi::xml_node xmlNodeDevice;
	xmlDoc.load_file(cfgpath + L"netserverNew.cfg");

	if (!xmlDoc.empty())
	{
		xmlNode = xmlDoc.child(L"Config");

		xmlNodeDevice = xmlNode.child(L"Device");
		while (xmlNodeDevice != nullptr && index < MAX_DEVICE)
		{
			read_child_attr(xmlNodeDevice, Type, typeDevice[index]);
			read_child_attr(xmlNodeDevice, Dsp, numberDSP[index]);
			read_child_attr(xmlNodeDevice, Open_timeout, open_timeout[index]);
			read_child_attr(xmlNodeDevice, Command_timeout, command_timeout[index]);
			read_child_attrV(xmlNodeDevice, IpAddress, ipaddress[index]);
			read_child_attr(xmlNodeDevice, Port_number, port_number[index]);
			read_child_attr(xmlNodeDevice, Serial, serial[index]);
			read_child_attrV(xmlNodeDevice, Label, label[index]);
			read_child_attr(xmlNodeDevice, ScanType, type[index]);

			++index;
			xmlNodeDevice = xmlNodeDevice.next_sibling(L"Device");
		}
	}
}

//------------------------------------------------------------------------------
// Функция из NetServer
void CNetWizardDlg::ReadAndCheckCfg(const long& index)
{
	FILE *out(nullptr);
	wchar_t buffer_temp[100] = { 0 };
	wchar_t *token(nullptr);
	wchar_t *context(nullptr);
	wchar_t seps[] = L"\n";
	out = _wfsopen(cfgpath + L"netserver.cfg", L"rt", _SH_DENYNO);
	if (out != NULL)
	{
		long Quantity(0);
		while (!feof(out))
		{
			fgetws(buffer_temp, 100, out);
			token = wcstok_s(buffer_temp, seps, &context);
			if (wcslen(buffer_temp) > 2)
				Quantity++;
			wcscpy_s(buffer_temp, 100, L"");
		}

		fseek(out, 0L, SEEK_SET);

		for (int i = index; i < index + Quantity; ++i)
		{
			if (i >= MAX_DEVICE)
				break;

			fwscanf_s(out, L"%d", &typeDevice[i]);
			fwscanf_s(out, L"%d", &numberDSP[i]);
			fwscanf_s(out, L"%d", &open_timeout[i]);
			fwscanf_s(out, L"%d", &command_timeout[i]);
			fwscanf_s(out, L"%s", buffer_temp, 99);
			ipaddress[i] = buffer_temp;
			fwscanf_s(out, L"%d", &port_number[i]);
		}
		fclose(out);
	}
	zfDeleteFileW(cfgpath + L"netserver.cfg");
}

//------------------------------------------------------------------------------
CString CNetWizardDlg::getInterfaces()
{
	ZdcpScanner::InterfaceList list;
	m_list->getCommonClass().GetScanner().GetInterfaceList(&list);

	if (list.empty())
	{
		return g_sNoNetworkConnections;
	}

	CString s(L"");

	if (list.size() == 1)
	{
		union { ULONG ul; BYTE b[4]; } m;
		ConvertLengthToIpv4Mask(list.front().netmask_length, &m.ul);
		s.Format(L"%s: %hs, %s: %u.%u.%u.%u",
			g_sIP_addressComputer.GetString(),
			list.front().addr_str.c_str(),
			g_sSubnetMask.GetString(), m.b[0], m.b[1], m.b[2], m.b[3]);
	}
	else
	{
		s.Format(L"%s: %hs/%u",
			g_sIP_address_Computer.GetString(),
			list.front().addr_str.c_str(),
			list.front().netmask_length);
		for (size_t i = 1; i < list.size(); ++i)
		{
			s.AppendFormat(L", %hs/%u",
				list[i].addr_str.c_str(),
				list[i].netmask_length);
		}
	}

	return s;
}

//------------------------------------------------------------------------------
Cdevice_analizatorList *CNetWizardDlg::GetList()
{
	return m_list.get();
}

//------------------------------------------------------------------------------
vecIpAdress CNetWizardDlg::FormingForSort()
{
	vecIpAdress data;
	for (long i = 0; i < MAX_DEVICE; ++i)
	{
		if (ipaddress[i] != L"0.0.0.0" && !ipaddress[i].IsEmpty())
		{
			m_list->getArrayDevices(m_Device);

			auto find = std::find_if(m_Device.begin(), m_Device.end(), [&](strCommonDevice& deviceId)
			{
				return ipaddress[i] == deviceId.IpDevice;
			});
			data.push_back(ipaddress[i]);
		}
	}
	return data;
}

//------------------------------------------------------------------------------
void CNetWizardDlg::CheckDevices()
{
	auto duration = boost::posix_time::milliseconds(100);
	long count = 100 < 1000 ? (1000 / 100) : 1;
	try
	{
		while (1)
		{
			if (m_bExitFromThread)
				break;

			bool bflag(false);
			uint32_t filter =
					ZdcpScanner::Device::Type::Category::ANALYSIS |
					ZdcpScanner::Device::Type::Category::TENSO |
					ZdcpScanner::Device::Type::Category::SEISMO;
			m_list->getCommonClass().GetScanner().GetDeviceList(&m_devices, filter);
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
					if (find != m_Device.end())
					{
						if (m_list->getCommonClass().CheckStruct(find, device.second) == true)
							bflag = true;
					}
				}
			}

			if (bflag)
				PostMessage(WM_UPDATE_AnalizDevice);

			for (long ticks = 0; ticks < count; ++ticks)
			{
				boost::this_thread::sleep(duration);
			}
		}
	}
	catch (boost::thread_interrupted&)
	{}

	threadAnaliz.joinable();
	m_queueExitAnaliz.notify_one();
}

//------------------------------------------------------------------------------
LRESULT CNetWizardDlg::OnMessageThread(WPARAM, LPARAM)
{
	m_list->SetBlockUpdate(true);
	m_list->SetRedraw(FALSE);
	std::wstring peer_name(L"");
	m_list->EndConnecting();
	m_list->ClearDevice();
	for (const auto& device : m_devices)
	{
		strCommonDevice _device;
		m_list->getCommonClass().GenerateNameForList(device, m_interfaces, &_device);
		m_list->pushDevice(_device);
	}

	m_list->setDataToList(FormingForSort());
	m_list->SetRedraw(TRUE);
	m_list->Invalidate();
	m_list->SetBlockUpdate(false);

	return 0;
}

//------------------------------------------------------------------------------
void CNetWizardDlg::ReactionChoosingDevice(_In_ int our_device)
{
	device_Id _IdDevice;
	m_list->GetDeviceId(&_IdDevice);

	device_ptr info = GetItem(_IdDevice);
	if (info == m_Device.end())
		return;

	CString findIp(info->IpDevice);
	// Нас отключают
	if (info->IpDevice.IsEmpty() || !our_device)
	{
		// Поиск по Ip, то что сохраняется в файл	
		auto itFound = std::find_if(ipaddress, ipaddress + MAX_DEVICE, [&findIp](CString &ip)
		{
			return (findIp == ip);
		});
		if (itFound != ipaddress + MAX_DEVICE)
		{
			info->IpDevice = L"0.0.0.0";
			ipaddress[itFound - ipaddress] = info->IpDevice;

			if (info->is_fictional_device)
				m_list->SetCountFiction(m_list->GetCountFiction() - 1);
		}
	}
	else// Нас включают
	{
		if (std::find_if(ipaddress, ipaddress + MAX_DEVICE, [&findIp](CString &ip)
		{
			return (findIp == ip);
		}) == ipaddress + MAX_DEVICE)
		{
			auto itFound = std::find_if(ipaddress, ipaddress + MAX_DEVICE, [](CString &ip)
			{
				return (ip == L"0.0.0.0");
			});
			if (itFound != ipaddress + MAX_DEVICE)
			{
				ipaddress[itFound - ipaddress] = findIp;
				typeDevice[itFound - ipaddress] = info->Type;

				type[itFound - ipaddress] = info->IdDevice.first;
				serial[itFound - ipaddress] = info->IdDevice.second;
				label[itFound - ipaddress] = info->label;
			}
		}
	}

	WriteAndCheckCfgNetServer();
	PostMessage(WM_UPDATE_AnalizDevice);
}

//------------------------------------------------------------------------------
void CNetWizardDlg::ReactionChangeIP(_In_ const CString& newIP, _In_ const CString& newMask)
{
	device_Id _IdDevice;
	m_list->GetDeviceId(&_IdDevice);

	device_ptr info = GetItem(_IdDevice);
	if (info == m_Device.end())
		return;

	if (helpClass.ShellFindProcesses(L"NetServer.exe") == false)
		ShellExecute(NULL, _T("open"), L"NetServer.exe", L"", _T(""), SW_HIDE);

	ZdcpScanner::Device::Id id;
	id.type = info->IdDevice.first;
	id.serial = info->IdDevice.second;

	std::string ip_str = CW2A(newIP);
	std::string netmask_str = CW2A(newMask);
	m_list->getCommonClass().GetScanner().RequestChangeAddress(id, ip_str, netmask_str);
}

//------------------------------------------------------------------------------
void CNetWizardDlg::ReactionChangeOrderNum(_In_ long position)
{
	device_Id _IdDevice;
	m_list->GetDeviceId(&_IdDevice);
	// Нас передвигают

	device_ptr info = GetItem(_IdDevice);
	if (info == m_Device.end())
		return;

	// иначе выйдем за диапазаон
	if (position == en_NumberMenuSelect::Up && info == m_Device.begin())
		return;
	if (position == en_NumberMenuSelect::Down && info == m_Device.end())
		return;

	CString csDev1(info->IpDevice);
	auto findSecond = position == en_NumberMenuSelect::Up ? std::prev(info) : std::next(info);
	CString csDev2(findSecond->IpDevice);

	auto DeviceFirst = std::find_if(ipaddress, ipaddress + MAX_DEVICE, [&csDev1](CString &ip)
	{
		return (csDev1 == ip);
	});
	auto DeviceSecond = std::find_if(ipaddress, ipaddress + MAX_DEVICE, [&csDev2](CString &ip)
	{
		return (csDev2 == ip);
	});

	std::swap(*findSecond, *info);
	std::swap(*DeviceFirst, *DeviceSecond);

	WriteAndCheckCfgNetServer();
}

//------------------------------------------------------------------------------
device_ptr CNetWizardDlg::GetItem(const device_Id& _IdDevice)
{
	m_list->getArrayDevices(m_Device);
	auto find = std::find_if(m_Device.begin(), m_Device.end(), [&_IdDevice](strCommonDevice& deviceId)
	{
		return _IdDevice == deviceId.IdDevice;
	});
	return find;
}

//------------------------------------------------------------------------------
void CNetWizardDlg::StopThread()
{
	if (threadAnaliz.joinable())
	{
		m_bExitFromThread = true;
		std::unique_lock<std::mutex> locker(m_lockExitAnaliz);
		m_queueExitAnaliz.wait_for(locker, TimeOut);
	}
}

//------------------------------------------------------------------------------
void CNetWizardDlg::ReadAllDeviceAtXml()
{
	m_list->ClearConnectingFromXml();
	for (long i = 0; i < MAX_DEVICE; ++i)
	{
		if (ipaddress[i] != L"0.0.0.0" && type[i] !=0 && serial[i]!=0)
		{
			connectingfInfo data;
			data.IdDevice.first = type[i];
			data.IdDevice.second = serial[i];
			data.label = label[i];
			data.ipDevice = ipaddress[i];
			data.Type = typeDevice[i];

			ZdcpScanner::Device::Type t;
			data.name = t.GetNameW(data.IdDevice.first);
			m_list->SetConnectingDevice(data);
		}
	}
}

//------------------------------------------------------------------------------
void CNetWizardDlg::FormingNewDevice(const connectingfInfo& itt)
{
	strCommonDevice m_strCommonDevice;

	m_strCommonDevice.IdDevice.first  = itt.IdDevice.first;
	m_strCommonDevice.IdDevice.second = itt.IdDevice.second;
	m_strCommonDevice.label			  = itt.label;
	m_strCommonDevice.Connceted		  = g_sOffdevice;
	m_strCommonDevice.is_Connecting   = true;
	m_strCommonDevice.IpDevice		  = itt.ipDevice;
	m_strCommonDevice.Type			  = itt.Type;
	m_strCommonDevice.is_fictional_device = true;

	m_strCommonDevice.name.Format(L"%s %s%llu",
		itt.name,
		g_sNo.GetString(),
		itt.IdDevice.second);

	m_list->SetBlockUpdate(true);
	m_list->pushDevice(m_strCommonDevice);
	m_list->DeleteAllItems();
	m_list->SetBlockCheckxml(false);
	m_list->setDataToList(FormingForSort());
	m_list->SetBlockCheckxml(true);
	m_list->SetBlockUpdate(false);
}
