#pragma once

#define VP_DEFAULT_UNUSED	-987654


//////////////////////////////////////////////////////////////////
//
//  Table definitions...  the FieldDefinitionStruct is used so 
//  when we change a table we only have to do it here and not where
//  the table is created.  See, this orders the columns and lets 
//  us loop through them
//  

struct TableDefinitionStruct
{
	BOOL			bPrimaryKey;
	BOOL			bUnique;
	const wchar_t*	strColumnName;
	const wchar_t*	strDataType;
	int				nDefaultVal;
};



//////////////////////////////////////////////////////////////////
//
#define TBL_SONGS				L"Songs"
#define TBL_SONGS_VER			1

#define DB_COL_SONG_ID					L"ID"
#define DB_COL_SONG_NAME			L"Name"
#define DB_COL_PATH_TO_MP3			L"PathToMp3"

static TableDefinitionStruct VPTblSongs[] = {
	{true,	false,	DB_COL_SONG_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_NAME,			L"TEXT",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_PATH_TO_MP3,			L"TEXT",	VP_DEFAULT_UNUSED},
	{0,		0,		0,							0,					0}
};


//////////////////////////////////////////////////////////////////
//
#define TBL_GAME_HISTORY			L"GameHistory"
#define TBL_GAME_HISTORY_VER		1

#define DB_COL_SONG_1_ID				L"Song1"
#define DB_COL_SONG_2_ID				L"Song2"
#define DB_COL_GAME_SCORE_MARGIN		L"GameScoreMargin"

static TableDefinitionStruct VPTblGameHistory[] = {
	{false,	false,	DB_COL_SONG_1_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_2_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_GAME_SCORE_MARGIN,		L"INTEGER",	VP_DEFAULT_UNUSED},
	{0,		0,		0,							0,					0}
};




//////////////////////////////////////////////////////////////////
//
#define TBL_CURRENT_SCHEDULE		L"CurrentSchedule"
#define TBL_CURRENT_SCHEDULE_VER	1

//#define DB_COL_SONG_1_ID				L"Song1"		// Already defined above
//#define DB_COL_SONG_2_ID				L"Song2"		// Already defined above
#define DB_COL_SONG_3_ID				L"Song3"
#define DB_COL_SONG_4_ID				L"Song4"
#define DB_COL_SONG_5_ID				L"Song5"

static TableDefinitionStruct VPTblCurrentSchedule[] = {
	{false,	false,	DB_COL_SONG_1_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_2_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_GAME_SCORE_MARGIN,		L"INTEGER",	VP_DEFAULT_UNUSED},
	{0,		0,		0,							0,					0}
};




// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
//
//  We use this array to make sure all tables are created and all defined columns exist.
//
//  Note that this does not support removing columns from existing tables, only adding.
//
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************

struct SVpSqlTableDefinitions
{
	const wchar_t*					m_strTableName;
	struct TableDefinitionStruct*	m_pTableDef;
	int								m_nTableVersion;
};



static SVpSqlTableDefinitions arrVpSqlTableDefinitions[] = {
	{TBL_SONGS,				VPTblSongs,				TBL_SONGS_VER},
	{TBL_GAME_HISTORY,		VPTblGameHistory,		TBL_GAME_HISTORY_VER},
	{TBL_CURRENT_SCHEDULE,	VPTblCurrentSchedule,	TBL_CURRENT_SCHEDULE_VER},
	{0,						0,						0}
}; // end VP SQL Table Definition array


#define VP_SQL_TABLE_DEF_COUNT	(sizeof (arrVpSqlTableDefinitions) / sizeof (SVpSqlTableDefinitions))


// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************

