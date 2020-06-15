
//****************************************************
//
//	cwh --	changed the tip control to be based off
//			of the CTitleListCtrl in the Title Window
//			instead of off of a CListBox.  This allows
//			multiple columns of data to be shown in the
//			title window
//
//****************************************************


//****************************************************
//
//	cwh --	2/8/01 -- changed the tip control so that 
//			it can be used by both list controls and
//			list boxes
//
//****************************************************


#include "stdafx.h"

#include "TitleTip.h"
#include "HScrollListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



LPCTSTR CTitleTip::m_pszWndClass = NULL;

CTitleTip::CTitleTip()
:m_nNoIndex(-1)//, m_nAutoPopDelay(500), m_LastMousePosition(0,0) // (commented out stuff for delay)
{
	// Register the window class if it has not already been registered by
	// previous instantiation of CTitleTip.
	if (m_pszWndClass == NULL)
	{
		m_pszWndClass = AfxRegisterWndClass(CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC);
	}
	m_nItemIndex = m_nNoIndex;
	m_pListBox = NULL;

//	m_nTimerId = 0; // delay
}

CTitleTip::~CTitleTip()
{
}


//------------------------------------------------
//
//	Create Tool Tips for a CListBox
//
BOOL CTitleTip::Create(CListBox* pParentWnd)
{
	ASSERT_VALID(pParentWnd);
	m_pListBox = pParentWnd;

	// Don't add border to regular (non owner-draw) listboxes because
	// owner-draw item automatically adds border.
	DWORD dwStyle = WS_POPUP;
	if (!IsListOwnerDraw())
	{
		dwStyle |= WS_BORDER;
	}

	return CreateEx(0, m_pszWndClass, NULL, dwStyle, 0, 0, 0, 0, pParentWnd->GetSafeHwnd(), NULL, NULL);
}



BOOL CTitleTip::IsListOwnerDraw()
{
	return FALSE;
}



void CTitleTip::Show(CRect DisplayRect, int nItemIndex)
{
	if (m_pListBox && (nItemIndex >= m_pListBox->GetCount()))
		return;
	else if (m_pListBox == NULL)
		return;

	ASSERT(nItemIndex >= 0);
	ASSERT(::IsWindow(m_hWnd));
	ASSERT(!DisplayRect.IsRectEmpty());


	// Invalidate if new item.
	if (m_nItemIndex != nItemIndex)
	{
		m_nItemIndex = nItemIndex;
		InvalidateRect(NULL);
	}

	// Adjust window position and visibility.
	CRect WindowRect;
	GetWindowRect(WindowRect);
	int nSWPFlags = SWP_SHOWWINDOW | SWP_NOACTIVATE;
	if (WindowRect == DisplayRect)
	{
		nSWPFlags |= SWP_NOMOVE | SWP_NOSIZE;
	}
	VERIFY(SetWindowPos(&wndTop, 
		DisplayRect.left, DisplayRect.top, DisplayRect.Width(), DisplayRect.Height(), 
		nSWPFlags));
/* delay
	if (m_nTimerId > 0)
	{
		VERIFY(KillTimer(m_nTimerId));
		m_nTimerId = 0;
	}
	VERIFY(::GetCursorPos(&m_LastMousePosition));
	m_nTimerId = SetTimer(1, m_nAutoPopDelay, NULL);
	ASSERT(m_nTimerId != 0);
*/

}


void CTitleTip::Hide()
{

	ASSERT(::IsWindow(m_hWnd));
	ShowWindow(SW_HIDE);

/* delay
	if (m_nTimerId > 0)
	{
		VERIFY(KillTimer(m_nTimerId));
		m_nTimerId = 0;
	}
*/

}
 


