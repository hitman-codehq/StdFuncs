
#include "StdFuncs.h"
#include "OS4Support.h"
#include "StdWindow.h"

#ifdef __amigaos__

#include <clib/debug_protos.h>
#include <dos/dostags.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <workbench/startup.h>

#elif defined(QT_GUI_LIB)

#include <QtWidgets/QMessageBox>
#include "Qt/QtWindow.h"

#endif /* QT_GUI_LIB */

#ifdef __unix__

#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#ifdef __APPLE__

#include <mach-o/dyld.h>

#endif /* __APPLE__ */

#endif /* __unix__ */

#include <stdio.h>
#include <string.h>
#include "File.h"

#ifdef __amigaos__

#define MAX_NAME_FROM_LOCK_LENGTH 1024

#ifdef __amigaos4__

#define DELETE_DIRECTORY(DirectoryName) Delete(DirectoryName)

#else /* ! __amigaos4__ */

#define DELETE_DIRECTORY(DirectoryName) DeleteFile(DirectoryName)

#endif /* ! __amigaos4__ */

#define VSNPRINTF vsnprintf

#elif defined(__unix__)

#define DELETE_DIRECTORY(DirectoryName) (rmdir(DirectoryName) == 0)
#define VSNPRINTF vsnprintf

#else /* ! __unix__ */

#define DELETE_DIRECTORY(DirectoryName) RemoveDirectory(DirectoryName)
#define VSNPRINTF _vsnprintf

#endif /* ! __unix__ */

#define PRINTF printf

/* Amiga style suffix that can be prepended to filenames to resolve */
/* them to the directory of the executable that is using the file */

static const char g_accProgDir[] = "PROGDIR:";
#define PROGDIR_LENGTH 8

/* OS4 allows using underscores to indicate hotkeys when using EasyRequest() but OS3 does not */

#ifdef __amigaos4__

static const char g_accOk[] = "_Ok";
static const char g_accOkCancel[] = "_Ok|_Cancel";
static const char g_accYesNo[] = "_Yes|_No";
static const char g_accYesNoCancel[] = "_Yes|_No|_Cancel";

#elif defined(__amigaos__)

static const char g_accOk[] = "Ok";
static const char g_accOkCancel[] = "Ok|Cancel";
static const char g_accYesNo[] = "Yes|No";
static const char g_accYesNoCancel[] = "Yes|No|Cancel";

#endif /* defined(__amigaos__) */

/* ETrue if running a GUI based program */

TBool g_bUsingGUI;

/* Pointer to root window on which all other windows open.  This was defined in CWindow as a static, */
/* but this caused the GUI framework to be linked in even for command line only programs.  Thus */
/* it is now kept as a global variable.  Less elegent perhaps, but it significantly reduces the */
/* amount of code statically linked in */

CWindow	*g_poRootWindow;

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Callback function used by EnumDisplayMonitors().
 * This function is called by the windows EnumDisplayMonitors() function for each display that is
 * attached to the host computer.  This enables the client to obtain the dimensions of each display.
 * It is used by the Utils::GetScreenSize() function when querying the display on which a particular
 * window is opened.
 *
 * @date	Tuesday 10-Jun-2015 6:30 am, Code HQ Ehinger Tor
 * @param	a_poMonitor		Handle of a structure that represents a display attached to the system
 * @param	a_poDC			Handle of the device context passed into EnumDisplayMonitors()
 * @param	a_poRect		Pointer to the RECT passed into EnumDisplayMonitors()
 * @param	a_lData			32 bit user data passed into EnumDisplayMonitors(), in our case a pointer
 * 							to the SRect structure into which to place information about the display
 * @return	Always FALSE to stop the scan, as we are only interested in the first display found
 */

