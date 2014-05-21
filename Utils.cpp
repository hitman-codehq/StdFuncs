
#include "StdFuncs.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <workbench/startup.h>

#elif defined(QT_GUI_LIB)

#include <QtGui/QMessageBox>
#include "Qt/StdWindow.h"

#endif /* QT_GUI_LIB */

#ifdef __linux__

#include <errno.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#endif /* __linux__ */

#include <stdio.h>
#include <string.h>
#include "File.h"

#ifdef __amigaos4__

#define MAX_NAME_FROM_LOCK_LENGTH 1024

#define DELETE_DIRECTORY(DirectoryName) IDOS->Delete(DirectoryName)
#define VSNPRINTF vsnprintf

#elif defined(__linux__)

#define DELETE_DIRECTORY(DirectoryName) (rmdir(DirectoryName) == 0)
#define VSNPRINTF vsnprintf

#else /* ! __linux__ */

#define DELETE_DIRECTORY(DirectoryName) RemoveDirectory(DirectoryName)
#define VSNPRINTF _vsnprintf

#endif /* ! __linux__ */

#define PRINTF printf

#ifndef __amigaos4__

/* List of month names for use by Utils::TimeToString() */

static const char *g_apccMonths[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

#endif /* ! __amigaos4__ */

/* Amiga style suffix that can be prepended to filenames to resolve */
/* them to the directory of the executable that is using the file */

static const char g_accProgDir[] = "PROGDIR:";
#define PROGDIR_LENGTH 8

/* ETrue if running a GUI based program */

TBool g_bUsingGUI;

/* Ptr to root window on which all other windows open.  This was defined in CWindow as a static, */
/* but this caused the GUI framework to be linked in even for command line only programs.  Thus */
/* it is now kept as a global variable.  Less elegent perhaps, but it significantly reduces the */
/* amount of code statically linked in */

CWindow	*g_poRootWindow;

/**
 *
 * Maps the OS's last error onto one of The Framework's standard errors.
 * This function will determine the last error reported by the OS and to map it
 * onto a standard error.
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

#ifdef __amigaos4__

	LONG Result;

	Result = IDOS->IoErr();

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

#elif defined(__linux__)

	if (errno == EEXIST)
	{
		RetVal = KErrAlreadyExists;
	}
	else if (errno == ENOENT)
	{
		RetVal = KErrNotFound;
	}
	else if ((errno == ENOTEMPTY) || (errno == EBUSY))
	{
		RetVal = KErrInUse;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#else /* ! __linux__ */

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

#endif /* ! __linux__ */

	return(RetVal);
}

