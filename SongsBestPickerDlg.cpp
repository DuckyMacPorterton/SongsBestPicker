
// DougHotkeysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SongsBestPicker.h"
#include "SongsBestPickerDlg.h"
#include "afxdialogex.h"

#include "Utils.h"
#include "HotkeyManagementDlg.h"
#include "HotkeyCommandDefs.h"
#include "HotkeyCommand.h"

#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define LIST_POD_COL_RANK		0
#define LIST_POD_COL_TITLE		1
#define LIST_POD_COL_ARTIST		2

#define LIST_GAMES_COL_OPPONENT	0
#define LIST_GAMES_COL_RATING	1
#define LIST_GAMES_COL_WONLOSS	2
#define LIST_GAMES_COL_MARGIN	3

#define LIST_STATS_WHAT			0
#define LIST_STATS_VALUE		1

#define SONG_STATUS_TIMER_ID	3
#define SONG_STATUS_TIMER_MS	1000

//
//  Resize amounts

#define MY_BIG_AMOUNT				100
#define MY_SMALL_AMOUNT				 1


#define VPCC_TYPE_TO_FILTER_TIMER_ID_TIMER_LEN_MS	500	
#define VPCC_TYPE_TO_FILTER_TIMER_ID				7

//
//  Weird slider crap

#define REAL_TO_SLIDER(x)	(100-x)
#define SLIDER_TO_REAL(x)	(100-x)


//
//  Command ids

#define ID_MY_NOTIFY					WM_APP + 0


//
//  These are the available columns

struct ColumnDefinitionStruct
{
	int				nType;
	const TCHAR*	strColumnName;
	int				nFormat;
	float			fWidthAsPercent;
	bool			bDefaultSortAscending;
};

//
//  Numbers do need to correspond to the index into garrSongList...  because I'm lazy.

#define LIST_SONG_COL_TITLE		0
#define LIST_SONG_COL_ARTIST	1
#define LIST_SONG_COL_ALBUM		2
#define LIST_SONG_COL_WONLOSS	3
#define LIST_SONG_COL_RATING	4
#define LIST_SONG_COL_MP3		5
#define LIST_SONG_COL_SOS		6
#define LIST_SONG_COL_ACTIVE	7
#define LIST_SONG_LIST_CTRL_INDEX	8

static ColumnDefinitionStruct garrSongColsAval[] = {
	{LIST_SONG_COL_TITLE,		L"Title",	LVCFMT_LEFT,	(float) 0.24,	true},
	{LIST_SONG_COL_ARTIST,		L"Artist",	LVCFMT_LEFT,	(float) 0.24,	true},
	{LIST_SONG_COL_ALBUM,		L"Album",	LVCFMT_LEFT,	(float) 0.24,	true},
	{LIST_SONG_COL_WONLOSS,		L"Record",	LVCFMT_CENTER,	(float) 0.09,	false},
	{LIST_SONG_COL_RATING,		L"Rating",	LVCFMT_CENTER,	(float) 0.08,	false},
	{LIST_SONG_COL_MP3,			L"MP3",		LVCFMT_LEFT,	(float) 0.10,	true},
	{LIST_SONG_COL_SOS,			L"SoS",		LVCFMT_CENTER,	(float) 0.10,	false},
	{LIST_SONG_COL_ACTIVE,		L"Active",	LVCFMT_CENTER,	(float) 0.05,	false},
	{LIST_SONG_LIST_CTRL_INDEX,	L"#",		LVCFMT_RIGHT,	(float) 0.04,	true},
};

#define SONG_LIST_AVAILABLE_COLUMN_COUNT (sizeof (garrSongColsAval) / sizeof (ColumnDefinitionStruct))


CString GetHotkeyCommandName (int nCmdID)
{
	for (int i = 0; i < HOTKEY_INIT_NUM_COMMANDS; i ++)
	{
		if (HKInit[i].nCommandID == nCmdID)
		{
			return HKInit[i].strCommandName;
		}
	}

	return _T("(Not found)");
}


//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
CString GetLastErrorAsString()
{
    //Get the error message, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return _T(""); //No error message has been recorded

    LPTSTR messageBuffer = nullptr;
    size_t size = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&messageBuffer, 0, NULL);

    CString message (messageBuffer, (int) size);

    //Free the buffer.
    LocalFree(messageBuffer);

    return message;
}




BOOL CSongsBestPickerDlg::PreTranslateMessage (MSG* pMsg)
{
	//
	//  Do this here because in a dialog, otherwise OnKeyDown doesn't work right 
	//  because the dialog itself never has the focus

	if (pMsg->message == WM_KEYDOWN)
	{
	}
	else if (pMsg->message == WM_KEYUP)
	{
		//
		//  We don't get this message...  that is, this 
		//  doesn't work

//		OnKeyUp (0, 0, 0);
	}


	return CDialog::PreTranslateMessage(pMsg);
}


CSongsBestPickerDlg::CSongsBestPickerDlg (CWnd* pParent /*=NULL*/)
	: CDialogEx(CSongsBestPickerDlg::IDD, pParent)
	, m_strCurSongTitle		(L"")
	, m_strCurSongPathToMp3	(L"")
	, m_strSongPlaybackPos	(_T(""))
	, m_strSongPlaybackLen	(_T(""))
	, m_strCurSongArtist	(_T(""))
	, m_strCurSongAlbum		(_T(""))
	, m_strH2HCurrentCaption(_T(""))
	, m_bUpdateResultsForPodSel(FALSE)
	, m_bSongIsStillInCompetition(FALSE)
{
	m_strTypeToFilterEmptyMsg = L"(type to filter ...)";

	m_hIcon = AfxGetApp()->LoadIcon(IDI_SONGS_BEST_PICKER);
}

void CSongsBestPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SONG_LIST, m_oSongList);
	DDX_Control(pDX, IDC_SONG_GAMES_LIST, m_oSongGameResultList);
	DDX_Control(pDX, IDC_CURRENT_POD_LIST, m_oCurrentPodList);
	DDX_Text(pDX, IDC_EDIT1, m_strCurSongTitle);
	DDX_Text(pDX, IDC_EDIT_PATH_TO_MP3, m_strCurSongPathToMp3);
	DDX_Text(pDX, IDC_SONG_POS, m_strSongPlaybackPos);
	DDX_Text(pDX, IDC_SONG_LENGTH, m_strSongPlaybackLen);
	DDX_Control(pDX, IDC_PROGRESS1, m_oSongPlayingProgress);
	DDX_Text(pDX, IDC_EDIT_ARTIST, m_strCurSongArtist);
	DDX_Text(pDX, IDC_EDIT_ALBUM, m_strCurSongAlbum);
	DDX_Control(pDX, IDC_STATS_LIST, m_oAccessoryList);
	DDX_Control(pDX, IDC_COMBO_POD_ID, m_oPodCombo);
	DDX_Text(pDX, IDC_STATIC_H2H_CURRENT, m_strH2HCurrentCaption);
	DDX_Control(pDX, IDC_EDIT_TYPE_TO_FILTER, m_oTypeToFilterPod);
	DDX_Control(pDX, IDC_SLIDER_VOLUME, m_oVolumeSlider);
	DDX_Check(pDX, IDC_CHECK_UPDATE_RESULTS_FOR_POD, m_bUpdateResultsForPodSel);
	DDX_Check(pDX, IDC_CHECK_SONG_ACTIVE, m_bSongIsStillInCompetition);
}

BEGIN_MESSAGE_MAP(CSongsBestPickerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_HOTKEY()
	ON_WM_KEYUP()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()

	ON_BN_CLICKED(IDOK,				&CSongsBestPickerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL,			&CSongsBestPickerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(ID_APPLY_HOTKEYS, &CSongsBestPickerDlg::OnBnClickedApplyHotkeys)
	ON_COMMAND (ID_FILE_EXIT,		&CSongsBestPickerDlg::OnBnClickedOk)

	ON_MESSAGE (ID_MY_NOTIFY,			OnTrayNotification)

	ON_REGISTERED_MESSAGE (UWM_CLICKED_PROGRESS_CTRL,	OnClickedProgressCtrl)
	ON_REGISTERED_MESSAGE (UWM_SONG_HEADER_DRAG_COL,	OnHeaderDragCol)


	ON_COMMAND(ID_DOUG_SHOWWINDOW,		OnShowMyWindow)
	ON_COMMAND(ID_IMPORTFROMM3UFILE,	OnImportFromM3UFile)
	ON_COMMAND(ID_RECALC_SONG_RATINGS,	OnRecalcSongRatings)
	ON_COMMAND(ID_RECALC_SONG_RATINGS_DONT_RESET,	OnRecalcSongRatingsNoReset)
	ON_COMMAND(ID_RESETSONGSTATISTICS,	OnResetSongStatistics)
	ON_COMMAND(ID_DELETESONGLIST,		OnDeleteSongList)
	ON_COMMAND(ID_FILE_ADDSONG,			OnAddSong)

	ON_COMMAND(ID_POPUP_DELETESONG,		OnDeleteSongFromList)
	ON_COMMAND(ID_POPUP_EDITSONGINFO,	OnEditSongInfo)
	ON_COMMAND(ID_PLAY_SONG,			OnPlaySongFromSongList)
	ON_COMMAND(ID_EXPORT_SONG_DATA,		OnExportSongData)
	ON_COMMAND(ID_EDITHOTKEYS,			OnEditHotkeys)
	ON_COMMAND(ID_VIEW_GENERALSTATS,	OnViewGeneralStats)
	ON_COMMAND(ID_VIEW_ERRORLOGS,		OnViewErrorLog)
	ON_COMMAND(ID_VIEW_CLEARERRORLOG,	OnClearErrorLog)
	ON_COMMAND(ID_COPY_SONG_MP3,		OnCopySongMp3)

	ON_WM_TIMER()

	ON_NOTIFY(LVN_GETDISPINFO,	IDC_SONG_LIST,			&CSongsBestPickerDlg::OnGetDispInfo)
	ON_NOTIFY(LVN_ITEMCHANGED,	IDC_SONG_LIST,			&CSongsBestPickerDlg::OnItemChangedSongList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CURRENT_POD_LIST,	&CSongsBestPickerDlg::OnItemChangedCurrentPodList)

	ON_EN_CHANGE(IDC_EDIT_TYPE_TO_FILTER,				&CSongsBestPickerDlg::OnChangeTypeToFilterLeft)
	ON_EN_SETFOCUS(IDC_EDIT_TYPE_TO_FILTER,				&CSongsBestPickerDlg::OnSetFocusTypeToFilter)
	ON_EN_KILLFOCUS(IDC_EDIT_TYPE_TO_FILTER,			&CSongsBestPickerDlg::OnKillFocusTypeToFilter)
	ON_COMMAND(ID_SET_FOCUS_TYPE_TO_FILTER,				OnSetFocusTypeToFilterHotkey)


	ON_BN_CLICKED(IDC_PLAY_SONG,	&CSongsBestPickerDlg::OnBnClickedPlaySong)
//	ON_BN_CLICKED(IDC_PAUSE_SONG,	&CSongsBestPickerDlg::PauseSong)
//	ON_BN_CLICKED(IDC_STOP_SONG,	&CSongsBestPickerDlg::StopSong)

	ON_NOTIFY(HDN_ITEMCLICK, 0,					&CSongsBestPickerDlg::OnHeaderClick)

	ON_BN_CLICKED(IDC_SUBMIT_POD_RANKINGS,		&CSongsBestPickerDlg::OnBnClickedSubmitPodRankings)
	ON_BN_CLICKED(IDC_BROWSE_FOR_SONG,			&CSongsBestPickerDlg::OnBnClickedBrowseForSong)
	ON_NOTIFY(NM_RCLICK, IDC_SONG_LIST,			&CSongsBestPickerDlg::OnRClickSongList)
	ON_BN_CLICKED(IDC_SAVE_SONG_CHANGES,		&CSongsBestPickerDlg::SaveSongInfoFromPlayer)
	ON_NOTIFY(NM_DBLCLK, IDC_CURRENT_POD_LIST,	&CSongsBestPickerDlg::OnDblclkCurrentPodList)

	ON_CBN_SELCHANGE(IDC_COMBO_POD_ID, &CSongsBestPickerDlg::OnSelChangeComboPod)
	ON_BN_CLICKED(IDC_BUTTON_PREV_POD, &CSongsBestPickerDlg::OnBnClickedButtonPrevPod)
	ON_BN_CLICKED(IDC_BUTTON_NEXT_POD, &CSongsBestPickerDlg::OnBnClickedButtonNextPod)
	
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnBnClickedDefault)

	ON_WM_VSCROLL()

END_MESSAGE_MAP()


// CDougHotkeysDlg message handlers

BOOL CSongsBestPickerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_hMainDlgWnd = GetSafeHwnd ();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//
	//  We want user to be able to drag / drop files into us

	DragAcceptFiles (true);

	ChangeWindowMessageFilter (WM_DROPFILES,	MSGFLT_ADD);
	ChangeWindowMessageFilter (WM_COPYDATA,		MSGFLT_ADD);
	ChangeWindowMessageFilter (0x0049,			MSGFLT_ADD);

	//
	//  Set the default columns, if we haven't already

	int nActiveColCount = 0;
	m_oSongManager.GetColumnCount (nActiveColCount);

	CRect rcList;
	m_oSongList.GetClientRect (rcList);

	if (0 == nActiveColCount)
	{
		for (int i = 0; i < SONG_LIST_AVAILABLE_COLUMN_COUNT; i ++)
		{
			m_oSongManager.SetColumnSetupInfo (i, garrSongColsAval[i].nType, garrSongColsAval[i].strColumnName, garrSongColsAval[i].nFormat, (int) (garrSongColsAval[i].fWidthAsPercent * rcList.Width ()));
		}
	}

	//
	//  Setup our list control

	m_oListCtrlFont.CreatePointFont (90, L"Courier New");
	m_oSongList.SetFont (&m_oListCtrlFont);
	m_oSongList.SetExtendedStyle (m_oSongList.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//  And our current pool / pod / competition / whatever we call it

	m_oCurrentPodList.GetClientRect (rcList);
	m_oCurrentPodList.InsertColumn (LIST_POD_COL_RANK,		L"Rank",	LVCFMT_CENTER,	(int) (rcList.Width () * 0.12));
	m_oCurrentPodList.InsertColumn (LIST_POD_COL_TITLE,		L"Song",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.44));
	m_oCurrentPodList.InsertColumn (LIST_POD_COL_ARTIST,	L"Artist",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.43));
	m_oCurrentPodList.SetExtendedStyle (m_oSongList.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//  Head to head results for a particular song

	m_oSongGameResultList.GetClientRect (rcList);
	m_oSongGameResultList.InsertColumn (LIST_GAMES_COL_OPPONENT,	L"Opponent",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.53));
	m_oSongGameResultList.InsertColumn (LIST_GAMES_COL_RATING,		L"Rat",			LVCFMT_CENTER,	(int) (rcList.Width () * 0.15));
	m_oSongGameResultList.InsertColumn (LIST_GAMES_COL_WONLOSS,		L"Win?",		LVCFMT_CENTER,	(int) (rcList.Width () * 0.15));
	m_oSongGameResultList.InsertColumn (LIST_GAMES_COL_MARGIN,		L"+/-",			LVCFMT_CENTER,	(int) (rcList.Width () * 0.15));

	//
	//  Some various stats

	m_oAccessoryList.GetClientRect (rcList);
	m_oAccessoryList.InsertColumn (LIST_STATS_WHAT,		L"What?",	LVCFMT_LEFT, (int)(rcList.Width () * 0.49));
	m_oAccessoryList.InsertColumn (LIST_STATS_VALUE,	L"Value",	LVCFMT_LEFT, (int)(rcList.Width () * 0.49));
	m_oAccessoryList.SetExtendedStyle (m_oAccessoryList.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//  Type to filter pods!

	m_oTypeToFilterPod.SetIcon  (IDI_ICON_X, true);

	//
	//  Our playback system
	
	void*		pExtraDriverData = NULL;
	FMOD_RESULT result = FMOD::System_Create (&m_pPlaybackSystem);
	if (result == FMOD_OK)
		result = m_pPlaybackSystem->init (32, FMOD_INIT_NORMAL, pExtraDriverData);

	if (result != FMOD_OK)
		AfxMessageBox (L"Error initializing playback system: " + CUtils::UTF8toUTF16 (FMOD_ErrorString (result)));

	//
	//  For anything to work, we need a valid database.  Confirm that we have one.

	m_oSongManager.SetFmodSystem (m_pPlaybackSystem);
	if (! m_oSongManager.GetError ().IsEmpty ())
		SetError (m_oSongManager.GetError (), true);
	else
		UpdateSongList (true);	// true to init columns


	m_oVolumeSlider.SetRange (0, 100, true);
	m_oVolumeSlider.SetPos (REAL_TO_SLIDER (100));

	//
	//  Let's allow ourselves to minimize to system tray

	m_oTrayIcon.Create (this, ID_MY_NOTIFY, L"Songs Best Picker!", m_hIcon, IDR_MENU_DOUGS_HOTKEYS);

	//
	//  And apply the hotkeys

	m_oHotkeyManager.LoadHotkeys (m_oSongManager.GetDB ());
	if (m_oHotkeyManager.GetCommandCount () == 0)
		m_oHotkeyManager.ResetToSystemDefaults ();

	ApplyHotkeys ();

	//
	//  Put the window where they had it before

	RestoreWindowPosition ();

	//
	//  Start a timer to keep tabs on things

	m_nSongPlayingStatusTimerID = SetTimer (SONG_STATUS_TIMER_ID, SONG_STATUS_TIMER_MS, NULL);

	//
	//  Make sure we have songs scheduled into pools for competition

	m_oSongManager.ScheduleMorePods ();
//	UpdatePodCombo ();
	UpdateTypeToFilterDisplay (L"");
	UpdateCurrentPod ();

	if (m_eSongPlayingStatus == ESongPlayStatus::ePlaying)
		PlaySong ();

	UpdateAccessoryListCtrl ();

	return TRUE;  // return TRUE  unless you set the focus to a control
} // end on init dialog




