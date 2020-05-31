
// DougHotkeysDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SongsBestPicker.h"
#include "SongsBestPickerDlg.h"
#include "afxdialogex.h"
#include <mciapi.h>
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define LIST_SONG_COL_NAME		0
#define LIST_SONG_COL_WONLOSS	1
#define LIST_SONG_COL_RATING	2
#define LIST_SONG_COL_MP3		3

#define SONG_STATUS_TIMER_ID	3
#define SONG_STATUS_TIMER_MS	1000

//
//  Resize amounts

#define MY_BIG_AMOUNT				100
#define MY_SMALL_AMOUNT				 1


//
//  Command ids

#define ID_GROW_TO_THE_RIGHT			WM_USER + 1
#define ID_SHRINK_FROM_THE_RIGHT		WM_USER + 2

#define ID_GROW_TO_THE_RIGHT_SMALL		WM_USER + 3
#define ID_SHRINK_FROM_THE_RIGHT_SMALL	WM_USER + 4

#define ID_GROW_TO_THE_LEFT				WM_USER + 5
#define ID_SHRINK_FROM_THE_LEFT			WM_USER + 6

#define ID_GROW_TO_THE_LEFT_SMALL		WM_USER + 7
#define ID_SHRINK_FROM_THE_LEFT_SMALL	WM_USER + 8

//
//  Up / down

#define ID_GROW_TO_THE_DOWN			WM_USER + 9
#define ID_SHRINK_FROM_THE_DOWN		WM_USER + 10

#define ID_GROW_TO_THE_DOWN_SMALL		WM_USER + 11
#define ID_SHRINK_FROM_THE_DOWN_SMALL	WM_USER + 12

#define ID_GROW_TO_THE_UP				WM_USER + 13
#define ID_SHRINK_FROM_THE_UP			WM_USER + 14

#define ID_GROW_TO_THE_UP_SMALL		WM_USER + 15
#define ID_SHRINK_FROM_THE_UP_SMALL	WM_USER + 16

