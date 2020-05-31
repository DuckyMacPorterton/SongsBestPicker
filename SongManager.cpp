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

		strDelete.Format (L"delete from %s", TBL_CURRENT_SCHEDULE);
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
	catch (CppSQLite3Exception& e)
	{
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
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
	catch (CppSQLite3Exception& e)
	{
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
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
	catch (CppSQLite3Exception& e)
	{
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}

} // end get next song



//************************************
// Method:    SetSongName
// FullName:  CSongManager::SetSongName
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nSongID
// Parameter: CString strName
//
//
//
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
//
//
//
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










