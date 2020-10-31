
#include "StdFuncs.h"
#include "StdFileRequester.h"
#include "StdWindow.h"
#include <string.h>

#ifdef __amigaos4__

#include <proto/asl.h>

#elif defined(QT_GUI_LIB)

#include <QtWidgets/QFileDialog>
#include "Qt/QtWindow.h"

#else /* ! QT_GUI_LIB */

#undef WIN32_LEAN_AND_MEAN
#include <commdlg.h>
#include <string>

#endif /* QT_GUI_LIB */

/* Strings for the file requester's title */

static const char g_accOpenText[] = "Select file to open...";
static const char g_accSaveText[] = "Select file to save as...";

/* Written: Sunday 10-Jun-2012 2:48 pm */
/* Cleans up any memory used by the class and sets the class's state back to how it was */
/* when it was created */

void RFileRequester::close()
{
	delete [] m_pcDirectoryName;
	m_pcDirectoryName = NULL;

	m_acFileName[0] = '\0';
}

/**
 * Displays a native file requester prompting the user for the name of a file to open or save.
 * Once obtained, a ptr to this filename can be obtained by calling RFileRequester::FileName().
 * Note that the filename may be 0 bytes long if the user clicked ok without selecting a file.
 * The a_pccFileName parameter can be a directory, in which case the file requester is opened
 * pointing to this directory, or a fully qualified filename, in which case the filename is split
 * and both parts are used in choosing the directory in which to open the requester.
 *
 * When saving a file, this function will prompt the user to confirm the overwrite of the file if
 * it exists.  Similarly, when either loading or saving a file, if an empty filename is selected
 * then this function will display an error and will now allow the user to continue until they
 * have selected a valid filename.
 *
 * @date	Saturday 26-Jun-2010 2:48 pm
 * @param	a_pccFileName	Ptr to fully qualified filename with which to initialise the file
 *							open requester.  May be NULL
 * @param	a_bSaveAs		ETrue to prompt for a file to save, else EFalse for a file to open
 * @return	KErrNone if a filename was requested successfully
 * @return	KErrCancel if the user clicked cancel without selecting a file
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if another error occurred obtaining the file
 */

