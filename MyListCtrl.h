#pragma once
#include <afxtooltipctrl.h>


static const UINT UWM_VP_AUTO_CLASSIFY_RECORDS	= ::RegisterWindowMessage(_T("UWM_VP_AUTO_CLASSIFY_RECORDS"));

enum EVpListCtrlTipTypeToShow
{
	eVpLcTipTypeUnknown,
	eVpLcTipTypeProgress,
	eVpLcTipTypeFieldName,
	eVpLcTipTypeClassify,
	eVpLcTipTypeListItem
};

class CMyListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CMyListCtrl)

public:
	CMyListCtrl();
	virtual ~CMyListCtrl();


//	CMyListCtrlHeader m_oRecordClassificationHeaderCtrl;

	void	SelectAll ();
	void	SelectNone ();
	void	UnselectAll ();

	int		FindStringExact (CString strToFind);
	int		FindByItemData (DWORD_PTR nData);

	void	SelectItem (int nItemIndex, bool bSetSelectionMark = false);
	void	UnselectItem (int nItemIndex);

	int		GetFirstSelectedItem ();

	void	MySetCursor (HCURSOR hCursor);
	bool	GetIsSettingCursor () {return m_bSettingCursor;};

	void	HideToolTip ();

protected:
	bool			m_bSettingCursor		= false;

	bool			m_bToolTipDisplayed		= false;
	CPoint			m_ptLastTipPos;
	HWND			m_hWndToolTip			= NULL;
	TOOLINFO		m_ToolInfo;
	bool			m_bCreatedDynamically	= false;

	EVpListCtrlTipTypeToShow	m_eTipTypeToShow	= eVpLcTipTypeUnknown;

	CString			m_strToolTipText;

	CMFCToolTipInfo	m_oToolTipInfo;
	CMFCToolTipCtrl	m_oToolTip;

protected:
	void			SetTipTypeToDisplay (EVpListCtrlTipTypeToShow eToShow);

	BOOL PreCreateWindow (CREATESTRUCT &cs);
	int  OnCreate (LPCREATESTRUCT lpCreateStruct);

	virtual void PreSubclassWindow();
	virtual BOOL PreTranslateMessage (MSG*pMsg);

			void CreateToolTip ();

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer (UINT_PTR nIDEvent);

	afx_msg void OnCustomDrawList (NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};


