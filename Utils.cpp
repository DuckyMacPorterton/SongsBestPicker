#include "stdafx.h"
#include "Utils.h"
#include <direct.h>
#include "MultiMon/Monitors.h"
#include "MultiMon/MultiMonitor.h"
#include "ModuleVersion.h"


//////////////////////////////////////////////////////////////////////
//
//    E N S U R E   D   B   T A B L E S   E X I S T 
//
//  This makes sure that our SQLite database has the correct table
//  definitions for us to save our data.
//
bool CUtils::EnsureDBTablesExist (CString& rstrError, CMyCppSQLite3DBPtr pDB)
{
	//
	//  This checks all the tables that always exist.  There are some more that will
	//  be created specifically for a certain field (for instance we have a 
	//  VP_TBL_FIELD_ITEMS table for each field) that we don't worry about here.

	try
	{
		//
		//  This checks all of our nice, useful tables exist and have every
		//  column they're supposed to  have in them
		
		for (int i = 0; arrVpSqlTableDefinitions[i].m_strTableName != 0; i ++)
		{
			if (!pDB->tableExists (arrVpSqlTableDefinitions[i].m_strTableName))
			{
				if (!CUtils::CreateDBTable (rstrError, pDB, arrVpSqlTableDefinitions[i].m_strTableName, arrVpSqlTableDefinitions[i].m_pTableDef))
					return false;
			}
			
			if (!EnsureAllColsExist (rstrError, pDB, arrVpSqlTableDefinitions[i].m_strTableName, arrVpSqlTableDefinitions[i].m_pTableDef))
				return false;
		}

		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		rstrError = e.errorMessage ();
		return false;
	}

} // end ensure db tables exist







///////////////////////////////////////////////////////////////////////
//
//  C R E A T E   D B   T A B L E
//
//  Creates a database table from our table definition struct
//  Returns true on success, false on failure
//
bool CUtils::CreateDBTable (CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, TableDefinitionStruct TableInfo[])
{
	//
	//  We do the primary key separately in case we have a multiple field
	//  primary key

	CString strPrimaryKey = L", PRIMARY KEY (";
	CString strDML;
	strDML.Format (L"create table %s (", strTableName);

	int nNumPrimaryKeys = 0;

	CStringArray arrIndexesToCreate;

	int numFields = CUtils::GetNumFieldsInTable (TableInfo);
	for (int i = 0; i < numFields; i++)
	{
		CString strCol;
		if (i != 0)
			strDML += ", ";

		strCol.Format (L"%s %s", TableInfo[i].strColumnName, TableInfo[i].strDataType);

		if (TableInfo[i].nDefaultVal != VP_DEFAULT_UNUSED)
			strCol += L" DEFAULT " + NumberToString (TableInfo[i].nDefaultVal);

		if (CString (TableInfo[i].strDataType).CompareNoCase (L"TEXT") == 0)
			strCol += L" collate nocase ";

		//
		//  We write it this way because PRIMARY KEY is inherently unique so no need to check it for both.

		if (TableInfo[i].bPrimaryKey) {
			nNumPrimaryKeys++;

			strCol += L" NOT NULL";

			if (nNumPrimaryKeys > 1)
				strPrimaryKey += _T (", ");
			strPrimaryKey += TableInfo[i].strColumnName;
		}
		else if (S_INDEX_UNIQUE == TableInfo[i].nIndexType)
			strCol += L" NOT NULL UNIQUE";
		else if (S_INDEX_INDEX == TableInfo[i].nIndexType)
		{
			arrIndexesToCreate.Add (L"create index " + strTableName + TableInfo[i].strColumnName + L" on " + strTableName + L" (" + TableInfo[i].strColumnName + L");");
		}

		strDML += strCol;
	}

	//
	//  Now finish our statement and execute it

	strPrimaryKey += L")";

	if (nNumPrimaryKeys > 0)
		strDML += strPrimaryKey;

	strDML += L");";

	try {
		pDB->execDML (strDML);

		//
		//  Now create our indexes

		for (int i = 0; i < arrIndexesToCreate.GetSize (); i ++)
		{
			pDB->execDML (arrIndexesToCreate[i]);
		}

		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		rstrError = e.errorMessage ();
		return false;
	}
} // end create db table



//////////////////////////////////////////////////////////////////////
//
//    G E T   N U M   F I E L D S   I N   T A B L E 
//
//  Returns the number of fields in a VP SQL table definition.  
//  This relies on the last field being defined empty.
//
int CUtils::GetNumFieldsInTable (TableDefinitionStruct TableDef[])
{
	int numFields = 0;
	while (TableDef[numFields].strColumnName != NULL)
		numFields++;

	return numFields;

} // end GetNumFieldsInTable









//////////////////////////////////////////////////////////////////////
//
//    E N S U R E   A L L   C O L S   E X I S T 
//
//  Makes sure all the correct columns exist in strTableName
//
bool CUtils::EnsureAllColsExist (CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, TableDefinitionStruct TableInfo[])
{

	int numFields = CUtils::GetNumFieldsInTable (TableInfo);
	for (int i = 0; i < numFields; i++)
	{
		if (!EnsureColExists (rstrError, pDB, strTableName, TableInfo[i].strColumnName, TableInfo[i].strDataType, TableInfo[i].nDefaultVal))
			return false;
	}

	return true;

} // end EnsureAllColsExist



