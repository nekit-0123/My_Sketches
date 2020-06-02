#pragma once
#include <boost/thread.hpp>
#include <pugixml/src/pugixml.hpp>
#include "AdditionalComponents\List\device_analizators\device_analizatorList.h"
#include <Dialog_ZET\AutoScaleXY.h>
#include <memory>

typedef std::vector<CString> vecIpAdress;

class CNetWizardDlg : public CDialog
{
public:
	CNetWizardDlg(CWnd* pParent = NULL);
	~CNetWizardDlg();
	enum { IDD = IDD_NETWIZARD_DIALOG };

private:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	Auxiliary	helpClass;
	ZdcpScanner::InterfaceList m_interfaces;
	ZdcpScanner::DeviceList m_devices;
	std::unique_ptr<Cdevice_analizatorList> m_list;
		
	// Глобальные переменные установок сохраняемых в файл
	long typeDevice		[MAX_DEVICE];
	long numberDSP		[MAX_DEVICE];
	long open_timeout	[MAX_DEVICE];
	long command_timeout[MAX_DEVICE];
	long port_number	[MAX_DEVICE];
	uint64_t serial		[MAX_DEVICE];
	CString label		[MAX_DEVICE];
	uint32_t type		[MAX_DEVICE];

	//**

	void ReactionChoosingDevice(_In_ int our_device);
	void ReactionChangeIP(_In_ const CString& newIP, _In_ const CString& newMask);
	void ReactionChangeOrderNum(long position);
	void CheckDevices();
	void WriteAndCheckCfgNetServer();
	void WriteNetServCfgFile();
	void ReadNetServCfgFile(long& index);
	void ReadAndCheckCfg(const long& index);
	bool CheckCfg(_In_ const long i, _In_ const pugi::xml_node& parent);
	void FormingNewDevice(const connectingfInfo& itt);
	void ReadAllDeviceAtXml();
	vecIpAdress FormingForSort();
	device_ptr GetItem(const device_Id& _IdDevice);
	LRESULT OnMessageThread(WPARAM, LPARAM);

	CString					cfgpath;
	boost::thread			threadAnaliz;
	std::condition_variable m_queueExitAnaliz;
	std::mutex              m_lockExitAnaliz;
	long					m_lFindCountDevices;	
	device_vector			m_Device;
	bool				    m_bExitFromThread;
public:
	void ReadAndCheckCfgNetServer();
	void StopThread();
	Cdevice_analizatorList *GetList();
	CString getInterfaces();
	CString ipaddress[MAX_DEVICE];
};
