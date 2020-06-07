
// DougHotkeysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SongsBestPicker.h"
#include "SongsBestPickerDlg.h"
#include "afxdialogex.h"

#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define LIST_SONG_COL_TITLE		0
#define LIST_SONG_COL_ARTIST	1
#define LIST_SONG_COL_ALBUM		2
#define LIST_SONG_COL_WONLOSS	3
#define LIST_SONG_COL_RATING	4
#define LIST_SONG_COL_MP3		5

#define LIST_POD_COL_RANK		0
#define LIST_POD_COL_TITLE		1
#define LIST_POD_COL_ARTIST		2

#define LIST_STATS_COL_OPPONENT	0
#define LIST_STATS_COL_WONLOSS	1
#define LIST_STATS_COL_MARGIN	2


#define SONG_STATUS_TIMER_ID	3
#define SONG_STATUS_TIMER_MS	1000

//
//  Resize amounts

#define MY_BIG_AMOUNT				100
#define MY_SMALL_AMOUNT				 1


//
//  Command ids

#define ID_SHOW_OUR_APP					WM_USER + 9

#define ID_PLAY_OR_PAUSE_SONG			WM_USER + 1
#define ID_NEXT_SONG					WM_USER + 2
#define ID_PREV_SONG					WM_USER + 3

#define ID_RANK_1						WM_USER + 4
#define ID_RANK_2						WM_USER + 5
#define ID_RANK_3						WM_USER + 6
#define ID_RANK_4						WM_USER + 7
#define ID_RANK_5						WM_USER + 8


#define ID_MY_NOTIFY					WM_APP + 0

//
//  Put them in a nice clean struct

struct HotkeyCommandDefinitionStruct
{
	int          nCommandID;
	const TCHAR *strCommandName;
	const TCHAR *strCommandDescription;

	UINT         nDefaultModifiers;
	UINT         nDefaultKey;
};

//
//  Here are our commands...

static HotkeyCommandDefinitionStruct HKInit[] = {

	{ID_SHOW_OUR_APP,			L"Bring this glorious app to the front", L"", MOD_CONTROL | MOD_SHIFT | MOD_ALT, 'A'},

	{ID_PLAY_OR_PAUSE_SONG,		L"Play or Pause Song",	L"",	MOD_CONTROL | MOD_SHIFT, 'A'},
	{ID_NEXT_SONG,				L"Next Song",			L"",	MOD_CONTROL | MOD_SHIFT, VK_NEXT},
	{ID_PREV_SONG,				L"Prev Song",			L"",	MOD_CONTROL | MOD_SHIFT, VK_PRIOR},

	{ID_RANK_1,					L"Rank 1",				L"",	MOD_CONTROL | MOD_SHIFT, '1'},
	{ID_RANK_2,					L"Rank 2",				L"",	MOD_CONTROL | MOD_SHIFT, '2'},
	{ID_RANK_3,					L"Rank 3",				L"",	MOD_CONTROL | MOD_SHIFT, '3'},
	{ID_RANK_4,					L"Rank 4",				L"",	MOD_CONTROL | MOD_SHIFT, '4'},
	{ID_RANK_5,					L"Rank 5",				L"",	MOD_CONTROL | MOD_SHIFT, '5'},

};

#define HOTKEY_INIT_NUM_COMMANDS (sizeof (HKInit) / sizeof (HotkeyCommandDefinitionStruct))



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
	, m_strSongPlaybackPos(_T(""))
	, m_strSongPlaybackLen(_T(""))
	, m_strCurSongArtist(_T(""))
	, m_strCurSongAlbum(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SONGS_BEST_PICKER); // IDR_MAINFRAME);
}

void CSongsBestPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SONG_LIST, m_oSongList);
	DDX_Control(pDX, IDC_STATS, m_oStatsList);
	DDX_Control(pDX, IDC_CURRENT_POD_LIST, m_oCurrentPodList);
	DDX_Text(pDX, IDC_EDIT1, m_strCurSongTitle);
	DDX_Text(pDX, IDC_EDIT2, m_strCurSongPathToMp3);
	DDX_Text(pDX, IDC_SONG_POS, m_strSongPlaybackPos);
	DDX_Text(pDX, IDC_SONG_LENGTH, m_strSongPlaybackLen);
	DDX_Control(pDX, IDC_PROGRESS1, m_oSongPlayingProgress);
	DDX_Text(pDX, IDC_EDIT_ARTIST, m_strCurSongArtist);
	DDX_Text(pDX, IDC_EDIT_ALBUM, m_strCurSongAlbum);
}

BEGIN_MESSAGE_MAP(CSongsBestPickerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_HOTKEY()
	ON_WM_KEYUP()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDOK,				&CSongsBestPickerDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL,			&CSongsBestPickerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(ID_APPLY_HOTKEYS, &CSongsBestPickerDlg::OnBnClickedApplyHotkeys)
	ON_COMMAND (ID_FILE_EXIT,		&CSongsBestPickerDlg::OnBnClickedOk)

	ON_MESSAGE (ID_MY_NOTIFY,			OnTrayNotification)

	ON_REGISTERED_MESSAGE (UWM_CLICKED_PROGRESS_CTRL,	OnClickedProgressCtrl)

	ON_COMMAND(ID_DOUG_SHOWWINDOW,		OnShowMyWindow)
	ON_COMMAND(ID_IMPORTFROMM3UFILE,	OnImportFromM3UFile)
	ON_COMMAND(ID_RECALC_SONG_RATINGS,	OnRecalcSongRatings)
	ON_COMMAND(ID_RESETSONGSTATISTICS,	OnResetSongStatistics)
	ON_COMMAND(ID_DELETESONGLIST,		OnDeleteSongList)

	ON_COMMAND(ID_POPUP_DELETESONG,		OnDeleteSongFromList)
	ON_COMMAND(ID_POPUP_EDITSONGINFO,	OnEditSongInfo)
	ON_COMMAND(ID_PLAY_SONG,			OnPlaySongFromSongList)

	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SONG_LIST,			&CSongsBestPickerDlg::OnItemChangedSongList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CURRENT_POD_LIST,	&CSongsBestPickerDlg::OnItemChangedCurrentPodList)

	ON_BN_CLICKED(IDC_PLAY_SONG,	&CSongsBestPickerDlg::OnBnClickedPlaySong)