//////////////////////////////////////////////////////////////////////
//
//    E N S U R E   C O L   E X I S T S 
//
//  This makes sure the column strColumnName exists in strTableName.
//  This will NOT create the table if it doesn't exist!
//
//  If it doesn't exist it will create it with data type strDataType.
//  If it already exists with a different data type this will NOT 
//  modify that column.  Remember that SQLite doesn't care about the
//  data type.
//
//  Returns true on success, false on failure.
//
bool CUtils::EnsureColExists (CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, CString strColName, CString strDataType /* = _T("INTEGER") */, int nDefaultValue /* = VP_DEFAULT_UNUSED */)
{
	CString strStmt;
	try
	{
		if (!pDB->tableExists (strTableName))
			return false;

		if (DoesColExist (rstrError, pDB, strTableName, strColName))
			return true;

		//  
		//  Create the column...

		if (nDefaultValue == VP_DEFAULT_UNUSED)
			strStmt.Format (_T ("ALTER TABLE %s ADD COLUMN %s %s;"), strTableName, strColName, strDataType);
		else
			strStmt.Format (_T ("ALTER TABLE %s ADD COLUMN %s %s DEFAULT %d;"), strTableName, strColName, strDataType, nDefaultValue);

		pDB->execDML (strStmt);
		return true;
	}
	catch (CppSQLite3Exception& e)
	{
		rstrError = e.errorMessage ();
		return false;
	}
} // end EnsureColExists



//////////////////////////////////////////////////////////////////////
//
//    D O E S   C O L   E X I S T 
//
//  Returns true if strColName is a column in table strTableName,
//  false if not.
//
bool CUtils::DoesColExist (CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, CString strColName)
{
	CString strStmt;
	try
	{
		strStmt.Format (_T ("PRAGMA table_info ('%s');"), strTableName);
		CppSQLite3Query query = pDB->execQuery (strStmt);

		for (; !query.eof (); query.nextRow ())
		{
			CString strColInTable = query.getStringField (1);
			if (strColInTable == strColName)
				return true;
		}

		return false;
	}
	catch (CppSQLite3Exception& e)
	{
		rstrError = e.errorMessage ();
		return false;
	}

} // end DoesColExust




/* static */ bool CUtils::DoesTableExist (CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName)
{
	if (NULL == pDB)
		return false;

	try
	{
		return pDB->tableExists (strTableName);
	}
	catch (CppSQLite3Exception& e)
	{
		rstrError = e.errorMessage ();
		return false;
	}
} // end DoesTableExist






//////////////////////////////////////////////////////////////////////
//
//    G E T   E R R O R   M E S S A G E   F R O M   E X C E P T I O N 
//
//  Returns the error message in CException
//
CString	CUtils::GetErrorMessageFromException (CException* pE, bool bDeleteException /* = false */)
{
	if (NULL == pE)
		return L"";

	CString strError;

	int nMaxMsgLength = 512;
	TCHAR* pBuffer = strError.GetBuffer (nMaxMsgLength);
	pE->GetErrorMessage (pBuffer, nMaxMsgLength);
	strError.ReleaseBuffer ();

	if (bDeleteException)
		pE->Delete ();

	return strError;

} // end GetErrorMessageFromException






//////////////////////////////////////////////////////////////////////
//
//    G E T   P R O G R A M   F I L E S   D I R 
//
//
CString	CUtils::GetProgramFilesDir ()
{
	TCHAR toasPath[MAX_PATH];
	
	::GetModuleFileName(NULL, toasPath, MAX_PATH); // returns full path including filename
	
	int  ch = '\\';
	TCHAR *pdest;
	
	pdest = _tcsrchr (toasPath, ch );
	pdest[1] = (TCHAR) NULL;

	CString RootDir;
	RootDir = toasPath;

	return RootDir;

} // end GetProgramFilesDir










//////////////////////////////////////////////////////////////
//
//  Takes a full pathname or less and returns just the 
//  filename up to the first period.
//
//  So c:\program files\vantagepoint\vp.exe
//  Returns vp
//
//  bRemoveAllExtensions handles files w/ multiple extensions:
//  c:\path\to\file.my.old.ext
//
//  If bRemoveAllExtensions is true, it returns
//  file
//  If bRemoveAllExtensions is false it returns
//  file.my.old
//
CString CUtils::GetFileNameFromPath (CString strFilename, bool bRemoveAllExtensions /* = true */)
{
	//
	//  First get rid of the path up to the filename

	CString strNoPath = strFilename;
	int nLastBS = strFilename.ReverseFind ('\\');
	if (nLastBS != -1)
		strNoPath = strFilename.Mid (nLastBS + 1);

	if (! bRemoveAllExtensions)
		return strNoPath;

	//
	//  Now get rid of the extension

	int nLastDot = strNoPath.ReverseFind ('.');

	//
	//  Reverse find annoys me as it doesn't let me start anywhere except last

	for (int nChar = nLastDot - 1; nChar >= 0; nChar--)
	{
		if (strNoPath[nChar] == '.')
			nLastDot = nChar;
	}

	CString strNoExt = strNoPath;
	if (nLastDot != -1)
		strNoExt = strNoPath.Left (nLastDot);
	return strNoExt;


} // end remove path and extension from filename