static BOOL CALLBACK MonitorEnumProc(HMONITOR a_poMonitor, HDC /*a_poDC*/, LPRECT /*a_poRect*/, LPARAM a_lData)
{
	struct SRect *ScreenSize;
	MONITORINFO MonitorInfo;

	/* Get a pointer to the SRect to be filled out */

	ScreenSize = (struct SRect *) a_lData;

	/* And query the system for the size of the display that was just passed in */

	MonitorInfo.cbSize = sizeof(MonitorInfo);

	if (GetMonitorInfo(a_poMonitor, &MonitorInfo))
	{
		ScreenSize->m_iTop = MonitorInfo.rcMonitor.top;
		ScreenSize->m_iLeft = MonitorInfo.rcMonitor.left;
		ScreenSize->m_iWidth = (MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left);
		ScreenSize->m_iHeight = (MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top);
	}

	/* Always return FALSE to stop the scan.  We are only interested in querying the first display in */
	/* the system as this is the one that matches the window that was passed to EnumDisplayMonitors() */

	return(FALSE);
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

/**
 *
 * Maps the OS's last error onto one of The Framework's standard errors.
 * This function will determine the last error reported by the OS and to map it onto a standard
 * error.
 *
 * @date	Thursday 27-Sep-2012 6:39 am, Code HQ Ehinger Tor
 * @return	KErrNone if the last OS operation was successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

TInt Utils::MapLastError()
{
	TInt RetVal;

#ifdef __amigaos__

	LONG Result;

	Result = IoErr();

	if (Result == ERROR_OBJECT_EXISTS)
	{
		RetVal = KErrAlreadyExists;
	}
	else if (Result == ERROR_OBJECT_NOT_FOUND)
	{
		RetVal = KErrNotFound;
	}
	else if (Result == ERROR_DIR_NOT_FOUND)
	{
		RetVal = KErrPathNotFound;
	}
	else if ((Result == ERROR_DIRECTORY_NOT_EMPTY) || (Result == ERROR_OBJECT_IN_USE))
	{
		RetVal = KErrInUse;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#elif defined(__unix__)

	if (errno == EEXIST)
	{
		RetVal = KErrAlreadyExists;
	}
	else if (errno == ENOENT)
	{
		RetVal = KErrNotFound;
	}
	else if ((errno == ENOTEMPTY) || (errno == EBUSY) || (errno == ETXTBSY))
	{
		RetVal = KErrInUse;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#else /* ! __unix__ */

	DWORD Error;

	Error = GetLastError();

	if (Error == ERROR_FILE_EXISTS)
	{
		RetVal = KErrAlreadyExists;
	}
	else if ((Error == ERROR_FILE_NOT_FOUND) || (Error == ERROR_INVALID_NAME))
	{
		RetVal = KErrNotFound;
	}
	else if (Error == ERROR_PATH_NOT_FOUND)
	{
		RetVal = KErrPathNotFound;
	}
	else if ((Error == ERROR_DIR_NOT_EMPTY) || (Error == ERROR_SHARING_VIOLATION))
	{
		RetVal = KErrInUse;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Maps the OS's last error onto one of The Framework's standard errors.
 * This function is an extension to Utils::MapLastError().  It uses that function to determine the
 * last error reported by the OS and to map it onto a standard error, but then performs some extra
 * file related checking on some operating systems to perhaps change the error returned.  This is
 * because different operating systems handle errors slightly differently and thus without this
 * function the results returned by The Framework's file I/O related functions would be different
 * on different platforms.
 *
 * This function should only be used in functions that perform file or directory I/O.  All other
 * functions should use Utils::MapLastError() directly to do their error mapping.
 *
 * @date	Monday 09-Oct-2012 5:47 am
 * @param	a_pccFileName		Pointer to the name of the file to be checked
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the file already exists
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

TInt Utils::MapLastFileError(const char *a_pccFileName)
{
	TInt RetVal;

	/* See what the last error was */

	RetVal = Utils::MapLastError();

#ifdef WIN32

	(void) a_pccFileName;

#else /* ! WIN32 */

	char *Name;
	TInt NameOffset;
	TEntry Entry;

	/* Unfortunately UNIX and Amiga OS don't have an error that can be mapped onto KErrPathNotFound */
	/* so we must do a little extra work here.  Amiga OS needs a little help too */

	if (RetVal == KErrNotFound)
	{
		/* Determine the path to the file that was opened */

		NameOffset = (Utils::filePart(a_pccFileName) - a_pccFileName);

		/* If the file is not in the current directory then check if the path exists */

		if (NameOffset > 0)
		{
			/* Strip off the trailing '/' or '\' but NOT the ':' as this is required */
			/* for correct RAM disc handling on Amiga OS */

			if (a_pccFileName[NameOffset - 1] != ':')
			{
				--NameOffset;
			}

			/* Allocate a buffer long enough to hold the path to the file */

			if ((Name = new char[NameOffset + 1]) != NULL)
			{
				memcpy(Name, a_pccFileName, NameOffset);
				Name[NameOffset] = '\0';

				/* Now check for the existence of the file */

				if (Utils::GetFileInfo(Name, &Entry) == KErrNotFound)
				{
					RetVal = KErrPathNotFound;
				}

				delete [] Name;
			}
			else
			{
				Utils::info("RFile::MapLastOpenError() => Out of memory");

				RetVal = KErrNoMemory;
			}
		}
	}

#endif /* ! WIN32 */

	return(RetVal);
}

/**
 * Appends a file or directory name to an existing path.
 * This function will append a file or directory to an existing path, ensuring that a '/' character
 * is appended as appropriate to the end of the existing path.  This will ensure that a single
 * slash is always present between the end of the existing path and the beginning of the newly
 * added path part.  It will not append an extra slash if one exists already.
 *
 * It will also treat Windows style directory names (such as "d:") specially in that they will not
 * have a slash appended to them.  This allows relative addressing of directories on drives where
 * the drive letter by itself represents the current directory of that drive.
 *
 * @date	Thursday 16-Jul-2009 3:58 pm
 * @param	a_pcDest		Pointer to the path to which to append the file or directory name
 * @param	a_pccSource		Pointer to the file or directory name to be appended
 * @param	a_stDestSize	The size of the destination buffer in bytes
 * @return	ETrue if the file or directory was appended successfully
 * @return	EFalse if the destination buffer was too small to hold the resulting path
 */

TBool Utils::addPart(char *a_pcDest, const char *a_pccSource, size_t a_stDestSize)
{
	TBool RetVal;

#ifdef __amigaos__

	RetVal = AddPart(a_pcDest, a_pccSource, a_stDestSize);

#else /* ! __amigaos__ */

	char Char;
	size_t Length;

	/* Assume failure */

	RetVal = EFalse;

	/* If there is anything already in the destination string, check there is enough space to */
	/* append to it before doing anything */

	if ((Length = strlen(a_pcDest)) > 0)
	{
		if ((Length + 1 + strlen(a_pccSource)) <= a_stDestSize)
		{
			RetVal = ETrue;

			/* Append the source string, plus a directory separator if required */

			Char = a_pcDest[Length - 1];

			if ((Char != '\\') && (Char != '/') && (Char != ':'))
			{
				strcat(a_pcDest, "/");
			}

			strcat(a_pcDest, a_pccSource);
		}
	}

	/* Otherwise just copy the source string into the destination, provided there is enough space */
	/* to do so */

	else
	{
		if (strlen(a_pccSource) <= a_stDestSize)
		{
			RetVal = ETrue;

			strcpy(a_pcDest, a_pccSource);
		}
	}

#endif /* ! __amigaos__ */

	return(RetVal);
}

#ifdef _DEBUG

/**
 * Displays information about an assertion failure.
 * This function will display information about an assertion failure, either in a message box, if
 * running on a GUI based system, or as a printf() if not.  It will also (on most platforms) exit
 * back to the operating system after displaying this message.
 *
 * @date	Saturday 11-Jul-2009 08:56 am
 * @param	a_pccMessage	The message to be displayed, in printf() format
 */

void Utils::AssertionFailure(const char *a_pccMessage, ...)
{
	va_list Args;

	va_start(Args, a_pccMessage);

	if (g_bUsingGUI)
	{
		MessageBox(EMBTOk, "Assertion Failure", a_pccMessage, Args);
	}
	else
	{
		PRINTF("Assertion Failure: %s\n", a_pccMessage);
	}

	va_end(Args);

#ifndef __amigaos__

	/* When an assertion happens we want to exit the system as if we allow execution */
	/* to continue then it is likely to crash anyway.  We will do this on all platforms */
	/* besides Amiga as the non protected Amiga OS isn't so good at handling this! */

	exit(1);

#endif /* ! __amigaos__ */

}

#endif /* _DEBUG */

/**
 * Parses a string to determine how many tokens it contains.
 * Parses a string, identifying tokens and keeping track of a count of how many it finds.  A token
 * is either a single word delimited by white space (spaces or tabs), or multiple words in a quoted
 * string.
 *
 * @date	Friday 04-Jun-2010 7:58 am
 * @param	a_pccBuffer	Pointer to buffer to parse for tokens
 * @return	The number of tokens found in the string
 */

TInt Utils::CountTokens(const char *a_pccBuffer)
{
	char Char;
	TBool FoundDelimeter, TokenStart;
	TInt Source, RetVal;

	/* Iterate through the string passed in and determine how many tokens are present */

	FoundDelimeter = EFalse;
	TokenStart = ETrue;
	RetVal = 0;

	for (Source = 0; a_pccBuffer[Source]; ++Source)
	{
		Char = a_pccBuffer[Source];

		/* If the current character is a " then we have either found the start of a new token */
		/* or the end of an old one */

		if (Char == '"')
		{
			/* FoundDelimter indicates whether we have found the start of a new token or the end */
			/* of an old one */

			if (!(FoundDelimeter))
			{
				FoundDelimeter = ETrue;
			}
			else
			{
				FoundDelimeter = EFalse;
			}

			/* Either way we have found a new token */

			TokenStart = ETrue;
		}

		/* If we have found white space then we have a new token, but only if the space is not inside */
		/* a quoted string */

		else if (((Char == ' ') || (Char == '\t')) && (!(FoundDelimeter)))
		{
			TokenStart = ETrue;
		}

		/* If this is the first character of a token then increment the token count */

		else
		{
			if (TokenStart)
			{
				++RetVal;
				TokenStart = EFalse;
			}
		}
	}

	return(RetVal);
}

/**
 * Creates a new directory.
 * Creates a directory with the name passed in.  The name can be either relative to the current
 * directory or a fully qualified path.
 *
 * @date	Saturday 18-Jul-2009 8:25 am
 * @param	a_pccDirectoryName	The name of the directory to be created
 * @return	KErrNone if successful
 * @return	KErrAlreadyExists if the dirctory already exists
 * @return	KErrNot found for any other error
 */

TInt Utils::CreateDirectory(const char *a_pccDirectoryName)
{
	TInt RetVal;

#ifdef __amigaos__

	BPTR Lock;

	if ((Lock = CreateDir(a_pccDirectoryName)) != 0)
	{
		RetVal = KErrNone;

		UnLock(Lock);
	}
	else
	{
		RetVal = (IoErr() == ERROR_OBJECT_EXISTS) ? KErrAlreadyExists : KErrNotFound;
	}

#elif defined(__unix__)

	if (mkdir(a_pccDirectoryName, 0755) == 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (errno == EEXIST) ? KErrAlreadyExists : KErrNotFound;
	}

#else /* ! __unix__ */

	if (::CreateDirectory(a_pccDirectoryName, NULL))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (GetLastError() == ERROR_ALREADY_EXISTS) ? KErrAlreadyExists : KErrNotFound;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Deletes a directory from the file system.
 * This function will delete a directory.  The directory in question must not be open for use in
 * any way and must be empty.
 *
 * @date	Friday 27-Jul-2012 10:27 am
 * @param	a_pccDirectoryName	Name of the directory to be deleted
 * @return	KErrNone if successful
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

TInt Utils::DeleteDirectory(const char *a_pccDirectoryName)
{
	TInt RetVal;

	/* First try to delete the directory */

	if (DELETE_DIRECTORY(a_pccDirectoryName))
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = MapLastFileError(a_pccDirectoryName);
	}

	return(RetVal);
}

/**
 * Detaches the currently running program from the CLI.
 * This function is mainly for Amiga OS and relaunches the current process asynchronously before
 * exiting the current process.  This gives the user the impression that the process has
 * automatically detached itself from the CLI.  Note that if successful, this function will not
 * return and will instead shut the process down using exit().  The second time it is called (by
 * the newly launched process), it will detect that the new process is already detached from the
 * CLI and will simply return without doing anything.
 *
 * @date	Monday 27-Aug-2012 06:56 am
 * @return	Does not return if successful
 * @return	KErrNone if process is already detached from the CLI
 * @return	KErrGeneral if unable to detach the process
 */

TInt Utils::Detach()
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;

#ifdef __amigaos__

	char *Command, *CommandNameBuffer;
	const char *CommandName;
	TInt ArgStrLength, CommandLength, CommandNameLength, Result;
	BPTR InputStream;
	struct CommandLineInterface *CLI;

	/* If we are started from the CLI and are not already backgrounded (ie. By the "run" */
	/* command or by our own relaunching of ourselves as backgrounded) then relaunch */
	/* the process asynchronously */

	CLI = Cli();

	if ((CLI) && (!(CLI->cli_Background ) && (CLI->cli_CommandName)))
	{
		/* Determine the size of the buffers needed for the command name and command line */

		CommandNameLength = CopyStringBSTRToC(CLI->cli_CommandName, NULL, 0);
		ArgStrLength = strlen(GetArgStr());

		/* Allocate a buffer long enough for the command name. This must include an extra */
		/* byte for the NULL terminator */

		CommandNameBuffer = new char[CommandNameLength + 1];

		/* Allocate a buffer long enough to hold the command line.  This must include */
		/* extra bytes for the " and space characters, as well as a NULL terminator */

		CommandLength = (CommandNameLength + ArgStrLength + 4);
		Command = new char[CommandLength];

		if ((CommandNameBuffer) && (Command))
		{
			/* Extract the path to the executable from the CLI structure and from that extract */
			/* the name of the executable.  This will be used for setting the task name so that */
			/* the program shows up in the task list as something more useful than "New Process" */

			CopyStringBSTRToC(CLI->cli_CommandName, CommandNameBuffer, (CommandNameLength + 1));
			CommandName = Utils::filePart(CommandNameBuffer);

			/* Build the string used to relaunch the process */

			snprintf(Command, CommandLength,"\"%s\" %s", CommandNameBuffer, GetArgStr());

			/* We want to use the parent's input stream for both input and output, so duplicate */
			/* it as it will be closed when the child process exits */

			if ((InputStream = Open("CONSOLE:", MODE_READWRITE)) != 0)
			{
				/* And launch the child process!  If successful then exit as this is essentially */
				/* performing a fork() and we don't want to keep the parent process alive */

#ifdef __amigaos4__

				Result = SystemTags(Command, SYS_Input, (ULONG) InputStream, SYS_Output, (ULONG) NULL, SYS_Error, NULL,
					SYS_Asynch, TRUE, NP_Name, CommandName, TAG_DONE);

#else /* ! __amigaos4__ */

				Result = SystemTags(Command, SYS_Input, (ULONG) InputStream, SYS_Output, (ULONG) NULL,
					SYS_Asynch, TRUE, NP_Name, CommandName, TAG_DONE);

#endif /* ! __amigaos4__ */

				/* Free the command buffers before we exit */

				delete [] Command;
				delete [] CommandNameBuffer;

				if (Result == 0)
				{
					exit(0);
				}
				else
				{
					RetVal = KErrGeneral;

					Close(InputStream);
				}
			}
		}
		else
		{
			Utils::info("Utils::Detach() => Out of memory");
		}

		delete [] Command;
		delete [] CommandNameBuffer;
	}

#endif /* __amigaos__ */

	return(RetVal);
}

/**
 * Duplicates a string into a new allocated buffer.
 * Allocates a buffer large enough to hold the string passed in and its NULL terminator and copies
 * the string into said buffer.  If a length is passed in (and 0 is a valid length) then it is
 * assumed the string passed in is not NULL terminated and that length is used as the string's
 * length.  Otherwise if the length is -1 then the string's length is determined with strlen().
 * The string returned by this function should be freed with delete [].
 *
 * @date	13-Feb-2013 6:48 am, Code HQ Ehinger Tor
 * @param	a_pccString		Pointer to the string to be duplicated
 * @param	a_iLength		Length of the string, not including NULL terminator, or -1
 * @return	Pointer to an allocated buffer containing the duplicated string if successful, else NULL
 */

char *Utils::DuplicateString(const char *a_pccString, TInt a_iLength)
{
	char *RetVal;
	size_t Length;

	ASSERTM((a_pccString != NULL), "Utils::DuplicateString() => Pointer to string passed in must not be NULL");

	/* If no length has been passed in then determine the length of the string */

	Length = (a_iLength == -1) ? strlen(a_pccString) : a_iLength;

	/* Allocate a buffer large enough to hold the string and copy the string */

	if ((RetVal = new char[Length + 1]) != NULL)
	{
		memcpy(RetVal, a_pccString, Length);
		RetVal[Length] ='\0';
	}

	return(RetVal);
}

/**
 * Displays an error message.
 * Displays a printf() style formatted message.  If a GUI is in use, the message will be displayed
 * in a message box.  Otherwise it will be printed on the terminal.  The message is displayed
 * regardless of whether the code is compiled in debug or release mode, and thus this function
 * should be used for displaying critical error messages.
 *
 * @date	Saturday 11-Jul-2009 08:56 am
 * @param	a_pccMessage	The message to be displayed, in printf() format
 */

void Utils::Error(const char *a_pccMessage, ...)
{
	// TODO: CAW - Risk of overflow, here, above and in Utils::info
	char Message[512];
	va_list Args;

	va_start(Args, a_pccMessage);

	if (g_bUsingGUI)
	{
		MessageBox(EMBTOk, "Error", a_pccMessage, Args);
	}
	else
	{
		VSNPRINTF(Message, sizeof(Message), a_pccMessage, Args);
		PRINTF("Error: %s\n", Message);
	}

	va_end(Args);
}

/**
 * Returns the extension of a filename.
 * Scans backwards from the end of string passed in, to find a '.' character.  This function is
 * thus a very naive method of finding the extension of a filename, although really it is just
 * searching for a full stop.
 *
 * @date	Thursday 22-Jul-2010 8:47 am
 * @param	a_pccFileName	Pointer to the string in which to search
 * @return	A pointer to the first character after the full stop, if found, else NULL
 */

const char *Utils::Extension(const char *a_pccFileName)
{
	char Char;
	const char *RetVal;

	RetVal = (a_pccFileName + strlen(a_pccFileName));

	while (RetVal >= a_pccFileName)
	{
		Char = *RetVal;

		if (Char == '.')
		{
			++RetVal;

			break;
		}

		--RetVal;
	}

	if (RetVal < a_pccFileName)
	{
		RetVal = NULL;
	}

	return(RetVal);
}

/**
 * Finds the filename contained within a fully qualified path.
 * This function will scan backwards through a fully qualified path, looking for any kind of directory
 * separator.  When found, this separator is considered the separator between the path part of a fully
 * qualified filename, and the filename part of the fully qualified filename.  The pointer to this
 * filename will then be returned.  If the path passed in does not contain any path separators (ie. it
 * contains no path) then the entire path will be returned.
 *
 * As an example, the following Amiga OS, UNIX and Windows paths will all result in "file.txt" being
 * returned by the function.  All paths are recognised by this function on all systems on which
 * The Framework runs:
 *
 * work:file.txt
 * work:path/file.txt
 * /file.txt
 * /path/file.txt
 * c:\\file.txt
 * c:\\path\\file.txt
 *
 * @date	Thursday 22-Jul-2010 8:11 am
 * @param	a_pccPath	Pointer to the fully qualified path to be scanned
 * @return	Pointer to the filename component of the path
 */

const char *Utils::filePart(const char *a_pccPath)
{
	char Char;
	const char *RetVal;

	/* Iterate through the string passed in from its very end, looking for any kind of directory */
	/* separator, whether valid for Amiga OS, UNIX or Windows */

	RetVal = (a_pccPath + strlen(a_pccPath));

	while (RetVal >= a_pccPath)
	{
		Char = *RetVal;

		if ((Char == '/') || (Char == '\\') || (Char == ':'))
		{
			++RetVal;

			break;
		}

		--RetVal;
	}

	/* If no directory separator was found, we will be pointing to just before the start of the */
	/* string.  In this case, return the entire string passed in */

	if (RetVal < a_pccPath)
	{
		++RetVal;
	}

	return(RetVal);
}

/**
 * Determines a filename from a WBArg structure.
 * This function determines the fully qualified filename of the object represented by a WBArg
 * structure.  This is done by determining the path represented by the WBArg's lock and then
 * appending the name to that path.  It will also return (in the variable a_pbDirectory)
 * whether or not the target of the WBArg structure is a file or a directory.
 *
 * @date	Monday 09-Apr-2007 12:06 am
 * @param	a_pcFullName	Pointer to a buffer into which to place the qualified filename
 * @param	a_poWBArg		Pointer to the WBArg structure to be used
 * @param	a_pbDirectory	Pointer to a variable into which to place a directory flag
 * @return	ETrue if successful, else EFalse
 */

#ifdef __amigaos__

TBool Utils::FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory)
{
	char Path[1024]; // TODO: CAW - Possible overflow here and in a_pcFullName
	TBool RetVal;
	struct TEntry Entry;

	/* Assume failure */

	RetVal = EFalse;

	/* Check whether the WBArg has a valid lock and if so, determine its path */

	if (a_poWBArg->wa_Lock)
	{
		if (NameFromLock(a_poWBArg->wa_Lock, Path, sizeof(Path)))
		{
			/* Create the fully qualified path from the path and filename */

			strcpy(a_pcFullName, Path);

			if (strlen(a_poWBArg->wa_Name) > 0)
			{
				DEBUGCHECK((AddPart(a_pcFullName, a_poWBArg->wa_Name, sizeof(Path)) != FALSE),
					"Utils::FullNameFromWBArg() => Unable to build filename");
			}

			/* Is this a directory? */

			if (GetFileInfo(a_pcFullName, &Entry) == KErrNone)
			{
				RetVal = ETrue;
				*a_pbDirectory = Entry.iIsDir;
			}
			else
			{
				Utils::info("Utils::FullNameFromWBArg() => Unable to obtain information about object");
			}
		}
		else
		{
			Utils::info("Utils::FullNameFromWBArg() => Unable to obtain filename from Lock");
		}
	}

	return(RetVal);
}

