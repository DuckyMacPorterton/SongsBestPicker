#include "stdafx.h"
#include "Utils.h"


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

	int numFields = CUtils::GetNumFieldsInTable (TableInfo);
	for (int i = 0; i < numFields; i++)
	{
		CString strCol;
		if (i != 0)
			strDML += ", ";

		strCol.Format (L"%s %s", TableInfo[i].strColumnName, TableInfo[i].strDataType);

		//
		//  We write it this way because PRIMARY KEY is inherently unique so no need to check it for both.

		if (TableInfo[i].bPrimaryKey) {
			nNumPrimaryKeys++;

			strCol += L" NOT NULL";

			if (nNumPrimaryKeys > 1)
				strPrimaryKey += _T (", ");
			strPrimaryKey += TableInfo[i].strColumnName;
		}
		else if (TableInfo[i].bUnique)
			strCol += L" NOT NULL UNIQUE";

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















// Function to calculate the Probability 
/* static */ float CUtils::EloProbability (float rating1, float rating2) 
{ 
    return (float) (1.0 * 1.0 / (1 + 1.0 *  
           pow(10, 1.0 * ((double) rating1 - rating2) / 400)));
} 
  



// Function to calculate Elo rating 
// K is a constant. 
// d determines whether Player A wins or Player B.  
/* static */ void CUtils::EloRating (float& Ra, float& Rb, int K, bool d) 
{   
    //
    //  FiveThirtyEight has K=20 for NBA and similar for soccer + NFL
    //  So try 20 first
    //  Chess might use 32.  Or it might change based on ratings.

    //
    //  To add margin of victory to this:

#ifdef asdfasdf
    The margin of victory multiplier is calculated as follows.

    Take a team’s margin of victory, add 3 points and then take the result to the power of 0.8.
    Divide the result by the following formula: 7.5+.006*(elo_diff), where elo_diff represents the Elo rating difference between the teams, accounting for home-court advantage. Elo_diff should be negative in games won by the underdog.

============

It isn’t complicated in principle. You just have to discount the margin of victory more when favorites win and increase it when underdogs win. The formula for it is as follows:

Margin of Victory Multiplier = LN(ABS(PD)+1) * (2.2/((ELOW-ELOL)*.001+2.2))

Where PD is the point differential in the game, ELOW is the winning team’s Elo Rating before the game, and ELOL is the losing team’s Elo Rating before the game.

#endif



    // To calculate the Winning 
    // Probability of Player B 
    float Pb = EloProbability (Ra, Rb); 
  
    // To calculate the Winning 
    // Probability of Player A 
    float Pa = EloProbability(Rb, Ra); 
  
    // Case -1 When Player A wins 
    // Updating the Elo Ratings 
    if (d == 1) { 
        Ra = Ra + K * (1 - Pa); 
        Rb = Rb + K * (0 - Pb); 
    } 
  
    // Case -2 When Player B wins 
    // Updating the Elo Ratings 
    else { 
        Ra = Ra + K * (0 - Pa); 
        Rb = Rb + K * (1 - Pb); 
    } 
  
//    cout << "Updated Ratings:-\n"; 
//    cout << "Ra = " << Ra << " Rb = " << Rb; 
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