////////////////////////////////////////////////////////////////////////////////////////////
//
//  GET IS CHAR INSIDE SUB EXPRESSION
//
//  For example, find out if a , is inside "'s, like in this:
//
//  "My dog, fred, is happy!", "String two", "String three"
//
//  If they ask about that comma between dog and fred (and pass in " as cExpressionStart and
//  cExpressionEnd), we return true.
//
//  bSupportQuotes is necessary but problematic (read:  a pain), but if we want to support having quotes
//  inside a quoted string, well...
//
//  Returns true if char at nCharIndex in strHaystack is inside a sub expression, false if not.
//
//
/* static */ bool CUtils::GetIsCharInsideSubExpression (CString strHaystack, int nCharIndex, CString strExpressionStart, CString strExpressionEnd, bool bSupportQuoting /* = false */, int nStartAtIndex /* = 0 */)
{
	//
	//  Count non-quoted ('s before nOpStartIndex
	//  and
	//  Count non-quoted )'s before nOpStartIndex
	//
	//  If they're not equal, we're in a sub query
	//
	//  Note that I decided not to support \quoting at the moment

	if (strExpressionStart == strExpressionEnd)
	{
		bool	bInsideSubExpression	= false;
		int		nNextCharIndex			= strHaystack.Find (strExpressionStart, nStartAtIndex);
		while ((nNextCharIndex < nCharIndex) && (nNextCharIndex != -1))
		{
			//
			//  If the char we found to start our expression (so, if looking to see if nCharIndex
			//  is inside "double quotes", this is talking about finding the first double quote) 
			//  is itself inside double quotes, don't count it

			if ((strExpressionStart == L"\"") || (! GetIsCharInsideSubExpression (strHaystack, nNextCharIndex, L"\"", L"\"", bSupportQuoting)))
			{
				//
				//  Is it quoted?  Annoyingly, we should probably check to see if THAT \ is quoted.   But not now.
				//  If it's \quoted, then it means the - ack.  I've massively confused myself.

				if (!bSupportQuoting || ((nNextCharIndex > 0) && (strHaystack[nNextCharIndex - 1] != '\\')))
				{
					bInsideSubExpression = !bInsideSubExpression;
				} 
			}

			nNextCharIndex	= strHaystack.Find (strExpressionStart, nNextCharIndex + 1);
		} // end loop through cExpressionStart chars in our string

		return bInsideSubExpression;

	} // end if cExpressionStart == cExpressionEnd

	else
	{
		//
		//  cExpressionStart and cExpressionEnd are different.

		int nLeftParen	= CountInstancesOfString (strHaystack, strExpressionStart,	nCharIndex, bSupportQuoting);
		int nRightParen = CountInstancesOfString (strHaystack, strExpressionEnd,	nCharIndex, bSupportQuoting);

		return (nLeftParen != nRightParen);
	}

} // end get is char inside sub expression




////////////////////////////////////////////////////////////////////
//
//  This goofy routine counts the number of times a string appears
//  in a string (or part of a string).   This routine is case sensitive
//
int CUtils::CountInstancesOfString (CString strBig, CString strLittle, int nCharsToCheck /*= -1*/, bool bSkipQuotedBsChars /* = false */)
{
	int i = -1;
	int nFound = 0;

	do
	{
		if ((-1 != nCharsToCheck) && (i >= nCharsToCheck))
			break;

		if ((i = strBig.Find (strLittle, i + 1)) != -1)
		{
			if ((-1 != nCharsToCheck) && (i >= nCharsToCheck))
				break;

			if (bSkipQuotedBsChars && (i > 0) && strBig[i-1] == '\\')
				continue;

			nFound ++;
		}
	} while (i != -1);

	return nFound;
} // end Count Instances of TCHAR








//////////////////////////////////////////////////////////////////////
//
//    U   T   F 1 6 T O   U   T   F 8
//
//
CStringA CUtils::UTF16toUTF8(const CStringW& utf16)
{
   CStringA utf8;
   int len = WideCharToMultiByte(CP_UTF8, 0, utf16, -1, NULL, 0, 0, 0);
   if (len>1)
   { 
      char *ptr = utf8.GetBuffer(len-1);
      if (ptr) WideCharToMultiByte(CP_UTF8, 0, utf16, -1, ptr, len, 0, 0);
      utf8.ReleaseBuffer();
   }
   return utf8;

} // end UTF16toUTF


CStringW CUtils::UTF8toUTF16(const CStringA& utf8)
{
   CStringW utf16;
   int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
   if (len>1)
   { 
      wchar_t *ptr = utf16.GetBuffer(len-1);
      if (ptr) MultiByteToWideChar(CP_UTF8, 0, utf8, -1, ptr, len);
      utf16.ReleaseBuffer();
   }
   return utf16;
}


