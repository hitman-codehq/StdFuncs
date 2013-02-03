
#include "StdFuncs.h"
#include <string.h>
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

#ifdef WIN32

#include <commctrl.h>

#endif /* WIN32 */

/* Written: Sunday 01-May-2011 7:10 am */
/* @param	a_poParentWindow	Ptr to the window to which the gadget should be attached */
/*			a_poParentLayout	Ptr to the gadget layout that will position this gadget */
/*			a_iNumParts			Number of parts to be created */
/*			a_piPartsOffsets	Ptr to array of sizes of parts as percentages */
/*			a_iGadgetID			Unique identifier of the status bar gadget */
/* @return	Ptr to the newly created status bar if successful, else NULL */
/* Creates an intance of the status bar gadget and attaches it to the parent window specified. */
/* The a_piPartsOffsets array is reused for calculating the sizes of the parts and so is no */
/* longer valid on return */

CStdGadgetStatusBar *CStdGadgetStatusBar::New(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout,
	TInt a_iNumParts, TInt *a_piPartsOffsets, TInt a_iGadgetID)
{
	CStdGadgetStatusBar *RetVal;

	if ((RetVal = new CStdGadgetStatusBar(a_poParentWindow, a_poParentLayout, a_iGadgetID)) != NULL)
	{
		if (RetVal->Construct(a_iNumParts, a_piPartsOffsets) != KErrNone)
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/* Written: Friday 29-Apr-2011 3:45 pm */
/* @param	a_iNumParts			Number of parts to be created */
/*			a_piPartsOffsets	Ptr to array of sizes of parts as percentages */
/* @return	KErrNone if successful, else KErrNoMemory */
/* Initialises an intance of the status bar gadget and attaches it to the parent window specified. */
/* The a_piPartsOffsets array is reused for calculating the sizes of the parts and so is no */
/* longer valid on return */

TInt CStdGadgetStatusBar::Construct(TInt a_iNumParts, TInt *a_piPartsOffsets)
{
	TInt Index, RetVal;

	/* Assume failure */

	RetVal = KErrNoMemory;

#ifdef __amigaos4__

	struct TagItem TagItem[] = { { CHILD_WeightedWidth, 0 }, { TAG_DONE, 0 } };

	/* Create a horizontal layout group into which can be placed the parts labels */

	// TODO: CAW - These tags are a bit bodgey
	if ((m_poGadget = (Object *) HGroupObject, LAYOUT_FixedVert, FALSE, LAYOUT_DeferLayout, TRUE, EndGroup) != NULL)
	{
		/* Create an array of ptrs into which we can place the ptrs to the parts labels in order */
		/* to access them l8r on, and create a part label for each slot in the array */

		if ((m_poPartsGadgets = new Object *[a_iNumParts]) != NULL)
		{
			/* Create a string gadget for each part requested by the user */

			for (Index = 0; Index < a_iNumParts; ++Index)
			{
				m_poPartsGadgets[Index] = (Object *) StringObject, GA_ReadOnly, TRUE, StringEnd;

				if (m_poPartsGadgets[Index])
				{
					/* Add the string gadget to the horizontal layout group, with the desired weighting */

					TagItem[0].ti_Data = a_piPartsOffsets[Index];
					IIntuition->IDoMethod(m_poGadget, LM_ADDCHILD, NULL, m_poPartsGadgets[Index], TagItem);
				}
				else
				{
					Utils::Info("CStdGadgetStatusBar::Construct() => Unable to create status bar label gadget");

					break;
				}
			}

			/* If all parts labels were created ok then indicate success */

			if (Index == a_iNumParts)
			{
				RetVal = KErrNone;
				m_iNumParts = a_iNumParts;
			}

			/* Otherwise destroy whatever labels were created */

			else
			{
				while (--Index >= 0)
				{
					IIntuition->DisposeObject(m_poPartsGadgets[Index]);
				}
			}
		}
		else
		{
			Utils::Info("CStdGadgetStatusBar::Construct() => Out of memory");
		}
	}
	else
	{
		Utils::Info("CStdGadgetStatusBar::Construct() => Unable to create status bar group gadget");
	}

	/* If anything went wrong then clean up whatever was successfully allocated */

	if (RetVal != KErrNone)
	{
		if (m_poGadget)
		{
			IIntuition->DisposeObject(m_poGadget);
			m_poGadget = NULL;
		}

		if (m_poPartsGadgets)
		{
			delete [] m_poPartsGadgets;
			m_poPartsGadgets = NULL;
		}
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement
	RetVal = KErrNone;

#else /* ! QT_GUI_LIB */

	TInt Offset, ParentWidth;
	INITCOMMONCONTROLSEX InitCommonControls;
	RECT Rect;

	/* Register the status bar window control class so that we can use it */

	InitCommonControls.dwSize = sizeof(InitCommonControls);
	InitCommonControls.dwICC = ICC_WIN95_CLASSES;

	if (InitCommonControlsEx(&InitCommonControls))
	{
		/* Create the underlying Windows control */

		m_poGadget = CreateWindowEx(0, STATUSCLASSNAME, NULL, (SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE),
			0, 0, 0, 0, m_poParentWindow->m_poWindow, NULL, NULL, NULL);

		if (m_poGadget)
		{
			/* Convert the parts offsets from percentages to pixel widths as required by the underlying */
			/* Windows control */

			Offset = 0;
			ParentWidth = m_poParentWindow->InnerWidth();

			for (Index = 0; Index < a_iNumParts; ++Index)
			{
				a_piPartsOffsets[Index] = (int) (Offset + (a_piPartsOffsets[Index] / 100.0f * ParentWidth));
				Offset += a_piPartsOffsets[Index];
			}

			/* And subdivide it into the requested number of parts */

			if (SendMessage(m_poGadget, SB_SETPARTS, a_iNumParts, (LPARAM) a_piPartsOffsets))
			{
				m_iNumParts = a_iNumParts;

				/* Determine the dimensions of the gadget and save them for l8r */

				if (GetClientRect(m_poGadget, &Rect))
				{
					RetVal = KErrNone;

					m_iWidth = (Rect.right - Rect.left);
					m_iHeight = m_iMinHeight = (Rect.bottom - Rect.top);
				}
				else
				{
					Utils::Info("CStdGadgetStatusBar::Construct() => Unable to determine size of status bar");
				}
			}
			else
			{
				Utils::Info("CStdGadgetStatusBar::Construct() => Unable to subdivide status bar into parts");
			}
		}
		else
		{
			Utils::Info("CStdGadgetStatusBar::Construct() => Unable to create status bar");
		}
	}
	else
	{
		Utils::Info("CStdGadgetStatusBar::Construct() => Unable to initialise common controls library");
	}

#endif /* ! QT_GUI_LIB */

	/* If the platform specific gadget was successfully created, finish the generic initialisation and */
	/* attach it to the parent layout */

	if (RetVal == KErrNone)
	{
		/* Allocate an array of ptrs to be used for storing the content of the parts */

		if ((m_ppcPartsText = new char *[a_iNumParts]) != NULL)
		{
			/* And attach the gadget */

			m_poParentLayout->Attach(this);
		}
		else
		{
			Utils::Info("CStdGadgetStatusBar::Construct() => Out of memory");
		}
	}

	return(RetVal);
}

/* Written: Monday 02-May-2011 7:21 am, CodeHQ-by-Thames */

CStdGadgetStatusBar::~CStdGadgetStatusBar()
{
	TInt Index;

#ifdef __amigaos4__

	delete [] m_poPartsGadgets;

#elif defined(__linux__)

	// TODO: CAW - Implement

#else /* ! __linux__ */

	if (m_poGadget)
	{
		DEBUGCHECK((DestroyWindow(m_poGadget) != FALSE), "CStdGadgetStatusBar::~CStdGadgetStatusBar() => Cannot destroy native status bar gadget");
	}

#endif /* ! __linux__ */

	/* Free the content of the parts, if allocated */

	if (m_ppcPartsText)
	{
		for (Index = 0; Index < m_iNumParts; ++Index)
		{
			Utils::FreeTempBuffer(m_ppcPartsText[Index]);
		}

		delete [] m_ppcPartsText;
	}
}

/* Written: Tuesday 20-Mar-2011 8:04 am, Hilton Košice */
/* @param	a_iPart		Index of part of the status bar from which to obtain the text */
/* @return	Ptr to the text from the status bar if it has been set, else an empty string */
/* Gets the content of the specified subpart of the status bar.  The text is cached so this */
/* is a very fast function to call. */

const char *CStdGadgetStatusBar::GetText(TInt a_iPart)
{
	const char *RetVal;

	ASSERTM((a_iPart < m_iNumParts), "CStdGadgetStatusBar::GetText() => Part # is out of range");
	ASSERTM((m_ppcPartsText != NULL), "CStdGadgetStatusBar::GetText() => Gadget not initialised");

	/* Retrieve the text from our saved copy, rather than relying on the underlying */
	/* operating system to give it to us */

	RetVal = (m_ppcPartsText[a_iPart]) ? m_ppcPartsText[a_iPart] : "";

	return(RetVal);
}

/* Written: Saturday 30-Apr-2011 8:51 am */
/* @param	a_iPart		Index of part of the status bar to which to assign the text */
/*			a_pccText	Text to assign to the status bar part */
/* Sets the content of the specified subpart of the status bar to the text passed in. */
/* If the old content is the same as the new content then nothing will be done, to */
/* avoid flicker */

void CStdGadgetStatusBar::SetText(TInt a_iPart, const char *a_pccText)
{
	ASSERTM((a_iPart < m_iNumParts), "CStdGadgetStatusBar::SetText() => Part # is out of range");
	ASSERTM((m_ppcPartsText != NULL), "CStdGadgetStatusBar::SetText() => Gadget not initialised");

	/* Don't do anything if the content hasn't changed */

	if (strcmp(GetText(a_iPart), a_pccText) != 0)
	{
		/* Save our own copy of the text so we don't have to depend on the underlying operating */
		/* system if we want to retrieve it later */

		if ((m_ppcPartsText[a_iPart] = (char *) Utils::GetTempBuffer(m_ppcPartsText[a_iPart], strlen(a_pccText) + 1)) != NULL)
		{
			strcpy(m_ppcPartsText[a_iPart], a_pccText);
		}

#ifdef __amigaos4__

		IIntuition->RefreshSetGadgetAttrs((struct Gadget *) m_poPartsGadgets[a_iPart], m_poParentWindow->m_poWindow,
			NULL, STRINGA_TextVal, (ULONG *) a_pccText, TAG_DONE);

#elif defined(__linux__)

		// TODO: CAW - Implement

#else /* ! __linux__ */

		DEBUGCHECK((SendMessage(m_poGadget, SB_SETTEXT, a_iPart, (LPARAM) a_pccText) != FALSE), "CStdGadgetStatusBar::SetText() => Unable to set status bar text");

#endif /* ! __linux__ */

	}
}
