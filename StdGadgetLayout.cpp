
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdWindow.h"

// TODO: CAW - Create a StdReaction.h file
#ifdef __amigaos4__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/intuition.h>
#include <reaction/reaction.h>

#endif /* __amigaos4__ */

/* Written: Monday 11-Jul-2011 5:44 am */
// TODO: CAW - Parameters + comments all through here + ensure consistency with others

CStdGadgetLayout *CStdGadgetLayout::New(CWindow *a_poParentWindow)
{
	TInt Result;
	CStdGadgetLayout *RetVal;

	ASSERTM((a_poParentWindow != NULL), "CStdGadgetLayout::New() => Ptr to parent window must be passed in");

	if ((RetVal = new CStdGadgetLayout(a_poParentWindow)) != NULL)
	{
		if ((Result = RetVal->Construct()) != KErrNone)
		{
			Utils::Info("CStdGadgetLayout::New() => Unable to initialise layout gadget (Error %d)", Result);

			delete RetVal;
			RetVal = NULL;
		}
		else
		{
			// TODO: CAW - Move & ensure consistency with others.  Note that object MUST be
			//             attached in order for it to be destroyed when the layout is destroyed
			//             due to Reaction ownership requirements.  If it is not attached then it
			//             must be destroyed manually
			a_poParentWindow->Attach(RetVal);
		}
	}

	return(RetVal);
}

/* Written: Monday 11-Jul-2011 5:46 am */

TInt CStdGadgetLayout::Construct()
{
	TInt RetVal;

	/* Assume failure */

	RetVal = KErrNoMemory;

#ifdef __amigaos4__

	if ((m_poGadget = (Object *) VGroupObject, LAYOUT_HorizAlignment, LALIGN_RIGHT, EndGroup) != NULL)
	{
		//m_poParentWindow->Attach(this);

		RetVal = KErrNone;
	}

#else /* ! __amigaos4__ */

	RetVal = KErrNone;

#endif /* ! __amigaos4__ */

	return(RetVal);
}

/* Written: Thursday 14-Jul-2011 6:57 am */

CStdGadgetLayout::~CStdGadgetLayout()
{
	CStdGadget *Gadget;

	/* Iterate through the list of attached gadgets, remove them from the gadget list and delete them */

	while ((Gadget = m_oGadgets.RemHead()) != NULL)
	{
		delete Gadget;
	}

	/* Now remove the layout gadget from its parent window */

	m_poParentWindow->Remove(this);
}

/* Written: Monday 11-Jul-2011 6:03 am */

void CStdGadgetLayout::Attach(CStdGadget *a_poGadget)
{
	ASSERTM((a_poGadget != NULL), "CStdGadgetLayout::Attach() => No gadget to be attached passed in");

	m_oGadgets.AddTail(a_poGadget);

#ifdef __amigaos4__

	/* Add the new BOOPSI gadget to the layout */

	if (IIntuition->IDoMethod(m_poGadget, LM_ADDCHILD, NULL, a_poGadget->m_poGadget, NULL))
	{
		// TODO: CAW
		ILayout->RethinkLayout((struct Gadget *) m_poParentWindow->m_poRootGadget, m_poParentWindow->m_poWindow, NULL, TRUE);
	}

#else /* ! __amigaos4__ */

	RethinkLayout();

#endif /* ! __amigaos4__ */

}

/* Written: Saturday 15-Oct-2011 2:46 pm, CodeHQ S�flingen */
/* @param	a_pvGadget	Ptr to the native gadget to be searched for */
/* @return	A ptr to the standard gadget that contains the native gadget, */
/*			if found, else NULL */
/* This function searches through the list of the layout's attached gadgets for */
/* a particular native gadget (whether Win32 or Amiga OS).  It is an internal */
/* function designed for use when mapping native events onto standard cross */
/* platform events and should not be used by client code.  Currently, due to */
/* inconsistencies in Win32 & Amiga OS methods of representing gadgets, this */
/* function searches for the native gadget ptr on Win32 and the gadget ID on */
/* Amiga OS! */

CStdGadget *CStdGadgetLayout::FindNativeGadget(void *a_pvGadget)
{
	CStdGadget *RetVal;

	/* Get a ptr to the first gadget in the list of the layout's attached gadgets */

	if ((RetVal = m_oGadgets.GetHead()) != NULL)
	{
		/* If it was found then iterate through the list and see if any gadgets */
		/* contain the requested native gadget */

		while (RetVal)
		{

#ifdef __amigaos4__

			if ((void *) RetVal->m_iGadgetID == a_pvGadget)

#else /* ! __amigaos4__ */

			if (RetVal->m_poGadget == a_pvGadget)

#endif /* ! __amigaos4__ */

			{
				break;
			}

			RetVal = m_oGadgets.GetSucc(RetVal);
		}
	}

	return(RetVal);
}

/* Written: Saturday 15-Oct-2011 12:42 pm, CodeHQ S�flingen */

void CStdGadgetLayout::RethinkLayout()
{

#ifndef __amigaos4__

	// TODO: CAW - A VERY temporary solution
	TInt Height;
	CStdGadget *Gadget, *HorizontalSliderGadget, *StatusBarGadget;

	Gadget = m_oGadgets.GetHead();
	HorizontalSliderGadget = StatusBarGadget = NULL;

	while (Gadget)
	{
		if (Gadget->GadgetType() == EStdGadgetHorizontalSlider)
		{
			HorizontalSliderGadget = Gadget;
		}
		else if (Gadget->GadgetType() == EStdGadgetStatusBar)
		{
			StatusBarGadget = Gadget;
		}

		Gadget = m_oGadgets.GetSucc(Gadget);
	}

	Gadget = m_oGadgets.GetHead();

	while (Gadget)
	{
		if (Gadget->GadgetType() == EStdGadgetVerticalSlider)
		{
			Gadget->SetGadgetPosition(-1, m_iY);

			Height = m_iHeight;

			if (HorizontalSliderGadget)
			{
				Height -= HorizontalSliderGadget->Height();
			}

			if (StatusBarGadget)
			{
				Height -= StatusBarGadget->Height();
			}

			Gadget->SetGadgetSize(-1, Height);
		}
		else if (Gadget->GadgetType() == EStdGadgetHorizontalSlider)
		{
			if (StatusBarGadget)
			{
				Gadget->SetGadgetPosition(-1, (m_iY + m_iHeight - Gadget->Height() - StatusBarGadget->Height()));
			}
			else
			{
				Gadget->SetGadgetPosition(-1, (m_iY + m_iHeight - Gadget->Height()));
			}
		}
		else if (Gadget->GadgetType() == EStdGadgetStatusBar)
		{
			Gadget->SetGadgetPosition(-1, (m_iY + m_iHeight - Gadget->Height()));
		}

		Gadget = m_oGadgets.GetSucc(Gadget);
	}

#endif /* ! __amigaos4__ */

}

/* Written: Tuesday 18-Oct-2011 7:15 am, CodeHQ S�flingen */
/* @return	The Y position of the layout gadget */
/* Amiga OS gadgets are positioned relative to the screen but the GUI framework depends */
/* on Win32 style client area relative positions.  So for Amiga OS we need to adjust the */
/* Y position of the layout gadget to account for this. */

TInt CStdGadgetLayout::Y()
{

#ifdef __amigaos4__

	m_iY = (CStdGadget::Y() - m_poParentWindow->m_poWindow->BorderTop);

#endif /* __amigaos4__ */

	return(m_iY);
}
