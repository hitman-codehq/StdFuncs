
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdWindow.h"

#ifndef __amigaos4__

#include <commctrl.h>

#endif /* ! __amigaos4__ */

/* Written: Sunday 01-May-2011 7:10 am */
/* @param	a_poParentWindow	Ptr to the window to which the gadget should be attached */
/*			a_iNumParts			Number of parts to be created */
/*			a_piPartsOffsets	Ptr to array of offsets of right sides of parts */
/*			a_iGadgetID			Unique identifier of the status bar gadget */
/* @return	Ptr to the newly created status bar if successful, else NULL */
/* Creates an intance of the status bar gadget and attaches it to the parent window specified. */

CStdGadgetStatusBar *CStdGadgetStatusBar::New(CWindow *a_poParentWindow, TInt a_iNumParts, TInt *a_piPartsOffsets, TInt a_iGadgetID)
{
	CStdGadgetStatusBar *RetVal;

	if ((RetVal = new CStdGadgetStatusBar) != NULL)
	{
		if (RetVal->Create(a_poParentWindow, a_iNumParts, a_piPartsOffsets, a_iGadgetID) != KErrNone)
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/* Written: Friday 29-Apr-2011 3:45 pm */
/* @param	a_poParentWindow	Ptr to the window to which the gadget should be attached */
/*			a_iNumParts			Number of parts to be created */
/*			a_piPartsOffsets	Ptr to array of offsets of right sides of parts */
/*			a_iGadgetID			Unique identifier of the status bar gadget */
/* @return	KErrNone if successful, else KErrNoMemory */
/* Initialises an intance of the status bar gadget and attaches it to the parent window specified. */

TInt CStdGadgetStatusBar::Create(CWindow *a_poParentWindow, TInt a_iNumParts, TInt *a_piPartsOffsets, TInt a_iGadgetID)
{
	TInt RetVal;

	m_poParentWindow = a_poParentWindow;
	m_iGadgetID = a_iGadgetID;

	/* Assume failure */

	RetVal = KErrNoMemory;

#ifdef __amigaos4__

	// TODO: CAW - Implement for Amiga OS

#else /* ! __amigaos4__ */

	INITCOMMONCONTROLSEX InitCommonControls;

	/* Register the status bar window control class so that we can use it */

	InitCommonControls.dwSize = sizeof(InitCommonControls);
	InitCommonControls.dwICC = ICC_WIN95_CLASSES;

	if (InitCommonControlsEx(&InitCommonControls))
	{
		/* Create the underlying Windows control */

		m_poGadget = CreateWindowEx(0, STATUSCLASSNAME, NULL, (SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE),
			0, 0, 0, 0, a_poParentWindow->m_poWindow, NULL, NULL, NULL);

		if (m_poGadget)
		{
			/* And subdivide it into the requested number of parts */

			if (SendMessage(m_poGadget, SB_SETPARTS, a_iNumParts, (LPARAM) a_piPartsOffsets))
			{
				RetVal = KErrNone;

				m_iNumParts = a_iNumParts;
			}
			else
			{
				Utils::Info("CStdGadgetStatusBar::Create() => Unable to subdivide status bar into parts");
			}
		}
		else
		{
			Utils::Info("CStdGadgetStatusBar::Create() => Unable to create status bar");
		}
	}
	else
	{
		Utils::Info("CStdGadgetStatusBar::Create() => Unable to initialise common controls library");
	}

#endif /* ! __amigaos4__ */

	/* If the gadget was successfully created, attach it to the parent window */

	if (RetVal == KErrNone)
	{
		a_poParentWindow->Attach(this);
	}

	return(RetVal);
}

/* Written: Saturday 30-Apr-2011 8:51 am */
/* @param	a_iPart		Index of part of the status bar to which to assign the text */
/*			a_pccText	Text to assign to the status bar part */
/* Sets the content of the specified subpart of the status bar to the text passed in. */

void CStdGadgetStatusBar::SetText(TInt a_iPart, const char *a_pccText)
{

#ifdef __amigaos4__

#else /* ! __amigaos4__ */

	ASSERTM((a_iPart < m_iNumParts), "CStdGadgetStatusBar::SetText() => Part # is out of range");

	DEBUGCHECK((SendMessage(m_poGadget, SB_SETTEXT, a_iPart, (LPARAM) a_pccText) != FALSE), "CStdGadgetStatusBar::SetText() => Unable to set status bar text");

#endif /* ! __amigaos4__ */

}
