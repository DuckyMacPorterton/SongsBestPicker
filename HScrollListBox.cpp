
//****************************************************
//
//	cwh -- 2/8/01 -- I changed HScrollListBox so that is can
//	be used to show pop up tool tips for subclasses list boxes.
//	It no longer launches the record dialog box upon double clicking
//	a name.  This is so that the HScrollListBox can be used for
//	cases than just the TitleWnd
//
//******************************************************


#include "stdafx.h"
#include "HScrollListBox.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(HScrollListBox, CListBox);



/////////////////////////////////////////////////////////////////////////////
// HScrollListBox

HScrollListBox::HScrollListBox()
: m_LastMouseMovePoint(0, 0), m_nNoIndex(-1)
{
	m_nMaxTextExtent = 0;
	m_bMouseCaptured = FALSE;

	m_nCurSel = -1;

	m_bUseAltTipText = false;
	m_AltTipTextArray.SetSize (0);
}

HScrollListBox::~HScrollListBox()
{
	ASSERT(!m_bMouseCaptured);
}




BOOL HScrollListBox::PreTranslateMessage(MSG* pMsg)
{
	return __super::PreTranslateMessage(pMsg);
}







BEGIN_MESSAGE_MAP(HScrollListBox, CListBox)
	//{{AFX_MSG_MAP(HScrollListBox)
	ON_WM_MOUSEMOVE()
	//ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	//ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEWHEEL()
	//ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
	//ON_MESSAGE(LB_ADDSTRING, OnContentChanged)
	//ON_MESSAGE(LB_INSERTSTRING, OnContentChanged)
	//ON_MESSAGE(LB_DELETESTRING, OnContentChanged)
	//ON_MESSAGE(LB_ADDFILE, OnContentChanged)
	//ON_MESSAGE(LB_DIR, OnContentChanged)
	//ON_MESSAGE(LB_RESETCONTENT, OnContentChanged)
	//ON_MESSAGE(LB_SETCARETINDEX, OnContentChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HScrollListBox message handlers

int HScrollListBox::AddString( LPCTSTR lpszItem )
{
	CDC *pCDC = GetDC();
	
	// Select our List Box font and get the length of the text
	CFont *pWndFont = GetFont( ) ;
	pCDC->SelectObject( pWndFont );
	CSize Size = pCDC->GetTextExtent( lpszItem );
	ReleaseDC( pCDC );

	// Update Horizontal Extent if necessary
	if( Size.cx > m_nMaxTextExtent )
	{
		m_nMaxTextExtent = Size.cx;
		SetHorizontalExtent( Size.cx + HORIZONTAL_FUDGE );
	}

	return CListBox::AddString( lpszItem );
}

int HScrollListBox::InsertString( int nIndex, LPCTSTR lpszItem )
{
	CDC *pCDC = GetDC();
	
	// Select our List Box font and get the length of the text
	CFont *pWndFont = GetFont( ) ;
	pCDC->SelectObject( pWndFont );
	CSize Size = pCDC->GetTextExtent( lpszItem );
	ReleaseDC( pCDC );

	// Update Horizontal Extent if necessary
	if( Size.cx > m_nMaxTextExtent )
	{
		m_nMaxTextExtent = Size.cx;
		SetHorizontalExtent( Size.cx + HORIZONTAL_FUDGE );
	}

	return CListBox::InsertString( nIndex, lpszItem );
}

int HScrollListBox::DeleteString( UINT nIndex )
{
	CString strText;
	GetText( nIndex, strText );

	CDC *pCDC = GetDC();
	CFont *pWndFont = GetFont( ) ;
	pCDC->SelectObject( pWndFont );
	CSize Size = pCDC->GetTextExtent( strText );

	int nNumInList = CListBox::DeleteString( nIndex );
	if( Size.cx == m_nMaxTextExtent )
	{
		// We have deleted the longest line, go find the longest one remaining
		m_nMaxTextExtent = 0;
		int i;
		for( i = 0; i < nNumInList; i++ )
		{
			GetText( i, strText );
			Size = pCDC->GetTextExtent( strText );
			if( Size.cx > m_nMaxTextExtent )
				m_nMaxTextExtent = Size.cx;
		}

		// Set the horizontal scroll width
		SetHorizontalExtent( m_nMaxTextExtent + HORIZONTAL_FUDGE );
	}
	ReleaseDC( pCDC );
	return nNumInList;
}
  