/**
 * Maps the OS's last error onto one of The Framework's standard errors.
 * This function is an extension to Utils::MapLastError().  It uses that function to
 * determine the last error reported by the OS and to map it onto a standard error,
 * but then performs some extra file related checking on some operating systems to
 * perhaps change the error returned.  This is because different operating systems
 * handle errors slightly differently and thus without this function the results
 * returned by The Framework's file I/O related functions would be different on
 * different platforms.
 *
 * This function should only be used in functions that perform file or directory I/O.
 * All other functions should use Utils::MapLastError() directly to do their error
 * mapping.
 *
 * @date	Monday 09-Oct-2012 5:47 am
 * @param	a_pccFileName		Ptr to the name of the file to be checked
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
	struct TEntry Entry;

	/* Unfortunately UNIX and Amiga OS don't have an error that can be mapped onto KErrPathNotFound */
	/* so we must do a little extra work here.  Amiga OS needs a little help too */

	if (RetVal == KErrNotFound)
	{
		/* Determine the path to the file that was opened */

		NameOffset = (Utils::FilePart(a_pccFileName) - a_pccFileName);

		/* If the file is not in the current directory then check if the path exists */

		if (NameOffset > 0)
		{
			/* Strip off the trailing '/' or '\\' but NOT the ':' as this is required */
			/* for correct RAM disk handling on Amiga OS */

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
				Utils::Info("RFile::MapLastOpenError() => Out of memory");

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
 * is appended as appropriate to the end of the existing path.  This will ensure that a single slash is
 * always present between the end of the existing path and the beginning of the newly added path part.
 * It will not append an extra slash if one exists already.
 *
 * It will also treat Windows style directory names (such as "d:") specially in that they will not have
 * a slash appended to them.  This allows relative addressing of directories on drives where the drive
 * letter by itself represents the current directory of that drive.
 *
 * @date	Thursday 16-Jul-2009 3:58 pm
 * @param	a_pcDest	Ptr to the path to which to append the file or directory name
 * @param	a_pccSource	Ptr to the file or directory name to be appended
 * @param	a_uiDestSize	The size of the destination buffer in bytes
 * @return	ETrue if the file or directory was appended successfully, otherwise EFalse if the destination
 *			buffer was too small to hold the resulting path
 */

TBool Utils::AddPart(char *a_pcDest, const char *a_pccSource, TUint a_uiDestSize)
{
	TBool RetVal;

#ifdef __amigaos4__

	RetVal = IDOS->AddPart(a_pcDest, a_pccSource, a_uiDestSize);

#else /* ! __amigaos4__ */

	char Char;
	TUint Length;

	/* Assume failure */

	RetVal = EFalse;

	/* If there is anything already in the destination string, check there is enough space to */
	/* append to it before doing anything */

	if ((Length = strlen(a_pcDest)) > 0)
	{
		if ((Length + 1 + strlen(a_pccSource)) <= a_uiDestSize)
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
		if (strlen(a_pccSource) <= a_uiDestSize)
		{
			RetVal = ETrue;

			strcpy(a_pcDest, a_pccSource);
		}
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

#ifdef _DEBUG

/* Written: Saturday 11-Jul-2009 08:56 am */

void Utils::AssertionFailure(const char *a_pccMessage, ...)
{
	va_list Args;

	va_start(Args, a_pccMessage);

	if (g_bUsingGUI)
	{
		MessageBox("Assertion Failure", a_pccMessage, EMBTOk, Args);
	}
	else
	{
		PRINTF("Assertion Failure: %s\n", a_pccMessage);
	}

#ifndef __amigaos4__

	/* When an assertion happens we want to exit the system as if we allow execution */
	/* to continue then it is likely to crash anyway.  We will do this on all platforms */
	/* besides Amiga as the non protected Amiga OS isn't so good at handling this! */

	exit(1);

#endif /* ! __amigaos4__ */

	va_end(Args);
}

#endif /* _DEBUG */

/**
 * Parses a string to determine how many tokens it contains.
 * Parses a string, identifying tokens and keeping track of a count of how many it finds.
 * A token is either a single word delimited by white space (spaces or tabs), or multiple
 * words in a quoted string.
 *
 * @date	Friday 04-Jun-2010 7:58 am
 * @param	a_pccBuffer	Ptr to buffer to parse for tokens
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

/* Written: Saturday 18-Jul-2009 8:25 am */

TInt Utils::CreateDirectory(const char *a_pccDirectoryName)
{
	TInt RetVal;

#ifdef __amigaos4__

	BPTR Lock;

	if ((Lock = IDOS->CreateDir(a_pccDirectoryName)) != 0)
	{
		RetVal = KErrNone;

		IDOS->UnLock(Lock);
	}
	else
	{
		RetVal = (IDOS->IoErr() == ERROR_OBJECT_EXISTS) ? KErrAlreadyExists : KErrNotFound;
	}

#elif defined(__linux__)

	if (mkdir(a_pccDirectoryName, 0755) == 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (errno == EEXIST) ? KErrAlreadyExists : KErrNotFound;
	}

#else /* ! __linux__ */

	if (::CreateDirectory(a_pccDirectoryName, NULL))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (GetLastError() == ERROR_ALREADY_EXISTS) ? KErrAlreadyExists : KErrNotFound;
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/**
 * Deletes a directory from the file system.
 * This function will delete a directory.  The directory in question must not be
 * open for use in any way and must be empty.
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

/* Written: Monday 27-Aug-2012 06:56 am */
/* @return	Does not return if successful */
/*			KErrNone if process is already detached from the CLI */
/*			KErrGeneral if unable to detach the process */
/* This function is mainly for Amiga OS and relaunches the current process */
/* asynchronously before exiting the current process.  This gives the user the */
/* impression that the process has automatically detached itself from the CLI. */
/* Note that if successful, this function will not return and will instead shut */
/* the process down using exit().  The second time it is called (by the newly */
/* launched process), it will detect that the new process is already detached from */
/* the CLI and will simply return without doing anything */

TInt Utils::Detach()
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;

#ifdef __amigaos4__

	char *Command, *CommandNameBuffer;
	const char *CommandName;
	TInt ArgStrLength, CommandLength, CommandNameLength, Result;
	BPTR Cos;
	struct CommandLineInterface *CLI;
	struct Process *Process;

	/* If we are started from the CLI and are not already backgrounded (ie. By the "run" */
	/* command or by our own relaunching of ourselves as backgrounded) then relaunch */
	/* the process asynchronously */

	CLI = IDOS->Cli();

	if ((CLI) && (!(CLI->cli_Background ) && (CLI->cli_CommandName)))
	{
		/* Determine the size of the buffers needed for the command name and command line */

		CommandNameLength = IDOS->CopyStringBSTRToC(CLI->cli_CommandName, NULL, 0);
		ArgStrLength = strlen(IDOS->GetArgStr());

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

			IDOS->CopyStringBSTRToC(CLI->cli_CommandName, CommandNameBuffer, (CommandNameLength + 1));
			CommandName = Utils::FilePart(CommandNameBuffer);

			/* Build the string used to relaunch the process */

			Process = (struct Process *) IExec->FindTask(NULL);
			IUtility->SNPrintf(Command, CommandLength,"\"%b\" %s", CLI->cli_CommandName, IDOS->GetArgStr());

			/* We want to use the parent's input stream for both input and output, so duplicate */
			/* it as it will be closed when the child process exits */

			if ((Cos = IDOS->DupFileHandle(Process->pr_CIS)) != 0)
			{
				/* And launch the child process!  If successful then exit as this is essentially */
				/* performing a fork() and we don't want to keep the parent process alive */

				Result = IDOS->SystemTags(Command, SYS_Input, Cos, SYS_Output, NULL, SYS_Error, NULL,
					SYS_Asynch, TRUE, NP_Name, CommandName, TAG_DONE);

				if (Result == 0)
				{
					/* Free the command buffers before we exit */

					delete [] Command;
					delete [] CommandNameBuffer;

					exit(0);
				}
				else
				{
					RetVal = KErrGeneral;

					IDOS->Close(Cos);
				}
			}
		}
		else
		{
			Utils::Info("Utils::Detach() => Out of memory");
		}

		delete [] Command;
		delete [] CommandNameBuffer;
	}

#endif /* __amigaos4__ */

	return(RetVal);
}

/* Written: Wednesday 13-Feb-2013 6:48 am, Code HQ Ehinger Tor */
/* @param	a_pccString	Ptr to the string to be duplicated */
/*			a_iLength	Length of the string, not including NULL terminator, or -1 */
/* @return	Ptr to an allocated buffer containing the duplicated string if successful, */
/*			else NULL */
/* Allocates a buffer large enough to hold the string passed in and its NULL terminator */
/* and copies the string into said buffer.  If a length is passed in (and 0 is a valid */
/* length) then it is assumed the string passed in is not NULL terminated and that length */
/* is used as the string's length.  Otherwise if the length is -1 then the string's length */
/* is determined with strlen().  The string returned by this function should be freed with */
/* delete [] */

char *Utils::DuplicateString(const char *a_pccString, TInt a_iLength)
{
	char *RetVal;

	ASSERTM((a_pccString != NULL), "Utils::DuplicateString() => Ptr to string passed in must not be NULL");

	/* If no length has been passed in then determine the length of the string */

	if (a_iLength == -1)
	{
		a_iLength = strlen(a_pccString);
	}

	/* Allocate a buffer large enough to hold the string and copy the string */

	if ((RetVal = new char[a_iLength + 1]) != NULL)
	{
		memcpy(RetVal, a_pccString, a_iLength);
		RetVal[a_iLength] ='\0';
	}

	return(RetVal);
}

/* Written: Saturday 11-Jul-2009 08:56 am */

void Utils::Error(const char *a_pccMessage, ...)
{
	// TODO: CAW - Risk of overflow, here, above and in Utils::Info
	char Message[512];
	va_list Args;

	va_start(Args, a_pccMessage);

	if (g_bUsingGUI)
	{
		MessageBox("Error", a_pccMessage, EMBTOk, Args);
	}
	else
	{
		VSNPRINTF(Message, sizeof(Message), a_pccMessage, Args);
		PRINTF("Error: %s\n", Message);
	}

	va_end(Args);
}

/* Written: Thursday 22-Jul-2010 8:47 am */

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
 * Finds the file name contained within a fully qualified path.
 * This function will scan backwards through a fully qualified path, looking for any kind of directory
 * separator.  When found, this separator is considered the separator between the path part of a fully
 * qualified file name, and the file name part of the fully qualified file name.  The pointer to this
 * file name will then be returned.  If the path passed in does not contain any path separators (ie. it
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
 * @return	Pointer to the file name component of the path
 */

const char *Utils::FilePart(const char *a_pccPath)
{
	char Char;
	const char *RetVal;

	/* Iterate through the string passed in from its very end, looking for any kind of directory separator, */
	/* whether valid for Amiga OS, UNIX or Windows */

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

	/* If no directory separator was found, we will be pointing to just before the start of the string.  In this */
	/* case, return the entire string passed in */

	if (RetVal < a_pccPath)
	{
		++RetVal;
	}

	return(RetVal);
}

/* Written: Monday 09-Apr-2007 12:06 am */

#ifdef __amigaos4__

TBool Utils::FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory)
{
	char Path[1024]; // TODO: CAW + Comment this function
	TBool RetVal;
	struct ExamineData *ExamineData;

	RetVal = EFalse;

	if (a_poWBArg->wa_Lock)
	{
		if (IDOS->NameFromLock(a_poWBArg->wa_Lock, Path, sizeof(Path)))
		{
			if ((ExamineData = IDOS->ExamineObjectTags(EX_FileLockInput, a_poWBArg->wa_Lock, TAG_DONE)) != NULL)
			{
				strcpy(a_pcFullName, Path);

				if (IDOS->AddPart(a_pcFullName, a_poWBArg->wa_Name, sizeof(Path)))
				{
					RetVal = ETrue;

					*a_pbDirectory = EXD_IS_DIRECTORY(ExamineData);
				}
				else
				{
					Utils::Info("Utils::FullNameFromWBArg() => Unable to build filename");
				}

				IDOS->FreeDosObject(DOS_EXAMINEDATA, ExamineData);
			}
			else
			{
				Utils::Info("Utils::FullNameFromWBArg() => Unable to examine object");
			}
		}
		else
		{
			Utils::Info("Utils::FullNameFromWBArg() => Unable to obtain filename from Lock");
		}
	}

	return(RetVal);
}

#endif /* __amigaos4__ */

/**
 * Obtains information about a given file.
 * This function is useful for obtaining directory listing type information about a single
 * file, without the overhead of having to use the RDir class to do so.  It will query the
 * given filename, which can be either relative or absolute, and will place the information
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
 * @pre		Ptr to file name passed in must not be NULL
 * @pre		Ptr to TEntry structure passed in must not be NULL
 *
 * @date	Saturday 04-Jul-2009 9:20 pm
 * @param	a_pccFileName	Ptr to the name of the file for which to obtain information
 * @param	a_poEntry		Ptr to the TEntry structure into which to place the information
 * @return	KErrNone if the information was obtained successfully
 * @return	KErrNotFound if the file could not be found
 * @return	KErrNotSupported if the specified path ends with a '/' or '\'
 * @return	KErrPathNotFound if an Amiga OS style PROGDIR: prefix could not be resolved
 */

TInt Utils::GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry)
{
	char *ProgDirName;
	TBool PathOk;
	TInt Length, RetVal;

	ASSERTM((a_poEntry != NULL), "Utils::GetFileInfo() => Ptr to file name passed in must not be NULL");
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

#ifdef __amigaos4__

			struct ClockData ClockData;
			struct ExamineData *ExamineData;

			/* Querying empty file names is not allowed by our API, even though the Amiga OS ExamineObject() */
			/* function supports it */

			if (*ProgDirName != '\0')
			{
				if ((ExamineData = IDOS->ExamineObjectTags(EX_StringNameInput, ProgDirName, TAG_DONE)) != NULL)
				{
					RetVal = KErrNone;

					/* Convert the new style date structure into something more usable that also contains */
					/* year, month and day information */

					IUtility->Amiga2Date(IDOS->DateStampToSeconds(&ExamineData->Date), &ClockData);

					/* Convert it so a Symbian style TDateTime structure */

					TDateTime DateTime(ClockData.year, (TMonth) (ClockData.month - 1), (ClockData.mday - 1), ClockData.hour,
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

					IDOS->FreeDosObject(DOS_EXAMINEDATA, ExamineData);
				}
			}

#elif defined(__linux__)

			struct stat Stat;
			struct tm *Tm;

			/* Obtain information about the file and convert its last modification time to the local time */

			if (lstat(ProgDirName, &Stat) == 0)
			{
				if ((Tm = localtime(&Stat.st_mtime)) != NULL)
				{
					RetVal = KErrNone;

					/* Convert the UNIX time information to a TDateTime that the TEntry can use internally */

					TDateTime DateTime((Tm->tm_year + 1900), (TMonth) Tm->tm_mon, Tm->tm_mday, Tm->tm_hour, Tm->tm_min, Tm->tm_sec, 0);

					/* Fill in the file's properties in the TEntry structure */

					a_poEntry->Set(S_ISDIR(Stat.st_mode), S_ISLNK(Stat.st_mode), Stat.st_size, Stat.st_mode, DateTime);
					a_poEntry->iPlatformDate = Stat.st_mtime;

					/* Copy the filename into the TEntry structure */

					strcpy(a_poEntry->iName, FilePart(a_pccFileName));
				}
			}

#else /* ! __linux__ */

			BY_HANDLE_FILE_INFORMATION FileInformation;
			HANDLE FindHandle, Handle;
			SYSTEMTIME SystemTime;
			WIN32_FIND_DATA FindData;

			/* Open the file and determine its properties */

			if ((Handle = CreateFile(ProgDirName, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL)) != INVALID_HANDLE_VALUE)
			{
				if (GetFileInformationByHandle(Handle, &FileInformation))
				{
					/* Convert the file's timestamp to a more useful format that can be put into the TEntry structure */

					if (FileTimeToSystemTime(&FileInformation.ftLastWriteTime, &SystemTime))
					{
						RetVal = KErrNone;

						/* Convert the Windows SYSTEMTIME structure to a TDateTime that the TEntry can use internally */

						TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
							SystemTime.wSecond, 0);

						/* Fill in the file's properties in the TEntry structure */

						a_poEntry->Set((FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0, FileInformation.nFileSizeLow,
							FileInformation.dwFileAttributes, DateTime);
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
								strcpy(a_poEntry->iName, Utils::FilePart(a_pccFileName));
							}
						}
					}
				}

				CloseHandle(Handle);
			}

#endif /* ! __linux__ */

		}
		else
		{
			RetVal = KErrNotSupported;
		}

		/* And free the resolved filename, but only if it contained the prefix */

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

/* Written: Saturday 08-May-2010 3:16 pm */

void Utils::GetScreenSize(TInt *a_piWidth, TInt *a_piHeight)
{

#ifdef __amigaos4__

	struct Screen *Screen;

	if ((Screen = IIntuition->LockPubScreen(NULL)) != NULL)
	{
		*a_piWidth = Screen->Width;
		*a_piHeight = Screen->Height;

		IIntuition->UnlockPubScreen(NULL, Screen);
	}

#elif defined(__linux__)

	// TODO: CAW - Implement this
	if (0)
	{
	}

#else /* ! __linux__ */

	RECT Rect;

	if (GetWindowRect(GetDesktopWindow(), &Rect))
	{
		*a_piWidth = (Rect.right - Rect.left);
		*a_piHeight = (Rect.bottom - Rect.top);
	}

#endif /* ! __linux__ */

	else
	{
		*a_piWidth = 640;
		*a_piHeight = 480;
	}
}

/* Written: Thursday 09-Jul-2009 06:58 am */

TBool Utils::GetShellHeight(TInt *a_piHeight)
{
	TBool RetVal;

	/* Assume failure */

	RetVal = EFalse;

#ifdef __amigaos4__

	char Buffer[32], *BufferPtr, Length, Char;
	TInt Result;
	BPTR InHandle, OutHandle;

	/* Get the standard DOS input and output handles to use for querying the shell dimensions */

	InHandle = IDOS->Input();
	OutHandle = IDOS->Output();

	/* Put the console into RAW mode */

	if (IDOS->SetMode(OutHandle, 1))
	{
		/* Write CSI request for console window dimensions */

		if (IDOS->Write(OutHandle, "\x9b" "0 q", 4) == 4)
		{
			/* Iterate through the response and copy it into a temporary buffer */

			Length = 0;
			BufferPtr = Buffer;

			do
			{
				if ((Result = IDOS->Read(InHandle, BufferPtr, 1)) > 0)
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

			IDOS->SetMode(OutHandle, 0);

			/* If the response was extract successfully, extract the height into the supplied variable */

			if ((Length > 9) && (Buffer[0] == '\x9b'))
			{
				RetVal = ETrue;
				IDOS->StrToLong(&Buffer[5], (LONG *) a_piHeight);
			}
		}
		else
		{
			Utils::Info("Utils::GetShellHeight() => Unable to request window dimensions");
		}
	}
	else
	{
		Utils::Info("Utils::GetShellHeight() => Unable to put console into RAW mode");
	}

#elif defined(__linux__)

	// TODO: CAW - Implement this
	RetVal = ETrue;
	*a_piHeight = 50;

#else /* ! __linux__ */

	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	HANDLE StdOut;

	/* Get the handle of the stdin device and from that obtain the console's height */

	if ((StdOut = GetStdHandle(STD_OUTPUT_HANDLE)) != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo(StdOut, &ScreenBufferInfo))
		{
			/* Signal success */

			RetVal = ETrue;

			/* And save the height of the console for the caller, converting the zero based line */
			/* number of the bottom most line into a count of lines */

			*a_piHeight = (ScreenBufferInfo.srWindow.Bottom + 1);
		}
		else
		{
			Utils::Info("Utils::GetShellHeight() => Unable to request window dimensions");
		}
	}
	else
	{
		Utils::Info("Utils::GetShellHeight() => Unable get handle to console");
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/**
 * Allocates or reallocates a temporary buffer.
 * This function is useful if you have a situation that calls for a temporary
 * buffer of an unknown and varying size (thus preventing the use of a satic
 * buffer) and do not want to dynamically allocate and delete the buffer every
 * time you use it.  If you use this function instead then the buffer will only
 * be reallocated if the new buffer is larger than the old one.  The first
 * time you call this function you should pass in NULL as the buffer ptr.
 * For subsequent calls, pass in the buffer returned by the previous call.
 * When done, use Utils::FreeTempBuffer() to free the allocated buffer.  If a
 * new buffer is allocated, the contents of the old one can be copied into it
 * if desired (this is done by default).
 *
 * @date	Tuesday 28-Feb-2012 8:43 am, Code HQ Ehinger Tor
 * @param	a_pccBuffer		Ptr to the currently allocated buffer
 * @param	a_iSize			Size in bytes of the new buffer to be allocated
 * @param	a_bCopyContents	ETrue to copy the contents of the old buffer.
 *							If not specified then this is ETrue by default
 * @return	A Ptr to the allocated buffer if successful, else NULL
 */

void *Utils::GetTempBuffer(char *a_pccBuffer, TInt a_iSize, TBool a_bCopyContents)
{
	char *OldBuffer, *RetVal;
	TInt Size;

	/* Assume failure */

	OldBuffer = RetVal = NULL;
	Size = 0;

	/* If the buffer has already been allocated then check to see if it is */
	/* large enough to hold the newly requested size */

	if (a_pccBuffer)
	{
		/* The size is stored in the long word just before the ptr that is */
		/* returned to the user */

		Size = *(TInt *) (a_pccBuffer - 4);

		/* If the current buffer is large enough then reuse it */

		if (Size >= a_iSize)
		{
			RetVal = a_pccBuffer;
		}

		/* Otherwise indicate that a new buffer is to be allocated */

		else
		{
			OldBuffer = a_pccBuffer;
			a_pccBuffer = NULL;
		}
	}

	/* If no buffer is allocated then allocate it now */

	if (!(a_pccBuffer))
	{
		// TODO: CAW - How do we ensure this doesn't throw an exception here and for other operating systems?
		RetVal = new char[a_iSize + 4];

		/* Save the size of the buffer in the first long word of the buffer and return */
		/* a ptr to just after that word to the user */

		*(TInt *) RetVal = a_iSize;
		RetVal += 4;

		/* If the buffer was already allocated, copy the old contents into the new buffer */
		/* if requested, and free the old buffer */

		if (OldBuffer)
		{
			if (a_bCopyContents)
			{
				memcpy(RetVal, OldBuffer, Size);
			}

			delete [] (OldBuffer - 4);
		}
	}

	return((void *) RetVal);
}

/* Written: Tuesday 28-Feb-2012 9:00 am, Code HQ Ehinger Tor */
/* @param	a_pccBuffer	Ptr to the buffer to be freed */
/* Frees a buffer allocated with Utils::GetTempBuffer().  It is ok to pass */
/* in NULL to this routine */

void Utils::FreeTempBuffer(char *a_pccBuffer)
{
	if (a_pccBuffer)
	{
		delete [] (a_pccBuffer - 4);
	}
}

#ifdef _DEBUG

void Utils::Info(const char *a_pccMessage, ...)
{
	va_list Args;

	va_start(Args, a_pccMessage);

#ifndef __linux__

	char Message[512];

	strcpy(Message, "Info: " );
	VSNPRINTF(&Message[6], (sizeof(Message) - 6), a_pccMessage, Args);

#endif /* ! __linux__ */

#ifdef __amigaos4__

	IExec->DebugPrintF("%s\n", Message);

#elif defined(__linux__)

	vsyslog(LOG_INFO, a_pccMessage, Args);

#else /* ! __linux__ */

	OutputDebugString(Message);
	OutputDebugString("\n");

#endif /* ! __linux__ */

	va_end(Args);
}

#endif /* _DEBUG */

/**
 * Loads a file into memory in its entirity.
 * This is a convenience function that can be used to load a file into memory.  The file's size will
 * be checked, just the right amount of memory allocated, and the file fill be loaded.  A single byte
 * is allocated than is required and this has a 0 placed into it.  This is to make it easier to parse
 * text files, as it can be depended upon that there is a NULL terminator at the end of the file.
 *
 * The ptr to the allocated buffer is returned to the user in *a_ppucBuffer and it is up to the user
 * to delete [] this memory.  If the function fails to load the file, a_ppucBuffer will be set to NULL.
 *
 * @param	a_pccFileName	Ptr to the name of the file to be opened
 * @param	a_ppucBuffer	Ptr to a variable into which to place the ptr to the allocated buffer
 * @return	The size of the file in bytes, not including the NULL terminator, if successful
 * @return	KErrNoMemory if not enough memory was available
 * @return	Otherwise any of the errors returned by Utils::GetFileInfo(), RFile::Open() or RFile::Read()
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

			if ((RetVal = File.Open(a_pccFileName, EFileRead)) == KErrNone)
			{
				if ((RetVal = File.Read(Buffer, Entry.iSize)) == (TInt) Entry.iSize)
				{
					/* NULL terminate the buffer and save its ptr for the calling client */

					Buffer[Entry.iSize] = '\0';
					*a_ppucBuffer = Buffer;
				}
				else
				{
					Utils::Info("Utils::LoadFile() => Unable to read file \"%s\"", a_pccFileName);
				}
			}
			else
			{
				Utils::Info("Utils::LoadFile() => Unable to open file \"%s\" for loading", a_pccFileName);
			}

			File.Close();

			/* If any error occurred, free the buffer as it won't get passed back to the client */

			if (RetVal < 0)
			{
				delete [] Buffer;
			}
		}
		else
		{
			RetVal = KErrNoMemory;

			Utils::Info("Utils::LoadFile() => Out of memory");
		}
	}

	return(RetVal);
}