//
//



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

	{ID_GROW_TO_THE_RIGHT,				_T("ID_GROW_TO_THE_RIGHT"),				_T(""),  MOD_ALT | MOD_SHIFT, VK_RIGHT},
	{ID_SHRINK_FROM_THE_RIGHT,			_T("ID_SHRINK_FROM_THE_RIGHT"),			_T(""), MOD_ALT | MOD_SHIFT, VK_LEFT},
	{ID_GROW_TO_THE_RIGHT_SMALL,		_T("ID_GROW_TO_THE_RIGHT_SMALL"),		_T(""), MOD_ALT | MOD_SHIFT | MOD_WIN, VK_RIGHT},
	{ID_SHRINK_FROM_THE_RIGHT_SMALL,	_T("ID_SHRINK_FROM_THE_RIGHT_SMALL"),	_T(""), MOD_ALT | MOD_SHIFT | MOD_WIN, VK_LEFT},
	
	{ID_GROW_TO_THE_LEFT,				_T("ID_GROW_TO_THE_LEFT"),				_T(""), MOD_ALT | MOD_WIN, VK_LEFT},
	{ID_SHRINK_FROM_THE_LEFT,			_T("ID_SHRINK_FROM_THE_LEFT"),			_T(""), MOD_ALT | MOD_WIN, VK_RIGHT},
	{ID_GROW_TO_THE_LEFT_SMALL,			_T("ID_GROW_TO_THE_LEFT_SMALL"),		_T(""), MOD_CONTROL | MOD_ALT | MOD_WIN, VK_LEFT},
	{ID_SHRINK_FROM_THE_LEFT_SMALL,		_T("ID_SHRINK_FROM_THE_LEFT_SMALL"),	_T(""), MOD_CONTROL | MOD_ALT | MOD_WIN, VK_RIGHT},

	//
	//   Up / down

	{ID_GROW_TO_THE_DOWN,				_T("ID_GROW_TO_THE_DOWN"),				_T(""),  MOD_ALT | MOD_SHIFT, VK_DOWN},
	{ID_SHRINK_FROM_THE_DOWN,			_T("ID_SHRINK_FROM_THE_DOWN"),			_T(""), MOD_ALT | MOD_SHIFT, VK_UP},
	{ID_GROW_TO_THE_DOWN_SMALL,			_T("ID_GROW_TO_THE_DOWN_SMALL"),		_T(""), MOD_ALT | MOD_SHIFT | MOD_WIN, VK_DOWN},
	{ID_SHRINK_FROM_THE_DOWN_SMALL,		_T("ID_SHRINK_FROM_THE_DOWN_SMALL"),	_T(""), MOD_ALT | MOD_SHIFT | MOD_WIN, VK_UP},
	
	{ID_GROW_TO_THE_UP,					_T("ID_GROW_TO_THE_UP"),				_T(""), MOD_ALT | MOD_WIN, VK_UP},
	{ID_SHRINK_FROM_THE_UP,				_T("ID_SHRINK_FROM_THE_UP"),			_T(""), MOD_ALT | MOD_WIN, VK_DOWN},
	{ID_GROW_TO_THE_UP_SMALL,			_T("ID_GROW_TO_THE_UP_SMALL"),			_T(""), MOD_CONTROL | MOD_ALT | MOD_WIN, VK_UP},
	{ID_SHRINK_FROM_THE_UP_SMALL,		_T("ID_SHRINK_FROM_THE_UP_SMALL"),		_T(""), MOD_CONTROL | MOD_ALT | MOD_WIN, VK_DOWN},


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

    CString message(messageBuffer, size);

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
	, m_strCurSongName		(L"")
	, m_strCurSongPathToMp3	(L"")
	, m_strSongPlaybackPos(_T(""))
	, m_strSongPlaybackLen(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSongsBestPickerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SONG_LIST,			m_oSongList);
	DDX_Control(pDX, IDC_STATS,				m_oStatsList);
	DDX_Control(pDX, IDC_CURRENT_POD_LIST,	m_oCurrentPodList);
	DDX_Text(pDX, IDC_EDIT1,				m_strCurSongName);
	DDX_Text(pDX, IDC_EDIT2,				m_strCurSongPathToMp3);
	DDX_Text(pDX, IDC_SONG_POS,				m_strSongPlaybackPos);
	DDX_Text(pDX, IDC_SONG_LENGTH,			m_strSongPlaybackLen);
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
	ON_COMMAND(ID_DOUG_SHOWWINDOW,		OnShowMyWindow)
	ON_COMMAND(ID_IMPORTFROMM3UFILE,	OnImportFromM3UFile)
	ON_COMMAND(ID_RESETSONGSTATISTICS,	OnResetSongStatistics)
	ON_COMMAND(ID_DELETESONGLIST,		OnDeleteSongList)

	ON_WM_TIMER()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SONG_LIST, &CSongsBestPickerDlg::OnItemChangedSongList)

	ON_BN_CLICKED(IDC_PLAY_SONG,	&CSongsBestPickerDlg::OnBnClickedPlaySong)
	ON_BN_CLICKED(IDC_PAUSE_SONG,	&CSongsBestPickerDlg::PauseSong)
	ON_BN_CLICKED(IDC_STOP_SONG,	&CSongsBestPickerDlg::StopSong)

	ON_MESSAGE (MM_MCINOTIFY,			OnMciNotify)

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

	m_oSongList.InsertColumn (LIST_SONG_COL_NAME,		L"Song",	LVCFMT_LEFT,	(int) (rcList.Width () * 0.5));
	m_oSongList.InsertColumn (LIST_SONG_COL_WONLOSS,	L"Record",	LVCFMT_CENTER,	(int) (rcList.Width () * 0.19));
	m_oSongList.InsertColumn (LIST_SONG_COL_RATING,		L"Rating",	LVCFMT_CENTER,	(int) (rcList.Width () * 0.19));
	m_oSongList.InsertColumn (LIST_SONG_COL_MP3,		L"MP3",		LVCFMT_LEFT,	(int) (rcList.Width () * 0.11));

	m_oSongList.SetExtendedStyle (m_oSongList.GetExtendedStyle () | LVS_EX_FULLROWSELECT);

	//
	//  For anything to work, we need a valid database.  Confirm that we have one.

	if (! m_oSongManager.GetError ().IsEmpty ())
		AfxMessageBox (L"Error: " + m_oSongManager.GetError ());
	else
		UpdateSongList ();


	//
	//  Let's allow ourselves to minimize to system tray

	m_oTrayIcon.Create (this, ID_MY_NOTIFY, _T("Doug's Rockin' Hotkeys!"), m_hIcon, IDR_MENU_DOUGS_HOTKEYS);


	//
	//  And apply the hotkeys

