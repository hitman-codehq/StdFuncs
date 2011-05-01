
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdDialog.h"

#ifdef __amigaos4__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/intuition.h>
#include <reaction/reaction.h>
#include <string.h>

#endif /* __amigaos4__ */

#ifdef WIN32

/* Written: Saturday 21-Aug-2010 12:21 pm */

static int CALLBACK DialogProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	int HiWord, LoWord, RetVal, Width, Height, WindowWidth, WindowHeight;
	RECT Size;

	CDialog *Dialog;

	/* Assume we don't handle the message */

	RetVal = 0;

	/* Get the ptr to the C++ class associated with this window from the window word */

	Dialog = (CDialog *) GetWindowLong(a_poWindow, GWL_USERDATA);

	switch (a_uiMessage)
	{
		case WM_INITDIALOG :
		{
			/* Save the ptr to the CDialog derived class and the handle to the dialog's window */
			/* where we can get to them l8r */

			SetWindowLong(a_poWindow, GWL_USERDATA, a_oLParam);
			Dialog = (CDialog *) a_oLParam;
			Dialog->m_poWindow = a_poWindow;

			/* Determine the size of the screen and centre the dialog in the middle of it */

			Utils::GetScreenSize(&Width, &Height);
			GetWindowRect(a_poWindow, &Size);
			WindowWidth = (Size.right - Size.left);
			WindowHeight = (Size.bottom - Size.top);

			DEBUGCHECK(SetWindowPos(a_poWindow, 0, ((Width - WindowWidth) / 2), ((Height - WindowHeight) / 2), 0, 0,
				(SWP_NOZORDER | SWP_NOSIZE)), "DialogProc() => SetWindowPos() failed");

			/* Allow the concrete class to peform any dialog initialisation it needs to */

			Dialog->InitDialog();

			RetVal = 1;

			break;
		}

		case WM_DESTROY :
		{
			/* When the WM_DESTROY message is received, the Windows window has already been */
			/* destroyed so set its handle to NULL so that we do not try to destroy it again */
			/* in CWindow's destructor */

			Dialog->m_poWindow = NULL;

			RetVal = 1;

			break;
		}

		case WM_ACTIVATE :
		{
			/* A dialog has been either activated or deactivated so notify the application of */
			/* this so that it can handle dialog bound messages with IsDialogMessage() */

			Dialog->Application()->SetCurrentDialog((a_oWParam) ? a_poWindow : NULL);

			break;
		}

		case WM_COMMAND :
		{
			/* Get a ptr to the CDialog derived class and allow it to handle the message.  If */
			/* it returns > 0 then close the dialog and use this as the dialog's return code */

			Dialog = (CDialog *) GetWindowLong(a_poWindow, GWL_USERDATA);

			/* Find out whether this command refers to a notification and the ID of the gadget to */
			/* which it refers */

			HiWord = HIWORD(a_oWParam);
			LoWord = LOWORD(a_oWParam);

			/* If this is an notification then convert the notification type to a standardised value */
			/* and let the client know about it */

			if (HiWord > 0)
			{
				if (HiWord == EN_CHANGE)
				{
					Dialog->HandleEvent(EStdEventChange, LoWord);
				}
			}

			/* Otherwise it's a normal command so just let the client handle the it */

			else
			{
				Dialog->HandleCommand(LoWord);
			}

			RetVal = 1;
		}
	}

	return(RetVal);
}

#endif /* WIN32 */

/* Written: Saturday 13-Nov-2010 8:26 am */

CDialog::~CDialog()
{
	/* Ensure that the dialog's window is closed */

	Close();

	/* And free the temporary text buffer used, if allocated */

	delete [] m_pcTextBuffer;
	m_pcTextBuffer = NULL;
}

/* Written: Saturday 21-Aug-2010 12:16 pm */
/* @param	a_iResourceID	ID of the dialog to be opened */
/* @return	KErrNone if the dialog was opened successfully; or */
/*			KErrGeneral if the dialog could not be opened */