/* Written: Wednesday 09-Mar-2011 6:27 am */
/* @param	a_pcPath Ptr to qualified path to be normalised */
/* Iterates through all the characters of a qualified path and converts any instances of the '\' */
/* directory separator charater to '/' to make parsing easier */

void Utils::NormalisePath(char *a_pcPath)
{
	TInt Index, Length;

	/* Iterate through all the characters of the path and pass any instances of '\' to '/' */

	Length = strlen(a_pcPath);

	for (Index = 0; Index < Length; ++Index)
	{
		if (a_pcPath[Index] == '\\')
		{
			a_pcPath[Index] = '/';
		}
	}
}

/* Written: Saturday 12-Mar-2011 11:00 am */
/* @param	a_pccTitle			Ptr to the title to be displayed in the mesage box */
/* @param	a_pccMessage		Ptr to the message to be displayed.  This can contain printf() style %d */
/*								specifiers that will be processed and matched with arguments pass in in the */
/*								a_oArgs varargs list */
/* @param	a_eMessageBoxType	Type of message box to displays, as specified by the TMessageBoxType enumeration */
/* @return	IDOK, IDCANCEL, IDYES or IDNO if successful, or 0 if the message box could not be opened */
/* Opens a message box to prompt the user with a question or simply to display information.  Different types */
/* of message boxes may be opened, which contain combinations of Ok, Cancel, Yes and No buttons according to */
/* the a_eMessageBoxType passed in */

