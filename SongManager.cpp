#include "stdafx.h"
#include "SongManager.h"
#include "SQLite/CppSQLite3-Unicode.h"
#include "Utils.h"
#include "Song.h"


CSongManager::CSongManager ()
{
	try
	{
		//
		//  Either open or create our database.  Open will create if it doesn't exist.

		CString strFilename = CUtils::GetProgramFilesDir () + L"SongsBestPicker.db";
		m_pDB = CMyCppSQLite3DBPtr (new CMyCppSQLite3DB);
		m_pDB->open (strFilename);
		
		//
		//  And make sure our tables are there and correct

		CString strError;
		if (! CUtils::EnsureDBTablesExist (strError, m_pDB))
			SetError (strError);
	}
	catch (CppSQLite3Exception& e)
	{
		SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end constructor



CSongManager::~CSongManager ()
{

}



//************************************
// Method:    GetError
// FullName:  CSongManager::GetError
// Access:    public 
// Returns:   CString
// Qualifier:
// Parameter: bool bClearError
//
//
//
//************************************
CString CSongManager::GetError (bool bClearError /*= false*/)
{
	CString strErr = m_strError;
	if (bClearError)
		m_strError.Empty ();
	return strErr;

} // end CSongManager::GetError



//************************************
// Method:    SetError
// FullName:  CSongManager::SetError
// Access:    protected 
// Returns:   bool
// Qualifier:
// Parameter: CString strError
//************************************
bool CSongManager::SetError (CString strError)
{
	if (! m_strError.IsEmpty ())
		m_strError += L"\n";
	m_strError += strError;
	return false;
}


//************************************
// Method:    LoadFromTextFile
// FullName:  CSongManager::LoadFromTextFile
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: bool bOverwriteExistingData
//************************************
bool CSongManager::InitSongsFromTextFile (CString strTextFile, EFileFormat eFileFormat, bool bOverwriteExistingData /*= false*/)
{
	if (NULL == m_pDB)
		return false;

	if (eFileFormat != EFileFormat::eM3U)
		return SetError (L"Currently we only support m3u files");

	try
	{
		m_pDB->execDML (L"begin transaction");

		//
		//  Phew.  Do we only want to support playlist format?  I think let's also support
		//  a tab delimited format.

		CFileException	oFileExcept;
		CStdioFileExPtr	pFileIn	= CStdioFileExPtr (new CStdioFileEx);
		if (! pFileIn->Open (strTextFile, CFile::modeRead, &oFileExcept))
			return SetError (CUtils::GetErrorMessageFromException (&oFileExcept));

		CString strInsert, strSongName, strPathToMp3;
		strInsert.Format (L"insert into %s (%s, %s, %s) values (null, ?, ?)", TBL_SONGS, DB_COL_SONG_ID, DB_COL_SONG_NAME, DB_COL_PATH_TO_MP3);
		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strInsert);

		while (ReadNextSongM3U (*pFileIn, strSongName, strPathToMp3))
		{
			//
			//  Add to our database.  null for the ID makes it auto-choose, though not technically "autoincrement"

			stmtQuery.bind (1, strSongName);
			stmtQuery.bind (2, strPathToMp3);
			stmtQuery.execDML ();

		} // end file read loop

		m_pDB->execDML (L"commit transaction");
		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		m_pDB->execDML (L"rollback transaction");
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		m_pDB->execDML (L"rollback transaction");
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end init songs from text file





//************************************
// Method:    DeleteAllSongs
// FullName:  CSongManager::DeleteAllSongs
// Access:    public 
// Returns:   bool
// Qualifier:
//************************************
bool CSongManager::DeleteAllSongs()
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strDelete;
		strDelete.Format (L"delete from %s", TBL_SONGS);
		m_pDB->execDML (strDelete);

		strDelete.Format (L"delete from %s", TBL_GAME_HISTORY);
		m_pDB->execDML (strDelete);

		strDelete.Format (L"delete from %s", TBL_SONG_PODS);
		m_pDB->execDML (strDelete);

		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end delete all songs



//************************************
// Method:    GetWonLossRecord
// FullName:  CSongManager::GetWonLossRecord
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nLastID
// Parameter: int & rnWins
// Parameter: int & rnLosses
//
//
//
//************************************
bool CSongManager::GetWonLossRecord (int nSongID, int& rnWins, int& rnLosses)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strQueryWins, strQueryLosses;
		strQueryWins.Format (L"SELECT count(*) FROM %s where (%s=%d and %s > 0) or (%s=%d and %s < 0)", TBL_GAME_HISTORY, 
			DB_COL_SONG_1_ID, nSongID, DB_COL_GAME_SCORE_MARGIN, DB_COL_SONG_2_ID, nSongID, DB_COL_GAME_SCORE_MARGIN);
		strQueryLosses.Format (L"SELECT count(*) FROM %s where (%s=%d and %s < 0) or (%s=%d and %s > 0)", TBL_GAME_HISTORY, 
			DB_COL_SONG_1_ID, nSongID, DB_COL_GAME_SCORE_MARGIN, DB_COL_SONG_2_ID, nSongID, DB_COL_GAME_SCORE_MARGIN);
	
		rnWins		= m_pDB->execScalar (strQueryWins);
		rnLosses	= m_pDB->execScalar (strQueryLosses);

		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e){
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::GetWonLossRecord







//************************************
// Method:    ReadNextSongM3U
// FullName:  CSongManager::ReadNextSongM3U
// Access:    protected 
// Returns:   bool
// Qualifier:
// Parameter: CStdioFile & roFileIn
// Parameter: CString & rstrSongName
// Parameter: CString & rstrPathToMp3
//************************************
bool CSongManager::ReadNextSongM3U (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3)
{
	rstrSongName.Empty ();
	rstrPathToMp3.Empty ();

	static CString strExtM3u	= L"extm3u";
	static CString strExtInf	= L"#extinf:";


	//
	//  Reads lines until we get to a line that doesn't start with a #

	CString strLine, strLineLower;
	while (roFileIn.ReadString (strLine))
	{
		strLine.Trim ();

		strLineLower = strLine;
		strLineLower.MakeLower ();

		int nTagStart = strLineLower.Find (strExtInf);
		if (0 == nTagStart)
		{
			//
			// EXTINF has length of song in seconds followed by the display title

			int nComma = strLine.Find (',', strExtInf.GetLength ());
			if (-1 == nComma)
				continue;

			rstrSongName = strLine.Mid (nComma + 1);
			rstrSongName.Trim ();

		} // end if is ExtInf line

		else if (strLineLower.Find ('#') == 0)
		{
			//
			// Unhandled ext m3u directive  

		}
		else
		{
			//
			//  Presumably the path to our file

			rstrPathToMp3 = strLine;
			rstrPathToMp3.Trim ();
			if (rstrSongName.IsEmpty ())
				rstrSongName = CUtils::GetFileNameFromPath (rstrPathToMp3, true);

			//
			//  See if we can turn this into a real full path, if it's not...  winamp doesn't have a drive letter
			//  on there, for instance

			CUtils::FindFile (rstrPathToMp3);
			return true; // All done.
		}
	} // end read line loop

	//
	//  If we didn't find the path to a file, it's no good...

	return false;

} // end ReadNextSongM3U



//************************************
// Method:    ReadnextSongTab
// FullName:  CSongManager::ReadnextSongTab
// Access:    protected 
// Returns:   bool
// Qualifier:
// Parameter: CStdioFile & roFileIn
// Parameter: CString & rstrSongName
// Parameter: CString & rstrPathToMp3
//************************************
bool CSongManager::ReadnextSongTab (CStdioFileEx& roFileIn, CString& rstrSongName, CString& rstrPathToMp3)
{
	return SetError (L"Tab Delimited files not yet supported");

} // end ReadnextSongTab




//************************************
// Method:    GetSongCount
// FullName:  CSongManager::GetSongCount
// Access:    public 
// Returns:   int
// Qualifier:
//************************************
bool CSongManager::GetSongCount (int& rnSongCount)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select count(*) from %s", TBL_SONGS);
		rnSongCount = m_pDB->execScalar (strQuery);
		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end get song count






//************************************
// Method:    GetNextSong
// FullName:  CSongManager::GetNextSong
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CString & rstrSongName
// Parameter: CString & rstrPathToMp3
// Parameter: int & rnSongID
// Parameter: int nPrevSongID
//************************************
bool CSongManager::GetNextSong(CString& rstrSongName, CString& rstrPathToMp3, int& rnSongID, int nPrevSongID /*= -1*/)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select * from %s where %s > %d order by %s limit 1", TBL_SONGS, DB_COL_SONG_ID, nPrevSongID, DB_COL_SONG_ID);
	
		CppSQLite3Query query = m_pDB->execQuery (strQuery);
		if (query.eof ())
			return false;

		rstrSongName	= query.getStringField	(DB_COL_SONG_NAME);
		rstrPathToMp3	= query.getStringField	(DB_COL_PATH_TO_MP3);
		rnSongID		= query.getIntField		(DB_COL_SONG_ID);
		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end get next song



//************************************
// Method:    GetSongDetails
// FullName:  CSongManager::GetSongDetails
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSongID
// Parameter: CString & rstrSongName
// Parameter: CString & rstrPathToMp3
//
//
//
//************************************
bool CSongManager::GetSongDetails (int nSongID, CString& rstrSongName, CString& rstrPathToMp3)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select * from %s where %s=%d", TBL_SONGS, DB_COL_SONG_ID, nSongID);
	
		CppSQLite3Query query = m_pDB->execQuery (strQuery);
		if (query.eof ())
			return false;

		rstrSongName	= query.getStringField	(DB_COL_SONG_NAME);
		rstrPathToMp3	= query.getStringField	(DB_COL_PATH_TO_MP3);
		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}

} // end CSongManager::GetSongDetails



//************************************
// Method:    GetAllSongsInRandomOrder
// FullName:  CSongManager::GetAllSongsInRandomOrder
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CIntArray & rarrSongIDs
//************************************
bool CSongManager::GetAllSongsInRandomOrder (CIntArray& rarrSongIDs)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		rarrSongIDs.SetSize (0);

		CString strQuery;
		strQuery.Format (L"select %s from %s order by random()", DB_COL_SONG_ID, TBL_SONGS);
	
		CppSQLite3Query oQuery = m_pDB->execQuery (strQuery);
		for (; !oQuery.eof (); oQuery.nextRow ())
		{
			rarrSongIDs.Add (oQuery.getIntField (0));
		}

		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}

} // end CSongManager::GetAllSongsInRandomOrder