//	ON_BN_CLICKED(IDC_PAUSE_SONG,	&CSongsBestPickerDlg::PauseSong)
//	ON_BN_CLICKED(IDC_STOP_SONG,	&CSongsBestPickerDlg::StopSong)

	ON_NOTIFY(HDN_ITEMDBLCLICK, 0, &CSongsBestPickerDlg::OnSongHeaderDblClick)
	ON_BN_CLICKED(IDC_SUBMIT_POD_RANKINGS, &CSongsBestPickerDlg::OnBnClickedSubmitPodRankings)
	ON_BN_CLICKED(IDC_BROWSE_FOR_SONG, &CSongsBestPickerDlg::OnBnClickedBrowseForSong)
	ON_NOTIFY(NM_RCLICK, IDC_SONG_LIST, &CSongsBestPickerDlg::OnRClickSongList)
	ON_BN_CLICKED(IDC_SAVE_SONG_CHANGES, &CSongsBestPickerDlg::SaveSongInfoFromPlayer)
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
	//  Setup our list control

	m_oListCtrlFont.CreatePointFont (90, L"Courier New");
	m_oSongList.SetFont (&m_oListCtrlFont);

	CRect rcList;
	m_oSongList.GetClientRect (rcList);

	m_oSongList.InsertColumn (LIST_SONG_COL_TITLE,		L"Title",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.24));
	m_oSongList.InsertColumn (LIST_SONG_COL_ARTIST,		L"Artist",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.24));
	m_oSongList.InsertColumn (LIST_SONG_COL_ALBUM,		L"Album",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.24));
	m_oSongList.InsertColumn (LIST_SONG_COL_WONLOSS,	L"Record",	LVCFMT_CENTER,	(int) (rcList.Width () * 0.09));
	m_oSongList.InsertColumn (LIST_SONG_COL_RATING,		L"Rating",	LVCFMT_CENTER,	(int) (rcList.Width () * 0.08));
	m_oSongList.InsertColumn (LIST_SONG_COL_MP3,		L"MP3",		LVCFMT_LEFT,	(int) (rcList.Width () * 0.10));

	m_oSongList.SetExtendedStyle (m_oSongList.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//  And our current pool / pod / competition / whatever we call it

	m_oCurrentPodList.GetClientRect (rcList);
	m_oCurrentPodList.InsertColumn (LIST_POD_COL_RANK,		L"Rank",	LVCFMT_CENTER,	(int) (rcList.Width () * 0.12));
	m_oCurrentPodList.InsertColumn (LIST_POD_COL_TITLE,		L"Song",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.44));
	m_oCurrentPodList.InsertColumn (LIST_POD_COL_ARTIST,	L"Artist",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.43));
	m_oCurrentPodList.SetExtendedStyle (m_oSongList.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//  Some song stats

	m_oStatsList.GetClientRect (rcList);
	m_oStatsList.InsertColumn (LIST_STATS_COL_OPPONENT,		L"Opponent",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.5));
	m_oStatsList.InsertColumn (LIST_STATS_COL_WONLOSS,		L"WonLoss",		LVCFMT_CENTER,	(int) (rcList.Width () * 0.3));
	m_oStatsList.InsertColumn (LIST_STATS_COL_MARGIN,		L"Margin",		LVCFMT_CENTER,	(int) (rcList.Width () * 0.18));

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
		AfxMessageBox (L"Error: " + m_oSongManager.GetError ());
	else
		UpdateSongList ();

	//
	//  Let's allow ourselves to minimize to system tray

	m_oTrayIcon.Create (this, ID_MY_NOTIFY, L"Songs Best Picker!", m_hIcon, IDR_MENU_DOUGS_HOTKEYS);

	//
	//  And apply the hotkeys

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
	UpdateCurrentPod ();

	if (m_eSongPlayingStatus == ESongPlayStatus::ePlaying)
		PlaySong ();


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
	//
	//  Keeping Left edge in place

	for (int i = 0; i < HOTKEY_INIT_NUM_COMMANDS; i ++)
	{
		AddHotkey (HKInit[i].nCommandID, HKInit[i].nDefaultModifiers, HKInit[i].nDefaultKey, HKInit[i].strCommandName);
	}

	m_bHotkeysApplied = true;

#ifdef ShowHotkeysInList
	//
	//  Now update user visible stuff, if necessary

	CWnd* pWnd = GetDlgItem (ID_APPLY_HOTKEYS);
	if (NULL != pWnd)
		pWnd->SetWindowText (_T("Remove Hotkeys"));

	if (NULL != m_oSongList.GetSafeHwnd ())
	{
		m_oSongList.DeleteAllItems ();

		for (auto it  = m_mapHotkeys.begin (); it != m_mapHotkeys.end (); it ++)
		{
			CString strID = GetHotkeyCommandName ((*it).first);
			CString strKey	= (*it).second;

			int nIndex = m_oSongList.InsertItem (0, strID);
			m_oSongList.SetItemText (nIndex, 1, strKey);
		}
	}
#endif
} // end ApplyHotkeys





