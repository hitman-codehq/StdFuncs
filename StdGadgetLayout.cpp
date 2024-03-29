
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdWindow.h"

#ifdef __amigaos__

#include "StdReaction.h"

#elif defined(QT_GUI_LIB)

#include <QtWidgets/QBoxLayout>
#include "Qt/QtWindow.h"

#endif /* QT_GUI_LIB */

/* The LAYOUT_InnerSpacing attribute cannot be queried but we need to know it, so we */
/* explicitly set it to a value and use that value in CStdGadgetLayout::GetSpacing() */

#define INNER_SPACING 3

/* Amiga OS member variables pertaining to magic flicker avoidance */

TBool CStdGadgetLayout::m_bEnableRefresh = ETrue;
CStdGadgetLayout *CStdGadgetLayout::m_poRethinker = NULL;

/**
 * Creates an instance of the CStdGadgetLayout class.
 * Allocates and initialises the class, providing two phase construction.
 *
 * @date	Monday 11-Jul-2011 5:44 am
 * @param	a_poParentLayout	Pointer to the layout to which to attach the gadget.  Must not be NULL
 * @param	a_bVertical			ETrue to create a vertical layout, else EFalse for a horizontal layout
 * @param	a_poClient			The observer to call to indicate a resize operation.  May be NULL if no
 *								callbacks are required
 * @param	a_poParentWindow	Pointer to the window in which to create the gadget.  Must be NULL
 * @return	A pointer to an initialised class instance if successful, else NULL
 */

CStdGadgetLayout *CStdGadgetLayout::New(CStdGadgetLayout *a_poParentLayout, TBool a_bVertical,
	MStdGadgetLayoutObserver *a_poClient, CWindow *a_poParentWindow)
{
	TInt Result;
	CStdGadgetLayout *RetVal;

	/* The layout must always have a parent window.  Normally this is not passed in and must be obtained from the */
	/* parent layout.  However, we have to handle the special use case of the very top level layout being created */
	/* by The Framework.  This will not have a parent layout but will instead have the parent window passed in by */
	/* the CWindow class during its initialisation.  Start by doing some parameter validation */

#ifdef _DEBUG

	if (a_poParentWindow)
	{
		ASSERTM((a_poParentLayout == NULL), "CStdGadgetLayout::New() => Parent layout must be NULL");
	}
	else
	{
		ASSERTM((a_poParentLayout != NULL), "CStdGadgetLayout::New() => Parent layout must not be NULL");
	}

#endif /* _DEBUG */

	if ((RetVal = new CStdGadgetLayout(a_poParentWindow ? a_poParentWindow : a_poParentLayout->m_poParentWindow,
		a_poParentLayout, a_bVertical, a_poClient)) != NULL)
	{
		/* Normally gadgets are created and attached to the parent window or layout in a two stage process. */
		/* However, because the root layout gadget is created before the window and layout framework is in place */
		/* we need to handle this and attach the layout to the parent window in the one step.  That we are */
		/* creating the root layout gadget is indicated by the parent window being non NULL */

		if ((Result = RetVal->Construct(a_poParentWindow != NULL)) != KErrNone)
		{
			Utils::info("CStdGadgetLayout::New() => Unable to initialise layout gadget (Error %d)", Result);

			delete RetVal;
			RetVal = NULL;
		}
		else
		{
			/* A layout can be inside another layout or inside a window.  Depending on whether a parent layout */
			/* gadget was passed in, attach the new layout to the appropariate parent */

			if (a_poParentLayout)
			{
				a_poParentLayout->Attach(RetVal);
			}
			else
			{
				a_poParentWindow->Attach(RetVal);
			}
		}
	}

	return(RetVal);
}

/**
 * CStdGadgetLayout Second phase constructor.
 * This second phase constructor will create the underlying OS specific gadgets required to
 * show the layout gadget.
 *
 * @date	Monday 11-Jul-2011 5:46 am
 * @param	a_bUseParentWindow	true to attach the layout to the parent window immediately, else false
 *								to follow the usual two stage create/attach process
 * @return	KErrNone if successful, otherwise KErrNoMemory
 */