void HScrollListBox::ResetContent( )
{
	m_nMaxTextExtent = 0;
	SetHorizontalExtent( 0 );
	CListBox::ResetContent();
}



void HScrollListBox::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (point != m_LastMouseMovePoint && IsAppActive())
	{
		m_LastMouseMovePoint = point;

		int nIndexHit = m_nNoIndex;

		CRect ClientRect;
		GetClientRect(ClientRect);
		if (ClientRect.PtInRect(point))
		{
			// Hit test.
			for (int n = 0; nIndexHit == m_nNoIndex && n < GetCount(); n++)
			{
				CRect ItemRect;
				GetItemRect(n, ItemRect);
				if (ItemRect.PtInRect(point))
				{
					nIndexHit = n;	
				}
			}
		}
		AdjustTitleTip(nIndexHit);
	}
	
	CListBox::OnMouseMove(nFlags, point);
}



/*
void HScrollListBox::OnSelchange() 
{
	int nSelIndex;
	if (GetStyle() & LBS_MULTIPLESEL)
	{
		nSelIndex = GetCaretIndex();	
	}
	else
	{
		nSelIndex = GetCurSel();
	}
	AdjustTitleTip(nSelIndex);
	m_TitleTip.InvalidateRect(NULL);
	m_TitleTip.UpdateWindow();
}



void HScrollListBox::OnDestroy() 
{
	AdjustTitleTip(m_nNoIndex);
	m_TitleTip.DestroyWindow();
	CListBox::OnDestroy();
	
}
*/

void HScrollListBox::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_bMouseCaptured)
	{
		ReleaseCapture();
		m_bMouseCaptured = FALSE;
	}

	
	CListBox::OnLButtonDown(nFlags, point);

	if (m_TitleTip.IsWindowVisible())
	{
		m_TitleTip.InvalidateRect(NULL);
		if (this != GetCapture())
		{
			CaptureMouse();
		}
	}

}

void HScrollListBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CListBox::OnLButtonUp(nFlags, point);

	if (this != GetCapture() && m_TitleTip.IsWindowVisible())
	{
		CaptureMouse();
	}

}


void HScrollListBox::OnLButtonDblClk (UINT nFlags, CPoint point)
{
	CListBox::OnLButtonDblClk (nFlags, point);

	if (m_bMouseCaptured)
	{
		VERIFY(ReleaseCapture());
		m_bMouseCaptured = FALSE;
	}
} // end on left button double click


/*
BOOL HScrollListBox::PreTranslateMessage(MSG* pMsg) 
{
	switch (pMsg->message)
	{
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			// Make the active view because that is the default
			// behaviour caused by WM_MOUSEACTIVATE when NO TitleTip
			// is over this window.
			AdjustTitleTip(m_nNoIndex);
			SFLFrameWndEx* pFrameWnd = (SFLFrameWndEx *)GetParentFrame();
			if (pFrameWnd)
			{
				BOOL bDone = FALSE;
				CWnd* pWnd = this;
				while (!bDone)
				{
					pWnd = pWnd->GetParent();
					if (!pWnd || pWnd == pFrameWnd)
					{
						bDone = TRUE;
					}
					else if (pWnd->IsKindOf(RUNTIME_CLASS(CView)))
					{
						pFrameWnd->SetActiveView((CView*)pWnd);
						bDone = TRUE;
					}
				}
			}
			break;
	}
	
	return CListBox::PreTranslateMessage(pMsg);
}


void HScrollListBox::OnKillFocus(CWnd* pNewWnd) 
{
	CListBox::OnKillFocus(pNewWnd);
	if (pNewWnd != &m_TitleTip)
	{
		AdjustTitleTip(m_nNoIndex);
	}
}

LONG HScrollListBox::OnContentChanged(UINT, LONG)
{
	// Turn off title tip.
	AdjustTitleTip(m_nNoIndex);
	return Default();
}
*/

