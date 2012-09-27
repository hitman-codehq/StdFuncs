
#include "StdFuncs.h"
#include "BaUtils.h"

#ifdef __linux__

#include <stdio.h>
#include <unistd.h>

#endif /* __linux__ */

#ifdef __amigaos4_

#define DELETE_FILE(FileName) IDOS->DeleteFile(FileName)
#define DELETE_DIRECTORY(DirectoryName) IDOS->DeleteFile(DirectoryName)

#elif defined(__linux__)

#define DELETE_FILE(FileName) (unlink(FileName) == 0)
#define DELETE_DIRECTORY(DirectoryName) rmdir(DirectoryName) == 0)

#else /* ! __linux__ */

#define DELETE_FILE(FileName) ::DeleteFile(FileName)
#define DELETE_DIRECTORY(DirectoryName) RemoveDirectory(DirectoryName)

#endif /* ! __linux__ */

/* Written: Thursday 27-Sep-2012 6:39 am, Code HQ Ehinger Tor */
/* Internal function that takes the OS specific error from the last function call and maps */
/* it onto the appropriate return code for BaflUtils::DeleteFile() */

static TInt MapDeleteError()
{
	TInt RetVal;

#ifdef WIN32

	DWORD Error;

	Error = GetLastError();

	if (Error == ERROR_FILE_NOT_FOUND)
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

#endif /* WIN32 */

	return(RetVal);
}

/* Written: Wednesday 01-Jul-2009 7:54 pm */
/* @param	a_pccFileName	Name of the file or directory to be deleted */
/* @return	KErrNone if successful */
/*			KErrPathNotFound if the path to the file or directory does not exist */
/*			KErrNotFound if the path is ok, but the file or directory does not exist */
/*			KErrInUse if the file or directory is open for use */
/*			KErrGeneral if some other unexpected error occurred */
/* This function will delete a file or directory.  The object in question must */
/* not be open for use in any way and if it is a directory then it must be empty */

TInt BaflUtils::DeleteFile(const char *a_pccFileName)
{
	TInt RetVal;

	/* First assume that the object is a file and try to delete it */

	if (DELETE_FILE(a_pccFileName))
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = MapDeleteError();

		if (RetVal == KErrGeneral)
		{
			/* Otherwise it was some other error so assume the object is a directory and try to delete it */

			if (DELETE_DIRECTORY(a_pccFileName))
			{
				RetVal = KErrNone;
			}
			else
			{
				RetVal = MapDeleteError();
			}
		}
	}

	return(RetVal);
}

/* Written: Monday 19-Apr-2010 6:26 am */

// TODO: CAW - What does this return on Symbian OS?  KErrAlreadyExists?  Create target directories if required
TInt BaflUtils::RenameFile(const char *a_pccOldFullName, const char *a_pccNewFullName)
{
	TInt RetVal;

#ifdef __amigaos4__

	RetVal = (IDOS->Rename(a_pccOldFullName, a_pccNewFullName)) ? KErrNone : KErrGeneral;

#elif defined(__linux__)

	RetVal = (rename(a_pccOldFullName, a_pccNewFullName) == 0) ? KErrNone : KErrGeneral;

#else /* ! __linux__ */

	RetVal = (MoveFile(a_pccOldFullName, a_pccNewFullName)) ? KErrNone : KErrGeneral;

#endif /* ! __linux__ */

	return(RetVal);
}