TInt CStdGadgetLayout::Construct(bool a_bUseParentWindow)
{
	TInt RetVal;

	/* Assume failure */

	RetVal = KErrNoMemory;

#ifdef __amigaos__

	(void) a_bUseParentWindow;

	TInt Orientation;

	/* Decide whether to create a vertical or horizontal layout */

	if (m_iGadgetType == EStdGadgetVerticalLayout)
	{
		Orientation = LAYOUT_VERTICAL;
	}
	else
	{
		Orientation = LAYOUT_HORIZONTAL;
	}

	if ((m_poLayout = NewObject(LAYOUT_GetClass(), NULL, LAYOUT_Orientation, Orientation,
		LAYOUT_HorizAlignment, LALIGN_RIGHT, LAYOUT_InnerSpacing, INNER_SPACING, TAG_DONE)) != 0)
	{
		RetVal = KErrNone;

		m_poGadget = m_poLayout;
	}

#elif defined(QT_GUI_LIB)

	if (m_iGadgetType == EStdGadgetVerticalLayout)
	{
		m_poLayout = new QVBoxLayout(a_bUseParentWindow ? m_poParentWindow->m_poCentralWidget : NULL);
	}
	else
	{
		m_poLayout = new QHBoxLayout(a_bUseParentWindow ? m_poParentWindow->m_poCentralWidget : NULL);
	}

	if (m_poLayout)
	{
		RetVal = KErrNone;

		/* Set the spacing between the layout and the content, and between the gadgets of the content */
		/* to be zero, so that child gadgets sit flush against one another and the edges of the layout */

		m_poLayout->setContentsMargins(0, 0, 0, 0);
		m_poLayout->setSpacing(0);
	}

#else /* ! QT_GUI_LIB */

	(void) a_bUseParentWindow;

	RetVal = KErrNone;

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/**
 * CStdGadgetLayout Destructor.
 * Destroys the underlying OS specific gadgets required by the layout gadget, destroys
 * any child gadgets attached to the layout and removes the layout from the parent window.
 *
 * @date	Thursday 14-Jul-2011 6:57 am
 */

CStdGadgetLayout::~CStdGadgetLayout()
{
	CStdGadget *Gadget;

	/* This layout gadget is now invalid, so ensure that no client status updates are called */

	m_poClient = NULL;

	/* Iterate through the list of attached layout gadgets, remove them from the list and delete them */

	while ((Gadget = m_oLayoutGadgets.remHead()) != NULL)
	{
		delete Gadget;
	}

	/* Iterate through the list of attached gadgets, remove them from the list and delete them */

	while ((Gadget = m_oGadgets.remHead()) != NULL)
	{
		delete Gadget;
	}

	/* Now remove the layout gadget from its parent layout */

	if (m_poParentLayout)
	{
		m_poParentLayout->remove(this);
	}

#ifdef QT_GUI_LIB

	/* And delete the Qt layout.  The layout is not a widget so it is not deleted by the CStdGadget */
	/* destructor */

	if (m_poLayout)
	{
		delete m_poLayout;
	}

#endif /* QT_GUI_LIB */

}

/**
 * Attach a gadget to the layout.
 * This function will attach the gadget passed in to the layout.  This will take care of adding the
 * gadget to the layout's internal list of gadgets, as well as attaching it to the underlying OS
 * specific layout system.
 *
 * @date	Monday 11-Jul-2011 6:03 am
 * @param	a_poGadget		Pointer to the gadget to attach to the layout
 */

void CStdGadgetLayout::Attach(CStdGadget *a_poGadget)
{
	ASSERTM((a_poGadget != NULL), "CStdGadgetLayout::Attach() => No gadget to be attached passed in");

	/* Add the gadget to the internal list of gadgets */

	m_oGadgets.addTail(a_poGadget);

#ifdef __amigaos__

	/* Add the new BOOPSI gadget to the layout */

	if (SetGadgetAttrs((struct Gadget *) m_poLayout, NULL, NULL,
		LAYOUT_AddChild, (ULONG) a_poGadget->m_poGadget, TAG_DONE))
	{
		if (m_bEnableRefresh)
		{
			rethinkLayout();
		}
	}

#elif defined(QT_GUI_LIB)

	/* Add the new Qt gadget to the layout.  Vertical sliders are added on the right of the layout, horizontal */
	/* sliders and status bars are added at the bottom and any other gadgets are added to the left.  Some */
	/* Framework gadgets do not have an underlying Qt widget so check that this exists before attaching it */

	if (a_poGadget->m_poGadget)
	{
		if (a_poGadget->GadgetType() == EStdGadgetVerticalSlider)
		{
			m_poLayout->addWidget(a_poGadget->m_poGadget, 0, Qt::AlignRight);
		}
		else if ((a_poGadget->GadgetType() == EStdGadgetHorizontalSlider) || (a_poGadget->GadgetType() == EStdGadgetStatusBar))
		{
			m_poLayout->addWidget(a_poGadget->m_poGadget, 0, Qt::AlignBottom);
		}
		else
		{
			m_poLayout->addWidget(a_poGadget->m_poGadget, 0, Qt::AlignLeft);
		}

		rethinkLayout();
	}

#else /* ! QT_GUI_LIB */

	rethinkLayout();

#endif /* ! QT_GUI_LIB */

}

/**
 * Attach a layout gadget to the layout.
 * This method will attach the layout gadget passed in to the layout.  This will take care of adding the
 * layout gadget to the layout's internal list of layouts, as well as attaching it to the underlying OS
 * specific layout system.
 *
 * @date	Friday 13-Aug-2021 3:06 pm, Code HQ @ Thomas's House
 * @param	a_poLayoutGadget	Pointer to the layout gadget to attach to the layout
 */

void CStdGadgetLayout::Attach(CStdGadgetLayout *a_poLayoutGadget)
{
	ASSERTM((a_poLayoutGadget != NULL), "CStdGadgetLayout::Attach() => No layout to be attached passed in");

	/* Add the layout to the internal list of layout gadgets */

	m_oLayoutGadgets.addTail(a_poLayoutGadget);

#ifdef __amigaos__

	ASSERTM((a_poLayoutGadget->m_poLayout != 0), "CStdGadgetLayout::Attach() => Layout's native implementation is NULL");

	if (SetGadgetAttrs((struct Gadget *) m_poLayout, NULL, NULL,
		LAYOUT_AddChild, (ULONG) a_poLayoutGadget->m_poLayout, TAG_DONE))
	{
		if (m_bEnableRefresh)
		{
			rethinkLayout();
		}
	}

#elif defined(QT_GUI_LIB)

	ASSERTM((a_poLayoutGadget->m_poLayout != NULL), "CStdGadgetLayout::Attach() => Layout's native implementation is NULL");

	m_poLayout->addLayout(a_poLayoutGadget->m_poLayout);
	rethinkLayout();

#else /* ! QT_GUI_LIB */

	rethinkLayout();

#endif /* ! QT_GUI_LIB */

}

/* Written: Saturday 15-Oct-2011 2:46 pm, Code HQ Soeflingen */
/* @param	a_pvGadget	Ptr to the native gadget to be searched for */
/* @return	A ptr to the standard gadget that contains the native gadget, */
/*			if found, else NULL */
/* This function searches through the list of the layout's attached gadgets for */
/* a particular native gadget (whether Windows or Amiga OS).  It is an internal */
/* function designed for use when mapping native events onto standard cross */
/* platform events and should not be used by client code.  Currently, due to */
/* inconsistencies in Windows & Amiga OS methods of representing gadgets, this */
/* function searches for the native gadget ptr on Windows and the gadget ID on */
/* Amiga OS! */

CStdGadget *CStdGadgetLayout::FindNativeGadget(void *a_pvGadget)
{
	CStdGadget *RetVal;

	/* Get a ptr to the first gadget in the list of the layout's attached gadgets */

	if ((RetVal = m_oGadgets.getHead()) != NULL)
	{
		/* If it was found then iterate through the list and see if any gadgets */
		/* contain the requested native gadget */

		while (RetVal)
		{

#ifdef __amigaos__

			if ((void *) RetVal->m_iGadgetID == a_pvGadget)

#else /* ! __amigaos__ */

			if (RetVal->m_poGadget == a_pvGadget)

#endif /* ! __amigaos__ */

			{
				break;
			}

			RetVal = m_oGadgets.getSucc(RetVal);
		}
	}

	return(RetVal);
}

/* Written: Friday 01-Jun-2012 7:11 am, Code HQ Ehinger Tor */
/* @return	The amount of spacing, in pixels, used between gadgets */
/* When a layout gadget is created, it can be instructed to layout the gadgets it */
/* owns with padding between them, or to lay them out hard up against one another. */
/* This function returns the size of the padding used, in pixels, or 0 if the gadgets */
/* are laid up against one another */

TInt CStdGadgetLayout::GetSpacing()
{

#ifdef __amigaos__

	return(INNER_SPACING);

#else /* ! __amigaos__ */

	return(0);

#endif /* ! __amigaos__ */

}

/**
 * Search for a gadget and send an update to it.
 * This method will search the tree of layout gadgets, looking for the first one
 * that contains the target gadget.  If found, an update will be sent to that gadget
 * and the search will stop.  This means that at most only one gadget will receive
 * an update.
 *
 * Note that the gadget passed in is *not* a CStdGadget derived object.  It is an
 * identifier for the CStdGadget's underlying native gadget.  For Windows, this is
 * the gadget's numeric ID and for Amiga OS this is a BOOPSI gadget.  That is because
 * this method is only meant for use by The Framework to map system events onto
 * framework events.
 *
 * @date	Sunday 15-Aug-2021 9:42 am, Code HQ @ Thomas's House
 * @param	a_pvGadget		Identifier of the native gadget to which to send the update
 * @param	a_ulData		Data to be sent with the update
 * @return	true if the update was sent, else false
 */

bool CStdGadgetLayout::SendUpdate(void *a_pvGadget, ULONG a_ulData)
{
	bool RetVal;
	CStdGadget *Gadget;
	CStdGadgetLayout *LayoutGadget;

	RetVal = false;
	LayoutGadget = m_oLayoutGadgets.getHead();

	/* Iterate through the layout's list of layouts and search for the target gadget */

	while ((LayoutGadget != NULL) && !RetVal)
	{
		if ((Gadget = LayoutGadget->FindNativeGadget(a_pvGadget)) != NULL)
		{
			/* Got it!  Let the gadget know that it has been updated */

			Gadget->Updated(a_ulData);
			RetVal = true;
		}

		/* The gadget was not found, so try sending the update to the next layout gadget */

		else
		{
			RetVal = LayoutGadget->SendUpdate(a_pvGadget, a_ulData);
		}

		LayoutGadget = m_oLayoutGadgets.getSucc(LayoutGadget);
	}

	return(RetVal);
}

#ifdef __amigaos__

/**
 * Reattaches a gadget to the layout.
 * This is an internal Amiga OS only method that will reattach a BOOPSI gadget to the layout.  It basically
 * works the same as CStdGadgetLayout::Attach() but it doesn't add the gadget to the m_oGadgets list.  It is
 * useful for implementing overrides of the CStdGadget::SetVisible() method.
 *
 * @date	Sunday 04-Jul-2021 8:04 am, Code HQ Bergmannstrasse
 * @param	a_poGadget		Pointer to the native gadget to attach to the layout
 */

void CStdGadgetLayout::ReAttach(CStdGadget *a_poGadget)
{
	ASSERTM((a_poGadget != NULL), "CStdGadgetLayout::ReAttach() => No gadget to be attached passed in");

	/* Add the BOOPSI gadget to the layout */

	// TODO: CAW (multi) - All of these need to be checked
	if (m_poParentWindow)
	{
		if (SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
			LAYOUT_AddChild, (ULONG) a_poGadget->m_poGadget, TAG_DONE))
		{
			if (m_bEnableRefresh)
			{
				rethinkLayout();
			}
		}
	}
}

