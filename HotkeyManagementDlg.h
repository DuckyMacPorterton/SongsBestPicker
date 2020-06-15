#pragma once


#define MOD_EXTENDED 0x0010		 // goes with the definitions in WINUSER.h
//#define HK_HOTKEY_UPDATED 0x0400 // message send to us when our hotkey control has been updated

#include "resource.h"
#include "HScrollListBox.h"
#include <map>
#include "HotkeyManager.h"
//#include "afxwin.h"

typedef std::multimap<UINT, UINT> CUIntToUIntMMap;

class CHotkeyCommand;
class CHotkeyManager;

/////////////////////////////////////////////////////////////////////////////
// CHotkeyManagementDlg dialog

class CHotkeyManagementDlg : public CDialog
{
// Construction
public:
	CHotkeyManagementDlg (CHotkeyManager *pManager, CWnd* pParent = NULL);   // standard constructor
	HACCEL m_hAccel;  // for accelerator

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	enum { IDD = IDD_HOTKEY_MANAGEMENT_DLG };
	CHotKeyCtrl		m_HotkeyCtrl;
	HScrollListBox  m_HotkeyList;
	HScrollListBox	m_CommandList;
	CString			m_strDescription;
	CString			m_strMatchingCommand;
	CEdit			m_oTypeToFilter;



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHotkeyManagementDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CHotkeyManager*		m_pHotkeyManager;
	CString				m_strTypeToFilter;
	CToolTipCtrl		m_oToolTip;

	CUIntToUIntMMap	 m_IllegalHotkeys;

	void  ActivateAcceleratorChanges ();

	void  LoadCommandsIntoList ();
	void  LoadHotkeysIntoList (CHotkeyCommand *pHKC);

	//
	//  These functions deal with making sure the users only use
	//  safe hotkeys

	void  AddIllegalHotkey (UINT nKey, UINT nModifiers);
	int   IsHotkeyAllowed (UINT nKey, UINT nModifiers);
	void  OnHotkeyUpdate ();
	void  SetupIllegalHotkeys ();

	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAssignHotkey();
	afx_msg void OnSelchangeListCommand();
	afx_msg void OnDeleteHotkey();
	afx_msg void OnDefaultHotkey();
	virtual BOOL OnInitDialog();
	afx_msg void OnResetAllDefaultsButton();

	void OnFind ();
	void OnEscape ();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditTypeToFilter();
	afx_msg void OnBnClickedHidden();
};

