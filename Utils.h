
#ifndef UTILS_H
#define UTILS_H

/** @file */

#include <stdarg.h>
#include "Dir.h"

#ifdef _MSC_VER

/* Disable a bogus MSVC warning */

#pragma warning(disable : 4710) /* Function not expanded */

#endif /* _MSC_VER */

/**
 * Types of message boxes that can be opened with Utils::MessageBox().
 * The type of the message box is determined by how many buttons it contains;  this is defined
 * by this enumeration.  When the user closes a message box it will return a value that indicates
 * which button was clicked.  This will be one of IDOK, IDCANCEL, IDYES or IDNO.
 */

enum TMessageBoxType
{
	EMBTOk,			/**< Single Ok button */
	EMBTOkCancel,	/**< Ok and Cancel buttons */
	EMBTYesNo,		/**< Yes and No buttons */
	EMBTYesNoCancel	/**< Yes, No and Cancel buttons */
};

/* Forward declaration to reduce the # of includes required */

class CWindow;

/* Export some globals for use elsewhere */

extern TBool g_bUsingGUI;	/**< ETrue if running a GUI based program */

typedef TBool (*ScanFunc)(const char *a_pccFileName, void *a_pvUserData);

class Utils
{
public:

	static TBool AddPart(char *a_pcDest, const char *a_pccSource, TUint a_uiDestSize);

	static TInt CountTokens(const char *a_pcBuffer);

	static TInt CreateDirectory(const char *a_pccDirectoryName);

	static TInt DeleteDirectory(const char *a_pccDirectoryName);

	static TInt Detach();

	static char *DuplicateString(const char *a_pccString, TInt a_iLength);

	static void Error(const char *a_pccMessage, ...);

	static const char *Extension(const char *a_pccFileName);

	static const char *FilePart(const char *a_pccPath);

	static TBool FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory);

	static TInt GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry);

	static void GetScreenSize(struct SRect &a_roScreenSize, CWindow *a_poWindow = NULL);

	static TBool GetShellHeight(TInt *a_piHeight);

	static void *GetTempBuffer(char *a_pccBuffer, TInt a_iSize, TBool a_bCopyContents = ETrue);

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

	static char *StripDags(char *a_pcLine, TInt *a_piLength);

	static TInt StringToInt(const char *a_pccString, TInt *a_piResult);

	static void TimeToString(char *a_pcDate, char *a_pcTime, const TDateTime &a_roDateTime);

	static TBool TimeToString(char *a_pcDate, char *a_pcTime, const TEntry &a_roEntry);

	static void TrimString(char *a_pcString);

	static ULONG Red32(unsigned long a_ulColour);

	static ULONG Green32(unsigned long a_ulColour);

	static ULONG Blue32(unsigned long a_ulColour);

	static ULONG ReverseRGB(unsigned long a_ulColour);
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
