#pragma once
#include "..\..\resource.h"
#include "ZdcpScanner.h"
#include "Controls\AnimationGif\PictureEx.h"

class CChangeIp : public CDialogEx
{
	DECLARE_DYNAMIC(CChangeIp)
public:
	CChangeIp(const CString& CurrentIp, const CString& Ip, const CString& NetMask);
	virtual ~CChangeIp();
	enum { IDD = IDD_CHANGEIP };
	CString getIP();
	CString getMask();
	void SetInfo(CString type, ZdcpScanner* m_scanner) 
	{
		this->type = type; 
		this->m_scanner = m_scanner;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX); 
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnIpnFieldchangedIpaddress2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnIpnFieldchangedMask(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void ShowComponents(int param);
	CString currentIP;
	CString resultIP;
	CString resultMask;
	CString type;
	ZdcpScanner* m_scanner; // для провреки валидности ip адреса
	CPictureEx WaitGif;
	long checkIPisBusy;
};