bool CSongsBestPickerDlg::AddHotkey (int nID, UINT nModifiers, UINT nVirtualKey, CString strNameForError)
{
	CString strFailed = _T("");
	if (! RegisterHotKey (m_hWnd, nID,		nModifiers, nVirtualKey))
	{
//		AfxMessageBox (_T("Failed to register ") + strNameForError);
		strFailed = _T("*FAILED* ");
	}

	CString strKeyName = strFailed;
	if (nModifiers & MOD_CONTROL)
		strKeyName += _T("Ctrl + ");
	else
		strKeyName += _T("       ");

	if (nModifiers & MOD_ALT)
		strKeyName += _T("Alt + ");
	else
		strKeyName += _T("      ");

	if (nModifiers & MOD_SHIFT)
		strKeyName += _T("Shift + ");
	else
		strKeyName += _T("        ");

	if (nModifiers & MOD_WIN)
		strKeyName += _T("Win + ");
	else
		strKeyName += _T("      ");

	strKeyName += GetKeyName (nVirtualKey);

	m_mapHotkeys[nID] = strKeyName;
	return true;
}



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
		AfxMessageBox (m_oSongManager.GetError (true));
	else
		UpdateSongList ();

} // end on import from m3u file



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
	m_oSongManager.RecalcAllSongRatings ();
	UpdateSongList ();

} // end CSongsBestPickerDlg::OnRecalcSongRatings



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
void CSongsBestPickerDlg::UpdateSongList()
{
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
		AfxMessageBox (m_oSongManager.GetError ());
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
			AfxMessageBox (m_oSongManager.GetError ());
			m_oSongList.DeleteAllItems ();
			m_oSongList.SetRedraw (true);
			m_oSongList.UpdateWindow ();
			return;
		}

		int nIndex = m_oSongList.InsertItem (i,					strSongTitle);
		m_oSongList.SetItemData (nIndex, nLastID);

		m_oSongList.SetItemText (nIndex, LIST_SONG_COL_ARTIST,	strSongArtist);
		m_oSongList.SetItemText (nIndex, LIST_SONG_COL_ALBUM,	strSongAlbum);
		m_oSongList.SetItemText (nIndex, LIST_SONG_COL_MP3,		strPathToMp3);

		if (! m_oSongManager.GetWonLossRecord (nLastID, nWins, nLosses)) {
			AfxMessageBox (m_oSongManager.GetError ());
			m_oSongList.SetRedraw (true);
			m_oSongList.UpdateWindow ();
			return;
		}

		strWonLoss.Format (L"%2d / %2d", nWins, nLosses);
		m_oSongList.SetItemText (nIndex, LIST_SONG_COL_WONLOSS, strWonLoss);

		int nRating = -1;
		if (m_oSongManager.GetSongRating (nLastID, nRating))
			m_oSongList.SetItemText (nIndex, LIST_SONG_COL_RATING, CUtils::NumberToString (nRating));

	} // end loop through songs

	CString strTitle;
	strTitle.Format (L"Title (%d songs)", nSongCount);

	LV_COLUMN col;
	col.mask = LVCF_TEXT;
	col.pszText = strTitle.GetBuffer ();
	m_oSongList.SetColumn (LIST_SONG_COL_TITLE, &col);

	m_oSongList.SetRedraw (true);
	m_oSongList.UpdateWindow ();

} // end CSongsBestPickerDlg::UpdateSongList




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
		return;

	if (strTitle.IsEmpty ())
	{
		m_oSongManager.GetSongDetails (nSongID, strTitle, strArtist, strAlbum, strPathToMp3);
	}

	m_oSongList.SetItemText (nListCtrlIndex, LIST_SONG_COL_TITLE,	strTitle);
	m_oSongList.SetItemText (nListCtrlIndex, LIST_SONG_COL_ARTIST,	strArtist);
	m_oSongList.SetItemText (nListCtrlIndex, LIST_SONG_COL_ALBUM,	strAlbum);
	m_oSongList.SetItemText (nListCtrlIndex, LIST_SONG_COL_MP3,		strPathToMp3);

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

	int nWins = 0, nLosses = 0, nSongRating = 0;

	m_oSongManager.GetWonLossRecord (nSongID, nWins, nLosses);
	m_oSongManager.GetSongRating	(nSongID, nSongRating);

	CString strWonLoss;
	strWonLoss.Format (L"%2d / %2d", nWins, nLosses);

	m_oSongList.SetItemText (nListCtrlIndex, LIST_SONG_COL_WONLOSS,	strWonLoss);
	m_oSongList.SetItemText (nListCtrlIndex, LIST_SONG_COL_RATING,	CUtils::NumberToString (nSongRating));

} // end CSongsBestPickerDlg::UpdateSongListWonLossSpecificSong



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
void CSongsBestPickerDlg::UpdateCurrentPod ()
{
	int nPrevPodID = -1;

	CIntArray arrSongIDs;
	if (! m_oSongManager.GetCurrentPod (m_nCurPodID, arrSongIDs))
		return;

	if (nPrevPodID != m_nCurPodID)
	{
		UnloadSong ();
		m_nCurSongID = -1;
	}

	m_oCurrentPodList.DeleteAllItems ();
	for (int i = 0; i < arrSongIDs.GetSize (); i ++)
	{
		CString strSongTitle, strSongArtist, strSongAlbum, strPathToMp3;
		m_oSongManager.GetSongDetails (arrSongIDs[i], strSongTitle, strSongArtist, strSongAlbum, strPathToMp3);

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
void CSongsBestPickerDlg::UpdateStatsForCurrentSong (int nSongID)
{
	m_oStatsList.DeleteAllItems ();
	if (-1 == nSongID)
		return;
	
	CIntArray arrOpponents, arrMargins;
	if (! m_oSongManager.GetHeadToHeadForSong (nSongID, arrOpponents, arrMargins))
		return;

	if (arrOpponents.GetSize () != arrMargins.GetSize ())
		return; 

	for (int i = 0; i < arrOpponents.GetSize (); i ++)
	{
		CString strOpponent, strWonLoss;
		m_oSongManager.GetSongTitle (arrOpponents[i], strOpponent);

		if (arrMargins[i] > 0)
			strWonLoss = L"Won";
		else
			strWonLoss = L"Lost";

		int nIndex = m_oStatsList.InsertItem (i, strOpponent);
		m_oStatsList.SetItemText (nIndex, LIST_STATS_COL_WONLOSS,	strWonLoss);
		m_oStatsList.SetItemText (nIndex, LIST_STATS_COL_MARGIN,	CUtils::NumberToString (arrMargins[i]));

		m_oStatsList.SetItemData (nIndex, arrOpponents[i]);
	}


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
		return false;

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

	LoadSongIntoPlayer (nNextSongID);
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

	int nNextToSelect	= m_oCurrentPodList.GetNextItem (nListCtrlIndex, LVNI_ABOVE);
	
	int nSongID = -1;
	if (-1 != nNextToSelect) {
		nSongID = (int) m_oCurrentPodList.GetItemData (nNextToSelect);
	}

	m_oCurrentPodList.SelectItem (nNextToSelect, true);

	LoadSongIntoPlayer (nSongID);
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

	if (m_nSongPlayingStatusTimerID == nIDEvent)
	{
		KillTimer (m_nSongPlayingStatusTimerID);
		UpdatePlayerStatus ();
		m_nSongPlayingStatusTimerID = SetTimer (SONG_STATUS_TIMER_ID, SONG_STATUS_TIMER_MS, NULL);

	}

	__super::OnTimer (nIDEvent);
} // end on timer




void CSongsBestPickerDlg::OnShowMyWindow ()
{
	if (IsWindowVisible () && (this == GetForegroundWindow ()))
		m_oTrayIcon.MinimiseToTray (this);
	else
		m_oTrayIcon.MaximiseFromTray (this);
}



CString CSongsBestPickerDlg::GetKeyName (unsigned int virtualKey)
{
	switch (virtualKey)
	{
        case VK_LEFT:	return _T("Left");
		case VK_UP:		return _T("Up");
		case VK_RIGHT:	return _T("Right");
		case VK_DOWN:	return _T("Down");
        case VK_PRIOR:  return _T("PgUp");
		case VK_NEXT:	return _T("PgDown");

		case VK_END:	return _T("End");
		case VK_HOME:	return _T("Home");
        case VK_INSERT:	return _T("Insert");
		case VK_DELETE:	return _T("Delete");
        case VK_DIVIDE: return _T("/");
        case VK_NUMLOCK:return _T("NumLock");
	}

	if (virtualKey >= 0x30 && virtualKey <= 0x5a)
	{
		TCHAR tc = (TCHAR) virtualKey;
		CString s (tc);
		return s;
	}

	return _T("Unknown");

#ifdef TryToDoItRight
	unsigned int scanCode = MapVirtualKey (virtualKey, MAPVK_VK_TO_VSC);

    // because MapVirtualKey strips the extended bit for some keys
    switch (virtualKey)
    {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
        case VK_PRIOR: case VK_NEXT: // page up and page down
        case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE:
        case VK_DIVIDE: // numpad slash
        case VK_NUMLOCK:
        {
            scanCode |= 0x100; // set extended bit
            break;
        }
    }

	CString strKeyName;
    if (GetKeyNameText (scanCode << 16, strKeyName.GetBuffer (), strKeyName.GetLength ()) != 0)
    {
        return strKeyName;
    }
    else
    {
		return GetLastErrorAsString ();
    }
#endif
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
			UpdateStatsForCurrentSong (nSongID);
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
#ifdef DoThis
		//
		//  Woohoo!  We have a newly selected item.

		int nListCtrlIndex = m_oCurrentPodList.GetFirstSelectedItem ();
		if (-1 != nListCtrlIndex)
		{
			int nSongID = (int) m_oCurrentPodList.GetItemData (nListCtrlIndex);
			LoadSongIntoPlayer (nSongID);
		}
#endif
	}

	*pResult = 0;
} // end item changed current pod list




//************************************
// Method:    LoadSongIntoPlayer
// FullName:  CSongsBestPickerDlg::LoadSongIntoPlayer
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSondID
//************************************
void CSongsBestPickerDlg::LoadSongIntoPlayer (int nSongID)
{
	UpdateData ();

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
		m_oSongManager.GetSongDetails (nSongID, m_strCurSongTitle, m_strCurSongArtist, m_strCurSongAlbum, m_strCurSongPathToMp3);
	}

	UpdateData (false);

//	UpdateCurrentPod			(nSongID);	// NOT the list ctrl index
	UpdateStatsForCurrentSong	(nSongID);	// NOT the list ctrl index


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

	m_oSongManager.SetSongDetails	(m_nCurSongID, m_strCurSongTitle, m_strCurSongArtist, m_strCurSongAlbum, m_strCurSongPathToMp3);
	UpdateSongListSpecificSong		(m_nCurSongID, m_strCurSongTitle, m_strCurSongArtist, m_strCurSongAlbum, m_strCurSongPathToMp3);

} // end CSongsBestPickerDlg::SaveSongInfoFromPlayer