LRESULT	CSongsBestPickerDlg::OnTrayNotification (WPARAM wParam, LPARAM lParam)
{
	return m_oTrayIcon.OnTrayNotification (wParam, lParam);
}


void CSongsBestPickerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == SC_MINIMIZE)
	{
		//
		//  Minimize to system tray

		m_oTrayIcon.MinimiseToTray (this);
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSongsBestPickerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSongsBestPickerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}





void CSongsBestPickerDlg::OnBnClickedOk()
{
	SaveWindowPosition ();

	RemoveHotkeys ();
	CDialogEx::OnOK();
}


void CSongsBestPickerDlg::OnBnClickedCancel()
{
	m_oTrayIcon.MinimiseToTray (this);	
//	OnBnClickedOk ();
}



//////////////////////////////////////////////////////////////////////
//
//    O N   G E T   D I S P   I N F O
//
//  This is where the list control asks us what's actually at a 
//  given index.  Lucky for us we've kept track.
//
void CSongsBestPickerDlg::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO*	pDispInfo	= reinterpret_cast<LV_DISPINFO*> (pNMHDR);
	LV_ITEM*		pItem		= &(pDispInfo->item);

	int nListCtrlIndex	= pItem->iItem;
	int nColumn			= pItem->iSubItem;

	//
	//  Now give it whatever info it's looking for

	if (pItem->mask & LVIF_TEXT)
	{
		//
		//  Now find the string for this item and display it

		bool	bCache = true;
		CString strToDisplay;
		GetDisplayStringForCol ((int) pDispInfo->item.lParam, nColumn, strToDisplay, nListCtrlIndex, &bCache);

		//
		//  So now we have out string to display, display it...  but only out
		//  to the length the control is ready to take

		_tcsncpy(pItem->pszText, strToDisplay, (size_t)pItem->cchTextMax - 1);
		pItem->pszText[pItem->cchTextMax - 1] = TCHAR (NULL);

		if (bCache)
			pItem->mask |= LVIF_DI_SETITEM;

	}

	*pResult = 0;

} // end OnGetDispInfo














//************************************
// Method:    PlaySong
// FullName:  CSongsBestPickerDlg::PlaySong
// Access:    public 
// Returns:   void
// Qualifier:
//
//************************************
void CSongsBestPickerDlg::PlaySong (CString strFileToPlay /* = L"" */)
{
	UpdateData ();

	if (strFileToPlay.IsEmpty () && NULL != m_pPlaybackChannel)
	{
		//
		//  If we're paused, just try to resume

		if (m_eSongPlayingStatus == ESongPlayStatus::ePaused)
		{
			m_pPlaybackChannel->setPaused (false);
			m_eSongPlayingStatus = ESongPlayStatus::ePlaying;
			return;
		}	
	}

	strFileToPlay = m_strCurSongPathToMp3;
	if (strFileToPlay.IsEmpty ())
		return;

	//
	//  Play it!
	
	FMOD_RESULT result = m_pPlaybackSystem->createSound (CUtils::UTF16toUTF8 (m_strCurSongPathToMp3), FMOD_DEFAULT, 0, &m_pCurSong);
	if (result != FMOD_OK) {
		m_eSongPlayingStatus = ESongPlayStatus::eStopped;
		AfxMessageBox (L"Error loading song: " + CUtils::UTF8toUTF16 (FMOD_ErrorString (result)) + L"\r\n\r\n" + m_strCurSongPathToMp3);
		return;
	}

	result = m_pPlaybackSystem->playSound (m_pCurSong, 0, false, &m_pPlaybackChannel);
	m_pPlaybackChannel->setVolume ((float) SLIDER_TO_REAL (m_oVolumeSlider.GetPos ()) / 100);
	m_eSongPlayingStatus = ESongPlayStatus::ePlaying;

	UINT nSongLenMS = 0;
	result = m_pCurSong->getLength (&nSongLenMS, FMOD_TIMEUNIT_MS);
	if (result != FMOD_OK) {
//		AfxMessageBox (L"Error loading song: " + CUtils::UTF8toUTF16 (FMOD_ErrorString (result)) + L"\r\n\r\n" + m_strCurSongPathToMp3);
		return;
	}

	int nSongLenSec = nSongLenMS / 1000;

	m_strSongPlaybackLen.Format (L"%d:%02d", nSongLenSec / 60, nSongLenSec % 60);
	m_oSongPlayingProgress.SetRange32 (0, nSongLenSec);
	m_oSongPlayingProgress.SetPos (0);

	//
	//  Tell the world what we're playing

	CString strTooltip;  strTooltip.Format (L"Now Playing:\n%s\n%s", m_strCurSongTitle, m_strCurSongArtist);
	m_oTrayIcon.SetTooltipText (strTooltip);

	UpdateData (false);

} // end play song



//************************************
// Method:    PauseSong
// FullName:  CSongsBestPickerDlg::PauseSong
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::PauseSong ()
{
	if (NULL == m_pPlaybackChannel)
		return;

	m_pPlaybackChannel->setPaused (true);
	m_eSongPlayingStatus = ESongPlayStatus::ePaused;

} // end CSongsBestPickerDlg::PauseSong



//************************************
// Method:    StopSong
// FullName:  CSongsBestPickerDlg::StopSong
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::StopSong ()
{
	if (NULL != m_pPlaybackChannel)
		m_pPlaybackChannel->stop ();

} // end CSongsBestPickerDlg::StopSong



//************************************
// Method:    UnloadSong
// FullName:  CSongsBestPickerDlg::UnloadSong
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::UnloadSong ()
{
	if (NULL != m_pPlaybackChannel)
		m_pPlaybackChannel->stop ();
	if (NULL != m_pCurSong)
		m_pCurSong->release ();

	m_pCurSong		= NULL;
	m_pPlaybackChannel	= NULL;

	m_eSongPlayingStatus = ESongPlayStatus::eStopped;

} // end CSongsBestPickerDlg::UnloadSong



//************************************
// Method:    OnImportFromM3UFile
// FullName:  CSongsBestPickerDlg::OnImportFromM3UFile
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnImportFromM3UFile()
{
	if (m_oSongList.GetItemCount () > 0)
	{
		if (IDYES != AfxMessageBox (L"Importing will add songs to your existing list.  If you want to start over, first Delete your song list.\r\n\r\nDo you still want to import songs?", MB_YESNO | MB_DEFBUTTON2))
			return;
	}

	CFileDialog oFD (true, L"*.*");
	if (IDOK != oFD.DoModal ())
		return;

	if (! m_oSongManager.InitSongsFromTextFile (oFD.GetPathName (), EFileFormat::eM3U))
		SetError (m_oSongManager.GetError (true), true);
	else
		UpdateSongList ();

} // end on import from m3u file



//************************************
// Method:    OnAddSong
// FullName:  CSongsBestPickerDlg::OnAddSong
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnAddSong ()
{
	//
	//  An easy way to get a unique name for the us r to edit
	CTime	tNow		= CTime::GetCurrentTime ();
	CString strDateTime = tNow.Format (L"%b %d, %Y %H:%M:%S");

	int nNewSongID = -1;
	if (! m_oSongManager.AddSong (nNewSongID, strDateTime, strDateTime)) {
		SetError (L"Failed to add song: " + m_oSongManager.GetError (true), true);
		return;
	}

	UpdateSongList ();

	LoadSongIntoPlayer (nNewSongID);
	MakePlayerInfoEditable (true);	// Have to call this after load song into player

} // end CSongsBestPickerDlg::OnAddSong



//************************************
// Method:    OnRecalcSongRatings
// FullName:  CSongsBestPickerDlg::OnRecalcSongRatings
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::OnRecalcSongRatings ()
{
	CWaitCursor wc;
	m_oSongManager.RecalcAllSongRatings ();
	UpdateSongList ();

} // end CSongsBestPickerDlg::OnRecalcSongRatings



//************************************
// Method:    OnRecalcSongRatingsNoReset
// FullName:  CSongsBestPickerDlg::OnRecalcSongRatingsNoReset
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::OnRecalcSongRatingsNoReset ()
{
	m_oSongManager.RecalcAllSongRatings (false);	//  False not to clear existing ratings first
	UpdateSongList ();

} // end CSongsBestPickerDlg::OnRecalcSongRatingsNoReset



//************************************
// Method:    OnDeleteSongList
// FullName:  CSongsBestPickerDlg::OnDeleteSongList
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnDeleteSongList()
{
	if (IDYES != AfxMessageBox (L"This will delete all songs and all song statistics.  That's deleting EVERYTHING.\r\n\r\nDo you want to delete everything?", MB_YESNO | MB_DEFBUTTON2))
		return;

	if (m_oSongManager.DeleteAllSongs ())
		UpdateSongList ();

} // end on delete song list



//************************************
// Method:    OnResetSongStatistics
// FullName:  CSongsBestPickerDlg::OnResetSongStatistics
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnResetSongStatistics()
{
	if (IDYES != AfxMessageBox (L"This will delete all song statistics.  You cannot get that back.\r\n\r\nDo you really want to do this?", MB_YESNO | MB_DEFBUTTON2))
		return;

	m_oSongManager.DeleteAllSongStats ();

} // end on reset song statistics



//************************************
// Method:    UpdateSongList
// FullName:  CSongsBestPickerDlg::UpdateSongList
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::UpdateSongList (bool bInitCols /* = false */)
{
	if (bInitCols)
	{
		RestoreSongListColumns ();
	}

	//
	//  For now, assume we'll have few enough songs that we don't need
	//  to do a virtual list control
	//
	//  Though to make it look good when we update our stats, we might want to.
	//  That or make our update more targeted.

	int		nLastID		= -1;
	CString strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, strWonLoss;
	int		nSongCount	= 0, nWins = 0, nLosses = 0;

	if (! m_oSongManager.GetSongCount (nSongCount)) {
		SetError (m_oSongManager.GetError (true));
		return;
	}

	//
	//  Clear it out

	m_oSongList.SetRedraw (false);
	m_oSongList.DeleteAllItems ();

	//
	// Load 'em up!

	for (int i = 0; i < nSongCount; i ++)
	{
		if (! m_oSongManager.GetNextSong (strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, nLastID, nLastID))
		{
			SetError (m_oSongManager.GetError (true));
			m_oSongList.DeleteAllItems ();
			m_oSongList.SetRedraw (true);
			m_oSongList.UpdateWindow ();
			return;
		}

		AddSongToSongListCtrl (nLastID);

	} // end loop through songs

	m_oSongList.SetRedraw (true);
	m_oSongList.UpdateWindow ();

	UpdateSongCount ();

} // end CSongsBestPickerDlg::UpdateSongList



//************************************
// Method:    AddSongToSongListCtrl
// FullName:  CSongsBestPickerDlg::AddSongToSongListCtrl
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
//************************************
void CSongsBestPickerDlg::AddSongToSongListCtrl (int nSongID)
{
	CString strToDisplay;
	GetDisplayStringForCol (nSongID, 0, strToDisplay);

//#define OldWays
#ifndef OldWays
	LV_ITEM Item;
	Item.lParam	  = (LPARAM) nSongID;		
	Item.pszText  = LPSTR_TEXTCALLBACK;		// using callbacks to get the text
	Item.mask	  = LVIF_TEXT | LVIF_PARAM;	// lParam and pszText fields active
	Item.iItem	  = m_oSongList.GetItemCount ();			// position to insert new item
	Item.iSubItem = 0;

	m_oSongList.InsertItem (&Item);
#else

	int nIndex = m_oSongList.InsertItem (m_oSongList.GetItemCount (), strToDisplay);
	m_oSongList.SetItemData (nIndex, nSongID);

	for (int nCol = 1; nCol < m_arrActiveColumns.GetSize (); nCol++)
	{
		if (GetDisplayStringForCol (nSongID, nCol, strToDisplay, nIndex))
			m_oSongList.SetItemText (nIndex, nCol, strToDisplay);
	}
#endif
} // end CSongsBestPickerDlg::AddSongToSongListCtrl




