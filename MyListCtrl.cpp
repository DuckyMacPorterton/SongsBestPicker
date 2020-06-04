// MyListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MyListCtrl.h"


#define VP_LIST_CTRL_SHOW_TIP_TIMER_ID			4	// arbitrary
#define VP_LIST_CTRL_SHOW_TIP_TIMER_MS			500

#define VP_LIST_CTRL_OUT_OF_RECT_TIMER_ID		5
#define VP_LIST_CTRL_OUT_OF_RECT_TIMER_MS		80

IMPLEMENT_DYNAMIC(CMyListCtrl, CListCtrl)
CMyListCtrl::CMyListCtrl()
{
	m_hWndToolTip			= NULL;

	m_bCreatedDynamically	= false;
	m_bToolTipDisplayed		= false;

	m_ptLastTipPos.x = -1;
	m_ptLastTipPos.y = -1;

}

CMyListCtrl::~CMyListCtrl()
{
	if (NULL != m_hWndToolTip)
		::DestroyWindow (m_hWndToolTip);
}


BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_NOTIFY_REFLECT ( NM_CUSTOMDRAW, OnCustomDrawList )
	ON_WM_CREATE ()
END_MESSAGE_MAP()


// CMyListCtrl message handlers






//
//  This is called before PreSubclassWindow when this is being
//  created dynamically.  When that's the case, we don't want
//  to create the tooltip in PreSubclassWindow, we want to do
//  it in OnCreate...
// 

BOOL CMyListCtrl::PreCreateWindow (CREATESTRUCT &cs)
{
	m_bCreatedDynamically = true;
	return CListCtrl::PreCreateWindow (cs);
}





#undef SubclassWindow // to avoid the SubclassWindow macro in windowsx.h
int CMyListCtrl::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
	int nRetVal = CListCtrl::OnCreate (lpCreateStruct);

	if (m_bCreatedDynamically) {
		CreateToolTip ();

//		if (m_bRecordClassificationListCtrl) {
//			m_oRecordClassificationHeaderCtrl.SubclassWindow (GetHeaderCtrl()->m_hWnd);
//		}
	}

	return nRetVal;
}



///////////////////////////////////////////////////////////////////////////////
// PreSubclassWindow
void CMyListCtrl::PreSubclassWindow() 
{
	//use our custom CHeaderCtrl as long as there
	//is a headerctrl object to subclass
//	if (m_bRecordClassificationListCtrl && (NULL != GetHeaderCtrl ()))
//		m_oRecordClassificationHeaderCtrl.SubclassWindow (GetHeaderCtrl()->m_hWnd);

	CListCtrl::PreSubclassWindow();
	if (! m_bCreatedDynamically) {
		CreateToolTip ();
	}
}





BOOL CMyListCtrl::PreTranslateMessage (MSG*pMsg) 
{
	m_oToolTip.RelayEvent (pMsg);
	return CListCtrl::PreTranslateMessage(pMsg);
}


