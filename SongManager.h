#pragma once
#include "SQLite/CppSQLite3-Unicode.h"
#include <map>
#include "Song.h"
#include "StdioFileEx/StdioFileEx.h"

typedef CArray<int>		CIntArray;

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

	bool	ScheduleMorePods ();

	bool	GetWonLossRecord (int nSongID, int& rnWins, int& rnLosses);

	bool	GetSongCount	(int& rnSongCount);
	bool	GetNextSong		(CString& rstrSongName, CString& rstrPathToMp3, int& rnSongID, int nPrevSongID = -1);
	bool	GetSongDetails	(int nSongID, CString& rstrSongName, CString& rstrPathToMp3);
	
	bool	GetAllSongsInRandomOrder (CIntArray& rarrSongIDs);

	bool	SetSongName		(int nSongID, CString strName);
	bool	SetSongPathToMp3(int nSongID, CString strPathtoMp3);

//	void	SetGameResult	(int nSong1ID, int nSong2ID, int nSong1MarginOfVictory);
//	bool	GetGameResult (UINT nOpponentID, int& rnMarginOfVictory);

	bool	GetUnfinishedPodCount (int& rnUnfinishedPoolCount);
	bool	GetCurrentPod	(CIntArray& rarrSongIDs);
	bool	SetPodRankings	(CIntArray& rarrSongIDs);

	bool	SetError (CString strError);

protected:
	bool	ReadNextSongM3U (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3);
	bool	ReadnextSongTab (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3);

protected:
	CString				m_strError;
	CMyCppSQLite3DBPtr	m_pDB;

	int					m_nPoolSize = 5;	//  Our DB table design is not flexible enough to let us change this right now

};


