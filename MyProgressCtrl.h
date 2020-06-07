#pragma once


static const UINT UWM_CLICKED_PROGRESS_CTRL = ::RegisterWindowMessage (L"UWM_CLICKED_PROGRESS_CTRL");

class CMyProgressCtrl :
	public CProgressCtrl
{
public:
	CMyProgressCtrl ();

protected:


public:
	DECLARE_MESSAGE_MAP()
	void	OnLButtonDown (UINT nFlags, CPoint point);

};

