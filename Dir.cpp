
#include "StdFuncs.h"

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/utility.h>

#endif /* __amigaos4__ */

#include <string.h>
#include "Dir.h"

// TODO: CAW - Add Utils::Info() calls all though here and RFile (check others) but only in debug mode?  Consistency!

static const char *EmptyString = "";

/* Written: Saturday 03-Nov-2007 7:27 pm */

TEntry::TEntry()
{
	iName = EmptyString;
	iIsDir = iIsLink = EFalse;
	iSize = 0;
	iAttributes = 0;
}

/* Written: Firday 10-Jul-2009 6:42 am */

TEntry::~TEntry()
{
	if (iName != EmptyString)
	{
		delete [] (char *) iName;
	}
}

/* Written: Saturday 03-Nov-2007 8:07 pm */

TBool TEntry::IsDir() const
{
	return(iIsDir);
}

/* Written: Saturday 03-Nov-2007 8:15 pm */

TBool TEntry::IsLink() const
{
	return(iIsLink);
}

/* Written: Monday 28-Dec-2009 12:15 pm */

TBool TEntry::IsHidden() const
{

#ifdef __amigaos4__

	return(EFalse); // TODO: CAW + use a similar check to below above?

#else /* ! __amigaos4__ */

	return(iAttributes & FILE_ATTRIBUTE_HIDDEN);

#endif /* ! __amigaos4__ */

}

/* Written: Saturday 03-Nov-2007 6:42 pm */

#ifdef __amigaos4__

void TEntry::Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_roDateTime,
	const struct DateStamp &a_roPlatformDate)

#else /* ! __amigaos4__ */

void TEntry::Set(TBool a_bIsDir, TBool a_bIsLink, TUint a_uiSize, TUint a_uiAttributes, const TDateTime &a_roDateTime,
	const FILETIME &a_roPlatformDate)

#endif /* ! __amigaos4__ */

{
	iIsDir = a_bIsDir;
	iIsLink = a_bIsLink;
	iSize = a_uiSize;
	iAttributes = a_uiAttributes;
	iModified = a_roDateTime;
	iPlatformDate = a_roPlatformDate;
}

/* Written: Saturday 03-Nov-2007 5:57 pm */

TEntryArray::TEntryArray()
{
	iCount = 0;
};

/* Written: Saturday 03-Nov-2007 5:58 pm */

TEntryArray::~TEntryArray()
{
	Purge();
}

/* Written: Friday 19-Jun-2009 7:11 am */

TEntry *TEntryArray::Append(const char *a_pccName)
{
	char *Name;
	TEntry *Entry;

	/* Allocate a new TEntry node and some memory to hold the name of the file it represents */

	if ((Entry = new TEntry) != NULL)
	{
		if ((Name = new char[strlen(a_pccName) + 1]) != NULL)
		{
			/* Copy the name of the file */

			strcpy(Name, a_pccName);
			Entry->iName = Name;

			/* And append the node to the list */

			++iCount;
			iEntries.AddTail(Entry);
		}
		else
		{
			delete Entry;
			Entry = NULL;
		}
	}

	return(Entry);
}

/* Written: Saturday 03-Nov-2007 6:18 pm */

TInt TEntryArray::Count() const
{
	return(iCount);
}

/* Written: Saturday 03-Nov-2007 6:20 pm */

const TEntry &TEntryArray::operator[](TInt a_iIndex) const
{
	// TODO: CAW - Will be slow + what if this is NULL? Check for count and assert
	TEntry *Entry;

	Entry = iEntries.GetHead();

	while (a_iIndex > 0)
	{
		Entry = iEntries.GetSucc(Entry);
		--a_iIndex;
	}

	return(*Entry);
}

/* Written: Saturday 11-Jul-2008 10:44 pm */

const TEntry *TEntryArray::GetHead() const
{
	return(iEntries.GetHead());
}

const TEntry *TEntryArray::GetSucc(const TEntry *a_poEntry) const
{
	return(iEntries.GetSucc(a_poEntry));
}

/* Written: Saturday 11-Jul-2010 3:36 pm */

