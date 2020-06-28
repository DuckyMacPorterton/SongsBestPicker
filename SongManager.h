#pragma once
#include "SQLite/CppSQLite3-Unicode.h"
#include <map>
#include "Song.h"
#include "StdioFileEx/StdioFileEx.h"
#include <fmod.hpp>
#include "typedefs.h"

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

	CMyCppSQLite3DBPtr GetDB () {return m_pDB;};

	void	SetFmodSystem (FMOD::System* pFmodSystem) {m_pFmodSystem = pFmodSystem;};

	CString	GetError (bool bClearError = false);

	bool	InitSongsFromTextFile	(CString strTextFile, EFileFormat eFileFormat, bool bOverwriteExistingData = false);
protected:
	CString GuessMoreLikelyToBeReal (CString strOne, CString strTwo);

public:
	bool	DeleteAllSongs		();
	bool	DeleteAllSongStats	();
	bool	DeleteSong			(int nSongID);
	bool	AddSong				(int& rnNewSongID, CString strTitle, CString strPathToMp3, CString strArtist = L"", CString strAlbum = L"");
	bool	GetDoesSongExist	(int nSongID);

	bool	ScheduleMorePods	();

	bool	GetWonLossRecord	(int nSongID, int& rnWins, int& rnLosses);
	bool	GetHeadToHeadForSong	(int nSongID, CIntArray&rarrOpponents, CIntArray& rarrMargins);

	bool	GetSongCount	(int& rnSongCount);
	bool	GetNextSong		(CString& rstrSongTitle, CString& rstrSongArtist, CString& rstrSongAlbum, CString& rstrPathToMp3, int& rnSongID, int nPrevSongID = -1);
	bool	GetSongDetails	(int nSongID, CString& rstrSongTitle, CString& rstrSongArtist, CString& rstrSongAlbum, CString& rstrPathToMp3);
	bool	GetSongTitle	(int nSongID, CString& rstrSongTitle);

	bool	SetSongDetails	(int nSongID, CString strSongTitle, CString strSongArtist, CString strSongAlbum, CString strPathToMp3);

	bool	GetSongRating	(int nSongID, int& rnSongRating);
	bool	SetSongRating	(int nSongID, int nSongRating);
	bool	RecalcAllSongRatings	(bool bResetExistingRatingsFirst = true);

	bool	GetSongStrengthOfSchedule	(int nSongID, int& rnStrengthOfSchedule);
	bool	SetSongStrengthOfSchedule	(int nSongID, int nStrengthOfSchedule);
	bool	RecalcStrengthOfSchedule	(int nSongID, int& rnStrengthOfSchedule);

	bool	GetAllSongsInRandomOrder (CIntArray& rarrSongIDs);

//	bool	SetSongTitle		(int nSongID, CString strName);
	bool	SetSongPathToMp3	(int nSongID, CString strPathtoMp3);

	bool	GetFinishedPodCount		(int& rnFinishedPoolCount);
	bool	GetUnfinishedPodCount	(int& rnUnfinishedPoolCount);
	bool	GetArtistCount			(int& rnCount);

	bool	GetAllPodIDs			(CIntArray& rarrPodIDs);

	bool	GetCurrentPod			(int& rnPodID, CIntArray& rarrSongIDs);
	bool	GetPod					(int nPodID, CIntArray& rarrSongIDs, bool& rbPodFinished);

	bool	SetPodRankings			(int nPodID, CIntArray& rarrSongIDs, bool bMarkPodFinished = true);

	bool	GetUndefeatedSongCount	(int& rnUndefeatedSongCount);
	bool	GetSongsNotPlayedCount	(int& rnSongsThatHaveNotPlayed);

	bool	GetTotalHeadToHeadCount (int& rnTotalHeadToHead);

	//
	//  For type to filter

	bool	GetAllSongsMatchingFilter	(CIntArray& rarrSongIDs,	bool bUseRegex, CString strFilter);
	bool	GetAllPodsMatchingFilter	(CIntArray& rarrPodIDs,		bool bUseRegex, CString strFilter);


	//
	//   Dealing with meta tags

	bool	LoadTagsFromMp3			(CString strPathToMp3, CString& rstrTitle, CString& rstrArtist, CString& rstrAlbum);
	bool	GetTagNamesForType (CString strTagType, CStringArray& rarrTagNames);
	bool	ReadSingleTag (FMOD::Sound* pSoundToLoadTags, CString strTagName, CString& rstrValue);

	//
	//  Non-song DB stuff

	bool	GetOtherValue (CString strName, CString& rstrValue);
	bool	SetOtherValue (CString strName, CString strValue);

	bool	GetColumnCount		(int& rnColumnsToDisplay);
	bool	GetColumnSetupInfo	(int nColIndex, int& rnColType, CString& rstrColName, int& rnFormat, int& rnWidth);
	bool	SetColumnSetupInfo	(int nColIndex, int nColType, CString strColName, int nFormat, int nWidth);
	bool	DeleteAllColumns	();

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


	int					m_nPodSize = 5;	//  Our DB table design is not flexible enough to let us change this right now

};


