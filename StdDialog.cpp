
#include "StdFuncs.h"
#include <string.h>
#include "StdApplication.h"
#include "StdDialog.h"
#include "StdReaction.h"

#if defined(WIN32) && !defined(QT_GUI_LIB)

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
			/* If the dialog is being activated then indicate this so that the framework */
			/* can reactivate it if focus is switched away from the application and then */
			/* back again */

			if (a_oWParam)
			{
				CDialog::m_poActiveDialog = Dialog;
			}

			/* A dialog has been either activated or deactivated so notify the RApplication of */
			/* this so that it can handle dialog bound messages with IsDialogMessage().  This */
			/* is nothing to do with the above activation handling but is to do with crazy */
			/* Windows message routing and is easier done separately */

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

			break;
		}
	}

	return(RetVal);
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

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

#elif defined(QT_GUI_LIB)

		// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

		CWindow *RootWindow;
		HWND Window;

		/* See if a root window has been set by the application and if so, open the dialog on that */
		/* window.  Otherwise just open it on the desktop */

		RootWindow = CWindow::RootWindow();
		Window = (RootWindow) ? RootWindow->m_poWindow : NULL;

		/* Open the dialog specified by the ID passed in */

		if ((m_poWindow = CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(a_iResourceID), Window, DialogProc, (LPARAM) this)) != NULL)
		{
			RetVal = KErrNone;

			/* And perform general postamble window opening work */

			CompleteOpen();
		}
		else

#endif /* ! QT_GUI_LIB */

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

#if defined(WIN32) && !defined(QT_GUI_LIB)

	/* If this is the active dialog then indicate that this is no longer the case */

	if (m_poActiveDialog == this)
	{
		m_poActiveDialog = NULL;
	}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

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

#elif defined(QT_GUI_LIB)

	// TODO: CAw - Implement

#else /* ! QT_GUI_LIB */

	CheckDlgButton(m_poWindow, a_iGadgetID, BST_CHECKED);

#endif /* ! QT_GUI_LIB */

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

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	HWND DialogItem;

	/* Get a handle to the gadget to be enabled or disabled */

	DialogItem = GetDlgItem(m_poWindow, a_iGadgetID);
	ASSERTM(DialogItem, "CDialog::EnableGadget() => Unable to get handle to gadget");

	/* And enable or disable it */

	EnableWindow(DialogItem, a_bEnable);

#endif /* ! QT_GUI_LIB */

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

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement
	RetVal = 0;

#else /* ! QT_GUI_LIB */

#ifdef _DEBUG

	BOOL Ok;

	RetVal = GetDlgItemInt(m_poWindow, a_iGadgetID, &Ok, FALSE);
	ASSERTM((Ok != FALSE), "CDialog::GetGadgetInt() => Unable to obtain gadget value");

#else /* ! _DEBUG */

	RetVal = GetDlgItemInt(m_poWindow, a_iGadgetID, NULL, FALSE);

#endif /* ! _DEBUG */

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/* Written: Saturday 21-Aug-2010 1:08 pm */
/* @param	a_iGadgetID	ID of the gadget for which to obtain the text */
/*			a_bGetText	ETrue to actually get the text, else EFalse to just get the length */
/* @return	Length of the text if successful */
/*			KErrNotFound if the gadget with the specified ID was not found */
/*			KErrNoMemory if not enough memory was available */
/* Queries the gadget specified by the a_iGadgetID identifier for its text contents */
/* and length.  Optionally, this function can obtain just the length.  This function gets the */
/* text into a reusable scratch buffer (m_pcTextBuffer) that is shared among all gadgets so */
/* the text is only valid until the next call to this function */