//************************************
// Method:    UpdateSongListSpecificSong
// FullName:  CSongsBestPickerDlg::UpdateSongListSpecificSong
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
//************************************
void CSongsBestPickerDlg::UpdateSongListSpecificSong (int nSongID, CString strTitle /* = L"" */, CString strArtist /* = L"" */, CString strAlbum /* = L"" */, CString strPathToMp3 /* = L"" */)
{
	int nListCtrlIndex = m_oSongList.FindByItemData (nSongID);
	if (-1 == nListCtrlIndex)
	{
		//
		//  It's not here yet - add it

		AddSongToSongListCtrl (nSongID);
		return;
	}

#ifdef needThis
	if (strTitle.IsEmpty ())
	{
		m_oSongManager.GetSongDetails (nSongID, strTitle, strArtist, strAlbum, strPathToMp3);
	}
#endif

	CString strToDisplay;
	for (int nCol = 0; nCol < m_arrActiveColumns.GetSize (); nCol++)
	{
		if (GetDisplayStringForCol (nSongID,			nCol, strToDisplay, nListCtrlIndex))
			m_oSongList.SetItemText (nListCtrlIndex,	nCol, strToDisplay);
	}

} // end CSongsBestPickerDlg::UpdateSongListSpecificSong



//************************************
// Method:    UpdateSongListWonLossSpecificSong
// FullName:  CSongsBestPickerDlg::UpdateSongListWonLossSpecificSong
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
//************************************
void CSongsBestPickerDlg::UpdateSongListWonLossSpecificSong (int nSongID)
{
	int nListCtrlIndex = m_oSongList.FindByItemData (nSongID);
	if (-1 == nListCtrlIndex)
		return;

	CString strToDisplay;
	for (int nCol = 0; nCol < m_arrActiveColumns.GetSize (); nCol++)
	{
		if (GetDisplayStringForCol (nSongID,			nCol, strToDisplay, nListCtrlIndex))
			m_oSongList.SetItemText (nListCtrlIndex,	nCol, strToDisplay);
	}

} // end CSongsBestPickerDlg::UpdateSongListWonLossSpecificSong




//************************************
// Method:    UpdateSongCountInSongList
// FullName:  CSongsBestPickerDlg::UpdateSongCountInSongList
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::UpdateSongCount ()
{
	if (m_arrActiveColumns.GetSize () == 0)
		return;

	int nSongCount = 0;
	m_oSongManager.GetSongCount (nSongCount);

	CString strRoot		= L"Title";
	int		nTitleCol	= CUtils::FindNumberInArray (m_arrActiveColumns, LIST_SONG_COL_TITLE);
	if (-1 == nTitleCol)
	{
		//
		//  We don't have a title showing, just choose the first col - but get the text to show

		int nWidth, nFormat, nType;
		m_oSongManager.GetColumnSetupInfo (0, nType, strRoot, nFormat, nWidth);
		nTitleCol = 0;
	}

	CString strTitle;
	strTitle.Format (L"%s (%d songs)", strRoot, nSongCount);


	LV_COLUMN col;
	col.mask = LVCF_TEXT;
	col.pszText = strTitle.GetBuffer ();
	m_oSongList.SetColumn (nTitleCol, &col);

} // end CSongsBestPickerDlg::UpdateSongCountInSongList



//************************************
// Method:    UpdateCurrentPod
// FullName:  CSongsBestPickerDlg::UpdateCurrentPod
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
//
//
//
//************************************
void CSongsBestPickerDlg::UpdateCurrentPod (int nPodToLoad /* = -1 */)
{
	int nPrevPodID = -1;

	CIntArray arrSongIDs;

	if (-1 == nPodToLoad)
	{
		if (! m_oSongManager.GetCurrentPod (m_nCurPodID, arrSongIDs)) {
			SetError (m_oSongManager.GetError (true));
			return;
		}

		m_bCurPodFinished = false;
	}
	else 
	{
		if (! m_oSongManager.GetPod (nPodToLoad, arrSongIDs, m_bCurPodFinished)) {
			SetError (m_oSongManager.GetError (true));
			return;
		}

		m_nCurPodID = nPodToLoad;
	}

	if (nPrevPodID != m_nCurPodID)
	{
		UnloadSong ();
		m_nCurSongID = -1;
	}

	m_oCurrentPodList.DeleteAllItems ();
	for (int i = 0; i < arrSongIDs.GetSize (); i ++)
	{
		CString strSongTitle, strSongArtist, strSongAlbum, strPathToMp3;
		BOOL	bSongStillInCompetition = false;
		m_oSongManager.GetSongDetails (arrSongIDs[i], strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, bSongStillInCompetition);

		if (-1 == m_nCurSongID)
			m_nCurSongID = arrSongIDs[i];

		int nListCtrlIndex = m_oCurrentPodList.InsertItem (i, CUtils::NumberToString (i + 1));
		m_oCurrentPodList.SetItemText (nListCtrlIndex, LIST_POD_COL_TITLE,	strSongTitle);
		m_oCurrentPodList.SetItemText (nListCtrlIndex, LIST_POD_COL_ARTIST, strSongArtist);
		m_oCurrentPodList.SetItemData (nListCtrlIndex, arrSongIDs[i]);

		if (m_nCurSongID == arrSongIDs[i])
		{
			m_oCurrentPodList.SelectItem (nListCtrlIndex, true);
			LoadSongIntoPlayer (m_nCurSongID);
		}
	}

	//
	//  If we haven't selected anything, select the current one

	CWnd* pSubmit = GetDlgItem (IDC_SUBMIT_POD_RANKINGS);
	pSubmit->EnableWindow (!m_bCurPodFinished);

	if (nPrevPodID != m_nCurPodID)
		UpdateAccessoryListCtrl ();	//  New stats!


} // end CSongsBestPickerDlg::UpdateCurrentPod



//************************************
// Method:    SetSongRank
// FullName:  CSongsBestPickerDlg::SetSongRank
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nRank
//************************************
bool CSongsBestPickerDlg::SetSongRank (int nRank)
{
	//
	//  I had this setting the rank of the currently playing song,
	//  but I think I want it to be whichever is selected in the pod list ctrl.
	//  That way they can move things around while another song plays.
	//  Which is what I just tried to do and failed.

	int nListCtrlIndex = m_oCurrentPodList.GetFirstSelectedItem ();
	if (-1 == nListCtrlIndex)
		return false;

	int nSongID = (int) m_oCurrentPodList.GetItemData (nListCtrlIndex);

	//
	//  Let's just re-label our song, then call sort... actually, 
	//  InsertItem will insert it sorted for us.

	m_oCurrentPodList.SetRedraw (false);

	CString strTitleWeMoved, strArtistWeMoved;
	int		nSongIdWeMoved = -1;

	for (int i = 0; i < m_oCurrentPodList.GetItemCount (); i ++)
	{
		int nListCtrlSongID = (int) m_oCurrentPodList.GetItemData (i);
		if (nListCtrlSongID == nSongID)
		{
			strTitleWeMoved		= m_oCurrentPodList.GetItemText (i, LIST_POD_COL_TITLE);
			strArtistWeMoved	= m_oCurrentPodList.GetItemText (i, LIST_POD_COL_ARTIST);
			m_oCurrentPodList.DeleteItem (i);
			break;
		}
	}

	for (int i = 0; i < m_oCurrentPodList.GetItemCount (); i ++)
	{
		if (i < (nRank-1))
			m_oCurrentPodList.SetItemText (i, LIST_POD_COL_RANK, CUtils::NumberToString (i + 1));
		else 
			m_oCurrentPodList.SetItemText (i, LIST_POD_COL_RANK, CUtils::NumberToString (i + 1 + 1));	// Extra + 1 because we'll insert one before these
	}

	//
	//  And put our newly re-ranked song back in.  List will sort it by the song rank so will be in the right order.

	int nIndex = m_oCurrentPodList.InsertItem (nRank - 1, CUtils::NumberToString (nRank));
	m_oCurrentPodList.SetItemText (nIndex, LIST_POD_COL_TITLE,	strTitleWeMoved);
	m_oCurrentPodList.SetItemText (nIndex, LIST_POD_COL_ARTIST, strArtistWeMoved);
	m_oCurrentPodList.SetItemData (nIndex, nSongID);

	m_oCurrentPodList.SelectItem (nIndex, true);	//  We don't have to unselect anything since this song was selected before we deleted it

	//
	//  Have it redraw itself

	m_oCurrentPodList.SetRedraw (true);
	m_oCurrentPodList.Invalidate ();

	return true;

} // end CSongsBestPickerDlg::SetSongRank



//************************************
// Method:    ExportSongData
// FullName:  CSongsBestPickerDlg::ExportSongData
// Access:    public 
// Returns:   bool
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnExportSongData ()
{
	CFileDialog oDlg (false, L"*.*");
	if (IDOK != oDlg.DoModal ())
		return;

	CStdioFileExPtr pOutFile = CStdioFileExPtr (new CStdioFileEx);
	if (! pOutFile->Open (oDlg.GetPathName (), CFile::modeCreate | CFile::modeWrite)) {
		SetError (L"Unable to open output file: " + oDlg.GetPathName (), true);
		return;
	}

	//
	//  Export the column names

	for (int i = 0; i < m_oSongList.GetColumnCount (); i ++)
	{
		if (i > 0)
			pOutFile->WriteString (L"\t");
		pOutFile->WriteString (m_oSongList.GetColumnName (i));
	}
	pOutFile->WriteString (L"\n");


	//
	//  Then the data

	int nSongCount = 0;
	m_oSongManager.GetSongCount (nSongCount);
	int nLastID = -1;
	for (int i = 0; i < nSongCount; i ++)
	{
		CString strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, strWonLoss;
		if (!m_oSongManager.GetNextSong (strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, nLastID, nLastID))
		{
			SetError (m_oSongManager.GetError (true), true);
			return;
		}

		CString strToDisplay;
		GetDisplayStringForCol (nLastID, 0, strToDisplay);
		pOutFile->WriteString (strToDisplay);

		for (int nCol = 1; nCol < m_arrActiveColumns.GetSize (); nCol ++)
		{
			pOutFile->WriteString (L"\t");

			int nListCtrlIndex = m_oSongList.FindByItemData (nLastID);

			if (GetDisplayStringForCol (nLastID, nCol, strToDisplay, nListCtrlIndex))
				pOutFile->WriteString (strToDisplay);
		}
		pOutFile->WriteString (L"\n");
	} // end loop through songs

	pOutFile->Close ();

} // end CSongsBestPickerDlg::ExportSongData




//************************************
// Method:    GetDisplayStringForCol
// FullName:  CSongsBestPickerDlg::GetDisplayStringForCol
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nColIndex
// Parameter: CString & rstrDisplayString
//************************************
bool CSongsBestPickerDlg::GetDisplayStringForCol (int nSongID, int nColIndex, CString& rstrDisplayString, int nListCtrlIndex /* = -1 */, bool* pbCache /* = NULL */)
{
	if (nColIndex < 0 || nColIndex >= m_arrActiveColumns.GetSize ())
		return false;

	if (NULL != pbCache)
		*pbCache = true;

	switch (m_arrActiveColumns[nColIndex])
	{
	case LIST_SONG_COL_TITLE:
	{
		rstrDisplayString = GetSongTitle (nSongID);
		return true;
	}
	case LIST_SONG_COL_ARTIST:
	{
		rstrDisplayString = GetSongArtist (nSongID);
		return true;
	}
	case LIST_SONG_COL_ALBUM:
	{
		rstrDisplayString = GetSongAlbum (nSongID);
		return true;
	}
	case LIST_SONG_COL_WONLOSS:
	{
		int nWon = 0, nLost = 0;
		if (! GetWonLossRecord (nSongID, nWon, nLost))
			return SetError (m_oSongManager.GetError (true));
		rstrDisplayString.Format (L"%2d - %2d", nWon, nLost);
		return true;
	}
	case LIST_SONG_COL_RATING:
	{
		int nRating = 0;
		if (! GetSongRating (nSongID, nRating))
			return SetError (m_oSongManager.GetError (true));
		rstrDisplayString = CUtils::N2S (nRating);
		return true;
	}
	case LIST_SONG_COL_MP3:
	{
		rstrDisplayString = GetSongPathToMp3 (nSongID);
		return true;
	}
	case LIST_SONG_COL_SOS:
	{
		int nSoS = 0;
		if (! GetSongStrengthOfSchedule (nSongID, nSoS))
			return SetError (m_oSongManager.GetError (true));
		rstrDisplayString = CUtils::N2S (nSoS);
		return true;
	}
	case LIST_SONG_COL_ACTIVE:
	{
		if (GetSongStillInCompetition (nSongID))
			rstrDisplayString = L"1";
		else
			rstrDisplayString = L"0";
		return true;
	}
	case LIST_SONG_LIST_CTRL_INDEX:
	{
		if (-1 == nListCtrlIndex)
			rstrDisplayString = L"";
		else
			rstrDisplayString = CUtils::N2S (nListCtrlIndex + 1);

		//
		//  We don't cache this one

		if (NULL != pbCache)
			*pbCache = false;

		return true;
	}


	default:
		return false;
	}
} // end CSongsBestPickerDlg::GetDisplayStringForCol



