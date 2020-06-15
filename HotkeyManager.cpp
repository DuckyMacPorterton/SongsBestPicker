// HotkeyManager.cpp: implementation of the CHotkeyManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HotkeyManager.h"

#include "HotkeyCommandDefs.h"
#include "HotkeyCommand.h"

#include "Utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHotkeyManager::CHotkeyManager ()
{
//	LoadHotkeys ();
}

CHotkeyManager::~CHotkeyManager()
{
	RemoveAllHotkeys ();
}



//************************************
// Method:    RemoveAllHotkeys
// FullName:  CHotkeyManager::RemoveAllHotkeys
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CHotkeyManager::RemoveAllHotkeys ()
{
	for (int i = 0;  i < m_arrHotkeys.GetSize (); i ++)
	{
		delete m_arrHotkeys[i];
	}
	m_arrHotkeys.SetSize (0);

} // end CHotkeyManager::RemoveAllHotkeys



//************************************
// Method:    SetError
// FullName:  CSongManager::SetError
// Access:    protected 
// Returns:   bool
// Qualifier:
// Parameter: CString strError
//************************************
bool CHotkeyManager::SetError (CString strError)
{
	if (! m_strError.IsEmpty ())
		m_strError += L"\n";
	m_strError += strError;
	return false;
}


///////////////////////////////////////////////////////////
//
//  L O A D   H O T K E Y S
//
//  First sets the hotkeys to the defaults, then overwrites
//  them with any saved changes...  this way if we've added
//  any new commands since their saved file was created
//  we'll still have those in memory.
//
bool CHotkeyManager::LoadHotkeys (CMyCppSQLite3DBPtr pDB)
{
	RemoveAllHotkeys ();

	//
	//  These commands are defined in HotkeyCommandDefs.h.
	//  Make any changes there

	if (NULL == pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select * from %s", TBL_HOTKEYS);

		CppSQLite3Query oQuery = pDB->execQuery (strQuery);
		for (; !oQuery.eof (); oQuery.nextRow ())
		{
			CHotkeyCommand* pHotkey = new CHotkeyCommand (oQuery.getIntField (DB_COL_COMMAND_ID), oQuery.getStringField (DB_COL_NAME));
			m_arrHotkeys.Add (pHotkey);

			CString strError;
			if (! pHotkey->LoadHotkeys (strError, pDB))
				SetError (strError);
		}

		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}

} // end load hotkeys



//************************************
// Method:    AddCommand
// FullName:  CHotkeyManager::AddCommand
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: CHotkeyCommand * pHotkey
//************************************
void CHotkeyManager::AddCommand (CHotkeyCommand* pHotkey)
{
	if (NULL == pHotkey)
		return;

	m_arrHotkeys.Add (pHotkey);
} // end CHotkeyManager::AddCommand