TInt Utils::MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, va_list a_oArgs)
{
	char Message[512];
	TInt RetVal;

	/* Format the message for use no matter how it is displayed */

	VSNPRINTF(Message, sizeof(Message), a_pccMessage, a_oArgs);

#if !defined(__linux__) || defined(QT_GUI_LIB)

	CWindow *RootWindow;

	/* See if a root window has been set by the application and if so, open the message box on */
	/* that window.  Otherwise just open it on the desktop or Workbench */

	RootWindow = CWindow::RootWindow();

#endif /* !defined(__linux__) || defined(QT_GUI_LIB) */

#ifdef __amigaos4__

	TInt Result;
	struct EasyStruct EasyStruct;

	struct TagItem Tags[] = { { ESA_Underscore, '_' }, { TAG_DONE, 0 } };

	/* Assume failure */

	RetVal = IDCANCEL;

	/* Build an EasyStruct for displaying the requester */

	EasyStruct.es_StructSize = sizeof(struct EasyStruct);
	EasyStruct.es_Flags = ESF_TAGGED;
	EasyStruct.es_Title = (char *) a_pccTitle;
	EasyStruct.es_TextFormat = Message;
	EasyStruct.es_TagList = Tags;

	/* Determine the type of requester to display, based on the type passed in */

	if (a_eMessageBoxType == EMBTOk)
	{
		EasyStruct.es_GadgetFormat = "_Ok";
	}
	else if (a_eMessageBoxType == EMBTOkCancel)
	{
		EasyStruct.es_GadgetFormat = "_Ok|_Cancel";
	}
	else if (a_eMessageBoxType == EMBTYesNo)
	{
		EasyStruct.es_GadgetFormat = "_Yes|_No";
	}
	else
	{
		EasyStruct.es_GadgetFormat = "_Yes|_No|_Cancel";
	}

	/* Display the requester */

	Result = IIntuition->EasyRequest((RootWindow) ? RootWindow->m_poWindow : NULL, &EasyStruct, NULL);

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

/* Written: Saturday 12-Mar-2011 6:25 pm */

TInt Utils::MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, ...)
{
	TBool RetVal;
	va_list Args;

	va_start(Args, a_eMessageBoxType);

	RetVal = MessageBox(a_pccTitle, a_pccMessage, a_eMessageBoxType, Args);

	va_end(Args);

	return(RetVal);
}

