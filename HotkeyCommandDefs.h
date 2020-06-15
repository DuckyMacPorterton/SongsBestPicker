#pragma once

#include "resource.h"
#include "VirtKeys.h"


#define ID_PLAY_OR_PAUSE_SONG			WM_USER + 1
#define ID_NEXT_SONG					WM_USER + 2
#define ID_PREV_SONG					WM_USER + 3

#define ID_RANK_1						WM_USER + 4
#define ID_RANK_2						WM_USER + 5
#define ID_RANK_3						WM_USER + 6
#define ID_RANK_4						WM_USER + 7
#define ID_RANK_5						WM_USER + 8

#define ID_SHOW_OUR_APP					WM_USER + 9


struct HotkeyCommandDefinitionStruct
{
	int          nCommandID;
	const TCHAR *strCommandName;

	UINT         nDefaultModifiers;
	UINT         nDefaultKey;
};


//
//  Here are our commands...

static HotkeyCommandDefinitionStruct HKInit[] = {

	{ID_SHOW_OUR_APP,			L"Bring this glorious app to the front", MOD_CONTROL | MOD_SHIFT | MOD_ALT, VK_A},

	{ID_PLAY_OR_PAUSE_SONG,		L"Play or Pause Song",	MOD_CONTROL | MOD_SHIFT,	VK_A},
	{ID_NEXT_SONG,				L"Next Song",			MOD_CONTROL | MOD_SHIFT,	VK_NEXT},
	{ID_PREV_SONG,				L"Prev Song",			MOD_CONTROL | MOD_SHIFT,	VK_PRIOR},

	{ID_RANK_1,					L"Rank 1",				MOD_CONTROL | MOD_SHIFT,	VK_1},
	{ID_RANK_2,					L"Rank 2",				MOD_CONTROL | MOD_SHIFT,	VK_2},
	{ID_RANK_3,					L"Rank 3",				MOD_CONTROL | MOD_SHIFT,	VK_3},
	{ID_RANK_4,					L"Rank 4",				MOD_CONTROL | MOD_SHIFT,	VK_4},
	{ID_RANK_5,					L"Rank 5",				MOD_CONTROL | MOD_SHIFT,	VK_5},

}; // end command definition


#define HOTKEY_INIT_NUM_COMMANDS (sizeof (HKInit) / sizeof (HotkeyCommandDefinitionStruct))

