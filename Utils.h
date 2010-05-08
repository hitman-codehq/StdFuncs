
#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include "Dir.h"

#ifdef WIN32

/* Disable a bogus MSVC warning */

#pragma warning(disable : 4710) /* Function not expanded */

#endif /* WIN32 */

/* Forward declaration to avoid including Dir.h */

class TEntry;

/* Export some globals for use elsewhere */

extern TBool g_bFromWorkbench; /* ETrue if launched from Workbench */

typedef TBool (*ScanFunc)(const char *a_pccFileName, void *a_pvUserData);

class Utils
{
public:

	static TBool AddPart(char *a_pcDest, const char *a_pccSource, TUint a_iDestLength);

	static TInt CreateDirectory(const char *a_pccDirectoryName);

	static void Error(const char *a_pccMessage, ...);

	static TBool FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory);

	static TInt GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry);

	static TBool GetShellHeight(TInt *a_piHeight);

#ifdef _DEBUG

	static void AssertionFailure(const char *a_pccMessage, ...);

	static void Info(const char *a_pccMessage, ...);

#else /* ! _DEBUG */

	static void Info(const char *, ...) { }

#endif /* ! _DEBUG */

	static TBool IsDirectory(const char *a_pccFileName, TBool *a_pbDirectory);

	static TInt LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer);

	static void MessageBox(const char *a_pccTitle, const char *a_pccMessage, va_list a_oArgs);

	static TBool ScanDirectory(const char *a_pccDirectoryName, TBool a_bScanFiles, ScanFunc a_pfScanFunc, void *a_pvUserData);

	static TInt SetFileDate(const char *a_pccFileName, const TEntry &a_roEntry);

	static TInt SetProtection(const char *a_pccFileName, TUint a_uiAttributes);

	static void TimeToString(char *a_pcDate, char *a_pcTime, const TEntry &a_roEntry);

	static ULONG Red32(unsigned long a_ulColour);

	static ULONG Green32(unsigned long a_ulColour);

	static ULONG Blue32(unsigned long a_ulColour);
};

#ifdef _DEBUG

#define ASSERTM(Expression, Message) if (!(Expression)) Utils::AssertionFailure(Message);

#else /* ! _DEBUG */

#define ASSERTM(Expression, Message)

#endif /* _DEBUG */

#endif /* ! UTILS_H */
