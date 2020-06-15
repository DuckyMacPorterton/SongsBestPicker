#pragma once

//****************************************************
//
//	cwh -- 2/8/01 -- I changed HScrollListBox so that is can
//	be used to show pop up tool tips for subclasses list boxes.
//	It no longer launches the record dialog box upon double clicking
//	a name.  This is so that the HScrollListBox can be used for
//	cases than just the TitleWnd
//
//******************************************************

#include "typedefs.h"
#include "TitleTip.h"

#define HORIZONTAL_FUDGE	4	// Add this additional fudge factor to horizontal extent

class HScrollListBox : public CListBox
{
	DECLARE_DYNAMIC (HScrollListBox)
public:
	HScrollListBox();
	virtual ~HScrollListBox();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HScrollListBox)
	public:
	//}}AFX_VIRTUAL

protected:
	int				m_BaseTextHeight;	// used for owner draw routines
	int				m_nMaxTextExtent;	// The maximum text extent of all strings within a list box
	CIntArray		m_TitleIndexes;		// title indexes

	const int m_nNoIndex; // Not a valid index
	CPoint m_LastMouseMovePoint; // Last position of mouse cursor
	BOOL m_bMouseCaptured; // Is mouse captured?
	CTitleTip m_TitleTip; // TitleTip that gets displayed when necessary

	//
	//  This is to support alternate text for the title tip

	int             m_bUseAltTipText;
	CStringArray    m_AltTipTextArray;

public:
	int AddString( LPCTSTR lpszItem );
	int InsertString( int nIndex, LPCTSTR lpszItem );
	int DeleteString( UINT nIndex );
	void ResetContent( );

	int	SelectStringExact (CString strToSelect);

	int     GetUseAltTipText () {return m_bUseAltTipText;};
	void    SetUseAltTipText (int b) {m_bUseAltTipText = b;};
	void    SetAltTipText (int nIndex, CString strText);
	CString GetAltTipText (int nIndex);

protected:
	// This method should be overridden by an owner-draw listbox.
	virtual int GetIdealItemRect(int nIndex, LPRECT lpRect);

	void AdjustTitleTip(int nNewIndex);
	void CaptureMouse();
	BOOL IsAppActive();


	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HScrollListBox)
	//}}AFX_VIRTUAL

public:
	int m_nCurSel;

	// Generated message map functions
protected:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	virtual int CompareItem(LPCOMPAREITEMSTRUCT);

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk (UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
			BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

			BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////