//************************************
// Method:    OnBnClickedPlaySong
// FullName:  CSongsBestPickerDlg::OnBnClickedPlaySong
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::OnBnClickedPlaySong ()
{
	//
	//  If we're already playing, this means stop, otherwise means play...

	if (m_eSongPlayingStatus == ESongPlayStatus::ePlaying)
		PauseSong ();
	else 
		PlaySong (m_strCurSongPathToMp3);

} // end on play song






void CSongsBestPickerDlg::OnSongHeaderDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	if (phdr->iItem == m_nSongsSortCol)
		m_bSongsSortAscending = !m_bSongsSortAscending;
	else
	{
		//
		//  Sort win pct or rating descending by default

		if (phdr->iItem == LIST_SONG_COL_WONLOSS ||
			phdr->iItem == LIST_SONG_COL_RATING)
			m_bSongsSortAscending = false;
		else
			m_bSongsSortAscending = true;
	}

	m_nSongsSortCol = phdr->iItem;

	CWaitCursor wc;
	m_oSongList.SortItems (SortCompareSongListCtrl, (DWORD_PTR) this);
	*pResult = 0;
} // end on song header dbl click



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

	if (LIST_SONG_COL_ARTIST == pDlg->m_nSongsSortCol)
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

	}
	else if (LIST_SONG_COL_ALBUM == pDlg->m_nSongsSortCol)
	{
		//
		//  Sorting on name column...  this is either alpha or by classification order

		CString str1 = pDlg->GetSongAlbum (nSongID1);
		CString str2 = pDlg->GetSongAlbum (nSongID2);

		return nOrderMultiplier * str1.CompareNoCase(str2);
	}

	else if (LIST_SONG_COL_WONLOSS == pDlg->m_nSongsSortCol)
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

		return 0;
	}
	else if (LIST_SONG_COL_RATING == pDlg->m_nSongsSortCol)
	{
		int nSong1Rating = 0, nSong2Rating = 0;

		pDlg->GetSongRating (nSongID1, nSong1Rating);
		pDlg->GetSongRating (nSongID2, nSong2Rating);

		if (nSong1Rating < nSong2Rating)
			return nOrderMultiplier * -1;
		if (nSong1Rating > nSong2Rating)
			return nOrderMultiplier * 1;

		return 0;
	}
	else if (LIST_SONG_COL_MP3 == pDlg->m_nSongsSortCol)
	{
		CString str1 = pDlg->GetSongPathToMp3 (nSongID1);
		CString str2 = pDlg->GetSongPathToMp3 (nSongID2);

		return nOrderMultiplier * str1.CompareNoCase(str2);
	}

	//
	//  Title compare is our backup sort for every column.   That's why
	//  it's not part of the if...else structure

