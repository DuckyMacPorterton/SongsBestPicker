#pragma once
#include "SQLite/CppSQLite3-Unicode.h"
#include <map>
#include "Song.h"
#include "StdioFileEx/StdioFileEx.h"


enum class EFileFormat
{
	eM3U,
	eTab,
};


class CSongManager
{
public:
	CSongManager ();
	~CSongManager ();

	CString	GetError (bool bClearError = false);

	bool	InitSongsFromTextFile	(CString strTextFile, EFileFormat eFileFormat, bool bOverwriteExistingData = false);
	bool	DeleteAllSongs			();


	bool	GetWonLossRecord (int nSongID, int& rnWins, int& rnLosses);

	bool	GetSongCount	(int& rnSongCount);
	bool	GetNextSong		(CString& rstrSongName, CString& rstrPathToMp3, int& rnSongID, int nPrevSongID = -1);

	void	SetGameResult	(int nSong1ID, int nSong2ID, int nSong1MarginOfVictory);
//	bool	GetGameResult (UINT nOpponentID, int& rnMarginOfVictory);


	bool	SetError (CString strError);

protected:
	bool	ReadNextSongM3U (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3);
	bool	ReadnextSongTab (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3);

protected:
	CString				m_strError;
	CMyCppSQLite3DBPtr	m_pDB;

};