/////////////////////////////////////////////////////////////////
//
//  S A V E   H O T K E Y S
//
//  Saves the current hotkeys to VPHotkeys.dat
//
bool CHotkeyManager::SaveHotkeys (CMyCppSQLite3DBPtr pDB)
{
	try
	{
		pDB->execDML (L"begin transaction");

		CString strInsert;
		strInsert.Format (L"insert or replace into %s (%s, %s, %s) values (?, ?, ?)", TBL_HOTKEYS,
			DB_COL_COMMAND_ID, DB_COL_NAME, DB_COL_DATA);

		CppSQLite3Statement stmtQuery = pDB->compileStatement (strInsert);

		for (int i = 0; i < m_arrHotkeys.GetSize (); i ++)
		{
			stmtQuery.bind (1, m_arrHotkeys[i]->GetID ());
			stmtQuery.bind (2, m_arrHotkeys[i]->GetName ());
			stmtQuery.bind (3, m_arrHotkeys[i]->GetItemData ());
			stmtQuery.execDML ();

			CString strError;
			if (! m_arrHotkeys[i]->SaveHotkeys (strError, pDB))
				SetError (strError);
		}

		pDB->execDML (L"commit transaction");

		return true;
	}
	catch (CppSQLite3Exception& e) {
		pDB->execDML (L"rollback transaction");
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		pDB->execDML (L"rollback transaction");
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end save hotkeys





//////////////////////////////////////////////////////////////////////
//
//  S E T   A S   V P   H O T K E Y S
//
//  Builds an accelerator table and attached it to the parent
//  frame window
//
void CHotkeyManager::SetAsVPHotkeys (HACCEL *phAccel)
{
	AfxMessageBox (L"Ha-ha, you can't set them yet");

#ifdef asdfsa
	//
	//  If they don't give us an HACCEL, use the main frame's

	if (NULL == phAccel)
	{
		CMainFrame* pMainFrame = GetMainFrame ();
		if (NULL == pMainFrame)
			return;

		phAccel = &(pMainFrame->m_hAccelTable);
		if (NULL == phAccel)
			return;
	}

	//
	//  To store our new accelerators...

	int numHotkeys = GetTotalNumberOfHotkeys ();
	ACCEL *AcceleratorTable = new ACCEL[numHotkeys];
	memset (AcceleratorTable, 0, sizeof (ACCEL) * numHotkeys);

	int nCurHotkey = 0;

	for (int nCat = 0; nCat < GetNumCategories (); nCat ++)
	{
		CString strCategory = GetCategoryByIndex (nCat);
		
		for (int nCmd = 0; nCmd < GetNumCommands (strCategory); nCmd ++)
		{
			CHotkeyCommand *pHKC = GetCommandByIndex (strCategory, nCmd);
			
			//
			//  Loop through the hotkeys in this command
			
			for (int nHotkey = 0; nHotkey < pHKC->GetHotkeyCount (); nHotkey ++)
			{
				UINT nKey, nModifiers;
				pHKC->GetHotkeyByIndex (nHotkey, nKey, nModifiers);
				
				AcceleratorTable[nCurHotkey].key = (WORD) nKey;
				AcceleratorTable[nCurHotkey].cmd = (WORD) pHKC->GetID ();
				
				BYTE virt = FVIRTKEY | FNOINVERT;  // tell it we're using virtual key codes | FVIRTKEY 
				
				if (nModifiers & MOD_CONTROL)
					virt |= FCONTROL;
				if (nModifiers & MOD_ALT)
					virt |= FALT;
				if (nModifiers & MOD_SHIFT)
					virt |= FSHIFT;
				
				AcceleratorTable[nCurHotkey].fVirt = virt;
				nCurHotkey ++;
				
			} // end loop through hotkeys in this command
		} // end loop through commands
	} // end loop through categories

	//
	//  Clear out the old one and replace it with our new one

	DestroyAcceleratorTable (*phAccel);
	*phAccel = CreateAcceleratorTable (AcceleratorTable, numHotkeys);

	//
	//  And clean up our memory

	delete [] AcceleratorTable;
#endif
} // end save as vp hotkeys





////////////////////////////////////////////////////////////////////
//
//  G E T   C O M M A N D   B Y   I D
//
//  Loops through the categories until we find a command that
//  has this ID.
//
//  Returns NULL if we don't find anything
//
CHotkeyCommand *CHotkeyManager::GetCommandByID (int nID)
{
	for (int i = 0; i < m_arrHotkeys.GetSize (); i ++)
	{
		if (m_arrHotkeys[i]->GetID () == nID)
			return m_arrHotkeys[i];
	}

	return NULL;
} // end get command by id



CHotkeyCommand* CHotkeyManager::GetCommandByItemData (INT64 nDataToMatch)
{
	if (0 == nDataToMatch)
		return NULL;

	for (int i = 0; i < m_arrHotkeys.GetSize (); i ++)
	{
		if (m_arrHotkeys[i]->GetItemData () == nDataToMatch)
			return m_arrHotkeys[i];
	}

	return NULL;
} // end GetCommandByItemData



CHotkeyCommand *CHotkeyManager::GetCommandByIndex (int nIndex)
{
	if (nIndex < 0 || nIndex >= m_arrHotkeys.GetSize ())
		return NULL;
	
	return m_arrHotkeys.GetAt (nIndex);
}





//////////////////////////////////////////////////////////////
//
//  G E T   T O T A L   N U M B E R   O F   H O T K E Y S
//
//  Counts and returns the total number of hotkeys across all 
//  commands and categories.
//
int CHotkeyManager::GetTotalNumberOfHotkeys ()
{
	int nNumHotkeys = 0;

	for (int i = 0; i < m_arrHotkeys.GetSize (); i ++)
	{
		nNumHotkeys += m_arrHotkeys[i]->GetHotkeyCount ();
	}

	return nNumHotkeys;
} // end get total number of hotkeys



//////////////////////////////////////////////////////////////////
//
//  G E T   H O T K E Y   C O M M A N D   U S I N G   H O T K E Y
//
//  Returns the command currently using the specified hotkey, if
//  any.  If not, returns NULL
//
CHotkeyCommand *CHotkeyManager::GetCommandUsingHotkey (UINT nKey, UINT nModifiers)
{
	for (int i = 0; i < m_arrHotkeys.GetSize (); i ++)
	{
		if (m_arrHotkeys[i]->FindHotkey (nKey, nModifiers))
			return m_arrHotkeys[i];
	}

	return NULL;

} // end get the command that uses this hotkey



////////////////////////////////////////////////////////////////
//
//  G E T   D E F A U L T   H O T K E Y   F O R   C O M M A N D
//
//  Sends back the default hotkey for nCommandID.
//  Returns true if there is one.
//  Returns false if not
//
int CHotkeyManager::GetDefaultHotkeyForCommand (int nCommandID, UINT &nKey, UINT &nModifiers)
{
	nKey = nModifiers = 0;

	for (int i = 0; i < HOTKEY_INIT_NUM_COMMANDS; i ++)
	{
		//
		//  Now add the command...

		if (nCommandID == HKInit[i].nCommandID)
		{
			//
			//   This is us!

			nKey = HKInit[i].nDefaultKey;
			nModifiers = HKInit[i].nDefaultModifiers;

			break;
		}
	}

	if (nKey == 0)
		return false;
	return true;

} // end get default hotkey for command



//////////////////////////////////////////////////////////////////
//
//  R E S E T   T O   S Y S T E M   D E F A U L T S
//
//  Resets all the commands to their system default values
//
void CHotkeyManager::ResetToSystemDefaults ()
{
	RemoveAllHotkeys ();

	//
	//  These commands are defined in HotkeyCommandDefs.h.
	//  Make any changes there

	CHotkeyCommand *pCommand;

	for (int i = 0; i < HOTKEY_INIT_NUM_COMMANDS; i ++)
	{
		//
		//  Now add the command...

		pCommand = new CHotkeyCommand (HKInit[i].nCommandID, HKInit[i].strCommandName);
		pCommand->AddHotkey (HKInit[i].nDefaultKey, HKInit[i].nDefaultModifiers);
		AddCommand (pCommand);
	}
} // end reset to system defaults



void CHotkeyManager::DumpHotkeys ()
{
#ifdef Dumpable
	CTOASApp* pApp = GetToasApp ();

	if (! pApp->GetProfileInt (_T("Options"), _T("DumpHotkeys"), false))
		return;

	try
	{
		CString strPath;
		strPath.Format(_T("%sHotkeyDump.%s.txt"), CVPUtils::GetRootDir(), CVPUtils::GetProductNameDecorated());

		CStdioFileExPtr pFileOut = CStdioFileExPtr (new CStdioFileEx);
		if (!pFileOut->Open(strPath, CFile::modeWrite | CFile::modeCreate))
		{
			AfxMessageBox(_T("Unable to open hotkey dump file: ") + strPath);
			return;
		}


		for (int nCategory = 0; nCategory < GetNumCategories(); nCategory++)
		{
			for (int nCmd = 0; nCmd < GetNumCommands(nCategory); nCmd++)
			{
				CHotkeyCommand* pCmd = GetCommandByIndex(nCategory, nCmd);
				if (NULL == pCmd)
					continue;

				CString strCategory = GetCategoryByIndex (nCategory);
				CString strHotkeys;
				for (int i = 0; i < pCmd->GetHotkeyCount(); i++)
				{
					UINT nKey = 0, nModifiers = 0;
					if (pCmd->GetHotkeyByIndex(i, nKey, nModifiers))
					{
						CString str;
						if (nModifiers & MOD_CONTROL)
							str = "Ctrl+";
						if (nModifiers & MOD_ALT)
							str += "Alt+";
						if (nModifiers & MOD_SHIFT)
							str += "Shift+";

						str += CVPUtils::GetKeyName(nKey, nModifiers & MOD_EXTENDED);

						if (i > 0)
							strHotkeys += _T(", ");
						strHotkeys += str;
					}
				} // end loop through hotkeys in this command


				CString strOut;
				strOut.Format(_T("%s\t%d\t%s\t%s\n"), strCategory, pCmd->GetID(), pCmd->GetName(), strHotkeys);
				pFileOut->WriteString(strOut);
			}
		}

		pFileOut->Close();
		CVPUtils::CopyTextToClipboard(strPath, true);

		CString s;
		s.Format (_T("Done dumping hotkeys.  Copied dump path to clipboard.\nFile is:\n%s"), strPath);
		AfxMessageBox (s);
	}
	catch (CException& rE)
	{
		VP_NOT_USED (rE);
	}
#endif
}