//************************************
// Method:    SetSongName
// FullName:  CSongManager::SetSongName
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
// Parameter: CString strName
//************************************
bool CSongManager::SetSongName (int nSongID, CString strSongName)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strInsert;
		strInsert.Format (L"update %s set %s=? where %s=?", TBL_SONGS, 
			DB_COL_SONG_NAME, DB_COL_SONG_ID);

		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strInsert);

		stmtQuery.bind (1, strSongName);
		stmtQuery.bind (2, nSongID);
		stmtQuery.execDML ();
		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::SetSongName




//************************************
// Method:    SetSongPathToMp3
// FullName:  CSongManager::SetSongPathToMp3
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
// Parameter: CString strPathtoMp3
//************************************
bool CSongManager::SetSongPathToMp3 (int nSongID, CString strPathtoMp3)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strInsert;
		strInsert.Format (L"update %s set %s=? where %s=?", TBL_SONGS, 
			DB_COL_PATH_TO_MP3, DB_COL_SONG_ID);

		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strInsert);

		stmtQuery.bind (1, strPathtoMp3);
		stmtQuery.bind (2, nSongID);
		stmtQuery.execDML ();
		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::SetSongPathToMp3



//************************************
// Method:    GetUnfinishedPoolCount
// FullName:  CSongManager::GetUnfinishedPoolCount
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int & rnUnfinishedPoolCount
//************************************
bool CSongManager::GetUnfinishedPodCount (int& rnUnfinishedPoolCount)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select count(*) from %s where %s=0", TBL_SONG_PODS, DB_COL_POOL_FINISHED);
		rnUnfinishedPoolCount = m_pDB->execScalar (strQuery);
		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::GetUnfinishedPoolCount





