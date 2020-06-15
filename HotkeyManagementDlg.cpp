// HotkeyManagementDlg.cpp : implementation file
//

#include "stdafx.h"

#include "afxkeyboardmanager.h"
#include "HotkeyManagementDlg.h"
#include "VirtKeys.h"

//#include "HotkeyCommand.h"
//#include "HotkeyManager.h"

#include "Utils.h"
#include "HotkeyCommand.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHotkeyManagementDlg dialog


CHotkeyManagementDlg::CHotkeyManagementDlg (CHotkeyManager *pManager, CWnd* pParent /*=NULL*/)
	: CDialog(CHotkeyManagementDlg::IDD, pParent)
{
	m_strMatchingCommand	= _T("");
	m_pHotkeyManager		= pManager;

	m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE (IDR_ACCEL_HOTKEY_DLG));

}


void CHotkeyManagementDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_HOTKEY, m_HotkeyCtrl);
	DDX_Control(pDX, IDC_LIST_CURRENT_KEYS, m_HotkeyList);
	DDX_Control(pDX, IDC_LIST_COMMAND, m_CommandList);
	DDX_Text(pDX, IDC_MATCHING_STATIC, m_strMatchingCommand);
	DDX_Control(pDX, IDC_EDIT_TYPE_TO_FILTER, m_oTypeToFilter);
}


BEGIN_MESSAGE_MAP(CHotkeyManagementDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ASSIGN, OnAssignHotkey)
	ON_LBN_SELCHANGE(IDC_LIST_COMMAND, OnSelchangeListCommand)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnDeleteHotkey)
	ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnDefaultHotkey)
	ON_EN_CHANGE(IDC_HOTKEY, OnHotkeyUpdate)
	ON_BN_CLICKED(IDC_RESET_ALL_DEFAULTS_BUTTON, OnResetAllDefaultsButton)
	ON_EN_CHANGE(IDC_EDIT_TYPE_TO_FILTER, &CHotkeyManagementDlg::OnEnChangeEditTypeToFilter)
	ON_COMMAND (ID_FIND, OnFind)
	ON_COMMAND (ID_ESCAPE, OnEscape)
	ON_BN_CLICKED(ID_HIDDEN, &CHotkeyManagementDlg::OnBnClickedHidden)
END_MESSAGE_MAP()

//	ON_MESSAGE(WM_APP + 1, OnHotkeyUpdate)