//************************************
// Method:    GetColumnSetupInfo
// FullName:  CSongsBestPickerDlg::GetColumnSetupInfo
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nColIndex
// Parameter: int & rnColType
// Parameter: CString & rstrColName
// Parameter: int & rnFormat
// Parameter: int & rnWidth
//************************************
bool CSongsBestPickerDlg::GetColumnSetupInfo (int nColIndex, int& rnColType, CString& rstrColName, int& rnFormat, int& rnWidth)
{
	if (nColIndex < 0 || nColIndex >= m_arrActiveColumns.GetSize ())
		return false;

	//
	//  If they've saved something, use it

	if (m_oSongManager.GetColumnSetupInfo (nColIndex, rnColType, rstrColName, rnFormat, rnWidth))
		return true;

	//
	//  If not, use the defaults

	CRect rcSongList;
	m_oSongList.GetClientRect (&rcSongList);

	switch (m_arrActiveColumns[nColIndex])
	{
	case LIST_SONG_COL_TITLE:
	{
		rnColType = LIST_SONG_COL_TITLE;
		rstrColName = L"Title";
		rnFormat = LVCFMT_LEFT;
		rnWidth = (int) ((double) rcSongList.Width () * 0.24);
		return true;
	}
	case LIST_SONG_COL_ARTIST:
	{
		rnColType = LIST_SONG_COL_ARTIST;
		rstrColName = L"Artist";
		rnFormat = LVCFMT_LEFT;
		rnWidth = (int) ((double) rcSongList.Width () * 0.24);
		return true;
	}
	case LIST_SONG_COL_ALBUM:
	{
		rnColType = LIST_SONG_COL_ALBUM;
		rstrColName = L"Album";
		rnFormat = LVCFMT_LEFT;
		rnWidth = (int) ((double) rcSongList.Width () * 0.15);
		return true;
	}
	case LIST_SONG_COL_WONLOSS:
	{
		rnColType = LIST_SONG_COL_WONLOSS;
		rstrColName = L"Record";
		rnFormat = LVCFMT_CENTER;
		rnWidth = (int) ((double) rcSongList.Width () * 0.09);
		return true;
	}
	case LIST_SONG_COL_RATING:
	{
		rnColType = LIST_SONG_COL_RATING;
		rstrColName = L"Rating";
		rnFormat = LVCFMT_CENTER;
		rnWidth = (int) ((double) rcSongList.Width () * 0.08);
		return true;
	}
	case LIST_SONG_COL_MP3:
	{
		rnColType = LIST_SONG_COL_MP3;
		rstrColName = L"MP3";
		rnFormat = LVCFMT_LEFT;
		rnWidth = (int) ((double) rcSongList.Width () * 0.10);
		return true;
	}
	case LIST_SONG_COL_SOS:
	{
		rnColType = LIST_SONG_COL_SOS;
		rstrColName = L"SoS";
		rnFormat = LVCFMT_LEFT;
		rnWidth = (int) ((double) rcSongList.Width () * 0.10);
		return true;
	}
	case LIST_SONG_COL_ACTIVE:
	{
		rnColType = LIST_SONG_COL_ACTIVE;
		rstrColName = L"Act";
		rnFormat = LVCFMT_CENTER;
		rnWidth = (int) ((double) rcSongList.Width () * 0.05);
		return true;
	}
	case LIST_SONG_LIST_CTRL_INDEX:
	{
		rnColType = LIST_SONG_LIST_CTRL_INDEX;
		rstrColName = L"#";
		rnFormat = LVCFMT_RIGHT;
		rnWidth = (int) ((double) rcSongList.Width () * 0.04);
		return true;
	}


	default:
		return false;
	}
} // end CSongsBestPickerDlg::GetColumnSetupInfo



//************************************
// Method:    GetColumnType
// FullName:  CSongsBestPickerDlg::GetColumnType
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: int nColIndex
//
//
//
//************************************
int CSongsBestPickerDlg::GetColumnType (int nColIndex)
{
	if (nColIndex < 0 || nColIndex >= m_arrActiveColumns.GetSize ())
		return -1;

	return m_arrActiveColumns[nColIndex];

} // end CSongsBestPickerDlg::GetColumnType



//************************************
// Method:    SwapColumns
// FullName:  CSongsBestPickerDlg::SwapColumns
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSwapFrom
// Parameter: int nSwapTo
//
//
//
//************************************
bool CSongsBestPickerDlg::SwapColumns (int nSwapFrom, int nSwapTo)
{
	//
	//  Switch our columns...

	int nFromType = 0,		nFromFormat = 0,	nFromWidth = 0;
	int nToType = 0,		nToFormat = 0,		nToWidth = 0;
	CString strFromName, strToName;

	if (! m_oSongManager.GetColumnSetupInfo (nSwapFrom,	nFromType,	strFromName,	nFromFormat,	nFromWidth))
		return SetError (m_oSongManager.GetError (true));
	if (! m_oSongManager.GetColumnSetupInfo (nSwapTo,	nToType,	strToName,		nToFormat,		nToWidth))
		return SetError (m_oSongManager.GetError (true));

	if (! m_oSongManager.SetColumnSetupInfo (nSwapFrom,	nToType,	strToName,		nToFormat,		nToWidth))
		return SetError (m_oSongManager.GetError (true));
	if (! m_oSongManager.SetColumnSetupInfo (nSwapTo,	nFromType,	strFromName,	nFromFormat,	nFromWidth))
		return SetError (m_oSongManager.GetError (true));

	int nTemp = m_arrActiveColumns[nSwapFrom];
	m_arrActiveColumns[nSwapFrom]	= m_arrActiveColumns[nSwapTo];
	m_arrActiveColumns[nSwapTo]		= nTemp;

	return true;

} // end CSongsBestPickerDlg::SwapColumns




//************************************
// Method:    ShowSongListColumnChooser
// FullName:  CSongsBestPickerDlg::ShowSongListColumnChooser
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::ShowSongListColumnChooser ()
{
	CMenu	menuPopup;

	VERIFY(menuPopup.LoadMenu(IDR_MENU_SONG_POPUP));
	CMenu* pMenuCols = menuPopup.GetSubMenu(0);
	if (NULL == pMenuCols)
		return;

	while (pMenuCols->DeleteMenu (0, MF_BYPOSITION));

	//
	//  Now add our available columns to the menu

	for (int i = 0; i < SONG_LIST_AVAILABLE_COLUMN_COUNT; i++)
	{
		pMenuCols->InsertMenu (i, MF_STRING | MF_BYPOSITION, IDC_COLUMN_BLOCK_START + i, garrSongColsAval[i].strColumnName);
	
		//
		//  If it's currently shown, add check mark

		if (-1 != CUtils::FindNumberInArray (m_arrActiveColumns, garrSongColsAval[i].nType))
			pMenuCols->CheckMenuItem (i, MF_BYPOSITION | MF_CHECKED);
	}

	CPoint ptScreen;
	GetCursorPos (&ptScreen);
	pMenuCols->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, this); // | TPM_RETURNCMD

} // end CSongsBestPickerDlg::ShowSongListColumnChooser




//************************************
// Method:    OnHeaderDragCol
// FullName:  CSongsBestPickerDlg::OnHeaderDragCol
// Access:    public 
// Returns:   LRESULT
// Qualifier:
// Parameter: WPARAM wSource
// Parameter: LPARAM lDest
//************************************
LRESULT CSongsBestPickerDlg::OnHeaderDragCol (WPARAM wSource, LPARAM lDest)
{
	int nDragFrom	= (int) wSource;
	int nDropTo		= (int) lDest;

	//
	//  Depending on whether we're dragging right to left (or left to right), 
	//  we'll shift column #'s differently


	if (nDragFrom > nDropTo)
	{
		//
		//  Dragged right to left, so need to shift everything >= nDropTo up by 1
		//  To avoid having to store this column info here, we'll just swap our way up

		SwapColumns (nDragFrom, nDropTo);
		for (int i = nDropTo + 1; i <= nDragFrom; i ++)
		{
			SwapColumns (i, nDragFrom);	
		}
	}
	else
	{
		//
		//  Dragged left to right, so need to shift everything >= nDropTo up by 1
		//  To avoid having to store this column info here, we'll just swap our way up

		nDropTo --;
		SwapColumns (nDragFrom, nDropTo);
		for (int i = nDropTo - 1; i >= nDragFrom; i --)
		{
			SwapColumns (i, nDragFrom);	
		}
	}

	//
	//  // Because when we try to do it nicely in the listctlr, dragging to column 0 is buggy

	if (nDropTo == 0)
		UpdateSongList (); 
	return true;

} // end CSongsBestPickerDlg::OnHeaderDragCol









//************************************
// Method:    UpdateStatsForCurrentSong
// FullName:  CSongsBestPickerDlg::UpdateStatsForCurrentSong
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
//
//
//
//************************************
void CSongsBestPickerDlg::UpdateGameResultsForSong (int nSongID)
{
	UpdateData ();

	m_oSongGameResultList.DeleteAllItems ();
	if (-1 == nSongID)
	{
		m_strH2HCurrentCaption = L"";
		UpdateData (false);
		return;
	}

	CIntArray arrOpponents, arrMargins;
	if (! m_oSongManager.GetHeadToHeadForSong (nSongID, arrOpponents, arrMargins)) {
		SetError (m_oSongManager.GetError (true));
		return;
	}

	if (arrOpponents.GetSize () != arrMargins.GetSize ())
		return; 

	for (int i = 0; i < arrOpponents.GetSize (); i ++)
	{
		int		nOppRating = 0;
		CString strOpponent, strWonLoss;
		m_oSongManager.GetSongTitle (arrOpponents[i],	strOpponent);
		m_oSongManager.GetSongRating (arrOpponents[i],	nOppRating);

		if (arrMargins[i] > 0)
			strWonLoss = L"Won";
		else
			strWonLoss = L"Lost";

		int nIndex = m_oSongGameResultList.InsertItem (i, strOpponent);
		m_oSongGameResultList.SetItemText (nIndex, LIST_GAMES_COL_WONLOSS,	strWonLoss);
		m_oSongGameResultList.SetItemText (nIndex, LIST_GAMES_COL_RATING,	CUtils::N2S (nOppRating));
		m_oSongGameResultList.SetItemText (nIndex, LIST_GAMES_COL_MARGIN,	CUtils::NumberToString (arrMargins[i]));

		m_oSongGameResultList.SetItemData (nIndex, arrOpponents[i]);
	}

	int nOurRating = 0;
	m_oSongManager.GetSongRating (nSongID, nOurRating);

	CString strTitle;
	m_oSongManager.GetSongTitle (nSongID, strTitle);
	m_strH2HCurrentCaption = L"Results for " + strTitle + L" (" + CUtils::N2S (nOurRating) + L")";

	UpdateData (false);

	m_oSongGameResultList.SortItems (SortCompareGameResult, (DWORD_PTR) this);


} // end CSongsBestPickerDlg::UpdateStatsForCurrentSong





//************************************
// Method:    UpdatePlayerStatus
// FullName:  CSongsBestPickerDlg::UpdatePlayerStatus
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::UpdatePlayerStatus ()
{
	if (m_eSongPlayingStatus != ESongPlayStatus::ePlaying)
		return;

	if (NULL == m_pPlaybackChannel)
		return;

	UpdateData ();

	UINT nCurPosMS = 0;
	auto result = m_pPlaybackChannel->getPosition (&nCurPosMS, FMOD_TIMEUNIT_MS);
	if (result != FMOD_OK) {
		AfxMessageBox (L"Error loading song: " + CUtils::UTF8toUTF16 (FMOD_ErrorString (result)) + L"\r\n\r\n" + m_strCurSongPathToMp3);
		m_eSongPlayingStatus = ESongPlayStatus::eStopped;
		return;
	}

	int nCurPosSec	= nCurPosMS / 1000;
	int nMinutes	= nCurPosSec / 60;
	int nSecRemain	= nCurPosSec % 60;

	m_strSongPlaybackPos.Format (L"%2d:%02d", nMinutes, nSecRemain);
	m_oSongPlayingProgress.SetPos (nCurPosSec);
	UpdateData (false);

	UINT nSongLenMS = 0;
	result = m_pCurSong->getLength (&nSongLenMS, FMOD_TIMEUNIT_MS);
	if ((nSongLenMS <= nCurPosMS) && (m_eSongPlayingStatus == ESongPlayStatus::ePlaying))
		SetNextSongActive ();

} // end CSongsBestPickerDlg::UpdatePlayerStatus




//************************************
// Method:    UpdateGeneralStats
// FullName:  CSongsBestPickerDlg::UpdateGeneralStats
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::UpdateAccessoryListCtrl ()
{
	m_oAccessoryList.DeleteAllItems ();
	int nValue = 0, nIndex = -1;

	switch (m_eWhatIsInAccessoryList)
	{
	case EShowingInList::eGeneralStats:
	{
		//
		//  # songs

		int nSongCount = 0;
		nIndex = m_oAccessoryList.InsertItem (0, L"Total Songs");
		m_oSongManager.GetSongCount (nSongCount);
		m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, CUtils::N2S (nSongCount));

		//
		//  Average rating in this pod

		nIndex = m_oAccessoryList.InsertItem (0, L"Current Pod Avg Rating");
		
		CIntArray arrCurPodSongIDs;
		int		nTotal = 0;
		bool	bPodFinished = false;
		m_oSongManager.GetPod (m_nCurPodID, arrCurPodSongIDs, bPodFinished);
		for (int i = 0; i < arrCurPodSongIDs.GetSize (); i ++)
		{
			int nRating = 0;
			m_oSongManager.GetSongRating (arrCurPodSongIDs[i], nRating);
			nTotal += nRating;
		}

		float fAverage = 0;
		if (arrCurPodSongIDs.GetSize () > 0)
			fAverage = (float) nTotal / arrCurPodSongIDs.GetSize ();

		m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, CUtils::N2S (fAverage));

		//
		//  Average SoS in this pod

		nIndex = m_oAccessoryList.InsertItem (0, L"Current Pod Avg SoS");
		nTotal = 0;
		for (int i = 0; i < arrCurPodSongIDs.GetSize (); i ++)
		{
			int nRating = 0;
			m_oSongManager.GetSongStrengthOfSchedule (arrCurPodSongIDs[i], nRating);
			nTotal += nRating;
		}

		fAverage = 0;
		if (arrCurPodSongIDs.GetSize () > 0)
			fAverage = (float) nTotal / arrCurPodSongIDs.GetSize ();

		m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, CUtils::N2S (fAverage));

		//
		//  # pods ranked

		nIndex = m_oAccessoryList.InsertItem (0, L"Pods Ranked");
		m_oSongManager.GetFinishedPodCount (nValue);
		m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, CUtils::N2S (nValue));

		//
		//  Undefeated songs

		nIndex = m_oAccessoryList.InsertItem (0, L"Undefeated Songs");
		m_oSongManager.GetUndefeatedSongCount (nValue);
		m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, CUtils::N2S (nValue));

		//
		//  Games played per song

		nIndex = m_oAccessoryList.InsertItem (0, L"Avg Games per Song");
		m_oSongManager.GetTotalHeadToHeadCount (nValue);
		if (0 != nSongCount)
			m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, CUtils::N2S ((float)(2*nValue) / nSongCount));

		//
		//  Total artists

		nIndex = m_oAccessoryList.InsertItem (0, L"Total Artists");
		m_oSongManager.GetArtistCount (nValue);
		m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, CUtils::N2S (nValue));
	}

	case EShowingInList::eErrorLog:
	{
		for (int i = 0; i < m_arrErrors.GetSize (); i ++)
		{
			int nIndex = m_oAccessoryList.InsertItem (i, CUtils::N2S (i + 1));
			m_oAccessoryList.SetItemText (nIndex, LIST_STATS_VALUE, m_arrErrors[i]);
		}

		break;
	}

	default:
		AfxMessageBox (L"I don't know what to show in the accessory list control");
	}
} // end CSongsBestPickerDlg::UpdateGeneralStats