//************************************
// Method:    GetUnfinishedPoolCount
// FullName:  CSongManager::GetUnfinishedPoolCount
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int & rnUnfinishedPoolCount
//************************************
bool CSongManager::GetCurrentPod (CIntArray& rarrSongIDs)
{
	rarrSongIDs.SetSize (0);
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select * from %s where %s=0 limit 1", TBL_SONG_PODS, DB_COL_POOL_FINISHED);
		CppSQLite3Query oQuery = m_pDB->execQuery (strQuery);
		
		if (oQuery.eof ())
			return false;

		for (int i = 0; i < m_nPoolSize; i ++)
			rarrSongIDs.Add (oQuery.getIntField (1 + i));	// first col is the pod ID, skip that... otherwise just count so we'll support if we allow different pod sizes later.

		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::GetUnfinishedPoolCount



//************************************
// Method:    SetPodRankings
// FullName:  CSongManager::SetPodRankings
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CIntArray & rarrSongIDs
//************************************
bool CSongManager::SetPodRankings (CIntArray& rarrSongIDs)
{
	return false;

} // end CSongManager::SetPodRankings





//************************************
// Method:    ScheduleMoreGames
// FullName:  CSongManager::ScheduleMoreGames
// Access:    public 
// Returns:   bool
// Qualifier:
//************************************
bool CSongManager::ScheduleMorePods ()
{
	//
	//  If there are still unfinished pools, don't schedule more games yet

	if (NULL == m_pDB)
		return false;

	int nUnfinishedPools = 0;
	if (!GetUnfinishedPodCount (nUnfinishedPools) || (nUnfinishedPools > 0))
		return false;

	try
	{
		//
		//  Ok, let's grab all the songs, then put them into buckets of 5 each.
		//  At first, that'll be it.  If we have a bucket at the end with < 5 songs,
		//  we'll go fill it out with songs we've already used.
		//
		//  Later, we'll look at minimizing rematches.  But for now...

		CIntArray	arrSongIDs;
		if (!GetAllSongsInRandomOrder (arrSongIDs))
			return false;

		m_pDB->execDML (L"begin transaction");

		int nSongCount = (int) arrSongIDs.GetSize ();
		if (nSongCount < m_nPoolSize)
			m_nPoolSize = nSongCount;

		//
		//  Sweet!  They're randomized.  Just pull them out in order...  and since they're random, if
		//  we need extras we can just pull from the front.

		int nPools	= nSongCount / m_nPoolSize;
		int nExtras = nSongCount % m_nPoolSize;

		CString strQuery;
		int		nSongIndex = 0;
		for (int nPoolIndex = 0; nPoolIndex < nPools; nPoolIndex++)
		{
			strQuery.Format (L"insert into %s values (null, ", TBL_SONG_PODS);	// null forces auto-increment for DB_COL_POD_ID
			for (int i = 0; i < m_nPoolSize; i++, nSongIndex++)
			{
				if (i > 0)
					strQuery += L", ";
				strQuery += CUtils::NumberToString (arrSongIDs[nSongIndex]);
			}

			strQuery += L", 0);";

			m_pDB->execDML (strQuery);
		} // end loop to store pools in DB

		//
		//  Now any extras

		if (nExtras > 0)
		{
			strQuery.Format (L"insert into %s values (null, ", TBL_SONG_PODS);
			for (int i = 0; i < m_nPoolSize; i++, nSongIndex++)
			{
				if (i > 0)
					strQuery += L", ";

				if (nSongIndex < nSongCount)
					strQuery += CUtils::NumberToString (arrSongIDs[nSongIndex]);
				else
				{
					//
					//  Ideally, we'll take the first song from the previously scheduled pods to fill this out.
					//  If there aren't enough songs available, we'll use the 2nd, then 3rd, etc.

					int nUseThisSongIndex = (nSongIndex % nSongCount) + ((i - 1) * m_nPoolSize);
					if (nUseThisSongIndex > nSongCount)
						nUseThisSongIndex = (nSongIndex % nSongCount);

					//
					//  If they don't have at least m_nPoolSize songs, well, this program isn't going
					//  to work that well for them.  So don't worry about it.  Just duplicate this song.

					if (nUseThisSongIndex > nSongCount)
						nUseThisSongIndex = 0;

					strQuery += CUtils::NumberToString (arrSongIDs[nUseThisSongIndex]);	//  So, wraps around and gives us indexes 0, 1, 2, etc
				}
			}

			strQuery += L", 0);";
			m_pDB->execDML (strQuery);
			m_pDB->execDML (L"commit transaction");

		} // end if we have an extra pool to finish up

		return true;
	}
	catch (CppSQLite3Exception& e) {
		m_pDB->execDML (L"rollback transaction");
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		m_pDB->execDML (L"rollback transaction");
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::ScheduleMoreGames








