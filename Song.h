#pragma once

#include <memory>

class CSong
{
public:
	CSong (CString strName, CString strPathToMp3, UINT nID = 0);
	~CSong () {};

	CString		GetName ()		{return m_strName;};
	UINT		GetID ()		{return m_nID;};
	CString		GetPathToMp3 ()	{return m_strPathToMp3;};

	void		AddGameResult (int nGameScoreMargin);
	void		GetGameResultDetails (int& rnGamesWon, int& rnGamesLost, int& rnCumulativeScoreMargin) {rnGamesLost = m_nGamesLost; rnGamesWon = m_nGamesWon; rnCumulativeScoreMargin = m_nCumulativeGameMargin;};

	float		GetSongRating ()				{return m_fSongRating;};
	void		SetSongRating (float fRating)	{m_fSongRating = fRating;};

protected:
	UINT			m_nID		= 0;
	CString			m_strName;
	CString			m_strPathToMp3;

	int				m_nGamesWon				= 0;
	int				m_nGamesLost			= 0;
	int				m_nCumulativeGameMargin = 0;

	float			m_fSongRating	= 0;

};


typedef std::shared_ptr<CSong>		CSongPtr;