#endif /* __amigaos__ */

/**
 * Determines the name of the current directory.
 * Queries the underlying operating system to determine the current directory, and returns the
 * directory's fully qualified path.
 *
 * @date	Sunday 26-Sep-2021 6:31 am, Code HQ Bergmannstrasse
 * @return	The fully qualified name of the current directory, if successful, else ""
 */

std::string Utils::GetCurrentDirectory()
{
	std::string retVal;

#ifdef __amigaos__

	char *directory;

	if ((directory = new char[MAX_PATH]) != nullptr)
	{
		if (GetCurrentDirName(directory, MAX_PATH))
		{
			retVal = directory;
		}

		delete [] directory;
	}

#elif defined(__unix__)

	char directory[MAX_PATH];

	if ((getcwd(directory, sizeof(directory))) != nullptr)
	{
		retVal = directory;
	}

#else /* ! __unix__ */

	char *directory;
	DWORD size;

	if ((size = ::GetCurrentDirectory(0, NULL)) != 0)
	{
		if ((directory = new char[size]) != nullptr)
		{
			if ((::GetCurrentDirectory(size, directory)) != 0)
			{
				retVal = directory;
			}

			delete [] directory;
		}
	}

#endif /* ! __unix__ */

	return retVal;
}

/**
 * Obtains information about a given file or directory.
 * This function is useful for obtaining directory listing information about a single file or
 * directory, without the overhead of having to use the RDir class to do so.  It will query
 * the given filename, which can be either relative or absolute, and will place the information
 * about it into the TEntry structure that is passed in.  This will then contain all of the
 * same information that would be in the TEntry structure had it been filled in by the RDir
 * class.
 *
 * For simplicity, there are some restrictions on the file paths passed in:
 *   - They must not end in a '/' or '\' unless they refer to a root directory
 *   - They must not contain wildcards
 *
 * The filename passed in may be prefixed by an Amiga OS style PROGDIR: prefix, which will
 * resolve to the directory from which the current executable was executed.
 *
 * Note that this function explicitly checks for wildcards being passed in and if detected,
 * returns failure.  If you wish to use wildcards then use the more powerful RDir class instead.
 *
 * @pre		Pointer to filename passed in must not be NULL
 * @pre		Pointer to TEntry structure passed in must not be NULL
 *
 * @date	Saturday 04-Jul-2009 9:20 pm
 * @param	a_pccFileName	Pointer to the name of the file for which to obtain information
 * @param	a_poEntry		Pointer to the TEntry structure into which to place the information
 * @param	a_bResolveLink	ETrue to return information about the target of the link, else EFalse
 *							for information about the link itself.  ETrue by default
 * @return	KErrNone if the information was obtained successfully
 * @return	KErrNotFound if the file could not be found
 * @return	KErrNotSupported if the specified path ends with a '/' or '\'
 * @return	KErrPathNotFound if an Amiga OS style PROGDIR: prefix could not be resolved
 */

TInt Utils::GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry, TBool a_bResolveLink)
{
	char *ProgDirName;
	size_t Length;
	TBool PathOk;
	TInt RetVal;

	ASSERTM((a_poEntry != NULL), "Utils::GetFileInfo() => Pointer to filename passed in must not be NULL");
	ASSERTM((a_poEntry != NULL), "Utils::GetFileInfo() => TEntry structure passed in must not be NULL");

	/* If the filename is prefixed with an Amiga OS style "PROGDIR:" then resolve it */

	if ((ProgDirName = Utils::ResolveProgDirName(a_pccFileName)) != NULL)
	{
		/* Assume failure */

		RetVal = KErrNotFound;

		/* Only perform the query if the path does not end with a '/' or '\'.  The check for */
		/* is not required on non Windows systems but it doesn't hurt to have it here */

		Length = strlen(ProgDirName);
		PathOk = ETrue;

		if ((Length > 0) && ((ProgDirName[Length - 1] == '/') || (ProgDirName[Length - 1] == '\\')))
		{
			PathOk = EFalse;

			if ((Length == 1) || ((Length == 3) && (ProgDirName[1] == ':')))
			{
				PathOk = ETrue;
			}
		}

		if (PathOk)
		{

#if defined(__amigaos4__)

			(void) a_bResolveLink;

			struct ClockData ClockData;
			struct ExamineData *ExamineData;

			/* Querying empty filenames is not allowed by our API, even though the Amiga OS ExamineObject() */
			/* function supports it */

			if (*ProgDirName != '\0')
			{
				if ((ExamineData = ExamineObjectTags(EX_StringNameInput, ProgDirName, TAG_DONE)) != NULL)
				{
					RetVal = KErrNone;

					/* Convert the new style date structure into something more usable that also contains */
					/* year, month and day information */

					Amiga2Date(DateStampToSeconds(&ExamineData->Date), &ClockData);

					/* Convert it so a Symbian style TDateTime structure */

					TDateTime DateTime(ClockData.year, (TMonth) (ClockData.month - 1), ClockData.mday, ClockData.hour,
						ClockData.min, ClockData.sec, 0);

					/* And populate the new TEntry instance with information about the file or directory */

					a_poEntry->Set(EXD_IS_DIRECTORY(ExamineData), EXD_IS_LINK(ExamineData), ExamineData->FileSize,
						ExamineData->Protection, DateTime);
					a_poEntry->iPlatformDate = ExamineData->Date;

					/* If the name of the directory is the special case of an Amiga OS volume then return just */
					/* ':' for consistency with other versions of this function, which would only return the slash */
					/* required to access this directory */

					if (ProgDirName[Length - 1] == ':')
					{
						a_poEntry->iName[0] = ':';
						a_poEntry->iName[1] = '\0';
					}

					/* Otherwise return the name of the directory or file */

					else
					{
						strcpy(a_poEntry->iName, ExamineData->Name);
					}

					FreeDosObject(DOS_EXAMINEDATA, ExamineData);
				}
			}

#elif defined(__amigaos__)

			(void) a_bResolveLink;

			BPTR _Lock;
			struct ClockData ClockData;

			/* Querying empty filenames is not allowed by our API, even though the Amiga OS API */
			/* function supports it */

			if (*ProgDirName != '\0')
			{
				// TODO: CAW - Move and rename this and check over this implementation
				struct FileInfoBlock *FileInfoBlock;

				if ((FileInfoBlock = (struct FileInfoBlock *) AllocDosObject(DOS_FIB, TAG_DONE)) != NULL)
				{
					if ((_Lock = Lock(ProgDirName, ACCESS_READ)) != 0)
					{
						if (Examine(_Lock, FileInfoBlock))
						{
							RetVal = KErrNone;

							/* Convert the new style date structure into something more usable that also contains */
							/* year, month and day information */

							Amiga2Date(DateStampToSeconds(&FileInfoBlock->fib_Date), &ClockData);

							/* Convert it so a Symbian style TDateTime structure */

							TDateTime DateTime(ClockData.year, (TMonth) (ClockData.month - 1), ClockData.mday, ClockData.hour,
								ClockData.min, ClockData.sec, 0);

							/* And populate the new TEntry instance with information about the file or directory */

							a_poEntry->Set((FileInfoBlock->fib_DirEntryType > 0), TYPE_IS_LINK(FileInfoBlock->fib_DirEntryType),
								FileInfoBlock->fib_Size, FileInfoBlock->fib_Protection, DateTime);
							a_poEntry->iPlatformDate = FileInfoBlock->fib_Date;

							/* If the name of the directory is the special case of an Amiga OS volume then return just */
							/* ':' for consistency with other versions of this function, which would only return the slash */
							/* required to access this directory */

							if (ProgDirName[Length - 1] == ':')
							{
								a_poEntry->iName[0] = ':';
								a_poEntry->iName[1] = '\0';
							}

							/* Otherwise return the name of the directory or file */

							else
							{
								strcpy(a_poEntry->iName, FileInfoBlock->fib_FileName);
							}
						}

						UnLock(_Lock);
					}
					else
					{
						if (IoErr() == ERROR_OBJECT_IN_USE)
						{
							RetVal = KErrInUse;
						}
					}

					FreeDosObject(DOS_FIB, FileInfoBlock);
				}
			}

#elif defined(__unix__)

			char *ResolvedFileName;
			int Result;
			struct stat Stat;
			struct tm *Tm;

			/* Open the file itself or - in case the file is a link - possibly the file that the link points to, and */
			/* determine its properties */

			if (a_bResolveLink)
			{
				Result = stat(ProgDirName, &Stat);
			}
			else
			{
				Result = lstat(ProgDirName, &Stat);
			}

			if (Result == 0)
			{
				if ((Tm = localtime(&Stat.st_mtime)) != NULL)
				{
					RetVal = KErrNone;

					/* Convert the UNIX time information to a TDateTime that the TEntry can use internally */

					TDateTime DateTime((Tm->tm_year + 1900), (TMonth) Tm->tm_mon, Tm->tm_mday, Tm->tm_hour, Tm->tm_min, Tm->tm_sec, 0);

					/* Fill in the file's properties in the TEntry structure */

					a_poEntry->Set(S_ISDIR(Stat.st_mode), S_ISLNK(Stat.st_mode), Stat.st_size, Stat.st_mode, DateTime);
					a_poEntry->iPlatformDate = Stat.st_mtime;

					/* If the name of the directory is the special case of the root directory then return just '/' */

					if (ProgDirName[Length - 1] == '/')
					{
						a_poEntry->iName[0] = '/';
						a_poEntry->iName[1] = '\0';
					}

					/* Otherwise copy the filename into the TEntry structure */

					else
					{
						strcpy(a_poEntry->iName, filePart(ProgDirName));
					}

					/* Assume that the file either is not a link, or that we couldn't resolve it */

					a_poEntry->iLink[0] = '\0';

					/* See if the file is a link and if so, return information about the file to which the link points */

					TEntry TargetFile;

					if (a_poEntry->IsLink())
					{
						if ((ResolvedFileName = Utils::ResolveFileName(a_pccFileName)) != NULL)
						{
							strcpy(a_poEntry->iLink, Utils::filePart(ResolvedFileName));
							delete [] ResolvedFileName;
						}
					}
				}
			}

#else /* ! __unix__ */

			char *ResolvedFileName;
			DWORD Flags;
			BY_HANDLE_FILE_INFORMATION FileInformation;
			HANDLE FindHandle, Handle;
			SYSTEMTIME SystemTime;
			WIN32_FIND_DATA FindData;

			/* Open the file itself or - in case the file is a link - possibly the file that the link points to, and */
			/* determine its properties */

			Flags = FILE_FLAG_BACKUP_SEMANTICS;

			if (!a_bResolveLink)
			{
				Flags |= FILE_FLAG_OPEN_REPARSE_POINT;
			}

			if ((Handle = CreateFile(ProgDirName, 0, 0, NULL, OPEN_EXISTING, Flags, NULL)) != INVALID_HANDLE_VALUE)
			{
				if (GetFileInformationByHandle(Handle, &FileInformation))
				{
					/* Convert the file's timestamp to a more useful format that can be put into the TEntry structure */

					if (FileTimeToSystemTime(&FileInformation.ftLastWriteTime, &SystemTime))
					{
						RetVal = KErrNone;

						/* Convert the Windows SYSTEMTIME structure to a TDateTime that the TEntry can use internally */

						TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
							SystemTime.wSecond, SystemTime.wMilliseconds);

						/* Fill in the file's properties in the TEntry structure */

						a_poEntry->Set((FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT),
							FileInformation.nFileSizeLow, FileInformation.dwFileAttributes, DateTime);
						a_poEntry->iPlatformDate = FileInformation.ftLastWriteTime;

						/* Try to find the file so that we can obtain its "real" name, which may vary by case */
						/* to the one passed in, and copy it into the TEntry structure */

						if ((FindHandle = FindFirstFile(ProgDirName, &FindData)) != INVALID_HANDLE_VALUE)
						{
							strcpy(a_poEntry->iName, FindData.cFileName);
							FindClose(FindHandle);
						}

						/* The file or directory could not be queried so just copy its name the best we can */

						else
						{
							/* If the name of the directory is the special case of a Windows drive specification such as */
							/* "x:\" then return just the slash at the end.  We only need to check the slash at this */
							/* point as control would not reach here if this was a full path with a slash (such as */
							/* "c:\\Windows\\" */

							if ((a_poEntry->IsDir()) && ((ProgDirName[Length - 1] == '\\') || (ProgDirName[Length - 1] == '/')))
							{
								strcpy(a_poEntry->iName, &ProgDirName[Length - 1]);
							}

							/* Otherwise return the filename part of the path passed in */

							else
							{
								strcpy(a_poEntry->iName, Utils::filePart(ProgDirName));
							}
						}

						/* Assume that the file either is not a link, or that we couldn't resolve it */

						a_poEntry->iLink[0] = '\0';

						/* See if the file is a link and if so, return information about the file to which the link points */

						TEntry TargetFile;

						if (a_poEntry->IsLink())
						{
							if ((ResolvedFileName = Utils::ResolveFileName(a_pccFileName)) != NULL)
							{
								strcpy(a_poEntry->iLink, Utils::filePart(ResolvedFileName));
								delete [] ResolvedFileName;
							}
						}
					}
				}

				CloseHandle(Handle);
			}

#endif /* ! __unix__ */

		}
		else
		{
			RetVal = KErrNotSupported;
		}

		/* And free the resolved filename, but only if it contained the PROGDIR: prefix */

		if (ProgDirName != a_pccFileName)
		{
			delete [] ProgDirName;
		}
	}
	else
	{
		RetVal = KErrPathNotFound;
	}

	return(RetVal);
}