void CMyListCtrl::CreateToolTip ()
{
	if (m_oToolTip.GetSafeHwnd () != NULL)
		return;

	m_oToolTip.Create (this, WS_POPUP | TTS_ALWAYSTIP); // WS_VISIBLE 


	m_hWndToolTip = ::CreateWindowEx(WS_EX_TOPMOST,
							  TOOLTIPS_CLASS,
							  NULL,
							  TTS_NOPREFIX | TTS_ALWAYSTIP,
							  CW_USEDEFAULT,
							  CW_USEDEFAULT,
							  200,	//  default width
							  20,	//  default height
							  m_hWnd,
							  NULL,
							  NULL,
							  NULL);
	ASSERT(m_hWndToolTip);

	// initialize toolinfo struct
	memset(&m_ToolInfo, 0, sizeof(m_ToolInfo));
	m_ToolInfo.cbSize = sizeof(m_ToolInfo);
	m_ToolInfo.uFlags = TTF_TRACK | TTF_TRANSPARENT;
	m_ToolInfo.hwnd = m_hWnd;


	SetToolTips (&m_oToolTip);


	// add list box
	::SendMessage(m_hWndToolTip, TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	::SendMessage(m_hWndToolTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &m_ToolInfo);
	::SendMessage(m_hWndToolTip, TTM_SETTIPBKCOLOR, ::GetSysColor(COLOR_HIGHLIGHT), 0);
	::SendMessage(m_hWndToolTip, TTM_SETTIPTEXTCOLOR, ::GetSysColor(COLOR_HIGHLIGHTTEXT), 0);

	// reduce top & bottom margins
	CRect rectMargins(0,-1,0,-1);
	::SendMessage(m_hWndToolTip, TTM_SETMARGIN, 0, (LPARAM)&rectMargins);

	// set font
	CFont *pFont = GetFont();
	::SendMessage(m_hWndToolTip, WM_SETFONT, (WPARAM)(HFONT)*pFont, FALSE);

	// remove border (listbox items only)
	LONG_PTR lStyle = ::GetWindowLongPtr (m_hWndToolTip, GWL_STYLE);
	lStyle &= ~WS_BORDER;
	::SetWindowLongPtr (m_hWndToolTip, GWL_STYLE, lStyle);

} // end create tool tip



///////////////////////////////////////////////////////////////////////////////
// OnMouseMove
void CMyListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (NULL == GetSafeHwnd ())
		return;
	
	CreateToolTip ();

	if (m_ptLastTipPos == point)
		return;

	CRect rectClient;
	GetClientRect(&rectClient);

	if (rectClient.PtInRect(point))
	{
		//
		//  Is this in the header?

		bool	bNeedToDisplayTip = false;
		CPoint	ptToolTipDisplay;

		LVHITTESTINFO lvhti;
		lvhti.pt = point;
		int nItem = SubItemHitTest(&lvhti);

		//
		//  If we've already set bNeedToDisplayTip to true then we don't check the rest, which is important because
		//  the list control will tell us that the header is part of item 0

		if (! bNeedToDisplayTip)
		{
			if ((nItem >= 0) && (lvhti.iSubItem >= 0) && (lvhti.flags & LVHT_ONITEMLABEL))
			{
				m_strToolTipText	= GetItemText (nItem, lvhti.iSubItem);

				CRect rectSubItemClient, rectSubItemScreen;
				GetSubItemRect(nItem, lvhti.iSubItem, LVIR_LABEL, rectSubItemClient);
				rectSubItemScreen = rectSubItemClient;
				ClientToScreen(&rectSubItemScreen);

				ptToolTipDisplay.x = rectSubItemScreen.left;
				ptToolTipDisplay.y = rectSubItemScreen.top;

				//
				//  Figure out how big the text in this cell is

				HDC hDC = ::GetDC(m_hWnd);
				ASSERT(hDC);

				CFont *pFont = GetFont();
				HFONT hOldFont = (HFONT) ::SelectObject(hDC, (HFONT) *pFont);

				SIZE size;
				::GetTextExtentPoint32 (hDC, m_strToolTipText, m_strToolTipText.GetLength(), &size);
				::SelectObject(hDC, hOldFont);
				::ReleaseDC(m_hWnd, hDC);

				//
				//  If it's bigger than the cell, then pop up the tool tip.  I'm having trouble
				//  getting the item's cell right because sometimes it needs to disregard tool tips
				//  and sometimes it doesn't.

				if (rectSubItemClient.right > rectClient.Width ())
					 rectSubItemClient.right -= ::GetSystemMetrics(SM_CXVSCROLL);
				if (size.cx > (rectSubItemClient.Width() - 5))
				{
					bNeedToDisplayTip	= true;
					SetTipTypeToDisplay (eVpLcTipTypeListItem);
					::SendMessage(m_hWndToolTip, WM_SETFONT, (WPARAM) GetFont ()->GetSafeHandle (), FALSE);
				}
			}
		}

		if (bNeedToDisplayTip)
		{
			m_ToolInfo.lpszText = (LPTSTR)(LPCTSTR) m_strToolTipText;
			
			m_oToolTipInfo.m_bBoldLabel =	true;
			m_oToolTipInfo.m_bDrawDescription = true;
			m_oToolTipInfo.m_bDrawIcon = true;
			m_oToolTipInfo.m_bRoundedCorners = true;
			m_oToolTipInfo.m_bDrawSeparator = true;

			m_oToolTip.SetToolInfo (&m_ToolInfo);
			m_oToolTip.SetParams (&m_oToolTipInfo);
			m_oToolTip.Popup ();

			::SendMessage(m_hWndToolTip, TTM_UPDATETIPTEXT, 0,		(LPARAM)&m_ToolInfo);
			::SendMessage(m_hWndToolTip, TTM_TRACKPOSITION, 0,		(LPARAM)MAKELONG(ptToolTipDisplay.x, ptToolTipDisplay.y));

			//
			//  Moving this to a timer
			//			::SendMessage(m_hWndToolTip, TTM_TRACKACTIVATE, TRUE,	(LPARAM)(LPTOOLINFO)&m_ToolInfo);

			SetTimer (VP_LIST_CTRL_SHOW_TIP_TIMER_ID, VP_LIST_CTRL_SHOW_TIP_TIMER_MS, NULL);
		}
		else
		{
			HideToolTip ();
		}
	} // end if point is in our client area
	else
	{
		HideToolTip ();
	}

	this->SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);	
	__super::OnMouseMove(nFlags, point);

	m_ptLastTipPos = point;
}

