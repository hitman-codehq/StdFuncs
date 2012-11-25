
#include "StdFuncs.h"
#include "BaUtils.h"
#include "File.h"
#include <string.h>

#ifdef __linux__

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#endif /* __linux__ */

/* Written: Friday 02-Jan-2009 9:03 pm */

RFile::RFile()
{

#ifdef __linux__

	m_oHandle = -1;

#elif defined(__amigaos4__)

	m_uiFileMode = m_oHandle = 0;

#else /* ! __amigaos4__ */

	m_oHandle = INVALID_HANDLE_VALUE;

#endif /* ! __amigaos4__ */

}

/* Written: Friday 02-Jan-2009 8:54 pm */
/* @param	a_pccFileName	Ptr to the name of the file to be created */
/*			a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian */
/*							API and is ignored (but should be EFileWrite for consistency) */
/* @return	KErrNone if successful */
/*			KErrAlreadyExists if the file already exists */
/*			KErrPathNotFound if the path to the file does not exist */
/*			KErrNotEnoughMemory if not enough memory was available */
/*			KErrGeneral if some other unexpected error occurred */
/* Creates a new file that can subsequently be used for writing operations.  If a file already */
/* exists with the same name then the function will fail.  The a_pccFileName parameter can specify */
/* a path to the file but the path must already exist */

TInt RFile::Create(const char *a_pccFileName, TUint a_uiFileMode)
{
	TInt RetVal;

	(void) a_uiFileMode;

#ifdef __amigaos4__

	TEntry Entry;

	/* Opening wildcards are not supported by our API although Amiga OS allows it! */

	if ((strstr(a_pccFileName, "#") == NULL) && (strstr(a_pccFileName, "?") == NULL) && (strstr(a_pccFileName, "*") == NULL))
	{
		/* Only create the file if it does not already exist.  Amiga OS does not */
		/* have a mode that allows us to do this so we have to manually perform a */
		/* check ourselves */

		if (Utils::GetFileInfo(a_pccFileName, &Entry) == KErrNotFound)
		{
			if ((m_oHandle = IDOS->Open(a_pccFileName, MODE_NEWFILE)) != 0)
			{
				RetVal = KErrNone;

				m_uiFileMode = EFileWrite;
			}
			else
			{
				/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

				RetVal = Utils::MapLastFileError(a_pccFileName);
			}
		}
		else
		{
			RetVal = KErrAlreadyExists;
		}
	}
	else
	{
		RetVal = KErrNotFound;
	}

#elif defined(__linux__)

	TInt Flags;

	Flags = (O_CREAT | O_EXCL | O_RDWR);

	/* Create a new file in read/write mode */

	if ((m_oHandle = open(a_pccFileName, Flags, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))) != -1)
	{
		/* Now lock the file so that it cannot be re-opened.  The RFile API does not support having */
		/* multiple locks on individual files */

		if (flock(m_oHandle, (LOCK_EX | LOCK_NB)) == 0)
		{
			RetVal = KErrNone;
		}
		else
		{
			Utils::Info("RFile::Create() => Unable to lock file for exclusive access");

			RetVal = KErrGeneral;

			Close();
		}
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_pccFileName);
	}

#else /* ! __linux__ */

	TInt Result;

	/* Create a new file in read/write mode */

	if ((m_oHandle = CreateFile(a_pccFileName, GENERIC_WRITE, 0, NULL, CREATE_NEW, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_pccFileName);

		/* For Windows we need some special handling as trying to open a file for */
		/* writing when it already exists will return ERROR_FILE_EXISTS but we want */
		/* to treat it as a ERROR_SHARING_VIOLATION, thus returning KErrInUse as on */
		/* the other platforms.  So try to open the file in read only mode, and if */
		/* it returns KErrInUse then change the error.  Otherwise the error is valid */
		/* so we want to retain it as it is */

		if (RetVal == KErrAlreadyExists)
		{
			if ((Result = Open(a_pccFileName, EFileRead)) == KErrInUse)
			{
				RetVal = KErrInUse;
			}

			/* Close the file as it may have been successfully opened by the above call */

			Close();
		}
	}

#endif /* ! __linux__ */

	return(RetVal);
}

/* Written: Monday 19-Apr-2010 6:26 am */
/* @param	a_pccFileName	Ptr to the name of the file to be created */
/*			a_uiFileMode	Mode in which to create the file.  Only for compatibility with Symbian */
/*							API and is ignored (but should be EFileWrite for consistency) */
/* @return	KErrNone if successful */
/*			KErrAlreadyExists if the file already exists */
/*			KErrPathNotFound if the path to the file does not exist */
/*			KErrGeneral if some other unexpected error occurred */
/* Creates a new file that can subsequently be used for writing operations.  If a file already */
/* exists with the same name then the function will replace it.  The a_pccFileName parameter can specify */
/* a path to the file but the path must already exist */

