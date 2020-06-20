
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
#include "HotkeyManager.h"
#include "EditWithIcon/IconEdit.h"

enum class ESongPlayStatus {
	eNotStarted,
	ePlaying,
	ePaused,
	eStopped
};

enum class EShowingInList {
	eGeneralStats,
	eErrorLog
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
	CMyListCtrl		m_oSongGameResultList;
	CMyListCtrl		m_oAccessoryList;

	CStringArray	m_arrErrors;

	CSongManager	m_oSongManager;

	bool			m_bSongsSortAscending	= true;
	int				m_nSongsSortCol			= 0;

	CIntArray		m_arrActiveColumns;
	EShowingInList	m_eWhatIsInAccessoryList		= EShowingInList::eGeneralStats;
	
	//
	//  Our current pod stuff

	CMyListCtrl		m_oCurrentPodList;
	CComboBox		m_oPodCombo;
	CIconEdit		m_oTypeToFilterPod;
	UINT_PTR		m_nTypeToFilterPodTimerId		= 0;
	CString			m_strTypeToFilterEmptyMsg;
	CString			m_strTypeToFilterLastText;
	bool			m_bTypeToFilterInEmptyMode		= true;
	bool			m_bTypeToFilterUseRegex			= true;
	CIntArray		m_arrPodComboIDs;

	//
	//  Handles info about what song is currently loaded into our player / editor

	int				m_nCurSongID			= -1;
	int				m_nCurPodID				= -1;

	bool			m_bCurPodFinished		= false;

	CString			m_strCurSongTitle;
	CString			m_strCurSongPathToMp3;
	CString			m_strCurSongArtist;
	CString			m_strCurSongAlbum;
	CString			m_strH2HCurrentCaption;

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
	bool					m_bHotkeysApplied		= false;
	CHotkeyManager			m_oHotkeyManager;

protected:
	DECLARE_MESSAGE_MAP()

	virtual BOOL	OnInitDialog();
	void			OnSysCommand(UINT nID, LPARAM lParam);
	void			OnPaint();
	HCURSOR			OnQueryDragIcon();

	static int CALLBACK SortCompareSongListCtrl (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	static int CALLBACK ComparePodSongRank	(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);


	bool	SetError (CString strError, bool bAlertUser = false, bool bRetVal = false);

public:
	void	PlaySong (CString strFileToPlay = L"");
	void	PauseSong ();
	void	StopSong ();
	void	UnloadSong ();

	void	ApplyHotkeys ();
	void	RemoveHotkeys ();
	void	OnShowMyWindow ();

	void	OnImportFromM3UFile		();
	void	OnAddSong				();
	void	OnRecalcSongRatings		();
	void	OnRecalcSongRatingsNoReset ();
	void	OnDeleteSongList		();
	void	OnResetSongStatistics	();

	void	UpdateSongList				(bool bInitCols = false);
	void	UpdateSongListSpecificSong	(int nSongID, CString strTitle = L"", CString strArtist = L"", CString strAlbum = L"", CString strPathToMp3 = L"");
	void	UpdateSongListWonLossSpecificSong (int nSongID);
	void	AddSongToSongListCtrl		(int nSongID);
	
	void	UpdateSongCount	();
	void	UpdateCurrentPod			(int nPodToLoad = -1);
	void	UpdateGameResultsForCurrentSong	(int nSongID);
	void	UpdatePlayerStatus			();
	void	UpdateAccessoryListCtrl		();
	void	UpdatePodCombo				();

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

	void	RestoreSongListColumns ();
	void	SaveSongListColumns ();

	bool	OnHandleHotkey (UINT nHotkey);

	bool	PlayOrPauseSong ();
	bool	SetNextSongActive ();
	bool	SetPrevSongActive ();
	bool	SetSongRank (int nRank);

	void	OnExportSongData ();
	void	OnEditHotkeys ();
	void	OnViewGeneralStats ();
	void	OnViewErrorLog ();
	void	OnClearErrorLog ();

	//
	//  These are for our user-configurable columns in the song list

	bool	GetDisplayStringForCol	(int nSongID, int nColIndex, CString& rstrDisplayString);
	bool	GetColumnSetupInfo		(int nColIndex, int& rnColType, CString& rstrColName, int& rnFormat, int& rnWidth);
	int		GetColumnType			(int nColIndex);
	bool	SwapColumns				(int nSwapFrom, int nSwapTo);

	void	ShowSongListColumnChooser ();

	//
	//  and back to your regularly scheduled program...



	void	OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);

	BOOL	OnCommand (WPARAM wParam, LPARAM lParam);


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
	void	OnDropFiles (HDROP hDropInfo);
	
	void	OnChangeTypeToFilterLeft ();
	void	OnKillFocusTypeToFilter	();
	void	OnSetFocusTypeToFilter	();



	LRESULT WindowProc (UINT message, WPARAM wParam, LPARAM lParam);

	LRESULT	OnClickedProgressCtrl	(WPARAM wParam, LPARAM lParam);
	LRESULT OnHeaderDragCol			(WPARAM wSource, LPARAM lDest);

	void	OnDblclkCurrentPodList	(NMHDR* pNMHDR, LRESULT* pResult);
	void	UpdateTypeToFilterDisplay (CString strFilter);
	void	SetTypeToFilterErrorMode (bool bError);
	void	SetTypeToFilterState (bool bIsTypeToFilterEmpty);
	void	OnSetFocusTypeToFilterHotkey ();
	afx_msg void OnSelChangeComboPod();
};