/**
 * Converts a filename to a fully qualified filename.
 * Takes a filename that contains either no path or a relative path and
 * converts it to a fully qualified filename.  The function will attempt
 * to determine the true path to the filename, which will only work if the
 * file actually exists.  If this is not the case then the current directory
 * will be used instead.  In the event that even this cannot be obtained,
 * the function will fail.
 *
 * The use of the current directory is useful when creating new files, but
 * care must be taken if using this function with software that changes the
 * current directory, especially if the changes could be done in a separate
 * thread.
 *
 * It is the responsibility of the caller to free the returned buffer with
 * delete [].  It is safe to pass in an already qualified filename - the
 * filename will simply be copied into an allocated buffer and returned.
 *
 * Despite its name, this function works for both directories and files.
 * For directories it will "clean" the names returned;  that is, it will
 * ensure that there is no trailing \ or / character at the end of the
 * directory name (unless it is the root directory) as this can cause
 * confusion with calling software if it tries to examine the returned
 * path to determine its type.
 *
 * @date	Thursday 01-Nov-2012 5:24 am, Code HQ Ehinger Tor
 * @param	a_pccFileName	Ptr to filename to be resolved
 * @return	Ptr to the fully qualified name of the filename passed in,
 *			if successful, else NULL
 */

char *Utils::ResolveFileName(const char *a_pccFileName)
{
	char *RetVal;

#ifdef __amigaos4__

	TBool LockedFile;
	BPTR Lock;

	/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
	/* by dos.library's autodocs */

	if ((RetVal = new char[MAX_NAME_FROM_LOCK_LENGTH]) != NULL)
	{
		/* Get a lock on the specified filename and convert it to a qualified filename */

		if ((Lock = IDOS->Lock(a_pccFileName, SHARED_LOCK)) != 0)
		{
			LockedFile = ETrue;
		}
		else
		{
			/* Obtaining the lock failed, probably due to the file not existing, so get */
			/* a lock on the current directory instead */

			Utils::Info("Utils::ResolveFileName() => Unable to obtain lock on \"%s\"", a_pccFileName);

			LockedFile = EFalse;
			Lock = IDOS->GetCurrentDir();
		}

		/* Get the fully qualified name of the file (if the lock was obtained successfully) */
		/* or the path of the current directory (if it wasn't) */

		if (IDOS->NameFromLock(Lock, RetVal, MAX_NAME_FROM_LOCK_LENGTH) == 0)
		{
			Utils::Info("Utils::ResolveFileName() => Unable to determine qualified filename for \"%s\"", a_pccFileName);

			delete [] RetVal;
			RetVal = NULL;
		}

		/* If we obtained the directory name then append the filename to it */

		if (!(LockedFile))
		{
			Utils::AddPart(RetVal, a_pccFileName, MAX_NAME_FROM_LOCK_LENGTH);
		}

		/* And unlock the lock on the file, if it was obtained successfully.  If it is a */
		/* directory lock then it is just a copy of the lock, so don't unlock it */

		if (LockedFile)
		{
			IDOS->UnLock(Lock);
		}
	}
	else
	{
		Utils::Info("Utils::ResolveFileName() => Out of memory");
	}

#elif defined(__linux__)

	/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
	/* by the realpath() manpage */

	if ((RetVal = new char[PATH_MAX]) != NULL)
	{
		/* Convert the filename into a fully qualified path and put it in the allocated buffer */

		if (!(realpath(a_pccFileName, RetVal)))
		{
			/* Converting the filename failed, probably due to the file not existing, so get the current */
			/* directory instead and append the filename to it */

			if (getcwd(RetVal, PATH_MAX))
			{
				Utils::AddPart(RetVal, a_pccFileName, PATH_MAX);
			}

			/* We couldn't even get the current directory so free the buffer and return failure */

			else
			{
				delete [] RetVal;
				RetVal = NULL;
			}
		}
	}
	else
	{
		Utils::Info("Utils::ResolveFileName() => Out of memory");
	}

#else /* ! __linux__ */

	TBool RemoveSlash;
	DWORD Length;

	/* Assume failure */

	RetVal = NULL;

	/* Determine now much memory is required to hold the fully qualified */
	/* path and allocate a buffer of the required size */

	if ((Length = GetFullPathName(a_pccFileName, 0, NULL, NULL)) > 0)
	{
		if ((RetVal = new char[Length]) != NULL)
		{
			/* Convert the filename into a fully qualified path and put it in */
			/* the allocated buffer */

			if ((Length = GetFullPathName(a_pccFileName, Length, RetVal, NULL)) > 0)
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
			}
			else
			{
				Utils::Info("Utils::ResolveFileName() => Unable to determine qualified filename");

				delete [] RetVal;
				RetVal = NULL;
			}
		}
		else
		{
			Utils::Info("Utils::ResolveFileName() => Out of memory");
		}
	}
	else
	{
		Utils::Info("Utils::ResolveFileName() => Unable to determine length of qualified filename");
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Monday 17-Dec-2012 6:24 am, John & Sally's House */
/* @param	a_pccFileName	Ptr to filename to be resolved */
/* @return	Ptr to the fully qualified name of the filename passed in, */
/*			if the filename passed in was suffixed with "PROGDIR:" */
/*			Otherwise ptr to a_pccFileName if no suffix was present */
/*			Otherwise NULL if the conversion of the path failed */
/* Takes a filename that may or may not contain the Amiga style */
/* "PROGDIR:" at the start and, if this prefix is found, returns a ptr */
/* to a fully qualified path that can be used for opening the file. */
/* It is the responsibility of the client to free the buffer containing */
/* this path.  If the filename passed in does not contain this prefix, */
/* a ptr to the filename passed passed in is returned */

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

#ifdef __amigaos4__

		BPTR Lock;

		/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
		/* by dos.library's autodocs */

		if ((RetVal = new char[MAX_NAME_FROM_LOCK_LENGTH]) != NULL)
		{
			/* Get a lock on the specified filename and convert it to a qualified filename */

			if ((Lock = IDOS->GetProgramDir()) != 0)
			{
				if (IDOS->NameFromLock(Lock, RetVal, MAX_NAME_FROM_LOCK_LENGTH ) != 0)
				{
					/* Append the name of the file to be opened in the executable's directory */

					Ok = Utils::AddPart(RetVal, &a_pccFileName[8], MAX_NAME_FROM_LOCK_LENGTH);
				}
				else
				{
					Utils::Info("Utils::ResolveProgDirName() => Unable to determine qualified filename for \"%s\"", a_pccFileName);
				}
			}
			else
			{
				Utils::Info("Utils::ResolveProgDirName() => Unable to obtain lock program directory");
			}
		}
		else
		{
			Utils::Info("Utils::ResolveProgDirName() => Out of memory");
		}

#elif defined(__linux__)

		TInt Result;
		char *FileNamePart;

		/* Allocate a buffer large enough to hold the fully qualified filename, as specified */
		/* by the realpath() manpage */

		if ((RetVal = new char[PATH_MAX]) != NULL)
		{
			/* Get the path to the executable running this code.  Unfortunately the */
			/* API doesn't allow us to query for the length of the path like some other */
			/* APIs so we have to use a fixed size buffer and hope for the best */

			if ((Result = readlink("/proc/self/exe", RetVal, PATH_MAX)) != -1)
			{
				RetVal[Result] = '\0';

				/* Get a ptr to the name of the executable and remove it */

				FileNamePart = (char *) Utils::FilePart(RetVal);
				*FileNamePart = '\0';

				/* Append the name of the file to be opened in the executable's directory */

				Ok = Utils::AddPart(FileNamePart, &a_pccFileName[8], PATH_MAX);
			}

			/* The path could not be obtained so display a debug string and just let the call fail */

			else
			{
				Utils::Info("Utils::ResolveProgDirName() => Cannot obtain path to executable");
			}
		}
		else
		{
			Utils::Info("Utils::ResolveFileName() => Out of memory");
		}

#else /* ! __linux__ */

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
				/* Get a ptr to the name of the executable and remove it */

				FileNamePart = (char *) Utils::FilePart(RetVal);
				*FileNamePart = '\0';

				/* Append the name of the file to be opened in the executable's directory */

				Ok = Utils::AddPart(FileNamePart, &a_pccFileName[8], MAX_PATH);
			}

			/* The path could not be obtained so display a debug string and just let the call fail */

			else
			{
				Utils::Info("Utils::ResolveProgDirName() => Cannot obtain path to executable");
			}
		}
		else
		{
			Utils::Info("Utils::ResolveProgDirName() => Out of memory");
		}