//************************************
// Method:    UpdatePodCombo
// FullName:  CSongsBestPickerDlg::UpdatePodCombo
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::UpdatePodCombo ()
{
	int nPrevSelPodID	= -1;
	int nCurSelIndex	= m_oPodCombo.GetCurSel ();
	if (-1 != nCurSelIndex)
		nPrevSelPodID = (int) m_oPodCombo.GetItemData (nCurSelIndex);

	//
	//  Our current pod is always on here and always first

	CIntArray	arrSongIDs;
	int			nCurPodID = -1;
	m_oSongManager.GetCurrentPod (nCurPodID, arrSongIDs);
	CString		strCurrent; strCurrent.Format (L"Pod %d (Current Pod)", nCurPodID);

	m_oPodCombo.ResetContent ();

	int nIndex = m_oPodCombo.AddString (strCurrent);
	m_oPodCombo.SetItemData (nIndex, nCurPodID);

	bool bSelected = false;

	for (int i = 0; i < m_arrPodComboIDs.GetSize (); i ++)
	{
		int nIndex = m_oPodCombo.AddString (L"Pod " + CUtils::N2S (m_arrPodComboIDs[i]));
		m_oPodCombo.SetItemData (nIndex, m_arrPodComboIDs[i]);

		if (! bSelected && (nPrevSelPodID == m_arrPodComboIDs[i])) {
			m_oPodCombo.SetCurSel (nIndex);
			bSelected = true;	 // we do this so we'll select the top "Current Pod" choice if our current pod is in here twice
		}
	}

	if (!bSelected && (m_oPodCombo.GetCount () > 0))
		m_oPodCombo.SetCurSel (0);

} // end CSongsBestPickerDlg::UpdatePodCombo



//************************************
// Method:    OnEditHotkeys
// FullName:  CSongsBestPickerDlg::OnEditHotkeys
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnEditHotkeys ()
{
	CHotkeyManagementDlg oHK (&m_oHotkeyManager);
	if (IDOK != oHK.DoModal ())
		return;

	ApplyHotkeys ();
	m_oHotkeyManager.SaveHotkeys (m_oSongManager.GetDB ());

} // end CSongsBestPickerDlg::OnEditHotkeys






//************************************
// Method:    OnViewGeneralStats
// FullName:  CSongsBestPickerDlg::OnViewGeneralStats
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnViewGeneralStats ()
{
	m_eWhatIsInAccessoryList = EShowingInList::eGeneralStats;
	UpdateAccessoryListCtrl ();

} // end CSongsBestPickerDlg::OnViewGeneralStats



//************************************
// Method:    OnViewErrorLogs
// FullName:  CSongsBestPickerDlg::OnViewErrorLogs
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnViewErrorLog ()
{
	m_eWhatIsInAccessoryList = EShowingInList::eErrorLog;
	UpdateAccessoryListCtrl ();

} // end CSongsBestPickerDlg::OnViewErrorLogs



//************************************
// Method:    OnClearErrorLog
// FullName:  CSongsBestPickerDlg::OnClearErrorLog
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::OnClearErrorLog ()
{
	m_arrErrors.SetSize (0);
	UpdateAccessoryListCtrl ();

} // end CSongsBestPickerDlg::OnClearErrorLog



void CSongsBestPickerDlg::OnBnClickedApplyHotkeys()
{
	if (! m_bHotkeysApplied)
	{
		ApplyHotkeys ();
	}
	else
	{
		RemoveHotkeys ();
	}
}



void CSongsBestPickerDlg::ApplyHotkeys ()
{
//	for (int i = 0; i < HOTKEY_INIT_NUM_COMMANDS; i ++)
	for (int i = 0; i < m_oHotkeyManager.GetCommandCount (); i ++)
	{
		//
		//  Add hotkey registers this with the system.  It used to also display them in a list ctrl.

		CHotkeyCommand* pHKC = m_oHotkeyManager.GetCommandByIndex (i);
		if (NULL == pHKC)
			continue;

		//
		//  Each command can have multiple hotkeys that trigger it

		for (int nHotkey = 0; nHotkey < pHKC->GetHotkeyCount (); nHotkey ++)
		{
			UINT nKey = 0, nModifiers = 0;
			pHKC->GetHotkeyByIndex (nHotkey, nKey, nModifiers);

			if (! RegisterHotKey (m_hWnd, pHKC->GetID (), nModifiers, nKey))
			{
				CString s; s.Format (L"Unable to register hotkey: %s:  %s\n", pHKC->GetName (), CUtils::GetHotkeyText (nKey, nModifiers));
				SetError (s);
			}
		}
	}

	m_bHotkeysApplied = true;

} // end ApplyHotkeys








void CSongsBestPickerDlg::RemoveHotkeys ()
{
	for (auto it  = m_mapHotkeys.begin (); it != m_mapHotkeys.end (); it ++)
	{
		UnregisterHotKey (m_hWnd, (*it).first);
	}

	m_mapHotkeys.clear ();
	m_bHotkeysApplied = false;

	CWnd* pWnd = GetDlgItem(ID_APPLY_HOTKEYS);
	if (NULL != pWnd)
		pWnd->SetWindowText(_T("Apply Hotkeys"));

	if (NULL != m_oSongList.GetSafeHwnd ())
	{
		m_oSongList.DeleteAllItems ();
	}
}




void CSongsBestPickerDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (! OnHandleHotkey (nHotKeyId))
		CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);

//	m_nHotkeyDownTimerId = SetTimer (1, 100, NULL);
}



//************************************
// Method:    OnHandleHotkey
// FullName:  CSongsBestPickerDlg::OnHandleHotkey
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: UINT nHotKeyId
//
//  Returns true if we handle the hotkey.  False if not.
//
//************************************
bool CSongsBestPickerDlg::OnHandleHotkey (UINT nHotKeyId)
{
	CWnd* pActiveWnd = GetForegroundWindow ();
	if (NULL == pActiveWnd)
		return false;

	CRect rcWnd;
	pActiveWnd->GetWindowRect (rcWnd);

	//
	//  Left / right

	switch (nHotKeyId)
	{
	case ID_SHOW_OUR_APP:
		OnShowMyWindow ();
//		this->ShowWindow (SW_NORMAL);
		break;

	case ID_PLAY_OR_PAUSE_SONG:
		return PlayOrPauseSong ();
	case ID_NEXT_SONG:
		return SetNextSongActive ();
	case ID_PREV_SONG:
		return SetPrevSongActive ();

	case ID_RANK_1:
		return SetSongRank (1);
	case ID_RANK_2:
		return SetSongRank (2);
	case ID_RANK_3:
		return SetSongRank (3);
	case ID_RANK_4:
		return SetSongRank (4);
	case ID_RANK_5:
		return SetSongRank (5);
	}

	return false;

} // end on handle hotkey




//************************************
// Method:    PlayOrPauseSong
// FullName:  CSongsBestPickerDlg::PlayOrPauseSong
// Access:    public 
// Returns:   bool
// Qualifier:
//
//
//
//************************************
bool CSongsBestPickerDlg::PlayOrPauseSong ()
{
	if (m_eSongPlayingStatus == ESongPlayStatus::ePlaying)
		PauseSong ();
	else 
		PlaySong ();

	return true;
} // end CSongsBestPickerDlg::PlayOrPauseSong



//************************************
// Method:    SetNextSongActive
// FullName:  CSongsBestPickerDlg::SetNextSongActive
// Access:    public 
// Returns:   bool
// Qualifier:
//************************************
bool CSongsBestPickerDlg::SetNextSongActive ()
{
	StopSong ();

	int nListCtrlIndex	= m_oCurrentPodList.GetFirstSelectedItem ();
	if (-1 != nListCtrlIndex)
		m_oCurrentPodList.UnselectItem (nListCtrlIndex);

	//
	//  Remember, we go in order the songs are in the database, NOT the current
	//  ranking of the pod.  

	CIntArray arrCurrentPodSongIDs;
	if (! m_oSongManager.GetCurrentPod (m_nCurPodID, arrCurrentPodSongIDs))
		return SetError (m_oSongManager.GetError (true));

	int nNextSongID = -1;
	for (int i = 0; i < arrCurrentPodSongIDs.GetSize (); i ++)
	{
		if (arrCurrentPodSongIDs[i] == m_nCurSongID)
		{
			//
			//  Aha!

			if (i < (arrCurrentPodSongIDs.GetSize () - 1))
				nNextSongID = arrCurrentPodSongIDs[i + 1];
			else if (arrCurrentPodSongIDs.GetSize () > 0)
				nNextSongID = arrCurrentPodSongIDs[0];
			else
				return false;
		}
	}

	m_oCurrentPodList.SelectItem (m_oCurrentPodList.FindByItemData (nNextSongID));

	LoadSongIntoPlayer (nNextSongID, false);
	PlaySong ();	// No arg plays the current song... which we just loaded
	return true;

} // end CSongsBestPickerDlg::SetNextSongActive



//************************************
// Method:    SetPrevSongActive
// FullName:  CSongsBestPickerDlg::SetPrevSongActive
// Access:    public 
// Returns:   bool
// Qualifier:
//************************************
bool CSongsBestPickerDlg::SetPrevSongActive ()
{
	StopSong ();

	int nListCtrlIndex	= m_oCurrentPodList.GetFirstSelectedItem ();
	if (-1 != nListCtrlIndex)
		m_oCurrentPodList.UnselectItem (nListCtrlIndex);
	//
	//  Remember, we go in order the songs are in the database, NOT the current
	//  ranking of the pod.  

	CIntArray arrCurrentPodSongIDs;
	if (! m_oSongManager.GetCurrentPod (m_nCurPodID, arrCurrentPodSongIDs))
		return SetError (m_oSongManager.GetError (true));

	int nNextSongID = -1;
	for (int i = 0; i < arrCurrentPodSongIDs.GetSize (); i ++)
	{
		if (arrCurrentPodSongIDs[i] == m_nCurSongID)
		{
			//
			//  Aha!

			if (i > 0)
				nNextSongID = arrCurrentPodSongIDs[i - 1];
			else if (arrCurrentPodSongIDs.GetSize () > 0)
				nNextSongID = (int)arrCurrentPodSongIDs[arrCurrentPodSongIDs.GetSize () - 1];
			else
				return false;
		}
	}

	m_oCurrentPodList.SelectItem (m_oCurrentPodList.FindByItemData (nNextSongID));

	LoadSongIntoPlayer (nNextSongID, false);
	PlaySong ();	// No arg plays the current song... which we just loaded
	return true;

} // end CSongsBestPickerDlg::SetPrevSongActive






//************************************
// Method:    OnTimer
// FullName:  CSongsBestPickerDlg::OnTimer
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT_PTR nIDEvent
//************************************
void CSongsBestPickerDlg::OnTimer (UINT_PTR nIDEvent) 
{

	if (nIDEvent == m_nSongPlayingStatusTimerID)
	{
		KillTimer (m_nSongPlayingStatusTimerID);
		UpdatePlayerStatus ();
		m_nSongPlayingStatusTimerID = SetTimer (SONG_STATUS_TIMER_ID, SONG_STATUS_TIMER_MS, NULL);
		return;
	}

	else if (nIDEvent == m_nTypeToFilterPodTimerId)
	{
		KillTimer (m_nTypeToFilterPodTimerId);

		CString strFilter;
		m_oTypeToFilterPod.GetWindowText (strFilter);
		
		if (strFilter != m_strTypeToFilterEmptyMsg)
			UpdateTypeToFilterDisplay (strFilter);

		return;
	}

	__super::OnTimer (nIDEvent);
} // end on timer




//************************************
// Method:    OnShowMyWindow
// FullName:  CSongsBestPickerDlg::OnShowMyWindow
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnShowMyWindow ()
{
	if (IsWindowVisible () && (this == GetForegroundWindow ()))
		m_oTrayIcon.MinimiseToTray (this);
	else
		m_oTrayIcon.MaximiseFromTray (this);
}




//************************************
// Method:    OnItemChangedSongList
// FullName:  CSongsBestPickerDlg::OnItemChangedSongList
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: NMHDR * pNMHDR
// Parameter: LRESULT * pResult
//
//
//
//************************************
void CSongsBestPickerDlg::OnItemChangedSongList (NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	//
	//  Remember that we get this notification both when an item is selected AND
	//  when it is unselected.  We only want to handle the selection

	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVIS_SELECTED))
	{
		//
		//  Woohoo!  We have a newly selected item.

		int nSongID = -1;
		int nListCtrlIndex = m_oSongList.GetFirstSelectedItem ();
		if (-1 == nListCtrlIndex) 
			nSongID = -1;
		else {
			nSongID = (int)m_oSongList.GetItemData (nListCtrlIndex);
			UpdateGameResultsForSong (nSongID);
		} // end if we have a valid song to check out

#ifdef LoadSongsFromMainList
		SaveSongInfoFromPlayer ();
		LoadSongIntoPlayer (nSongID);
#endif
	} // end if a new item is selected

} // end item changed in song list






//************************************
// Method:    OnItemChangedCurrentPodList
// FullName:  CSongsBestPickerDlg::OnItemChangedCurrentPodList
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: NMHDR * pNMHDR
// Parameter: LRESULT * pResult
//************************************
void CSongsBestPickerDlg::OnItemChangedCurrentPodList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	//
	//  Remember that we get this notification both when an item is selected AND
	//  when it is unselected.  We only want to handle the selection

	if ((pNMLV->uChanged & LVIF_STATE) && (pNMLV->uNewState & LVIS_SELECTED))
	{
		UpdateData ();
		if (! m_bUpdateResultsForPodSel)
			return;

		//
		//  Woohoo!  We have a newly selected item.

		int nSongID = -1;
		int nListCtrlIndex = m_oCurrentPodList.GetFirstSelectedItem ();
		if (-1 == nListCtrlIndex) 
			nSongID = -1;
		else {
			nSongID = (int)m_oCurrentPodList.GetItemData (nListCtrlIndex);
			UpdateGameResultsForSong (nSongID);
		} // end if we have a valid song to check out
	}

	*pResult = 0;
} // end item changed current pod list





//************************************
// Method:    OnDblclkCurrentPodList
// FullName:  CSongsBestPickerDlg::OnDblclkCurrentPodList
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: NMHDR * pNMHDR
// Parameter: LRESULT * pResult
//************************************
void CSongsBestPickerDlg::OnDblclkCurrentPodList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	int nListCtrlIndex = pNMItemActivate->iItem;
	if (-1 == nListCtrlIndex)
		return;

	int nSongID = (int) m_oCurrentPodList.GetItemData (nListCtrlIndex);
	LoadSongIntoPlayer (nSongID);
	PlaySong ();

	*pResult = 0;
}