/**
 * Queries the system for the size of the display.
 * This function allows the caller to obtain the size of the display on which it is running.  For
 * single monitor systems this is simple enough, but for multiple monitor systems it is a little
 * more complex.  For these, a window must be passed to this function and the function will return
 * the size of the screen on which that window was opened.  This is simply a mechanism for choosing
 * which of the attached displays should be considered the "main" display for which to return
 * information.
 *
 * @date	Saturday 08-May-2010 3:16 pm, Code HQ Ehinger Tor
 * @param	a_roScreenSize	Reference to a structure in which to return the information
 * @param	a_poWindow		Pointer to the application's main window.  Only required for multiple
 *							screen systems
 */

void Utils::GetScreenSize(struct SRect &a_roScreenSize, CWindow *a_poWindow)
{
	a_roScreenSize.m_iLeft = a_roScreenSize.m_iTop = 0;
	a_roScreenSize.m_iWidth = 640;
	a_roScreenSize.m_iHeight = 480;

#ifdef __amigaos__

	(void) a_poWindow;

	struct Screen *Screen;

	if ((Screen = LockPubScreen(NULL)) != NULL)
	{
		a_roScreenSize.m_iWidth = Screen->Width;
		a_roScreenSize.m_iHeight = Screen->Height;

		UnlockPubScreen(NULL, Screen);
	}

#elif defined(__unix__)

	(void) a_poWindow;

	// TODO: CAW - Implement this
	if (0)
	{
	}

#elif !defined(QT_GUI_LIB)

	TInt InnerWidth, OuterWidth, Width, Height;
	RECT ClientRect, WindowRect;

	/* If a window was passed in then query the system for the display on which that window was opened */

	if (a_poWindow)
	{
		/* GetSystemMetrics(SM_CXSIZEFRAME) is broken from Windows Vista onwards, so to obtain the size of */
		/* the window borders we must calculate them by subtracting the size of the window's client area from */
		/* the size of the window itself */

		if (GetWindowRect(a_poWindow->m_poWindow, &WindowRect))
		{
			if (GetClientRect(a_poWindow->m_poWindow, &ClientRect))
			{
				OuterWidth = (WindowRect.right - WindowRect.left);
				InnerWidth = (ClientRect.right - ClientRect.left);
				Width = (OuterWidth - InnerWidth);

				/* For the horizontal borders the trick above does not work due to the presence of the title bar */
				/* so perform the hideous (but working) hack of multiplying the border height by 2.  Thankfully */
				/* this code will never run on a pre-Vista system.  Ugh! */

				Height = (GetSystemMetrics(SM_CYSIZEFRAME) * 2);

				/* If the window is maximised then only the client area is visible.  The borders are still */
				/* actually present but are now outside the bounds of the display.  In this case we must adjust */
				/* the size of the window or it will stretch across more than one display and we will obtain */
				/* the size information about the wrong display */

				if ((WindowRect.left < 0) || (WindowRect.top < 0))
				{
					WindowRect.left += Width;
					WindowRect.top += Height;
					WindowRect.right -= Width;
					WindowRect.bottom -= Height;
				}

				/* Query the system for the size of the display on which the window was opened */

				EnumDisplayMonitors(NULL, &WindowRect, MonitorEnumProc, (LPARAM) &a_roScreenSize);
			}
		}
	}

	/* Otherwise just return the size of the default desktop display */

	else
	{
		if (GetWindowRect(GetDesktopWindow(), &WindowRect))
		{
			a_roScreenSize.m_iWidth = (WindowRect.right - WindowRect.left);
			a_roScreenSize.m_iHeight = (WindowRect.bottom - WindowRect.top);
		}
	}

#else /* ! !defined(QT_GUI_LIB) */

	(void) a_poWindow;

#endif /* ! !defined(QT_GUI_LIB) */

}

/**
 * Returns the height of the console in which the program is running.
 * This function will determine the height of the console or shell in which the program is running,
 * in lines, and will return it.  If there is no console present (for instance, if the program is a
 * Windows application that uses WinMain() rather than main()) then it will return -1 to indicate
 * this.
 *
 * @date	Thursday 09-Jul-2009 06:58 am
 * @return	The height of the current console, or -1 if there is no console present
 */

