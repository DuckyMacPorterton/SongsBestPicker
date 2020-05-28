#include "stdafx.h"
#include "SongManager.h"
#include "SQLite/CppSQLite3-Unicode.h"
#include "Utils.h"
#include "Song.h"


CSongManager::CSongManager ()
{

}


CSongManager::~CSongManager ()
{

}



//************************************
// Method:    LoadFromTextFile
// FullName:  CSongManager::LoadFromTextFile
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: bool bOverwriteExistingData
//************************************
bool CSongManager::LoadFromTextFile(bool bOverwriteExistingData /*= false*/)
{

	return false;
}



//************************************
// Method:    LoadFromDB
// FullName:  CSongManager::LoadFromDB
// Access:    protected 
// Returns:   bool
// Qualifier:
//************************************
bool CSongManager::LoadFromDB ()
{
	try
	{
		//
		//  Simple.  We have 2 tables.  

		m_pDB = CMyCppSQLite3DBPtr (new CMyCppSQLite3DB);
		
		CString strDataFile;
		strDataFile.Format (L"%sSongsBestPicker.db", CUtils::GetProgramFilesDir ());
		
		m_pDB->open (strDataFile);	//  throws exception on error

		CString			strQuery	= L"select * from " + CString(TBL_SONGS);
		CppSQLite3Query query		= m_pDB->execQuery(strQuery);

		//
		//  First load all the songs

		for (; !query.eof(); query.nextRow())
		{
			UINT	nSongID			= query.getIntField		(L"ID");
			CString	strSongName		= query.getStringField	(L"Name");
			CString	strPathToMp3	= query.getStringField	(L"PathToMp3");

			CSongPtr pNewSong = CSongPtr (new CSong (strSongName, strPathToMp3, nSongID));
			m_mapIdToSong[nSongID] = pNewSong;

		} // end loop to load song names

		//
		//  Then scan through the game history to grab our past results

		strQuery	= L"select * from " + CString(TBL_GAME_HISTORY);
		query		= m_pDB->execQuery (strQuery);

		//
		//  First load all the songs

		for (; !query.eof(); query.nextRow())
		{
			UINT	nSong1ID		= query.getIntField		(COL_SONG_1_ID);
			UINT	nSong2ID		= query.getIntField		(COL_SONG_2_ID);
			int		nGameMargin		= query.getIntField		(COL_GAME_SCORE_MARGIN);

			m_mapIdToSong[nSong1ID]->AddGameResult (nGameMargin);
			m_mapIdToSong[nSong2ID]->AddGameResult (-1 * nGameMargin);	//  Game margins always relative to team 1 in the table, so the opposite must've happened to game 1

		} // end loop to load song names

	}
	catch (CppSQLite3Exception& e)
	{
		//
		//  Get the error message before deleting m_pDB because if it throws 
		//  an exception while in there it overwrites our error

//		int nErrorCode = e.errorCode ();
		return SetError (e.errorMessage ());
	}
	catch (CException* e)
	{
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}

} // end load from db





//************************************
// Method:    SaveToDB
// FullName:  CSongManager::SaveToDB
// Access:    public 
// Returns:   bool
// Qualifier:
//************************************
bool CSongManager::SaveToDB()
{
	return false;

}







//************************************
// Method:    LoadGameHistory
// FullName:  CSong::LoadGameHistory
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CMyCppSQLite3DBPtr pDB
//************************************
bool CSongManager::LoadGamesFromDB (CMyCppSQLite3DBPtr pDB, CString &rstrError)
{
	try
	{
		CString strQuery;
		strQuery.Format (L"select * from %s where %s=%d or %s=%d", TBL_GAME_HISTORY, COL_SONG_1_ID, COL_SONG_2_ID);
		CppSQLite3Query query = pDB->execQuery (strQuery);

		for (; !query.eof(); query.nextRow())
		{
			UINT nSong1Id = query.getIntField			(COL_SONG_1_ID);
			UINT nSong2Id = query.getIntField			(COL_SONG_2_ID);

			//
			//  Game Score margin is always relative to team 1

			int	nGameScoreMargin = query.getIntField	(COL_GAME_SCORE_MARGIN);


		}
	}
	catch (CppSQLite3Exception& e)
	{
		//
		//  Get the error message before deleting m_pDB because if it throws 
		//  an exception while in there it overwrites our error

//		int nErrorCode = e.errorCode ();
		rstrError = e.errorMessage ();
		return false;
	}
	catch (CException* e)
	{
		rstrError = CUtils::GetErrorMessageFromException (e, true);
		return false;
	}


} // end load game history








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

	return false;
}