//************************************
// Method:    LoadSongIntoPlayer
// FullName:  CSongsBestPickerDlg::LoadSongIntoPlayer
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSondID
//************************************
void CSongsBestPickerDlg::LoadSongIntoPlayer (int nSongID, bool bShowGameResults /* = true */)
{
	UpdateData ();
	MakePlayerInfoEditable (false);

	if (nSongID != m_nCurSongID) {
		UnloadSong ();
	}

	m_nCurSongID = nSongID;

	if (-1 == nSongID) {
		m_strCurSongTitle.Empty ();
		m_strCurSongPathToMp3.Empty ();
		m_strCurSongAlbum.Empty ();
		m_strCurSongArtist.Empty ();
	}
	else
	{
		m_oSongManager.GetSongDetails (nSongID, m_strCurSongTitle, m_strCurSongArtist, m_strCurSongAlbum, m_strCurSongPathToMp3, m_bSongIsStillInCompetition);
	}

	UpdateData (false);

//	if (bShowGameResults)
//		UpdateGameResultsForCurrentSong	(nSongID);	// NOT the list ctrl index
//	else
		UpdateGameResultsForSong	(-1);		// Clears the game results list

} // end load song into player



//************************************
// Method:    SaveSongInfoFromPlayer
// FullName:  CSongsBestPickerDlg::SaveSongInfoFromPlayer
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::SaveSongInfoFromPlayer ()
{
	UpdateData ();

	//
	//  Save any changes the user made

	m_strCurSongTitle.Trim ();
	m_strCurSongAlbum.Trim ();
	m_strCurSongArtist.Trim ();
	m_strCurSongPathToMp3.Trim ();

	m_oSongManager.SetSongDetails	(m_nCurSongID, m_strCurSongTitle, m_strCurSongArtist, m_strCurSongAlbum, m_strCurSongPathToMp3, m_bSongIsStillInCompetition);
	UpdateSongListSpecificSong		(m_nCurSongID); // , m_strCurSongTitle, m_strCurSongArtist, m_strCurSongAlbum, m_strCurSongPathToMp3);

	MakePlayerInfoEditable (false);

} // end CSongsBestPickerDlg::SaveSongInfoFromPlayer




//************************************
// Method:    MakePlayerInfoEditable
// FullName:  CSongsBestPickerDlg::MakePlayerInfoEditable
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: bool bEditable
//************************************
void CSongsBestPickerDlg::MakePlayerInfoEditable (bool bEditable)
{
	GetDlgItem (IDC_EDIT1)->EnableWindow (bEditable);
	GetDlgItem (IDC_EDIT_PATH_TO_MP3)->EnableWindow (true); // bEditable);	//  So I can always copy out path
	GetDlgItem (IDC_EDIT_ARTIST)->EnableWindow (bEditable);
	GetDlgItem (IDC_EDIT_ALBUM)->EnableWindow (bEditable);
	GetDlgItem (IDC_BROWSE_FOR_SONG)->EnableWindow (bEditable);
	GetDlgItem (IDC_SAVE_SONG_CHANGES)->EnableWindow (bEditable);
	GetDlgItem (IDC_CHECK_SONG_ACTIVE)->EnableWindow (bEditable);

} // end CSongsBestPickerDlg::MakePlayerInfoEditable



//************************************
// Method:    OnBnClickedPlaySong
// FullName:  CSongsBestPickerDlg::OnBnClickedPlaySong
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnBnClickedPlaySong ()
{
	//
	//  If we're already playing, this means stop, otherwise means play...

	if (m_eSongPlayingStatus == ESongPlayStatus::ePlaying)
		PauseSong ();
	else 
		PlaySong ();

} // end on play song






void CSongsBestPickerDlg::OnHeaderClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	if (phdr->hdr.hwndFrom == m_oSongList.GetHeaderCtrl ()->GetSafeHwnd ())
		SortColSongList (phdr->iItem);
	else if (phdr->hdr.hwndFrom == m_oSongGameResultList.GetHeaderCtrl ()->GetSafeHwnd ())
		SortColGameResult (phdr->iItem);

} // end on song header dbl click






//************************************
// Method:    SortColSongList
// FullName:  CSongsBestPickerDlg::SortColSongList
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nCol
//************************************
void CSongsBestPickerDlg::SortColSongList (int nCol)
{
	if (nCol == m_nSongsSortCol)
		m_bSongsSortAscending = !m_bSongsSortAscending;
	else
	{
		//
		//  Sort win pct or rating descending by default

		int nColType = GetColumnType (nCol);
		if (-1 == nColType)
			m_bSongsSortAscending = true;
		else
			m_bSongsSortAscending = garrSongColsAval[nColType].bDefaultSortAscending;
	}

	m_nSongsSortCol = nCol;

	CWaitCursor wc;
	m_oSongList.SortItems (SortCompareSongListCtrl, (DWORD_PTR) this);

} // end CSongsBestPickerDlg::SortColSongList



//************************************
// Method:    SortColGameResult
// FullName:  CSongsBestPickerDlg::SortColHeadToHead
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nCol
//************************************
void CSongsBestPickerDlg::SortColGameResult (int nCol)
{
	if (nCol == m_nGamesSortCol)
		m_bGamesSortAscending = !m_bGamesSortAscending;
	else
	{
		//
		//  Sort win pct or rating descending by default

		switch (nCol)
		{
		case LIST_GAMES_COL_RATING:
		case LIST_GAMES_COL_MARGIN:
			m_bGamesSortAscending = false;
			break;

		default:
			m_bGamesSortAscending = true;
		}
	}

	m_nGamesSortCol = nCol;

	CWaitCursor wc;
	m_oSongGameResultList.SortItems (SortCompareGameResult, (DWORD_PTR) this);

} // end CSongsBestPickerDlg::SortColHeadToHead




//************************************
// Method:    GetCurrentPodSongRank
// FullName:  CSongsBestPickerDlg::GetCurrentPodSongRank
// Access:    protected 
// Returns:   int
// Qualifier:
// Parameter: int nSongID
//************************************
int CSongsBestPickerDlg::GetCurrentPodSongRank (int nSongID)
{
	for (int i = 0; i < m_oCurrentPodList.GetItemCount (); i ++)
	{
		if (nSongID == (int) m_oCurrentPodList.GetItemData (i))
		{
			CString strRank = m_oCurrentPodList.GetItemText (i, LIST_POD_COL_RANK);
			int nRank = 0;
			if (CUtils::MyAtoI (strRank, nRank))
				return nRank;
			return -1;
		}
	}

	return -1;

} // end get current pod song rank



int CSongsBestPickerDlg::ComparePodSongRank (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	//  It's passing in the GetItemData number to us...  use that to find the rank and sort on that.

	int nSongID1 = (int) lParam1;
	int nSongID2 = (int) lParam2;

	CSongsBestPickerDlg* pDlg = (CSongsBestPickerDlg*) lParamSort;

	int nSongRank1 = pDlg->GetCurrentPodSongRank (nSongID1);
	int nSongRank2 = pDlg->GetCurrentPodSongRank (nSongID2);

	if (nSongRank1 < nSongRank2)
		return -1;
	if (nSongRank2 > nSongRank1)
		return 1;
	return 0;

} // end compare pod song rank



//************************************
// Method:    SetError
// FullName:  CSongsBestPickerDlg::SetError
// Access:    protected 
// Returns:   void
// Qualifier:
// Parameter: CString strError
//************************************
bool CSongsBestPickerDlg::SetError (CString strError, bool bAlertUser /* = false */, bool bRetVal /* = false */)
{
	TRACE (strError + L"\n");

	m_arrErrors.Add (strError);
	OnViewErrorLog ();

	if (bAlertUser)
		AfxMessageBox (strError);

	return bRetVal;

} // end CSongsBestPickerDlg::SetError



int CSongsBestPickerDlg::SortCompareSongListCtrl (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	//  It's passing in the GetItemData number to us...  we're sorting on that
	//  anyway.

	int nSongID1 = (int) lParam1;
	int nSongID2 = (int) lParam2;

	CSongsBestPickerDlg* pDlg = (CSongsBestPickerDlg*) lParamSort;

	int nOrderMultiplier = 1;
	if (! pDlg->m_bSongsSortAscending)
		nOrderMultiplier = -1;

	//
	//  Sort differently based on which column we're sorting

	switch (pDlg->GetColumnType (pDlg->m_nSongsSortCol))
	{
	case LIST_SONG_COL_ARTIST:
	{
		//
		//  Sorting on name column...  this is either alpha or by classification order

		CString str1 = pDlg->GetSongArtist (nSongID1);
		CString str2 = pDlg->GetSongArtist (nSongID2);

		int nResult = str1.CompareNoCase (str2);
		if (0 != nResult)
			return nOrderMultiplier * str1.CompareNoCase(str2);
		//
		//  Otherwise return the title compare
		break;
	}
	case LIST_SONG_COL_ALBUM:
	{
		//
		//  Sorting on name column...  this is either alpha or by classification order

		CString str1 = pDlg->GetSongAlbum (nSongID1);
		CString str2 = pDlg->GetSongAlbum (nSongID2);

		return nOrderMultiplier * str1.CompareNoCase(str2);
	}

	case LIST_SONG_COL_WONLOSS: 
	{
		//
		//  Sorting on confidence

		int nSong1Wins = 0, nSong1Losses = 0;
		int nSong2Wins = 0, nSong2Losses = 0;

		pDlg->GetWonLossRecord (nSongID1, nSong1Wins, nSong1Losses);
		pDlg->GetWonLossRecord (nSongID2, nSong2Wins, nSong2Losses);

		float fWinPct1 = 0, fWinPct2 = 0;
		if (nSong1Wins + nSong1Losses > 0)
			fWinPct1 = ((float)nSong1Wins / (nSong1Wins + nSong1Losses));
		if (nSong2Wins + nSong2Losses > 0)
			fWinPct2 = ((float)nSong2Wins / (nSong2Wins + nSong2Losses));

		if (fWinPct1 < fWinPct2)
			return nOrderMultiplier * -1;
		if (fWinPct1 > fWinPct2)
			return nOrderMultiplier * 1;

		//
		//  If win pct is the same, reward or punish # of wins / losses

		if (nSong1Wins > nSong2Wins)
			return 1 * nOrderMultiplier;
		if (nSong2Wins > nSong1Wins)
			return -1 * nOrderMultiplier;

		if (nSong1Losses > nSong2Losses)
			return 1 * nOrderMultiplier;
		if (nSong2Losses > nSong1Losses)
			return -1 * nOrderMultiplier;

		//
		//  Otherwise return the title compare
		break;
	}
	case LIST_SONG_COL_RATING:
	{
		int nSong1Rating = 0, nSong2Rating = 0;

		pDlg->GetSongRating (nSongID1, nSong1Rating);
		pDlg->GetSongRating (nSongID2, nSong2Rating);

		if (nSong1Rating < nSong2Rating)
			return nOrderMultiplier * -1;
		if (nSong1Rating > nSong2Rating)
			return nOrderMultiplier * 1;

		//
		//  Otherwise return the title compare
		break;
	}
	case LIST_SONG_COL_MP3:
	{
		CString str1 = pDlg->GetSongPathToMp3 (nSongID1);
		CString str2 = pDlg->GetSongPathToMp3 (nSongID2);

		return nOrderMultiplier * str1.CompareNoCase(str2);
	}
	case LIST_SONG_COL_ACTIVE:
	{
		BOOL b1 = pDlg->GetSongStillInCompetition (nSongID1);
		BOOL b2 = pDlg->GetSongStillInCompetition (nSongID2);

		if (b1 != b2)
			return nOrderMultiplier * (b1 < b2);

		//
		//  Otherwise return the title compare
		break;
	}

	case LIST_SONG_LIST_CTRL_INDEX:
	{
		int nIndex1 = pDlg->GetListCtrlIndex (nSongID1);
		int nIndex2 = pDlg->GetListCtrlIndex (nSongID2);

		return nOrderMultiplier * (nIndex1 < nIndex2);
	}

	case LIST_SONG_COL_SOS:
	{
		int nSong1SoS = 0, nSong2SoS = 0;

		pDlg->GetSongStrengthOfSchedule (nSongID1, nSong1SoS);
		pDlg->GetSongStrengthOfSchedule (nSongID2, nSong2SoS);

		if (nSong1SoS < nSong2SoS)
			return nOrderMultiplier * -1;
		if (nSong1SoS > nSong2SoS)
			return nOrderMultiplier * 1;

		//
		//  Otherwise return the title compare
		break;
	}
	} // end switch

	//
	//  Title compare is our backup sort for every column.   That's why
	//  it's not part of the if...else structure

//	if (LIST_SONG_COL_TITLE == pDlg->m_nSongsSortCol)
	{
		//
		//  Sorting on name column...  this is either alpha or by classification order

		CString str1 = pDlg->GetSongTitle (nSongID1);
		CString str2 = pDlg->GetSongTitle (nSongID2);

		if (pDlg->m_nSongsSortCol != LIST_SONG_COL_TITLE)
			nOrderMultiplier = 1;	//  If we're a secondary sort col, we're always ascending.

		return nOrderMultiplier * str1.CompareNoCase(str2); // We always do secondary sort ascending
	}


	return 0;

} // end compare items




//************************************
// Method:    SortCompareGameResult
// FullName:  CSongsBestPickerDlg::SortCompareGameResult
// Access:    protected static 
// Returns:   int CALLBACK
// Qualifier:
// Parameter: LPARAM lParam1
// Parameter: LPARAM lParam2
// Parameter: LPARAM lParamSort
//
//
//************************************
int CALLBACK CSongsBestPickerDlg::SortCompareGameResult (LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	//  It's passing in the GetItemData number to us...  we're sorting on that
	//  anyway.

	int nSongID1 = (int) lParam1;
	int nSongID2 = (int) lParam2;

	CSongsBestPickerDlg* pDlg = (CSongsBestPickerDlg*) lParamSort;

	int nOrderMultiplier = 1;
	if (! pDlg->m_bGamesSortAscending)
		nOrderMultiplier = -1;

	//
	//  Sort differently based on which column we're sorting

	switch (pDlg->m_nGamesSortCol)
	{
	case LIST_GAMES_COL_RATING:
	{
		int nSong1Rating = 0, nSong2Rating = 0;

		pDlg->GetSongRating (nSongID1, nSong1Rating);
		pDlg->GetSongRating (nSongID2, nSong2Rating);

		if (nSong1Rating < nSong2Rating)
			return nOrderMultiplier * -1;
		if (nSong1Rating > nSong2Rating)
			return nOrderMultiplier * 1;

		//
		//  Otherwise return the title compare
		break;
	}

	case LIST_GAMES_COL_WONLOSS:
	{
		CString strVal1, strVal2;

		int nIndex = pDlg->m_oSongGameResultList.FindByItemData (nSongID1);
		if (-1 != nIndex) {
			strVal1 = pDlg->m_oSongGameResultList.GetItemText (nIndex, LIST_GAMES_COL_MARGIN);
		}

		nIndex = pDlg->m_oSongGameResultList.FindByItemData (nSongID2);
		if (-1 != nIndex) {
			strVal2 = pDlg->m_oSongGameResultList.GetItemText (nIndex, LIST_GAMES_COL_MARGIN);
		}

		int nResult = strVal1.CompareNoCase (strVal2);
		if (0 != nResult)
			return nOrderMultiplier * nResult;

		//
		//  Otherwise return the title compare
		break;
	}

	case LIST_GAMES_COL_MARGIN:
	{
		CString strVal;
		int nMargin1 = 0, nMargin2 = 0;

		int nIndex = pDlg->m_oSongGameResultList.FindByItemData (nSongID1);
		if (-1 != nIndex) {
			strVal = pDlg->m_oSongGameResultList.GetItemText (nIndex, LIST_GAMES_COL_MARGIN);
			CUtils::MyAtoI (strVal, nMargin1);
		}

		nIndex = pDlg->m_oSongGameResultList.FindByItemData (nSongID2);
		if (-1 != nIndex) {
			strVal = pDlg->m_oSongGameResultList.GetItemText (nIndex, LIST_GAMES_COL_MARGIN);
			CUtils::MyAtoI (strVal, nMargin2);
		}

		if (nMargin1 < nMargin2)
			return nOrderMultiplier * -1;
		if (nMargin1 > nMargin2)
			return nOrderMultiplier * 1;

		//
		//  Otherwise return the title compare
		break;
	}
	} // end switch

	//
	//  Title compare is our backup sort for every column.   That's why
	//  it's not part of the if...else structure

//	if (LIST_GAMES_COL_OPPONENT == pDlg->m_nSongsSortCol)
	{
		//
		//  Sorting on name column...  this is either alpha or by classification order

		CString str1 = pDlg->GetSongTitle (nSongID1);
		CString str2 = pDlg->GetSongTitle (nSongID2);

		if (pDlg->m_nSongsSortCol != LIST_GAMES_COL_OPPONENT)
			nOrderMultiplier = 1;	//  If we're a secondary sort col, we're always ascending.

		return nOrderMultiplier * str1.CompareNoCase(str2); // We always do secondary sort ascending
	}


	return 0;
} // end CSongsBestPickerDlg::SortCompareGameResult



