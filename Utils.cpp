
#include "StdFuncs.h"
#include "StdWindow.h"

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

TBool g_bUsingGUI;	/* ETrue if running a GUI based program */

/* Written: Thursday 16-Jul-2009 3:58 pm */

TBool Utils::AddPart(char *a_pcDest, const char *a_pccSource, TUint a_iDestLength)
{
	TBool RetVal;

#ifdef __amigaos4__

	RetVal = IDOS->AddPart(a_pcDest, a_pccSource, a_iDestLength);

#else /* ! __amigaos4__ */

	TUint Length;

	/* Assume failure */

	RetVal = EFalse;

	/* If there is anything already in the destination string, check there is enough space to */
	/* append to it before doing anything */

	if ((Length = strlen(a_pcDest)) > 0)
	{
		if ((Length + 1 + strlen(a_pccSource)) <= a_iDestLength)
		{
			RetVal = ETrue;

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
			RetVal = ETrue;

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

	if (g_bUsingGUI)
	{
		MessageBox("Assertion Failure", a_pccMessage, EMBTOk, Args);
	}
	else
	{
		PRINTF("Assertion Failure: %s\n", a_pccMessage);
	}

	va_end(Args);
}

#endif /* _DEBUG */

/* Written: Friday 04-Jun-2010 7:58 am */

// TODO: CAW - Oddly named function + doesn't take " into account
TInt Utils::CountTokens(char *a_pcBuffer)
{
	char Char;
	TBool TokenStart;
	TInt Source, Dest, RetVal;

	/* Iterate through the string passed in and determine how many tokens are present */

	TokenStart = ETrue;
	Dest = RetVal = 0;

	for (Source = 0; a_pcBuffer[Source]; ++Source)
	{
		Char = a_pcBuffer[Source];

		/* If the current character is a space then copy it only if the previous character was not */
		/* also a space */

		if (Char == ' ')
		{
			TokenStart = ETrue;
			a_pcBuffer[Dest++] = a_pcBuffer[Source];
		}

		/* The current character is not a space so just copy it */

		else
		{
			a_pcBuffer[Dest++] = a_pcBuffer[Source];

			/* If this is the first character of a token then increment the token count */

			if (TokenStart)
			{
				++RetVal;
				TokenStart = EFalse;
			}
		}
	}

	return(RetVal);
}

/* Written: Saturday 18-Jul-2009 8:25 am */

TInt Utils::CreateDirectory(const char *a_pccDirectoryName)
{
	TInt RetVal;

#ifdef __amigaos4__

	BPTR Lock;

	if ((Lock = IDOS->CreateDir(a_pccDirectoryName)) != 0)
	{
		RetVal = KErrNone;

		IDOS->UnLock(Lock);
	}
	else
	{
		RetVal = (IDOS->IoErr() == ERROR_OBJECT_EXISTS) ? KErrAlreadyExists : KErrNotFound;
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

	if (g_bUsingGUI)
	{
		MessageBox("Error", a_pccMessage, EMBTOk, Args);
	}
	else
	{
		VSNPRINTF(Message, sizeof(Message), a_pccMessage, Args);
		PRINTF("Error: %s\n", Message);
	}

	va_end(Args);
}

/* Written: Thursday 22-Jul-2010 8:47 am */

const char *Utils::Extension(const char *a_pccFileName)
{
	char Char;
	const char *RetVal;

	RetVal = (a_pccFileName + strlen(a_pccFileName));

	while (RetVal >= a_pccFileName)
	{
		Char = *RetVal;

		if (Char == '.')
		{
			++RetVal;

			break;
		}

		--RetVal;
	}

	if (RetVal < a_pccFileName)
	{
		RetVal = NULL;
	}

	return(RetVal);
}

/* Written: Thursday 22-Jul-2010 8:11 am */

const char *Utils::FilePart(const char *a_pccPath)
{
	char Char;
	const char *RetVal;

	RetVal = (a_pccPath + strlen(a_pccPath));

	while (RetVal > a_pccPath)
	{
		Char = *RetVal;

		if ((Char == '/') || (Char == '\\') || (Char == ':'))
		{
			++RetVal;

			break;
		}

		--RetVal;
	}

	return(RetVal);
}

/* Written: Monday 09-Apr-2007 12:06 am */

#ifdef __amigaos4__

TBool Utils::FullNameFromWBArg(char *a_pcFullName, struct WBArg *a_poWBArg, TBool *a_pbDirectory)
{
	char Path[1024]; // TODO: CAW + Comment this function
	TBool RetVal;
	struct ExamineData *ExamineData;

	RetVal = EFalse;

	if (a_poWBArg->wa_Lock)
	{
		if (IDOS->NameFromLock(a_poWBArg->wa_Lock, Path, sizeof(Path)))
		{
			if ((ExamineData = IDOS->ExamineObjectTags(EX_FileLockInput, a_poWBArg->wa_Lock, TAG_DONE)) != NULL)
			{
				strcpy(a_pcFullName, Path);

				if (IDOS->AddPart(a_pcFullName, a_poWBArg->wa_Name, sizeof(Path)))
				{
					RetVal = ETrue;

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

TInt Utils::GetFileInfo(const char *a_pccFileName, TEntry *a_poEntry)
{
	TInt RetVal;

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

		/* Copy the filename into the TEntry structure */

		strcpy(a_poEntry->iName, ExamineData->Name);

		IDOS->FreeDosObject(DOS_EXAMINEDATA, ExamineData);
	}
	else
	{
		RetVal = KErrNotFound;

		Info("GetFileInfo() => Unable to examine file \"%s\"", a_pccFileName);
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
			RetVal = KErrNone;

			/* Convert the Win32 SYSTEMTIME structure to a TDateTime that the TEntry can use internally */

			TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute,
				SystemTime.wSecond, 0);

			/* Fill in the file's properties in the TEntry structure */

			a_poEntry->Set((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY), 0, FindData.nFileSizeLow, FindData.dwFileAttributes,
				DateTime, FindData.ftLastWriteTime);

			/* Copy the filename into the TEntry structure */

			strcpy(a_poEntry->iName, FindData.cFileName);
		}
		else
		{
			Utils::Info("Unable to convert FILETIME to SYSTEMTIME");
		}

		FindClose(Handle);
	}
	else
	{
		Info("GetFileInfo() => Unable to examine file \"%s\"", a_pccFileName);
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 08-May-2010 3:16 pm */

void Utils::GetScreenSize(TInt *a_piWidth, TInt *a_piHeight)
{

#ifdef __amigaos4__

	struct Screen *Screen;

	if ((Screen = IIntuition->LockPubScreen(NULL)) != NULL)
	{
		*a_piWidth = Screen->Width;
		*a_piHeight = Screen->Height;

		IIntuition->UnlockPubScreen(NULL, Screen);
	}

#else /* ! __amigaos4__ */

	RECT Rect;

	if (GetWindowRect(GetDesktopWindow(), &Rect))
	{
		*a_piWidth = (Rect.right - Rect.left);
		*a_piHeight = (Rect.bottom - Rect.top);
	}

#endif /* ! __amigaos4__ */

	else
	{
		*a_piWidth = 640;
		*a_piHeight = 480;
	}
}

/* Written: Thursday 09-Jul-2009 06:58 am */

TBool Utils::GetShellHeight(TInt *a_piHeight)
{
	TBool RetVal;

	/* Assume failure */

	RetVal = EFalse;

#ifdef __amigaos4__

	char Buffer[32], *BufferPtr, Length, Char;
	TInt Result;
	BPTR InHandle, OutHandle;

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
				RetVal = ETrue;
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

	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	HANDLE StdOut;

	/* Get the handle of the stdin device and from that obtain the console's height */

	if ((StdOut = GetStdHandle(STD_OUTPUT_HANDLE)) != INVALID_HANDLE_VALUE)
	{
		if (GetConsoleScreenBufferInfo(StdOut, &ScreenBufferInfo))
		{
			/* Signal success */

			RetVal = ETrue;

			/* And save the height of the console for the caller, converting the zero based line */
			/* number of the bottom most line into a count of lines */

			*a_piHeight = (ScreenBufferInfo.srWindow.Bottom + 1);
		}
		else
		{
			Utils::Info("Utils::GetShellHeight() => Unable to request window dimensions");
		}
	}
	else
	{
		Utils::Info("Utils::GetShellHeight() => Unable get handle to console");
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

#ifdef _DEBUG

void Utils::Info(const char *a_pccMessage, ...)
{
	char Message[512];
	va_list Args;

	va_start(Args, a_pccMessage);
	strcpy(Message, "Info: " );
	VSNPRINTF(&Message[6], (sizeof(Message) - 6), a_pccMessage, Args);

#ifdef __amigaos4__

	IExec->DebugPrintF("%s\n", Message);

#else /* ! __amigaos4__ */

	OutputDebugString(Message);
	OutputDebugString("\n");

#endif /* ! __amigaos4__ */

	va_end(Args);
}

#endif /* _DEBUG */

// TODO: CAW - Make this GetFileInfo() so that LoadFile() and OpenDirectory() can use it.  Comments all through here!
/* Written: Monday 09-Apr-2007 12:20 am */

// TODO: CAW
#ifdef __amigaos4__

TBool Utils::IsDirectory(const char *a_pccFileName, TBool *a_pbDirectory)
{
	TBool RetVal;
	struct ExamineData *ExamineData;

	RetVal = EFalse;

	if ((ExamineData = IDOS->ExamineObjectTags(EX_StringNameInput, a_pccFileName, TAG_DONE)) != NULL)
	{
		RetVal = ETrue;
		*a_pbDirectory = (EXD_IS_DIRECTORY(ExamineData)) ? ETrue : EFalse;

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
TInt Utils::LoadFile(const char *a_pccFileName, unsigned char **a_ppucBuffer)
{
	unsigned char *Buffer;
	TInt RetVal;
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
				if ((RetVal = File.Read(Buffer, Entry.iSize)) == (TInt) Entry.iSize)
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

/* Written: Wednesday 09-Mar-2011 6:27 am */
/* @param	a_pcPath Ptr to qualified path to be normalised */
/* Iterates through all the characters of a qualified path and converts any instances of the '\' */
/* directory separator charater to '/' to make parsing easier */

void Utils::NormalisePath(char *a_pcPath)
{
	TInt Index, Length;

	/* Iterate through all the characters of the path and pass any instances of '\' to '/' */

	Length = strlen(a_pcPath);

	for (Index = 0; Index < Length; ++Index)
	{
		if (a_pcPath[Index] == '\\')
		{
			a_pcPath[Index] = '/';
		}
	}
}

/* Written: Saturday 12-Mar-2011 11:00 am */
/* @param	a_pccTitle			Ptr to the title to be displayed in the mesage box */
/* @param	a_pccMessage		Ptr to the message to be displayed.  This can contain printf() style %d */
/*								specifiers that will be processed and matched with arguments pass in in the */
/*								a_oArgs varargs list */
/* @param	a_eMessageBoxType	Type of message box to displays, as specified by the TMessageBoxType enumeration */
/* @return	IDOK, IDCANCEL, IDYES or IDNO if successful, or 0 if the message box could not be opened */
/* Opens a message box to prompt the user with a question or simply to display information.  Different types */
/* of message boxes may be opened, which contain combinations of Ok, Cancel, Yes and No buttons according to */
/* the a_eMessageBoxType passed in */

TInt Utils::MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, va_list a_oArgs)
{
	char Message[512];
	TInt Result, RetVal;

	CWindow *RootWindow;

	/* See if a root window has been set by the application and if so, open the message box on */
	/* that window.  Otherwise just open it on the desktop or Workbench */

	RootWindow = CWindow::GetRootWindow();

	VSNPRINTF(Message, sizeof(Message), a_pccMessage, a_oArgs);

#ifdef __amigaos4__

	struct EasyStruct EasyStruct;

	struct TagItem Tags[] = { { ESA_Underscore, '_' }, { TAG_DONE, 0 } };

	/* Assume failure */

	RetVal = IDCANCEL;

	/* Build an EasyStruct for displaying the requester */

	EasyStruct.es_StructSize = sizeof(struct EasyStruct);
	EasyStruct.es_Flags = ESF_TAGGED;
	EasyStruct.es_Title = (char *) a_pccTitle;
	EasyStruct.es_TextFormat = Message;
	EasyStruct.es_TagList = Tags;

	/* Determine the type of requester to display, based on the type passed in */

	if (a_eMessageBoxType == EMBTOk)
	{
		EasyStruct.es_GadgetFormat = "_Ok";
	}
	else if (a_eMessageBoxType == EMBTOkCancel)
	{
		EasyStruct.es_GadgetFormat = "_Ok|_Cancel";
	}
	else if (a_eMessageBoxType == EMBTYesNo)
	{
		EasyStruct.es_GadgetFormat = "_Yes|_No";
	}
	else
	{
		EasyStruct.es_GadgetFormat = "_Yes|_No|_Cancel";
	}

	/* Display the requester */

	Result = IIntuition->EasyRequest((RootWindow) ? RootWindow->m_poWindow : NULL, &EasyStruct, NULL);

	/* And convert the result to one of the standard return values */

	if (Result >= 0)
	{
		if (a_eMessageBoxType == EMBTOk)
		{
			RetVal = IDOK;
		}
		else if (a_eMessageBoxType == EMBTOkCancel)
		{
			RetVal = (Result == 0) ? IDCANCEL : IDOK;
		}
		else if (a_eMessageBoxType == EMBTYesNo)
		{
			RetVal = (Result == 0) ? IDNO : IDYES;
		}
		else
		{
			if (Result == 0)
			{
				RetVal = IDCANCEL;
			}
			else
			{
				RetVal = (Result == 1) ? IDYES : IDNO;
			}
		}
	}
	else
	{
		IDOS->Printf(a_pccTitle);
		IDOS->Printf(": %s\n", Message);
	}

#else /* ! __amigaos4__ */

	UINT Type;

	/* Determine the type of Win32 message box to display, based on the type passed in */

	if (a_eMessageBoxType == EMBTOk)
	{
		Type = MB_OK;
	}
	else if (a_eMessageBoxType == EMBTOkCancel)
	{
		Type = MB_OKCANCEL;
	}
	else if (a_eMessageBoxType == EMBTYesNo)
	{
		Type = MB_YESNO;
	}
	else
	{
		Type = MB_YESNOCANCEL;
	}

	/* Display the message box and return the standard Win32 return code */

	RetVal = ::MessageBox((RootWindow) ? RootWindow->m_poWindow : NULL, Message, a_pccTitle, Type);

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 12-Mar-2011 6:25 pm */

TInt Utils::MessageBox(const char *a_pccTitle, const char *a_pccMessage, enum TMessageBoxType a_eMessageBoxType, ...)
{
	TBool RetVal;
	va_list Args;

	va_start(Args, a_eMessageBoxType);

	RetVal = MessageBox(a_pccTitle, a_pccMessage, a_eMessageBoxType, Args);

	va_end(Args);

	return(RetVal);
}

#ifdef __amigaos4__

// TODO: CAW - Convert this to use RDir?  Why is it OS4 only?  Comments and error messages, ExamineDir() result
//             ObtainDirContextTags() are not all required
TBool Utils::ScanDirectory(const char *a_pccDirectoryName, TBool a_bScanFiles, ScanFunc a_pfScanFunc, void *a_pvUserData)
{
	TBool Error, RetVal;
	APTR Context;
	struct ExamineData *ExamineData;

	/* Assume failure */

	Error = RetVal = EFalse;

	/* Allocate some data structures into which the information about the directory entries may be placed, */
	/* and a control structure for scanning through the directory */

	Context = IDOS->ObtainDirContextTags(EX_StringNameInput, a_pccDirectoryName,
		EX_DataFields, (EXF_DATE | EXF_PROTECTION | EXF_NAME | EXF_SIZE | EXF_TYPE), TAG_END);

	if (Context)
	{
		RetVal = ETrue;

		while ((ExamineData = IDOS->ExamineDir(Context)) != NULL)
		{
			// TODO: CAW - There was a bug with this. What about directory links?
			if ((EXD_IS_FILE(ExamineData)) || (EXD_IS_LINK(ExamineData)))
			{
				if (a_bScanFiles)
				{
					if (!(a_pfScanFunc((const char *) ExamineData->Name, a_pvUserData)))
					{
						Error = ETrue;

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
						Error = ETrue;

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

TInt Utils::SetFileDate(const char *a_pccFileName, const TEntry &a_roEntry)
{
	TInt RetVal;

#ifdef __amigaos4__

	// TODO: Return value here and for SetProtection(). Should this be in Utils?
	RetVal = KErrNone;
	IDOS->SetFileDate(a_pccFileName, &a_roEntry.iPlatformDate);

#else /* ! __amigaos4__ */

	HANDLE Handle;

	/* Assume failure */

	RetVal = KErrGeneral;

	/* Open a handle to the file and set its datestamp to that passed in */

	if ((Handle = CreateFile(a_pccFileName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL)) != INVALID_HANDLE_VALUE)
	{
		if (SetFileTime(Handle, &a_roEntry.iPlatformDate, &a_roEntry.iPlatformDate, &a_roEntry.iPlatformDate))
		{
			RetVal = KErrNone;
		}

		CloseHandle(Handle);
	}

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 18-Jul-2009 7:59 am */

TInt Utils::SetProtection(const char *a_pccFileName, TUint a_uiAttributes)
{
	TInt RetVal;

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

	// TODO: CAW - This is broken and return code should be checked
	DateTime.dat_Stamp = a_roEntry.iPlatformDate;
	DateTime.dat_StrTime = a_pcTime;
	DateTime.dat_StrDate = a_pcDate;

	if (IDOS->DateToStr(&DateTime))
	{
	}

#else /* ! __amigaos4__ */

	// TODO: CAW - Check against Amiga format + this is ugly!
	sprintf(a_pcDate, "%02d-%s-%04d", a_roEntry.iModified.DateTime().Day(), g_apccMonths[a_roEntry.iModified.DateTime().Month()],
		a_roEntry.iModified.DateTime().Year());

	sprintf(a_pcTime, "%02d:%02d", a_roEntry.iModified.DateTime().Hour(), a_roEntry.iModified.DateTime().Minute());

#endif /* __amigaos4__ */

}

/* Written: Sunday 01-Aug-2010 1:20 pm */
/* @param	a_pcString Ptr to the string to be trimmed of white space */
/* Trims white space from the start and end of a string, modifying the string that was passed in */

void Utils::TrimString(char *a_pcString)
{
	char *String, *Dest;
	int Length;

	/* Firstly determine if there is any white space at the start of the string that needs to be trimmed */

	String = a_pcString;

	while ((*String) && ((*String == ' ') || (*String == '\t')))
	{
		++String;
	}

	/* If any white space was found then we need copy copy the string over the top of the white space */
	/* to remove it */

	if (String != a_pcString)
	{
		Dest = a_pcString;

		while (*String)
		{
			*Dest++ = *String++;
		}

		*Dest = '\0';
	}

	/* Now trim any white space found at the end of the string.  To start with, ensure there are actually */
	/* characters in the string to be trimmed */

	if ((Length = strlen(a_pcString)) > 0)
	{
		/* Get a ptr to the last character of the string before the NULL terminator */

		String = (a_pcString + Length - 1);

		/* Iterate backwards until a non white space character or the beginning of the string is found */

		while ((String > a_pcString) && ((*String == ' ') || (*String == '\t')))
		{
			--String;
		}

		/* And NULL terminate the string after the non white space character */

		*(String + 1) = '\0';
	}
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
