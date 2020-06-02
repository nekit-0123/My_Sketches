#pragma once
#include "AdditionalComponents\List\device_7xxx\device_7xxxList.h"
#include "resource.h"
#include <memory>
#include <7xxxXmlWorker\XMLWorker.h>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>

class device_7xxx : public CDialogEx
{
	DECLARE_DYNAMIC(device_7xxx)
public:
	device_7xxx(CWnd* pParent = nullptr);  
	virtual ~device_7xxx();
	enum { IDD = IDD_7XXX_TAB };
private:
	virtual void DoDataExchange(CDataExchange* pDX); 
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	ZdcpScanner::InterfaceList m_interfaces;
	ZdcpScanner::DeviceList m_devices;

	void followChangeConfig(void* arg);
	long Check7xxxDevice();
	void ReactionChoosingDevice(_In_ int enabled);
	void ReactionChangeIP(_In_ const CString& newIP, _In_ const CString& newMask);
	void FormingNewDevice(const Connecting7xxxptr& itt);
	device_ptr GetItem(const device_Id& _IdDevice);
	LRESULT OnMessageThread(WPARAM, LPARAM);

	Auxiliary									helpClass;
	boost::thread								thread;
	std::condition_variable						m_queueExit7xxx;
	std::mutex									m_lockExit7xxx;
	std::unique_ptr<zet76::config>				m_config7xxx;
	device_vector								vecDevice7xxx;
	std::unique_ptr <Cdevice_7xxxist>			m_list;
	long m_lFindCountDevices;
	CString cfgpath;
	bool m_bExitFromThread;
public:
	Cdevice_7xxxist *GetList();
	void StopThread();
};
