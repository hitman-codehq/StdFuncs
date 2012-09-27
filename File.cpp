
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
/* @param	a_pccName		Ptr to the name of the file to be created */
/*			a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian */
/*							API and is ignored (but should be EFileWrite for consistency) */
/* @return	KErrNone if successful */
/*			KErrAlreadyExists if the file already exists */
/*			KErrPathNotFound if the path to the file does not exist */
/*			KErrGeneral if some other unexpected error occurred */
/* Creates a new file that can subsequently be used for writing operations.  If a file already */
/* exists with the same name then the function will fail.  The a_pccName parameter can specify */
/* a path to the file but the path must already exist */

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

	TInt Flags;

	Flags = (O_CREAT | O_EXCL);
	Flags |= (a_uiFileMode & EFileWrite) ? O_RDWR : O_RDONLY;

	if ((m_oHandle = open(a_pccName, Flags, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) != -1)
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
/* @param	a_pccName		Ptr to the name of the file to be created */
/*			a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian */
/*							API and is ignored (but should be EFileWrite for consistency) */
/* @return	KErrNone if successful */
/*			KErrAlreadyExists if the file already exists */
/*			KErrPathNotFound if the path to the file does not exist */
/*			KErrGeneral if some other unexpected error occurred */
/* Creates a new file that can subsequently be used for writing operations.  If a file already */
/* exists with the same name then the function will replace it.  The a_pccName parameter can specify */
/* a path to the file but the path must already exist */

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
/* @param	a_pccName		Ptr to the name of the file to be opened */
/*			a_uiFileMode	Mode in which to open the file */
/* @return	KErrNone if successful */
/*			KErrPathNotFound if the path to the file does not exist */
/*			KErrNotFound if the path is ok, but the file does not exist */
/*			KErrGeneral if some other unexpected error occurred */
/* Opens an existing file that can subsequently be used for reading operations.  The file can be opened */
/* in the file mode EFileRead, EFileWrite, or a logical combination of them both.  If the file mode */
/* EFileWrite is specified then the file will also be writeable.  The a_pccName parameter can */
/* optionally specify a path to the file */

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

	DWORD Error, FileMode;

	FileMode = GENERIC_READ;

	if (a_uiFileMode & EFileWrite)
	{
		FileMode |= GENERIC_WRITE;
	}

	if ((m_oHandle = CreateFile(a_pccName, FileMode, 0, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		Error = GetLastError();

		if (Error == ERROR_PATH_NOT_FOUND)
		{
			RetVal = KErrPathNotFound;
		}
		else if (Error == ERROR_FILE_NOT_FOUND)
		{
			RetVal = KErrNotFound;
		}
		else
		{
			RetVal = KErrGeneral;
		}
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 10:20 pm */
/* @param	a_pcucBuffer	Ptr to the buffer to read the data into */
/*			a_iLength		Number of bytes in the buffer to be read */
/* @return	Number of bytes read, if successful, otherwise KErrGeneral */
/* Reads a number of bytes from the file.  There must be sufficient data in the file to be */
/* able to satisfy the read request, or the function will fail.  It is safe to try and write */
/* 0 bytes.  In this case 0 will be returned */

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

	/* Only try to read if at least one byte is being read.  Windows acts strangely if you */
	/* try to read 0 bytes (it returns ERROR_ACCESS_DENIED) even though it can handle trying */
	/* to write 0 bytes, so we will handle the 0 length ourselves */

	if (a_iLength > 0)
	{
		if (ReadFile(m_oHandle, a_pucBuffer, a_iLength, &BytesRead, NULL))
		{
			RetVal = BytesRead;
		}
		else
		{
			RetVal = KErrGeneral;
		}
	}
	else
	{
		RetVal = 0;
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 10:29 pm */
/* @param	a_pcucBuffer	Ptr to the buffer to be written to the file */
/*			a_iLength		Number of bytes in the buffer to be written */
/* @return	Number of bytes written, if successful, otherwise KErrGeneral */
/* Writes a number of bytes to the file.  The file must have been opened in a writeable mode, */
/* either by using RFile::Open(EFileWrite) or with RFile::Replace() or RFile::Create().  In the */
/* latter two cases, files are always opened as writeable, regardless of the file mode passed in. */
/* It is safe to try and write 0 bytes.  In this case 0 will be returned */

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
/* Closes a file that has been created for reading & writing, or opened for reading. */

void RFile::Close()
{

#ifdef __linux__

	if (m_oHandle != -1)
	{
		DEBUGCHECK(close(m_oHandle) == 0); // TODO: CAW - Assert below as well?
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