//************************************
// Method:    GetSongNameByID
// FullName:  CSongsBestPickerDlg::GetSongNameByID
// Access:    public 
// Returns:   CString
// Qualifier:
// Parameter: int nSongID
//************************************
CString CSongsBestPickerDlg::GetSongTitle (int nSongID)
{
	CString strSongTitle, strPathToMp3, strSongArtist, strSongAlbum;
	BOOL	bSongStillInCompetition = false;
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, bSongStillInCompetition)) {
		SetError (m_oSongManager.GetError (true));
		return L"";
	}

	return strSongTitle;

} // end CSongsBestPickerDlg::GetSongNameByID



//************************************
// Method:    GetSongStillInCompetition
// FullName:  CSongsBestPickerDlg::GetSongStillInCompetition
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: int nSongID
//************************************
BOOL CSongsBestPickerDlg::GetSongStillInCompetition (int nSongID)
{
	CString strSongTitle, strPathToMp3, strSongArtist, strSongAlbum;
	BOOL	bSongStillInCompetition = false;
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, bSongStillInCompetition)) {
		SetError (m_oSongManager.GetError (true));
		return false;
	}

	return bSongStillInCompetition;

} // end CSongsBestPickerDlg::GetSongStillInCompetition





//************************************
// Method:    GetListCtrlIndex
// FullName:  CSongsBestPickerDlg::GetListCtrlIndex
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: int nSongID
//************************************
int CSongsBestPickerDlg::GetListCtrlIndex (int nSongID)
{
	return m_oSongList.FindByItemData (nSongID);

} // end CSongsBestPickerDlg::GetListCtrlIndex



//************************************
// Method:    GetSongArtist
// FullName:  CSongsBestPickerDlg::GetSongArtist
// Access:    public 
// Returns:   CString
// Qualifier:
// Parameter: int nSongID
//
//
//
//************************************
CString CSongsBestPickerDlg::GetSongArtist (int nSongID)
{
	CString strSongTitle, strPathToMp3, strSongArtist, strSongAlbum;
	BOOL	bSongStillInCompetition = false;
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, bSongStillInCompetition)) {
		SetError (m_oSongManager.GetError (true));
		return L"";
	}

	return strSongArtist;

} // end CSongsBestPickerDlg::GetSongArtist



//************************************
// Method:    GetSongAlbum
// FullName:  CSongsBestPickerDlg::GetSongAlbum
// Access:    public 
// Returns:   CString
// Qualifier:
// Parameter: int nSongID
//
//
//
//************************************
CString CSongsBestPickerDlg::GetSongAlbum (int nSongID)
{
	CString strSongTitle, strPathToMp3, strSongArtist, strSongAlbum;
	BOOL	bSongStillInCompetition = false;
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, bSongStillInCompetition)) {
		SetError (m_oSongManager.GetError (true));
		return L"";
	}

	return strSongAlbum;

} // end CSongsBestPickerDlg::GetSongAlbum



//************************************
// Method:    GetSongNameByID
// FullName:  CSongsBestPickerDlg::GetSongNameByID
// Access:    public 
// Returns:   CString
// Qualifier:
// Parameter: int nSongID
//************************************
CString CSongsBestPickerDlg::GetSongPathToMp3 (int nSongID)
{
	CString strSongTitle, strPathToMp3, strSongArtist, strSongAlbum;
	BOOL	bSongStillInCompetition = false;
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3, bSongStillInCompetition)) {
		SetError (m_oSongManager.GetError (true));
		return L"";
	}

	return strPathToMp3;

} // end CSongsBestPickerDlg::GetSongNameByID



//************************************
// Method:    GetWonLossRecord
// FullName:  CSongsBestPickerDlg::GetWonLossRecord
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSongID
// Parameter: int & rnWon
// Parameter: int & rnLost
//
//************************************
bool CSongsBestPickerDlg::GetWonLossRecord (int nSongID, int& rnWon, int& rnLost)
{
	return m_oSongManager.GetWonLossRecord (nSongID, rnWon, rnLost);

} // end CSongsBestPickerDlg::GetWonLossRecord



//************************************
// Method:    GetSongRating
// FullName:  CSongsBestPickerDlg::GetSongRating
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSongID
// Parameter: int & rnRating
//
//************************************
bool CSongsBestPickerDlg::GetSongRating (int nSongID, int& rnRating)
{
	return m_oSongManager.GetSongRating (nSongID, rnRating);

} // end CSongsBestPickerDlg::GetSongRating



//************************************
// Method:    GetSongStrengthOfSchedule
// FullName:  CSongsBestPickerDlg::GetSongStrengthOfSchedule
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSongID
// Parameter: int & rnStrengthOfSchedule
//
//
//
//************************************
bool CSongsBestPickerDlg::GetSongStrengthOfSchedule (int nSongID, int& rnStrengthOfSchedule)
{
	return m_oSongManager.GetSongStrengthOfSchedule (nSongID, rnStrengthOfSchedule);

} // end CSongsBestPickerDlg::GetSongStrengthOfSchedule



//************************************
// Method:    OnDeleteSongFromList
// FullName:  CSongsBestPickerDlg::OnDeleteSongFromList
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnDeleteSongFromList ()
{
	int nSelectedSong = m_oSongList.GetFirstSelectedItem ();
	if (-1 == nSelectedSong)
		return;

	CString strSong = m_oSongList.GetItemText (nSelectedSong, LIST_SONG_COL_TITLE);
	if (IDYES != AfxMessageBox (L"Do you really want to delete the song: " + strSong + L"\r\n\r\nThis will only remove the song from this software, not delete it from your disk", MB_YESNO | MB_DEFBUTTON2))
		return;

	int nSongID = (int) m_oSongList.GetItemData (nSelectedSong);
	if (! m_oSongManager.DeleteSong (nSongID))
	{
		SetError (L"Error deleting song: " + m_oSongManager.GetError (true), true);
		return;
	}

	//
	//  Let's remove that...

	int nListCtrlIndex = m_oSongList.FindByItemData (nSongID);
	if (-1 == nListCtrlIndex)
		return;
	m_oSongList.DeleteItem (nListCtrlIndex);
	UpdateSongCount ();

} // end CSongsBestPickerDlg::OnDeleteSongFromList



//************************************
// Method:    OnEditSongInfo
// FullName:  CSongsBestPickerDlg::OnEditSongInfo
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnEditSongInfo ()
{
	int nCurListCtrlIndex = m_oSongList.GetFirstSelectedItem ();
	if (-1 == nCurListCtrlIndex)
		return;

	int nSongID = (int) m_oSongList.GetItemData (nCurListCtrlIndex);
	LoadSongIntoPlayer (nSongID);
	MakePlayerInfoEditable (true);	// Have to call this after load song into player

} // end CSongsBestPickerDlg::OnEditSongInfo



//************************************
// Method:    OnPlaySongFromSongList
// FullName:  CSongsBestPickerDlg::OnPlaySongFromSongList
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnPlaySongFromSongList ()
{
	int nSelectedSong = m_oSongList.GetFirstSelectedItem ();
	if (-1 == nSelectedSong)
		return;

	int nSongID = (int) m_oSongList.GetItemData (nSelectedSong);
	LoadSongIntoPlayer (nSongID);
	PlaySong ();

} // end CSongsBestPickerDlg::OnPlaySongFromSongList



//************************************
// Method:    RestoreWindowPosition
// FullName:  CSongsBestPickerDlg::RestoreWindowPosition
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::RestoreWindowPosition ()
{
	int nHeight = 0, nWidth = 0, nX = 0, nY = 0;

	CRect	rcWnd;
	CString strVal;

	m_oSongManager.GetOtherValue (L"x",			strVal);
	if (strVal.IsEmpty ())
		return;

	rcWnd.left = _tstoi (strVal);

	m_oSongManager.GetOtherValue (L"y",			strVal);
	rcWnd.top = _tstoi (strVal);

	m_oSongManager.GetOtherValue (L"height",	strVal);
	rcWnd.bottom = rcWnd.top + _tstoi (strVal);

	m_oSongManager.GetOtherValue (L"width",		strVal);
	rcWnd.right = rcWnd.left + _tstoi (strVal);

	CUtils::EnsureWindowIsVisible (rcWnd);
	MoveWindow (rcWnd);

	//
	//  Some other settings we save...

	if (m_oSongManager.GetOtherValue (L"volume",	strVal))
	{
		int nVolume = 100;
		if (CUtils::MyAtoI (strVal, nVolume))
		{
			m_oVolumeSlider.SetPos (REAL_TO_SLIDER (nVolume));
			if (NULL != m_pPlaybackChannel)
			{
				float fVol = (float) nVolume / 100;
				m_pPlaybackChannel->setVolume (fVol);
			}
		}
	}

} // end CSongsBestPickerDlg::RestoreWindowPosition



//************************************
// Method:    SaveWindowPosition
// FullName:  CSongsBestPickerDlg::SaveWindowPosition
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::SaveWindowPosition ()
{
	CRect r;
	this->GetWindowRect (r);

	m_oSongManager.SetOtherValue (L"height",	CUtils::N2S (r.Height ()));
	m_oSongManager.SetOtherValue (L"width",		CUtils::N2S (r.Width  ()));
	m_oSongManager.SetOtherValue (L"x",			CUtils::N2S (r.left));
	m_oSongManager.SetOtherValue (L"y",			CUtils::N2S (r.top));

	m_oSongManager.SetOtherValue (L"TitleWidth",		CUtils::N2S (m_oSongList.GetColumnWidth (LIST_SONG_COL_TITLE)));
	m_oSongManager.SetOtherValue (L"ArtistWidth",		CUtils::N2S (m_oSongList.GetColumnWidth (LIST_SONG_COL_ARTIST)));
	m_oSongManager.SetOtherValue (L"AlbumWidth",		CUtils::N2S (m_oSongList.GetColumnWidth (LIST_SONG_COL_ALBUM)));
	m_oSongManager.SetOtherValue (L"RecordWidth",		CUtils::N2S (m_oSongList.GetColumnWidth (LIST_SONG_COL_WONLOSS)));
	m_oSongManager.SetOtherValue (L"RatingWidth",		CUtils::N2S (m_oSongList.GetColumnWidth (LIST_SONG_COL_RATING)));
	m_oSongManager.SetOtherValue (L"PathToMp3Width",	CUtils::N2S (m_oSongList.GetColumnWidth (LIST_SONG_COL_MP3)));

	//
	//  Might as well save volume

	CString strVolume = CUtils::N2S (SLIDER_TO_REAL (m_oVolumeSlider.GetPos ()));
	m_oSongManager.SetOtherValue (L"Volume", strVolume);

	//
	//  Let's save the column positions from the song list

	SaveSongListColumns ();

} // end CSongsBestPickerDlg::SaveWindowPosition





//************************************
// Method:    RestoreSongListColumns
// FullName:  CSongsBestPickerDlg::RestoreSongListColumns
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::RestoreSongListColumns ()
{
	//
	//  Clear 'em out to start...

	while (m_oSongList.DeleteColumn (0));

	//
	//  ... then build 'em back up

	int nActiveColCount = 0;
	m_oSongManager.GetColumnCount (nActiveColCount);

	m_arrActiveColumns.SetSize (0);
	for (int i = 0; i < nActiveColCount; i++)
	{
		int nColType = -1, nColFormat = LVCFMT_LEFT, nColWidth = 10;
		CString strColName;

		m_oSongManager.GetColumnSetupInfo (i, nColType, strColName, nColFormat, nColWidth);
		m_oSongList.InsertColumn (i, strColName, nColFormat, nColWidth);
		m_arrActiveColumns.Add (nColType);	//  One of the few things we're keeping in memory, as opposed to querying DB every time
	} // end loop to create columns
} // end CSongsBestPickerDlg::RestoreSongListColumns



//************************************
// Method:    SaveSongListColumns
// FullName:  CSongsBestPickerDlg::SaveSongListColumns
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::SaveSongListColumns ()
{
	CRect rcList;
	m_oSongList.GetClientRect (&rcList);

	m_oSongManager.DeleteAllColumns ();

	for (int i = 0; i  < m_arrActiveColumns.GetSize (); i ++)
	{
		int nColType	= m_arrActiveColumns[i];
		int nColWidth	= 0;
		if (i < m_oSongList.GetColumnCount ())
			nColWidth = m_oSongList.GetColumnWidth (i);
		else
			nColWidth = (int) (rcList.Width () * garrSongColsAval[nColType].fWidthAsPercent);

		if (nColWidth < 5)
			nColWidth = 5;

		m_oSongManager.SetColumnSetupInfo (i, nColType, garrSongColsAval[nColType].strColumnName, garrSongColsAval[nColType].nFormat, nColWidth);
	}
} // end CSongsBestPickerDlg::SaveSongListColumns