//	if (LIST_SONG_COL_TITLE == pDlg->m_nSongsSortCol)
	{
		//
		//  Sorting on name column...  this is either alpha or by classification order

		CString str1 = pDlg->GetSongTitle (nSongID1);
		CString str2 = pDlg->GetSongTitle (nSongID2);

		return nOrderMultiplier * str1.CompareNoCase(str2);
	}


	return 0;

} // end compare items




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
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3))
		return L"";

	return strSongTitle;

} // end CSongsBestPickerDlg::GetSongNameByID



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
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3))
		return L"";

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
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3))
		return L"";

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
	if (! m_oSongManager.GetSongDetails	(nSongID, strSongTitle, strSongArtist, strSongAlbum, strPathToMp3))
		return L"";

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
	m_oSongManager.DeleteSong (nSongID);

	UpdateSongListSpecificSong		(m_nCurSongID);


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

	MoveWindow (rcWnd);

	//
	//  Now the song columns

	m_oSongManager.GetOtherValue (L"TitleWidth",	strVal);
	m_oSongList.SetColumnWidth (LIST_SONG_COL_TITLE, _tstoi (strVal));

	m_oSongManager.GetOtherValue (L"ArtistWidth",	strVal);
	m_oSongList.SetColumnWidth (LIST_SONG_COL_ARTIST, _tstoi (strVal));

	m_oSongManager.GetOtherValue (L"AlbumWidth",	strVal);
	m_oSongList.SetColumnWidth (LIST_SONG_COL_ALBUM, _tstoi (strVal));

	m_oSongManager.GetOtherValue (L"RecordWidth",	strVal);
	m_oSongList.SetColumnWidth (LIST_SONG_COL_WONLOSS, _tstoi (strVal));

	m_oSongManager.GetOtherValue (L"RatingWidth",	strVal);
	m_oSongList.SetColumnWidth (LIST_SONG_COL_RATING, _tstoi (strVal));

	m_oSongManager.GetOtherValue (L"PathToMp3Width",	strVal);
	m_oSongList.SetColumnWidth (LIST_SONG_COL_MP3, _tstoi (strVal));


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

} // end CSongsBestPickerDlg::SaveWindowPosition



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
	*pResult = 0;
} // end on rclick song list




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




