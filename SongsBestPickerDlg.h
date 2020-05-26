
// DougHotkeysDlg.h : header file
//

#pragma once

#include "SystemTray.h"
#include "afxcmn.h"
#include <map>

// CDougHotkeysDlg dialog
class CSongsBestPickerDlg : public CDialogEx
{
// Construction
public:
	CSongsBestPickerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DOUGHOTKEYS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON			m_hIcon;
	
	CSystemTray				m_oTrayIcon; 
	CListCtrl				m_oHotkeyList;
	std::map<int, CString>	m_mapHotkeys;
	
	bool					m_bHotkeysApplied = false;

	CFont					m_oListCtrlFont;

	UINT					m_nHotkeyCurrentlyDown	= 0;
	UINT					m_nHotkeyDownTimerId	= 0;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	void	ApplyHotkeys ();
	void	RemoveHotkeys ();
	void	OnShowMyWindow ();
	bool	AddHotkey (int nID, UINT nModifiers, UINT nVirtualKey, CString strNameForError);

	void	MoveWindowForHotkey (UINT nHotkey, int nLargeMoveMultiplier = 1, int nSmallMoveMultiplier = 1);

	CString GetKeyName (unsigned int virtualKey);

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedApplyHotkeys();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);

	afx_msg LRESULT	OnTrayNotification (WPARAM wParam, LPARAM lParam);
	afx_msg void OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
			void OnTimer (UINT_PTR nIDEvent);
			BOOL PreTranslateMessage (MSG* pMsg);
};