//////////////////////////////////////////////////////////////////////
//
//  This is sort of a front end / manager for GetIsValidUtf8String.
//  It makes multiple calls to GetIsValidUtf8String to determine
//  the last valid byte.
//
/* static */ int CUtils::GetLastValidUtfCharacterByte (CStringA astrToTest)
{
	//
	//  Check what we think is the last character.
	//  UTF-8 characters are, at most, 4 bytes.

	for (int i = 1; i <= 4; i ++)
	{
		if (i > astrToTest.GetLength ())
			return -1;

		CStringA strSubString = astrToTest.Right (i);
		if (GetIsValidUtf8String (strSubString))
			return astrToTest.GetLength () - 1;
	}

	//
	//  If we got to here it means that we have a partial utf-8 character as our last
	//  character.  So back off until we find the end of a valid character
	//
	//  -2 because -1 would be the last character, which we just checked above.  We could
	//  have included that in this loop, but I find it more readable this way, and the 
	//  overwhelming majority of the time, we'll end in a valid char, which will be caught
	//  up top.

	for (int nStartChar = astrToTest.GetLength () - 2; nStartChar >= 0; nStartChar --)
	{
		//
		//  Does this loop look familiar to you?

		for (int i = 1; i <= 4; i++)
		{
			if (nStartChar - 1 + 1 < 0)
				return -1;

			CStringA strSubString = astrToTest.Mid (nStartChar - i + 1, i);
			if (GetIsValidUtf8String(strSubString))
			{
				return nStartChar;	//  Last valid byte is nStartChar, even if it took more than 1 byte to make this one valid
			}
		}
	} // end loop through start characters, walking backwards through the string

	//
	//  If we got here, we don't have any valid characters.

	return -1;

} // end get last valid utf character byte


//////////////////////////////////////////////////////////////////////
//
//  As you can probably guess by the variable names, Doug did not write
//  this.
//
/* static */ bool CUtils::GetIsValidUtf8String (CStringA astrToTest)
{
    int c,i,ix,n,j;
    for (i=0, ix = astrToTest.GetLength(); i < ix; i++)
    {
        c = (unsigned char) astrToTest[i];
        //if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
        if (0x00 <= c && c <= 0x7f) n=0; // 0bbbbbbb
        else if ((c & 0xE0) == 0xC0) n=1; // 110bbbbb
        else if ( c==0xed && i<(ix-1) && ((unsigned char)astrToTest[i+1] & 0xa0)==0xa0) return false; //U+d800 to U+dfff
        else if ((c & 0xF0) == 0xE0) n=2; // 1110bbbb
        else if ((c & 0xF8) == 0xF0) n=3; // 11110bbb
        //else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return false;
        for (j=0; j<n && i<ix; j++) { // n bytes matching 10bbbbbb follow ?
            if ((++i == ix) || (( (unsigned char)astrToTest[i] & 0xC0) != 0x80))
                return false;
        }
    }
    return true;

} // end get is valid utf 8 string






//////////////////////////////////////////////////////////////////////
//
//    D E T E C T   C O D E   P A G E   F O R   S T R I N G 
//
//
bool CUtils::DetectCodePageForString (char* pText, UINT& rnCodePage, UINT nDefaultCodePage /* = CP_UTF8 */, UINT nDefaultFlags /* = MLDETECTCP_8BIT */)
{
#ifdef AlwaysUtf8
	return 65001;
#endif

	static int nRecursionCounter = 0;
	CRecursionCounterAuto RecursionCounter (&nRecursionCounter);
	if (nRecursionCounter > 2)
		return false;

	bool				bRetVal = false;

#define DETECT_INFO_ARRAY_SIZE	8

	DetectEncodingInfo	arrInfo[DETECT_INFO_ARRAY_SIZE];
	memset (&arrInfo, 0, sizeof (DetectEncodingInfo) * DETECT_INFO_ARRAY_SIZE);

	//
	//  Need this as an int for the call to DetectInputCodepage

	int nDetectInfoArraySize = DETECT_INFO_ARRAY_SIZE;

	CoInitialize (NULL);
	 
	{ 
		CComPtr<IMultiLanguage2> MultiLanguage;
		HRESULT hRes = MultiLanguage.CoCreateInstance (CLSID_CMultiLanguage); // __uuidof (MultiLanguage2));
		if (SUCCEEDED (hRes))
		{
//			UINT nFlags = MLDETECTCP_8BIT; // MLDETECTCP_HTML; // MLDETECTCP_8BIT;
			int	nBytesInString = (int) strlen (pText);

			if (SUCCEEDED (MultiLanguage->DetectInputCodepage (nDefaultFlags, nDefaultCodePage, pText, &nBytesInString, &(arrInfo[0]), &nDetectInfoArraySize)))
			{
				for (int i = 0; i < nDetectInfoArraySize; i ++)
				{
					MIMECPINFO	CodePageInfo;

					if (SUCCEEDED (MultiLanguage->GetCodePageInfo (arrInfo[i].nCodePage, arrInfo[i].nLangID, &CodePageInfo)))
					{
//						TRACE (_T("Found code page %d: %s\n"), CodePageInfo.uiCodePage, CodePageInfo.wszDescription);

						//
						//  If this is not one of our standard, happy code pages, give it a shot with the HTML flag
						//  and see what it finds

						if (
							(CodePageInfo.uiCodePage != 65000) && 
							(CodePageInfo.uiCodePage != 65001) && 
							(CodePageInfo.uiCodePage != 20127) && 
							(CodePageInfo.uiCodePage != 1252)) 
						{
							//
							//  If we didn't find any standard code page that way, go back to using
							//  our the one we found using MLDETECTCP_8BIT

							DetectCodePageForString (pText, rnCodePage, CP_UTF8, MLDETECTCP_HTML);
							if ((rnCodePage != 65000) &&
								(rnCodePage != 65001) && 
								(rnCodePage != 20127) && 
								(rnCodePage != 1252)) 
								rnCodePage = CodePageInfo.uiCodePage;
						}
						else
							rnCodePage = CodePageInfo.uiCodePage;

						//
						//  Blatant hack -- don't allow it to be treated as UTF-7 (65000), that will
						//  break things for us.  We do not support UTF-7.  So force it to be UTF-8.
						//
						//  Also don't let it be US-ASCII.  In that case force it to be UTF-8 

						if (65000 == rnCodePage)
							rnCodePage = 65001;
						else if (20127 == rnCodePage)
							rnCodePage = 65001;

						bRetVal = true;
						break;
					}
				}
			}
		}
	} // end to kill MultiLanguage before CoUninitialize

	CoUninitialize ();

	return bRetVal;

} // end DetectCodePageForString