TInt CDialog::GetGadgetText(TInt a_iGadgetID, TBool a_bGetText)
{
	TInt Length, RetVal;

	/* Assume failure */

	RetVal = KErrNotFound;
	Length = 0;

#ifdef __amigaos4__

	const char *Text;
	APTR Gadget;

	/* If the text gadget is currently being edited then extract the information directly */
	/* from the SGWork structure associated with the gadget.  It must be done like this as */
	/* the STRINGA_TextVal attribute will not be updated until after the editing is complete */

	if ((m_poEditHookData) && (m_poEditHookData->Gadget->GadgetID == a_iGadgetID))
	{
		RetVal = KErrNone;
		Length = (m_poEditHookData->NumChars + 1);
		Text = m_poEditHookData->WorkBuffer;
	}
	else
	{
		/* Find a ptr to the BOOPSI gadget and if found then get a ptr to the gadget's text and save */
		/* its length */

		if ((Gadget = GetBOOPSIGadget(a_iGadgetID)) != NULL)
		{
			if (IIntuition->GetAttr(STRINGA_TextVal, Gadget, (ULONG *) &Text) > 0)
			{
				RetVal = KErrNone;
				Length = (strlen(Text) + 1);
			}
			else
			{
				Utils::Info("CDialog::GetGadgetText() => Unable to obtain gadget text for gadget with ID %d", a_iGadgetID);
			}
		}
		else
		{
			Utils::Info("CDialog::GetGadgetText() => Unable to find gadget with ID %d", a_iGadgetID);
		}
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement
	Length = 0;

#else /* ! QT_GUI_LIB */

	HWND Gadget;

	/* Get a ptr to the Windows control to query */

	if ((Gadget = GetDlgItem(m_poWindow, a_iGadgetID)) != NULL)
	{
		/* Determine the length of the text held in the gadget and add space for a NULL terminator */

		Length = (GetWindowTextLength(Gadget) + 1);
		RetVal = KErrNone;
	}
	else
	{
		Utils::Info("CDialog::GetGadgetText() => Unable to find gadget with ID %d", a_iGadgetID);
	}

#endif /* ! QT_GUI_LIB */

	if (RetVal == KErrNone)
	{
		/* If the user wants the contents of the gadget as well then obtain them */

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
				RetVal = Length;

#elif defined(QT_GUI_LIB)

				// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

				/* For Windows we still have to obtain the text itself */

				if ((Length = GetWindowText(Gadget, m_pcTextBuffer, Length)) >= 0)
				{
					RetVal = Length;
				}
				else
				{
					RetVal = KErrNotFound;
				}

#endif /* ! QT_GUI_LIB */

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

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement
	return(EFalse);

#else /* ! QT_GUI_LIB */

	return(IsDlgButtonChecked(m_poWindow, a_iGadgetID) == BST_CHECKED);

#endif /* ! QT_GUI_LIB */

}

/* Written: Sunday 24-Oct-2010 5:30 pm */

TBool CDialog::OfferKeyEvent(TInt a_iKey, TBool a_bKeyDown)
{
	/* If the key is the escape key being pressed down then simulate an IDCANCEL event so that */
	/* clients can close the dialog if desired */

	if ((a_iKey == STD_KEY_ESC) && (a_bKeyDown))
	{
		Close(IDCANCEL);
	}

#ifdef __amigaos4__

	APTR Gadget;
	ULONG Disabled;

	/* If they key is an enter key being released then simulate an IDOK event */

	if ((a_iKey == STD_KEY_ENTER) && (!(a_bKeyDown)))
	{
		/* We only want to do this if there is an active IDOK gadget present so find */
		/* a ptr to the BOOPSI gadget and if found then query its disable state */

		if ((Gadget = GetBOOPSIGadget(IDOK)) != NULL)
		{
			if (IIntuition->GetAttr(GA_Disabled, (struct Gadget *) Gadget, &Disabled))
			{
				/* If is not disabled then send the fake IDOK command to the client */

				if (!(Disabled))
				{
					HandleCommand(IDOK);
				}
			}
		}
	}

#endif /* __amigaos4__ */

	return(EFalse);
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

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((SetFocus(GetDlgItem(m_poWindow, a_iGadgetID)) != NULL), "CDialog::SetGadgetFocus() => Unable to activate control");

#endif /* ! QT_GUI_LIB */

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

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((SetDlgItemText(m_poWindow, a_iGadgetID, a_pccText) != FALSE), "CDialog::SetGadgetText() => Unable to set gadget text");

#endif /* ! QT_GUI_LIB */

}

#ifdef __amigaos4__

/* Written: Saturday 12-Jan-2012 12:20 pm */
/* @param	a_poHook	Ptr to a standard Amiga OS Hook structure, used by all hooks */
/*			a_poWork	Ptr to a structure that contains the current state of the string gadget */
/*			a_pulMessage	Ptr to a message indicating the editing command currently in progress */
/* @returns	1 if the hook was handled, else 0 */
/* Hook function called during the editing of text gadgets.  This is a static function that simply */
/* finds the CDialog with which the gadget is associated and passes control to the dialog for */
/* processing of the hook */

TInt CDialog::StringEditHook(struct Hook *a_poHook,  struct SGWork *a_poWork, TUint *a_pulMessage)
{
	CDialog *Dialog;

	/* Get a ptr to the dialog associated with the hook */

	Dialog = (CDialog *) a_poHook->h_Data;

	/* And call the edit function in that dialog */

	return(Dialog->DoStringEditHook(a_poHook, a_poWork, a_pulMessage));
}

/* Written: Saturday 12-Jan-2012 12:21 pm */
/* @param	a_poHook	Ptr to a standard Amiga OS Hook structure, used by all hooks */
/*			a_poWork	Ptr to a structure that contains the current state of the string gadget */
/*			a_pulMessage	Ptr to a message indicating the editing command currently in progress */
/* @returns	1 if the hook was handled, else 0 */
/* Hook function called during the editing of text gadgets.  This is required to provide real */
/* time update events to client code of the framework.  If a key has been pressed then it will */
/* save a ptr to the SGWork structure in case the client queries the value of the string gadget */
/* (as the STRINGA_TextVal attribute has not yet been updated when this hook is called) and will */
/* then notify the client code of the update */

TInt CDialog::DoStringEditHook(struct Hook * /*a_poHook */,  struct SGWork *a_poWork, TUint *a_pulMessage)
{
	TInt RetVal;

	/* Only process key presses */

	if (*a_pulMessage == SGH_KEY)
	{
		/* Indicate that we processed the hook */

		RetVal = 1;

		/* Save the SGWork structure and notify the client code of the string gadget update */

		m_poEditHookData = a_poWork;
		HandleEvent(EStdEventChange, a_poWork->Gadget->GadgetID);
		m_poEditHookData = NULL;
	}
	else
	{
		/* Indicate that we did not process the hook */

		RetVal = 0;
	}

	return(RetVal);
}

#endif /*__amigaos4__ */
