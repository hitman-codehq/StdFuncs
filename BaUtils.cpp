
#include "StdFuncs.h"
#include "BaUtils.h"
#include <string.h>

#ifdef __linux__

#include <stdio.h>
#include <unistd.h>

#endif /* __linux__ */

#ifdef __amigaos4__

#define DELETE_FILE(FileName) IDOS->DeleteFile(FileName)

#elif defined(__linux__)

#define DELETE_FILE(FileName) (unlink(FileName) == 0)

#else /* ! __linux__ */

#define DELETE_FILE(FileName) ::DeleteFile(FileName)

#endif /* ! __linux__ */

/**
 * Deletes a file from the file system.
 * The operation of this function unfortunately varies across operating systems.
 * For most operating systems, the file in question must not be open for use in any
 * way or the function will fail.  However, on UNIX based systems, if the file is
 * open for use at the point at which this function is called, the function will
 * succeed and the file will be flagged for deletion when the client process quits.
 * Thus in this case KErrNone will be returned, even though the file will continue
 * to exist for the duration of the lifetime of the client process.
 * @date	Wednesday 01-Jul-2009 7:54 pm
 * @param	a_pccFileName	Name of the file to be deleted
 * @return	KErrNone if successful
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

TInt BaflUtils::DeleteFile(const char *a_pccFileName)
{
	TInt RetVal;

	/* First try to delete the file */

	if (DELETE_FILE(a_pccFileName))
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_pccFileName);
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
