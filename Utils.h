
#ifndef UTILS_H
#define UTILS_H

/** @file */

#include <stdarg.h>
#include <string>
#include "Dir.h"

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

extern TBool g_bUsingGUI;		/**< ETrue if running a GUI based program */
extern volatile bool g_break;	/**< Set to true if when ctrl+c is hit by the user */

class Utils
{
private:

	static TInt MessageBox(enum TMessageBoxType a_eMessageBoxType, const char *a_pccTitle, const char *a_pccMessage, va_list a_oArgs);

public:

	static TBool addPart(char *a_pcDest, const char *a_pccSource, size_t a_stDestSize);

	static TInt CountTokens(const char *a_pcBuffer);

	static TInt CreateDirectory(const char *a_pccDirectoryName);

	static TInt DeleteDirectory(const char *a_pccDirectoryName);

	static TInt Detach(char *a_ppcArgV[]);

	static char *DuplicateString(const char *a_pccString, TInt a_iLength);

	static void Error(const char *a_pccMessage, ...);

	static const char *Extension(const char *a_pccFileName);

	static const char *filePart(const char *a_pccPath);

	static TBool FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory);

	static std::string GetCurrentDirectory();

	static TInt GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry, TBool a_bResolveLink = ETrue);

	static void GetScreenSize(struct SRect &a_roScreenSize, CWindow *a_poWindow = NULL);

	static int GetShellHeight();

	static bool GetString(std::string &a_roBuffer);

	static void *GetTempBuffer(void *a_pvBuffer, size_t a_stSize, TBool a_bCopyContents = ETrue);

	static void FreeTempBuffer(void *a_pvBuffer);

#ifdef _DEBUG

	static void AssertionFailure(const char *a_pccMessage, ...);

	static void info(const char *a_pccMessage, ...);

#else /* ! _DEBUG */

	static void info(const char *, ...) { }

#endif /* ! _DEBUG */

	static TInt LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer);

	static void LocalisePath(char *a_pcPath);

	static TInt makeLink(const char *a_pccSource, const char *a_pccDest);

	static TInt MapLastError();

	static TInt MapLastFileError(const char *a_pccFileName);

	static TInt MessageBox(enum TMessageBoxType a_eMessageBoxType, const char *a_pccTitle, const char *a_pccMessage, ...);

	static void NormalisePath(char *a_pcPath);

	static char *ResolveFileName(const char *a_pccFileName, TBool a_bGetDeviceName = EFalse);

	static char *ResolveProgDirName(const char *a_pccFileName);

	static TInt SetDeleteable(const char *a_pccFileName);

	static TInt setFileDate(const char *a_pccFileName, const TEntry &a_roEntry, TBool a_bResolveLink = ETrue);

	static TInt setProtection(const char *a_pccFileName, TUint a_uiAttributes);

	static void setSignal();

	static int splitHost(const char *a_host, std::string &a_server, unsigned short &a_port, unsigned short a_defaultPort = 80);

	static char *StripDags(char *a_pcLine, TInt *a_piLength);

	static TInt StringToInt(const char *a_pccString, TInt *a_piResult);

	static void TimeToString(std::string &a_roDate, std::string &a_roTime, const TDateTime &a_roDateTime);

	static TBool TimeToString(std::string &a_roDate, std::string &a_roTime, const TEntry &a_roEntry);

	static void TrimString(char *a_pcString);

	static ULONG Red32(unsigned long a_ulColour);

	static ULONG Green32(unsigned long a_ulColour);

	static ULONG Blue32(unsigned long a_ulColour);

	static ULONG ReverseRGB(unsigned long a_ulColour);

	static void swap32(int32_t *value)
	{
		swap32((uint32_t *) value);
	}

	static void swap32(uint32_t *value);

	static void swap64(TInt64 *value);
};

/* Various assertion type macros which only do anything in debug builds */

#ifdef _DEBUG

#define ASSERTM(Expression, Message) if (!(Expression)) Utils::AssertionFailure(Message);
#define DEBUGCHECK(Function, Message) if (!(Function)) Utils::AssertionFailure(Message);
#define DEBUGFAILURE(Message) Utils::AssertionFailure(Message);

#else /* ! _DEBUG */

#define ASSERTM(Expression, Message)
#define DEBUGCHECK(Function, Message) (void) Function
#define DEBUGFAILURE(Message)

#endif /* ! _DEBUG */

/* Profiling macros, which can report on the time spent in a method or block of code */

#ifdef STD_PROFILE

#define STD_PROFILE_START(Message) \
	TTime StartTime, EndTime; \
	TUint MilliSeconds; \
\
	StartTime.HomeTime(); \
	Utils::info(Message);

#define STD_PROFILE_END(Message) \
	EndTime.HomeTime(); \
	MilliSeconds = (TUint) ((EndTime.Int64() - StartTime.Int64()) / 1000); \
\
	Utils::info(Message" => %lu ms", MilliSeconds);

#else /* ! STD_PROFILE */

#define STD_PROFILE_START(Message)
#define STD_PROFILE_END(Message)

#endif /* ! STD_PROFILE */

#endif /* ! UTILS_H */
