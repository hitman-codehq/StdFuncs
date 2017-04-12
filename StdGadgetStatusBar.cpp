
#include "StdFuncs.h"
#include <string.h>
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

#ifdef QT_GUI_LIB

#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include "Qt/StdWindow.h"

#elif defined(WIN32)

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

	/* Use the Layout gadget's weighting system to make the String gadgets as small as possible */

	struct TagItem TagItem[] = { { CHILD_WeightedWidth, 0 }, { TAG_DONE, 0 } };

	/* Create a horizontal layout group into which can be placed the parts labels */

	if ((m_poGadget = (Object *) HGroupObject, LAYOUT_FixedVert, FALSE, EndGroup) != NULL)
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

			/* If all parts labels were created ok then indicate success.  On failure the individual */
			/* String gadgets used for the parts do not need to be destroyed as they have been added */
			/* to the Layout gadget representing the status bar and will be automatically destroyed */
			/* when the Layout gadget is destroyed below */

			if (Index == a_iNumParts)
			{
				RetVal = KErrNone;
				m_iNumParts = a_iNumParts;
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

	/* Create an array of ptrs into which we can place the ptrs to the parts labels in order */
	/* to access them l8r on, and create a part label for each slot in the array */

	if ((m_poPartsGadgets = new QLabel *[a_iNumParts]) != NULL)
	{
		/* Create a QLabel gadget for each part requested by the user */

		for (Index = 0; Index < a_iNumParts; ++Index)
		{
			m_poPartsGadgets[Index] = new QLabel(m_poParentWindow->m_poWindow);

			if (m_poPartsGadgets[Index])
			{
				/* Add the QLabel gadget to the QStatusBar belonging to the window */

				m_poParentWindow->m_poWindow->statusBar()->addWidget(m_poPartsGadgets[Index], a_piPartsOffsets[Index]);
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

			/* And save the dimensions of the widget for l8r */

			m_iHeight = m_poParentWindow->m_poWindow->statusBar()->height();
		}

		/* Otherwise destroy whatever labels were created.  The labels belong to the */
		/* window's status bar and will be disposed of when it is destroyed, but we don't */
		/* want them left attached because if a new CStdGadgetStatusBar gadget is l8r on */
		/* created successfully then the window will also contain these old QLabel objects */

		else
		{
			while (--Index >= 0)
			{
				delete m_poPartsGadgets[Index];
			}
		}
	}
	else
	{
		Utils::Info("CStdGadgetStatusBar::Construct() => Out of memory");
	}

	/* If anything went wrong then clean up whatever was successfully allocated */

	if (RetVal != KErrNone)
	{
		delete [] m_poPartsGadgets;
		m_poPartsGadgets = NULL;
	}

#else /* ! QT_GUI_LIB */

	TInt Offset, ParentWidth;
	RECT Rect;

	/* Register the status bar window control class so that we can use it */

	InitCommonControls();

	/* Create the underlying Windows control */

	m_poGadget = CreateWindowEx(0, STATUSCLASSNAME, NULL, (SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE),
		0, 0, 0, 0, m_poParentWindow->m_poWindow, NULL, NULL, NULL);

	if (m_poGadget)
	{
		/* Create a copy of the offset array for use if the window is resized, in which case the positions */
		/* of the parts need to be recalculated based on the new size */

		if ((m_piPartsOffsets = new TInt[a_iNumParts]) != NULL)
		{
			memcpy(m_piPartsOffsets, a_piPartsOffsets, (a_iNumParts * sizeof(TInt)));

			/* Convert the parts offsets from percentages to pixel widths as required by the underlying */
			/* Windows control */

			Offset = 0;
			ParentWidth = m_poParentWindow->InnerWidth();

			for (Index = 0; Index < a_iNumParts; ++Index)
			{
				a_piPartsOffsets[Index] = (int) (Offset + (a_piPartsOffsets[Index] / 100.0f * ParentWidth));
				Offset += a_piPartsOffsets[Index];
			}

			/* Subdivide the status bar into the requested number of parts at the requested positions */

			if (SendMessage(m_poGadget, SB_SETPARTS, a_iNumParts, (LPARAM) a_piPartsOffsets))
			{
				m_iNumParts = a_iNumParts;

				/* And determine the dimensions of the gadget and save them for l8r */

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
	}
	else
	{
		Utils::Info("CStdGadgetStatusBar::Construct() => Unable to create status bar");
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

/**
  * CStdGadgetStatusBar destructor.
  * Deletes all gadgets used for displaying the status bar, and/or the text associated
  * with them.
  *
  * @date	Monday 02-May-2011 7:21 am, Code HQ-by-Thames
  */

CStdGadgetStatusBar::~CStdGadgetStatusBar()
{
	int Index;

#ifdef __amigaos4__

	delete [] m_poPartsGadgets;

#elif defined(QT_GUI_LIB)

	/* Iterate through the parts widgets attached to the system status bar and delete them */

	for (Index = 0; Index < m_iNumParts; ++Index)
	{
		if (m_poPartsGadgets[Index])
		{
			delete m_poPartsGadgets[Index];
		}
	}

	delete [] m_poPartsGadgets;

#else /* ! QT_GUI_LIB */

	if (m_poGadget)
	{
		DEBUGCHECK((DestroyWindow(m_poGadget) != FALSE), "CStdGadgetStatusBar::~CStdGadgetStatusBar() => Cannot destroy native status bar gadget");
	}

	delete [] m_piPartsOffsets;

#endif /* ! QT_GUI_LIB */

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

/**
 * Sets the width and height of the gadget in pixels.
 * This is an override of the parent CStdGadget::SetSize() function.  It will call the same named
 * method in the parent class to perform the sizing operation and will then recalculate the
 * positions of the parts within the status bar gadget to reflect the new size.
 *
 * @date	Tuesday 23-Jun-2015 06:47 am, Code HQ Ehinger Tor
 * @param	a_iWidth		Width of the gadget in pixels
 * @param	a_iHeight		Height of the gadget in pixels
 */

void CStdGadgetStatusBar::SetSize(TInt a_iWidth, TInt a_iHeight)
{

#if defined(WIN32) && !defined(QT_GUI_LIB)

	TInt Index, Offset, ParentWidth, *PartsOffsets;
	RECT Rect;

	/* Call the parent method to perform the sizing operation */

	CStdGadget::SetSize(a_iWidth, a_iHeight);

	/* Allocate a tempoary array in which to calculate the pixel offsets of the parts */

	if ((PartsOffsets = new TInt[m_iNumParts]) != NULL)
	{
		Offset = 0;
		ParentWidth = m_poParentWindow->InnerWidth();

		/* Convert the percentage offsets of the parts into pixel offsets */

		for (Index = 0; Index < m_iNumParts; ++Index)
		{
			PartsOffsets[Index] = (TInt)(Offset + (m_piPartsOffsets[Index] / 100.0f * ParentWidth));
			Offset += PartsOffsets[Index];
		}

		/* Subdivide the status bar into the requested number of parts at the requested positions */

		if (SendMessage(m_poGadget, SB_SETPARTS, m_iNumParts, (LPARAM) PartsOffsets))
		{
			/* And determine the dimensions of the gadget and save them for l8r */

			if (GetClientRect(m_poGadget, &Rect))
			{
				m_iWidth = (Rect.right - Rect.left);
				m_iHeight = m_iMinHeight = (Rect.bottom - Rect.top);
			}
		}

		delete [] PartsOffsets;
	}

#else /* ! (defined(WIN32) && !defined(QT_GUI_LIB) */

	(void) a_iWidth;
	(void) a_iHeight;

#endif /* ! (defined(WIN32) && !defined(QT_GUI_LIB) */

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

		if ((m_ppcPartsText[a_iPart] = (char *) Utils::GetTempBuffer(m_ppcPartsText[a_iPart], (strlen(a_pccText) + 1), EFalse)) != NULL)
		{
			strcpy(m_ppcPartsText[a_iPart], a_pccText);
		}

#ifdef __amigaos4__

		IIntuition->RefreshSetGadgetAttrs((struct Gadget *) m_poPartsGadgets[a_iPart], m_poParentWindow->m_poWindow,
			NULL, STRINGA_TextVal, (ULONG *) a_pccText, TAG_DONE);

#elif defined(QT_GUI_LIB)

		m_poPartsGadgets[a_iPart]->setText(a_pccText);

#elif defined(WIN32)

		DEBUGCHECK((SendMessage(m_poGadget, SB_SETTEXT, a_iPart, (LPARAM) a_pccText) != FALSE), "CStdGadgetStatusBar::SetText() => Unable to set status bar text");

#endif /* WIN32 */

	}
}
