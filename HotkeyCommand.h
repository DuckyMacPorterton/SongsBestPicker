#pragma once
#include "SQLite/CppSQLite3-Unicode.h"

class CHotkey
{
public:
	CHotkey ();

	UINT   m_nModifiers;
	UINT   m_nKey;
};

typedef CArray<CHotkey, CHotkey &> CHotkeyArray;

class CHotkeyCommand : public CObject
{
public:
	CHotkeyCommand ();
	CHotkeyCommand (int nID, CString strName);
	virtual ~CHotkeyCommand();

	bool		LoadHotkeys (CString& rstrError, CMyCppSQLite3DBPtr pDB);
	bool		SaveHotkeys (CString& rstrError, CMyCppSQLite3DBPtr pDB);

	CString		GetName () {return m_strName;};
	void		SetName (CString str) {m_strName = str;};

	int			GetID () {return m_nCommandID;};
	void		SetID (int nID) {m_nCommandID = nID;};

	INT64		GetItemData () {return m_nData;};
	void		SetItemData (DWORD_PTR dwData) {m_nData = dwData;};

	//
	//  These deal with the keystrokes themselves...

	int      GetHotkeyCount ();
	void     RemoveAllHotkeys ();
	int      RemoveHotkeyByIndex (int nIndex);
	int      RemoveHotkey (UINT nKey, UINT nModifiers);

	void     AddHotkey (UINT nKey, UINT nModifiers);
	int      GetHotkeyByIndex (int nIndex, UINT &nKey, UINT &nModifiers);

	int      FindHotkey (UINT nKey, UINT nModifiers);

protected:
	int			m_nCommandID	= 0;
	CString		m_strName;
	INT64		m_nData			= 0;	//  Is DWORD_PTR in other things, but easier just to always store 64 bits

	CHotkeyArray m_Hotkeys;
};
