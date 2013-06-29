
#include "StdFuncs.h"
#include "StdFileRequester.h"
#include "StdWindow.h"
#include <string.h>

#ifdef __amigaos4__

#include <proto/asl.h>

#elif defined(QT_GUI_LIB)

#include <QtGui/QFileDialog>
#include "Qt/StdWindow.h"

#endif /* QT_GUI_LIB */

/* Strings for the file requester's title */

static const char g_accOpenText[] = "Select file to open...";
static const char g_accSaveText[] = "Select file to save as...";

/* Written: Sunday 10-Jun-2012 2:48 pm */
/* Cleans up any memory used by the class and sets the class's state back to how it was */
/* when it was created */

void RFileRequester::Close()
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
 * and both parts are used in choosing the directory in which to open the requester
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
	TBool Qualified;
	TInt Length, RetVal;
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

					FileName = Utils::FilePart(a_pccFileName);
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

					ASSERTM((strlen(FileName) < MAX_FILEREQUESTER_PATH), "RFileRequester::GetFileName() => File name passed in is too long");
					strcpy(m_acFileName, FileName);
				}

				/* No path so just open in the current directory, displaying the filename in the requester */

				else
				{
					ASSERTM((strlen(a_pccFileName) < MAX_FILEREQUESTER_PATH), "RFileRequester::GetFileName() => File name passed in is too long");
					strcpy(m_acFileName, a_pccFileName);
				}
			}
		}
	}

	/* Get the root window on which to open the requester */

	RootWindow = CWindow::RootWindow();

#ifdef __amigaos4__

	APTR Requester;
	struct Screen *Screen;

	/* See if a root window has been set by the application and if so, open the dialog on that */
	/* window.  Otherwise just open it on the desktop */

	Screen = (RootWindow) ? RootWindow->m_poWindow->WScreen : NULL;

	/* Taglist containing the tags for laying out the requester, and their default values. */
	/* We define it here so that it can be dynamically initialised with the screen ptr */

	struct TagItem Tags[] = { { ASLFR_Screen, (ULONG) Screen }, { ASLFR_TitleText, (ULONG) g_accOpenText },
		{ ASLFR_InitialDrawer, (ULONG) "" }, { ASLFR_InitialFile, (ULONG) m_acFileName },
		{ ASLFR_DoSaveMode, a_bSaveAs }, { TAG_DONE, 0 } };

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

	if ((Requester = IAsl->AllocAslRequest(ASL_FileRequest, Tags)) != NULL)
	{
		/* And display it on the screen */

		if (IAsl->AslRequestTags(Requester, TAG_DONE))
		{
			/* Indicate success */

			RetVal = KErrNone;

			/* Save the fully qualified name of the file for l8r use */

			strcpy(m_acFileName, ((struct FileRequester *) Requester)->fr_Drawer);
			Utils::AddPart(m_acFileName, ((struct FileRequester *) Requester)->fr_File, sizeof(m_acFileName));
		}

		/* Either the user hit "Cancel" or the requester could not be displayed, so check to see */
		/* what happened */

		else
		{
			/* If DOS's last error is 0 then the user hit cancel, so indicate this.  Otherwise */
			/* return that another error occurred */

			RetVal = (IDOS->IoErr() == 0) ? KErrCancel : KErrGeneral;
		}

		/* And free the requester now that we have finished with it */

		IAsl->FreeAslRequest(Requester);
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

		strcpy(m_acFileName, File.toAscii()); // TODO: CAW - Length + strange copy/assert above?
	}
 	else
	{
		RetVal = KErrCancel;
	}

#else /* ! QT_GUI_LIB */

	BOOL GotFileName;
	OPENFILENAME OpenFileName;

	/* Initialise the OPENFILENAME structure to display the last filename we used, if any */

	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OpenFileName);
	OpenFileName.lpstrFile = m_acFileName;
	OpenFileName.nMaxFile = MAX_FILEREQUESTER_PATH;

	/* See if a root window has been set by the application and if so, open the dialog on that */
	/* window.  Otherwise just open it on the desktop */

	OpenFileName.hwndOwner = (RootWindow) ? RootWindow->m_poWindow : NULL;

	/* If a directory name has been included in the path passed in then try to open the */
	/* requester in that directory - if Windows will take any notice of our request! */

	if (DirectoryName)
	{
		OpenFileName.lpstrInitialDir = DirectoryName;
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
		Utils::Info("RFileRequester::GetFileName() => Unable to obtain filename, error = %d", RetVal);
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}