///////////////////////////////////////////////////////////////////////////////
// OnTimer
void CMyListCtrl::OnTimer (UINT_PTR nIDEvent) 
{
	//
	//  Don't think a timer will fire without a valid window, but not 100% positive
	//  and running into issues where our client windows aren't fully formed, so...

	if (NULL == GetSafeHwnd ())
		return;

	if (VP_LIST_CTRL_SHOW_TIP_TIMER_ID == nIDEvent)
	{
		//
		//  Show our tooltip and start our out of rect check timer

		KillTimer (VP_LIST_CTRL_SHOW_TIP_TIMER_ID);
		::SendMessage (m_hWndToolTip, TTM_TRACKACTIVATE, TRUE,	(LPARAM)(LPTOOLINFO)&m_ToolInfo);
		SetTimer (VP_LIST_CTRL_OUT_OF_RECT_TIMER_ID, VP_LIST_CTRL_OUT_OF_RECT_TIMER_MS, NULL);
	}

	if (VP_LIST_CTRL_OUT_OF_RECT_TIMER_ID == nIDEvent)
	{
		CPoint ptClient;
		::GetCursorPos(&ptClient);
		ScreenToClient(&ptClient);

		//
		//  Problem is, the header is considered part of the client area, so check that, too

		CRect rectClient;
		GetClientRect(&rectClient);

		CHeaderCtrl* pHeader = GetHeaderCtrl ();
		if (NULL == pHeader)
			return;

		CRect rcHeader;
		pHeader->GetWindowRect (rcHeader);
		this->ScreenToClient (rcHeader);  // header rect now in terms of list ctrl coords

		DWORD dwStyle = GetStyle();
		if ((!rectClient.PtInRect(ptClient)) || ((dwStyle & WS_VISIBLE) == 0) || rcHeader.PtInRect (ptClient))
		{
			HideToolTip ();
		}
	}
} // end on timer




void CMyListCtrl::OnCustomDrawList ( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	static bool bHighlighted = false;
	
    *pResult = CDRF_DODEFAULT;

    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
        *pResult = CDRF_NOTIFYITEMDRAW;
	}
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
//        int iRow = (int)pLVCD->nmcd.dwItemSpec;
		
#ifdef asdf
		bHighlighted = IsRowHighlighted(m_hWnd, iRow);
		if (bHighlighted)
		{
			pLVCD->clrText   = g_MyClrFgHi; // Use my foreground hilite color
			pLVCD->clrTextBk = g_MyClrBgHi; // Use my background hilite color
			
			EnableHighlighting(m_hWnd, iRow, false);
		}
