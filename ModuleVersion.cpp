////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
// CModuleVersion provides an easy way to get version info
// for a module.(DLL or EXE).
//
#include "StdAfx.h"
#include "ModuleVersion.h"


CModuleVersion::CModuleVersion()
{
   m_pVersionInfo = NULL;           // raw version info data 
}

//////////////////
// Destroy: delete version info
//
CModuleVersion::~CModuleVersion()
{
   delete [] m_pVersionInfo;
}

//////////////////
// Get file version info for a given module
// Allocates storage for all info, fills "this" with
// VS_FIXEDFILEINFO, and sets codepage.
//
BOOL CModuleVersion::GetFileVersionInfo(LPCTSTR modulename)
{
   m_translation.charset = 1252;    // default = ANSI code page
   memset((VS_FIXEDFILEINFO*)this, 0, sizeof(VS_FIXEDFILEINFO));

   // get module handle
   TCHAR filename[_MAX_PATH];
   HMODULE hModule = ::GetModuleHandle(modulename);
   if (hModule==NULL && modulename!=NULL)
      return FALSE;

   // get module file name
   DWORD len = GetModuleFileName(hModule, filename,
      sizeof(filename)/sizeof(filename[0]));
   if (len <= 0)
      return FALSE;

   // read file version info
   DWORD dwDummyHandle; // will always be set to zero
   len = GetFileVersionInfoSize(filename, &dwDummyHandle);
   if (len <= 0)
      return FALSE;

   m_pVersionInfo = new BYTE[len]; // allocate version info
   if (!::GetFileVersionInfo(filename, 0, len, m_pVersionInfo))
      return FALSE;

   LPVOID lpvi;
   UINT iLen;
   if (!VerQueryValue(m_pVersionInfo, _T("\\"), &lpvi, &iLen))
      return FALSE;

   // copy fixed info to myself, which am derived from VS_FIXEDFILEINFO
   *(VS_FIXEDFILEINFO*)this = *(VS_FIXEDFILEINFO*)lpvi;

   // Get translation info
   if (VerQueryValue(m_pVersionInfo,
      _T("\\VarFileInfo\\Translation"), &lpvi, &iLen) && iLen >= 4) {
      m_translation = *(TRANSLATION*)lpvi;
      TRACE(_T("code page = %d\n"), m_translation.charset);
   }

   return dwSignature == VS_FFI_SIGNATURE;
}

//////////////////
// Get string file info.
// Key name is something like "CompanyName".
// returns the value as a CString.
//
CString CModuleVersion::GetValue(LPCTSTR lpKeyName)
{
   CString sVal;
   if (m_pVersionInfo) {

      // To get a string value must pass query in the form
      //
      //    "\StringFileInfo\<langID><codepage>\keyname"
      //
      // where <langID><codepage> is the languageID concatenated with the
      // code page, in hex. Wow.
      //
      CString query;
      query.Format(_T("\\StringFileInfo\\%04x%04x\\%s"),
                   m_translation.langID,
                   m_translation.charset,
                   lpKeyName);

      LPCTSTR pVal;
      UINT iLenVal;
      if (VerQueryValue(m_pVersionInfo, (LPTSTR)(LPCTSTR)query,
          (LPVOID*)&pVal, &iLenVal)) {

         sVal = pVal;
      }
   }
   return sVal;
}

// typedef for DllGetVersion proc
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

/////////////////
// Get DLL Version by calling DLL's DllGetVersion proc
//
BOOL CModuleVersion::DllGetVersion(LPCTSTR modulename, DLLVERSIONINFO& dvi)
{
   HINSTANCE hinst = LoadLibrary(modulename);
   if (!hinst)
      return FALSE;

   // Must use GetProcAddress because the DLL might not implement 
   // DllGetVersion. Depending upon the DLL, the lack of implementation of the 
   // function may be a version marker in itself.
   //
   DLLGETVERSIONPROC pDllGetVersion =
      (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");

   if (!pDllGetVersion)
      return FALSE;

   memset(&dvi, 0, sizeof(dvi));        // clear
   dvi.cbSize = sizeof(dvi);            // set size for Windows

   return SUCCEEDED((*pDllGetVersion)(&dvi));
}

BOOL CModuleVersion::IsWinNT()
{
   OSVERSIONINFO v;
   v.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
   GetVersionEx(&v);
   return (v.dwPlatformId==VER_PLATFORM_WIN32_NT);
}






CVersionInfo::CVersionInfo(void)
{
	Reset ();
}

CVersionInfo::~CVersionInfo(void)
{
}




//////////////////////////////////////////////////////////////////////
//
//    S E T   V E R S I O N   I N F O 
//
//
bool CVersionInfo::SetVersionInfo (int nMajor, int nMinor, int nBuildCode, int nExtra)
{
	m_nMajor = nMajor;
	m_nMinor = nMinor;
	m_nBuildCode = nBuildCode;
	m_nExtra = nExtra;

	return true;

} // end SetVersionInfo




//////////////////////////////////////////////////////////////////////
//
//    S E T   V E R S I O N   I N F O 
//
//
bool CVersionInfo::SetVersionInfo (CString strMajorDotMinor, CString strBuildCode, CString strExtra)
{
	Reset ();

	int nFirstPeriod = strMajorDotMinor.Find ('.');
	if (-1 == nFirstPeriod)
		return false;

	CString strMajor	= strMajorDotMinor.Left (nFirstPeriod);
	CString strMinor	= strMajorDotMinor.Mid (nFirstPeriod + 1);

	m_nMajor		= _tstoi (strMajor);
	m_nMinor		= _tstoi (strMinor);
	m_nBuildCode	= _tstoi (strBuildCode);
	m_nExtra		= _tstoi (strExtra);

	return true;

} // end SetVersionInfo




bool CVersionInfo::operator< (const CVersionInfo& vi) const
{
	if (m_nMajor < vi.m_nMajor)
		return true;
	if (m_nMajor > vi.m_nMajor)
		return false;

	//
	//  Same major version...

	if (m_nMinor < vi.m_nMinor)
		return true;
	if (m_nMinor > vi.m_nMinor)
		return false;

	//
	//  Same major and minor versions

	if (m_nBuildCode < vi.m_nBuildCode)
		return true;
	if (m_nBuildCode > vi.m_nBuildCode)
		return false;

	//
	//  Same major, minor, and build code

	if (m_nExtra < vi.m_nExtra)
		return true;
	if (m_nExtra > vi.m_nExtra)
		return false;

	//
	//  Same everything

	return false;

} // end operator < 





bool CVersionInfo::operator> (const CVersionInfo& vi) const
{
	if (vi < (*this))
		return true;
	return false;

} // end operator >


bool CVersionInfo::operator== (const CVersionInfo& vi) const
{
	if ((! ((*this) < vi)) &&
		(! ((*this) > vi)))
		return true;

	return false;
}



//////////////////////////////////////////////////////////////////////
//
//    G E T   V E R S I O N   S T R I N G 
//
//
CString	CVersionInfo::GetVersionString ()
{
	CString str;
//	str.Format (CStringFromTable (IDS_STR_D_D_BUILD_D, _T("%d.%d build %d")), m_nMajor, m_nMinor, m_nBuildCode);
	str.Format (_T("%d.%d build %d"), m_nMajor, m_nMinor, m_nBuildCode);
	return str;

} // end GetVersionString