TInt CDialog::Open(TInt a_iResourceID)
{
	TInt RetVal;

	/* If the dialog is already open then just bring it to the front and activate it */

	if (m_bOpen)
	{
		RetVal = KErrNone;

		Activate();
	}
	else
	{

#ifdef __amigaos4__

		(void) a_iResourceID;

#else /* ! __amigaos4__ */

		CWindow *RootWindow;
		HWND Window;

		/* See if a root window has been set by the application and if so, open the dialog on that */
		/* window.  Otherwise just open it on the desktop */

		RootWindow = CWindow::GetRootWindow();
		Window = (RootWindow) ? RootWindow->m_poWindow : NULL;

		/* Open the dialog specified by the ID passed in */

		if ((m_poWindow = CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(a_iResourceID), Window, DialogProc, (LPARAM) this)) != NULL)
		{
			RetVal = KErrNone;

			/* And perform general postamble window opening work */

			CompleteOpen();
		}
		else

#endif /* ! __amigaos4__ */

		{
			RetVal = KErrGeneral;
		}
	}

	return(RetVal);
}

/* Written: Sunday 24-Oct-2010 2:17 pm */

void CDialog::Close(TInt a_iGadgetID)
{
	/* Call the standard close routine to actually close the dialog box */

	Close();

	/* And notify the observer, if there is one, that the dialog has been closed */

	if (m_poDialogObserver)
	{
		m_poDialogObserver->DialogClosed(this, a_iGadgetID);
	}
}

/* Written: Monday 15-Nov-2010 7:0 am */

void CDialog::Close()
{
	/* Call the superclass close to actually close the dialog */

	CWindow::Close();

#ifdef __amigaos4__

	/* And free the gadget ID -> APTR mappings */

	delete [] m_poGadgetMappings;
	m_poGadgetMappings = NULL;

#endif /* __amigaos4__ */

}

/* Written: Saturday 27-Aug-2010 10:25 am */
/* @param	a_iGadgetID	ID of the gadget to be checked */
/* Enables the checkmark on a checkbox gadget.  It is assumed that the gadget represented by */
/* the ID passed in is a valid checkbox gadget and no error checking is done to confirm this */

void CDialog::CheckGadget(TInt a_iGadgetID)
{

#ifdef __amigaos4__

	APTR Gadget;

	/* Find a ptr to the BOOPSI gadget and if found then set the state of the checkbox gadget */

	if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
	{
		IIntuition->RefreshSetGadgetAttrs((struct Gadget *) Gadget, m_poWindow, NULL, GA_Selected, TRUE, TAG_DONE);
	}

#else /* ! __amigaos4__ */

	CheckDlgButton(m_poWindow, a_iGadgetID, BST_CHECKED);

#endif /* ! __amigaos4__ */

}

/* Written: Tuesday 24-Aug-2010 7:07 am */
/* @param	a_iGadgetID	ID of the gadget to be enabled or disabled */
/*			a_bEnabled	ETrue to enable the gadget, else EFalse to disable it */
/* Enables or disables a particular gadget in a dialog, specified by gadget ID. */
/* Disabling means that the gadget is greyed out and cannot be used */

void CDialog::EnableGadget(TInt a_iGadgetID, TBool a_bEnable)
{

#ifdef __amigaos4__

	APTR Gadget;

	/* Find a ptr to the BOOPSI gadget and if found then enable or disable it */

	if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
	{
		IIntuition->RefreshSetGadgetAttrs((struct Gadget *) Gadget, m_poWindow, NULL, GA_Disabled, (!(a_bEnable)), TAG_DONE);
	}

#else /* ! __amigaos4__ */

	HWND DialogItem;

	/* Get a handle to the gadget to be enabled or disabled */

	DialogItem = GetDlgItem(m_poWindow, a_iGadgetID);
	ASSERTM(DialogItem, "CDialog::EnableGadget() => Unable to get handle to gadget");

	/* And enable or disable it */

	EnableWindow(DialogItem, a_bEnable);

#endif /* ! __amigaos4__ */

}

#ifdef __amigaos4__

/* Written: Sunday 24-Sep-2010 1:45 pm */
/* @param	a_iGadgetID	ID of the gadget to be found */
/* @return	Ptr to the BOOPSI gadget if successful, else NULL if not found */
/* Amiga OS only function that will scan the requester's list of BOOPSI gadgets for */
/* one that matches a specified ID */

