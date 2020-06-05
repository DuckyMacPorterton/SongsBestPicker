#include "stdafx.h"
#include "SongManager.h"
#include "SQLite/CppSQLite3-Unicode.h"
#include "Utils.h"
#include "Song.h"
#include <fmod_errors.h>

#define ELO_DEFAULT_K	32


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

	CWaitCursor wc;

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

		CString strInsert, strSongName, strPathToMp3, strArtist, strTitle, strAlbum;
		strInsert.Format (L"insert into %s (%s, %s, %s, %s, %s) values (null, ?, ?, ?, ?)", TBL_SONGS, 
			DB_COL_SONG_ID, DB_COL_PATH_TO_MP3, DB_COL_SONG_ARTIST, DB_COL_SONG_TITLE, DB_COL_SONG_ALBUM);
		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strInsert);

		while (ReadNextSongM3U (*pFileIn, strSongName, strPathToMp3))
		{
			//
			//  See if we can read any meta tags from the mp3.  If not, we'll use the
			//  notes from the m3u file as our title

			LoadTagsFromMp3 (strPathToMp3, strTitle, strArtist, strAlbum);
			if (strTitle.IsEmpty ())
				strTitle = strSongName; 

			//
			//  Add to our database.  null for the ID makes it auto-choose, though not technically "autoincrement"

			stmtQuery.bind (1, strPathToMp3);
			stmtQuery.bind (2, strArtist);
			stmtQuery.bind (3, strTitle);
			stmtQuery.bind (4, strAlbum);
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
// Method:    GuessMoreLikelyToBeReal
// FullName:  CSongManager::GuessMoreLikelyToBeReal
// Access:    protected 
// Returns:   CString
// Qualifier:
// Parameter: CString strOne
// Parameter: CString strTwo
//
//  When we load tags from files, some are good and some are gibberish.
//  This tries to figure which is most likely to be good.
//
//************************************
CString CSongManager::GuessMoreLikelyToBeReal (CString strOne, CString strTwo)
{
	if (strOne.IsEmpty ())
		return strTwo;
	if (strTwo.IsEmpty ())
		return strOne;

	//
	//  Otherwise, let's return the one with the least junk

	int nJunkCount1 = 0;

	for (int i = 0; i < strOne.GetLength (); i ++)
	{
		if (! iswprint (strOne[i])) 
			nJunkCount1 ++;
	}

	int nJunkCount2 = 0;
	for (int i = 0; i < strTwo.GetLength (); i ++)
	{
		if (! iswprint (strTwo[i])) 
			nJunkCount2 ++;
	}

	if (nJunkCount1 > nJunkCount2)
		return strTwo;
	return strOne;

} // end CSongManager::GuessMoreLikelyToBeReal



//************************************
// Method:    LoadTagsFromMp3
// FullName:  CSongManager::LoadTagsFromMp3
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CString strPathToMp3
// Parameter: CString & rstrTitle
// Parameter: CString & rstrArtist
// Parameter: CString & rstrAlbum
//************************************
bool CSongManager::LoadTagsFromMp3 (CString strPathToMp3, CString& rstrTitle, CString& rstrArtist, CString& rstrAlbum)
{
	rstrTitle.Empty ();
	rstrArtist.Empty ();
	rstrAlbum.Empty ();

	FMOD::Sound* pSoundToLoadTags = NULL;

	FMOD_RESULT result = m_pFmodSystem->createSound (CUtils::UTF16toUTF8 (strPathToMp3), FMOD_OPENONLY, 0, &pSoundToLoadTags);	// FMOD_DEFAULT
	if (result != FMOD_OK) {
		TRACE (L"LoadTagsFromMp3: Error loading song: " + CUtils::UTF8toUTF16 (FMOD_ErrorString (result)) + L" / " + strPathToMp3);
		return false;
	}

	//
	//  First get the artist

	CStringArray arrTagNames;
	if (GetTagNamesForType (L"Artist", arrTagNames))
	{
		for (int i = 0; i < arrTagNames.GetSize (); i ++)
		{
			CString strTemp;
			ReadSingleTag (pSoundToLoadTags, arrTagNames[i], strTemp);
			rstrArtist = GuessMoreLikelyToBeReal (rstrArtist, strTemp);
		}
	}

	rstrArtist.Trim ();

	//
	//  Then the title

	if (GetTagNamesForType (L"Title", arrTagNames))
	{
		for (int i = 0; i < arrTagNames.GetSize (); i ++)
		{
			CString strTemp;
			ReadSingleTag (pSoundToLoadTags, arrTagNames[i], strTemp);
			rstrTitle = GuessMoreLikelyToBeReal (rstrTitle, strTemp);
		}
	}

	rstrTitle.Trim ();

	//
	//  And I guess we'll get the album

	if (GetTagNamesForType (L"Album", arrTagNames))
	{
		for (int i = 0; i < arrTagNames.GetSize (); i ++)
		{
			CString strTemp;
			ReadSingleTag (pSoundToLoadTags, arrTagNames[i], strTemp);
			rstrAlbum = GuessMoreLikelyToBeReal (rstrAlbum, strTemp);
		}
	}

	rstrAlbum.Trim ();

	pSoundToLoadTags->release ();
	return true;

#ifdef LoopThroughAllTags
	int nSongTags = 0, nSongTagsUpdatedSinceLastCall = 0;;
	pSoundToLoadTags->getNumTags (&nSongTags, &nSongTagsUpdatedSinceLastCall);

	for (int i = 0; i < nSongTags; i++)
	{
		FMOD_TAG oTag;
		auto result = pSoundToLoadTags->getTag (NULL, i, &oTag);
		if (result != FMOD_OK)
		{
			TRACE (L"%s\n", CUtils::UTF8toUTF16 (FMOD_ErrorString (result)));
			break;
		}

		if (FMOD_TAGTYPE_ID3V2 != oTag.type && FMOD_TAGTYPE_ID3V1 != oTag.type)
			continue;

		CStringA strTagData;
		char* pBuffer = strTagData.GetBuffer (oTag.datalen);
		memcpy (pBuffer, oTag.data, oTag.datalen);
		strTagData.ReleaseBuffer ();

		TRACE (L"Tag %d: %s: %s\n", i, CUtils::UTF8toUTF16 (oTag.name), CUtils::UTF8toUTF16 (strTagData));
	}
#endif

} // end CSongManager::LoadTagsFromMp3




//************************************
// Method:    ReadSingleTag
// FullName:  CSongManager::ReadSingleTag
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: FMOD::Sound * pSoundToLoadTags
// Parameter: CString strTagName
// Parameter: CStrin & rstrValue
//************************************
bool CSongManager::ReadSingleTag (FMOD::Sound* pSoundToLoadTags, CString strTagName, CString& rstrValue)
{
	if (NULL == pSoundToLoadTags)
		return false;

	//
	//  Turns out this is case sensitive if we call getTag ("tagname").
	//  I don't want that.  So we'll loop and do the case insensitiving ourselves

	CStringA strTagNameA = CUtils::UTF16toUTF8 (strTagName);

	int nSongTags = 0, nSongTagsUpdatedSinceLastCall = 0;;
	pSoundToLoadTags->getNumTags (&nSongTags, &nSongTagsUpdatedSinceLastCall);

	for (int i = 0; i < nSongTags; i++)
	{
		FMOD_TAG oTag;
		auto result = pSoundToLoadTags->getTag (NULL, i, &oTag);
		if (result != FMOD_OK)
			continue;

		if (strTagNameA.CompareNoCase (oTag.name) == 0)
		{
			CStringA strTagData;
			char* pBuffer = strTagData.GetBuffer (oTag.datalen);
			memcpy (pBuffer, oTag.data, oTag.datalen);
			strTagData.ReleaseBuffer ();
			rstrValue = CUtils::UTF8toUTF16 (strTagData);
			return true;
		}
	}

	return false;

} // end CSongManager::ReadSingleTag



//************************************
// Method:    GetTagNamesForType
// FullName:  CSongManager::GetTagNamesForType
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: CString strTagType
// Parameter: CStingArray & rarrTagNames
//
//  So, for tag type, "Artist" (<--- the name I would understand), returns
//  an array of things that info might be tagged in the mp3.  So, Artist might be:
//    Artist, or TP1, or TPE2, or whatever
//
//************************************
bool CSongManager::GetTagNamesForType (CString strTagType, CStringArray& rarrTagNames)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		rarrTagNames.SetSize (0);

		CString strQuery;
		strQuery.Format (L"select %s from %s where %s=?", DB_COL_TAG_NAME, TBL_MP3_TAGS, DB_COL_TAG_TYPE);

		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strQuery);

		stmtQuery.bind (1, strTagType);

		CppSQLite3Query oQuery = stmtQuery.execQuery ();
		for (; !oQuery.eof (); oQuery.nextRow ())
		{
			rarrTagNames.Add (oQuery.getStringField (0));
		}

		//
		//  If we don't have anything, let's have some defaults

		if (rarrTagNames.GetSize () == 0)
		{
			if (strTagType.CompareNoCase (L"Artist") == 0)
			{
				rarrTagNames.Add (L"Artist");
				rarrTagNames.Add (L"TP1");
				rarrTagNames.Add (L"TPE1");
				rarrTagNames.Add (L"TPE2");
				rarrTagNames.Add (L"Author");
				rarrTagNames.Add (L"WM/AlbumArtist");
			}
			else if (strTagType.CompareNoCase (L"Title") == 0)
			{
				rarrTagNames.Add (L"Title");
				rarrTagNames.Add (L"TIT2");
				rarrTagNames.Add (L"TT2");
				rarrTagNames.Add (L"WM/AlbumTitle");
			}
			else if (strTagType.CompareNoCase (L"Album") == 0)
			{
				rarrTagNames.Add (L"Album");
				rarrTagNames.Add (L"TAL");
				rarrTagNames.Add (L"TALB");
				rarrTagNames.Add (L"WM/AlbumTitle");
			}
		}

		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::GetTagNamesForType



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

		strDelete.Format (L"delete from %s", TBL_SONG_HEAD_TO_HEAD);
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
		strQueryWins.Format (L"SELECT count(*) FROM %s where (%s=%d and %s > 0) or (%s=%d and %s < 0)", TBL_SONG_HEAD_TO_HEAD, 
			DB_COL_SONG_1_ID, nSongID, DB_COL_SCORE_MARGIN, DB_COL_SONG_2_ID, nSongID, DB_COL_SCORE_MARGIN);
		strQueryLosses.Format (L"SELECT count(*) FROM %s where (%s=%d and %s < 0) or (%s=%d and %s > 0)", TBL_SONG_HEAD_TO_HEAD, 
			DB_COL_SONG_1_ID, nSongID, DB_COL_SCORE_MARGIN, DB_COL_SONG_2_ID, nSongID, DB_COL_SCORE_MARGIN);
	
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
bool CSongManager::GetNextSong(CString& rstrSongTitle, CString& rstrSongArtist, CString& rstrSongAlbum, CString& rstrPathToMp3, int& rnSongID, int nPrevSongID /*= -1*/)
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

		rstrSongTitle	= query.getStringField	(DB_COL_SONG_TITLE);
		rstrSongArtist	= query.getStringField	(DB_COL_SONG_ARTIST);
		rstrSongAlbum	= query.getStringField	(DB_COL_SONG_ALBUM);
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
bool CSongManager::GetSongDetails (int nSongID, CString& rstrSongTitle, CString& rstrSongArtist, CString& rstrSongAlbum, CString& rstrPathToMp3)
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

		rstrSongTitle	= query.getStringField	(DB_COL_SONG_TITLE);
		rstrSongArtist	= query.getStringField	(DB_COL_SONG_ARTIST);
		rstrSongAlbum	= query.getStringField	(DB_COL_SONG_ALBUM);
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
// Method:    SetSongDetails
// FullName:  CSongManager::SetSongDetails
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSongID
// Parameter: CString strSongTitle
// Parameter: CString strSongArtist
// Parameter: CString strSongAlbum
// Parameter: CString strPathToMp3
//
//
//
//************************************
bool CSongManager::SetSongDetails (int nSongID, CString strSongTitle, CString strSongArtist, CString strSongAlbum, CString strPathToMp3)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strInsert;
		strInsert.Format (L"update %s set %s=?, %s=?, %s=? where %s=?", TBL_SONGS, 
			DB_COL_SONG_TITLE, DB_COL_SONG_ARTIST, DB_COL_SONG_ALBUM, DB_COL_SONG_ID);

		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strInsert);

		stmtQuery.bind (1, strSongTitle);
		stmtQuery.bind (2, strSongArtist);
		stmtQuery.bind (3, strSongAlbum);
		stmtQuery.bind (4, nSongID);
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
} // end CSongManager::SetSongDetails



