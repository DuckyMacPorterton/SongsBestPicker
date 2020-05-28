#pragma once
#include "SQLite/CppSQLite3-Unicode.h"
#include <map>
#include "Song.h"


typedef std::map<UINT, CSongPtr>		CSongMap;	//  So, UINT == other team ID, int == margin of victory.  + == win, - == loss


class CSongManager
{
public:
	CSongManager ();
	~CSongManager ();

	bool	LoadFromTextFile (bool bOverwriteExistingData = false);
	CString	GetError () {return m_strError;};

protected:
	bool	LoadFromDB ();
	bool	SaveToDB ();

	bool	LoadGamesFromDB (CMyCppSQLite3DBPtr pDB, CString &rstrError);

	void	SetGameResult (UINT nSong1ID, UINT nSong2ID, int nSong1MarginOfVictory);
//	bool	GetGameResult (UINT nOpponentID, int& rnMarginOfVictory);


	bool	SetError (CString strError);


protected:
	CString				m_strError;
	CMyCppSQLite3DBPtr	m_pDB;

	CSongMap			m_mapIdToSong;
};