void TEntryArray::Purge()
{
	TEntry *Entry;

	/* Iterate through the list of nodes and delete each one */

	while ((Entry = iEntries.RemHead()) != NULL)
	{
		delete Entry;
	}

	/* And indicate that the array is now empty */

	iCount = 0;
}

/* Written: Saturday 11-Jul-2008 11:42 pm */

void TEntryArray::Remove(const TEntry *a_poEntry)
{
	iEntries.Remove((TEntry *) a_poEntry); // TODO: CAW - Cast
	--iCount;
}

/* Written: Saturday 03-Nov-2007 5:24 pm */

RDir::RDir()
{

#ifdef __amigaos4__

	iPath = NULL;
	iPattern = NULL;
	iContext = NULL;

#endif /* __amigaos4__ */

	iSingleEntry.iName = NULL;
	iSingleEntryOk = EFalse;
}

/* Written: Saturday 03-Nov-2007 4:43 pm */

TInt RDir::Open(const char *a_pccPattern)
{
	TInt RetVal;
	TEntry *Entry;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Get information about the file or directory being passed in. If it is a file or a link */
	/* then save the information for l8r and don't continue */

	// TODO: CAW
	if (Utils::GetFileInfo(a_pccPattern, &iSingleEntry) == KErrNone)
	{
		if (!(iSingleEntry.IsDir()))
		{
			iSingleEntryOk = ETrue;

			/* Append the entry to the array of files and directories being listed */

			if ((Entry = iEntries.Append(iSingleEntry.iName)) != NULL)
			{
				RetVal = KErrNone;

				// TODO: CAW - Why isn't this using Set()? Is Set() required?
				Entry->iIsDir = iSingleEntry.iIsDir;
				Entry->iIsLink = iSingleEntry.iIsLink;
				Entry->iSize = iSingleEntry.iSize;
				Entry->iAttributes = iSingleEntry.iAttributes;
				Entry->iModified = iSingleEntry.iModified;
				Entry->iPlatformDate = iSingleEntry.iPlatformDate;
			}
		}
	}

#ifdef __amigaos4__

	char *Pattern;
	TInt Length;
	LONG Result;
	STRPTR FileNameOffset;

	/* Allocate a buffer for the path passed in and save it for l8r use */

	// TODO: CAW - Make like Windows version, here and in Read()
	if ((iPath = new char[strlen(a_pccPattern) + 1]) != NULL)
	{
		strcpy(iPath, a_pccPattern);

		if (!(iSingleEntryOk))
		{
			Length = (strlen(a_pccPattern) * 2 + 4 + 2); // TODO: CAW

			if ((iPattern = new char[Length]) != NULL)
			{
				RetVal = KErrNone;

				Pattern = IDOS->FilePart(a_pccPattern);

				if (*Pattern == '\0')
				{
					Pattern = "#?"; // TODO: CAW - Needed?
				}

				if ((Result = IDOS->ParsePatternNoCase(Pattern, iPattern, Length)) == 1)
				{
					FileNameOffset = IDOS->PathPart(iPath);
					iPath[FileNameOffset - iPath] = '\0';
				}
				else if (Result == 0)
				{
					delete [] iPattern;
					iPattern = NULL;
				}
				else
				{
					RetVal = KErrGeneral; // TODO: CAW
				}
			}

			if (RetVal == KErrNone)
			{
				/* Open a context for the directory to be scanned */

				iContext = IDOS->ObtainDirContextTags(EX_StringNameInput, iPath,
					EX_DataFields, (EXF_DATE | EXF_PROTECTION | EXF_NAME | EXF_SIZE | EXF_TYPE), TAG_END);

				if (iContext)
				{
					RetVal = KErrNone; // TODO: CAW
				}
				else
				{
					Result = IDOS->IoErr();

					if (Result == ERROR_OBJECT_NOT_FOUND)
					{
						RetVal = KErrNotFound;
					}
					else
					{
						RetVal = KErrGeneral;
					}
				}
			}
		}
	}

#else /* ! __amigaos4__ */

	char Path[10240]; // TODO: CAW
	WIN32_FIND_DATA FindData;

	if (!(iSingleEntryOk))
	{
		// TODO: CAW - Should only scan the end of a_pccPattern using FilePart()
		if (!(strstr(a_pccPattern, "*")) && (!(strstr(a_pccPattern, "?"))))
		{
			if (strlen(a_pccPattern) > 0)
			{
				strcpy(Path, a_pccPattern);
				strcat(Path, "/*.*");
			}
			else
			{
				strcpy(Path, "*.*"); // TODO: CAW - Needed
			}
		}
		else
		{
			strcpy(Path, a_pccPattern); // TODO: CAW - Needed
		}

		if ((iHandle = FindFirstFile(Path, &FindData)) != INVALID_HANDLE_VALUE)
		{
			RetVal = KErrNone;

			if ((strcmp(FindData.cFileName, ".")) && (strcmp(FindData.cFileName, "..")))
			{
				// TODO: CAW - What frees that + all in one function please + name + finish date time stuff
				TEntry *Lah = iEntries.Append(FindData.cFileName);

				if (Lah)
				{
					SYSTEMTIME SystemTime;

					if (FileTimeToSystemTime(&FindData.ftLastWriteTime, &SystemTime))
					{
						TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, 0);

						// TODO: CAW - Link stuff, here and in Utils.cpp
						Lah->Set((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0/*EXD_IS_LINK(Entry)*/, FindData.nFileSizeLow, FindData.dwFileAttributes, DateTime,
							FindData.ftLastWriteTime); // TODO: CAW - Extend T_File/T_Dir for this
					}
					else
					{
						RetVal = KErrGeneral; // TODO: CAW + this stuff is done here twice and in Utils::GetFileInfo()
					}
				}
				else
				{
					RetVal = KErrNoMemory;
				}
			}
		}
		else
		{
			// TODO: CAW - Ensure these match Symbian return values. Not finding a file is ok as it means
			//			that the directory was opened successfully but that no files were in it. Write a
			//			test case to handle this situation
			if (GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				RetVal = KErrNone;
			}
			else
			{
				RetVal = KErrNotFound;
			}
		}
	}

#endif /* ! __amigaos4__ */

	/* If anything went wrong, clean up whatever was allocated */

	if (RetVal != KErrNone)
	{
		Close();
	}

	return(RetVal);
}

