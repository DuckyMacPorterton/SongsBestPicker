
// DougHotkeysDlg.h : header file
//

#pragma once

#include "SystemTray.h"
#include "afxcmn.h"
#include <map>
#include "SongManager.h"
#include "MyListCtrl.h"
#include "ThreadPool.h"

//#define UseMci
#ifndef UseMci
#include "MP3\Mp3.h"
#endif

enum class ESongPlayStatus {
	eNotStarted,
	ePlaying,
	ePaused,
	eStopped
};



class CSongsBestPickerDlg : public CDialogEx
{
public:
	CSongsBestPickerDlg (CWnd* pParent = NULL);

	enum { IDD = IDD_MAIN_DIALOG};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);


protected:
	HICON			m_hIcon			= NULL;
	CFont			m_oListCtrlFont;
	HWND			m_hMainDlgWnd	= NULL;

	CSystemTray		m_oTrayIcon; 
	CMyListCtrl		m_oSongList;
	CMyListCtrl		m_oStatsList;
	CMyListCtrl		m_oCurrentPodList;

	CSongManager	m_oSongManager;
	CProgressCtrl	m_oSongPlayingProgress;

	int				m_nCurSongListCtrlIndex	= -1;
	CString			m_strCurSongName,		m_strLastLoadedSongName;
	CString			m_strCurSongPathToMp3,	m_strLastLoadedPathToMp3;

	CString			m_strSongPlaybackPos;
	CString			m_strSongPlaybackLen;

	ESongPlayStatus	m_eSongPlayingStatus			= ESongPlayStatus::eNotStarted;
	UINT_PTR		m_nSongPlayingStatusTimerID		= 0;

	Mp3				m_oCurrentSong;


	std::map<int, CString>	m_mapHotkeys;
	bool					m_bHotkeysApplied = false;

	UINT					m_nHotkeyCurrentlyDown	= 0;
	UINT					m_nHotkeyDownTimerId	= 0;


protected:
	virtual BOOL	OnInitDialog();
	void			OnSysCommand(UINT nID, LPARAM lParam);
	void			OnPaint();
	HCURSOR			OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	void	PlaySong (CString strFileToPlay);
	void	PauseSong ();
	void	StopSong ();
	LRESULT	OnMciNotify (WPARAM wParam, LPARAM lParam);

	int		GetSongLengthSecs ();
	int		GetSongPositionSecs ();

	void	ApplyHotkeys ();
	void	RemoveHotkeys ();
	void	OnShowMyWindow ();
	bool	AddHotkey (int nID, UINT nModifiers, UINT nVirtualKey, CString strNameForError);

	void	OnImportFromM3UFile		();
	void	OnDeleteSongList		();
	void	OnResetSongStatistics	();

	void	UpdateSongList				();
	void	UpdateCurrentPod			(int nSongID = -1);
	void	UpdateStatsForCurrentSong	(int nSongID = -1);
	void	UpdatePlayerStatus			();


	void	OnScheduleGames ();

	void	MoveWindowForHotkey (UINT nHotkey, int nLargeMoveMultiplier = 1, int nSmallMoveMultiplier = 1);
	CString GetKeyName (unsigned int virtualKey);
	void	OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);

	LRESULT	OnTrayNotification (WPARAM wParam, LPARAM lParam);
	void	OnKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );
	void	OnTimer (UINT_PTR nIDEvent);
	BOOL	PreTranslateMessage (MSG* pMsg);
	
	void	OnBnClickedOk();
	void	OnBnClickedCancel();
	void	OnBnClickedApplyHotkeys();

	void	OnItemChangedSongList(NMHDR* pNMHDR, LRESULT* pResult);
	void	OnBnClickedPlaySong();
};