#endif /* ! __linux__ */

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

#ifdef __amigaos4__

// TODO: CAW - Convert this to use RDir?  Why is it OS4 only?  Comments and error messages, ExamineDir() result
//             ObtainDirContextTags() are not all required
TBool Utils::ScanDirectory(const char *a_pccDirectoryName, TBool a_bScanFiles, ScanFunc a_pfScanFunc, void *a_pvUserData)
{
	TBool Error, RetVal;
	APTR Context;
	struct ExamineData *ExamineData;

	/* Assume failure */

	Error = RetVal = EFalse;

	/* Allocate some data structures into which the information about the directory entries may be placed, */
	/* and a control structure for scanning through the directory */

	Context = IDOS->ObtainDirContextTags(EX_StringNameInput, a_pccDirectoryName,
		EX_DataFields, (EXF_DATE | EXF_PROTECTION | EXF_NAME | EXF_SIZE | EXF_TYPE), TAG_DONE);

	if (Context)
	{
		RetVal = ETrue;

		while ((ExamineData = IDOS->ExamineDir(Context)) != NULL)
		{
			// TODO: CAW - There was a bug with this. What about directory links?
			if ((EXD_IS_FILE(ExamineData)) || (EXD_IS_LINK(ExamineData)))
			{
				if (a_bScanFiles)
				{
					if (!(a_pfScanFunc((const char *) ExamineData->Name, a_pvUserData)))
					{
						Error = ETrue;

						break;
					}
				}
			}
			else if (EXD_IS_DIRECTORY(ExamineData))
			{
				if (!(a_bScanFiles))
				{
					if (!(a_pfScanFunc((const char *) ExamineData->Name, a_pvUserData)))
					{
						Error = ETrue;

						break;
					}
				}
			}
		}

		IDOS->ReleaseDirContext(Context);
	}
	else
	{
		Utils::Info("Utils::ScanDirectory() => Unable to create context to examine object");
	}

	return(RetVal);
}

