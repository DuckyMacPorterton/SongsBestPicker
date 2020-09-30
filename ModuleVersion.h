#pragma once


////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
//
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//

// tell linker to link with version.lib for VerQueryValue, etc.
#pragma comment(linker, "/defaultlib:version.lib")

/*
#ifndef DLLVERSIONINFO
// following is from shlwapi.h, in November 1997 release of the Windows SDK

typedef struct _DllVersionInfo
{
    DWORD cbSize;
    DWORD dwMajorVersion;                   // Major version
    DWORD dwMinorVersion;                   // Minor version
    DWORD dwBuildNumber;                    // Build number
    DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

// Platform IDs for DLLVERSIONINFO
#define DLLVER_PLATFORM_WINDOWS      0x00000001      // Windows 95
#define DLLVER_PLATFORM_NT           0x00000002      // Windows NT

#endif // DLLVERSIONINFO
*/


//////////////////
// CModuleVersion version info about a module.
// To use:
//
// CModuleVersion ver
// if (ver.GetFileVersionInfo("_T("mymodule))) {
//    // info is in ver, you can call GetValue to get variable info like
//    CString s = ver.GetValue(_T("CompanyName"));
// }
//
// You can also call the static fn DllGetVersion to get DLLVERSIONINFO.
//
class CModuleVersion : public VS_FIXEDFILEINFO {
protected:
   BYTE* m_pVersionInfo;   // all version info

   struct TRANSLATION {
      WORD langID;         // language ID
      WORD charset;        // character set (code page)
   } m_translation;

public:
      BOOL IsWinNT();
   CModuleVersion();
   virtual ~CModuleVersion();

   BOOL     GetFileVersionInfo(LPCTSTR modulename);
   CString  GetValue(LPCTSTR lpKeyName);
   static BOOL DllGetVersion(LPCTSTR modulename, DLLVERSIONINFO& dvi);
};







class CVersionInfo
{
public:
	CVersionInfo(void);
	~CVersionInfo(void);


	bool operator< (const CVersionInfo& vi) const;
	bool operator> (const CVersionInfo& vi) const;
	bool operator== (const CVersionInfo& vi) const;

	bool	SetVersionInfo (int nMajor, int nMinor, int nBuildCode, int nExtra);
	bool	SetVersionInfo (CString strMajorDotMinor, CString strBuildCode, CString strExtra);

	CString	GetVersionString ();

	int		GetVersionMajor ()		{return m_nMajor;};
	int		GetVersionMinor ()		{return m_nMinor;};
	int		GetVersionBuildCode ()	{return m_nBuildCode;};
	int		GetVersionExtra ()		{return m_nExtra;};

	void	Reset () {m_nMajor = m_nMinor = m_nBuildCode = m_nExtra = 0;};

protected:

	int	m_nMajor		= 0;
	int	m_nMinor		= 0;
	int	m_nBuildCode	= 0;
	int m_nExtra		= 0;

};