#endif /* __amigaos__ */

/**
 * Remove a gadget from the layout.
 * This function will remove the gadget passed in from the layout.  This will take care of removing the
 * gadget from the layout's internal list of gadgets, as well as removing it from the underlying OS specific
 * layout system.
 *
 * @date	Friday 29-Dec-2023 5:11 pm, Code HQ @ Ashley's house
 * @param	a_poGadget		Pointer to the gadget to remove from the layout
 */

void CStdGadgetLayout::remove(CStdGadget *a_poGadget)
{
	ASSERTM((a_poGadget != NULL), "CStdGadgetLayout::remove() => No gadget to be removed passed in");

	/* Remove the layout gadget from this layout's private list of layout gadgets */

	m_oGadgets.remove(a_poGadget);

#ifdef __amigaos__

	/* And remove the BOOPSI gadget from the layout */

	if (SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
					   LAYOUT_RemoveChild, (ULONG) a_poGadget->m_poGadget, TAG_DONE))
	{
		if (m_bEnableRefresh)
		{
			rethinkLayout();
		}
	}

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	/* And rethink the layout to reflect the change.  Qt will do this via a QEvent::LayoutRequest in */
	/* its event filter so no need to do it for Qt */

	rethinkLayout();

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

}

/**
 * Remove a layout gadget from the layout.
 * This function will remove the layout gadget passed in from the layout.  This will take care of removing the
 * gadget from the layout's internal list of layouts, as well as removing it from the underlying OS specific
 * layout system.
 *
 * @date	Thursday 15-Jul-2021 7:20 am, Code HQ Bergmannstrasse
 * @param	a_poLayoutGadget	Pointer to the gadget to remove from the layout
 */