int Utils::GetShellHeight()
{
	int RetVal;

	/* Assume failure */

	RetVal = -1;

#ifdef __amigaos__

	char Buffer[32], *BufferPtr, Length, Char;
	TInt Result;
	BPTR InHandle, OutHandle;

	/* Get the standard DOS input and output handles to use for querying the shell dimensions */

	InHandle = Input();
	OutHandle = Output();

	/* Put the console into RAW mode */

	if (SetMode(OutHandle, 1))
	{
		/* Write CSI request for console window dimensions */

		if (Write(OutHandle, "\x9b" "0 q", 4) == 4)
		{
			/* Iterate through the response and copy it into a temporary buffer */

			Length = 0;
			BufferPtr = Buffer;

			do
			{
				if ((Result = Read(InHandle, BufferPtr, 1)) > 0)
				{
					++Length;
					Char = *BufferPtr++;
				}
				else
				{
					break;
				}
			}
			while (Char != 'r');

			/* Put the console back into cooked mode. No point in checking for the unlikely error */
			/* that might happen because what would we do anyway? */

			SetMode(OutHandle, 0);

			/* If the response was extract successfully, extract the height into the supplied variable */

			if ((Length > 9) && (Buffer[0] == '\x9b'))
			{
				StrToLong(&Buffer[5], (LONG *) &RetVal);
			}
		}
		else
		{
			Utils::info("Utils::GetShellHeight() => Unable to request window dimensions");
		}
	}
	else
	{
		Utils::info("Utils::GetShellHeight() => Unable to put console into RAW mode");
	}

#elif defined(__unix__)

	// TODO: CAW - Implement this
	RetVal = 50;

#else /* ! __unix__ */

	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	HANDLE StdOut;

	/* Get the handle of the stdin device and from that obtain the console's height */

	if ((StdOut = GetStdHandle(STD_OUTPUT_HANDLE)) != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo(StdOut, &ScreenBufferInfo))
		{
			/* And determine the height of the console for the caller, converting the zero based line */
			/* number of the bottom most line into a count of lines */

			RetVal = (ScreenBufferInfo.srWindow.Bottom + 1);
		}
		else
		{
			Utils::info("Utils::GetShellHeight() => Unable to request window dimensions");
		}
	}
	else
	{
		Utils::info("Utils::GetShellHeight() => Unable get handle to console");
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Obtains a line of text from stdin.
 * This function is an overflow-safe replacement for the deprecated gets() function, that causes
 * deprecated messages on some compilers and is completely missing from others.  Use it to obtain
 * a line of text from the stdin input stream.  The string will be placed into the a_pcBuffer
 * parameter and will be NULL terminated.
 *
 * @date	Wednesday 28-Sep-2016 6:40 am, Code HQ Ehinger Tor
 * @param	a_roString		Reference to a string into which to place the line of text
 * @return	true if the string was obtained successfully, else false
 */

bool Utils::GetString(std::string &a_roString)
{
	bool Done;
	char InputBuffer[2];

	/* Loop around and try to obtain input from stdin until we have a full line of text */
	/* or an error occurs */

	Done = false;

	do
	{
		if ((fgets(InputBuffer, sizeof(InputBuffer), stdin)) && (strlen(InputBuffer) == 1))
		{
			/* If the last character is a '\n' then the user has hit <enter> and we have */
			/* successfully obtained the line.  Replace the \n with a NULL terminator and */
			/* break out */

			if (InputBuffer[0] == '\n')
			{
				Done = true;
			}

			/* Otherwise we have a valid character so append it to the input string */

			else
			{
				a_roString.push_back(InputBuffer[0]);
			}
		}
		else
		{
			/* If an error occurred then break out.  If no error occurred then the text has not */
			/* been read in its entirity so keep reading */

			if (ferror(stdin) != 0)
			{
				Done = true;
			}
		}
	} while (!Done);

	/* If all went well then ferror() will return 0 so use this for our return value */

	return(!ferror(stdin));
}

/**
 * Allocates or reallocates a temporary buffer.
 * This function is useful if you have a situation that calls for a temporary buffer of an unknown
 * and varying size (thus preventing the use of a static buffer) and do not want to dynamically
 * allocate and delete the buffer every time you use it.  If you use this function instead then
 * the buffer will only be reallocated if the new buffer is larger than the old one.  The first
 * time you call this function you should pass in NULL as the buffer pointer.  For subsequent
 * calls, pass in the buffer returned by the previous call.  When done, use Utils::FreeTempBuffer()
 * to free the allocated buffer.  If a new buffer is allocated, the contents of the old one can be
 * copied into it if desired (this is done by default).
 *
 * @date	Tuesday 28-Feb-2012 8:43 am, Code HQ Ehinger Tor
 * @param	a_pvBuffer		Pointer to the currently allocated buffer
 * @param	a_stSize		Size in bytes of the new buffer to be allocated
 * @param	a_bCopyContents	ETrue to copy the contents of the old buffer.
 *							If not specified then this is ETrue by default
 * @return	A Pointer to the allocated buffer if successful, else NULL
 */

void *Utils::GetTempBuffer(void *a_pvBuffer, size_t a_stSize, TBool a_bCopyContents)
{
	char *OldBuffer, *RetVal;
	size_t Size;

	/* Assume failure */

	OldBuffer = RetVal = NULL;
	Size = 0;

	/* If the buffer has already been allocated then check to see if it is */
	/* large enough to hold the newly requested size */

	if (a_pvBuffer)
	{
		/* The size is stored in the long word just before the pointer that is */
		/* returned to the user */

		Size = *(size_t *) ((char *) a_pvBuffer - sizeof(a_stSize));

		/* If the current buffer is large enough then reuse it */

		if (Size >= a_stSize)
		{
			RetVal = (char *) a_pvBuffer;
		}

		/* Otherwise indicate that a new buffer is to be allocated */

		else
		{
			OldBuffer = (char *) a_pvBuffer;
			a_pvBuffer = NULL;
		}
	}

	/* If no buffer is allocated then allocate it now */

	if (!a_pvBuffer)
	{
		// TODO: CAW - How do we ensure this doesn't throw an exception here and for other operating systems?
		RetVal = new char[a_stSize + sizeof(a_stSize)];

		/* Save the size of the buffer in the first long word of the buffer and return */
		/* a pointer to just after that word to the user */

		*(size_t *) RetVal = a_stSize;
		RetVal += sizeof(a_stSize);

		/* If the buffer was already allocated, copy the old contents into the new buffer */
		/* if requested, and free the old buffer */

		if (OldBuffer)
		{
			if (a_bCopyContents)
			{
				memcpy(RetVal, OldBuffer, Size);
			}

			delete [] (OldBuffer - sizeof(a_stSize));
		}
	}

	return(RetVal);
}

/**
 * Frees a temporary buffer.
 * Frees a buffer allocated with Utils::GetTempBuffer().  It is ok to pass in NULL to this method.
 *
 * @date	Tuesday 28-Feb-2012 9:00 am, Code HQ Ehinger Tor
 * @param	a_pvBuffer		Pointer to the buffer to be freed
 */

void Utils::FreeTempBuffer(void *a_pvBuffer)
{
	if (a_pvBuffer)
	{
		delete [] ((char *) a_pvBuffer - sizeof(size_t));
	}
}

#ifdef _DEBUG

/**
 * Displays an information or warning message.
 * Displays a printf() style formatted message.  The message is displayed in an OS specific manner.
 * On most systems, this means that it will be output to the debug console and will thus only be
 * viewable with a debugging tool, which is quite often the development IDE itself.  On Mac OS it
 * is output to stdout.  The message is only displayed if the code is compiled in debug mode, and
 * thus this function should only be used for displaying informational messages.
 *
 * @param	a_pccMessage	The message to be displayed, in printf() format
 */

void Utils::info(const char *a_pccMessage, ...)
{
	char Message[512];
	va_list Args;

	va_start(Args, a_pccMessage);

	strcpy(Message, "Info: " );
	VSNPRINTF(&Message[6], (sizeof(Message) - 6), a_pccMessage, Args);

#ifdef __amigaos4__

	DebugPrintF("%s\n", Message);

#elif defined(__amigaos__)

	KPrintF("%s\n", Message);

#elif defined(__unix__)

	syslog(LOG_INFO, "%s", Message);

#ifdef __APPLE__

	/* On Mac OS syslog output is difficult to find and doesn't get picked up by the output */
	/* window of IDEs such as Qt Creator, so we'll output the messages to stdout as well. */
	/* The call to fflush() is to ensure that it is still printed in the event of a crash */

	printf("%s\n", Message);
	fflush(stdout);

#endif /* __APPLE__ */

#else /* ! __unix__ */

	OutputDebugString(Message);
	OutputDebugString("\n");

#endif /* ! __unix__ */

	va_end(Args);
}

#endif /* _DEBUG */

/**
 * Loads a file into memory in its entirety.
 * This is a convenience function that can be used to load a file into memory.  The file's size
 * will be checked, just the right amount of memory allocated, and the file will be loaded.  A
 * single byte more is allocated than is required and this has a 0 placed into it.  This is to
 * make it easier to parse text files, as it can be depended upon that there is a NULL terminator
 * at the end of the file.
 *
 * The pointer to the allocated buffer is returned to the user in *a_ppucBuffer and it is up to
 * the user to delete [] this memory.  If the function fails to load the file, a_ppucBuffer will
 * be set to NULL.
 *
 * @param	a_pccFileName	Pointer to the name of the file to be opened
 * @param	a_ppucBuffer	Pointer to a variable into which to place the pointer to the allocated
 *							buffer
 * @return	The size of the file in bytes, not including the NULL terminator, if successful
 * @return	KErrNoMemory if not enough memory was available
 * @return	Otherwise any of the errors returned by Utils::GetFileInfo(), RFile::open() or
 *			RFile::read()
 */

TInt Utils::LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer)
{
	unsigned char *Buffer;
	TInt RetVal;
	TEntry Entry;

	/* Assume failure */

	*a_ppucBuffer = NULL;

	/* Obtain information about the file, such as its size */

	if ((RetVal = Utils::GetFileInfo(a_pccFileName, &Entry)) == KErrNone)
	{
		/* Allocate a buffer of the appropriate size */

		if ((Buffer = new unsigned char[Entry.iSize + 1]) != NULL)
		{
			RFile File;

			/* Open the file and read its contents into the buffer */

			if ((RetVal = File.open(a_pccFileName, EFileRead)) == KErrNone)
			{
				if ((RetVal = File.read(Buffer, Entry.iSize)) == (TInt) Entry.iSize)
				{
					/* NULL terminate the buffer and save its pointer for the calling client */

					Buffer[Entry.iSize] = '\0';
					*a_ppucBuffer = Buffer;
				}
				else
				{
					Utils::info("Utils::LoadFile() => Unable to read file \"%s\"", a_pccFileName);
				}
			}
			else
			{
				Utils::info("Utils::LoadFile() => Unable to open file \"%s\" for loading", a_pccFileName);
			}

			File.close();

			/* If any error occurred, free the buffer as it won't get passed back to the client */

			if (RetVal < 0)
			{
				delete [] Buffer;
				*a_ppucBuffer = NULL;
			}
		}
		else
		{
			RetVal = KErrNoMemory;

			Utils::info("Utils::LoadFile() => Out of memory");
		}
	}

	return(RetVal);
}

/**
 * Converts a path to native format.
 * Iterates through all the characters of a qualified path and converts any instances of the '/'
 * and '\' directory separator character to either '/' when running under UNIX or Amiga OS or to
 * '\' when running under Windows.
 *
 * @date	Monday 15-Feb-2016 07:30 am, Code HQ Ehinger Tor
 * @param	a_pcPath		Pointer to qualified path to be localised
 */

void Utils::LocalisePath(char *a_pcPath)
{
	size_t Index, Length;

	/* Iterate through all the characters of the path and localise any instances of '\' to '/' */

	Length = strlen(a_pcPath);

	for (Index = 0; Index < Length; ++Index)
	{

#ifdef WIN32

		if (a_pcPath[Index] == '/')
		{
			a_pcPath[Index] = '\\';
		}

#else /* ! WIN32 */

		if (a_pcPath[Index] == '\\')
		{
			a_pcPath[Index] = '/';
		}

#endif /* ! WIN32 */

	}
}

/**
 * Creates a soft link to a file.
 * This function will create a soft link to a destination file.  The destination file does not
 * need to exist at the time of link creation.  Both the source and destination filenames can
 * be either absolute or relative.
 *
 * @date	Monday 11-Jan-2016 07:09 am, Code HQ Ehinger Tor
 * @param	a_pccSource		Pointer to the name of the link to create
 * @param	a_pccDest		Pointer to the name of the file to which to link to
 * @return	KErrNone if the link was created successfully
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if the link was not able to be created
 */

TInt Utils::makeLink(const char *a_pccSource, const char *a_pccDest)
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;

#ifdef __amigaos__

	if (MakeLink(a_pccSource, (APTR) a_pccDest, LINK_SOFT) == 0)
	{
		RetVal = KErrGeneral;
	}

