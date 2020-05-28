#pragma once
#include "SqlDefs.h"
#include "SQLite/CppSQLite3-Unicode.h"


class CUtils
{
public:
	static bool		EnsureDBTablesExist (CString& rstrError, CMyCppSQLite3DB* pDB);
	static bool		CreateDBTable		(CString& rstrError, CMyCppSQLite3DB* pDB, CString strTableName, TableDefinitionStruct TableInfo[]);
	static int		GetNumFieldsInTable (TableDefinitionStruct TableDef[]);

	static bool		EnsureAllColsExist	(CString& rstrError, CMyCppSQLite3DB* pDB, CString strTableName, TableDefinitionStruct TableInfo[]);
	static bool		EnsureColExists		(CString& rstrError, CMyCppSQLite3DB* pDB, CString strTableName, CString strColName, CString strDataType = L"INTEGER", int nDefaultValue = VP_DEFAULT_UNUSED);
	static bool		DoesColExist		(CString& rstrError, CMyCppSQLite3DB* pDB, CString strTableName, CString strColName);

	static bool		DoesTableExist		(CString& rstrError, CMyCppSQLite3DB* pDB, CString strTableName);

	static CString	GetErrorMessageFromException (CException* pE, bool bDeleteException = false);
	static CString	GetProgramFilesDir ();

	static void		EloRating		(float& Ra, float& Rb, int K, bool d);
	static float	EloProbability	(float rating1, float rating2);

};

