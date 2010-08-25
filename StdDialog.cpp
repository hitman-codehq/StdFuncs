
#include "StdFuncs.h"
#include "StdDialog.h"

/* Written: Saturday 21-Aug-2010 12:21 pm */

static int CALLBACK DialogProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	int HiWord, LoWord, Result, RetVal, Width, Height, WindowWidth, WindowHeight;
	RECT Size;

	CDialog *Dialog;

	/* Assume we don't handle the message */

	RetVal = 0;

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

			else if ((Result = Dialog->HandleCommand(LoWord)) > 0)
			{
				EndDialog(a_poWindow, Result);
			}

			RetVal = 1;
		}
	}

	return(RetVal);
}

/* Written: Tuesday 24-Aug-2010 7:07 am */
/* @param	a_iGadgetID	ID of the gadget to be enabled or disabled */
/*			a_bEnabled	ETrue to enable the gadget, else EFalse to disable it */
/* Enables or disables a particular gadget in a dialog, specified by gadget ID. */
/* Disabling means that the gadget is greyed out and cannot be used */

void CDialog::EnableGadget(TInt a_iGadgetID, TBool a_bEnable)
{
	HWND DialogItem;

	/* Get a handle to the gadget to be enabled or disabled */

	DialogItem = GetDlgItem(m_poWindow, a_iGadgetID);
	ASSERTM(DialogItem, "CDialog::EnableGadget() => Unable to get handle to gadget");

	/* And enable or disable it */

	EnableWindow(DialogItem, a_bEnable);
}

/* Written: Saturday 21-Aug-2010 1:08 pm */
/* @param	a_iGadgetID	ID of the gadget for which to obtain the text */
/*			a_bGetText	ETrue to actually get the text, else EFalse to just get the length */
/* @returns	Length of the text if successful, else KErrNoMemory */
/* Queries the gadget specified by the a_iGadgetID identifier for its text contents */
/* and length.  Optionally, this function can obtain just the length */

TInt CDialog::GetGadgetText(TInt a_iGadgetID, TBool a_bGetText)
{
	TInt Length, RetVal;

	/* Determine the length of the text held in the gadget and add space for a NULL terminator */

	Length = (GetWindowTextLength(GetDlgItem(m_poWindow, a_iGadgetID)) + 1);

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

			if ((Length = GetDlgItemText(m_poWindow, a_iGadgetID, m_pcTextBuffer, Length)) >= 0)
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

	return(RetVal);
}

void CDialog::SetGadgetText(TInt a_iGadgetID, const char *a_pccText)
{

#ifdef _DEBUG

	ASSERTM((SetDlgItemText(m_poWindow, a_iGadgetID, a_pccText) != FALSE), "CDialog::SetGadgetText() => Unable to set gadget text");

#else /* ! _DEBUG */

	SetDlgItemText(m_poWindow, a_iGadgetID, a_pccText);

#endif /* ! _DEBUG */

}

/* Written: Saturday 21-Aug-2010 12:16 pm */
/* @param	a_iResourceID	ID of the dialog to be opened */
/* @returns A dialog specific value returned when the user selected ok; or */
/*			IDCANCEL if the dialog was closed with cancel or the escape key; or */
/*			KErrGeneral if the dialog could not be opened */

TInt CDialog::Open(TInt a_iResourceID)
{
	TInt RetVal;

	/* Open the dialog specified by the ID passed in */

	if ((RetVal = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(a_iResourceID), NULL, DialogProc, (LPARAM) this)) < 0)
	{
		RetVal = KErrGeneral;
	}

	return(RetVal);
}