#endif

		*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
		
	}
	else if(CDDS_ITEMPOSTPAINT == pLVCD->nmcd.dwDrawStage)
	{
	if (bHighlighted)
      {
//        int  iRow = (int)pLVCD->nmcd.dwItemSpec;

//        EnableHighlighting(m_hWnd, iRow, true);
      }

      *pResult = CDRF_DODEFAULT;

	}
} // end on custom draw list





void CMyListCtrl::SelectAll ()
{
	for (int i = 0; i < GetItemCount (); i ++)
	{
		this->SetItemState (i, LVIS_SELECTED, LVIS_SELECTED);
	}

}


void CMyListCtrl::UnselectAll ()
{
	SelectNone ();
}


void CMyListCtrl::SelectNone ()
{
	for (int i = 0; i < GetItemCount (); i ++)
	{
		this->SetItemState (i, (UINT) ~LVIS_SELECTED, LVIS_SELECTED);
	}
}



int	CMyListCtrl::FindStringExact (CString strToFind)
{
	LVFINDINFO fi;

	fi.psz		= strToFind;
	fi.flags	= LVFI_STRING;

	return FindItem (&fi);
}

void CMyListCtrl::SelectItem (int nItemIndex, bool bSetSelectionMark /* = false */)
{
	if (bSetSelectionMark) {
		SetItemState (nItemIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		EnsureVisible (nItemIndex, false);
	}
	else
		SetItemState (nItemIndex, LVIS_SELECTED, LVIS_SELECTED);

}


void CMyListCtrl::UnselectItem (int nItemIndex)
{
	//	if (0 >= nItemIndex && nItemIndex < GetItemCount ())
	{
		SetItemState (nItemIndex, 0, LVIS_SELECTED);
	}
}


//
//
//
//
int CMyListCtrl::FindByItemData (DWORD_PTR nData)
{
	LVFINDINFO fi;

	fi.lParam	= nData;
	fi.flags	= LVFI_PARAM;

	return FindItem (&fi);
}


//************************************
// Method:    GetFirstSelectedItem
// FullName:  CMyListCtrl::GetFirstSelectedItem
// Access:    public 
// Returns:   int
// Qualifier:
//
//  Returns the index of the first selected item.
//
//************************************
int	CMyListCtrl::GetFirstSelectedItem ()
{
	POSITION pos = GetFirstSelectedItemPosition ();
	if (NULL == pos)
		return -1;

	return GetNextSelectedItem (pos);
}


//************************************
// Method:    MySetCursor
// FullName:  CMyListCtrl::MySetCursor
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: HCURSOR hCursor
//************************************
void CMyListCtrl::MySetCursor (HCURSOR hCursor)
{
	m_bSettingCursor = true;
	SetCursor (hCursor);
	m_bSettingCursor = false;

} // end my set cursor


//************************************
// Method:    SetTipTypeToDisplay
// FullName:  CMyListCtrl::SetTipTypeToDisplay
// Access:    protected 
// Returns:   void
// Qualifier:
// Parameter: EVpListCtrlTipTypeToShow eToShow
//************************************
void CMyListCtrl::SetTipTypeToDisplay (EVpListCtrlTipTypeToShow eToShow)
{
	if (eToShow != m_eTipTypeToShow) {
		KillTimer (VP_LIST_CTRL_SHOW_TIP_TIMER_ID);
	}

	m_eTipTypeToShow = eToShow;
}



//************************************
// Method:    HideToolTip
// FullName:  CMyListCtrl::HideToolTip
// Access:    public 
// Returns:   void
// Qualifier:
//************************************
void CMyListCtrl::HideToolTip ()
{
	KillTimer (VP_LIST_CTRL_SHOW_TIP_TIMER_ID);
	KillTimer (VP_LIST_CTRL_OUT_OF_RECT_TIMER_ID);

	m_oToolTip.Pop ();
	::SendMessage(m_hWndToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)(LPTOOLINFO) &m_ToolInfo);
}