void CStdGadgetLayout::remove(CStdGadgetLayout *a_poLayoutGadget)
{
	ASSERTM((a_poLayoutGadget != NULL), "CStdGadgetLayout::remove() => No gadget to be removed passed in");

	/* Remove the layout gadget from this layout's private list of layout gadgets */

	m_oLayoutGadgets.remove(a_poLayoutGadget);

#ifdef __amigaos__

	/* And remove the BOOPSI gadget from the layout */

	if (SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
		LAYOUT_RemoveChild, (ULONG) a_poLayoutGadget->m_poGadget, TAG_DONE))
	{
		if (m_bEnableRefresh)
		{
			rethinkLayout();
		}
	}

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	/* And rethink the layout to reflect the change.  Qt will do this via a QEvent::LayoutRequest in */
	/* its event filter so no need to do it for Qt */

	rethinkLayout();

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

}

/**
 * Rethink the layout of the gadget tree.
 * Iterates through the gadgets owned by the layout, and through all of the layouts owned by the layout,
 * rethinking their size and position.
 *
 * @date	Saturday 15-Oct-2011 12:42 pm, Code HQ Soeflingen
 */

void CStdGadgetLayout::rethinkLayout()
{
	CStdGadgetLayout *LayoutGadget;

#ifdef __amigaos__

	ASSERTM((m_poParentWindow != NULL), "CStdGadgetLayout::rethinkLayout() => Layout gadget does not have a parent window");
	ASSERTM((m_poGadget != NULL), "CStdGadgetLayout::rethinkLayout() => Layout gadget not initialised");

	/* If a rethink is just starting then indicate that it is underway and trigger a native Intuition */
	/* rethink.  If a rethink is already underway then this layout is a child of another layout that */
	/* started the rethink, so a request for Intuition to rethink is redundant and would cause flicker */

	if (!m_poRethinker)
	{
		m_poRethinker = this;
		RethinkLayout((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL, TRUE);
	}

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	// TODO: CAW - A VERY temporary solution - this needs to handle both horizontal and vertical layouts
	TInt Height, InnerHeight, InnerWidth, MinHeight, RemainderHeight, Y;
	RECT Rect;
	CStdGadget *Gadget, *HorizontalSliderGadget, *StatusBarGadget;

	Y = 0;
	InnerHeight = m_iHeight;
	LayoutGadget = m_oLayoutGadgets.getHead();

	if (LayoutGadget)
	{
		while (LayoutGadget)
		{
			if (LayoutGadget->Weight() == 1)
			{
				/* CStdGadgetLayout::MinHeight() is expensive so cache the result */

				MinHeight = LayoutGadget->MinHeight();

				LayoutGadget->m_iHeight = MinHeight;
				InnerHeight -= MinHeight;
			}
			else
			{
				LayoutGadget->m_iHeight = -1;
			}

			LayoutGadget = m_oLayoutGadgets.getSucc(LayoutGadget);
		}

		/* Each vertical layout gadget will be the same height, but the last one might be slightly */
		/* larger due to division rounding, so we calculate its height slightly differently */

		Height = (m_iHeight / m_oLayoutGadgets.Count());
		RemainderHeight = (m_iHeight - (Height * (m_oLayoutGadgets.Count() - 1)));

		LayoutGadget = m_oLayoutGadgets.getHead();

		while (LayoutGadget)
		{
			LayoutGadget->m_iY = Y;
			LayoutGadget->m_iWidth = m_iWidth;

			if (LayoutGadget->Weight() == 1)
			{
				Y += LayoutGadget->MinHeight();
			}
			else if (LayoutGadget->Weight() == 50)
			{
				if (m_oLayoutGadgets.getSucc(LayoutGadget) == NULL)
				{
					LayoutGadget->m_iHeight = RemainderHeight;
				}
				else
				{
					LayoutGadget->m_iHeight = Height;
				}

				Y += Height;
			}
			else
			{
				LayoutGadget->m_iHeight = InnerHeight;
				Y += InnerHeight;
			}

			LayoutGadget = m_oLayoutGadgets.getSucc(LayoutGadget);
		}
	}

	Gadget = m_oGadgets.getHead();
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

		Gadget = m_oGadgets.getSucc(Gadget);
	}

	Gadget = m_oGadgets.getHead();
	InnerWidth = m_poParentWindow->InnerWidth();

	while (Gadget)
	{
		if (Gadget->GadgetType() == EStdGadgetVerticalSlider)
		{
			Gadget->SetPosition((InnerWidth - Gadget->Width()), m_iY);

			Height = m_iHeight;

			if (HorizontalSliderGadget)
			{
				Height -= HorizontalSliderGadget->Height();
			}

			if (StatusBarGadget)
			{
				Height -= StatusBarGadget->Height();
			}

			Gadget->SetSize(-1, Height);
		}
		else if (Gadget->GadgetType() == EStdGadgetHorizontalSlider)
		{
			if (StatusBarGadget)
			{
				Gadget->SetPosition(-1, (m_iY + m_iHeight - Gadget->Height() - StatusBarGadget->Height()));
			}
			else
			{
				Gadget->SetPosition(-1, (m_iY + m_iHeight - Gadget->Height()));
			}

			Gadget->SetSize(InnerWidth, -1);
		}
		else if (Gadget->GadgetType() == EStdGadgetStatusBar)
		{
			Gadget->SetPosition(-1, (m_iY + m_iHeight - Gadget->Height()));

			/* This is a temporary hack.  No matter what I try, setting the status bar width to anything besides */
			/* (Rect.right - Rect.left) results in disappearing status bars when resizing the window.  This only */
			/* happens with status bars that are not the bottom most one and only when resizing the window.  The */
			/* only conclusion I can come to is that status bars do not like being resized in response to a WM_SIZE */
			/* event.  In the future I will have to write my own custom status bar to avoid this but for now this */
			/* mostly works as the status bar looks mostly as it should, although it misses a little of its right */
			/* hand section.  I have wasted enough time on this nonsense!  Ugh. */

			GetWindowRect(Gadget->m_poGadget, &Rect);
			Gadget->SetSize((Rect.right - Rect.left), -1);
		}

		Gadget = m_oGadgets.getSucc(Gadget);
	}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	/* Now iterate through any child layouts that are owned by this one and request them to also perform a rethink */

	LayoutGadget = m_oLayoutGadgets.getHead();

	while (LayoutGadget)
	{
		LayoutGadget->rethinkLayout();
		LayoutGadget = m_oLayoutGadgets.getSucc(LayoutGadget);
	}

	/* If there is a client interested in getting updates, let it know that the layout gadget has been resized */

	if (m_poClient)
	{
		m_poClient->Resize();
	}

	/* If the layout that started the rethink is this one then the rethink is over, so indicate this */

	if (m_poRethinker == this)
	{
		m_poRethinker = NULL;
	}
}

