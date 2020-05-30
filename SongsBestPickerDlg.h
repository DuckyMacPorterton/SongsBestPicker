
// DougHotkeysDlg.h : header file
//

#pragma once

#include "SystemTray.h"
#include "afxcmn.h"
#include <map>
#include "SongManager.h"
#include "MyListCtrl.h"

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
	CMyListCtrl				m_oSongList;
	CMyListCtrl				m_oStatsList;
	CMyListCtrl				m_oCurrentPodList;

	CSongManager			m_oSongManager;



	std::map<int, CString>	m_mapHotkeys;
	
	bool					m_bHotkeysApplied = false;

	CFont					m_oListCtrlFont;

	UINT					m_nHotkeyCurrentlyDown	= 0;
	UINT					m_nHotkeyDownTimerId	= 0;



protected:
	virtual BOOL	OnInitDialog();
	void			OnSysCommand(UINT nID, LPARAM lParam);
	void			OnPaint();
	HCURSOR			OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	void	ApplyHotkeys ();
	void	RemoveHotkeys ();
	void	OnShowMyWindow ();
	bool	AddHotkey (int nID, UINT nModifiers, UINT nVirtualKey, CString strNameForError);

	void	OnImportFromM3UFile		();
	void	OnDeleteSongList		();
	void	OnResetSongStatistics	();

	void	UpdateSongList ();



	void	MoveWindowForHotkey (UINT nHotkey, int nLargeMoveMultiplier = 1, int nSmallMoveMultiplier = 1);

	CString GetKeyName (unsigned int virtualKey);

	void	OnBnClickedOk();
	void	OnBnClickedCancel();
	void	OnBnClickedApplyHotkeys();
	void	OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);

	LRESULT	OnTrayNotification (WPARAM wParam, LPARAM lParam);
	void	OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	void	OnTimer (UINT_PTR nIDEvent);
	BOOL	PreTranslateMessage (MSG* pMsg);
};
