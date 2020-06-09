#pragma once
 
/////////////////////////////////////////////////////////////////////////////
// CMyListCtrlHeaderCtrl window

static const UINT UWM_SONG_HEADER_DRAG_COL	= ::RegisterWindowMessage(L"UWM_SONG_HEADER_DRAG_COL");



class CMyListCtrlHeaderCtrl : public CHeaderCtrl
{
public:
	CMyListCtrlHeaderCtrl();

public:
	virtual ~CMyListCtrlHeaderCtrl();
	void SetCallback(CWnd* pWnd, void (CWnd::*fpDragCol)(int, int));
 
protected:
	BOOL	m_bCheckForDrag	= false;
	BOOL	m_bDragging		= false;
	int*	m_pWidth		= NULL;
	int		m_nDragCol		= -1;
	int		m_nDropPos		= -1;
	CRect	marker_rect;
	CWnd*	m_pOwnerWnd		= NULL;
 
 protected:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
 
	DECLARE_MESSAGE_MAP()
};

