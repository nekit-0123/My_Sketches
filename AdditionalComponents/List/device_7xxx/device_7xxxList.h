#pragma once
#include "..\listCtrl.h"
#include "zet76\zet76_config.h"

using Connecting7xxxptr = std::pair<zet76::device::id, zet76::config::mode_info>;
using vecConnecting7xxx = std::vector<Connecting7xxxptr>;

class Cdevice_7xxxist : public CListCtrlDevice
{
public:
	Cdevice_7xxxist();
	virtual ~Cdevice_7xxxist();
	DECLARE_MESSAGE_MAP()
public:
	void setDataToList(_In_ const std::vector<CString>& data = { L"" }) override;
	void EndConnecting()											    override;
	void SetConnectingDevice(const Connecting7xxxptr& data );
	void ClearConnectingFromXml();
	void onInsertDevice(const boost::function< void(const Connecting7xxxptr&) >& f);
	void SetBlockCheckxml(const bool& param)
	{
		this->param = param;
	}
	long GetCountFiction() const { return countFictionDevice; }
	void SetCountFiction(const long&  count) { countFictionDevice = count; }
private:
	void Sort(_In_ const std::vector<CString>& data)			   override;
	void StartConnecting(bool started, int item=-1)				   override;
	void UpdateConnecting()										   override;
	void OnRButtonDown(NMHDR *pNMHDR, LRESULT *pResult)		       override;
	LRESULT OnMessageThread(WPARAM, LPARAM);

	int					 _connectingImage;
	bool				 exitFromtread;
	bool				 treadIsRunning;
	vecConnecting7xxx    ConnectingFromXml;

	std::condition_variable g_queuecheck7xxx;
	std::mutex              g_lockExit7xxx;
	boost::signals2::signal< void(const Connecting7xxxptr&) > InsertTurnOffDevice;
	bool param;
	long countFictionDevice;
};