/***********************************************

  C H E C K   I F   F I L E   E X I S T S

  Checks to see if a file or directory exists

***********************************************/
bool CUtils::FileExists (CString strName)
{

  struct _stat64 FileStatus;

  if (_tstat64 (strName, &FileStatus) == 0)
    return true;
  else
    return false;
} // end if file exists




//************************************
// Method:    FindFile
// FullName:  CUtils::FindFile
// Access:    public static 
// Returns:   bool
// Qualifier:
// Parameter: CString & rstrPathToSong
//
//
//
//************************************
bool CUtils::FindFile (CString& rstrPathToSong)
{
	if (FileExists (rstrPathToSong))
		return true;

	//
	//  Otherwise, if their path doesn't have a drive letter, check each drive at that same path.   

	if (rstrPathToSong.Find (':') != -1)
		return false;

	bool	bFound			= false;
	int		nCurrentDrive	= _getdrive();

	for (int drive = 1; drive < 26; drive++)
	{
		if (_chdrive(drive) == 0)
		{
			CString strTestThisPath;
			strTestThisPath.Format(L"%c:%s", drive + 'A' - 1, rstrPathToSong);

			if (FileExists (strTestThisPath)) {
				bFound = true;
				rstrPathToSong = strTestThisPath;
				break;
			}
		}
	} // end loop through drives

	// Restore original drive.
	_chdrive (nCurrentDrive);

	return bFound;

} // end find file







//************************************
// Method:    GetHotkeyText
// FullName:  CUtils::GetHotkeyText
// Access:    public static 
// Returns:   CString
// Qualifier:
// Parameter: UINT nKey
// Parameter: UINT nModifiers
//************************************
CString CUtils::GetHotkeyText (UINT nKey, UINT nModifiers)
{
	CString str;
	if (nModifiers & MOD_CONTROL)
		str = "Ctrl+";
	if (nModifiers & MOD_ALT)
		str += "Alt+";
	if (nModifiers & MOD_SHIFT)
		str += "Shift+";

	str += CUtils::GetKeyName (nKey); // , nModifiers & MOD_EXTENDED);
	return str;

} // end CUtils::GetHotkeyText



//************************************
// Method:    NumberToString
// FullName:  CVPUtils::NumberToString
// Access:    public static 
// Returns:   CString
// Qualifier:
// Parameter: int nNumber
//
//  I've almost done this a billion times, never have.
//  Makes it clean on the caller end to swap out a pretty
//  print call where we just want the number non-pretty printed,
//  but as a string.
//
//************************************
/* static */ CString CUtils::NumberToString (int nNumberToPrint, bool bOutputAsHex /* = false */, int nPadZeroTotalDigitCount /* = 0 */)
{
	CString strPretty;

	if (bOutputAsHex)
	{
		if (nPadZeroTotalDigitCount > 0)
			strPretty.Format (L"%0*X", nPadZeroTotalDigitCount, nNumberToPrint);
		else
			strPretty.Format (L"%X", nNumberToPrint);
	}
	else
	{
		if (nPadZeroTotalDigitCount > 0)
			strPretty.Format (L"%0*d", nPadZeroTotalDigitCount, nNumberToPrint);
		else
			strPretty.Format (L"%d", nNumberToPrint);
	}

	return strPretty;

} // end number to string


//************************************
// Method:    NumberToString
// FullName:  CUtils::NumberToString
// Access:    public static 
// Returns:   CString
// Qualifier:
// Parameter: float fNumber
// Parameter: int nPadZeroTotalDigitCount
//
//
//
//************************************
CString CUtils::NumberToString (float fNumber, int nDigitsPastZero /* = 2 */, int nPadZeroTotalDigitCount /*= 0*/)
{
	CString strPretty;

	if (nPadZeroTotalDigitCount > 0)
		strPretty.Format (L"%0*.*f", nDigitsPastZero, nPadZeroTotalDigitCount, fNumber);
	else
		strPretty.Format (L"%.*f", nDigitsPastZero, fNumber);

	return strPretty;
} // end CUtils::NumberToString



//************************************
// Method:    FindNumberInArray
// FullName:  CUtils::FindNumberInArray
// Access:    public static 
// Returns:   -1 if not found, otherwise index
// Qualifier:
// Parameter: CIntArray & rarrNumbers
// Parameter: int nToFind
//************************************
int CUtils::FindNumberInArray (CIntArray& rarrNumbers, int nToFind)
{
	for (int i = 0; i < rarrNumbers.GetSize (); i ++)
	{
		if (nToFind == rarrNumbers[i])
			return i;
	}

	return -1;

} // end CUtils::FindNumberInArray



