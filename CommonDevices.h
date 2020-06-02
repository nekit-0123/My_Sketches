#pragma once
#include "ZdcpScanner.h"
#include "res\Phrases.h"

typedef std::vector<strCommonDevice>::iterator  device_ptr;
typedef std::vector<strCommonDevice>		    device_vector;
typedef std::pair<uint32_t, uint64_t>			device_Id;

class CCommonDevices
{
public:
	CCommonDevices::CCommonDevices()
	{
		// Запустить сканирование
		m_scanner.Start();
	}
	CCommonDevices::~CCommonDevices()
	{
		m_scanner.Stop();
	}

	//------------------------------------------------------------------------------
	bool CheckStruct(_In_ const device_ptr& left, _In_ const ZdcpScanner::Device &right)
	{
		if (left->peer_ip != right.peer_ip.c_str())
			return true;

		if (left->IpDevice != right.ip.c_str())
			return true;

		if (left->status != right.status)
		{
			if (right.status == ZdcpScanner::Device::Status::CONNECTED && right.peer_ip != "0.0.0.0")
				return true;
		}

		if (left->established != right.established)
			return true;

		if (right.status == ZdcpScanner::Device::Status::CONNECTED && !left->is_our_device)
		{
			CString peerName(m_scanner.GetPeerName(right.peer_ip).c_str());
			if (right.peer_ip == "0.0.0.0")// скорее всего конектимся к себе
				return false;

			if (!peerName.IsEmpty())
			{
				if (left->Connceted != g_sConnectTo + L" " + peerName)
					return true;
			}
			else
			{
				if (left->Connceted != g_sConnectTo + L" " + CA2W(right.peer_ip.c_str()))
					return true;
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------
	void GenerateNameForList(_In_ const std::pair<ZdcpScanner::Device::Id, ZdcpScanner::Device>& device, ZdcpScanner::InterfaceList& m_interfaces, _Out_ strCommonDevice* _device)
	{
		if (_device == nullptr)
			return;

		std::wstring peer_name(L"");
		// Имя (серийник в hex если категория ZETSENSOR или если он 64-битный)
		if (device.second.id.type.GetCategory() == ZdcpScanner::Device::Type::Category::ZETSENSOR || (device.second.id.serial >> 32) != 0ull)
		{
			_device->name.Format(L"%hs %s%016llx",
					device.second.id.type.GetName(),
					g_sNo.GetString(),
					device.second.id.serial);
		}
		else
		{
			_device->name.Format(L"%hs %s%llu",
					device.second.id.type.GetName(),
					g_sNo.GetString(),
					device.second.id.serial);
		}
		// Описатель
		_device->label = device.second.label.c_str();
		// Статус
		switch (device.second.status)
		{
		case ZdcpScanner::Device::Status::AVAILABLE:
			_device->Connceted = g_sAvailable;
			break;
		case ZdcpScanner::Device::Status::CONNECTED:
			peer_name = m_scanner.GetPeerName(device.second.peer_ip).c_str();
			if (!peer_name.empty())
				_device->Connceted = g_sConnectTo + L" " + peer_name.c_str();
			else
				_device->Connceted = g_sConnectTo + L" " + CA2W(device.second.peer_ip.c_str());
			break;
		default:
			_device->Connceted = g_sUnknown;
			break;
		}

		if (device.second.id.type.GetZadcType() == -1)
			_device->Type = device.first.type.value;
		else 
			_device->Type = device.second.id.type.GetZadcType();

		_device->status = device.second.status;
		_device->IdDevice = std::make_pair(device.first.type.value, device.second.id.serial);


		_device->IpDevice = device.second.ip.c_str();
		_device->peer_ip = device.second.peer_ip.c_str();
		if (device.second.status == ZdcpScanner::Device::Status::CONNECTED && !device.second.peer_ip.empty())
		{
			auto match = [&device](ZdcpScanner::Interface &iface) -> bool {
				return iface.addr_str == device.second.peer_ip;
			};
			_device->is_our_device = (m_interfaces.end() != std::find_if(m_interfaces.begin(), m_interfaces.end(), match));
		}
		if (device.second.established)
		{
			_device->is_our_device = true;
			_device->established = true;
		}

		if (_device->is_our_device)
			_device->Connceted = g_sDeviceConnected;

	}

	ZdcpScanner& GetScanner()
	{
		return m_scanner;
	}

private:
	ZdcpScanner m_scanner;
};

//------------------------------------------------------------------------------