//	ApplyHotkeys ();

	//
	//  Start a timer to keep tabs on things

	m_nSongPlayingStatusTimerID = SetTimer (SONG_STATUS_TIMER_ID, SONG_STATUS_TIMER_MS, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}




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
	RemoveHotkeys ();
	CDialogEx::OnOK();
}


void CSongsBestPickerDlg::OnBnClickedCancel()
{
	OnBnClickedOk ();
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
void CSongsBestPickerDlg::PlaySong (CString strFileToPlay)
{
	//
	//  Play our song...
	//  Apparently this does not work with spaces in the filename, even with quotes.

	mciSendString (L"close all", NULL, 0, NULL);

	//
	//  Now actually try to play our file

	CString		strShortFilename;
	wchar_t* pBuffer = strShortFilename.GetBuffer (MAX_PATH);

	if (0 != GetShortPathName (strFileToPlay, pBuffer, MAX_PATH))
	{
		strShortFilename.ReleaseBuffer ();

		CString strOpenCmd;
		strOpenCmd.Format (L"open \"%s\" alias MediaFile", strShortFilename);	 // mpegvideo 
		MCIERROR mciResult = mciSendString (strOpenCmd, NULL, 0, NULL);
		if (0 == mciResult)
		{
			mciResult = mciSendString (L"set MediaFile time format milliseconds", NULL, 0, NULL);
			if (0 != mciResult)
				TRACE (L"Error: %s\n", CUtils::GetMciErrorString (mciResult));

			mciResult = mciSendString (L"play MediaFile from 0 notify", NULL, 0, m_hMainDlgWnd); //  wait
			if (0 == mciResult)
				m_eSongPlayingStatus = ESongPlayStatus::ePlaying;
			else
				TRACE (L"Error: %s\n", CUtils::GetMciErrorString (mciResult));
		}
		else
		{
			TRACE (L"Error: %s\n", CUtils::GetMciErrorString (mciResult));
		}
	}

} // end play song






//************************************
// Method:    PauseSong
// FullName:  CSongsBestPickerDlg::PauseSong
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::PauseSong ()
{
	mciSendString (L"pause MediaFile", NULL, 0, NULL);

} // end CSongsBestPickerDlg::PauseSong



//************************************
// Method:    StopSong
// FullName:  CSongsBestPickerDlg::StopSong
// Access:    public 
// Returns:   void
// Qualifier:
//
//
//
//************************************
void CSongsBestPickerDlg::StopSong ()
{
	mciSendString (L"stop MediaFile", NULL, 0, NULL);

} // end CSongsBestPickerDlg::StopSong













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
	AfxMessageBox (L"Not supported yet");

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

	int		nLastID		= -1;
	CString strSongName, strPathToMp3, strWonLoss;
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
		if (! m_oSongManager.GetNextSong (strSongName, strPathToMp3, nLastID, nLastID))
		{
			AfxMessageBox (m_oSongManager.GetError ());
			m_oSongList.DeleteAllItems ();
			m_oSongList.SetRedraw (true);
			m_oSongList.UpdateWindow ();
			return;
		}

		int nIndex = m_oSongList.InsertItem (i, strSongName);
		m_oSongList.SetItemData (nIndex, nLastID);
		m_oSongList.SetItemText (nIndex, LIST_SONG_COL_MP3, strPathToMp3);

		if (! m_oSongManager.GetWonLossRecord (nLastID, nWins, nLosses)) {
			AfxMessageBox (m_oSongManager.GetError ());
			m_oSongList.SetRedraw (true);
			m_oSongList.UpdateWindow ();
			return;
		}

		strWonLoss.Format (L"%2d / %2d", nWins, nLosses);
		m_oSongList.SetItemText (nIndex, LIST_SONG_COL_WONLOSS, strWonLoss);

	} // end loop through songs

	m_oSongList.SetRedraw (true);
	m_oSongList.UpdateWindow ();

} // end CSongsBestPickerDlg::UpdateSongList




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
void CSongsBestPickerDlg::UpdateCurrentPod (int nSongID /* = -1 */)
{
	m_oCurrentPodList.DeleteAllItems ();
	if (-1 == nSongID)
		return;

} // end CSongsBestPickerDlg::UpdateCurrentPod



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
void CSongsBestPickerDlg::UpdateStatsForCurrentSong (int nSongID /* = -1 */)
{
	m_oStatsList.DeleteAllItems ();
	if (-1 == nSongID)
		return;

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

	UpdateData ();

	CString strValue;
	wchar_t* pBuffer = strValue.GetBuffer (128);
	mciSendString (L"status MediaFile position wait", pBuffer, 128, NULL);
	strValue.ReleaseBuffer ();

	int nTimeMS = 0;
	if (CUtils::MyAtoI (strValue, nTimeMS))
	{
		int nTimeSec = (int) (nTimeMS / 1000);

		int nMinutes	= nTimeSec / 60;
		int nSecRemain	= nTimeSec % 60;

		m_strSongPlaybackPos.Format (L"%2d:%02d", nMinutes, nSecRemain);
		UpdateData (false);
	}

#ifdef IfThisWorked

	pBuffer = strValue.GetBuffer (128);
	mciSendString (L"status MediaFile length wait", pBuffer, 128, NULL);
	strValue.ReleaseBuffer ();
	TRACE (L"length: %s\n", strValue);

	pBuffer = strValue.GetBuffer (128);
	mciSendString (L"status MediaFile time format wait", pBuffer, 128, NULL);
	strValue.ReleaseBuffer ();
	TRACE (L"time format: %s\n", strValue);

	pBuffer = strValue.GetBuffer (128);
	mciSendString (L"status MediaFile position start", pBuffer, 128, NULL);
	strValue.ReleaseBuffer ();
	TRACE (L"position start: %s\n", strValue);

	pBuffer = strValue.GetBuffer (128);
	mciSendString (L"status MediaFile postroll duration", pBuffer, 128, NULL);
	strValue.ReleaseBuffer ();
	TRACE (L"postroll duration: %s\n", strValue);

	pBuffer = strValue.GetBuffer (128);
	mciSendString (L"status MediaFile start position", pBuffer, 128, NULL);
	strValue.ReleaseBuffer ();
	TRACE (L"start position: %s\n\n", strValue);
#endif

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
	MoveWindowForHotkey (nHotKeyId);
	m_nHotkeyCurrentlyDown = nHotKeyId;

	CDialogEx::OnHotKey(nHotKeyId, nKey1, nKey2);

//	m_nHotkeyDownTimerId = SetTimer (1, 100, NULL);
}



