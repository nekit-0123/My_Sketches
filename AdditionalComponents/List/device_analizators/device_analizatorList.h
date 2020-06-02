#pragma once
#include "..\listCtrl.h"

// Информация о задействованном устройстве
typedef struct _connectingfInfo
{
	uint32_t Type;
	std::pair<uint32_t, uint64_t> IdDevice;
	CString name;
	CString label;
	CString ipDevice;

	_connectingfInfo():
		Type(0),
		name(L""),
		label(L""),
		ipDevice(L"")
	{
		IdDevice.first = 0;
		IdDevice.second = 0;
	}

	_connectingfInfo(const _connectingfInfo &data)
	{
		IdDevice.first = data.IdDevice.first;
		IdDevice.second = data.IdDevice.second;
		name = data.name;
		label = data.label;
		ipDevice = data.ipDevice;
		Type = data.Type;
	}

} connectingfInfo;

using vecConnectingfInfo = std::vector<connectingfInfo>;


class Cdevice_analizatorList : public CListCtrlDevice
{
public:
	Cdevice_analizatorList();
	virtual ~Cdevice_analizatorList();
	DECLARE_MESSAGE_MAP()
public:
	void setDataToList(const std::vector<CString>& data = { L"" })  override;
	void EndConnecting()										    override;

	void onInsertDevice(const boost::function< void(const connectingfInfo&) >& f);
	void SetConnectingDevice(const connectingfInfo& data);
	void ClearConnectingFromXml();
	void SetBlockCheckxml(const bool& param)
	{
		this->param = param;
	}
	long GetCountFiction() const { return countFictionDevice; }
	void SetCountFiction(const long&  count) { countFictionDevice = count; }
private:
	void Sort(const std::vector<CString>& data)						override;
	void StartConnecting(bool started, int item=-1)				    override;
	void UpdateConnecting()										    override;
	void OnRButtonDown(NMHDR *pNMHDR, LRESULT *pResult)				override;
	LRESULT OnMessageThread(WPARAM, LPARAM);

	long QuanOurDevice;
	CString cstrText;
	int					 _connectingImage;
	bool				 exitFromtread;
	bool				 treadIsRunning;

	std::condition_variable g_queuecheck;
	std::mutex              g_lockExit;
	boost::signals2::signal< void(const connectingfInfo&) > InsertTurnOffDevice;
	vecConnectingfInfo    ConnectingFromXml;
	bool param;
	long countFictionDevice;
};