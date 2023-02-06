
#include "StdFuncs.h"
#include "FileUtils.h"
#include <string.h>

#if defined(__unix__)

#include <stdio.h>
#include <unistd.h>

#endif /* __unix__ */

#ifdef __amigaos__

#ifdef __amigaos4__

#define DELETE_FILE(FileName) Delete(FileName)

#else /* ! __amigaos4__ */

#define DELETE_FILE(FileName) DeleteFile(FileName)

#endif /* ! __amigaos4__ */

#elif defined(__unix__)

#define DELETE_FILE(FileName) (unlink(FileName) == 0)

#else /* ! __unix__ */

#define DELETE_FILE(FileName) DeleteFile(FileName)

#endif /* ! __unix__ */

/**
 * Deletes a file from the file system.
 * The operation of this function unfortunately varies across operating systems.
 * For most operating systems, the file in question must not be open for use in any
 * way or the function will fail.  However, on UNIX based systems, if the file is
 * open for use at the point at which this function is called, the function will
 * succeed and the file will be flagged for deletion when the client process quits.
 * Thus in this case KErrNone will be returned, even though the file will continue
 * to exist for the duration of the lifetime of the client process.
 *
 * @date	Wednesday 01-Jul-2009 7:54 pm
 * @param	a_fileName	Name of the file to be deleted
 * @return	KErrNone if successful
 * @return	KErrInUse if the file or directory is in use
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the path is ok, but the file does not exist
 * @return	KErrPathNotFound if the path to the file does not exist
 * @return	KErrGeneral if some other unexpected error occurred
 */

int RFileUtils::deleteFile(const char *a_fileName)
{
	int RetVal;

	/* First try to delete the file */

	if (DELETE_FILE(a_fileName))
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_fileName);
	}

	return(RetVal);
}

/**
 * Obtains information about a given local file or directory.
 * Wrapper method that enables local operation of the @link Utils::GetFileInfo() @endlink method.
 * See @link Utils::GetFileInfo() @endlink for more detailed information.
 *
 * @pre		Pointer to filename passed in must not be NULL
 * @pre		Pointer to TEntry structure passed in must not be NULL
 *
 * @date	Saturday 25-Feb-2023 6:06 am, Code HQ Tokyo Tsukuda
 * @param	a_fileName	Pointer to the name of the file for which to obtain information
 * @param	a_entry		Pointer to the TEntry structure into which to place the information
 * @return	KErrNone if successful, otherwise one of the system errors
 */

int RFileUtils::getFileInfo(const char *a_fileName, TEntry *a_entry)
{
	ASSERTM((a_entry != NULL), "RFileUtils::getFileInfo() => Pointer to filename passed in must not be NULL");
	ASSERTM((a_entry != NULL), "RFileUtils::getFileInfo() => TEntry structure passed in must not be NULL");

	return Utils::GetFileInfo(a_fileName, a_entry);
}

/**
 * Renames a file.
 * Renames the specified file to the new name passed in.
 *
 * @date	Monday 19-Apr-2010 6:26 am
 * @param	a_oldFullName	The current name of the file to be renamed
 * @param	a_newFullName	The new name to which to rename the file
 * @return	KErrNone if successful, else KErrGeneral
 */

int RFileUtils::renameFile(const char *a_oldFullName, const char *a_newFullName)
{

#ifdef __amigaos__

	return (Rename(a_oldFullName, a_newFullName)) ? KErrNone : KErrGeneral;

#elif defined(__unix__)

	return (rename(a_oldFullName, a_newFullName) == 0) ? KErrNone : KErrGeneral;

#else /* ! __unix__ */

	return (MoveFile(a_oldFullName, a_newFullName)) ? KErrNone : KErrGeneral;

#endif /* ! __unix__ */

}
