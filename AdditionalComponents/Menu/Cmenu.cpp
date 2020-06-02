#include "stdafx.h"
#include "..\..\res\Phrases.h"
#include "..\..\ZdcpScanner.h"
#include "Cmenu.h"

MyMenu::MyMenu()
{}

MyMenu::~MyMenu()
{}


void MyMenu::Add(_In_ const infoForMenu& formenu_)
{
	_infoMenu = formenu_;

	DestroyMenu();
	CreatePopupMenu();

	// Задейстовать/Отключить
	long EnableOrDisable(MF_DISABLED);
	if (!_infoMenu.is_our_device && !_infoMenu.isConnecting)
	{
		EnableOrDisable = _infoMenu.status == ZdcpScanner::Device::Status::AVAILABLE ? MF_ENABLED : MF_DISABLED;

		AppendMenu(EnableOrDisable | MF_STRING, en_NumberMenuSelect::NumOnOff, g_sEmploy);
	}
	else
		AppendMenu(MF_ENABLED | MF_STRING, en_NumberMenuSelect::NumOnOff, g_sUnplug);

	// Смена ip 
	EnableOrDisable = _infoMenu.enaleChangeIp ? MF_ENABLED : MF_DISABLED;
	AppendMenu(EnableOrDisable | MF_STRING, en_NumberMenuSelect::ChangeIp, g_sChangeIp);

	// Вверх / Вниз
	if (_infoMenu.ArrowBoth)
	{
		AppendMenu(MF_ENABLED | MF_BYPOSITION, en_NumberMenuSelect::Up, g_sUp);
		AppendMenu(MF_ENABLED | MF_BYPOSITION, en_NumberMenuSelect::Down, g_sDown);
	}
	else if (_infoMenu.ArrowUp)
		AppendMenu(MF_ENABLED | MF_BYPOSITION, en_NumberMenuSelect::Up, g_sUp);
	else if (_infoMenu.ArrowDown)
		AppendMenu(MF_ENABLED | MF_BYPOSITION, en_NumberMenuSelect::Down, g_sDown);
}

void MyMenu::OnMenuSelect(_Out_ infoForMenu* infoMenuptr_)
{
	if (infoMenuptr_ == nullptr)
		return;

	*infoMenuptr_ = _infoMenu;
}

long MyMenu::GetNumTab() const
{
	return m_lNumTab;
}