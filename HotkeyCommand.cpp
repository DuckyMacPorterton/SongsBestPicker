// HotkeyCommand.cpp: implementation of the CHotkeyCommand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HotkeyCommand.h"
#include "SQLite/CppSQLite3-Unicode.h"
#include "SqlDefs.h"
#include "Utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHotkey::CHotkey ()
{
	m_nKey = 0;
	m_nModifiers = 0;
}


//
//

CHotkeyCommand::CHotkeyCommand (int nID, CString strName)
{
	m_nCommandID = nID;
	m_strName = strName;

	m_Hotkeys.SetSize (0);
}

CHotkeyCommand::CHotkeyCommand()
{
	m_Hotkeys.SetSize (0);
	m_nCommandID = -1;
}

CHotkeyCommand::~CHotkeyCommand()
{

}


//************************************
// Method:    LoadHotkeyCombos
// FullName:  CHotkeyCommand::LoadHotkeyCombos
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CMyCppSQLite3DBPtr pDB
//************************************
bool CHotkeyCommand::LoadHotkeys (CString& rstrError, CMyCppSQLite3DBPtr pDB)
{
	if (NULL == pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select * from %s where %s=%d", TBL_HOTKEY_COMBOS, DB_COL_COMMAND_ID, m_nCommandID);

		CppSQLite3Query oQuery = pDB->execQuery (strQuery);
		for (; !oQuery.eof (); oQuery.nextRow ())
		{
			AddHotkey ((UINT) oQuery.getIntField (DB_COL_KEY), (UINT) oQuery.getIntField (DB_COL_MODIFIERS));
		}

		return true;
	}
	catch (CppSQLite3Exception& e) {
		rstrError = e.errorMessage ();
		return false;
	}
	catch (CException* e) {
		rstrError = CUtils::GetErrorMessageFromException (e, true);
		return false;
	}
} // end CHotkeyCommand::LoadHotkeyCombos




//************************************
// Method:    SaveHotkeys
// FullName:  CHotkeyCommand::SaveHotkeys
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CString & rstrError
// Parameter: CMyCppSQLite3DBPtr pDB
//************************************
bool CHotkeyCommand::SaveHotkeys (CString& rstrError, CMyCppSQLite3DBPtr pDB)
{
	if (NULL == pDB)
		return false;

	try
	{
		//
		//  First remove anything existing

		CString strDelete;
		strDelete.Format (L"delete from %s where %s=%d", TBL_HOTKEY_COMBOS, DB_COL_COMMAND_ID, m_nCommandID);
		pDB->execDML (strDelete);

		//
		//  Then put current hotkeys there

		CString strInsert;
		strInsert.Format (L"insert into %s (%s, %s, %s) values (%d, ?, ?)", TBL_HOTKEY_COMBOS, 
			DB_COL_COMMAND_ID, DB_COL_KEY, DB_COL_MODIFIERS, m_nCommandID);

		CppSQLite3Statement stmtQuery = pDB->compileStatement (strInsert);

		for (int i = 0; i < m_Hotkeys.GetSize (); i ++)
		{
			stmtQuery.bind (1, (int) m_Hotkeys[i].m_nKey);
			stmtQuery.bind (2, (int) m_Hotkeys[i].m_nModifiers);
			stmtQuery.execDML ();
		}

		return true;
	}
	catch (CppSQLite3Exception& e) {
		rstrError = e.errorMessage ();
		return false;
	}
	catch (CException* e) {
		rstrError = CUtils::GetErrorMessageFromException (e, true);
		return false;
	}

} // end CHotkeyCommand::SaveHotkeys





int CHotkeyCommand::GetHotkeyCount ()
{
	return (int) m_Hotkeys.GetSize ();
} 

void CHotkeyCommand::RemoveAllHotkeys ()
{
	m_Hotkeys.SetSize (0);
}

int CHotkeyCommand::RemoveHotkeyByIndex (int nIndex)
{
	if (nIndex < 0 || nIndex >= m_Hotkeys.GetSize ())
		return false;

	m_Hotkeys.RemoveAt (nIndex);
	return true;
}


int CHotkeyCommand::RemoveHotkey (UINT nKey, UINT nModifiers)
{
	for (int i = 0; i < m_Hotkeys.GetSize (); i ++)
	{
		if (m_Hotkeys[i].m_nKey == nKey &&
			m_Hotkeys[i].m_nModifiers == nModifiers)
		{
			m_Hotkeys.RemoveAt (i);
			return true;
		}
	}

	return false;
}


void CHotkeyCommand::AddHotkey (UINT nKey, UINT nModifiers)
{
	if (nKey == 0)
		return;

	CHotkey hk;
	hk.m_nKey = nKey;
	hk.m_nModifiers = nModifiers;

	m_Hotkeys.Add (hk);
}

int CHotkeyCommand::GetHotkeyByIndex (int nIndex, UINT &nKey, UINT &nModifiers)
{
	if (nIndex < 0 || nIndex >= m_Hotkeys.GetSize ())
		return false;

	nKey = m_Hotkeys[nIndex].m_nKey;
	nModifiers = m_Hotkeys[nIndex].m_nModifiers;

	return true;
}



/////////////////////////////////////////////////////////////////////
//
//  F I N D   H O T K E Y
//
//  Returns true if this command currently uses this hotkey.
//  False otherwise.
//
int CHotkeyCommand::FindHotkey (UINT nKey, UINT nModifiers)
{
	for (int i = 0; i < m_Hotkeys.GetSize (); i ++)
	{
		if (m_Hotkeys[i].m_nKey == nKey &&
			m_Hotkeys[i].m_nModifiers == nModifiers)
			return true;
	}

	return false;

} // end find hotkey