TInt RFile::Replace(const char *a_pccFileName, TUint a_uiFileMode)
{
	TInt RetVal;

	/* Try to delete the file specified so that it can be recreated */

	RetVal = BaflUtils::DeleteFile(a_pccFileName);

	/* If the file was deleted successfully or if it didn't exist, continue on to create a new file */

	if ((RetVal == KErrNone) || (RetVal == KErrNotFound))
	{
		RetVal = Create(a_pccFileName, a_uiFileMode);
	}

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 8:57 pm */
/* @param	a_pccFileName	Ptr to the name of the file to be opened */
/*			a_uiFileMode	Mode in which to open the file */
/* @return	KErrNone if successful */
/*			KErrPathNotFound if the path to the file does not exist */
/*			KErrNotFound if the path is ok, but the file does not exist */
/*			KErrInUse if the file is already open for writing */
/*			KErrNotEnoughMemory if not enough memory was available */
/*			KErrGeneral if some other unexpected error occurred */
/* Opens an existing file that can subsequently be used for reading operations.  The file can be opened */
/* in the file mode EFileRead, EFileWrite, or a logical combination of them both.  If the file mode */
/* EFileWrite is specified then the file will also be writeable.  The a_pccFileName parameter can */
/* optionally specify a path to the file */

TInt RFile::Open(const char *a_pccFileName, TUint a_uiFileMode)
{
	TInt RetVal;

#ifdef __amigaos4__

	/* Opening wildcards are not supported by our API although Amiga OS allows it! */

	if ((strstr(a_pccFileName, "#") == NULL) && (strstr(a_pccFileName, "?") == NULL) && (strstr(a_pccFileName, "*") == NULL))
	{
		/* Open the existing file.  We want to open it as having an exclusive lock */
		/* and being read only if EFileWrite is not specified but neither of */
		/* these features are supported by Amiga OS so we will emulate them l8r */

		if ((m_oHandle = IDOS->Open(a_pccFileName, MODE_OLDFILE)) != 0)
		{
			/* And change the shared lock to an exclusive lock as our API only */
			/* supports opening files exclusively */

			if (IDOS->ChangeMode(CHANGE_FH, m_oHandle, EXCLUSIVE_LOCK) != 0)
			{
				RetVal = KErrNone;

				/* Save the read/write mode for l8r use */

				m_uiFileMode = a_uiFileMode;
			}
			else
			{
				Utils::Info("RFile::Open() => Unable to lock file for exclusive access");

				RetVal = KErrGeneral;

				Close();
			}
		}
		else
		{
			/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

			RetVal = Utils::MapLastFileError(a_pccFileName);
		}
	}
	else
	{
		RetVal = KErrNotFound;
	}

#elif defined(__linux__)

	TInt Flags;

	/* Open an existing file in read or read/write mode as requested */

	Flags = (a_uiFileMode & EFileWrite) ? O_RDWR : O_RDONLY;

	if ((m_oHandle = open(a_pccFileName, Flags, 0)) != -1)
	{
		/* Now lock the file so that it cannot be re-opened.  The RFile API does not support having */
		/* multiple locks on individual files */

		if (flock(m_oHandle, (LOCK_EX | LOCK_NB)) == 0)
		{
			RetVal = KErrNone;
		}
		else
		{
			Utils::Info("RFile::Open() => Unable to lock file for exclusive access");

			RetVal = KErrGeneral;

			Close();
		}
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_pccFileName);

	}

#else /* ! __linux__ */

	DWORD FileMode;

	FileMode = GENERIC_READ;

	if (a_uiFileMode & EFileWrite)
	{
		FileMode |= GENERIC_WRITE;
	}

	if ((m_oHandle = CreateFile(a_pccFileName, FileMode, 0, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		/* See if this was successful.  If it wasn't due to path not found etc. then return this error */

		RetVal = Utils::MapLastFileError(a_pccFileName);
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

	/* Only allow writing if the file was opened using the EFileWrite file mode.  Amiga OS */
	/* doesn't support this functionality so we have to emulate it */

	if (m_uiFileMode & EFileWrite)
	{
		/* Now perform the write and ensure all of the bytes were written */

		RetVal = IDOS->Write(m_oHandle, a_pcucBuffer, a_iLength);

		if (RetVal == -1)
		{
			RetVal = KErrGeneral;
		}
	}
	else
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

	/* Perform the write and ensure all of the bytes were written */

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
		DEBUGCHECK((close(m_oHandle) == 0), "RFile::Close() => Unable to close file");
		m_oHandle = -1;
	}

#elif defined(__amigaos4__)

	if (m_oHandle != 0)
	{
		DEBUGCHECK((IDOS->Close(m_oHandle) != 0), "RFile::Close() => Unable to close file");
		m_oHandle = 0;
	}

#else /* ! __amigaos4__ */

	if (m_oHandle != INVALID_HANDLE_VALUE)
	{
		DEBUGCHECK((CloseHandle(m_oHandle) != FALSE), "RFile::Close() => Unable to close file");
		m_oHandle = INVALID_HANDLE_VALUE;
	}

#endif /* ! __linux__ */

}
