#pragma once


// CAboutBoxDlg dialog

class CAboutBoxDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CAboutBoxDlg)

public:
	CAboutBoxDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CAboutBoxDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedWebsiteBtn();
	virtual BOOL OnInitDialog();
	
};