APTR CDialog::GetBOOPSIGadget(TInt a_iGadgetID)
{
	TInt Index;
	APTR RetVal;

	/* Iterate through the list of gadget mappings and find the BOOPSI ptr that matches */
	/* the gadget ID */

	RetVal = NULL;

	for (Index = 0; Index < m_iNumGadgetMappings; ++Index)
	{
		if (m_poGadgetMappings[Index].m_iID == a_iGadgetID)
		{
			RetVal = m_poGadgetMappings[Index].m_poGadget;

			break;
		}
	}

	return(RetVal);
}

#endif /* __amigaos4__ */

/* Written: Saturday 27-Aug-2010 10:12 am */
/* @param	a_iGadgetID	ID of the gadget whose value to obtain */
/* @return	The value contained by the integer string gadget */
/* Queries the integer string gadget represented by the specified gadget ID and */
/* returns its value.  No error checking is done as to whether the gadget in */
/* question is actually an integer string gadget, however the function will assert */
/* if it fails to get the gadget's value */

TInt CDialog::GetGadgetInt(TInt a_iGadgetID)
{
	TInt RetVal;

#ifdef __amigaos4__

	APTR Gadget;

	/* Assume failure and use a default value */

	RetVal = 0;

	/* Find a ptr to the BOOPSI gadget and if found then get a ptr to the gadget's text and save */
	/* its length */

	if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
	{
		DEBUGCHECK((IIntuition->GetAttr(STRINGA_LongVal, Gadget, (ULONG *) &RetVal) != 0), "CDialog::GetGadgetInt() => Unable to get gadget integer");
	}

#else /* ! __amigaos4__ */

#ifdef _DEBUG

	BOOL Ok;

	RetVal = GetDlgItemInt(m_poWindow, a_iGadgetID, &Ok, FALSE);
	ASSERTM((Ok != FALSE), "CDialog::GetGadgetInt() => Unable to obtain gadget value");

#else /* ! _DEBUG */

	RetVal = GetDlgItemInt(m_poWindow, a_iGadgetID, NULL, FALSE);

#endif /* ! _DEBUG */

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Saturday 21-Aug-2010 1:08 pm */
/* @param	a_iGadgetID	ID of the gadget for which to obtain the text */
/*			a_bGetText	ETrue to actually get the text, else EFalse to just get the length */
/* @return	Length of the text if successful */
/*			KErrNotFound if the gadget with the specified ID was not found */
/*			KErrNoMemory if out of memory */
/* Queries the gadget specified by the a_iGadgetID identifier for its text contents */
/* and length.  Optionally, this function can obtain just the length.  This function gets the */
/* text into a reusable scratch buffer (m_pcTextBuffer) that is shared among all gadgets so */
/* the text is only valid until the next call to this function */

TInt CDialog::GetGadgetText(TInt a_iGadgetID, TBool a_bGetText)
{
	TInt Length, RetVal;

#ifdef __amigaos4__

	const char *Text;
	APTR Gadget;

	/* Assume failure */

	RetVal = KErrNotFound;

	/* Find a ptr to the BOOPSI gadget and if found then get a ptr to the gadget's text and save */
	/* its length */

	if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
	{
		if (IIntuition->GetAttr(STRINGA_TextVal, Gadget, (ULONG *) &Text) > 0)
		{
			RetVal = KErrNone;
			Length = (strlen(Text) + 1);
		}
	}

	if (RetVal == KErrNone)

#else /* ! __amigaos4__ */

	/* Determine the length of the text held in the gadget and add space for a NULL terminator */

	Length = (GetWindowTextLength(GetDlgItem(m_poWindow, a_iGadgetID)) + 1);

#endif /* ! __amigaos4__ */

	{
		/* If the user wants the contents of get gadget as well then obtain them */

		if (a_bGetText)
		{
			/* Assume failure */

			RetVal = KErrNoMemory;

			/* If the currently allocated buffer is too small then delete it */

			if (m_iTextBufferLength < Length)
			{
				delete [] m_pcTextBuffer;
				m_pcTextBuffer = NULL;
				m_iTextBufferLength = 0;
			}

			/* And allocate a buffer large enough */

			if (!(m_pcTextBuffer))
			{
				m_pcTextBuffer = new char[Length];
			}

			/* If the buffer was allocated successfully the get the contents of the text gadget into it */

			if (m_pcTextBuffer)
			{
				m_iTextBufferLength = Length;

#ifdef __amigaos4__

				/* For Amiga OS we already have a ptr to the text so just make a copy of it */

				strcpy(m_pcTextBuffer, Text);

#else /* ! __amigaos4__ */

				/* For Win32 we still have to obtain the text itself */

				// TODO: CAW - Make the Win32 version return KErrNotFound
				if ((Length = GetDlgItemText(m_poWindow, a_iGadgetID, m_pcTextBuffer, Length)) >= 0)

#endif /* ! __amigaos4__ */

				{
					RetVal = Length;
				}
			}
		}

		/* Otherwise just return the length of the gadget's text */

		else
		{
			RetVal = Length;
		}
	}

	return(RetVal);
}

/* Written: Saturday 27-Aug-2010 10:28 am */
/* @param	a_iGadgetID	ID of the gadget for which to obtain checked status */
/* @return	ETrue if the gadget is checked, else EFalse */
/* Examines the checkbox gadget respresented by the ID passed in to see if it is checked. */
/* It is assumed that this gadget is a valid checkbox gadget and no error checking is done */
/* to confirm this */

TBool CDialog::IsGadgetChecked(TInt a_iGadgetID)
{

#ifdef __amigaos4__

	TBool RetVal;
	ULONG Checked;
	APTR Gadget;

	/* Find a ptr to the BOOPSI gadget and if found then get the state of the checkbox gadget */

	RetVal = 0;

	if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
	{
		if (IIntuition->GetAttr(GA_Selected, Gadget, &Checked) > 0)
		{
			RetVal = Checked;
		}
	}

	return(RetVal);

#else /* ! __amigaos4__ */

	return(IsDlgButtonChecked(m_poWindow, a_iGadgetID) == BST_CHECKED);

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 24-Oct-2010 5:30 pm */

void CDialog::OfferKeyEvent(TInt a_iKey, TBool a_bKeyDown)
{
	/* If the key is the escape key being pressed down then simulate an IDCANCEL event so that */
	/* clients can close the dialog if desired */

	if ((a_iKey == STD_KEY_ESC) && (a_bKeyDown))
	{
		Close(IDCANCEL);
	}
}

/* Written: Sunday 24-Oct-2010 5:21 pm */

void CDialog::SetGadgetFocus(TInt a_iGadgetID)
{

#ifdef __amigaos4__

	APTR Gadget;

	ASSERTM((m_poRootGadget != NULL), "CDialog::SetGadgetFocus() => Root layout gadget not initialised");

	/* Find a ptr to the BOOPSI gadget and if found then get a ptr to the gadget's text and save */
	/* its length */

	if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
	{
		DEBUGCHECK(ILayout->ActivateLayoutGadget((struct Gadget *) m_poRootGadget, m_poWindow, NULL, (uint32) Gadget),
			"CDialog::SetGadgetFocus() => Unable to set focus of gadget");
	}

#else /* ! __amigaos4__ */

	DEBUGCHECK((SetFocus(GetDlgItem(m_poWindow, a_iGadgetID)) != NULL), "CDialog::SetGadgetFocus() => Unable to activate control");

#endif /* ! __amigaos4__ */

}

/* Written: Sunday 24-Oct-2010 1:00 pm */

void CDialog::SetGadgetText(TInt a_iGadgetID, const char *a_pccText)
{

#ifdef __amigaos4__

	APTR Gadget;

	/* Find a ptr to the BOOPSI gadget and if found then get a ptr to the gadget's text and save */
	/* its length */

	if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
	{
		IIntuition->RefreshSetGadgetAttrs((struct Gadget *) Gadget, m_poWindow, NULL, STRINGA_TextVal, (ULONG *) a_pccText, TAG_DONE);
	}

#else /* ! __amigaos4__ */

	DEBUGCHECK((SetDlgItemText(m_poWindow, a_iGadgetID, a_pccText) != FALSE), "CDialog::SetGadgetText() => Unable to set gadget text");

#endif /* ! __amigaos4__ */

}
