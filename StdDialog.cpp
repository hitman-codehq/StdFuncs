
#include "StdFuncs.h"
#include "StdDialog.h"

/* Written: Saturday 21-Aug-2010 12:21 pm */

static int CALLBACK DialogProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	int Result, RetVal;
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
			((CDialog *) a_oLParam)->m_poWindow = a_poWindow;

			RetVal = 1;
		}

		case WM_COMMAND :
		{
			/* Get a ptr to the CDialog derived class and allow it to handle the message.  If */
			/* it returns > 0 then close the dialog and use this as the dialog's return code */

			Dialog = (CDialog *) GetWindowLong(a_poWindow, GWL_USERDATA);

			if ((Result = Dialog->HandleCommand(LOWORD(a_oWParam))) > 0)
			{
				EndDialog(a_poWindow, Result);
			}

			RetVal = 1;
		}
	}

	return(RetVal);
}

/* Written: Saturday 21-Aug-2010 1:08 pm */
/* @param	a_iGadgetID	ID of the gadget for which to obtain the text */
/* @returns	ETrue if the text was obtained successfully, else EFalse */
/* Queries the gadget specified by the a_iGadgetID identifier for its text contents. */
/* No checking is performed that the ID is valid or that it refers to a text gadget */

TBool CDialog::GetGadgetText(TInt a_iGadgetID)
{
	int Length;
	TBool RetVal;

	/* Determine the length of the text held in the gadget and add space for a NULL terminator */

	Length = (GetWindowTextLength(GetDlgItem(m_poWindow, a_iGadgetID)) + 1);

	/* If the currently allocated buffer is too small then delete it */

	if (m_iTextBufferLength < Length)
	{
		delete [] m_pcTextBuffer;
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

		RetVal = (GetDlgItemText(m_poWindow, a_iGadgetID, m_pcTextBuffer, Length) > 0) ? ETrue : EFalse;
	}
	else
	{
		RetVal = EFalse;
	}

	return(RetVal);
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