//************************************
// Method:    OnBnClickedSubmitPodRankings
// FullName:  CSongsBestPickerDlg::OnBnClickedSubmitPodRankings
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnBnClickedSubmitPodRankings()
{
	//
	//  Take the order of songs in our pod and save them as rankings

	CIntArray arrSongIDs;
	for (int i = 0; i < m_oCurrentPodList.GetItemCount (); i ++)
	{
		arrSongIDs.Add ((int) m_oCurrentPodList.GetItemData (i));
	}

	auto ePrevSongStatus = m_eSongPlayingStatus;
	m_oSongManager.SetPodRankings (m_nCurPodID, arrSongIDs);
	UpdateCurrentPod (); //  This will move us to our next pod

	//
	//  If it was playing before, make it play now

	if (ePrevSongStatus == ESongPlayStatus::ePlaying)
		PlaySong ();

	//
	//  Update our song list with the new won/loss records.

	for (int i = 0; i < arrSongIDs.GetSize (); i ++)
	{
		UpdateSongListWonLossSpecificSong (arrSongIDs[i]);
	}

	UpdateAccessoryListCtrl ();

} // end submit pod rankings



//************************************
// Method:    OnBnClickedBrowseForSong
// FullName:  CSongsBestPickerDlg::OnBnClickedBrowseForSong
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnBnClickedBrowseForSong()
{
	UpdateData ();

	CFileDialog oDlg (true, L"*.*", m_strCurSongPathToMp3);
	if (IDOK != oDlg.DoModal ())
		return;

	m_strCurSongPathToMp3 = oDlg.GetPathName ();
	UpdateData (false);

} // end browse for song




//************************************
// Method:    OnRClickSongList
// FullName:  CSongsBestPickerDlg::OnRClickSongList
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: NMHDR * pNMHDR
// Parameter: LRESULT * pResult
//************************************
void CSongsBestPickerDlg::OnRClickSongList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CPoint ptScreen = pNMItemActivate->ptAction;
	this->ClientToScreen (&ptScreen);

	CMenu	menuPopup;
	
	VERIFY(menuPopup.LoadMenu(IDR_MENU_SONG_POPUP));
	CMenu*	pMenuSongList = menuPopup.GetSubMenu(0);
	if (NULL == pMenuSongList)
		return;

	pMenuSongList->TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScreen.x, ptScreen.y, this); // | TPM_RETURNCMD
	*pResult = 1;
} // end on rclick song list



//////////////////////////////////////////////////////////////////////
//
//  W I N D O W   P R O C
//
//  Hey, look at me!  This is the first time I've ever used the window
//  proc function.  I hope it works.  I'm doing it because I want to
//  grab the OnContextMenu for the list controls.
//
LRESULT CSongsBestPickerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_CONTEXTMENU)
	{
		//
		//  If the window is one of our list controls, grab it...

		HWND hWnd = (HWND) wParam;
		if (hWnd == m_oSongList.GetSafeHwnd ())
		{
			ShowSongListColumnChooser ();
			return true;
		}
		else if (hWnd == m_oCurrentPodList.GetSafeHwnd ())
		{
			return true;
		}
		else if (hWnd == m_oSongGameResultList.GetSafeHwnd ())
		{
			return true;
		}

#ifdef Whatever
		for (int i = 0; i < m_arrListCtrls.GetSize (); i ++)
		{
			if (hWnd == m_arrListCtrls[i]->m_hWnd)
			{
				int xPos = GET_X_LPARAM(lParam); 
				int yPos = GET_Y_LPARAM(lParam); 
				PopupMenuListCtrl (i, xPos, yPos);
				break;
			}
		}
#endif

	}
	
	return CDialog::WindowProc(message, wParam, lParam);
} // end window proc






//************************************
// Method:    OnClickedProgressCtrl
// FullName:  CSongsBestPickerDlg::OnClickedProgressCtrl
// Access:    public 
// Returns:   LRESULT
// Qualifier:
// Parameter: WPARAM wParam
// Parameter: LPARAM lParam
//
//
//
//************************************
LRESULT CSongsBestPickerDlg::OnClickedProgressCtrl (WPARAM wParam, LPARAM lParam)
{
	if ((-1 == m_nCurSongID) || (NULL == m_pPlaybackChannel) || (NULL == m_pCurSong))
		return true;

	//
	//  Move the currently playing song to nPercent

	int		nPercent	= (int) wParam;
	UINT	nSongLenMS	= 0;

	auto result = m_pCurSong->getLength (&nSongLenMS, FMOD_TIMEUNIT_MS);
	if (result != FMOD_OK) 
		return true;

	UINT	nSetPosTo = (UINT) (((float)nPercent / 100) * nSongLenMS);
	m_pPlaybackChannel->setPosition (nSetPosTo, FMOD_TIMEUNIT_MS);

	return true;

} // end CSongsBestPickerDlg::OnClickedProgressCtrl






///////////////////////////////////////////////////////////////////////
//
//   O N   C O M M A N D
//
//   Catch our script commands...  we don't know which ones they'll be
//   at compile time
//
BOOL CSongsBestPickerDlg::OnCommand (WPARAM wParam, LPARAM lParam)
{
	int nID = LOWORD (wParam);

	//
	//  First handle script commands

	if ((nID >= IDC_COLUMN_BLOCK_START) && (nID <= IDC_COLUMN_BLOCK_END))
	{
		//
		//  Show (or hide) this command

		int nColumnType = garrSongColsAval[nID - IDC_COLUMN_BLOCK_START].nType;
		int nCurIndex = CUtils::FindNumberInArray (m_arrActiveColumns, nColumnType);
		if (-1 != nCurIndex)
			m_arrActiveColumns.RemoveAt (nCurIndex);	//  Hide it...
		else
			m_arrActiveColumns.Add (nColumnType);		//  Add as a new column....

		SaveSongListColumns ();
		UpdateSongList (true);	// true to re-do columns

		return true;
	} // end if command to show or hide a column type

	return __super::OnCommand (wParam, lParam);
} // end on command




///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ON DROP FILES
//
//
void CSongsBestPickerDlg::OnDropFiles (HDROP hDropInfo)
{
	DWORD	nBuffer			= 0;
	UINT	nFilesDropped	= DragQueryFile (hDropInfo, 0xFFFFFFFF, NULL, 0);

	for (UINT i = 0; i < nFilesDropped; i ++)
	{
		// Get the buffer size for the first filename
		nBuffer = DragQueryFile (hDropInfo, i, NULL, 0);

		// Get path and name of the file

		CString    strPathToMp3;
		DragQueryFile (hDropInfo, i, strPathToMp3.GetBuffer (nBuffer + 1), nBuffer + 1);
		strPathToMp3.ReleaseBuffer();

		//
		//  Let's see if we can get some tags on this file

		m_oSongManager.GetError (true); // true to clear - which is why we're doing this
		CString strTitle, strArtist, strAlbum;
		if (! m_oSongManager.LoadTagsFromMp3 (strPathToMp3, strTitle, strArtist, strAlbum)) {
			SetError (m_oSongManager.GetError (true));
			continue;
		}

		if (strTitle.IsEmpty ())
			strTitle = CUtils::GetFileNameFromPath (strPathToMp3);

		//
		//  Add to our database.  null for the ID makes it auto-choose, though not technically "autoincrement"

		int nNewSongID = -1;
		if (m_oSongManager.AddSong (nNewSongID, strTitle, strPathToMp3, strArtist, strAlbum))
			UpdateSongListSpecificSong (nNewSongID, strTitle, strArtist, strAlbum, strPathToMp3);
		else
			SetError (m_oSongManager.GetError (true));

	} // end loop through files dropped on us

//	CString strError = m_oSongManager.GetError (true);
//	if (! strError.IsEmpty ())
//		AfxMessageBox (strError);

	DragFinish(hDropInfo);
}







///////////////////////////////////////////////////////////////////////////
//
//  When the user types in a type to filter, we don't immediately update
//  the filtered display.  What we want to do is set a timer and update
//  after that runs out.  That way if they type multiple characters quickly,
//  we won't lag after the first one (which will tend to match lots of items).
//
//
void CSongsBestPickerDlg::OnChangeTypeToFilterLeft()
{
	//
	//  Push the existing timer back.  We want to wait until a lag
	//  in their typing to update things

	KillTimer (m_nTypeToFilterPodTimerId);
	m_nTypeToFilterPodTimerId = SetTimer (VPCC_TYPE_TO_FILTER_TIMER_ID, VPCC_TYPE_TO_FILTER_TIMER_ID_TIMER_LEN_MS, NULL);

} // end change in type to filter edit left





//************************************
// Method:    OnSetFocusTypeToFilterLeft
// FullName:  CSongsBestPickerDlg::OnSetFocusTypeToFilterLeft
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnSetFocusTypeToFilter ()
{
//	CString s;
//	m_oTypeToFilterPod.GetWindowText (s);
//	s.Trim ();

	if (m_bTypeToFilterInEmptyMode)
		SetTypeToFilterState (false);
} // end OnSetFocusTypeToFilter 


//************************************
// Method:    OnCopySongMp3
// FullName:  CSongsBestPickerDlg::OnCopySongMp3
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnCopySongMp3 ()
{
	int nCurListCtrlIndex = m_oSongList.GetFirstSelectedItem ();
	if (-1 == nCurListCtrlIndex)
		return;

	int nSongID = (int) m_oSongList.GetItemData (nCurListCtrlIndex);
	if (-1 == nSongID)
		return;

	CString strTitle, strArtist, strAlbum, strPathToMp3;
	BOOL	bStillinCompetition;

	m_oSongManager.GetSongDetails (nSongID, strTitle, strArtist, strAlbum, strPathToMp3, bStillinCompetition);

	CUtils::CopyTextToClipboard (strPathToMp3);

} // end CSongsBestPickerDlg::OnCopySongMp3



//************************************
// Method:    OnKillFocusTypeToFilter
// FullName:  CSongsBestPickerDlg::OnKillFocusTypeToFilter
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnKillFocusTypeToFilter ()
{
	CString s;
	m_oTypeToFilterPod.GetWindowText (s);
	s.Trim ();

	if (s.IsEmpty ())
		SetTypeToFilterState (true);
} // end OnKillFocusTypeToFilter





//************************************
// Method:    SetTypeToFilterErrorMode
// FullName:  CCleanupConfirmNewDlg::SetTypeToFilterErrorMode
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: bool bError
// Parameter: HWND hListCtrlWnd
//************************************
void CSongsBestPickerDlg::SetTypeToFilterErrorMode (bool bError)
{
	if (bError)
		m_oTypeToFilterPod.SetIcon  (IDI_EXCLAMATION_PT, true);
	else
		m_oTypeToFilterPod.SetIcon  (IDI_ICON_X, true);
}


/////////////////////////////////////////////////////////////////////////
//
// Set Type To Filter Font
//
//  Before they type anything, we want a dim gray text.  Once they
//  type we want nice bold black
//
//
void CSongsBestPickerDlg::SetTypeToFilterState (bool bIsTypeToFilterEmpty)
{
	if (bIsTypeToFilterEmpty)
	{
		m_oTypeToFilterPod.SetWindowText (m_strTypeToFilterEmptyMsg);
		m_bTypeToFilterInEmptyMode	= true;
	}
	else
	{
		m_oTypeToFilterPod.SetWindowText (L"");
		m_bTypeToFilterInEmptyMode	= false;
	}
} // end SetTypeToFilterFont


//************************************
// Method:    OnSetFocusTypeToFilterHotkey
// FullName:  CSongsBestPickerDlg::OnSetFocusTypeToFilterHotkey
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnSetFocusTypeToFilterHotkey ()
{
	m_oTypeToFilterPod.SetFocus ();
}


//////////////////////////////////////////////////////////////////////
//
//  UpdateTypeToFilterDisplay
//
//  What we do is filter the available pods in the dropdown combo
//
void CSongsBestPickerDlg::UpdateTypeToFilterDisplay (CString strFilter)
{
	static int nTypeToFilterRecursionCounter = 0;
	CRecursionCounterAuto RecursionCounter (&nTypeToFilterRecursionCounter);
	if (RecursionCounter.GetRecursionDepth () > 1)
		return;

	try
	{
		m_strTypeToFilterLastText = strFilter;

		if (strFilter.IsEmpty ())
		{
			//
			//  Not filtering.  Populate the combo

			m_oSongManager.GetAllPodIDs (m_arrPodComboIDs);
			UpdatePodCombo ();
			return;
		} // end if nothing to filter on

		//
		//  We are filtering.  Figure out which pods belong.  We apply our strFilter to 
		//  all fields in all songs in each pod

		CIntArray arrMatchingPods;
		if (! m_oSongManager.GetAllPodsMatchingFilter (arrMatchingPods, m_bTypeToFilterUseRegex, strFilter)) {
			SetError (m_oSongManager.GetError (true));
			return;
		}

		//
		//  Do we need to update?

		if (CUtils::AreArraysEqual (arrMatchingPods, m_arrPodComboIDs))
			return;

		//
		//   Update our combo.  

		m_arrPodComboIDs.SetSize (arrMatchingPods.GetSize ());
		for (int i = 0; i < arrMatchingPods.GetSize (); i ++)
			m_arrPodComboIDs[i] = arrMatchingPods[i];

		UpdatePodCombo ();
	}
	catch (std::exception& e)
	{
		SetError (CUtils::UTF8toUTF16 (e.what ()));
		return;
	}
	catch (CException* e)
	{
		SetError (CUtils::GetErrorMessageFromException (e, true));
		return;
	}
} // end update type to filter display 




//************************************
// Method:    OnSelChangeComboPod
// FullName:  CSongsBestPickerDlg::OnSelChangeComboPod
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnSelChangeComboPod ()
{
	int nCurIndex = m_oPodCombo.GetCurSel ();
	if (-1 == nCurIndex)
		return;

	int nSelPodID = (int) m_oPodCombo.GetItemData (nCurIndex);
	if (nSelPodID == m_nCurPodID)
		return;

	//
	//  We have to load the new pod in...

	UpdateCurrentPod (nSelPodID);

}




void CSongsBestPickerDlg::OnBnClickedButtonPrevPod()
{
	// TODO: Add your control notification handler code here
}


void CSongsBestPickerDlg::OnBnClickedButtonNextPod()
{
	// TODO: Add your control notification handler code here
}



//************************************
// Method:    OnBnClickedDefault
// FullName:  CSongsBestPickerDlg::OnBnClickedDefault
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CSongsBestPickerDlg::OnBnClickedDefault ()
{
	//
	//  Do nothing

} // end CSongsBestPickerDlg::OnBnClickedDefault



void CSongsBestPickerDlg::OnVScroll (UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->GetSafeHwnd () == m_oVolumeSlider.GetSafeHwnd ())
	{
		//
		//  Well, our numbers seem to go up as the slider goes down.  I haven't found
		//  the style / setting to invert that.  I'm tired of looking.  I'll do it manually.

		int nSBPos = SLIDER_TO_REAL (m_oVolumeSlider.GetPos ());	//  Invert our values
		float fVol = (float) nSBPos / 100;
		m_pPlaybackChannel->setVolume (fVol);
	}
}