BEGIN_MESSAGE_MAP(CTitleTip, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTitleTip message handlers

void CTitleTip::OnPaint() 
{
	ASSERT(m_nItemIndex != m_nNoIndex);

	CPaintDC DC(this);

	int nSavedDC = DC.SaveDC();

	CRect ClientRect;
	GetClientRect(ClientRect);

	if (IsListOwnerDraw())
	{
		// Let the list do the real drawing.
		DRAWITEMSTRUCT DrawItemStruct;

		DrawItemStruct.CtlType = ODT_LISTBOX;
		if (m_pListBox)
			DrawItemStruct.CtlID = m_pListBox->GetDlgCtrlID();
		DrawItemStruct.itemID = m_nItemIndex;
		DrawItemStruct.itemAction = ODA_DRAWENTIRE;
		if (m_pListBox)
			DrawItemStruct.hwndItem = m_pListBox->GetSafeHwnd();
		DrawItemStruct.hDC = DC.GetSafeHdc();
		DrawItemStruct.rcItem = ClientRect;
		if (m_pListBox)
			DrawItemStruct.itemData = m_pListBox->GetItemData(m_nItemIndex);
		//cwh -- DrawItemStruct.itemState = (m_pListCtrl->GetSel(m_nItemIndex) > 0 ? ODS_SELECTED : 0);

		if (m_pListBox)
		{
			if (m_pListBox->GetStyle() & LBS_MULTIPLESEL)
			{
				if (m_pListBox->GetCurSel() == m_nItemIndex)
				{
					DrawItemStruct.itemState |= ODS_FOCUS;
				}
			}
			else
			{
				DrawItemStruct.itemState |= ODS_FOCUS;
			}

			m_pListBox->DrawItem(&DrawItemStruct);
		}
	}
	else
	{
		CFont* pFont = NULL;

		// Do all of the drawing ourselves
		if (m_pListBox)
			pFont = m_pListBox->GetFont();
		else {
			ASSERT (0);
			return;
		}

		ASSERT_VALID(pFont);
		DC.SelectObject(pFont);
	

		//set the background of tip window to white
		COLORREF clrBackground = RGB(255, 255, 255);

		//set the background of the top window to the
		//highlight color if the user has selected the
		//row which the tip window is being shown for
		if (m_pListBox && m_pListBox->GetSel(m_nItemIndex))
		{
			DC.SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			clrBackground = ::GetSysColor(COLOR_HIGHLIGHT);
		}

		// Draw background
		DC.FillSolidRect(ClientRect, clrBackground);

		// Draw text of item
		CString strItem;
		if (m_pListBox)
		{
			//
			//  For list boxes we support alternate title tip text only if it's an HScrollListBox

			if (m_pListBox->IsKindOf (RUNTIME_CLASS(HScrollListBox)))
			{
				strItem.Empty ();
				HScrollListBox *pListBox = (HScrollListBox *) m_pListBox;
				if (pListBox->GetUseAltTipText ())
					strItem = pListBox->GetAltTipText (m_nItemIndex);

				if (strItem.IsEmpty ())
					m_pListBox->GetText (m_nItemIndex, strItem);
			} // end if we're using alternate tip text
			else
				m_pListBox->GetText(m_nItemIndex, strItem);
		}

//		ASSERT(!strItem.IsEmpty());
		DC.SetBkMode(TRANSPARENT);
		DC.TextOut(1, -1, strItem);
	}

	DC.RestoreDC(nSavedDC);

	// Do not call CWnd::OnPaint() for painting messages
}

/* delay
void CTitleTip::OnTimer (UINT_PTR nIDEvent) 
{
	CPoint CurrentMousePosition;

	VERIFY(::GetCursorPos(&CurrentMousePosition));
	if (CurrentMousePosition == m_LastMousePosition)
	{
		Hide();
	}
	else
	{
		m_LastMousePosition = CurrentMousePosition;
	}

	
//	CWnd::OnTimer(nIDEvent);
}
*/


BOOL CTitleTip::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (NULL != m_pListBox)
		return (BOOL) m_pListBox->SendMessage (WM_MOUSEWHEEL, (zDelta <<16) | nFlags, pt.x | (pt.y<<16));
	else
		return __super::OnMouseWheel(nFlags, zDelta, pt);
}


