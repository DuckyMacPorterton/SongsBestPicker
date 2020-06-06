
// DougHotkeysDlg.h : header file
//

#pragma once

#include "SystemTray.h"
#include "afxcmn.h"
#include <map>
#include "SongManager.h"
#include "MyListCtrl.h"
#include "ThreadPool.h"

#include "fmod.hpp"
#include "fmod_errors.h"

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

	bool			m_bSongsSortAscending	= true;
	int				m_nSongsSortCol			= 0;

	//
	//  Handles info about what song is currently loaded into our player / editor

	int				m_nCurSongID			= -1;
	int				m_nCurPodID				= -1;

	CString			m_strCurSongTitle,		m_strLastLoadedSongTitle;
	CString			m_strCurSongPathToMp3,	m_strLastLoadedPathToMp3;

	//
	//  Playback info

	FMOD::System*	m_pPlaybackSystem	= NULL;
	FMOD::Sound*	m_pCurSong			= NULL;
	FMOD::Channel*	m_pPlaybackChannel	= NULL;



	CString			m_strSongPlaybackPos;
	CString			m_strSongPlaybackLen;
	CProgressCtrl	m_oSongPlayingProgress;

	ESongPlayStatus	m_eSongPlayingStatus			= ESongPlayStatus::eNotStarted;
	UINT_PTR		m_nSongPlayingStatusTimerID		= 0;

	std::map<int, CString>	m_mapHotkeys;
	bool					m_bHotkeysApplied = false;

protected:
	virtual BOOL	OnInitDialog();
	void			OnSysCommand(UINT nID, LPARAM lParam);
	void			OnPaint();
	HCURSOR			OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	static int CALLBACK SortCompareSongListCtrl (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK ComparePodSongRank	(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	int GetCurrentPodSongRank (int nSongID);

public:
	void	PlaySong (CString strFileToPlay = L"");
	void	PauseSong ();
	void	StopSong ();
	void	UnloadSong ();

	void	ApplyHotkeys ();
	void	RemoveHotkeys ();
	void	OnShowMyWindow ();
	bool	AddHotkey (int nID, UINT nModifiers, UINT nVirtualKey, CString strNameForError);

	void	OnImportFromM3UFile		();
	void	OnRecalcSongRatings		();
	void	OnDeleteSongList		();
	void	OnResetSongStatistics	();
	void	OnRemoveDuplicateSongs	();

	void	UpdateSongList				();
	void	UpdateCurrentPod			();
	void	UpdateStatsForCurrentSong	(int nSongID = -1);
	void	UpdatePlayerStatus			();

	void	LoadSongIntoPlayer	(int nSondID);
	void	SaveSongInfoFromPlayer ();

	CString	GetSongTitle		(int nSongID);
	CString	GetSongArtist		(int nSongID);
	CString	GetSongAlbum		(int nSongID);
	CString	GetSongPathToMp3	(int nSongID);
	bool	GetWonLossRecord	(int nSongID, int& rnWon, int& rnLost);
	bool	GetSongRating		(int nSongID, int& rnRating);

	void	OnDeleteSongFromList ();
	void	OnEditSongInfo ();
	void	OnPlaySongFromSongList ();


	bool	OnHandleHotkey (UINT nHotkey);

	bool	PlayOrPauseSong ();
	bool	SetNextSongActive ();
	bool	SetPrevSongActive ();
	bool	SetSongRank (int nRank);

	CString GetKeyName (unsigned int virtualKey);
	void	OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);

	LRESULT	OnTrayNotification (WPARAM wParam, LPARAM lParam);
	void	OnTimer (UINT_PTR nIDEvent);
	BOOL	PreTranslateMessage (MSG* pMsg);
	
	void	OnBnClickedOk();
	void	OnBnClickedCancel();
	void	OnBnClickedApplyHotkeys();

	void	OnItemChangedSongList(NMHDR* pNMHDR, LRESULT* pResult);
	void	OnBnClickedPlaySong();
	void	OnSongHeaderDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedCurrentPodList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSubmitPodRankings();
	afx_msg void OnBnClickedBrowseForSong();
	afx_msg void OnRClickSongList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSaveSongChanges();
};