//************************************
// Method:    GetSongRating
// FullName:  CSongManager::GetSongRating
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSongID
// Parameter: int & rnSongRating
//
//
//
//************************************
bool CSongManager::GetSongRating (int nSongID, int& rnSongRating)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strQuery;
		strQuery.Format (L"select %s from %s where %s=%d", DB_COL_SONG_RATING, TBL_SONGS, DB_COL_SONG_ID, nSongID);

		rnSongRating = m_pDB->execScalar (strQuery);
		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::GetSongRating



//************************************
// Method:    SetSongRating
// FullName:  CSongManager::SetSongRating
// Access:    public 
// Returns:   bool
// Qualifier:
// Parameter: int nSongID
// Parameter: int nSongRating
//
//
//
//************************************
bool CSongManager::SetSongRating (int nSongID, int nSongRating)
{
	if (NULL == m_pDB)
		return false;

	try
	{
		CString strInsert;
		strInsert.Format (L"update %s set %s=? where %s=?", TBL_SONGS, 
			DB_COL_SONG_RATING, DB_COL_SONG_ID);

		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strInsert);

		stmtQuery.bind (1, nSongRating);
		stmtQuery.bind (2, nSongID);
		stmtQuery.execDML ();
		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}
} // end CSongManager::SetSongRating



