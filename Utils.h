
#ifndef UTILS_H
#define UTILS_H

#include <stdarg.h>
#include "Dir.h"

#ifdef _MSC_VER

/* Disable a bogus MSVC warning */

#pragma warning(disable : 4710) /* Function not expanded */

#endif /* _MSC_VER */

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

	static TInt CountTokens(const char *a_pcBuffer);

	static TInt CreateDirectory(const char *a_pccDirectoryName);

	static TInt DeleteDirectory(const char *a_pccDirectoryName);

	static TInt Detach();

	static void Error(const char *a_pccMessage, ...);

	static const char *Extension(const char *a_pccFileName);

	static const char *FilePart(const char *a_pccPath);

	static TBool FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory);

	static TInt GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry);

	static void GetScreenSize(TInt *a_piWidth, TInt *a_piHeight);

	static TBool GetShellHeight(TInt *a_piHeight);

	static void *GetTempBuffer(char *a_pccBuffer, TInt a_iSize);

	static void FreeTempBuffer(char *a_pccBuffer);

#ifdef _DEBUG

	static void AssertionFailure(const char *a_pccMessage, ...);

	static void Info(const char *a_pccMessage, ...);

#else /* ! _DEBUG */

	static void Info(const char *, ...) { }

#endif /* ! _DEBUG */

	static TInt LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer);

	static TInt MapLastError();

	static TInt MapLastFileError(const char *a_pccFileName);

	static TInt MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, va_list a_oArgs);

	static TInt MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, ...);

	static void NormalisePath(char *a_pcPath);

	static char *ResolveFileName(const char *a_pccFileName);

	static char *ResolveProgDirName(const char *a_pccFileName);

	static TBool ScanDirectory(const char *a_pccDirectoryName, TBool a_bScanFiles, ScanFunc a_pfScanFunc, void *a_pvUserData);

	static TInt SetDeleteable(const char *a_pccFileName);

	static TInt SetFileDate(const char *a_pccFileName, const TEntry &a_roEntry);

	static TInt SetProtection(const char *a_pccFileName, TUint a_uiAttributes);

	static TBool TimeToString(char *a_pcDate, char *a_pcTime, const TEntry &a_roEntry);

	static void TrimString(char *a_pcString);

	static ULONG Red32(unsigned long a_ulColour);

	static ULONG Green32(unsigned long a_ulColour);

	static ULONG Blue32(unsigned long a_ulColour);
};

/* Various assertion type macros which only do anything in debug builds */

#ifdef _DEBUG

#define ASSERTM(Expression, Message) if (!(Expression)) Utils::AssertionFailure(Message);
#define DEBUGCHECK(Function, Message) if (!(Function)) Utils::AssertionFailure(Message);
#define DEBUGFAILURE(Message) Utils::AssertionFailure(Message);

#else /* ! _DEBUG */

#define ASSERTM(Expression, Message)
#define DEBUGCHECK(Function, Message) Function
#define DEBUGFAILURE(Message)

#endif /* ! _DEBUG */

#endif /* ! UTILS_H */
