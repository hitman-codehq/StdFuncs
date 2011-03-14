
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

/* Types of message boxes that can be opened with Utils::MessageBox() */

enum TMessageBoxType
{
	EMBTOk, EMBTOkCancel, EMBTYesNo, EMBTYesNoCancel
};

/* Export some globals for use elsewhere */

extern TBool g_bUsingGUI;	/* ETrue if running a GUI based program */

typedef TBool (*ScanFunc)(const char *a_pccFileName, void *a_pvUserData);

class Utils
{
public:

	static TBool AddPart(char *a_pcDest, const char *a_pccSource, TUint a_iDestLength);

	static TInt CountTokens(char *a_pcBuffer);

	static TInt CreateDirectory(const char *a_pccDirectoryName);

	static void Error(const char *a_pccMessage, ...);

	static const char *Extension(const char *a_pccFileName);

	static const char *FilePart(const char *a_pccPath);

	static TBool FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory);

	static TInt GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry);

	static void GetScreenSize(TInt *a_piWidth, TInt *a_piHeight);

	static TBool GetShellHeight(TInt *a_piHeight);

#ifdef _DEBUG

	static void AssertionFailure(const char *a_pccMessage, ...);

	static void Info(const char *a_pccMessage, ...);

#else /* ! _DEBUG */

	static void Info(const char *, ...) { }

#endif /* ! _DEBUG */

	static TBool IsDirectory(const char *a_pccFileName, TBool *a_pbDirectory);

	static TInt LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer);

	static TInt MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, va_list a_oArgs);

	static TInt MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, ...);

	static void NormalisePath(char *a_pcPath);

	static TBool ScanDirectory(const char *a_pccDirectoryName, TBool a_bScanFiles, ScanFunc a_pfScanFunc, void *a_pvUserData);

	static TInt SetFileDate(const char *a_pccFileName, const TEntry &a_roEntry);

	static TInt SetProtection(const char *a_pccFileName, TUint a_uiAttributes);

	static void TimeToString(char *a_pcDate, char *a_pcTime, const TEntry &a_roEntry);

	static void TrimString(char *a_pcString);

	static ULONG Red32(unsigned long a_ulColour);

	static ULONG Green32(unsigned long a_ulColour);

	static ULONG Blue32(unsigned long a_ulColour);
};

#ifdef _DEBUG

#define ASSERTM(Expression, Message) if (!(Expression)) Utils::AssertionFailure(Message);

#else /* ! _DEBUG */

#define ASSERTM(Expression, Message)

#endif /* _DEBUG */

/* A macro to call a function and check its return value only in debug builds */

#ifdef _DEBUG

#define DEBUGCHECK(Function, Message) if (!(Function)) Utils::AssertionFailure(Message);

#else /* ! _DEBUG */

#define DEBUGCHECK(Function, Message) Function

#endif /* ! _DEBUG */

#endif /* ! UTILS_H */