#elif defined(__unix__)

	if (symlink(a_pccDest, a_pccSource) != 0)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __unix__ */

	char *DestFileName;

	(void) a_pccSource;

	/* The Framework works with the '/' but this does not work when creating links in Windows, so */
	/* we must copy the qualified link target into a temporary buffer and localise it so that it */
	/* contains only the '\' path separator */

	if ((DestFileName = new char[strlen(a_pccDest) + 1]) != NULL)
	{
		strcpy(DestFileName, a_pccDest);
		LocalisePath(DestFileName);

		/* Now create the symbolic link using the localised path */

#ifdef _WIN32_WINNT_LONGHORN

		if (CreateSymbolicLink(a_pccSource, DestFileName, 0) == 0)

#endif /* _WIN32_WINNT_LONGHORN */

		{
			RetVal = KErrGeneral;
		}

		delete [] DestFileName;
	}
	else
	{
		RetVal = KErrNoMemory;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Converts a path to normalised (UNIX/Amiga OS) format.
 * Iterates through all the characters of a qualified path and converts any instances of the '\'
 * directory separator charater to '/' to make parsing easier.
 *
 * @date	Wednesday 09-Mar-2011 6:27 am
 * @param	a_pcPath	Pointer to qualified path to be normalised
 */

void Utils::NormalisePath(char *a_pcPath)
{
	size_t Index, Length;

	/* Iterate through all the characters of the path and pass any instances of '\' to '/' */

	Length = strlen(a_pcPath);

	for (Index = 0; Index < Length; ++Index)
	{
		if (a_pcPath[Index] == '\\')
		{
			a_pcPath[Index] = '/';
		}
	}

#ifdef WIN32

	/* Some Windows APIs return a capital drive letter and some return a lower case one, so */
	/* make it consistent */

	if (Length >= 2)
	{
		a_pcPath[0] = (char) toupper(a_pcPath[0]);
	}

#endif /* WIN32 */

}

/**
 * Displays a generic message box and waits for input.
 * This is the internal version of the same named public Utils::MessageBox() method.  See the
 * documentation for that method for more information.
 *
 * @date	Saturday 12-Mar-2011 11:00 am
 * @param	a_eMessageBoxType	Type of message box to display, as specified by the
 *								TMessageBoxType enumeration
 * @param	a_pccTitle			Pointer to the title to be displayed in the mesage box
 * @param	a_pccMessage		Pointer to the message to be displayed.  This can contain printf()
 *								style specifiers that will be processed and matched with arguments
 *								passed in in the a_oArgs varargs list
 * @param	a_oArgs				List of arguments with which to populate the printf() style %d
 *								specifiers
 * @return	IDOK, IDCANCEL, IDYES or IDNO if successful, indicating which button was pressed
 * @return	0 if the message box could not be opened
 */

TInt Utils::MessageBox(enum TMessageBoxType a_eMessageBoxType, const char *a_pccTitle, const char *a_pccMessage, va_list a_oArgs)
{
	char Message[512];
	TInt RetVal;

	/* Format the message for use no matter how it is displayed */

	VSNPRINTF(Message, sizeof(Message), a_pccMessage, a_oArgs);

#if !defined(__unix__) || defined(QT_GUI_LIB)

	CWindow *RootWindow;

	/* See if a root window has been set by the application and if so, open the message box on */
	/* that window.  Otherwise just open it on the desktop or Workbench */

	RootWindow = CWindow::GetRootWindow();

#endif /* !defined(__unix__) || defined(QT_GUI_LIB) */

#ifdef __amigaos__

	TInt Result;
	struct EasyStruct EasyStruct;

	/* Assume failure */

	RetVal = IDCANCEL;

	/* Build an EasyStruct for displaying the requester */

	EasyStruct.es_StructSize = sizeof(struct EasyStruct);
	EasyStruct.es_Flags = 0;
	EasyStruct.es_Title = a_pccTitle;
	EasyStruct.es_TextFormat = Message;

#ifdef __amigaos4__

	struct TagItem Tags[] = { { ESA_Underscore, '_' }, { TAG_DONE, 0 } };

	EasyStruct.es_Flags = ESF_TAGGED;
	EasyStruct.es_TagList = Tags;

#endif /* __amigaos4__ */

	/* Determine the type of requester to display, based on the type passed in */

	if (a_eMessageBoxType == EMBTOk)
	{
		EasyStruct.es_GadgetFormat = g_accOk;
	}
	else if (a_eMessageBoxType == EMBTOkCancel)
	{
		EasyStruct.es_GadgetFormat = g_accOkCancel;
	}
	else if (a_eMessageBoxType == EMBTYesNo)
	{
		EasyStruct.es_GadgetFormat = g_accYesNo;
	}
	else
	{
		EasyStruct.es_GadgetFormat = g_accYesNoCancel;
	}

	/* Display the requester */

	Result = EasyRequest((RootWindow) ? RootWindow->m_poWindow : NULL, &EasyStruct, NULL);

	/* And convert the result to one of the standard return values */

	if (Result >= 0)
	{
		if (a_eMessageBoxType == EMBTOk)
		{
			RetVal = IDOK;
		}
		else if (a_eMessageBoxType == EMBTOkCancel)
		{
			RetVal = (Result == 0) ? IDCANCEL : IDOK;
		}
		else if (a_eMessageBoxType == EMBTYesNo)
		{
			RetVal = (Result == 0) ? IDNO : IDYES;
		}
		else
		{
			if (Result == 0)
			{
				RetVal = IDCANCEL;
			}
			else
			{
				RetVal = (Result == 2) ? IDNO : IDYES;
			}
		}
	}
	else
	{
		PRINTF("%s: %s\n", a_pccTitle, Message);
	}

#elif defined(QT_GUI_LIB)

	QWidget *ParentWindow;
	QMessageBox::StandardButtons Buttons;
	QMessageBox::StandardButton Result;

	/* Determine the type of Qt message box to display, based on the type passed in */

	if (a_eMessageBoxType == EMBTOk)
	{
		Buttons = QMessageBox::Ok;
	}
	else if (a_eMessageBoxType == EMBTOkCancel)
	{
		Buttons = (QMessageBox::Ok | QMessageBox::Cancel);
	}
	else if (a_eMessageBoxType == EMBTYesNo)
	{
		Buttons = (QMessageBox::Yes | QMessageBox::No);
	}
	else
	{
		Buttons = (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	}

	/* Display the appropriate message box for the message box type passed in */

	ParentWindow = (RootWindow) ? (QWidget *) RootWindow->m_poWindow : NULL;

	if (a_eMessageBoxType == EMBTOk)
	{
		Result = QMessageBox::information(ParentWindow, a_pccTitle, Message, Buttons);
	}
	else
	{
		Result = QMessageBox::question(ParentWindow, a_pccTitle, Message, Buttons);
	}

	/* And convert the result to one of the standard return values */

	if (a_eMessageBoxType == EMBTOk)
	{
		RetVal = IDOK;
	}
	else if (a_eMessageBoxType == EMBTOkCancel)
	{
		RetVal = (Result == QMessageBox::Cancel) ? IDCANCEL : IDOK;
	}
	else if (a_eMessageBoxType == EMBTYesNo)
	{
		RetVal = (Result == QMessageBox::No) ? IDNO : IDYES;
	}
	else
	{
		if (Result == QMessageBox::Cancel)
		{
			RetVal = IDCANCEL;
		}
		else
		{
			RetVal = (Result == QMessageBox::No) ? IDNO : IDYES;
		}
	}

#elif defined(WIN32)

	UINT Type;

	/* Determine the type of Windows message box to display, based on the type passed in */

	if (a_eMessageBoxType == EMBTOk)
	{
		Type = (MB_OK | MB_ICONINFORMATION);
	}
	else if (a_eMessageBoxType == EMBTOkCancel)
	{
		Type = (MB_OKCANCEL | MB_ICONQUESTION);
	}
	else if (a_eMessageBoxType == EMBTYesNo)
	{
		Type = (MB_YESNO | MB_ICONQUESTION);
	}
	else
	{
		Type = (MB_YESNOCANCEL | MB_ICONQUESTION);
	}

	/* Display the message box and return the standard Windows return code */

	RetVal = ::MessageBox((RootWindow) ? RootWindow->m_poWindow : NULL, Message, a_pccTitle, Type);

#else /* ! defined(WIN32) */

	(void) a_eMessageBoxType;

	/* Using a non GUI version of The Framework so just print the message and indicate that the */
	/* message box could not be opened */

	PRINTF("%s: %s\n", a_pccTitle, Message);
	RetVal = 0;

#endif /* ! WIN32 */

	return(RetVal);
}

/**
 * Displays a generic message box and waits for input.
 * Opens a message box to prompt the user with a question or simply to display information.
 * Different types of message boxes may be opened, which contain combinations of Ok, Cancel, Yes
 * and No buttons according to the a_eMessageBoxType passed in.
 *
 * @date	Saturday 12-Mar-2011 6:25 pm
 * @param	a_eMessageBoxType	Type of message box to display, as specified by the
 *								TMessageBoxType enumeration
 * @param	a_pccTitle			Pointer to the title to be displayed in the mesage box
 * @param	a_pccMessage		Pointer to the message to be displayed.  This can contain printf()
 *								style specifiers that will be processed and matched with arguments
 *								passed in in the a_oArgs varargs list
 * @return	IDOK, IDCANCEL, IDYES or IDNO if successful, indicating which button was pressed
 * @return	0 if the message box could not be opened
 */

TInt Utils::MessageBox(enum TMessageBoxType a_eMessageBoxType, const char *a_pccTitle, const char *a_pccMessage, ...)
{
	TBool RetVal;
	va_list Args;

	va_start(Args, a_pccMessage);

	RetVal = MessageBox(a_eMessageBoxType, a_pccTitle, a_pccMessage, Args);

	va_end(Args);

	return(RetVal);
}

/**
 * Converts a filename to a fully qualified filename.
 * Takes a filename that contains either no path or a relative path and converts it to a fully
 * qualified filename.  The function will attempt to determine the true path to the filename,
 * which will only work if the file actually exists.  If this is not the case then the current
 * directory will be used instead.  In the event that even this cannot be obtained, the function
 * will fail.
 *
 * The use of the current directory is useful when creating new files, but care must be taken
 * if using this function with software that changes the current directory, especially if the
 * changes could be done in a separate thread.
 *
 * It is the responsibility of the caller to free the returned buffer with delete [].  It is safe
 * to pass in an already qualified filename - the filename will simply be copied into an allocated
 *  buffer and returned.
 *
 * Despite its name, this function works for both directories and files. For directories it will
 * "clean" the names returned;  that is, it will ensure that there is no trailing \ or / character
 * at the end of the directory name (unless it is the root directory) as this can cause confusion
 * with calling software if it tries to examine the returned path to determine its type.
 *
 * @date	Thursday 01-Nov-2012 5:24 am, Code HQ Ehinger Tor
 * @param	a_pccFileName		Pointer to filename to be resolved
 * @param	a_bGetDeviceName	ETrue to return the underlying device name, instead of the volume
 *								name, on systems that support this.  EFalse by default
 * @return	Pointer to the fully qualified name of the filename passed in, if successful, else NULL
 */

char *Utils::ResolveFileName(const char *a_pccFileName, TBool a_bGetDeviceName)
{
	char *RetVal;

#ifdef __amigaos__

	TBool LockedFile;
	TInt Result;
	BPTR Lock;

	/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
	/* by dos.library's autodocs */

	if ((RetVal = new char[MAX_NAME_FROM_LOCK_LENGTH]) != NULL)
	{
		/* Get a lock on the specified filename and convert it to a qualified filename */

		if ((Lock = Lock(a_pccFileName, SHARED_LOCK)) != 0)
		{
			LockedFile = ETrue;
		}
		else
		{
			/* Obtaining the lock failed, probably due to the file not existing, so get */
			/* a lock on the current directory instead */

			Utils::info("Utils::ResolveFileName() => Unable to obtain lock on \"%s\"", a_pccFileName);

			LockedFile = EFalse;
			Lock = Lock("", SHARED_LOCK);
		}

		/* Get the fully qualified name of the file (if the lock was obtained successfully) */
		/* or the path of the current directory (if it wasn't), taking into account whether */
		/* the user wants the device name or the volume name */

		if (a_bGetDeviceName)
		{

#ifdef __amigaos4__

			Result = DevNameFromLock(Lock, RetVal, MAX_NAME_FROM_LOCK_LENGTH, DN_FULLPATH);

#else /* ! __amigaos4__ */

			ASSERTM(0, "Utils::ResolveFileName() => Not implemented for OS3");

			Result = 0;

#endif /* ! __amigaos4__ */

		}
		else
		{
			Result = NameFromLock(Lock, RetVal, MAX_NAME_FROM_LOCK_LENGTH);
		}

		if (Result == 0)
		{
			Utils::info("Utils::ResolveFileName() => Unable to determine qualified filename for \"%s\"", a_pccFileName);

			delete [] RetVal;
			RetVal = NULL;
		}

		/* If we obtained the directory name then append the filename to it */

		if (!(LockedFile))
		{
			Utils::addPart(RetVal, a_pccFileName, MAX_NAME_FROM_LOCK_LENGTH);
		}

		/* And unlock the lock on the file, if it was obtained successfully.  If it is a */
		/* directory lock then it is just a copy of the lock, so don't unlock it */

		if (LockedFile)
		{
			UnLock(Lock);
		}
	}
	else
	{
		Utils::info("Utils::ResolveFileName() => Out of memory");
	}

#elif defined(__unix__)

	(void) a_bGetDeviceName;

	/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
	/* by the realpath() manpage */

	if ((RetVal = new char[PATH_MAX]) != NULL)
	{
		/* Convert the filename into a fully qualified path and put it in the allocated buffer */

		if (!(realpath(a_pccFileName, RetVal)))
		{
			/* Converting the filename failed, possibly due to the file not existing.  Check whether */
			/* the filename is already resolved and if so, just use it as is */

			if (a_pccFileName[0] == '/')
			{
				strcpy(RetVal, a_pccFileName);
			}

			/* Otherwise get the current directory and append the filename to it */

			else
			{
				if (getcwd(RetVal, PATH_MAX))
				{
					Utils::addPart(RetVal, a_pccFileName, PATH_MAX);
				}

				/* We couldn't even get the current directory so free the buffer and return failure */

				else
				{
					delete [] RetVal;
					RetVal = NULL;
				}
			}
		}
	}
	else
	{
		Utils::info("Utils::ResolveFileName() => Out of memory");
	}

#else /* ! __unix__ */

	size_t Length;

	(void) a_bGetDeviceName;

	/* Assume failure */

	RetVal = NULL;

#ifdef _WIN32_WINNT_LONGHORN

	HANDLE Handle;

	/* Open the file so that we can query information about it.  We use the FILE_FLAG_OPEN_REPARSE_POINT as this */
	/* function can be used to query information about links so we do not want to resolve them */

	if ((Handle = CreateFile(a_pccFileName, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL)) != INVALID_HANDLE_VALUE)
	{
		/* Allocate a buffer into which to place the real name of the file and query Windows for that name */

		if ((RetVal = new char[MAX_PATH]) != NULL)
		{
			if (GetFinalPathNameByHandle(Handle, RetVal, (MAX_PATH - 1), 0) > 0)
			{
				/* The "\\?\" prefix is used to denote that the path refers to the underlying device namespace */
				/* rather than the file namespace.  This is not useful to use so we strip it off if it is present */

				if (strncmp(RetVal, "\\\\?\\", 4) == 0)
				{
					/* If the path refers to a file on a network drive then it will be in UNC format.  This is not */
					/* useful for us, so delete the resolved string and we will fall through to using the pre */
					/* Longhorn functionality below */

					if (strncmp(RetVal, "\\\\?\\UNC", 7) == 0)
					{
						delete [] RetVal;
						RetVal = NULL;
					}
					else
					{
						Length = (strlen(RetVal) - 4);
						memmove(RetVal, (RetVal + 4), Length);
						RetVal[Length] = '\0';
					}
				}

				if (RetVal)
				{
					NormalisePath(RetVal);
				}
			}
			else
			{
				delete [] RetVal;
				RetVal = NULL;
			}

			CloseHandle(Handle);
		}
	}

#endif /* _WIN32_WINNT_LONGHORN */

	TBool RemoveSlash;

	/* If the filename could not be resolved then fall through to the simplified implementation of this function */

	if (!RetVal)
	{
		/* Determine now much memory is required to hold the fully qualified */
		/* path and allocate a buffer of the required size */

		if ((Length = GetFullPathName(a_pccFileName, 0, NULL, NULL)) > 0)
		{
			if ((RetVal = new char[Length]) != NULL)
			{
				/* Convert the filename into a fully qualified path and put it in */
				/* the allocated buffer */

				if ((Length = GetFullPathName(a_pccFileName, (DWORD) Length, RetVal, NULL)) > 0)
				{
					/* Remove any trailing '\' characters at the end of the returned filename.  Windows */
					/* helpfully converts any '/' passed in to a '\' so we only need to worry about backslashes */

					RemoveSlash = ETrue;

					/* Have a special check for paths referring to the root directory of a particular drive */

					if ((Length == 3) && (RetVal[1] == ':'))
					{
						RemoveSlash = EFalse;
					}

					/* And remove the slash if required */

					if ((RemoveSlash) && (RetVal[Length - 1] == '\\'))
					{
						RetVal[Length - 1] = '\0';
					}

					/* Ensure that forward slashes and an upper case drive letter are used */

					NormalisePath(RetVal);
				}
				else
				{
					Utils::info("Utils::ResolveFileName() => Unable to determine qualified filename");

					delete [] RetVal;
					RetVal = NULL;
				}
			}
			else
			{
				Utils::info("Utils::ResolveFileName() => Out of memory");
			}
		}
		else
		{
			Utils::info("Utils::ResolveFileName() => Unable to determine length of qualified filename");
		}
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Resolves an Amiga OS style PROGDIR: prefix.
 * Takes a path and filename that may or may not contain the Amiga style "PROGDIR:" prefix at the start and,
 * if this prefix is found, determines the directory from which the program was executed and replaces
 * PROGDIR: with this path + '/'.  This is useful for loading resources from the directory in which the program
 * resides.  It then returns a pointer to a fully qualified path that can be used for opening the file.  It is
 * the responsibility of the client to free the buffer containing this path.  If the filename passed in does
 * not contain this prefix, a pointer to the filename passed in is returned.
 *
 * @date	Monday 17-Dec-2012 6:24 am, John & Sally's House
 * @param	a_pccFileName	Pointer to filename to be resolved
 * @return	Pointer to the fully qualified name of the filename passed in, if the filename passed
 *			in was suffixed with "PROGDIR:"
 * @return	Pointer to a_pccFileName if no suffix was present
 * @return	NULL if the conversion of the path failed
 */

char *Utils::ResolveProgDirName(const char *a_pccFileName)
{
	char *RetVal;
	TBool Ok;

	/* If the file being opened is prefixed with the magic "PROGDIR:" prefix */
	/* then determine the path to the executable that is running this code */
	/* and try to determine the specified file in the same directory */

	if (strncmp(a_pccFileName, g_accProgDir, PROGDIR_LENGTH) == 0)
	{
		/* Prepare for possible failure */

		RetVal = NULL;
		Ok = EFalse;

#ifdef __amigaos__

		BPTR Lock;

		/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
		/* by dos.library's autodocs */

		if ((RetVal = new char[MAX_NAME_FROM_LOCK_LENGTH]) != NULL)
		{
			/* Get a lock on the specified filename and convert it to a qualified filename */

			if ((Lock = GetProgramDir()) != 0)
			{
				if (NameFromLock(Lock, RetVal, MAX_NAME_FROM_LOCK_LENGTH ) != 0)
				{
					/* Append the name of the file to be opened in the executable's directory */

					Ok = Utils::addPart(RetVal, &a_pccFileName[PROGDIR_LENGTH], MAX_NAME_FROM_LOCK_LENGTH);
				}
				else
				{
					Utils::info("Utils::ResolveProgDirName() => Unable to determine qualified filename for \"%s\"", a_pccFileName);
				}
			}
			else
			{
				Utils::info("Utils::ResolveProgDirName() => Unable to obtain lock program directory");
			}
		}
		else
		{
			Utils::info("Utils::ResolveProgDirName() => Out of memory");
		}

#elif defined(__unix__)

		TInt Result;
		char *FileNamePart;

		/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
		/* by the realpath() manpage */

		if ((RetVal = new char[PATH_MAX]) != NULL)
		{

#ifdef __APPLE__

			char *CanonicalPath;
			uint32_t Length;

			Length = PATH_MAX;
			Result = -1;

			/* Query the path to the executable.  It is possible for this to dynamically return */
			/* how much memory is required to hold the returned path, but then realpath() below */
			/* has a maximum hard coded size, so there is no point using this feature */

			if (_NSGetExecutablePath(RetVal, &Length) == 0)
			{
				/* Try to resolve symlinks, to find the location of the real executable */

				if ((CanonicalPath = realpath(RetVal, nullptr)) != nullptr)
				{
					Length = static_cast<uint32_t>(strlen(CanonicalPath));

					if (Length < MAX_PATH)
					{
						strncpy(RetVal, CanonicalPath, MAX_PATH);
						Result = static_cast<TInt>(Length);
					}

					free(CanonicalPath);
				}
			}

#else /* ! __APPLE__ */

			/* Get the path to the executable running this code.  Unfortunately the */
			/* API doesn't allow us to query for the length of the path like some other */
			/* APIs so we have to use a fixed size buffer and hope for the best */

			Result = readlink("/proc/self/exe", RetVal, PATH_MAX);

#endif /* ! __APPLE__ */

			if (Result != -1)
			{
				RetVal[Result] = '\0';

				/* Get a pointer to the name of the executable and remove it */

				FileNamePart = (char *) Utils::filePart(RetVal);
				*FileNamePart = '\0';

				/* Append the name of the file to be opened in the executable's directory */

				Ok = Utils::addPart(FileNamePart, &a_pccFileName[PROGDIR_LENGTH], PATH_MAX);
			}

			/* The path could not be obtained so display a debug string and just let the call fail */

			else
			{
				Utils::info("Utils::ResolveProgDirName() => Cannot obtain path to executable");
			}
		}
		else
		{
			Utils::info("Utils::ResolveFileName() => Out of memory");
		}

#else /* ! __unix__ */

		char *FileNamePart;

		/* Allocate a buffer large enough to hold the fully qualified filename, as */
		/* specified by the GetModuleFileName() documentation */

		if ((RetVal = new char[MAX_PATH]) != NULL)
		{
			/* Get the path to the executable running this code.  Unfortunately the */
			/* API doesn't allow us to query for the length of the path like other */
			/* Windows APIs so we have to use a fixed size buffer and hope for the best */

			if (GetModuleFileName(NULL, RetVal, MAX_PATH) > 0)
			{
				/* Get a pointer to the name of the executable and remove it */

				FileNamePart = (char *) Utils::filePart(RetVal);
				*FileNamePart = '\0';

				/* Append the name of the file to be opened in the executable's directory */

				Ok = Utils::addPart(FileNamePart, &a_pccFileName[PROGDIR_LENGTH], MAX_PATH);
			}

			/* The path could not be obtained so display a debug string and just let the call fail */

			else
			{
				Utils::info("Utils::ResolveProgDirName() => Cannot obtain path to executable");
			}
		}
		else
		{
			Utils::info("Utils::ResolveProgDirName() => Out of memory");
		}

#endif /* ! __unix__ */

		/* If anything failed then free the string the was allocated and indicate failure */

		if (!(Ok))
		{
			delete [] RetVal;
			RetVal = NULL;
		}
	}

	/* Nothing has been resolved so just return the original filename */

	else
	{
		RetVal = (char *) a_pccFileName;
	}

	return(RetVal);
}

/**
 * Removes write protection from a file.
 * Makes the file specified by the a_pccFileName parameter deleteable.  This is done in a slightly
 * different manner on different platfoms.  On Amiga OS it clears the 'd' bit.  On UNIX it sets
 * the current user's 'w' bit and on Windows it clears the read only, system and hidden bits by
 * setting the file type to normal, thus clearing all bits.
 *
 * @date	Thursday 16-Aug-2012 12:55 pm
 * @param	a_pccFileName	Pointer to the name of the file to be made deleteable
 * @return	KErrNone if successful
 * @return	KErrNotFound if the file could not be found
 * @return	KErrGeneral if some other unspecified error occurred
 */

TInt Utils::SetDeleteable(const char *a_pccFileName)
{
	TInt RetVal;
	TEntry Entry;

#if defined(__amigaos__) || defined(__unix__)

	/* Get the current file attributes as the chmod() function only lets us set all attributes, */
	/* not just a single one */

	// TODO: CAW - Extra errors?
	if ((RetVal = Utils::GetFileInfo(a_pccFileName, &Entry)) == KErrNone)
	{
		/* Now make the file writeable so that we can delete it */

#ifdef __amigaos__

		Entry.iAttributes &= ~EXDF_NO_DELETE;

#else /* ! __amigaos__ */

		Entry.iAttributes |= S_IWUSR;

#endif /* ! __amigaos__ */

		RetVal = Utils::setProtection(a_pccFileName, Entry.iAttributes);
	}

#else /* ! defined(__amigaos__) || defined(__unix__) */

	/* Use our version of setProtection() rather than the native SetFileAttributes() function */
	/* so that we don't have to worry about error handling */

	RetVal = Utils::setProtection(a_pccFileName, FILE_ATTRIBUTE_NORMAL);

#endif /* ! defined(__amigaos__) || defined(__unix__) */

	return(RetVal);

}

/**
 * Sets the time and date of a file.
 * Takes the time and date passed in and assigns it to a file.  This function can follow symbolic
 * links in order to set the time and date on the link's target, if requested.  However, this
 * functionality does not work on Amiga OS.
 *
 * @date	Saturday 18-Jul-2009 8:06 am
 * @param	a_pccFileName	The name of the file for which to set the time and date
 * @param	a_roEntry		A TEntry structure containing the time and date to be set
 * @param	a_bResolveLink	ETrue to follow links, else EFalse
 * @return	KErrNone if the file's time and date were set successfully
 * @return	KErrNotFound if the file could not be found
 * @return	KErrGeneral if any other error occurred
 */

TInt Utils::setFileDate(const char *a_pccFileName, const TEntry &a_roEntry, TBool a_bResolveLink)
{
	TInt RetVal;

#ifdef __amigaos__

	(void) a_bResolveLink;

#ifdef __amigaos4__

	if (SetDate(a_pccFileName, &a_roEntry.iPlatformDate) != 0)

#else /* ! __amigaos4__ */

	if (SetFileDate(a_pccFileName, &a_roEntry.iPlatformDate) != 0)

#endif /* ! __amigaos4__ */

	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (IoErr() == ERROR_OBJECT_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#elif defined(__unix__)

	int Result;
	struct timeval TimeVal[2];

	/* Set both the access and modification time of the file to the time passed in */

	TimeVal[0].tv_sec = TimeVal[1].tv_sec = a_roEntry.iPlatformDate;
	TimeVal[0].tv_usec = TimeVal[1].tv_usec = 0;

	/* Set the time stamp of either the file itself, or the link that points to it, as requested */

	if (a_bResolveLink)
	{
		Result = utimes(a_pccFileName, TimeVal);
	}
	else
	{
		Result = lutimes(a_pccFileName, TimeVal);
	}

	if (Result == 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (errno == ENOENT) ? KErrNotFound : KErrGeneral;
	}

#else /* ! __unix__ */

	DWORD Flags;
	HANDLE Handle;

	/* Open the file itself or - in case the file is a link - possibly the file that the link points to, and */
	/* set its datestamp to that passed in */

	Flags = FILE_FLAG_BACKUP_SEMANTICS;

	if (!a_bResolveLink)
	{
		Flags |= FILE_FLAG_OPEN_REPARSE_POINT;
	}

	if ((Handle = CreateFile(a_pccFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, Flags, NULL)) != INVALID_HANDLE_VALUE)
	{
		if (SetFileTime(Handle, &a_roEntry.iPlatformDate, &a_roEntry.iPlatformDate, &a_roEntry.iPlatformDate))
		{
			RetVal = KErrNone;
		}
		else
		{
			RetVal = KErrGeneral;
		}

		CloseHandle(Handle);
	}
	else
	{
		RetVal = (GetLastError() == ERROR_FILE_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Sets the protection bits of a file.
 * Assigns a set of protection bits to the specified file.  Note that the protection bits are native
 * to the platform on which the function is being called!  This parameter is of the same format as
 * that found in the TEntry::iAttributes member variable and this can be passed unchanged into this
 * function if possible.
 *
 * @date	Saturday 18-Jul-2009 7:59 am
 * @param	a_pccFileName	Pointer to the name of the file for which to set the protection bits
 * @param	a_uiAttributes	The set of protection bits to apply to the file
 * @return	KErrNone if the protection bits were set successfully
 * @return	KErrNotFound if the file could not be found
 * @return	KErrGeneral if some other unspecified error occurred
 */

TInt Utils::setProtection(const char *a_pccFileName, TUint a_uiAttributes)
{
	TInt RetVal;

#ifdef __amigaos__

	if (SetProtection(a_pccFileName, a_uiAttributes))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (IoErr() == ERROR_OBJECT_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#elif defined(__unix__)

	if (chmod(a_pccFileName, a_uiAttributes) == 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (errno == ENOENT) ? KErrNotFound : KErrGeneral;
	}

#else /* ! __unix__ */

	if (SetFileAttributes(a_pccFileName, a_uiAttributes))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (GetLastError() == ERROR_FILE_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#endif /* ! __unix__ */

	return(RetVal);
}

/**
 * Strips white space from the start and end of the line.
 * This function will strip spaces and tabs from the start and end of the line, as well as the
 * carriage return (0x0d) from the end.  It returns a pointer to the first non whitespace
 * character in the line and the length of the "new" line, that has the white space stripped.
 *
 * @date	Sunday 14-Jul-2013 8:36 am, Code HQ Ehinger Tor
 * @param	a_pcLine	Pointer to the line to have its white space stripped
 * @param	a_piLength	Number of characters in the line to be stripped.  Upon return, this
 *						will contain the new length of the line
 * @return	A pointer to the first non whitespace character in the line
 * @return	The start of the line passed in if no changes were made
 */

char *Utils::StripDags(char *a_pcLine, TInt *a_piLength)
{
	char Char, *RetVal;
	TInt Index, Length;

	/* Get a pointer to the start of the line */

	RetVal = a_pcLine;

	/* Iterate through the line until a non whitespace character is found */

	Length = *a_piLength;

	for (Index = 0; Index < Length; ++Index)
	{
		Char = RetVal[Index];

		if ((Char != ' ') && (Char != '\t'))
		{
			break;
		}
	}

	/* If we have found one or more whitespace characters, adjust the pointer to the line */
	/* and the length of the line */

	if (Index > 0)
	{
		RetVal += Index;
		Length -= Index;
	}

	/* Save the new length of the line */

	*a_piLength = Length;

	/* Now search backwards along the line, looking for white space */

	Index = (Length - 1);

	while (Index >= 0)
	{
		/* If the current letter isn't white space then we're done so break out */

		Char = RetVal[Index];

		if ((Char != ' ') && (Char != '\t') && (Char != 0x0d))
		{
			break;
		}

		--Index;
	}

	/* If we have found any whitespace then save the new length */

	if (Index >= 0)
	{
		++Index;
		*a_piLength = Index;
	}

	return(RetVal);
}

/**
 * Converts a numeric string to an integer.
 * Converts a numeric string to an integer and validates that the string to be returned
 * actually contains a valid integer, consisting of only characters between 0 and 9.
 *
 * @date	Tuesday 26-Mar-2013 6:15 am, Code HQ Ehinger Tor
 * @param	a_pccString		Pointer to numeric string to be converted
 * @param	a_piResult		Pointer to an integer into which to place the result
 * @return	KErrNone if the string was converted successfully
 * @return	KErrCorrupt if the string did not contain a valid number
 */

TInt Utils::StringToInt(const char *a_pccString, TInt *a_piResult)
{
	char Char;
	TInt Index, RetVal;

	ASSERTM((a_pccString != NULL), "Utils::StringToInt() => Pointer to string to convert must be passed in");
	ASSERTM((a_piResult != NULL), "Utils::StringToInt() => Pointer to destination integer must be passed in");

	/* First, manually confirm that the string is a valid number, as atoi() does */
	/* not differentiate between '0' and an invalid number - both return 0! */

	Index = 0;

	while ((Char = a_pccString[Index]) != '\0')
	{
		if ((Char < '0') || (Char > '9'))
		{
			break;
		}

		++Index;
	}

	/* If we made it to the end of the string and the string is not empty then we have a valid number */

	if ((Index > 0) && (Char == '\0'))
	{
		RetVal = KErrNone;

		/* Convert the valid number to an integer and return it */

		*a_piResult= atoi(a_pccString);
	}
	else
	{
		RetVal = KErrCorrupt;
	}

	return(RetVal);
}

/**
 * Converts an instance of TDateTime to human readable strings.
 * This function will convert an instance of the TDateTime structure into a human readable
 * format in two strings - one for the date and one for the time.  Unlike the other overloaded
 * function of this name, this one does not honour the current locale settings.  It will always
 * use English words, will insert a day string into the date, will use a UK style date format
 * and will use 12 hour time format.
 *
 * @date	Wednesday 18-Feb-2015 07:08 am, Code HQ Ehinger Tor
 * @param	a_pcDate		Pointer to string into which to place the date string
 * @param	a_pcTime		Pointer to a string into which to place the time string
 * @param	a_roDateTime	Reference to the TDateTime structure to be converted
 */

void Utils::TimeToString(char *a_pcDate, char *a_pcTime, const TDateTime &a_roDateTime)
{
	TInt DayOfWeek, Hour;

	/* Write out the date and time in the same format as Amiga OS using the FORMAT_DOS format */

	DayOfWeek = a_roDateTime.DayOfWeek();

	sprintf(a_pcDate, "%s %02d-%s-%04d", g_apccDays[DayOfWeek], a_roDateTime.Day(), g_apccMonths[a_roDateTime.Month()],
		a_roDateTime.Year());

	/* Do some special processing to take into account that we are printing in 12 hour time format. */
	/* 13:00 becomes 1:00 and 0 am/pm becomes 12 am/pm etc. */

	Hour = (a_roDateTime.Hour() > 12) ? (a_roDateTime.Hour() - 12) : a_roDateTime.Hour();

	if (Hour == 0)
	{
		Hour = 12;
	}

	sprintf(a_pcTime, "%d:%02d %s", Hour, a_roDateTime.Minute(), (a_roDateTime.Hour() < 12) ? "am" : "pm");
}

/**
 * Converts an instance of a TEntry to human readable strings.
 * This function will convert the components of a TEntry structure that pertain to the date and time
 * into a human readable format in two strings - one for the date and one for the time.  Unlike the
 * other overloaded function of this name, this one will honour the current locale settings.  It
 * will also use Amiga OS specific functions to create the string when run under Amiga OS.
 *
 * @date	Saturday 23-Jul-2009 06:58 am
 * @param	a_pcDate		Pointer to string into which to place the date string
 * @param	a_pcTime		Pointer to a string into which to place the time string
 * @param	a_roEntry		Reference to the TEntry structure to be converted
 * @return	ETrue if successful, else EFalse if date stamp passed in was invalid
 */

TBool Utils::TimeToString(char *a_pcDate, char *a_pcTime, const TEntry &a_roEntry)
{
	TBool RetVal;

#ifdef __amigaos__

	struct DateTime DateTime = { { 0, 0, 0 }, FORMAT_DOS, DTF_SUBST, NULL, NULL, NULL };

	/* Convert the entry passed in into date and time strings, taking care to respect */
	/* the current locale */

	DateTime.dat_Stamp = a_roEntry.iPlatformDate;
	DateTime.dat_Format = FORMAT_DOS;
	DateTime.dat_Flags = DTF_SUBST;
	DateTime.dat_StrDay = NULL;
	DateTime.dat_StrTime = a_pcTime;
	DateTime.dat_StrDate = a_pcDate;

	/* Although unlikely, this can fail so we have to check it */

	RetVal = (DateToStr(&DateTime) != 0);

#else /* ! __amigaos__ */

	RetVal = ETrue;

	/* Write out the date and time in the same format as Amiga OS using the FORMAT_DOS format */

	sprintf(a_pcDate, "%02d-%s-%04d", a_roEntry.iModified.DateTime().Day(), g_apccMonths[a_roEntry.iModified.DateTime().Month()],
		a_roEntry.iModified.DateTime().Year());

	sprintf(a_pcTime, "%02d:%02d", a_roEntry.iModified.DateTime().Hour(), a_roEntry.iModified.DateTime().Minute());

#endif /* __amigaos__ */

	return(RetVal);
}

/**
 * Trims white space from either end of a string.
 * Trims white space from the start and end of a string, modifying the string that was passed in.
 *
 * @date	Sunday 01-Aug-2010 1:20 pm
 * @param	a_pcString		Pointer to the string to be trimmed of white space
 */

void Utils::TrimString(char *a_pcString)
{
	char *String, *Dest;
	size_t Length;

	/* Firstly determine if there is any white space at the start of the string that needs to be trimmed */

	String = a_pcString;

	while ((*String) && ((*String == ' ') || (*String == '\t')))
	{
		++String;
	}

	/* If any white space was found then we need copy copy the string over the top of the white space */
	/* to remove it */

	if (String != a_pcString)
	{
		Dest = a_pcString;

		while (*String)
		{
			*Dest++ = *String++;
		}

		*Dest = '\0';
	}

	/* Now trim any white space found at the end of the string.  To start with, ensure there are actually */
	/* characters in the string to be trimmed */

	if ((Length = strlen(a_pcString)) > 0)
	{
		/* Get a pointer to the last character of the string before the NULL terminator */

		String = (a_pcString + Length - 1);

		/* Iterate backwards until a non white space character or the beginning of the string is found */

		while ((String > a_pcString) && ((*String == ' ') || (*String == '\t')))
		{
			--String;
		}

		/* And NULL terminate the string after the non white space character */

		*(String + 1) = '\0';
	}
}

/**
* Extracts the red component of an RGB colour.
*
* @date		Saturday 16-Sep-2006 4:49 pm
* @param	a_ulColour		The RGB value from which to extract
* @return	The red component of the RGB value passed in
*/

ULONG Utils::Red32(unsigned long a_ulColour)
{
	ULONG Red;

	Red = (a_ulColour & 0xff);

	return((Red << 24) | (Red << 16) | (Red << 8) | Red);
}

/**
* Extracts the green component of an RGB colour.
*
* @date		Saturday 16-Sep-2006 4:49 pm
* @param	a_ulColour		The RGB value from which to extract
* @return	The green component of the RGB value passed in
*/

ULONG Utils::Green32(unsigned long a_ulColour)
{
	ULONG Green;

	Green = ((a_ulColour & 0xff00) >> 8);

	return((Green << 24) | (Green << 16) | (Green << 8) | Green);
}

/**
 * Extracts the blue component of an RGB colour.
 *
 * @date	Saturday 16-Sep-2006 4:49 pm
 * @param	a_ulColour		The RGB value from which to extract
 * @return	The blue component of the RGB value passed in
 */

ULONG Utils::Blue32(unsigned long a_ulColour)
{
	ULONG Blue;

	Blue = ((a_ulColour & 0xff0000) >> 16);

	return((Blue << 24) | (Blue << 16) | (Blue << 8) | Blue);
}

/**
 * Converts from a RGB to BGR or vice versa.
 * This function takes the 24 bit RGB value passed in and swaps the red and blue components.  These
 * components are not interpreted in any way, just swapped, so it can be used both to change an RGB
 * value to BGR and vice versa.
 *
 * @date	Wednesday 27-Feb-2013 7:11 am
 * @param	a_ulColour	RGB colour to be reversed
 * @return	The colour passed in with the red & blue components reversed
 */

ULONG Utils::ReverseRGB(unsigned long a_ulColour)
{
	ULONG RetVal;

	RetVal = (a_ulColour & 0xff00);
	RetVal |= ((a_ulColour & 0xff0000) >> 16);
	RetVal |= ((a_ulColour & 0xff) << 16);

	return(RetVal);
}

/**
 * Swaps endianness of a 32 bit word.
 * Reverses the endianness of the word that is passed in.  The word is swapped in place, making it
 * convenient to call this method call from a conditionally defined macro.
 *
 * @date	Saturday 24-Oct-2020 7:04 am, Code HQ Bergmannstrasse
 * @param	value		The 32 bit word to have its bytes swapped
 */

void Utils::swap32(uint32_t *value)
{
	unsigned char temp;
	unsigned char *number = (unsigned char *) value;

	temp = number[0];
	number[0] = number[3];
	number[3] = temp;

	temp = number[1];
	number[1] = number[2];
	number[2] = temp;
}

/**
 * Swaps endianness of a 64 bit word.
 * Reverses the endianness of the word that is passed in.  The word is swapped in place, making it
 * convenient to call this method call from a conditionally defined macro.
 *
 * @date	Sunday 24-Jan-2021 9:16 am, Code HQ Bergmannstrasse
 * @param	value		The 64 bit word to have its bytes swapped
 */

void Utils::swap64(TInt64 *value)
{
	uint32_t temp;
	uint32_t *number = (uint32_t *) value;

	temp = number[0];
	number[0] = number[1];
	number[1] = temp;

	swap32(number);
	swap32(number + 1);
}
