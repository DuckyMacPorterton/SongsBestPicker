#pragma once
#include "SQLite/CppSQLite3-Unicode.h"
#include <map>
#include "Song.h"
#include "StdioFileEx/StdioFileEx.h"
#include <fmod.hpp>

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

	void	SetFmodSystem (FMOD::System* pFmodSystem) {m_pFmodSystem = pFmodSystem;};

	CString	GetError (bool bClearError = false);

	bool	InitSongsFromTextFile	(CString strTextFile, EFileFormat eFileFormat, bool bOverwriteExistingData = false);
protected:
	CString GuessMoreLikelyToBeReal (CString strOne, CString strTwo);

public:
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

	//
	//   Dealing with meta tags

	bool	LoadTagsFromMp3			(CString strPathToMp3, CString& rstrTitle, CString& rstrArtist, CString& rstrAlbum);
	bool	GetTagNamesForType (CString strTagType, CStringArray& rarrTagNames);
	bool	ReadSingleTag (FMOD::Sound* pSoundToLoadTags, CString strTagName, CString& rstrValue);

	//
	//  Errors?  We never have errors

	bool	SetError (CString strError);

protected:
	bool	ReadNextSongM3U (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3);
	bool	ReadnextSongTab (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3);

protected:
	CString				m_strError;
	CMyCppSQLite3DBPtr	m_pDB;
	FMOD::System*		m_pFmodSystem	= NULL;


	int					m_nPoolSize = 5;	//  Our DB table design is not flexible enough to let us change this right now

};