TInt RFileRequester::GetFileName(const char *a_pccFileName, TBool a_bSaveAs)
{
	const char *DirectoryName, *FileName;
	size_t Length;
	TBool Qualified;
	TInt RetVal;
	CWindow *RootWindow;
	TEntry Entry;

	/* If a qualified filename has been passed in that contains a directory then use it to */
	/* initialise the requester so that it opens in that directory */

	DirectoryName = NULL;

	if (a_pccFileName)
	{
		/* Ensure that the directory or filename actually exists.  If not then we won't try to */
		/* affect where we open the dialog box */

		if (Utils::GetFileInfo(a_pccFileName, &Entry) == KErrNone)
		{
			/* It's a directory so open the requester there and signal that there is no filename */

			if (Entry.iIsDir)
			{
				DirectoryName = a_pccFileName;
				m_acFileName[0] = '\0';
			}
			else
			{
				/* Otherwise see if the filename passed in is a qualified path.  If so then we */
				/* will split it into a directory in which to open the requester, and a filename */
				/* that can be displayed in the requester */

				Qualified = ((strstr(a_pccFileName, "/")) || (strstr(a_pccFileName, "\\")) || (strstr(a_pccFileName, ":")));

				if (Qualified)
				{
					/* Separate the directory part of the qualified filename from the file part */

					FileName = Utils::filePart(a_pccFileName);
					Length = (FileName - a_pccFileName + 1);

					/* Allocate enough memory to hold the directory part and copy the directory */
					/* name into it */

					delete [] m_pcDirectoryName;
					m_pcDirectoryName = new char[Length];

					if (m_pcDirectoryName)
					{
						strncpy(m_pcDirectoryName, a_pccFileName, Length);
						m_pcDirectoryName[Length - 1] = '\0';

						/* And indicate that we wish the requester to use this as the initial */
						/* directory in which it opens */

						DirectoryName = m_pcDirectoryName;
					}

					/* Display the filename part in the requester */

					ASSERTM((strlen(FileName) < MAX_FILEREQUESTER_PATH), "RFileRequester::GetFileName() => Filename passed in is too long");
					strcpy(m_acFileName, FileName);
				}

				/* No path so just open in the current directory, displaying the filename in the requester */

				else
				{
					ASSERTM((strlen(a_pccFileName) < MAX_FILEREQUESTER_PATH), "RFileRequester::GetFileName() => Filename passed in is too long");
					strcpy(m_acFileName, a_pccFileName);
				}
			}
		}
	}

	/* Get the root window on which to open the requester */

	RootWindow = CWindow::RootWindow();

#ifdef __amigaos4__

	char *Drawer, *QualifiedFileName;
	TInt QualifiedFileNameLength;
	APTR Requester;
	struct Screen *Screen;

	/* See if a root window has been set by the application and if so, open the dialog on that */
	/* window.  Otherwise just open it on the desktop */

	Screen = (RootWindow) ? RootWindow->m_poWindow->WScreen : NULL;

	/* Taglist containing the tags for laying out the requester, and their default values. */
	/* We define it here so that it can be dynamically initialised with the screen ptr */

	struct TagItem Tags[] = { { ASLFR_Screen, (ULONG) Screen }, { ASLFR_TitleText, (ULONG) g_accOpenText },
		{ ASLFR_InitialDrawer, (ULONG) "" }, { ASLFR_InitialFile, (ULONG) m_acFileName },
		{ ASLFR_DoSaveMode, (ULONG) a_bSaveAs }, { TAG_DONE, 0 } };

	/* Dynamically determine the requester title to use */

	if (a_bSaveAs)
	{
		Tags[1].ti_Data = (ULONG) g_accSaveText;
	}

	/* If a directory name has been included in the path passed in then try to open the */
	/* requester in that directory */

	if (DirectoryName)
	{
		Tags[2].ti_Data = (ULONG) DirectoryName;
	}

	/* Allocate an ASL file requester */

	if ((Requester = AllocAslRequest(ASL_FileRequest, Tags)) != NULL)
	{
		/* Display the requester as many times as it takes to get a valid filename or the user hits cancel */

		QualifiedFileName = NULL;

		do
		{
			/* And display it on the screen */

			if (AslRequestTags(Requester, TAG_DONE))
			{
				TEntry Entry;

				/* Assume failure */

				RetVal = KErrCancel;

				/* Determine the fully qualified filename of the file that was just selected so that we can determine */
				/* whether or not it is valid */

				Drawer = ((struct FileRequester *) Requester)->fr_Drawer;
				FileName = ((struct FileRequester *) Requester)->fr_File;
				QualifiedFileNameLength = (strlen(Drawer) + 1 + strlen(FileName) + 1);
				QualifiedFileName = (char *) Utils::GetTempBuffer(QualifiedFileName, QualifiedFileNameLength, EFalse);

				if (QualifiedFileName)
				{
					strcpy(QualifiedFileName, Drawer);
					Utils::addPart(QualifiedFileName, FileName, QualifiedFileNameLength);

					/* Filenames of length 0 are now allowed */

					if (strlen(FileName) == 0)
					{
						Utils::MessageBox(EMBTOk, "Warning", "Please choose a valid filename");
					}

					/* If we are in saving mode and the file already exists then confirm that it is Ok to overwrite it */

					else if ((a_bSaveAs) && (Utils::GetFileInfo(QualifiedFileName, &Entry) == KErrNone))
					{
						RetVal = Utils::MessageBox(EMBTYesNo, "Warning", "File already exists, overwrite?");

						if (RetVal == IDYES)
						{
							RetVal = KErrNone;
						}
					}

					/* All checks have passed so indicate that it is ok to use the name */

					else
					{
						RetVal = KErrNone;
					}

					/* If we have a valid filename then save the fully qualified name of the file for l8r use */

					if (RetVal == KErrNone)
					{
						strcpy(m_acFileName, QualifiedFileName);
					}
				}
				else
				{
					/* Out of memory, so indicate this and exit the loop */

					RetVal = KErrNoMemory;

					break;
				}
			}

			/* Either the user hit "Cancel" or the requester could not be displayed, so check to see */
			/* what happened */

			else
			{
				/* If DOS's last error is 0 then the user hit cancel, so indicate this.  Otherwise */
				/* return that another error occurred */

				RetVal = (IoErr() == 0) ? KErrCancel : KErrGeneral;

				break;
			}
		}
		while (RetVal != KErrNone);

		/* And free the temporary buffer and ASL requester now that we have finished with them */

		Utils::FreeTempBuffer(QualifiedFileName);
		FreeAslRequest(Requester);
	}
	else
	{
		RetVal = KErrNoMemory;
	}

#elif defined(QT_GUI_LIB)

	QString Directory, File;
	QWidget *Parent;

	/* See if a root window has been set by the application and if so, open the dialog on that */
	/* window.  Otherwise just open it on the desktop */

	Parent = (RootWindow) ? RootWindow->m_poWindow : NULL;

 	/* If a directory name has been included in the path passed in then try to open the */
	/* requester in that directory */

	Directory = (DirectoryName) ? DirectoryName : "";

	/* Request a filename from the user using the open or save requester as appropriate */

	if (a_bSaveAs)
	{
		File = QFileDialog::getSaveFileName(Parent, g_accSaveText, Directory, "*");
	}
	else
	{
		File = QFileDialog::getOpenFileName(Parent, g_accOpenText, Directory, "*");
	}

	if (File.length() > 0)
	{
		RetVal = KErrNone;

		/* Save the filename (which is fully qualified) for l8r use */

		strcpy(m_acFileName, File.toLatin1()); // TODO: CAW - Length + strange copy/assert above?
	}
 	else
	{
		RetVal = KErrCancel;
	}

#else /* ! QT_GUI_LIB */

	size_t Index;
	BOOL GotFileName;
	std::string StartDirectory;
	OPENFILENAME OpenFileName;

	/* Initialise the OPENFILENAME structure to display the last filename we used, if any */

	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OpenFileName);
	OpenFileName.lpstrFile = m_acFileName;
	OpenFileName.nMaxFile = MAX_FILEREQUESTER_PATH;
	OpenFileName.Flags = OFN_OVERWRITEPROMPT;

	/* See if a root window has been set by the application and if so, open the dialog on that */
	/* window.  Otherwise just open it on the desktop */

	OpenFileName.hwndOwner = (RootWindow) ? RootWindow->m_poWindow : NULL;

	/* If a directory name has been included in the path passed in then try to open the */
	/* requester in that directory.  For this, the directory needs to be specified using */
	/* backslashes, even though forward slashes work just fine everywhere else in Windows! */
	/* So create a temporary path string using backslashes and use that */

	if (DirectoryName)
	{
		StartDirectory = DirectoryName;

		for (Index = 0; Index < StartDirectory.length(); ++Index)
		{
			if (StartDirectory[Index] == '/')
			{
				StartDirectory[Index] = '\\';
			}
		}

		/* Use the newly created path for the start directory */

		OpenFileName.lpstrInitialDir = StartDirectory.c_str();
	}

	/* Query the user for the filename */

	if (a_bSaveAs)
	{
		OpenFileName.lpstrTitle = g_accSaveText;
		GotFileName = GetSaveFileName(&OpenFileName);
	}
	else
	{
		OpenFileName.lpstrTitle = g_accOpenText;
		GotFileName = GetOpenFileName(&OpenFileName);
	}

	/* Determine whether the filename was successfully obtained, the dialog was closed by the user */
	/* or if an error occurred */

	if (GotFileName)
	{
		RetVal = KErrNone;
	}
	else
	{
		RetVal = (CommDlgExtendedError() == 0) ? KErrCancel : KErrGeneral;
	}

	/* If an error occurred, output a trace warning */

	if ((RetVal != KErrNone) && (RetVal != KErrCancel))
	{
		Utils::info("RFileRequester::GetFileName() => Unable to obtain filename, error = %d", RetVal);
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}
