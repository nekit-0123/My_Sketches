#pragma once
#include "resource.h"
#include "..\..\ZdcpScanner.h"

struct DeviceInfo
{
	DeviceInfo()
		: type(0)
		, serial(0)
		, name(L"")
		, ip(L"")
		, label(L'')
		, isOurDevice(false)
		, is_fictional_device(false)

	{};
	~DeviceInfo() {};

	DeviceInfo(const DeviceInfo& data)
	{
		type = data.type;
		serial = data.serial;
		name = data.name;
		ip = data.ip;
		label = data.label;
		status = data.status;
		isOurDevice = data.isOurDevice;
		is_fictional_device = data.is_fictional_device;
	}

	void operator = (const DeviceInfo& data)
	{
		type = data.type;
		serial = data.serial;
		name = data.name;
		ip = data.ip;
		label = data.label;
		status = data.status;
		isOurDevice = data.isOurDevice;
		is_fictional_device = data.is_fictional_device;
	}

	uint64_t serial;
	uint32_t type;
	ZdcpScanner::Device::Status status;
	CString name;
	CString ip;
	CString label;
	bool isOurDevice;
	bool is_fictional_device;
};

class CDeviceInfo : public CDialogEx
{
public:
	DECLARE_DYNAMIC(CDeviceInfo)

	CDeviceInfo(CWnd* pParent = nullptr);
	virtual ~CDeviceInfo();
	void SetStrInfo(const DeviceInfo& info);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MINIINFO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	void NotIP();
	DeviceInfo info;
	CStatic m_icon;
};