/* Written: Wednesday 23-Nov-2011 6:26 am, Code HQ Soeflingen */

bool CStdGadgetLayout::SetWeight(TInt a_iWeight)
{
	/* If nothing has changed then return without doing anything.  Some older operating systems (Amiga OS3 ) */
	/* don't take care of this and gadgets will flicker if nothing has changed */

	if (a_iWeight == m_iWeight)
	{
		return false;
	}

	/* Save the gadget's new weight */

	m_iWeight = a_iWeight;

#ifdef __amigaos__

	/* For Amiga OS we also need to notify the layout gadget itself so that it */
	/* will resize itself */

	if (m_poParentWindow)
	{
		ULONG WeightTag = (m_iGadgetType == EStdGadgetHorizontalLayout) ? CHILD_WeightedWidth: CHILD_WeightedHeight;

		SetGadgetAttrs((struct Gadget *) m_poParentLayout->m_poGadget, m_poParentWindow->m_poWindow, NULL,
			LAYOUT_ModifyChild, (ULONG) m_poGadget, WeightTag, a_iWeight, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	/* This is also the case for Qt.  For Windows, we will perform the layout ourselves later on */

	m_poParentLayout->GetLayout()->setStretchFactor(GetLayout(), a_iWeight);

#endif /* QT_GUI_LIB */

	return(true);
}


/**
 * Set the input focus to this gadget.
 * Set the focus to this layout gadget, so that all keyboard input flows directly to the gadget.
 *
 * @date	Saturday 08-Apr-2023 1:38 pm, Excelsior Caffé Akihabara
 */

void CStdGadgetLayout::SetFocus()
{

#ifdef QT_GUI_LIB

	if (m_poLayout)
	{
		m_poLayout->parentWidget()->setFocus();
	}

#endif /* QT_GUI_LIB */

}

/* Written: Thursday 31-May-2012 7:16 am, Code HQ Ehinger Tor */
/* @return	The X position of the layout gadget */
/* Amiga OS gadgets are positioned relative to the screen but the GUI framework depends */
/* on Windows style client area relative positions.  So for Amiga OS we need to adjust the */
/* X position of the layout gadget to account for this. */

TInt CStdGadgetLayout::X()
{

#ifdef __amigaos__

	if (m_poParentWindow)
	{
		m_iX = (CStdGadget::X() - m_poParentWindow->m_poWindow->BorderLeft);
	}

#elif defined(QT_GUI_LIB)

	m_iX = (m_poLayout) ? m_poLayout->geometry().x() : 0;

#endif /* QT_GUI_LIB */

	return(m_iX);
}

/* Written: Tuesday 18-Oct-2011 7:15 am, Code HQ Soeflingen */
/* @return	The Y position of the layout gadget */
/* Amiga OS gadgets are positioned relative to the screen but the GUI framework depends */
/* on Windows style client area relative positions.  So for Amiga OS we need to adjust the */
/* Y position of the layout gadget to account for this. */

TInt CStdGadgetLayout::Y()
{

#ifdef __amigaos__

	if (m_poParentWindow)
	{
		m_iY = (CStdGadget::Y() - m_poParentWindow->m_poWindow->BorderTop);
	}

#elif defined(QT_GUI_LIB)

	m_iY = (m_poLayout) ? m_poLayout->geometry().y() : 0;

#endif /* QT_GUI_LIB */

	return(m_iY);
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 13-Aug-2021 4:28 pm, Code HQ @ Thomas's House
 * @param	Parameter		Description
 * @return	Return value
 */

// TODO: CAW (multi) - Merge this into CStdGadget or update function header
TInt CStdGadgetLayout::Width()
{
	TInt RetVal;

#ifdef QT_GUI_LIB

	// TODO: CAW - Double assign
	RetVal = m_iWidth = (m_poLayout) ? m_poLayout->geometry().width() : 0;

#else /* ! QT_GUI_LIB */

	RetVal = CStdGadget::Width();

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 13-Aug-2021 3:46 pm, Code HQ @ Thomas's House
 * @param	Parameter		Description
 * @return	Return value
 */

// TODO: CAW (multi) - Merge this into CStdGadget or update function header
TInt CStdGadgetLayout::Height()
{
	TInt RetVal;

#ifdef QT_GUI_LIB

	// TODO: CAW - Double assign
	RetVal = m_iHeight = (m_poLayout) ? m_poLayout->geometry().height() : 0;

#else /* ! QT_GUI_LIB */

	RetVal = CStdGadget::Height();

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/**
 * Returns the minimum height of the layout in pixels.
 * Returns the minimum height of the layout gadget, based on the minimum size of the
 * gadgets that are attached to it.
 *
 * @date	Wednesday 23-Nov-2011 6:45 am, Code HQ Soeflingen
 * @return	The minimum height of the gadget in pixels
 */

TInt CStdGadgetLayout::MinHeight()
{
	TInt RetVal;
	CStdGadget *Gadget;

	RetVal = 0;

	/* Get a ptr to the first gadget in the list of the layout's attached gadgets */

	if ((Gadget = m_oGadgets.getHead()) != NULL)
	{
		/* If it was found then iterate through the list and count up the minimum size of */
		/* all of the attached gadgets */

		while (Gadget)
		{
			RetVal += Gadget->MinHeight();

			Gadget = m_oGadgets.getSucc(Gadget);
		}
	}

	return(RetVal);
}