//************************************
// Method:    AreArraysEqual
// FullName:  CUtils::AreArraysEqual
// Access:    public static 
// Returns:   bool
// Qualifier:
// Parameter: CIntArray & rarrOne
// Parameter: CIntArray & rarrTwo
//************************************
bool CUtils::AreArraysEqual (CIntArray& rarrOne, CIntArray& rarrTwo)
{
	if (rarrOne.GetSize () != rarrTwo.GetSize ())
		return false;

	for (int i = 0; i < rarrOne.GetSize (); i ++)
	{
		if (rarrOne[i] != rarrTwo[i])
			return false;
	}

	return true;

} // end CUtils::AreArraysEqual



//////////////////////////////////////////////////////////////////////
//
//    M Y   A T O   I 
//
//  The problem here is that atoi doesn't tell you it it was
//  successful or not.  If it fails it returns 0, which is a 
//  perfectly valid return value if you send in "0".
//
//  strtol does do that, but it's ugly, so we clean it up
//  and make it simple.
//
bool CUtils::MyAtoI (CString str, int& rnValFound, bool bRequireMatchEntireString /* = true */)
{
	if (str.IsEmpty ())
		return false;

	TCHAR* pBuffer		= str.GetBuffer ();
	TCHAR* pFoundChar	= NULL;
	rnValFound = _tcstol (pBuffer, &pFoundChar, 10); // 0 to auto-determine if it's decimal or hex - breaks - leading 0 makes it act like octal
	
	//
	//  If pFoundChar is set and contains a NULL char, we've matched the whole thing.

	if ((NULL != pFoundChar) && (*pFoundChar == NULL))
		return true;		//  We actually matched something

	//
	//  If we matched something but not the whole string...

	if ((pFoundChar != pBuffer) && (!bRequireMatchEntireString))
		return true;

	//
	//  So we haven't matched what we need.  See if it's a hex number.

	rnValFound = _tcstol (pBuffer, &pFoundChar, 16); // 0 to auto-determine if it's decimal or hex - breaks - leading 0 makes it act like octal
	
	//
	//  If pFoundChar is set and contains a NULL char, we've matched the whole thing.

	if ((NULL != pFoundChar) && (*pFoundChar == NULL))
		return true;		//  We actually matched something

	//
	//  If we matched something but not the whole string...

	if ((pFoundChar != pBuffer) && (!bRequireMatchEntireString))
		return true;

	rnValFound = 0;
	return false;

} // end MyAtoI




//////////////////////////////////////////////////////////////////////////
//
//  The point of SleepMsg is that sleep will freeze the entire thread
//  so if it's called from a thread that owns a window then that's 
//  no good...  so this will sleep but let the message handling
//  continue.  I'm conflicted about it but there have been a couple
//  places where it worked and nothing else I could find did.
//
//  Timeout is in milliseconds
//
BOOL CUtils::SleepMsg (DWORD dwTimeoutMS)
{
#define MSGF_SLEEPMSG 0x5300

//	DWORD dwStart = GetTickCount();
	ULONGLONG nStart = GetTickCount64 ();

	ULONGLONG nElapsed = 0;
	while ((nElapsed = GetTickCount64 () - nStart) < dwTimeoutMS) {

		DWORD dwStatus = MsgWaitForMultipleObjectsEx(0, NULL,
			(DWORD) (dwTimeoutMS - nElapsed), QS_ALLINPUT,
			0 | MWMO_INPUTAVAILABLE);


		if (dwStatus == WAIT_OBJECT_0) {
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					PostQuitMessage((int)msg.wParam);
					return FALSE; // abandoned due to WM_QUIT
				}
				if (!CallMsgFilter(&msg, MSGF_SLEEPMSG)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		} // end if dwstatus == WAIT_OBJECT_)
	}

	return TRUE; // timed out
} // end sleep msg



CString CUtils::GetMciErrorString (MCIERROR nMciError)
{
	CString str;
	LPWSTR pstrError = str.GetBuffer (128);
	mciGetErrorString (nMciError, pstrError, 128);
	str.ReleaseBuffer ();

	return str;
} // end get mci error string











// Function to calculate the Probability 
/* static */ float CUtils::EloProbability (float rating1, float rating2) 
{ 
    return (float) (1.0 * 1.0 / (1 + 1.0 *  
           pow(10, 1.0 * ((double) rating1 - rating2) / 400)));
} 
  