#endif /* __amigaos4__ */

/* Written: Thursday 16-Aug-2012 12:55 pm */
/* @param	a_pccFileName	Ptr to the name of the file to be made deleteable */
/* @return	KErrNone if successful */
/*			KErrNotFound if the file could not be found */
/*			KErrGeneral if some other unspecified error occurred */
/* Makes the file specified by the a_pccFileName parameter deleteable.  This is done in a slightly */
/* different manner on different platfoms.  On Amiga OS it clears the 'd' bit.  On UNIX it sets */
/* the current user's 'w' bit and on Windows it clears the read only, system and hidden bits by */
/* setting the file type to normal, thus clearing all bits */

TInt Utils::SetDeleteable(const char *a_pccFileName)
{
	TInt RetVal;
	TEntry Entry;

#if defined(__amigaos4__) || defined(__linux__)

	/* Get the current file attributes as the chmod() function only lets us set all attributes, */
	/* not just a single one */

	// TODO: CAW - Extra errors?
	if ((RetVal = Utils::GetFileInfo(a_pccFileName, &Entry)) == KErrNone)
	{
		/* Now make the file writeable so that we can delete it */

#ifdef __amigaos4__

		Entry.iAttributes &= ~EXDF_NO_DELETE;

#else /* ! __amigaos4__ */

		Entry.iAttributes |= S_IWUSR;

#endif /* ! __amigaos4__ */

		RetVal = Utils::SetProtection(a_pccFileName, Entry.iAttributes);
	}

#else /* ! defined(__amigaos4__) || defined(__linux__) */

	/* Use our version of SetProtection() rather than the native SetFileAttributes() function */
	/* so that we don't have to worry about error handling */

	RetVal = Utils::SetProtection(a_pccFileName, FILE_ATTRIBUTE_NORMAL);

#endif /* ! defined(__amigaos4__) || defined(__linux__) */

	return(RetVal);

}

/* Written: Saturday 18-Jul-2009 8:06 am */

