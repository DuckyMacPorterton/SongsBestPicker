// HotkeyManager.h: interface for the CHotkeyManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <map>
#include "SQLite/CppSQLite3-Unicode.h"

#define HOTKEY_MANAGER_VERSION  1

class CHotkeyCommand;

//
//  Commands for a particular category are here...

typedef CTypedPtrArray<CPtrArray, CHotkeyCommand *> CHotkeyCommandArray;

//
//  This holds the set of hotkey categories

//typedef std::map<CString, CHotkeyCommandArray *> CStringToHotkeyCommandMap;

//
//  And our actual manager class...

class CHotkeyManager : public CObject
{
public:
	CHotkeyManager ();
	virtual ~CHotkeyManager();

	void	SetAsVPHotkeys (HACCEL *phAccel);
	bool	SaveHotkeys (CMyCppSQLite3DBPtr pDB);
	bool	LoadHotkeys (CMyCppSQLite3DBPtr pDB);

	void	AddCommand (CHotkeyCommand* pHotkey);
	int		GetCommandCount () {return (int) m_arrHotkeys.GetSize ();};

	void	RemoveAllHotkeys ();

	bool	SetError (CString strError);
	void	DumpHotkeys ();

public:
	//
	//  Command stuff

	CHotkeyCommand* GetCommandByIndex (int nIndex);

	CHotkeyCommand* GetCommandUsingHotkey (UINT nKey, UINT nModifiers);
	CHotkeyCommand* GetCommandByID (int nID);
	CHotkeyCommand* GetCommandByItemData (INT64 nDataToMatch);

	int     GetTotalNumberOfHotkeys ();

	//
	//  Default stuff... 

	int     GetDefaultHotkeyForCommand (int nCommandID, UINT &nKey, UINT &nModifiers);
	void    ResetToSystemDefaults ();

protected:
	CHotkeyCommandArray *GetCommandArray () {return &m_arrHotkeys;};

	CString						m_strError;

	CHotkeyCommandArray			m_arrHotkeys;	//  No need for multiple categories, we don't have that many hotkeys
};
