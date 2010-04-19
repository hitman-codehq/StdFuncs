
// TODO: CAW - Not everything has dates.  What about other files?

#include "StdFuncs.h"

#ifdef __amigaos4__

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <workbench/startup.h>

#endif /* __amigaos4__ */

#include <stdio.h>
#include <string.h>
#include "File.h"

#ifdef __amigaos4__

// TODO: CAW - Sort out whether to use Amiga OS or CRT
#define PRINTF IDOS->Printf
#define VSNPRINTF vsnprintf

#else /* ! __amigaos4__ */

#define PRINTF printf
#define VSNPRINTF _vsnprintf

static const char *g_apccMonths[] =
{
	// TODO: CAW - Check against Amiga formats
	"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"
};

#endif /* ! __amigaos4__ */

bool g_bFromWorkbench;	/* true if launched from Workbench */

/* Written: Thursday 16-Jul-2009 3:58 pm */

bool Utils::AddPart(char *a_pcDest, const char *a_pccSource, unsigned int a_iDestLength)
{
	bool RetVal;

#ifdef __amigaos4__

	RetVal = IDOS->AddPart(a_pcDest, a_pccSource, a_iDestLength);

#else /* ! __amigaos4__ */

	unsigned int Length;

	/* Assume failure */

	RetVal = false;

	/* If there is anything already in the destination string, check there is enough space to */
	/* append to it before doing anything */

	if ((Length = strlen(a_pcDest)) > 0)
	{
		if ((Length + 1 + strlen(a_pccSource)) <= a_iDestLength)
		{
			RetVal = true;

			/* Append the source string, plus a directory separator if required */

			if ((a_pcDest[Length - 1] != '\\') && (a_pcDest[Length - 1] != '/'))
			{
				strcat(a_pcDest, "/");
			}

			strcat(a_pcDest, a_pccSource);
		}
	}

	/* Otherwise just copy the source string into the destination, provided there is enough space */
	/* to do so */

	else
	{
		if (strlen(a_pccSource) < a_iDestLength)
		{
			RetVal = true;

			strcpy(a_pcDest, a_pccSource);
		}
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

#ifdef _DEBUG

/* Written: Saturday 11-Jul-2009 08:56 am */

void Utils::AssertionFailure(const char *a_pccMessage, ...)
{
	va_list Args;

	va_start(Args, a_pccMessage);

	if (g_bFromWorkbench)
	{
		MessageBox("Assertion", a_pccMessage, Args);
	}
	else
	{
		PRINTF("Assertion Failure: %s\n", a_pccMessage);
	}

	va_end(Args);
}

#endif /* _DEBUG */

/* Written: Saturday 18-Jul-2009 8:25 am */

int Utils::CreateDirectory(const char *a_pccDirectoryName)
{
	int RetVal;

#ifdef __amigaos4__

	BPTR Lock;

	// TODO: CAW - Hacky test + do error checking (IoErr()?)
	if ((Lock = IDOS->CreateDir(a_pccDirectoryName)) != 0)
	{
		RetVal = KErrNone;

		IDOS->UnLock(Lock);
	}
	else
	{

		RetVal = KErrGeneral;
	}

#else /* ! __amigaos4__ */

	if (::CreateDirectory(a_pccDirectoryName, NULL))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (GetLastError() == ERROR_ALREADY_EXISTS) ? KErrAlreadyExists : KErrNotFound;
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 11-Jul-2009 08:56 am */

// TODO: CAW - Ensure we always or never use a . in error messages!
void Utils::Error(const char *a_pccMessage, ...)
{
	// TODO: CAW - Risk of overflow, here, above and in Utils::Info
	char Message[512];
	va_list Args;

	va_start(Args, a_pccMessage);

	if (g_bFromWorkbench)
	{
		MessageBox("Error", a_pccMessage, Args);
	}
	else
	{
		VSNPRINTF(Message, sizeof(Message), a_pccMessage, Args);
		PRINTF("Error: %s\n", Message);
	}

	va_end(Args);
}

/* Written: Monday 09-Apr-2007 12:06 am */

#ifdef __amigaos4__
// TODO: CAW

bool Utils::FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, bool *a_pbDirectory)
{
	bool RetVal;
	char Path[1024]; // TODO: CAW + Comment this function
	struct ExamineData *ExamineData;

	RetVal = false;

	if (a_poWBArg->wa_Lock)
	{
		if (IDOS->NameFromLock(a_poWBArg->wa_Lock, Path, sizeof(Path)))
		{
			if ((ExamineData = IDOS->ExamineObjectTags(EX_FileLockInput, a_poWBArg->wa_Lock, TAG_DONE)) != NULL)
			{
				strcpy(a_pcFullName, Path);

				if (IDOS->AddPart(a_pcFullName, a_poWBArg->wa_Name, sizeof(Path)))
				{
					RetVal = true;

					*a_pbDirectory = EXD_IS_DIRECTORY(ExamineData);
				}
				else
				{
					Info("FullNameFromWBArg() => Unable to build filename");
				}

				IDOS->FreeDosObject(DOS_EXAMINEDATA, ExamineData);
			}
			else
			{
				Info("FullNameFromWBArg() => Unable to examine object");
			}
		}
		else
		{
			Info("FullNameFromWBArg() => Unable to obtain file name from Lock");
		}
	}

	return(RetVal);
}

#endif /* __amigaos4__ */

/* Written: Saturday 04-Jul-2009 9:20 pm */

int Utils::GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry)
{
	int RetVal;
	char *Name;

#ifdef __amigaos4__

	struct ClockData ClockData;
	struct ExamineData *ExamineData;

	if ((ExamineData = IDOS->ExamineObjectTags(EX_StringNameInput, a_pccFileName, TAG_DONE)) != NULL)
	{
		RetVal = KErrNone;

		/* Convert the new style date structure into something more usable that also contains */
		/* year, month and day information */

		IUtility->Amiga2Date(IDOS->DateStampToSeconds(&ExamineData->Date), &ClockData);

		/* Convert it so a Symbian style TDateTime structure */

		TDateTime DateTime(ClockData.year, (TMonth) (ClockData.month - 1), (ClockData.mday - 1), ClockData.hour,
			ClockData.min, ClockData.sec, 0);

		/* And populate the new TEntry instance with information about the file or directory */

		a_poEntry->Set(EXD_IS_DIRECTORY(ExamineData), EXD_IS_LINK(ExamineData), ExamineData->FileSize,
			ExamineData->Protection, DateTime, ExamineData->Date);

		// TODO: CAW - Error checking + this is a bit dodgy that the client has to free the string
		if ((Name = new char[strlen(ExamineData->Name) + 1]) != NULL)
		{
			strcpy(Name, ExamineData->Name);
			a_poEntry->iName = Name;
		}

		IDOS->FreeDosObject(DOS_EXAMINEDATA, ExamineData);
	}
	else
	{
		RetVal = KErrNotFound;

		Info("GetFileInfo() => Unable to examine object");
	}

#else /* ! __amigaos4__ */

	HANDLE Handle;
	SYSTEMTIME SystemTime;
	WIN32_FIND_DATA FindData;

	/* Assume failure */

	RetVal = KErrNotFound;

	/* Open the file to determine its properties */

	if ((Handle = FindFirstFile(a_pccFileName, &FindData)) != INVALID_HANDLE_VALUE)
	{
		/* Convert the file's timestamp to a more useful format that can be put into the TEntry structure */

		if (FileTimeToSystemTime(&FindData.ftLastWriteTime, &SystemTime))
		{
			/* Convert the Win32 SYSTEMTIME structure to a TDateTime that the TEntry can use internally */

			TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
				SystemTime.wSecond, 0);

			/* Fill in the file's properties in the TEntry structure */

			a_poEntry->Set((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0, FindData.nFileSizeLow, FindData.dwFileAttributes,
				DateTime, FindData.ftLastWriteTime);

			/* Allocate a buffer for the filename and copy the filename into it */

			if ((Name = new char[strlen(FindData.cFileName) + 1]) != NULL)
			{
				RetVal = KErrNone;

				strcpy(Name, FindData.cFileName);
				a_poEntry->iName = Name;
			}
			else
			{
				Utils::Info("Out of memory");
			}
		}
		else
		{
			Utils::Info("Unable to convert FILETIME to SYSTEMTIME");
		}

		FindClose(Handle);
	}
	else
	{
		Info("GetFileInfo() => Unable to examine file");
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Thursday 09-Jul-2009 06:58 am */

bool Utils::GetShellHeight(int *a_piHeight)
{

#ifdef __amigaos4__

	bool RetVal;
	char Buffer[32], *BufferPtr, Length, Char;
	int Result;
	BPTR InHandle, OutHandle;

	/* Assume failure */

	RetVal = false;

	/* Get the standard DOS input and output handles to use for querying the shell dimensions */

	InHandle = IDOS->Input();
	OutHandle = IDOS->Output();

	/* Put the console into RAW mode */

	if (IDOS->SetMode(OutHandle, 1))
	{
		/* Write CSI request for console window dimensions */

		if (IDOS->Write(OutHandle, "\x9b" "0 q", 4) == 4)
		{
			/* Iterate through the response and copy it into a temporary buffer */

			Length = 0;
			BufferPtr = Buffer;

			do
			{
				if ((Result = IDOS->Read(InHandle, BufferPtr, 1)) > 0)
				{
					++Length;
					Char = *BufferPtr++;
				}
				else
				{
					break;
				}
			}
			while (Char != 'r');

			/* Put the console back into cooked mode. No point in checking for the unlikely error */
			/* that might happen because what would we do anyway? */

			IDOS->SetMode(OutHandle, 0);

			/* If the response was extract successfully, extract the height into the supplied variable */

			if ((Length > 9) && (Buffer[0] == '\x9b'))
			{
				RetVal = true;
				IDOS->StrToLong(&Buffer[5], (LONG *) a_piHeight);
			}
		}
		else
		{
			Utils::Info("Utils::GetShellHeight() => Unable to request window dimensions");
		}
	}
	else
	{
		Utils::Info("Utils::GetShellHeight() => Unable to put console into RAW mode");
	}

#else /* ! __amigaos4__ */

	bool RetVal = false; // TODO: CAW

#endif /* ! __amigaos4__ */

	return(RetVal);
}

#ifdef _DEBUG

void Utils::Info(const char *a_pccMessage, ...)
{

// TODO: CAW
#ifdef __amigaos4__

	char Message[512];
	va_list Args;

	va_start(Args, a_pccMessage);
	vsnprintf(Message, sizeof(Message), a_pccMessage, Args);

	if (g_bFromWorkbench)
	{
		IExec->DebugPrintF("Info: %s\n", Message);
	}
	else
	{
		IDOS->Printf("Info: %s\n", Message);
	}

	va_end(Args);

#endif /* __amigaos4__ */

}

#endif /* _DEBUG */

/* Written: Monday 15-Feb-2010 8:59 am */

// TODO: CAW - Implement this for OS4 as well or get rid of this
bool Utils::IsDevice(const char *a_pccPath)
{
	bool RetVal;
	int Character;

	/* Assume the path passed in does not represent a device name */

	RetVal = false;

	/* If the path passed in is only two characters long then see if it represents a driver letter */

	if (strlen(a_pccPath) == 2)
	{
		Character = tolower(*a_pccPath);

		if ((Character >= 'a') && (Character <= 'z'))
		{
			if (a_pccPath[1] == ':')
			{
				RetVal = true;
			}
		}
	}

	return(RetVal);
}

// TODO: CAW - Make this GetFileInfo() so that LoadFile() and OpenDirectory() can use it.  Comments all through here!
/* Written: Monday 09-Apr-2007 12:20 am */

// TODO: CAW
#ifdef __amigaos4__

bool Utils::IsDirectory(const char *a_pccFileName, bool *a_pbDirectory)
{
	bool RetVal;
	struct ExamineData *ExamineData;

	RetVal = false;

	if ((ExamineData = IDOS->ExamineObjectTags(EX_StringNameInput, a_pccFileName, TAG_DONE)) != NULL)
	{
		RetVal = true;
		*a_pbDirectory = (EXD_IS_DIRECTORY(ExamineData)) ? true : false;

		IDOS->FreeDosObject(DOS_EXAMINEDATA, ExamineData);
	}
	else
	{
		Info("IsDirectory() => Unable to examine object");
	}

	return(RetVal);
}

#endif

// TODO: CAW - This is going to break AMC + standardise on errors
int Utils::LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer)
{
	unsigned char *Buffer;
	int RetVal;
	TEntry Entry;

	/* Obtain information about the file, such as its size */

	if ((RetVal = Utils::GetFileInfo(a_pccFileName, &Entry)) == KErrNone)
	{
		/* Allocate a buffer of the appropriate size */

		if ((Buffer = new unsigned char[Entry.iSize + 1]) != NULL)
		{
			RFile File;

			/* Open the file and read its contents into the buffer */

			if ((RetVal = File.Open(a_pccFileName, EFileRead)) == KErrNone)
			{
				if ((RetVal = File.Read(Buffer, Entry.iSize)) == (int) Entry.iSize)
				{
					RetVal = KErrNone;

					/* NULL terminate the buffer and save its ptr for the calling client */

					Buffer[Entry.iSize] = '\0';
					*a_ppucBuffer = Buffer;
				}
				else
				{
					Utils::Info("Utils::LoadFile() => Unable to read file \"%s\"", a_pccFileName);
				}
			}
			else
			{
				Utils::Info("Utils::LoadFile() => Unable to open file \"%s\" for loading", a_pccFileName);
			}

			File.Close();

			/* If any error occurred, free the buffer as it won't get passed back to the client */

			if (RetVal != KErrNone)
			{
				delete [] Buffer;
			}
		}
		else
		{
			RetVal = KErrNoMemory;

			Utils::Info("Utils::LoadFile() => Out of memory");
		}
	}
	else
	{
		RetVal = KErrNotFound;

		Utils::Info("Utils::LoadFile() => Unable to get information about file \"%s\"", a_pccFileName);
	}

	return(RetVal);
}

void Utils::MessageBox(const char *a_pccTitle, const char *a_pccMessage, va_list a_oArgs)
{

// TODO: CAW
#ifdef __amigaos4__

	char Message[512];
	struct EasyStruct EasyStruct;
	struct Window *Requester;

	vsnprintf(Message, sizeof(Message), a_pccMessage, a_oArgs);

	EasyStruct.es_StructSize = sizeof(struct EasyStruct);
	EasyStruct.es_Flags = 0;
	EasyStruct.es_Title = (char *) a_pccTitle;
	EasyStruct.es_TextFormat = Message;
	EasyStruct.es_GadgetFormat = "Ok";

	// TODO: CAW - This is a strange function. Look into it and others + BuildSysRequest()
	Requester = IIntuition->BuildEasyRequest(NULL, &EasyStruct, 0, TAG_DONE);

	if (Requester)
	{
		IExec->Wait((1 << Requester->UserPort->mp_SigBit));
		IIntuition->FreeSysRequest(Requester);
	}
	else
	{
		IDOS->Printf(a_pccTitle);
		IDOS->Printf(": %s\n", Message);
	}

#endif /* __amigaos4__ */

}

// TODO: CAW
#ifdef __amigaos4__

// TODO: CAW - Convert this to use RDir
bool Utils::ScanDirectory(const char *a_pccDirectoryName, bool a_bScanFiles, ScanFunc a_pfScanFunc, void *a_pvUserData)
{
	bool Error, RetVal;
	APTR Context;
	struct ExamineData *ExamineData;

	/* Assume failure */

	Error = RetVal = false;

	// TODO: CAW - Fix all comments & error messages. Check result of ExamineDir()
	/* Allocate some data structures into which the information about the directory entries may be placed, */
	/* and a control structure for scanning through the directory */

	// TODO: CAW - Not all of these are required
	Context = IDOS->ObtainDirContextTags(EX_StringNameInput, a_pccDirectoryName,
		EX_DataFields, (EXF_DATE | EXF_PROTECTION | EXF_NAME | EXF_SIZE | EXF_TYPE), TAG_END); // TODO: CAW

	if (Context)
	{
		RetVal = true;

		while ((ExamineData = IDOS->ExamineDir(Context)) != NULL)
		{
			// TODO: CAW - There was a bug with this. What about directory links?
			if ((EXD_IS_FILE(ExamineData)) || (EXD_IS_LINK(ExamineData)))
			{
				if (a_bScanFiles)
				{
					if (!(a_pfScanFunc((const char *) ExamineData->Name, a_pvUserData)))
					{
						Error = true;

						break;
					}
				}
			}
			else if (EXD_IS_DIRECTORY(ExamineData))
			{
				if (!(a_bScanFiles))
				{
					if (!(a_pfScanFunc((const char *) ExamineData->Name, a_pvUserData)))
					{
						Error = true;

						break;
					}
				}
			}
		}

		IDOS->ReleaseDirContext(Context);
	}
	else
	{
		Info("ScanDirectory() => Unable to create context to examine object");
	}

	return(RetVal);
}

#endif /* __amigaos4__ */

/* Written: Saturday 18-Jul-2009 8:06 am */

int Utils::SetFileDate(const char *a_pccFileName, const TEntry &a_roEntry)
{
	int RetVal;

#ifdef __amigaos4__

	// TODO: Return value here and for SetProtection(). Should this be in Utils?
	RetVal = KErrNone;
	IDOS->SetFileDate(a_pccFileName, &a_roEntry.iAmigaDate);

#else /* ! __amigaos4__ */

	HANDLE Handle;

	/* Assume failure */

	RetVal = KErrGeneral;

	/* Open a handle to the file and set its datestamp to that passed in */

	if ((Handle = CreateFile(a_pccFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL)) != INVALID_HANDLE_VALUE)
	{
		if (SetFileTime(Handle, &a_roEntry.iWindowsDate, &a_roEntry.iWindowsDate, &a_roEntry.iWindowsDate))
		{
			RetVal = KErrNone;
		}

		CloseHandle(Handle);
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 18-Jul-2009 7:59 am */

int Utils::SetProtection(const char *a_pccFileName, unsigned int a_uiAttributes)
{
	int RetVal;

#ifdef __amigaos4__

	// TODO: CAW + Write a test case for this, that tests KErrNone, KErrNotFound and KErrGeneral return codes
	RetVal = KErrNone;
	IDOS->SetProtection(a_pccFileName, a_uiAttributes);

#else /* ! __amigaos4__ */

	if (SetFileAttributes(a_pccFileName, a_uiAttributes))
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (GetLastError() == ERROR_FILE_NOT_FOUND) ? KErrNotFound : KErrGeneral;
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 23-Jul-2009 06:58 am */

void Utils::TimeToString(char *a_pcDate, char *a_pcTime, const TEntry &a_roEntry)
{

#ifdef __amigaos4__

	struct DateTime DateTime = { { 0, 0, 0 }, FORMAT_DOS, DTF_SUBST, NULL, NULL, NULL };

	// TODO: CAW - This is broken
	DateTime.dat_Stamp = a_roEntry.iAmigaDate;
	DateTime.dat_StrTime = a_pcTime;
	DateTime.dat_StrDate = a_pcDate;

	if (IDOS->DateToStr(&DateTime))
	{
		// TODO: CAW - Return value
		//printf(" %s %s", Time, Date);
	}

#else /* ! __amigaos4__ */

	// TODO: CAW - Check against Amiga format + this is ugly!
	sprintf(a_pcDate, "%02d-%s-%04d", a_roEntry.iModified.DateTime().Day(), g_apccMonths[a_roEntry.iModified.DateTime().Month()],
		a_roEntry.iModified.DateTime().Year());

	sprintf(a_pcTime, "%02d:%02d", a_roEntry.iModified.DateTime().Hour(), a_roEntry.iModified.DateTime().Minute());

#endif /* __amigaos4__ */

}

/* Written: Saturday 16-Sep-2006 4:49 pm */

ULONG Utils::Red32(unsigned long a_ulColour)
{
	ULONG Red;

	Red = ((a_ulColour & 0xff0000) >> 16);

	return((Red << 24) | (Red << 16) | (Red << 8) | Red);
}

/* Written: Saturday 16-Sep-2006 4:49 pm */

ULONG Utils::Green32(unsigned long a_ulColour)
{
	ULONG Green;

	Green = ((a_ulColour & 0xff00) >> 8);

	return((Green << 24) | (Green << 16) | (Green << 8) | Green);
}

/* Written: Saturday 16-Sep-2006 4:49 pm */

ULONG Utils::Blue32(unsigned long a_ulColour)
{
	ULONG Blue;

	Blue = (a_ulColour & 0xff);

	return((Blue << 24) | (Blue << 16) | (Blue << 8) | Blue);
}

