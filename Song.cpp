#include "stdafx.h"
#include "Song.h"
#include "SQLite/CppSQLite3-Unicode.h"
#include "Utils.h"

//************************************
// Method:    CSong
// FullName:  CSong::CSong
// Access:    public 
// Returns:   
// Qualifier:
// Parameter: CString strName
//************************************
CSong::CSong (CString strName, CString strPathToMp3, UINT nID /* = 0 */)
{
	static UINT nLastIdUsed = 0;
	if (0 == nID)
	{
		nLastIdUsed ++;
		nID = nLastIdUsed;
	}

	m_strName		= strName;
	m_strPathToMp3	= strPathToMp3;
	m_nID			= nID;

} // end song constructor




//************************************
// Method:    AddGameResult
// FullName:  AddGameResult
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: int nGameScoreMargin
//************************************
void CSong::AddGameResult (int nGameScoreMargin)
{
	if (nGameScoreMargin > 0)
		m_nGamesWon ++;
	else if (nGameScoreMargin < 0)
		m_nGamesLost ++;

	m_nCumulativeGameMargin += nGameScoreMargin;

}
