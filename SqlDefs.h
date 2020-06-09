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
//#define TBL_SONGS_VER			1
//#define TBL_SONGS_VER			2	// Added tag info, artist, title, album
//#define TBL_SONGS_VER			3	// Removed song name
#define TBL_SONGS_VER			4	// Added strength of schedul

#define DB_COL_SONG_ID				L"ID"
#define DB_COL_PATH_TO_MP3			L"PathToMp3"
#define DB_COL_SONG_ARTIST			L"Artist"
#define DB_COL_SONG_TITLE			L"Title"
#define DB_COL_SONG_ALBUM			L"Album"
#define DB_COL_SONG_RATING			L"Rating"
#define DB_COL_SONG_STRENGTH_OF_SCHEDULE	L"StrengthOfSchedule"

static TableDefinitionStruct VPTblSongs[] = {
	{true,	false,	DB_COL_SONG_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	true,	DB_COL_PATH_TO_MP3,			L"TEXT",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_ARTIST,			L"TEXT",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_TITLE,			L"TEXT",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_ALBUM,			L"TEXT",	VP_DEFAULT_UNUSED},
	{false, false,	DB_COL_SONG_RATING,			L"INTEGER",	1000},
	{false, false,	DB_COL_SONG_STRENGTH_OF_SCHEDULE,			L"INTEGER",	1000},
	{0,		0,		0,							0,					0}
};


//////////////////////////////////////////////////////////////////
//
#define TBL_SONG_HEAD_TO_HEAD		L"SongHeadToHead"
#define TBL_SONG_HEAD_TO_HEAD_VER	1

#define DB_COL_POD_ID					L"PodID"			// Each grouping of 5 songs gets an ID
#define DB_COL_SONG_1_ID				L"Song1"
#define DB_COL_SONG_2_ID				L"Song2"
#define DB_COL_SCORE_MARGIN				L"ScoreMargin"

static TableDefinitionStruct VPTblSongHeadToHead[] = {
	{false,	false,	DB_COL_POD_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_1_ID,			L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_2_ID,			L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SCORE_MARGIN,		L"INTEGER",	VP_DEFAULT_UNUSED},
	{0,		0,		0,							0,					0}
};




//////////////////////////////////////////////////////////////////
//
#define TBL_SONG_PODS		L"SongPods"
//#define TBL_SONG_PODS_VER	1
#define TBL_SONG_PODS_VER	2	// Added PodID

//#define DB_COL_POD_ID					L"PodID"			// Each grouping of 5 songs gets an ID
//#define DB_COL_SONG_1_ID				L"Song1"		// Already defined above
//#define DB_COL_SONG_2_ID				L"Song2"		// Already defined above
#define DB_COL_SONG_3_ID				L"Song3"
#define DB_COL_SONG_4_ID				L"Song4"
#define DB_COL_SONG_5_ID				L"Song5"
#define DB_COL_POOL_FINISHED			L"Finished"

static TableDefinitionStruct VPTblCurrentSchedule[] = {
	{true,	false,	DB_COL_POD_ID,					L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_1_ID,				L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_SONG_2_ID,				L"INTEGER",	-1},
	{false,	false,	DB_COL_SONG_3_ID,				L"INTEGER",	-1},
	{false,	false,	DB_COL_SONG_4_ID,				L"INTEGER",	-1},
	{false,	false,	DB_COL_SONG_5_ID,				L"INTEGER",	-1},
	{false, false,  DB_COL_POOL_FINISHED,			L"INTEGER",  0},
	{0,		0,		0,							0,					0}
};





//////////////////////////////////////////////////////////////////
//
#define TBL_MP3_TAGS			L"MP3Tags"
#define TBL_MP3_TAGS_VER		1

#define DB_COL_TAG_TYPE				L"TagType"		//  So, "Artist" or "Title" - the thing we understand
#define DB_COL_TAG_NAME				L"TagName"		//  TPE1 or TP1 or whatever cryptic crap is in there

static TableDefinitionStruct VPTblMp3Tags[] = {
	{false,	false,	DB_COL_TAG_TYPE,		L"TEXT",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_TAG_NAME,		L"TEXT",	VP_DEFAULT_UNUSED},
	{0,		0,		0,						0,			0}
};



//////////////////////////////////////////////////////////////////
//
#define TBL_OTHER_STUFF			L"OtherStuff"
#define TBL_OTHER_STUFF_VER		1

#define DB_COL_WHAT				L"What"			
#define DB_COL_VALUE			L"Value"		

static TableDefinitionStruct VPTblOtherStuff[] = {
	{true,	false,	DB_COL_WHAT,		L"TEXT",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_VALUE,		L"TEXT",	VP_DEFAULT_UNUSED},
	{0,		0,		0,						0,			0}
};



//////////////////////////////////////////////////////////////////
//
#define TBL_COLUMNS			L"Columns"
#define TBL_COLUMNS_VER		1

#define DB_COL_INDEX		L"ColIndex"
#define DB_COL_TYPE			L"ColType"		
#define DB_COL_NAME			L"Name"
#define DB_COL_FORMAT		L"Format"
#define DB_COL_WIDTH		L"Width"

static TableDefinitionStruct VPTblColumns[] = {
	{true,	false,	DB_COL_INDEX,		L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_TYPE,		L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_NAME,		L"TEXT",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_FORMAT,		L"INTEGER",	VP_DEFAULT_UNUSED},
	{false,	false,	DB_COL_WIDTH,		L"INTEGER",	VP_DEFAULT_UNUSED},
	{0,		0,		0,					0,			0}
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
	{TBL_SONG_HEAD_TO_HEAD,	VPTblSongHeadToHead,	TBL_SONG_HEAD_TO_HEAD_VER},
	{TBL_SONG_PODS,			VPTblCurrentSchedule,	TBL_SONG_PODS_VER},
	{TBL_MP3_TAGS,			VPTblMp3Tags,			TBL_MP3_TAGS_VER},
	{TBL_OTHER_STUFF,		VPTblOtherStuff,		TBL_OTHER_STUFF_VER},
	{TBL_COLUMNS,			VPTblColumns,			TBL_COLUMNS_VER},
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

