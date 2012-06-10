
#include "StdFuncs.h"
#include "StdFileRequester.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/asl.h>
#include <string.h>

#endif /* __amigaos4__ */

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

/* Written: Saturday 26-Jun-2010 2:48 pm */
/* @param	a_pccFileName	Ptr to fully qualified filename with which to initialise the file */
/*							open requester.  May be NULL */
/* @param	a_bSaveAs		ETrue to prompt for a file to save, else EFalse for a file to open */
/* @return	KErrNone if a filename was requested successfully */
/*          KErrCancel if the user clicked cancel without selecting a file */
/*          KErrNoMemory if there wasn't enough memory to allocate the requester */
/*          KErrGeneral if another error occurred obtaining the file */
/* Displays a native file requester prompting the user for the name of a file to open or save. */
/* Once obtained, a ptr to this filename can be obtained by calling RFileRequester::FileName(). */
/* Note that the filename may be 0 bytes long if the user clicked ok without selecting a file. */
/* The a_pccFileName parameter can be a directory, in which case the file requester is opened */
/* pointing to this directory, or a fully qualified filename, in which case the filename is split */
/* and both parts are used in choosing which directory in which to open the requester */

TInt RFileRequester::GetFileName(const char *a_pccFileName, TBool a_bSaveAs)
{

#ifdef __amigaos4__

	TInt RetVal;
	CWindow *RootWindow;
	APTR Requester;
	struct Screen *Screen;

	/* See if a root window has been set by the application and if so, open the dialog on that */
	/* window.  Otherwise just open it on the desktop */

	// TODO: CAW - This is tedious - make a GetRootWindowWindow() or similar function
	RootWindow = CWindow::GetRootWindow();
	Screen = (RootWindow) ? CWindow::GetRootWindow()->m_poWindow->WScreen : NULL;

	/* Taglist containing the tags for laying out the requester, and their default values. */
	/* We define it here so that it can be dynamically initialised with the screen ptr */

	struct TagItem Tags[] = { { ASLFR_Screen, (LONG) Screen }, { ASLFR_TitleText, (LONG) g_accOpenText },
		{ ASLFR_DoSaveMode, a_bSaveAs }, { TAG_DONE, FALSE } };

	/* Dynamically determine the requester title to use */

	if (a_bSaveAs)
	{
		Tags[1].ti_Data = (LONG) g_accSaveText;
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

	return(RetVal);

#else /* ! __amigaos4__ */

	const char *FileName;
	BOOL GotFileName, Qualified;
	TInt Length, RetVal;
	CWindow *RootWindow;
	TEntry Entry;
	OPENFILENAME OpenFileName;

	/* Initialise the OPENFILENAME structure to display the last filename we used, if any */

	memset(&OpenFileName, 0, sizeof(OpenFileName));
	OpenFileName.lStructSize = sizeof(OpenFileName);
	OpenFileName.lpstrFile = m_acFileName;
	OpenFileName.nMaxFile = MAX_FILEREQUESTER_PATH;

	/* See if a root window has been set by the application and if so, open the dialog on that */
	/* window.  Otherwise just open it on the desktop */

	RootWindow = CWindow::GetRootWindow();
	OpenFileName.hwndOwner = (RootWindow) ? RootWindow->m_poWindow : NULL;

	/* If a qualified filename has been passed in then use it to initialise the dialog.  This */
	/* is an attempt to get Windows to open our dialog in the directory represented in the */
	/* filename passed in, but it is entirely possible that Windows will entirely ignore our */
	/* attempts to influence it as it tries to be "smart" about which directory it uses */

	if (a_pccFileName)
	{
		/* Ensure that the directory or filename actually exists.  If not then we won't try to */
		/* affect where Windows opens the dialog box */

		// TODO: CAW - We want to use Utils::IsDirectory() here + simplify this if possible
		if (Utils::GetFileInfo(a_pccFileName, &Entry) == KErrNone)
		{
			if (Entry.iIsDir)
			{
				OpenFileName.lpstrInitialDir = a_pccFileName;
				m_acFileName[0] = '\0';
			}
			else
			{
				Qualified = ((strstr(a_pccFileName, "/")) || (strstr(a_pccFileName, "\\")));

				if (Qualified)
				{
					FileName = Utils::FilePart(a_pccFileName);
					Length = (FileName - a_pccFileName + 1);

					delete [] m_pcDirectoryName;
					m_pcDirectoryName = new char[Length];

					if (m_pcDirectoryName)
					{
						strncpy(m_pcDirectoryName, a_pccFileName, Length);
						m_pcDirectoryName[Length - 1] = '\0';

						OpenFileName.lpstrInitialDir = m_pcDirectoryName;
					}
				}

				ASSERTM((strlen(a_pccFileName) < MAX_FILEREQUESTER_PATH), "RFileRequester::GetFileName() => File name passed in is too long");
				strcpy(m_acFileName, a_pccFileName);
			}
		}
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

	return(RetVal);

#endif /* ! __amigaos4__ */

}
