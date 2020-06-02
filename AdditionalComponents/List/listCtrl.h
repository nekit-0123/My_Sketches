#pragma once
#include "..\Menu\Cmenu.h"
#include <boost/signals2.hpp>
#include <thread>
#include "..\..\CommonDevices.h"
#include <condition_variable>
#include <mutex>

class CListCtrlDevice : public CListCtrl
{
public:
	CListCtrlDevice();
	virtual ~CListCtrlDevice();
	virtual void setDataToList(const std::vector<CString>& data = { L"" }) = 0;
	virtual void EndConnecting()										   = 0;

	void InsertDevice(const int nItem, const int Count, const CString name);
	void AddColumn();
	void ClearDevice();
	void pushDevice(const strCommonDevice& data);
	void SetNumTab(en_NumberTab::Number NumberTab);
	void onReactionChooseDevice(const boost::function< void(int) >& f);
	void onReactionChangeIPadress(const boost::function< void(CString, CString) >& f);
	void onReactionChangeOrderNumber(const boost::function< void(long) >& f);
	void getArrayDevices(device_vector& Device);

	void GetDeviceId(device_Id* _IdDevice)
	{
		if (_IdDevice == nullptr)
			return;
		*_IdDevice = IdDevice;
	}
	long GetDeviceSize() { return m_Device.size(); }

	CCommonDevices& getCommonClass()
	{
		return commonWork;
	}

	void SetBlockUpdate(bool val) { blockUpdate = val; }
protected:
	DECLARE_MESSAGE_MAP()	
	afx_msg void OnListCtrlCustomDraw(NMHDR* pNMHDR, LRESULT* pResult); 
	afx_msg BOOL OnNeedToolTip(UINT nID, NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	virtual afx_msg void OnRButtonDown(NMHDR *pNMHDR, LRESULT *pResult)			  = 0;
	virtual void Sort(const std::vector<CString>& data)							  = 0;
	virtual void StartConnecting(bool started, int item=-1)						  = 0;
	virtual void UpdateConnecting()												  = 0;

	void LoadItemImages();
	void OnMenuSelectDeviceUp();
	void OnMenuSelectDeviceDown();
	void OnMenuSelectOnOff();
	void OnMenuSelectChangeIp();
	void SetItemTextInList(const strCommonDevice& str);
	void InsertGroup(const int&  val, const CString& str);
	void SetItemImage(int iItem, int iImage);
	long GetState(const strCommonDevice & itt);
	long CheckShield();
	device_ptr GetItem(int Item);

	device_Id			 IdDevice;
	device_vector		 m_Device;
	CCommonDevices	     commonWork;
	MyMenu				 menu;
	en_NumberTab::Number NumberTab;
	std::thread updateIco;

	boost::signals2::signal< void(int) >			  ActivateDevice;
	boost::signals2::signal< void(CString, CString) > ChangeIPadress;
	boost::signals2::signal< void(long) >			  ChangeOrderNumber;
	CImageList			 m_imageList;
	CString				 cstrText;
	volatile bool		 blockUpdate;
}; 