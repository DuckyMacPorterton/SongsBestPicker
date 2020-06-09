
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
#include "MyProgressCtrl.h"

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

	CIntArray		m_arrActiveColumns;

	//
	//  Handles info about what song is currently loaded into our player / editor

	int				m_nCurSongID			= -1;
	int				m_nCurPodID				= -1;

	CString			m_strCurSongTitle;
	CString			m_strCurSongPathToMp3;
	CString			m_strCurSongArtist;
	CString			m_strCurSongAlbum;

	//
	//  Playback info

	FMOD::System*	m_pPlaybackSystem	= NULL;
	FMOD::Sound*	m_pCurSong			= NULL;
	FMOD::Channel*	m_pPlaybackChannel	= NULL;

	CString			m_strSongPlaybackPos;
	CString			m_strSongPlaybackLen;
	CMyProgressCtrl	m_oSongPlayingProgress;

	ESongPlayStatus	m_eSongPlayingStatus			= ESongPlayStatus::eNotStarted;
	UINT_PTR		m_nSongPlayingStatusTimerID		= 0;

	std::map<int, CString>	m_mapHotkeys;
	bool					m_bHotkeysApplied = false;

protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL	OnInitDialog();
	void			OnSysCommand(UINT nID, LPARAM lParam);
	void			OnPaint();
	HCURSOR			OnQueryDragIcon();

	static int CALLBACK SortCompareSongListCtrl (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK ComparePodSongRank	(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);


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
	void	OnAddSong				();
	void	OnRecalcSongRatings		();
	void	OnDeleteSongList		();
	void	OnResetSongStatistics	();

	void	UpdateSongList				();
	void	UpdateSongListSpecificSong	(int nSongID, CString strTitle = L"", CString strArtist = L"", CString strAlbum = L"", CString strPathToMp3 = L"");
	void	UpdateSongListWonLossSpecificSong (int nSongID);
	void	UpdateSongCount	();

	void	UpdateCurrentPod			();
	void	UpdateStatsForCurrentSong	(int nSongID);
	void	UpdatePlayerStatus			();

	void	LoadSongIntoPlayer		(int nSondID);
	void	SaveSongInfoFromPlayer	();
	void	MakePlayerInfoEditable	(bool bEditable);

	CString	GetSongTitle			(int nSongID);
	CString	GetSongArtist			(int nSongID);
	CString	GetSongAlbum			(int nSongID);
	CString	GetSongPathToMp3		(int nSongID);
	int		GetCurrentPodSongRank	(int nSongID);

	bool	GetWonLossRecord	(int nSongID, int& rnWon, int& rnLost);
	bool	GetSongRating		(int nSongID, int& rnRating);
	bool	GetSongStrengthOfSchedule (int nSongID, int& rnStrengthOfSchedule);

	void	OnDeleteSongFromList ();
	void	OnEditSongInfo ();
	void	OnPlaySongFromSongList ();

	void	RestoreWindowPosition ();
	void	SaveWindowPosition ();

	bool	OnHandleHotkey (UINT nHotkey);

	bool	PlayOrPauseSong ();
	bool	SetNextSongActive ();
	bool	SetPrevSongActive ();
	bool	SetSongRank (int nRank);

	//
	//  These are for our user-configurable columns in the song list

	bool	GetDisplayStringForCol	(int nSongID, int nColIndex, CString& rstrDisplayString);
	bool	GetColumnSetupInfo		(int nColIndex, int& rnColType, CString& rstrColName, int& rnFormat, int& rnWidth);
	int		GetColumnType			(int nColIndex);
	bool	SwapColumns				(int nSwapFrom, int nSwapTo);

	//
	//  and back to your regularly scheduled program...


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
	void	OnItemChangedCurrentPodList(NMHDR* pNMHDR, LRESULT* pResult);
	void	OnBnClickedSubmitPodRankings();
	void	OnBnClickedBrowseForSong();
	void	OnRClickSongList(NMHDR* pNMHDR, LRESULT* pResult);

	LRESULT	OnClickedProgressCtrl	(WPARAM wParam, LPARAM lParam);
	LRESULT OnHeaderDragCol			(WPARAM wSource, LPARAM lDest);

	void	OnDblclkCurrentPodList	(NMHDR* pNMHDR, LRESULT* pResult);

};
