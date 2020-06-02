#pragma once
#include "../../res/Definitions.h"

struct infoForMenu
{
	infoForMenu() :
		status(ZdcpScanner::Device::Status::AVAILABLE),
		iItem(-1),
		ip(L""),
		type(L""),
		is_our_device(false),
		enaleChangeIp(false),
		ArrowUp(false),
		ArrowDown(false),
		ArrowBoth(false),
		isConnecting(false)
	{}
	std::pair<uint32_t, uint64_t> IdDevice;	// Тип и серийник устройства
	CString ip;
	CString type;
	ZdcpScanner::Device::Status status;
	int  iItem;
	bool is_our_device;
	bool enaleChangeIp;
	bool ArrowUp;
	bool ArrowDown;
	bool ArrowBoth;
	bool isConnecting;
};

class MyMenu : public CMenu
{
public:
	MyMenu();
	virtual ~MyMenu();
	MyMenu& operator = (en_NumberTab::Number _num)
	{
		m_lNumTab = _num;
		return *this;
	}
	void Add(_In_ const infoForMenu& formenu_);
	void OnMenuSelect(_Out_ infoForMenu* infoMenuptr_);
	long GetNumTab() const;
private:
	long m_lNumTab;
	infoForMenu _infoMenu;
};