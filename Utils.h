
#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include "Dir.h"

/* Forward declaration to avoid including Dir.h */

class TEntry;

/* Export some globals for use elsewhere */

extern bool g_bFromWorkbench; /* true if launched from Workbench */

typedef bool (*ScanFunc)(const char *a_pccFileName, void *a_pvUserData);

class Utils
{
public:

	static bool AddPart(char *a_pcDest, const char *a_pccSource, unsigned int a_iDestLength);

	static int CreateDirectory(const char *a_pccDirectoryName);

	static void Error(const char *a_pccMessage, ...);

	static bool FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, bool *a_pbDirectory);

	static int GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry);

	static bool GetShellHeight(int *a_piHeight);

#ifdef _DEBUG

	static void AssertionFailure(const char *a_pccMessage, ...);

	static void Info(const char *a_pccMessage, ...);

#else /* ! _DEBUG */

	static void Info(const char *, ...) { }

#endif /* ! _DEBUG */

	static bool IsDevice(const char *a_pccPath);

	static bool IsDirectory(const char *a_pccFileName, bool *a_pbDirectory);

	static int LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer);

	static void MessageBox(const char *a_pccTitle, const char *a_pccMessage, va_list a_oArgs);

	static bool ScanDirectory(const char *a_pccDirectoryName, bool a_bScanFiles, ScanFunc a_pfScanFunc, void *a_pvUserData);

	static int SetFileDate(const char *a_pccFileName, const TEntry &a_roEntry);

	static int SetProtection(const char *a_pccFileName, unsigned int a_uiAttributes);

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