void CSongsBestPickerDlg::MoveWindowForHotkey (UINT nHotKeyId, int nLargeMoveMultiplier /* = 1 */, int nSmallMoveMultiplier /* = 1 */)
{
	CWnd* pActiveWnd = GetForegroundWindow ();
	if (NULL == pActiveWnd)
		return;

	CRect rcWnd;
	pActiveWnd->GetWindowRect (rcWnd);

	//
	//  Left / right

	if (ID_GROW_TO_THE_RIGHT == nHotKeyId)
		rcWnd.right += MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_RIGHT == nHotKeyId)
		rcWnd.right -= MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_GROW_TO_THE_RIGHT_SMALL == nHotKeyId)
		rcWnd.right += MY_SMALL_AMOUNT * nSmallMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_RIGHT_SMALL == nHotKeyId)
		rcWnd.right -= MY_SMALL_AMOUNT * nSmallMoveMultiplier;

	else if (ID_GROW_TO_THE_LEFT == nHotKeyId)
		rcWnd.left -= MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_LEFT == nHotKeyId)
		rcWnd.left += MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_GROW_TO_THE_LEFT_SMALL == nHotKeyId)
		rcWnd.left -= MY_SMALL_AMOUNT * nSmallMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_LEFT_SMALL == nHotKeyId)
		rcWnd.left += MY_SMALL_AMOUNT * nSmallMoveMultiplier;

	//
	//  Up / down


	else if (ID_GROW_TO_THE_DOWN == nHotKeyId)
		rcWnd.bottom += MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_DOWN == nHotKeyId)
		rcWnd.bottom -= MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_GROW_TO_THE_DOWN_SMALL == nHotKeyId)
		rcWnd.bottom += MY_SMALL_AMOUNT * nSmallMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_DOWN_SMALL == nHotKeyId)
		rcWnd.bottom -= MY_SMALL_AMOUNT * nSmallMoveMultiplier;

	else if (ID_GROW_TO_THE_UP == nHotKeyId)
		rcWnd.top -= MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_UP == nHotKeyId)
		rcWnd.top += MY_BIG_AMOUNT * nLargeMoveMultiplier;
	else if (ID_GROW_TO_THE_UP_SMALL == nHotKeyId)
		rcWnd.top -= MY_SMALL_AMOUNT * nSmallMoveMultiplier;
	else if (ID_SHRINK_FROM_THE_UP_SMALL == nHotKeyId)
		rcWnd.top += MY_SMALL_AMOUNT * nSmallMoveMultiplier;


	//
	//  Actually move us


	pActiveWnd->MoveWindow (rcWnd);
}