// Function to calculate Elo rating 
// K is a constant. 
// d determines whether Player A wins or Player B.  
/* static */ void CUtils::EloRating (float& fRatingWinner, float& fRatingLoser, int K_arg, int nWinnerScore, int nLoserScore) 
{   
    //
    //  FiveThirtyEight has K=20 for NBA and similar for soccer + NFL
    //  So try 20 first
    //  Chess might use 32.  Or it might change based on ratings.

    //
    //  To add margin of victory to this:
	//
	//  I got this from: https://math.stackexchange.com/questions/850002/improving-the-elo-rating-system-to-account-for-game-results
	//
	// Margin of Victory Multiplier = LN(ABS(PD)+1) * (2.2/((ELOW-ELOL)*.001+2.2))

	float fK = (float) (K_arg * (log ((double) abs (nWinnerScore - nLoserScore) + 1) * (2.2 /(((double) fRatingWinner-fRatingLoser)*.001+2.2))));
	

#ifdef asdfasdf
    The margin of victory multiplier is calculated as follows.

    Take a team’s margin of victory, add 3 points and then take the result to the power of 0.8.
    Divide the result by the following formula: 7.5+.006*(elo_diff), where elo_diff represents the Elo rating difference between the teams, accounting for home-court advantage. Elo_diff should be negative in games won by the underdog.

============

It isn’t complicated in principle. You just have to discount the margin of victory more when favorites win and increase it when underdogs win. The formula for it is as follows:

Margin of Victory Multiplier = LN(ABS(PD)+1) * (2.2/((ELOW-ELOL)*.001+2.2))

Where PD is the point differential in the game, ELOW is the winning team’s Elo Rating before the game, and ELOL is the losing team’s Elo Rating before the game.

#endif

	//
	//  To calculate the Winning  Probability of Player B 

	float Pb = EloProbability (fRatingWinner, fRatingLoser);
	float Pa = EloProbability (fRatingLoser,  fRatingWinner);

	fRatingWinner = fRatingWinner + fK * (1 - Pa);
	fRatingLoser = fRatingLoser + fK * (0 - Pb);
} 
  
#ifdef ThisWorks
int main() 
{ 
    // Ra and Rb are current ELO ratings 
    float Ra = 1200, Rb = 1000; 
      
    int K = 30; 
    bool d = 1; 
    EloRating(Ra, Rb, K, d); 
  
    return 0; 
} 
#endif


///////////////////////////////////////////////////////////////
//
//  E N S U R E   W I N D O W   I S   V I S I B L E
//
//  Makes sure a window (a rectangle in screen coordinates) fits entirely on the
//  screen.  If not, adjusts it so it will
//
//  Returns true if we had to change the position, false if not
//
bool CUtils::EnsureWindowIsVisible (CRect& rc)
{
	CRect rcPassedIn = rc;

	CMonitors oMonitors;
	CMonitor  oNearestMonitor = oMonitors.GetNearestMonitor (rc);
	
	//
	//  We only mess with things if the window is not completely visible

	if (! oNearestMonitor.IsCompletelyOnMonitor (rc))
	{
		oNearestMonitor.ClipRectToMonitor (rc, true);
		oNearestMonitor.CenterRectToMonitor (rc, true);
	}

	return rc != rcPassedIn;


} // end ensure window is visible




/*************************************

  S T R I S T R

  Performs strstr in a case insensitive manner.  

  I'm still not sure whether to be proud or tremendously ashamed of this routine,
  but it's used quite a lot so if you happen to know that doing it this way
  really sucks, please change it (and let me know) -- dlp 3/98

  2019/03/19 DLP -	Well, 20 years later, I ran into a bug in the entity extraction 
					routine that calls this, which prompted me to look at this.  And, yeah,
					I made it quite a bit better.

*************************************/
TCHAR* CUtils::stristr (wchar_t* pstrHaystack, const wchar_t* pstrNeedle, int* pnMatchStart /* = NULL */)
{
	int nHaystackCount	= (int) _tcslen (pstrHaystack);
	int nNeedleCount	= (int) _tcslen (pstrNeedle);
	if (0 == nNeedleCount)
		return NULL;

	int nToCheck = nHaystackCount - nNeedleCount +1;

	for (int nHaystack = 0; nHaystack < nToCheck; nHaystack ++)
	{
		//
		//  First char match, let's check the rest...

		if (_wcsnicmp (&(pstrHaystack[nHaystack]), pstrNeedle, nNeedleCount) == 0)
		{
			//
			//  Match!

			if (NULL != pnMatchStart)
				*pnMatchStart = nHaystack;
			return &(pstrHaystack[nHaystack]);
		} // end if we matched
	} // end loop through haystack

	if (NULL != pnMatchStart)
		*pnMatchStart = -1;

	return NULL;

} // end stristr








CString CUtils::GetKeyName (unsigned int virtualKey)
{
	switch (virtualKey)
	{
        case VK_LEFT:	return _T("Left");
		case VK_UP:		return _T("Up");
		case VK_RIGHT:	return _T("Right");
		case VK_DOWN:	return _T("Down");
        case VK_PRIOR:  return _T("PgUp");
		case VK_NEXT:	return _T("PgDown");

		case VK_END:	return _T("End");
		case VK_HOME:	return _T("Home");
        case VK_INSERT:	return _T("Insert");
		case VK_DELETE:	return _T("Delete");
        case VK_DIVIDE: return _T("/");
        case VK_NUMLOCK:return _T("NumLock");
	}

	if (virtualKey >= 0x30 && virtualKey <= 0x5a)
	{
		TCHAR tc = (TCHAR) virtualKey;
		CString s (tc);
		return s;
	}

	return _T("Unknown");

#ifdef TryToDoItRight
	unsigned int scanCode = MapVirtualKey (virtualKey, MAPVK_VK_TO_VSC);

    // because MapVirtualKey strips the extended bit for some keys
    switch (virtualKey)
    {
        case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
        case VK_PRIOR: case VK_NEXT: // page up and page down
        case VK_END: case VK_HOME:
        case VK_INSERT: case VK_DELETE:
        case VK_DIVIDE: // numpad slash
        case VK_NUMLOCK:
        {
            scanCode |= 0x100; // set extended bit
            break;
        }
    }

	CString strKeyName;
    if (GetKeyNameText (scanCode << 16, strKeyName.GetBuffer (), strKeyName.GetLength ()) != 0)
    {
        return strKeyName;
    }
    else
    {
		return GetLastErrorAsString ();
    }
#endif
}



