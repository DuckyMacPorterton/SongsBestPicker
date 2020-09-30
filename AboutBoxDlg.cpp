// AboutBoxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SongsBestPicker.h"
#include "AboutBoxDlg.h"
#include "afxdialogex.h"
#include <vector>
#include "Utils.h"


// CAboutBoxDlg dialog

IMPLEMENT_DYNAMIC(CAboutBoxDlg, CDialogEx)

CAboutBoxDlg::CAboutBoxDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ABOUTBOX, pParent)
{

}

CAboutBoxDlg::~CAboutBoxDlg()
{
}

void CAboutBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutBoxDlg, CDialogEx)
	ON_BN_CLICKED(ID_WEBSITE_BTN, &CAboutBoxDlg::OnBnClickedWebsiteBtn)
END_MESSAGE_MAP()



void CAboutBoxDlg::OnBnClickedWebsiteBtn()
{
	CString strURL = L"https://github.com/DuckyMacPorterton/SongsBestPicker";
	ShellExecute (0, _T("open"), strURL, 0, 0, SW_SHOWNORMAL);
	
}


BOOL CAboutBoxDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetDlgItem (IDC_STATIC_1)->SetWindowText (L"SongsBestPicker Woot! Version " + CUtils::GetProductVersion ());

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
