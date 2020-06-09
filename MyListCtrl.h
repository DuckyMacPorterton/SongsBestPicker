#pragma once
#include <afxtooltipctrl.h>
#include "MyListCtrlHeader.h"


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

	CMyListCtrlHeaderCtrl   m_oHeaderCtrl;
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

	void OnMouseMove(UINT nFlags, CPoint point);
	void OnTimer (UINT_PTR nIDEvent);

	void OnCustomDrawList (NMHDR* pNMHDR, LRESULT* pResult);

	LRESULT OnHeaderDragCol (WPARAM wSource, LPARAM lDest);

	DECLARE_MESSAGE_MAP()
};


