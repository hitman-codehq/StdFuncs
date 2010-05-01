
#include "StdFuncs.h"
#include "BaUtils.h"
#include "File.h"

/* Written: Friday 02-Jan-2009 9:03 pm */

RFile::RFile()
{
	m_oHandle = 0;
}

/* Written: Friday 02-Jan-2009 8:54 pm */

int RFile::Create(const char *a_pccName, unsigned int a_uiFileMode)
{
	int RetVal;

	(void) a_uiFileMode;

#ifdef __amigaos4__

	// TODO: CAW - Map Symbian -> Amiga open modes, here and in Open(). Return correct errors
	if ((m_oHandle = IDOS->Open(a_pccName, MODE_NEWFILE)) != 0)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = KErrNotFound;
	}

#else /* ! __amigaos4__ */

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

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Monday 19-Apr-2010 6:26 am */

int RFile::Replace(const char *a_pccName, unsigned int a_uiFileMode)
{
	int RetVal;

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

int RFile::Open(const char *a_pccName, unsigned int a_uiFileMode)
{
	int RetVal;

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

#else /* ! __amigaos4__ */

	DWORD FileMode;

	FileMode = GENERIC_READ;

	if (a_uiFileMode & EFileWrite)
	{
		FileMode |= GENERIC_WRITE;
	}

	if ((m_oHandle = CreateFile(a_pccName, FileMode, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE)
	{
		RetVal = KErrNone;
	}
	else
	{
		// TODO: CAW - Write a test for this
		RetVal = (GetLastError() == ERROR_SHARING_VIOLATION) ? KErrInUse : KErrNotFound;
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 10:20 pm */

int RFile::Read(unsigned char *a_pucBuffer, int a_iLength) const
{
	int RetVal;

	// TODO: CAW - Assert on being open

#ifdef __amigaos4__

	RetVal = IDOS->Read(m_oHandle, a_pucBuffer, a_iLength);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __amigaos4__ */

	DWORD BytesRead;

	if (ReadFile(m_oHandle, a_pucBuffer, a_iLength, &BytesRead, NULL))
	{
		RetVal = BytesRead;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 10:29 pm */

int RFile::Write(const unsigned char *a_pcucBuffer, int a_iLength)
{
	int RetVal;

	// TODO: CAW - Assert on being open

#ifdef __amigaos4__

	RetVal = IDOS->Write(m_oHandle, a_pcucBuffer, a_iLength);

	if (RetVal == -1)
	{
		RetVal = KErrGeneral;
	}

#else /* ! __amigaos4__ */

	DWORD BytesWritten;

	if ((WriteFile(m_oHandle, a_pcucBuffer, a_iLength, &BytesWritten, NULL)) &&
		((int) BytesWritten == a_iLength))
	{
		RetVal = a_iLength;
	}
	else
	{
		RetVal = KErrGeneral;
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Friday 02-Jan-2009 8:58 pm */

void RFile::Close()
{
	if (m_oHandle != 0)
	{

#ifdef __amigaos4__

		IDOS->Close(m_oHandle);

#else /* ! __amigaos4__ */

		CloseHandle(m_oHandle);

#endif /* ! __amigaos4__ */

		m_oHandle = 0;
	}
}