//************************************
// Method:    RecalcAllSongRatings
// FullName:  CSongManager::RecalcAllSongRatings
// Access:    public 
// Returns:   bool
// Qualifier:
//
//
//
//************************************
bool CSongManager::RecalcAllSongRatings ()
{
	if (NULL == m_pDB)
		return false;

	try
	{
		//
		//  First clear out all the old ratings

		CString strInsert;
		strInsert.Format (L"update %s set %s=%d", TBL_SONGS, DB_COL_SONG_RATING, 1000);
		m_pDB->execDML (strInsert);

		//
		//  Now just loop through all the games and run ELO on the results

		CString strQuery;
		strQuery.Format (L"select %s, %s, %s from %s", DB_COL_SONG_1_ID, DB_COL_SONG_2_ID, DB_COL_SCORE_MARGIN, TBL_SONG_HEAD_TO_HEAD);

		CppSQLite3Query oQuery = m_pDB->execQuery (strQuery);
		for (; !oQuery.eof (); oQuery.nextRow ())
		{
			int nWinningSong	= oQuery.getIntField (DB_COL_SONG_1_ID);
			int nLosingSong		= oQuery.getIntField (DB_COL_SONG_2_ID);
			int nMargin			= oQuery.getIntField (DB_COL_SCORE_MARGIN);

			int nWinnerRating = 0, nLoserRating = 0;
			if (GetSongRating (nWinningSong, nWinnerRating) && GetSongRating (nLosingSong, nLoserRating))
			{
				float fWinnerRating = (float)nWinnerRating;
				float fLoserRating	= (float)nLoserRating;
				CUtils::EloRating (fWinnerRating, fLoserRating, ELO_DEFAULT_K, nMargin, 0);

				SetSongRating (nWinningSong,	(int)fWinnerRating);
				SetSongRating (nLosingSong,		(int)fLoserRating);
			}
		} // end loop through all previous head to head song matchups

		return true;
	}
	catch (CppSQLite3Exception& e) {
		return SetError (e.errorMessage ());
	}
	catch (CException* e) {
		return SetError (CUtils::GetErrorMessageFromException (e, true));
	}

} // end CSongManager::RecalcAllSongRatings



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
bool CSongManager::GetCurrentPod (int& rnPodID, CIntArray& rarrSongIDs)
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

		rnPodID = oQuery.getIntField (DB_COL_POD_ID);

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
bool CSongManager::SetPodRankings (int nPodID, CIntArray& rarrSongIDs)
{
	if (NULL == m_pDB)
		return false;

	if (rarrSongIDs.GetSize () != 5)
	{
		static bool bShown = false;
		if (! bShown)
		{
			AfxMessageBox (L"Error: SetPodRankings currently only supports 5 song pods");
			bShown = true;
		}
		return false;
	}

	try
	{
		m_pDB->execDML (L"begin transaction");

		CString strInsert;
		strInsert.Format (L"update %s set %s=?, %s=?, %s=?, %s=?, %s=?, %s=1 where %s=?", TBL_SONG_PODS, 
			DB_COL_SONG_1_ID, DB_COL_SONG_2_ID, DB_COL_SONG_3_ID, DB_COL_SONG_4_ID, DB_COL_SONG_5_ID,
			DB_COL_POOL_FINISHED,
			DB_COL_POD_ID);

		CppSQLite3Statement stmtQuery = m_pDB->compileStatement (strInsert);

		stmtQuery.bind (1, rarrSongIDs[0]);
		stmtQuery.bind (2, rarrSongIDs[1]);
		stmtQuery.bind (3, rarrSongIDs[2]);
		stmtQuery.bind (4, rarrSongIDs[3]);
		stmtQuery.bind (5, rarrSongIDs[4]);
		stmtQuery.bind (6, nPodID);
		stmtQuery.execDML ();

		//
		//  Now add these results to our head to head results

		for (int nWinner = 0; nWinner < rarrSongIDs.GetSize (); nWinner ++)
		{
			for (int nLoser = nWinner + 1; nLoser < rarrSongIDs.GetSize (); nLoser ++)
			{
				strInsert.Format (L"insert into %s (%s, %s, %s, %s) values (%d, %d, %d, %d)",	TBL_SONG_HEAD_TO_HEAD,
					DB_COL_POD_ID,	DB_COL_SONG_1_ID,		DB_COL_SONG_2_ID,		DB_COL_SCORE_MARGIN,
					nPodID,			rarrSongIDs[nWinner],	rarrSongIDs[nLoser],	nLoser - nWinner);
				m_pDB->execDML (strInsert);

				//
				//  And a ratings update

				int nWinnerRating = 0, nLoserRating = 0;
				if (GetSongRating (nWinner, nWinnerRating) && GetSongRating (nLoser, nLoserRating))
				{
					float fWinnerRating = (float) nWinnerRating;
					float fLoserRating	= (float) nLoserRating;
					CUtils::EloRating (fWinnerRating, fLoserRating, ELO_DEFAULT_K, m_nPoolSize - nWinner, m_nPoolSize - nLoser);

					SetSongRating (nWinner, (int) fWinnerRating);
					SetSongRating (nLoser,	(int) fLoserRating);
				}
			}
		}

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

		if (arrSongIDs.GetSize () == 0)
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








