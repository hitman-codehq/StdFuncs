
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

/* Written: Wednesday 01-Jul-2009 7:54 pm */
/* @param	a_pccFileName	Name of the file to be deleted */
/* @return	KErrNone if successful */
/*			KErrPathNotFound if the path to the file does not exist */
/*			KErrNotFound if the path is ok, but the file does not exist */
/*			KErrInUse if the file is open for use */
/*			KErrGeneral if some other unexpected error occurred */
/* This function will delete a file.  The file in question must not be */
/* open for use in any way */

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

		RetVal = Utils::MapLastError();

#ifndef WIN32

		char Name[MAX_PATH];
		struct TEntry Entry;
		TInt NameOffset;

		/* Unfortunately UNIX doesn't have an error that can be mapped onto KErrPathNotFound so we */
		/* must do a little extra work to determine this */

		if (RetVal == KErrNotFound)
		{
			// TODO: CAW - Variables + create a Utils::PathPart().  The test passes but only out of luck
			NameOffset = (Utils::FilePart(a_pccFileName) - a_pccFileName);

			if (NameOffset > 0)
			{
				memcpy(Name, a_pccFileName, NameOffset);
				Name[--NameOffset] = '\0';

				if (Utils::GetFileInfo(Name, &Entry) == KErrNotFound)
				{
					RetVal = KErrPathNotFound;
				}
			}
		}

#endif /* __linux__ */

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
