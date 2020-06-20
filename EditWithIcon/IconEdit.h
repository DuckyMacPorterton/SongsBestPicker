#pragma once

/////////////////////////////////////////////////////////////////////////////
// CIconWnd window

static const UINT UWM_VP_CLEANUP_CLEAR_TYPE_TO_FILTER	= ::RegisterWindowMessage (_T("UWM_VP_CLEANUP_CLEAR_TYPE_TO_FILTER"));


class CIconWnd : public CStatic
{
// Construction/destruction
public:
	CIconWnd();
	virtual ~CIconWnd();

// Operations
public:
	void SetIcon( HICON icon );

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:
	HICON	m_icon;
	bool	m_bMouseOver	= false;
	HCURSOR	m_hCursorPrev	= NULL;

public:
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};



/////////////////////////////////////////////////////////////////////////////
// CIconEdit window

class CIconEdit : public CEdit
{
// Construction/destruction
public:
	CIconEdit();
	virtual ~CIconEdit();

public:
	void SetIcon (HICON icon,	bool bIconOnRight = false);
	void SetIcon (UINT iconres, bool bIconOnRight = false);

protected:
	virtual void PreSubclassWindow();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam); // Maps to WM_SETFONT
	DECLARE_MESSAGE_MAP()

protected:
	void		Prepare();
	void		CreateIcon();

	CIconWnd	m_icon;
	HICON		m_internalIcon;

	bool		m_bIconOnRight = false;

	CRect		m_rcEditPrev;

	CRect		m_rectNCBottom;	//  These help us center text vertically
	CRect		m_rectNCTop;	//  These help us center text vertically


public:
	afx_msg void OnMouseMove	(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave	();
	afx_msg void OnLButtonDown	(UINT nFlags, CPoint point);

	afx_msg void OnNcCalcSize	(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint		();
	afx_msg UINT OnGetDlgCode	();
};