////////////////////////////////////////////////////////////////////////
//
//  C O P Y   T E X T   T O   C L I P B O A R D
//
//  Stores str on the clipboard.
//
//  Returns true if successful, false otherwise
//
bool CUtils::CopyTextToClipboard (CString str, bool bSilent /* = false */)
{
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, sizeof (TCHAR) * (str.GetLength() + 1));
	if (NULL == hMem)
		return false;

	TCHAR* lpDest = (TCHAR *)GlobalLock(hMem);
	if (NULL == lpDest)
		return false;

	_tcscpy (lpDest, str);
	GlobalUnlock(hMem);
				
	//try to open the clipboard


	int bOpened = false;
	for (int i = 0; i < 10; i ++)
	{
		if (OpenClipboard (NULL))
		{
			bOpened = true;
			break;
		}

		SleepMsg (50);
	}

	if (! bOpened)
	{
		if (! bSilent)
			AfxMessageBox (L"Cannot open the Clipboard");
		return FALSE;
	}
				
	// Remove the current Clipboard contents
	if( !EmptyClipboard() )
	{
		if (! bSilent)
			AfxMessageBox (L"Cannot empty the Clipboard");
		return FALSE;
	}
				
	//export records to clipboard
#ifdef _UNICODE
	UINT nFormat = CF_UNICODETEXT;
#else
	UINT nFormat = CF_TEXT;
#endif

	if ( ::SetClipboardData (nFormat, hMem) == NULL ) // CF_TEXT
	{
		if (! bSilent)
			AfxMessageBox (L"Unable to set Clipboard data");
		CloseClipboard();
		return FALSE;
	}
				
	//close clipboard
	CloseClipboard();

	return true;

} // end copy text to clipboard











//////////////////////////////////////////////////////////////////////
//
//    C O P Y   T E X T   F R O M   C L I P B O A R D 
//
//  If there's text on the clipboard, sends it back in rStr.
//  
//  Returns true if we find something, false if 
//
bool CUtils::CopyTextFromClipboard (CString& rStr, bool bSilent /* = false */)
{
	UINT nTextFormat = CF_TEXT;
#ifdef _UNICODE
	nTextFormat = CF_UNICODETEXT;
#endif

	if (!IsClipboardFormatAvailable (nTextFormat)) 
		return false; 
	if (!OpenClipboard (NULL)) 
		return false; 

	bool bFoundText = false;

	HANDLE hglb = GetClipboardData (nTextFormat); 
	if (hglb != NULL) 
	{ 
		LPTSTR lptstrClipboard = (LPTSTR) GlobalLock(hglb); 
		if (lptstrClipboard != NULL) 
		{ 
			rStr = lptstrClipboard;
			bFoundText = true;

			GlobalUnlock(hglb); 
		} 
	} 

	CloseClipboard(); 
	return bFoundText; 

} // end CopyTextFromClipboard






//////////////////////////////////////////////////////////////////////
//
//    G E T   P R O D U C T   V E R S I O N 
//
//
CString CUtils::GetProductVersion ()
{
	CVersionInfo VI;
	if (! GetProductVersionDetails (&VI))
		return L"Error Getting Product Version";

	CString strVersion = VI.GetVersionString ();
	return strVersion;

} // end GetProductVersion


//////////////////////////////////////////////////////////////////////
//
//    G E T   P R O D U C T   V E R S I O N   D E T A I L S 
//
//
bool CUtils::GetProductVersionDetails (CVersionInfo* pVersionInfo)
{
	if (NULL == pVersionInfo)
		return false;

	CString strExeName = GetExeName ();

	CModuleVersion ver;
	if (! ver.GetFileVersionInfo (strExeName))
		return false;

	int nMajorVersion	= HIWORD(ver.dwFileVersionMS);
	int nMinorVersion	= LOWORD(ver.dwFileVersionMS);

	int nBuildCode		= HIWORD(ver.dwFileVersionLS);
	int nExtra			= LOWORD(ver.dwFileVersionLS);

	pVersionInfo->SetVersionInfo (nMajorVersion, nMinorVersion, nBuildCode, nExtra);

	return true;

} // end GetProductVersionDetails



//////////////////////////////////////////////////////////////////////
//
//    G E T   E X E   N A M E 
//
//
/* static */ CString CUtils::GetExeName (bool bReturnFullPath /* = false */)
{
	TCHAR toasPath[MAX_PATH];
	::GetModuleFileName (NULL, toasPath, MAX_PATH); // returns full path including filename

	CString strFullPath = toasPath;

	if (bReturnFullPath)
		return strFullPath;

	int nLastBS = strFullPath.ReverseFind ('\\');
	if (-1 == nLastBS)
		return strFullPath;

	return strFullPath.Mid (nLastBS + 1);

} // end GetExeName
