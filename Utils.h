#pragma once
#include "SqlDefs.h"
#include "SQLite/CppSQLite3-Unicode.h"
#include <Mlang.h>
#include <mciapi.h>


class CUtils
{
public:
	static bool		EnsureDBTablesExist (CString& rstrError, CMyCppSQLite3DBPtr pDB);
	static bool		CreateDBTable		(CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, TableDefinitionStruct TableInfo[]);
	static int		GetNumFieldsInTable (TableDefinitionStruct TableDef[]);

	static bool		EnsureAllColsExist	(CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, TableDefinitionStruct TableInfo[]);
	static bool		EnsureColExists		(CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, CString strColName, CString strDataType = L"INTEGER", int nDefaultValue = VP_DEFAULT_UNUSED);
	static bool		DoesColExist		(CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName, CString strColName);

	static bool		DoesTableExist		(CString& rstrError, CMyCppSQLite3DBPtr pDB, CString strTableName);

	static CString	GetErrorMessageFromException (CException* pE, bool bDeleteException = false);
	static CString	GetMciErrorString (MCIERROR nMciError);

	static CString	GetProgramFilesDir ();

	static CString	GetFileNameFromPath (CString strFullPath, bool bRemoveAllExtensions = true);

	static bool		GetIsCharInsideSubExpression (CString strHaystack, int nCharIndex, CString strExpressionStart, CString strExpressionEnd, bool bSupportQuoting = false, int nStartAtIndex = 0);
	static int		CountInstancesOfString (CString strBig, CString strLittle, int nCharsToCheck = -1, bool bSkipQuotedBsChars = false);

	static CStringA UTF16toUTF8 (const CStringW& utf16);
	static CStringW UTF8toUTF16 (const CStringA& utf8);

	static int		GetLastValidUtfCharacterByte (CStringA astrToTest);
	static bool		GetIsValidUtf8String (CStringA astrToTest);

	static bool		DetectCodePageForString (char* pText, UINT& rnCodePage, UINT nDefaultCodePage = CP_UTF8, UINT nDefaultFlags = MLDETECTCP_8BIT);

	static bool		MyAtoI (CString str, int& rnValFound, bool bRequireMatchEntireString = true);
	static CString	NumberToString (int nNumber,		bool bOutputAsHex = false, int nPadZeroTotalDigitCount = 0);


	static bool		FileExists (CString strName);
	static bool		FindFile (CString& rstrPathToSong);

	static BOOL		SleepMsg (DWORD dwTimeoutMS);

	static void		EloRating		(float& Ra, float& Rb, int K, bool d);
	static float	EloProbability	(float rating1, float rating2);



};



class CRecursionCounterAuto
{
public:
	CRecursionCounterAuto (int* pnCounter) {m_pnCounter = pnCounter; (*m_pnCounter) ++;};
	~CRecursionCounterAuto () {(*m_pnCounter) --;};

	int GetRecursionDepth () {return *m_pnCounter;};

protected:
	int*	m_pnCounter;
};
