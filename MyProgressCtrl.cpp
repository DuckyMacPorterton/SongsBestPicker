#include "stdafx.h"
#include "MyProgressCtrl.h"


BEGIN_MESSAGE_MAP(CMyProgressCtrl, CProgressCtrl)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()





CMyProgressCtrl::CMyProgressCtrl ()
{
	
}




//************************************
// Method:    OnLButtonDown
// FullName:  CSongsBestPickerDlg::OnLButtonDown
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: UINT nFlags
// Parameter: CPoint point
//
//  Trying to handle clicks on the progress bar
//
//************************************
void CMyProgressCtrl::OnLButtonDown (UINT nFlags, CPoint ptClickClient) 
{
	//
	//  Woot!  Ok, send a message to our parent and tell it what happened.

	CRect rcClient;
	GetClientRect (rcClient);

	int		nDeltaX		= rcClient.right - ptClickClient.x;
	int		nPercent	= 100 - (int) (((float)nDeltaX / rcClient.Width ()) * 100);

	//
	//  Passing floats isn't as simple as casting it to WPARAM, apparently, and this is easy.

	CWnd* pParent = GetParent ();
	if (NULL != pParent)
		pParent->SendMessage (UWM_CLICKED_PROGRESS_CTRL, (WPARAM) nPercent);

//	__super::OnLButtonDown (nFlags, ptClickScreen);

} // end on lbutton down