int HScrollListBox::GetIdealItemRect(int nIndex, LPRECT lpRect)
{
	// Calculate the ideal rect for an item. The ideal rect is dependent
	// on the length of the string. This only works for regular 
	// (non owner-draw)listboxes.
	if (NULL == lpRect)
	{
		ASSERT (0);
		return false;
	}

	ASSERT(nIndex >= 0);
	int	nStatus = GetItemRect(nIndex, lpRect);	
	if (LB_ERR  == nStatus)
		return false;

	//if (nStatus != LB_ERR && !(dwStyle & LBS_OWNERDRAWFIXED) && !(dwStyle & LBS_OWNERDRAWVARIABLE))
	//{
		CString strItem;

		if (m_bUseAltTipText)
			strItem = GetAltTipText (nIndex);
		else
			GetText(nIndex, strItem);
		
		if (!strItem.IsEmpty())
		{
			// Calulate the ideal text length.
			CClientDC DC(this);
			CFont* pOldFont = DC.SelectObject(GetFont());
			CSize ItemSize = DC.GetTextExtent(strItem);
			DC.SelectObject(pOldFont);

			// Take the maximum of regular width and ideal width.
			const int cxEdgeSpace = 2;
			lpRect->right = max(lpRect->right, 
				lpRect->left + ItemSize.cx + (cxEdgeSpace * 2));
		}
	//}
	//else
	//{
	//	TRACE("Owner-draw listbox detected - override CTitleTipListBox::GetIdeaItemRect()\n");
	//}
	return nStatus;
}

void HScrollListBox::AdjustTitleTip(int nNewIndex)
{
	if (!::IsWindow(m_TitleTip.m_hWnd))
	{
		VERIFY(m_TitleTip.Create(this));
	}

	if (nNewIndex == m_nNoIndex)
	{
		m_TitleTip.Hide();
	}
	else
	{
		CRect IdealItemRect;
		GetIdealItemRect(nNewIndex, IdealItemRect);
		CRect ItemRect;
		GetItemRect(nNewIndex, ItemRect);
		if (ItemRect == IdealItemRect)
		{
			m_TitleTip.Hide();
		}
		else
		{
			// Adjust the rect for being near the edge of screen.
			ClientToScreen(IdealItemRect);

//#define OldWays
#ifdef OldWays
			int nScreenWidth = ::GetSystemMetrics(SM_CXVIRTUALSCREEN); // SM_CXFULLSCREEN);
			if (IdealItemRect.right > nScreenWidth)
			{
				IdealItemRect.OffsetRect(nScreenWidth - IdealItemRect.right, 0);
			}
			if (IdealItemRect.left < 0)
			{
				IdealItemRect.OffsetRect(-IdealItemRect.left, 0);
			}
#else

			CUtils::EnsureWindowIsVisible (IdealItemRect);
#endif

			m_TitleTip.Show(IdealItemRect, nNewIndex);  
		}
	}

	if (m_TitleTip.IsWindowVisible())
	{
		// Make sure we capture mouse so we can detect when to turn off 
		// title tip.
		if (!m_bMouseCaptured && GetCapture() != this)
		{
			CaptureMouse();
		}
	}
	else
	{
		// The tip is invisible so release the mouse.
		if (m_bMouseCaptured)
		{
			VERIFY(ReleaseCapture());
			m_bMouseCaptured = FALSE;
		}
	}
}

void HScrollListBox::CaptureMouse()
{
	ASSERT(!m_bMouseCaptured);
	CPoint Point;
	VERIFY(GetCursorPos(&Point));
	ScreenToClient(&Point);
	m_LastMouseMovePoint = Point;
	SetCapture();
	m_bMouseCaptured = TRUE;
}


