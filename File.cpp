
#include "StdFuncs.h"
#include "BaUtils.h"
#include "File.h"

#ifdef __linux__

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#endif /* __linux__ */

/* Written: Friday 02-Jan-2009 9:03 pm */

RFile::RFile()
{

#ifdef __linux__

	m_oHandle = -1;

#else /* ! __linux__ */

	m_oHandle = 0;

#endif /* ! __linux__ */

}

/* Written: Friday 02-Jan-2009 8:54 pm */

TInt RFile::Create(const char *a_pccName, TUint a_uiFileMode)
{
	TInt RetVal;

	(void) a_uiFileMode;

#ifdef __amigaos4__

	// TODO: CAW - Map Symbian -> Amiga open modes, here and in Open(). Return correct errors and update test for all ports
	if ((m_oHandle = IDOS->Open(a_pccName, MODE_NEWFILE)) != 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = KErrNotFound;
	}

#elif defined(__linux__)

	if ((m_oHandle = open(a_pccName, (O_CREAT | O_EXCL), (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) != -1)
	{
		RetVal = KErrNone;
	}
	else
	{
		if (errno == EEXIST)
		{
			RetVal = KErrAlreadyExists;
		}
		else if (errno == ENOENT)
		{
			// TODO: CAW - Finish these
			// RetVal = KErrNotFound; // TODO: CAW - WTF?  How does this differ from KErrPathNotFound?
			RetVal = KErrPathNotFound;
		}
		else
		{
			RetVal = KErrGeneral;
		}
	}

#else /* ! __linux__ */

	if ((m_oHandle = CreateFile(a_pccName, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		DWORD Error;

		Error = GetLastError();

		if (Error == ERROR_FILE_EXISTS)
		{
			RetVal = KErrAlreadyExists;
		}
		else if (Error == ERROR_PATH_NOT_FOUND)
		{
			RetVal = KErrPathNotFound;
		}
		else
		{
			RetVal = KErrGeneral;
		}
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Monday 19-Apr-2010 6:26 am */

TInt RFile::Replace(const char *a_pccName, TUint a_uiFileMode)
{
	TInt RetVal;

	/* Try to delete the file specified so that it can be recreated */

	RetVal = BaflUtils::DeleteFile(a_pccName);

	/* If the file was deleted successfully or if it didn't exist, continue on to create a new file */

	if ((RetVal == KErrNone) || (RetVal == KErrNotFound))
	{
		RetVal = Create(a_pccName, a_uiFileMode);
	}

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 8:57 pm */

TInt RFile::Open(const char *a_pccName, TUint a_uiFileMode)
{
	TInt RetVal;

#ifdef __amigaos4__

	(void) a_uiFileMode;

	if ((m_oHandle = IDOS->Open(a_pccName, MODE_OLDFILE)) != 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = KErrNotFound;
	}

#elif defined(__linux__)

	TInt Flags;

	Flags = (a_uiFileMode & EFileWrite) ? O_RDWR : O_RDONLY;

	if ((m_oHandle = open(a_pccName, Flags, 0)) != -1)
	{
		RetVal = KErrNone;
	}
	else
	{
		if (errno == ENOENT)
		{
			// TODO: CAW - Finish these
			RetVal = KErrNotFound; // TODO: CAW - WTF?  How does this differ from KErrPathNotFound?  These all need to be documented
			//RetVal = KErrPathNotFound;
		}
		else
		{
			RetVal = KErrGeneral;
		}
	}

#else /* ! __linux__ */

	DWORD FileMode;

	FileMode = GENERIC_READ;

	if (a_uiFileMode & EFileWrite)
	{
		FileMode |= GENERIC_WRITE;
	}

	if ((m_oHandle = CreateFile(a_pccName, FileMode, (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		// TODO: CAW - Write a test for this + implement for all platforms.  Also test for such things as trying to write to a read only file
		RetVal = (GetLastError() == ERROR_SHARING_VIOLATION) ? KErrInUse : KErrNotFound;
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 10:20 pm */
// TODO: CAW - What about passing in a_iLength of 0?

TInt RFile::Read(unsigned char *a_pucBuffer, TInt a_iLength) const
{
	TInt RetVal;

	ASSERTM(m_oHandle, "RFile::Read() => File is not open");

#ifdef __amigaos4__

	RetVal = IDOS->Read(m_oHandle, a_pucBuffer, a_iLength);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#elif defined(__linux__)

	RetVal = read(m_oHandle, a_pucBuffer, a_iLength);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __linux__ */

	DWORD BytesRead;

	if (ReadFile(m_oHandle, a_pucBuffer, a_iLength, &BytesRead, NULL))
	{
		RetVal = BytesRead;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 10:29 pm */
// TODO: CAW - What happens if 0 bytes are written?  Define behaviour and make a test

TInt RFile::Write(const unsigned char *a_pcucBuffer, TInt a_iLength)
{
	TInt RetVal;

	ASSERTM(m_oHandle, "RFile::Write() => File is not open");

#ifdef __amigaos4__

	RetVal = IDOS->Write(m_oHandle, a_pcucBuffer, a_iLength);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#elif defined(__linux__)

	// TODO: CAW - What if this returns < a_iLength?
	RetVal = write(m_oHandle, a_pcucBuffer, a_iLength);

	if (RetVal != a_iLength)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __linux__ */

	DWORD BytesWritten;

	if ((WriteFile(m_oHandle, a_pcucBuffer, a_iLength, &BytesWritten, NULL)) &&
		((TInt) BytesWritten == a_iLength))
	{
		RetVal = a_iLength;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 8:58 pm */

void RFile::Close()
{

#ifdef __linux__

	if (m_oHandle != -1)
	{
		close(m_oHandle); // TODO: CAW - Assert, here and below?
		m_oHandle = -1;
	}

#else /* ! __linux__ */

	if (m_oHandle != 0)
	{

#ifdef __amigaos4__

		IDOS->Close(m_oHandle);

#else /* ! __amigaos4__ */

		CloseHandle(m_oHandle);

#endif /* ! __amigaos4__ */

		m_oHandle = 0;
	}

#endif /* ! __linux__ */

}
