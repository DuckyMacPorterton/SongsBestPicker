#pragma once

//***************************************
//
//	cwh -- 2/8/01 -- enhanced so that you can
//		use tool tips with CListBox in addition
//		to CListCtrl
//
//***************************************

class CTitleTip : public CWnd
{
public:
	CTitleTip();

	virtual BOOL Create(CListBox* pParentWnd);

	virtual void Show(CRect DisplayRect, int nItemIndex);
	virtual void Hide();
public:
	virtual ~CTitleTip();

protected:
	const int		m_nNoIndex;			// Not a valid index
	static LPCTSTR	m_pszWndClass;		// Registered class name
	int				m_nItemIndex;		// Index of currently displayed listbox item

	CListBox		*m_pListBox;		// Parent list box

	BOOL IsListOwnerDraw();

protected:
	afx_msg void OnPaint();
			BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	DECLARE_MESSAGE_MAP()
};

