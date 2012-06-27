
#include "StdFuncs.h"
#include "BaUtils.h"

#ifdef __linux__

#include <stdio.h>
#include <unistd.h>

#endif /* __linux__ */

/* Written: Wednesday 01-Jul-2009 7:54 pm */

TInt BaflUtils::DeleteFile(const char *a_pccFileName)
{

#ifdef __amigaos4__

	return(IDOS->DeleteFile(a_pccFileName) ? KErrNone : KErrNotFound);

#elif defined(__linux__)

	return((unlink(a_pccFileName) == 0) ? KErrNone : KErrNotFound);

#else /* ! __linux__ */

	TInt RetVal;

	RetVal = KErrNotFound;

	// TODO: CAW - Check Symbian OS return code for this, here and above. Also properly check Windows return codes
	/*Result = GetLastError();

	if (Result == ERROR_FILE_NOT_FOUND)
	{*/
	if (::DeleteFile(a_pccFileName))
	{
		RetVal = KErrNone;
	}
	else if (RemoveDirectory(a_pccFileName))
	{
		RetVal = KErrNone;
	}

	return(RetVal);

#endif /* ! __linux__ */

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