void CSongsBestPickerDlg::OnKeyUp (UINT nChar, UINT nRepCnt, UINT nFlags)
{
	//
	//  This doesn't work

	if (0 != m_nHotkeyCurrentlyDown)
	{
		KillTimer (m_nHotkeyDownTimerId);
		m_nHotkeyDownTimerId	= 0;
		m_nHotkeyCurrentlyDown	= 0;
	}
}



void CSongsBestPickerDlg::OnTimer (UINT_PTR nIDEvent) 
{

	if (m_nSongPlayingStatusTimerID == nIDEvent)
	{
		KillTimer (m_nSongPlayingStatusTimerID);
		UpdatePlayerStatus ();
		m_nSongPlayingStatusTimerID = SetTimer (SONG_STATUS_TIMER_ID, SONG_STATUS_TIMER_MS, NULL);

	}

#ifdef SupportTimerForHotkey
	if (0 != m_nHotkeyCurrentlyDown)
	{
		TRACE (_T("Timer\n"));

		//
		//  Still down...  apply it again

		MoveWindowForHotkey (m_nHotkeyCurrentlyDown, 3);
		return;
	}
#endif

	__super::OnTimer (nIDEvent);
}




void CSongsBestPickerDlg::OnShowMyWindow ()
{
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
		UpdateData ();

		//
		//  Save any changes the user made

		if (-1 != m_nCurSongListCtrlIndex)
		{
			m_strCurSongName.Trim ();
			m_strCurSongPathToMp3.Trim ();

			if (! m_strCurSongName.IsEmpty ())
				m_oSongList.SetItemText (m_nCurSongListCtrlIndex, LIST_SONG_COL_NAME, m_strCurSongName);
			if (! m_strCurSongPathToMp3.IsEmpty ())
				m_oSongList.SetItemText (m_nCurSongListCtrlIndex, LIST_SONG_COL_MP3, m_strCurSongPathToMp3);
		}


		//
		//  Woohoo!  We have a newly selected item.

		int nSongID				= -1;
		m_nCurSongListCtrlIndex	= m_oSongList.GetFirstSelectedItem ();
		if (-1 == m_nCurSongListCtrlIndex) {
			nSongID = -1;
			m_strCurSongName.Empty ();
			m_strCurSongPathToMp3.Empty ();
		}
		else
		{
			nSongID					= (int) m_oSongList.GetItemData (m_nCurSongListCtrlIndex);
			m_strCurSongName		= m_oSongList.GetItemText (m_nCurSongListCtrlIndex, LIST_SONG_COL_NAME);
			m_strCurSongPathToMp3	= m_oSongList.GetItemText (m_nCurSongListCtrlIndex, LIST_SONG_COL_MP3);
		}

		UpdateCurrentPod			(nSongID);	// NOT the list ctrl index
		UpdateStatsForCurrentSong	(nSongID);	// NOT the list ctrl index

		UpdateData (false);
	} // end if a new item is selected

} // end item changed in song list





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

	PlaySong (m_strCurSongPathToMp3);

} // end on play song







//************************************
// Method:    OnMciNotify
// FullName:  CSongsBestPickerDlg::OnMciNotify
// Access:    public 
// Returns:   LRESULT
// Qualifier:
// Parameter: WPARAM wParam
// Parameter: LPARAM lParam
//
//
//
//************************************
LRESULT CSongsBestPickerDlg::OnMciNotify (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case MCI_NOTIFY_SUCCESSFUL:
		//
		//  For "Play", successful means it's finished playing

		m_eSongPlayingStatus = ESongPlayStatus::eStopped;
		break;
	case MCI_NOTIFY_FAILURE:
		TRACE (L"Failure\n");
		break;
	case MCI_NOTIFY_ABORTED:
		TRACE (L"Aborted\n");
		break;
	case MCI_NOTIFY_SUPERSEDED:
		TRACE (L"Superseded\n");
		break;
	default:
		TRACE (L"%d\n", (int) wParam);
	}

	return true;

} // end CSongsBestPickerDlg::OnMciNotify