BOOL HScrollListBox::IsAppActive()
{
	/*
	BOOL bAppActive = FALSE;

	CWnd* pTopParent = GetTopLevelParent();
	CWnd* pActiveWnd = GetActiveWindow();
	if (pActiveWnd == pTopParent || pTopParent->IsChild(pActiveWnd))
	{
		bAppActive = TRUE;
	}
	return bAppActive;
	*/

	//cwh -- 2/8/01 -- for now just claim
	//that the control is always active.  This
	//only works if the HScrollListBox is in
	//a dialog box.
	return TRUE;
}



void HScrollListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	//this is only right here as a cludge
	m_BaseTextHeight = 17;

	lpMIS->itemHeight = GetItemHeight( lpMIS->itemID );
	if( lpMIS->itemHeight == 0 )
		lpMIS->itemHeight = m_BaseTextHeight;
}

void HScrollListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* 		pDC = CDC::FromHandle(lpDIS->hDC);;
	COLORREF 	brushColor;
	CString		tStr;

	if (lpDIS->itemState & ODS_SELECTED)
	{
		// item has been selected - hilite frame
		brushColor = GetSysColor(COLOR_HIGHLIGHT);
		pDC->SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
		pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));		
	}
	else
	{
		brushColor = GetSysColor(COLOR_WINDOW);
		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
		pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	}
		                             
	CBrush	br(brushColor);		                             	
	pDC->FillRect(&lpDIS->rcItem, &br);
    
 	if( lpDIS->itemID != -1 )
   	{
   		CRect	rect;
   		rect.top = lpDIS->rcItem.top;
  		rect.bottom = lpDIS->rcItem.bottom;
   		rect.right = lpDIS->rcItem.right;
   		rect.left = lpDIS->rcItem.left;

		//draw item in list box
		GetText(lpDIS->itemID, tStr);
		pDC->DrawText(tStr, -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_TABSTOP | (2 << 8) );	//the (2<<8) means shift the number 3 into the high byte.  This is for the DT_SET_TABSTOP option
	}
}


int HScrollListBox::CompareItem(LPCOMPAREITEMSTRUCT)
{ 
	return 0; 
}



/////////////////////////////////////////////////////////////
//
//  S E T   A L T   T I P   T E X T
//
//  Sets the alternate text to use for tool tips for nIndex
//
void HScrollListBox::SetAltTipText (int nIndex, CString strText)
{
	m_AltTipTextArray.SetAtGrow (nIndex, strText);

} // end set alt tip text


/////////////////////////////////////////////////////////////
//
//  G E T   A L T   T I P   T E X T
//
//  Gets the alternate text to use for tool tips for nIndex
//  If the alt text isn't set returns the text for that item
//  from the list box
//
CString HScrollListBox::GetAltTipText (int nIndex)
{
	if ((nIndex < 0) || (nIndex > this->GetCount ()))
		return L"";

	//  So we have a valid nIndex for the listbox... check to see
	//  if it's also valid for our alt text array

	if (nIndex >= m_AltTipTextArray.GetSize ())
	{
		//  Just return the text from the list box

		CString str;
		GetText (nIndex, str);
		return str;
	}

	//
	//  Our index is valid, but if the alt text is empty return the list box text

	if (m_AltTipTextArray[nIndex].IsEmpty ())
	{
		CString str;
		GetText (nIndex, str);
		return str;
	}

	//
	//  Aha!  Actual alt text!  Return it...

	return m_AltTipTextArray[nIndex];

} // end get alt tip text



//////////////////////////////////////////////////////////////////////////////
//
//  SELECT STRING EXACT
//
//  Like SelectString but does a strcasecamp.  Which means it's not exact,
//  I suppose, but it does a full compare rather than just matching a prefix.
//
int HScrollListBox::SelectStringExact (CString strToSelect)
{
	for (int i = 0; i < GetCount (); i ++)
	{
		CString str; 
		GetText (i, str);

		if (str.CompareNoCase (strToSelect) == 0)
		{
			SetSel (i, true);
			return i;
		}

	}

	return LB_ERR;
} 



BOOL HScrollListBox::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return CListBox::OnMouseWheel(nFlags, zDelta, pt);
}