TInt Utils::SetFileDate(const char *a_pccFileName, const TEntry &a_roEntry)
{
	TInt RetVal;

#ifdef __amigaos4__

	if (IDOS->SetDate(a_pccFileName, &a_roEntry.iPlatformDate) != 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (IDOS->IoErr() == ERROR_OBJECT_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#elif defined(__linux__)

	struct utimbuf Time;

	/* Set both the access and modification time of the file to the time passed in */

	Time.actime = Time.modtime = a_roEntry.iPlatformDate;

	if (utime(a_pccFileName, &Time) == 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (errno == ENOENT) ? KErrNotFound : KErrGeneral;
	}

#else /* ! __linux__ */

	HANDLE Handle;

	/* Open a handle to the file and set its datestamp to that passed in */

	if ((Handle = CreateFile(a_pccFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL)) != INVALID_HANDLE_VALUE)
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

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Saturday 18-Jul-2009 7:59 am */
/* @param	a_pccFileName	Ptr to the name of the file for which to set the protection bits */
/*			a_uiAttributes	The set of protection bits to apply to the file */
/* @return	KErrNone if the protection bits were set successfully */
/*			KErrNotFound if the file could not be found */
/*			KErrGeneral if some other unspecified error occurred */
/* Assigns a set of protection bits to the specified file.  Note that the protection bits are */
/* native to the platform on which the function is being called!  This parameter is of the same */
/* format as that found in the TEntry::iAttributes member variable and this can be passed unchanged */
/* into this function if possible */

TInt Utils::SetProtection(const char *a_pccFileName, TUint a_uiAttributes)
{
	TInt RetVal;

#ifdef __amigaos4__

	if (IDOS->SetProtection(a_pccFileName, a_uiAttributes))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (IDOS->IoErr() == ERROR_OBJECT_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#elif defined(__linux__)

	if (chmod(a_pccFileName, a_uiAttributes) == 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (errno == ENOENT) ? KErrNotFound : KErrGeneral;
	}

#else /* ! __linux__ */

	if (SetFileAttributes(a_pccFileName, a_uiAttributes))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (GetLastError() == ERROR_FILE_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/**
 * Strips white space from the start and end of the line.
 * This function will strip spaces and tabs from the start and end of the
 * line, as well as the carriage return (0x0d) from the end.  It returns a
 * ptr to the first non whitespace character in the line and the length of
 * the "new" line, that has the white space stripped.
 *
 * @date	Sunday 14-Jul-2013 8:36 am, Code HQ Ehinger Tor
 * @param	a_pcLine	Ptr to the line to have its white space stripped
 * @param	a_piLength	Number of characters in the line to be stripped.
 *						Upon return, this will contain the new length of
 *						the line
 * @return	A ptr to the first non whitespace character in the line, or
 *			the start of the line passed in if no changes were made
 */

char *Utils::StripDags(char *a_pcLine, TInt *a_piLength)
{
	char Char, *RetVal;
	TInt Index, Length;

	/* Get a ptr to the start of the line */

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

	/* If we have found one or more whitespace characters, adjust the ptr to the line */
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
 * @param	a_pccString		Ptr to numeric string to be converted
 * @param	a_piResult		Ptr to an integer into which to place the result
 * @return	KErrNone if the string was converted successfully
 * @return	KErrCorrupt if the string did not contain a valid number
 */

TInt Utils::StringToInt(const char *a_pccString, TInt *a_piResult)
{
	char Char;
	TInt Index, RetVal;

	ASSERTM((a_pccString != NULL), "Utils::StringToInt() => Ptr to string to convert must be passed in");
	ASSERTM((a_piResult != NULL), "Utils::StringToInt() => Ptr to destination integer must be passed in");

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

/* Written: Saturday 23-Jul-2009 06:58 am */

TBool Utils::TimeToString(char *a_pcDate, char *a_pcTime, const TEntry &a_roEntry)
{
	TBool RetVal;

#ifdef __amigaos4__

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

	RetVal = (IDOS->DateToStr(&DateTime) != 0);

#else /* ! __amigaos4__ */

	RetVal = ETrue;

	/* Write out the date and time in the same format as Amiga OS using the FORMAT_DOS format */

	sprintf(a_pcDate, "%02d-%s-%04d", a_roEntry.iModified.DateTime().Day(), g_apccMonths[a_roEntry.iModified.DateTime().Month()],
		a_roEntry.iModified.DateTime().Year());

	sprintf(a_pcTime, "%02d:%02d", a_roEntry.iModified.DateTime().Hour(), a_roEntry.iModified.DateTime().Minute());

#endif /* __amigaos4__ */

	return(RetVal);
}

/* Written: Sunday 01-Aug-2010 1:20 pm */
/* @param	a_pcString Ptr to the string to be trimmed of white space */
/* Trims white space from the start and end of a string, modifying the string that was passed in */

void Utils::TrimString(char *a_pcString)
{
	char *String, *Dest;
	TInt Length;

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
		/* Get a ptr to the last character of the string before the NULL terminator */

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

/* Written: Saturday 16-Sep-2006 4:49 pm */

ULONG Utils::Red32(unsigned long a_ulColour)
{
	ULONG Red;

	Red = (a_ulColour & 0xff);

	return((Red << 24) | (Red << 16) | (Red << 8) | Red);
}

/* Written: Saturday 16-Sep-2006 4:49 pm */

ULONG Utils::Green32(unsigned long a_ulColour)
{
	ULONG Green;

	Green = ((a_ulColour & 0xff00) >> 8);

	return((Green << 24) | (Green << 16) | (Green << 8) | Green);
}

/* Written: Saturday 16-Sep-2006 4:49 pm */

ULONG Utils::Blue32(unsigned long a_ulColour)
{
	ULONG Blue;

	Blue = ((a_ulColour & 0xff0000) >> 16);

	return((Blue << 24) | (Blue << 16) | (Blue << 8) | Blue);
}

/* Written: Wednesday 27-Feb-2013 7:11 am */
/* @param	a_ulColour	RGB colour to be reversed */
/* @return	The colour passed in with the red & blue components reversed */
/* This function takes the 24 bit RGB value passed in and swaps the red and blue components. */
/* These components are not interpreted in any way, just swapped, so it can be used both to */
/* change an RGB value to BGR and vice versa */

ULONG Utils::ReverseRGB(unsigned long a_ulColour)
{
	ULONG RetVal;

	RetVal = (a_ulColour & 0xff00);
	RetVal |= ((a_ulColour & 0xff0000) >> 16);
	RetVal |= ((a_ulColour & 0xff) << 16);

	return(RetVal);
}