/* Written: Saturday 03-Nov-2007 4:49 pm */

void RDir::Close()
{
	/* Free the contents of the TEntry array in case it the RDir class is reused */

	iEntries.Purge();

#ifdef __amigaos4__

	delete [] iPath;
	iPath = NULL;

	delete [] iPattern;
	iPattern = NULL;

	if (iContext)
	{
		IDOS->ReleaseDirContext(iContext);
		iContext = NULL;
	}

#else /* ! __amigaos4__ */

	if (iHandle)
	{
		FindClose(iHandle);
		iHandle = NULL;
	}

#endif /* ! __amigaos4__ */

	// TODO: CAW
	delete [] (char *) iSingleEntry.iName;
	iSingleEntry.iName = NULL;

	iSingleEntryOk = EFalse;
}

/* Written: Saturday 03-Nov-2007 5:38 pm */

TInt RDir::Read(TEntryArray *&a_roEntries)
{
	TInt RetVal;

#ifdef __amigaos4__

	char *LinkName;
	TBool AddFile, Error;
	TInt Length, Size;
	struct ClockData ClockData;
	struct ExamineData *ExamineData;
	struct TEntry *Entry, LinkEntry;

	/* Assume success */

	Error = EFalse;
	RetVal = KErrNone;
	a_roEntries = &iEntries;

	/* Iterate through the scanned entries and prepare a TEntry instance to contain */
	/* the appropriate information about each entry */

	if (iContext) // TODO: CAW - Test without this + perform full failure testing
	{
		while ((ExamineData = IDOS->ExamineDir(iContext)) != NULL)
		{
			/* Add the file to the list by default */

			AddFile = ETrue;

			/* If we are using pattern matching then check to see if the file matches the patttern */
			/* and if not, don't add it to the list */

			if (iPattern)
			{
				if (!(IDOS->MatchPatternNoCase(iPattern, ExamineData->Name)))
				{
					AddFile = EFalse; // TODO: CAW - What about error checking using IDOS->IoErr() below?
				}
			}

			/* Add the file to the list if it matches or no wildcards were used */

			if (AddFile)
			{
				if ((Entry = iEntries.Append(ExamineData->Name)) != NULL)
				{
					/* Convert the new style date structure into something more usable that also contains */
					/* year, month and day information */

					IUtility->Amiga2Date(IDOS->DateStampToSeconds(&ExamineData->Date), &ClockData);

					/* Convert it so a Symbian style TDateTime structure */

					TDateTime DateTime(ClockData.year, (TMonth) (ClockData.month - 1), (ClockData.mday - 1), ClockData.hour,
						ClockData.min, ClockData.sec, 0);

					/* If the file found is a link then special processing will be required to determine */
					/* its real size, as ExamineDir() will return an ExamineData::FileSize of -1 for links. */
					/* If this fails then we will just print a warning and continue, as there is not much */
					/* that can be done about it */

					if (EXD_IS_LINK(ExamineData))
					{
						/* Set the size to 0 by default so as not to mess up any size calculations with -1 */

						Size = 0;

						/* Allocate a temporary buffer large enough to hold the fully qualified link name */

						Length = (strlen(iPath) + 1 + strlen(ExamineData->Name) + 1);

						if ((LinkName = new char[Length]) != NULL)
						{
							/* And copy the link name into the buffer */

							strcpy(LinkName, iPath);
							IDOS->AddPart(LinkName, ExamineData->Name, Length);

							/* Obtain the size of the file that the link points to */

							if (Utils::GetFileInfo(LinkName, &LinkEntry) == KErrNone) // TODO: CAW
							{
								Size = LinkEntry.iSize;
							}

							delete [] LinkName;
						}
						else
						{
							Utils::Info("Unable to allocate buffer to resolve link size\n");
						}
					}
					else
					{
						Size = ExamineData->FileSize;
					}

					/* And populate the new TEntry instance with information about the file or directory */

					Entry->Set(EXD_IS_DIRECTORY(ExamineData), EXD_IS_LINK(ExamineData), Size, ExamineData->Protection,
						DateTime, ExamineData->Date);
				}
				else
				{
					RetVal = KErrNoMemory;

					break;
				}
			}
		}

		/* There are not many errors that can be returned from IDOS->ExamineDir(), except ones to */
		/* indicate that a disk error occurred. Check for anything besides ERROR_NO_MORE_ENTRIES and */
		/* return an error if approrpriate so that client software can abort its attempt to read the */
		/* directory */

		if (IDOS->IoErr() != ERROR_NO_MORE_ENTRIES)
		{
			RetVal = KErrGeneral;
		}
	}

	/* If iContext == NULL then either we are being called before Open() has been called, or Open() */
	/* has been called for a single file. Return KErrNone or an error as appropriate */

	else
	{
		if (!(iSingleEntryOk))
		{
			RetVal = KErrGeneral; // TODO: CAW - What should this be?
		}
	}

#else /* ! __amigaos4__ */

	WIN32_FIND_DATA FindData;

	/* Assume success */

	RetVal = KErrNone;
	a_roEntries = &iEntries;

	if (!(iSingleEntryOk))
	{
		while (FindNextFile(iHandle, &FindData))
		{
			if ((strcmp(FindData.cFileName, ".")) && (strcmp(FindData.cFileName, "..")))
			{
				// TODO: CAW - What frees that + all in one function please + name + finish date time stuff
				TEntry *Lah = iEntries.Append(FindData.cFileName);

				if (Lah)
				{
					SYSTEMTIME SystemTime;

					// TODO: CAW - This should take DST into account. What about on Amiga OS?
					if (FileTimeToSystemTime(&FindData.ftLastWriteTime, &SystemTime))
					{
						TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, 0);

						Lah->Set((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0/*EXD_IS_LINK(Entry)*/, FindData.nFileSizeLow, FindData.dwFileAttributes, DateTime,
							FindData.ftLastWriteTime);
					}
					else
					{
						RetVal = KErrGeneral; // TODO: CAW
					}
				}
				else
				{
					RetVal = KErrNoMemory;

					break;
				}
			}
		}
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}