BOOL CHotkeyManagementDlg::PreTranslateMessage(MSG* pMsg)
{
	m_oToolTip.RelayEvent (pMsg);

	if (m_hAccel != NULL)
	{
		if (::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
		{
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}



/////////////////////////////////////////////////////////////////////////////
// CHotkeyManagementDlg message handlers



BOOL CHotkeyManagementDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//
	//  Setup our tooltips

	CString strTip = L"Ctrl+F for Type to Filter, ESCape returns to command list.";

	m_oToolTip.Create (this);
	m_oToolTip.AddTool (&m_oTypeToFilter, strTip);
	m_oToolTip.AddTool (GetDlgItem (IDC_STATIC_FIELD), strTip);
	m_oToolTip.Activate (TRUE);
	
	//
	//  And our categories

	LoadCommandsIntoList ();

	//
	//  Now tell our hotkey control what keys to allow...

	SetupIllegalHotkeys ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CHotkeyManagementDlg::OnOK() 
{
	//
	//  Activate the changes and save them
	//  in the registry

//	ActivateAcceleratorChanges ();
//	m_pHotkeyManager->SaveHotkeys ();
	CDialog::OnOK();
}


void CHotkeyManagementDlg::OnCancel() 
{
	//
	//  Don't want escape to close us

	CDialog::OnCancel();
}



//////////////////////////////////////////////////////////////////
//
//  L O A D   C O M M A N D S   I N T O   L I S T
//
//  Clears out whatever's there and puts all the commands in 
//  the manager into the list
//
void CHotkeyManagementDlg::LoadCommandsIntoList ()
{
	m_CommandList.ResetContent ();

	for (int i = 0; i < m_pHotkeyManager->GetCommandCount (); i ++)
	{
		CHotkeyCommand* pHKC = m_pHotkeyManager->GetCommandByIndex (i);

		//
		//  If we have a m_strTypeToFilter, then only add commands matching it

		if (! m_strTypeToFilter.IsEmpty ())
		{
			if (NULL == CUtils::stristr (pHKC->GetName ().GetBuffer (),		m_strTypeToFilter))
				continue;
		}

		int nIndex = m_CommandList.AddString (pHKC->GetName ());
		m_CommandList.SetItemData (nIndex, (DWORD_PTR) pHKC);

	} // end loop through commands

	//
	//  Select the first command

	m_CommandList.SetCurSel (0);
	OnSelchangeListCommand ();

} // end load commands into list



//////////////////////////////////////////////////////////////////
//
//  L O A D   H O T K E Y S   I N T O   L I S T
//
//  Clears out whatever's there and puts all the hotkeys 
//  belonging to this command into the list
//
void CHotkeyManagementDlg::LoadHotkeysIntoList (CHotkeyCommand *pHKC)
{
	m_HotkeyList.ResetContent ();

	if (pHKC == NULL)
		return;

	UINT nKey, nModifiers;
	for (int i = 0; i < pHKC->GetHotkeyCount (); i ++)
	{
		if (pHKC->GetHotkeyByIndex (i, nKey, nModifiers))
		{
			CString str = CUtils::GetHotkeyText (nKey, nModifiers);

			int nIndex = m_HotkeyList.AddString (str);
			m_HotkeyList.SetItemData (nIndex, i);
		}
	} // end loop through hotkeys in this command
} // end load hotkeys into list




////////////////////////////////////////////////////////////////
//
//  O N   H O T K E Y   U P D A T E
//
//  When the user has hit a hotkey we should display what other
//  command is using it...  this function is called in response
//  to the message HK_HOTKEY_UPDATED sent by CMyHotkeyCtrl
//
void CHotkeyManagementDlg::OnHotkeyUpdate ()
{
	UpdateData ();
	m_strMatchingCommand.Empty ();

	//
	//  Grab the current hotkey...
	
	WORD wVirtualKeyCode, wModifiers;
	m_HotkeyCtrl.GetHotKey (wVirtualKeyCode, wModifiers);

	if (wVirtualKeyCode != 0)
	{
		UINT nModifiers = 0;
		
		if (wModifiers & HOTKEYF_ALT)
			nModifiers |= MOD_ALT;
		if (wModifiers & HOTKEYF_CONTROL)
			nModifiers |= MOD_CONTROL;
		if (wModifiers & HOTKEYF_SHIFT)
			nModifiers |= MOD_SHIFT;
		if (wModifiers & HOTKEYF_EXT)
			nModifiers |= MOD_EXTENDED;

		//
		//  First make sure it's a hotkey we allow

		if (! IsHotkeyAllowed (wVirtualKeyCode, nModifiers))
		{
			m_HotkeyCtrl.SetHotKey (0, 0);
			m_strMatchingCommand = "Illegal hotkey";
		}
		else
		{
			
			//
			//  Make sure no other command is already using this hotkey
			
			CHotkeyCommand *pConflict = m_pHotkeyManager->GetCommandUsingHotkey (wVirtualKeyCode, nModifiers);
			
			if (pConflict)
			{
				m_strMatchingCommand.Format (L"Hotkey currently assign to %s", pConflict->GetName ());
			}
		}
	}

	UpdateData (false);

} // end on hotkey update







////////////////////////////////////////////////////////////////////
//
//  O N   A S S I G N   H O T K E Y
//
//  When the user clicks Assign it saves the current hotkey as 
//  belonging to the currently selected command
//
void CHotkeyManagementDlg::OnAssignHotkey() 
{
	//
	//  Grab the current command...

	int nCurSel = m_CommandList.GetCurSel ();
	if (nCurSel == -1)
		return;

	CHotkeyCommand *pHKC = (CHotkeyCommand *) m_CommandList.GetItemData (nCurSel);
	if (pHKC == NULL)
		return;

	//
	//  Grab the current hotkey...

	WORD wVirtualKeyCode, wModifiers;
	m_HotkeyCtrl.GetHotKey (wVirtualKeyCode, wModifiers);

	if (wVirtualKeyCode == 0)
		return;

	UINT nModifiers = 0;

	if (wModifiers & HOTKEYF_ALT)
		nModifiers |= MOD_ALT;
	if (wModifiers & HOTKEYF_CONTROL)
		nModifiers |= MOD_CONTROL;
	if (wModifiers & HOTKEYF_SHIFT)
		nModifiers |= MOD_SHIFT;
	if (wModifiers & HOTKEYF_EXT)
		nModifiers |= MOD_EXTENDED;

	//
	//  Make sure no other command is already using this hotkey
	
	CHotkeyCommand *pConflict = m_pHotkeyManager->GetCommandUsingHotkey (wVirtualKeyCode, nModifiers);

	if (pConflict)
	{
		CString s; s.Format (L"That hotkey is already being used by %s.\nDo you want to replace it?", pConflict->GetName ());
		if (AfxMessageBox (s, MB_YESNO | MB_DEFBUTTON2) != IDYES)
			return;
		
		//
		//  Remove the old one...

		pConflict->RemoveHotkey (wVirtualKeyCode, nModifiers);
	}


	pHKC->AddHotkey (wVirtualKeyCode, nModifiers);

	LoadHotkeysIntoList (pHKC);

	//
	//  Clear the hotkey control

	m_HotkeyCtrl.SetHotKey (0, 0);

} // end on assign hotkey


//////////////////////////////////////////////////////////////////
//
//  O N   D E L E T E   H O T K E Y 
//
//  Nukes the currently selected hotkey
//
void CHotkeyManagementDlg::OnDeleteHotkey() 
{
	int nCurCmdSel = m_CommandList.GetCurSel ();
	if (nCurCmdSel == -1)
		return;

	CHotkeyCommand *pHKC = (CHotkeyCommand *) m_CommandList.GetItemData (nCurCmdSel);
	if (pHKC == NULL)
		return;

	int nCurKeySel = m_HotkeyList.GetCurSel ();
	if (nCurKeySel == -1)
		return;

	if (! pHKC->RemoveHotkeyByIndex ((int) m_HotkeyList.GetItemData (nCurKeySel)))
		return;

	//
	//  We re-load the strings instead of just deleting that one to keep
	//  our indexes correct (that is, the ItemData which is the index to
	//  which hotkey it is)

	LoadHotkeysIntoList (pHKC);

} // end delete hotkey



//////////////////////////////////////////////////////////////////
//
//  O N   D E F A U L T   H O T K E Y 
//
//  Sets the currently selected command to use the default hotkey
//  for that command
//
void CHotkeyManagementDlg::OnDefaultHotkey() 
{
	//
	//  Get the command and then look up its default hotkey...

	int nCurCmdSel = m_CommandList.GetCurSel ();
	if (nCurCmdSel == -1)
		return;

	CHotkeyCommand *pHKC = (CHotkeyCommand *) m_CommandList.GetItemData (nCurCmdSel);
	if (pHKC == NULL)
		return;

	UINT nKey, nModifiers;
	if (m_pHotkeyManager->GetDefaultHotkeyForCommand (pHKC->GetID (), nKey, nModifiers))
	{
		//
		//  We have a default hotkey...  see if there are any conflicts for it and add it

		CHotkeyCommand *pConflict = m_pHotkeyManager->GetCommandUsingHotkey (nKey, nModifiers);
		
		if (pConflict != NULL)
		{
			if (pConflict == pHKC)
			{
				AfxMessageBox (L"The default hotkey is already assigned to this command");
				return;
			}

			CString s; s.Format (L"The default hotkey is already being used by %s.\nDo you want to replace it?", pConflict->GetName ());
			if (AfxMessageBox (s, MB_YESNO | MB_DEFBUTTON2) != IDYES)
				return;

			//
			//  Remove the old one...

			pConflict->RemoveHotkey (nKey, nModifiers);

		}

		//
		//  ...  and add the new one

		pHKC->AddHotkey (nKey, nModifiers);

		//
		//  We re-load the strings instead of just deleting that one to keep
		//  our indexes correct (that is, the ItemData which is the index to
		//  which hotkey it is)
		
		LoadHotkeysIntoList (pHKC);

	}
	else
	{
		// 
		//  There isn't a default hotkey

		AfxMessageBox (L"There is no default hotkey for this command");
		return;
	}

} // end on default hotkey





////////////////////////////////////////////////////////////////////////
//
//  When the user selects a new command, load it into our hotkey list...
//
void CHotkeyManagementDlg::OnSelchangeListCommand() 
{
	UpdateData ();

	CHotkeyCommand *pHKC = NULL;
	int nCurSel = m_CommandList.GetCurSel ();

	if (nCurSel == -1)
	{
		LoadHotkeysIntoList (NULL);
		m_strMatchingCommand.Empty ();
		m_strDescription.Empty ();
		UpdateData (false);
		return;
	}

	pHKC = (CHotkeyCommand *) m_CommandList.GetItemData (nCurSel);
	LoadHotkeysIntoList (pHKC);

	//
	//  And clear the hotkey and description...

	m_HotkeyCtrl.SetHotKey (0, 0);
	m_strMatchingCommand.Empty ();

	UpdateData (false);

} // end sel change command list



///////////////////////////////////////////////////////////////////
//
//  A C T I V A T E   A C C E L E R A T O R   C H A N G E S
//
//  Takes the accelerator data we have and turns it into
//  the active accelerator table
//
void CHotkeyManagementDlg::ActivateAcceleratorChanges ()
{
	//
	//  This could easily be generalized to take a handle at 
	//  dialog startup and thus replace any accelerator table

	AfxMessageBox (L"Can't set hotkeys yet");

//	SFLFrameWndEx *pFrame = (SFLFrameWndEx *)GetParentFrame();
//	m_pHotkeyManager->SetAsVPHotkeys (&pFrame->m_hAccelTable);

} // end activate accelerator changes



///////////////////////////////////////////////////////////////
//
//  A D D   I L L E G A L   H O T K E Y
//
//  Takes hotkeys that we shouldn't let the user use...
//
void CHotkeyManagementDlg::AddIllegalHotkey (UINT nKey, UINT nModifiers)
{
	m_IllegalHotkeys.insert (std::make_pair (nKey, nModifiers));
}



//////////////////////////////////////////////////////////////////
//
//  I S   H O T K E Y   A L L O W E D
//
//  Checks to see if doug likes the hotkey or not.  Returns true 
//  if he's ok with it, false if it sucks and should be thrown out
//
int CHotkeyManagementDlg::IsHotkeyAllowed (UINT nKey, UINT nModifiers)
{
	if (nKey == 0)
		return false;

	//
	//  See if this particular hotkey is to be ignored
	
	CUIntToUIntMMap::iterator it = m_IllegalHotkeys.find (nKey);
	while ((it != m_IllegalHotkeys.end ()) && ((*it).first == nKey))
	{
		//  That means this key is supposed to be ignored in combination
		//  with some particular modifiers...  check this one
		
		if ((*it).second == nModifiers)
			return false;
		
		it ++;
	}

	return true;

} // is hotkey allowed



///////////////////////////////////////////////////////////
//
//  S E T U P   I L L E G A L   H O T K E Y S
//
//  We don't want to let them do alt-letters or shift-letters
//  or letters or numbers by themselves, or a few other things...
//
void CHotkeyManagementDlg::SetupIllegalHotkeys ()
{
	//
	//  If they don't use any modifier, make it ctrl+

	m_HotkeyCtrl.SetRules (HKCOMB_NONE, HOTKEYF_CONTROL);

	//
	//  First alt-any letter and shift-any letter

	for (int i = VK_A; i <= VK_Z; i ++)
	{
		AddIllegalHotkey (i, MOD_ALT);
		AddIllegalHotkey (i, MOD_SHIFT);
	}

} // end setup illegal hotkeys



/////////////////////////////////////////////////////////////
//
//  O N   R E S E T   A L L   D E F A U L T S   B U T T O N
//
//  Reset all commands to their default values
//
void CHotkeyManagementDlg::OnResetAllDefaultsButton() 
{
	if (AfxMessageBox (L"This will overwrite any changes you might have made.  Do you want to do this?", MB_YESNO | MB_DEFBUTTON2) != IDYES)
		return;

	m_pHotkeyManager->ResetToSystemDefaults ();

	LoadCommandsIntoList ();
	OnSelchangeListCommand ();

}


//////////////////////////////////////////////////////////////////////
//
//  Let's them type to filter what commands we show
//
void CHotkeyManagementDlg::OnEnChangeEditTypeToFilter ()
{
#ifdef WorthFiltering
	m_CategoryList.SetSel (-1, false); // unselect all

	m_oTypeToFilter.GetWindowText (m_strTypeToFilter);
	m_strTypeToFilter.Trim ();

	//
	//  For every command matching the filter, select the parent category, and the
	//  function that loads the categories will filter on commands while displaying

	for (int i = 0; i < m_pHotkeyManager->GetCommandCount (); i++)
	{
		CHotkeyCommand* pHKC = m_pHotkeyManager->GetCommandByIndex (i);

		CString strCmdName = pHKC->GetName ();

		if (NULL != CUtils::stristr (strCmdName.GetBuffer (), m_strTypeToFilter))
		{
			//
			//   A Match!  Select this category and move on

			break; // no need to keep checking this category
		}
	}

	//
	//  Now we've got a set of categories, load the commands into the list

	LoadCommandsIntoList ();
#endif
} // end on edit change type to filter


void CHotkeyManagementDlg::OnFind ()
{
	m_oTypeToFilter.SetFocus ();
	m_oTypeToFilter.SetSel ((DWORD) -1);
} 



void CHotkeyManagementDlg::OnEscape ()
{
	m_CommandList.SetFocus ();
} 




void CHotkeyManagementDlg::OnBnClickedHidden()
{
	//  So ENTER doesn't close dialog
}
