
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

#ifdef __amigaos__

#include <gadgets/scroller.h>
#include <intuition/icclass.h>

#elif defined(QT_GUI_LIB)

#include "StdApplication.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyle>
#include "Qt/QtWindow.h"

#endif /* QT_GUI_LIB */

/**
 * Create a new instance of the CStdGadgetSlider class.
 * Creates an intance of the slider gadget and attaches it to the parent layout specified.
 *
 * @date	Sunday 01-May-2011 7:41 am
 * @param	a_poParentLayout	Pointer to the gadget layout that will hold this gadget
 * @param	a_bVertical			ETrue if this is a vertical slider, else EFalse
 * @param	a_poClient			Ptr to the client to be notified when the slider's value changes
 * @param	a_iGadgetID			Unique identifier of the slider gadget
 * @return	Pointer to the newly created slider gadget if successful, else NULL
 */

CStdGadgetSlider *CStdGadgetSlider::New(CStdGadgetLayout *a_poParentLayout, TBool a_bVertical,
	MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID)
{
	CStdGadgetSlider *RetVal;

	if ((RetVal = new CStdGadgetSlider(a_poParentLayout, a_bVertical, a_poClient, a_iGadgetID)) != NULL)
	{
		if (RetVal->Construct() != KErrNone)
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/* Written: Sunday 21-Nov-2010 8:08 am */
/* @return	KErrNone if successful, else KErrNoMemory */
/* Initialises an intance of the slider gadget and attaches it to the parent window specified. */

TInt CStdGadgetSlider::Construct()
{

#ifdef __amigaos__

	/* Create the underlying BOOPSI gadget */

	CreateNative();

#elif defined(QT_GUI_LIB)

	QScrollBar *ScrollBar;
	Qt::Orientation Orientation;

	/* Decide whether to create a vertical or horizontal scrollbar */

	Orientation = (m_iGadgetType == EStdGadgetVerticalSlider) ? Qt::Vertical : Qt::Horizontal;

	/* Create the underlying Qt widget */

	if ((m_poGadget = ScrollBar = new QScrollBar(Orientation)) != NULL)
	{
		/* And connect the valueChanged() signal to it so that we know when the slider is moved */

		QObject::connect(ScrollBar, SIGNAL(valueChanged(int)), &m_oSlider, SLOT(valueChanged(int)));

		/* Save the scrollbar's height for l8r use.  Its height will depend on whether it is */
		/* a horizontal or vertical scrollbar */

		if (m_iGadgetType == EStdGadgetVerticalSlider)
		{
			// TODO: CAW (multi) - What to do about hard coded values, here and below
			m_iWidth = 20; //m_poParentWindow->Application()->Application()->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
			m_iHeight = 500; //m_poParentLayout->Height();
		}
		else
		{
			m_iWidth = m_poParentLayout->Width();
			m_iHeight = 20; //m_poParentWindow->Application()->Application()->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
		}
	}

#else /* ! QT_GUI_LIB */

	DWORD Style;

	/* Decide whether to create a vertical or horizontal scroller and setup its dimensions accordingly */

	Style = (WS_CHILD | WS_VISIBLE);

	if (m_iGadgetType == EStdGadgetVerticalSlider)
	{
		Style |= SBS_VERT;
		m_iWidth = GetSystemMetrics(SM_CXVSCROLL);
		m_iHeight = m_poParentLayout->Height();
		m_iX = (m_poParentLayout->Width() - m_iWidth);
	}
	else
	{
		m_iWidth = m_poParentLayout->Width();
		m_iHeight = m_iMinHeight = GetSystemMetrics(SM_CYHSCROLL);
		m_iY = (m_poParentLayout->Height() - m_iHeight);
	}

	/* Now create the underlying Windows control */

	m_poGadget = CreateWindow("SCROLLBAR", NULL, Style, m_iX, m_iY, m_iWidth, m_iHeight,
		m_poParentWindow->m_poWindow, NULL, NULL, NULL);

#endif /* ! QT_GUI_LIB */

	/* Attach the gadget to the parent window, if it was created successfully */

	if (m_poGadget)
	{
		m_poParentLayout->Attach(this);
	}

	return((m_poGadget) ? KErrNone : KErrNoMemory);
}

/* Written: Monday 05-Nov-2011 9:20 am, Code HQ Soeflingen */

CStdGadgetSlider::~CStdGadgetSlider()
{
	/* This gadget is now invalid, so ensure that no client status updates are called */

	m_poClient = NULL;

#ifdef QT_GUI_LIB

	if (m_poGadget)
	{
		delete m_poGadget;
	}

#endif

#if defined(WIN32) && !defined(QT_GUI_LIB)

	if (m_poGadget)
	{
		DEBUGCHECK((DestroyWindow(m_poGadget) != FALSE), "CStdGadgetSlider::~CStdGadgetSlider() => Cannot destroy native slider gadget");
	}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

}

#ifdef __amigaos__

/**
 * Creates the native underlying gadget.
 * This is a convenience method, currently only for Amiga OS, used for creating the native BOOPSI scroller
 * gadget.  This is useful as it must be created from more than one place on Amiga OS.  It will also set the
 * new gadget's range and position attributes from stored values, if they have been previously set.
 *
 * @date	Monday 05-Jul-2021 7:25 am, Code HQ Bergmannstrasse
 * @return	true if the gadget was created successfully, else false
 */

bool CStdGadgetSlider::CreateNative()
{
	int Orientation;

	/* Decide whether to create a vertical or horizontal scroller */

	if (m_iGadgetType == EStdGadgetVerticalSlider)
	{
		Orientation = SORIENT_VERT;
	}
	else
	{
		Orientation = SORIENT_HORIZ;
	}

	/* Create the underlying BOOPSI gadget */

	m_poGadget = (Object *) NewObject(SCROLLER_GetClass(), NULL, GA_ID, (ULONG) m_iGadgetID,
		ICA_TARGET, ICTARGET_IDCMP, SCROLLER_Orientation, Orientation, TAG_DONE);

	/* If the range or position attributes were previously set, apply them now.  We do this using SetAttrs() to */
	/* avoid unwanted redraws causing flicker */

	if (m_poGadget)
	{
		if (m_iMaxRange != 0 && m_iPageSize != 0)
		{
			SetAttrs((struct Gadget *) m_poGadget, SCROLLER_Visible, (ULONG) m_iPageSize, SCROLLER_Total, (ULONG) m_iMaxRange, TAG_DONE);
		}

		if (m_iPosition != 1)
		{
			SetAttrs((struct Gadget *) m_poGadget, SCROLLER_Top, (ULONG) (m_iPosition - 1), TAG_DONE);
		}
	}

	return(m_poGadget != NULL);
}

#endif /* __amigaos__ */

/* Written: Sunday 21-Nov-2010 11:01 am */
/* @param	a_ulData	For Amiga OS this is not used.  For Windows it is the type of update */
/*						this is, such as SB_THUMBTRACK etc.  For Qt, it is the position of */
/*						the scrollbar */
/* Determines the current position of the slider gadget and notifies client code about it */

void CStdGadgetSlider::Updated(ULONG a_ulData)
{
	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::Updated() => Slider gadget has not been created");

#ifdef __amigaos__

	ULONG Result;

	(void) a_ulData;

	/* If there is a client interested in getting updates, determine the current value of the */
	/* proportional gadget and let the client know it */

	if (m_poClient)
	{
		GetAttr(SCROLLER_Top, m_poGadget, &Result);
		m_poClient->SliderUpdated(this, (Result + 1));
	}

#elif defined(QT_GUI_LIB)

	/* If there is a client interested in getting updates, pass the message along to it, but only if */
	/* the position request didn't originally come from the client */

	if (m_poClient && !m_bSettingValue)
	{
		m_poClient->SliderUpdated(this, a_ulData);
	}

#else /* ! QT_GUI_LIB */

	TInt Position;
	SCROLLINFO ScrollInfo;

	/* If there is a client interested in getting updates, determine the current value of the */
	/* scrollbar and let the client know it */

	if (m_poClient)
	{
		/* Determine information about the movement of the scroll box in the scrollbar */

		Position = -1;
		ScrollInfo.cbSize = sizeof(ScrollInfo);
		ScrollInfo.fMask = (SIF_PAGE | SIF_POS | SIF_TRACKPOS);

		if (GetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo))
		{
			ScrollInfo.fMask = SIF_POS;

			/* If this is a tracking update then we need to get the current tracking position */

			if (a_ulData == SB_THUMBTRACK)
			{
				/* Now make the tracking position of the scroll box the true position */

				ScrollInfo.nPos = Position = ScrollInfo.nTrackPos;
			}

			/* For lines up and down, adjust the position by one line as appropriate */

			else if (a_ulData == SB_LINEUP)
			{
				ScrollInfo.nPos = Position = (ScrollInfo.nPos - 1);
			}
			else if (a_ulData == SB_LINEDOWN)
			{
				ScrollInfo.nPos = Position = (ScrollInfo.nPos + 1);
			}

			/* For pages up and down, adjust the position by one page as appropriate */

			else if (a_ulData == SB_PAGEUP)
			{
				ScrollInfo.nPos = (ScrollInfo.nPos - ScrollInfo.nPage);

				/* Ensure that the scroller doesn't jump to the left of its minimum position */

				Position = (ScrollInfo.nPos >= 1) ? ScrollInfo.nPos : 1;
			}
			else if (a_ulData == SB_PAGEDOWN)
			{
				ScrollInfo.nPos = Position = (ScrollInfo.nPos + ScrollInfo.nPage);

				/* Ensure that the scroller doesn't jump to the right of its maximum position */

				if ((Position + m_iPageSize) > m_iMaxRange)
				{
					Position = (m_iMaxRange - m_iPageSize + 1);
				}
			}
		}

		/* If we obtained the scroll box position successfully, update the client with */
		/* the new position */

		if (Position != -1)
		{
			SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE);
			m_poClient->SliderUpdated(this, Position);
		}
	}

#endif /* ! QT_GUI_LIB */

}

/**
 * Manually sets the position of the slider.
 * The caller must be careful to take into account that the range is correctly calculated.  PageSize will have
 * been previously set with a call to SetRange() and this represents the number of lines that can be displayed
 * on screen at once.  Therefore, the highest number that can be passed to this function as a_iPosition is the
 * number that will represent the line number of the top line of the screen, that is to say, (MaxRange - PageSize).
 *
 * @date	Tuesday 30-Nov-2010 7:38 am
 * @param	The position, between 1 and (MaxRange - PageSize), to which to set the slider
 */

void CStdGadgetSlider::SetPosition(TInt a_iPosition)
{
	ASSERTM((a_iPosition >= 1), "CStdGadgetSlider::SetPosition() => a_iPosition is too small");

	/* On Amiga OS it is safe to call this when the underlying native gadget has been destroyed */

	if (!m_poGadget)
	{
		return;
	}

	m_iPosition = a_iPosition;

#ifdef __amigaos__

	// TODO: CAW (multi) - Is using the window really necessary?  Check for all usages of this function
	SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
		SCROLLER_Top, (ULONG) (a_iPosition - 1), TAG_DONE);

#elif defined(QT_GUI_LIB)

	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::SetPosition() => Slider gadget has not been created");

	/* Qt sliders start from 0, whereas the CStdGadgetSlider gadget starts from 1 so take this into */
	/* account.  For compatibility with the other versions, don't notify the client during this operation */

	m_bSettingValue = true;
	((QScrollBar *) m_poGadget)->setValue(a_iPosition - 1);
	m_bSettingValue = false;

#else /* ! QT_GUI_LIB */

	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::SetPosition() => Slider gadget has not been created");

	SCROLLINFO ScrollInfo;

	ScrollInfo.cbSize = sizeof(ScrollInfo);
	ScrollInfo.fMask = SIF_POS;
	ScrollInfo.nPos = a_iPosition;
	SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE);

#endif /* ! QT_GUI_LIB */

}

/**
 * Sets the slider's maximum range.
 * Sets the range of the slider so that when it moves up and down it matches the top and bottom of the data being
 * displayed on screen.  The range passed in is from 1 - a_iMaxRange so some adjustment may be required for
 * particular GUI targets.
 *
 * @date	Tuesday 23-Nov-2010 7:56 am
 * @param	a_iPageSize		The number of lines per page;  that is, the number of lines scrolled up or
 *							down when the page up and page down section of the slider is clicked
 *			a_iMaxRange		The maximum number of lines that can be displayed
 */

void CStdGadgetSlider::SetRange(TInt a_iPageSize, TInt a_iMaxRange)
{
	ASSERTM((a_iPageSize <= a_iMaxRange), "CStdGadgetSlider::SetRange() => a_iPageSize is too large");

	/* On Amiga OS it is safe to call this when the underlying native gadget has been destroyed */

	if (!m_poGadget)
	{
		return;
	}

	/* If nothing has changed then return without doing anything.  Some older operating systems (Amiga OS3 ) */
	/* don't take care of this and gadgets will flicker if nothing has changed */

	if ((a_iMaxRange == m_iMaxRange) && (a_iPageSize == m_iPageSize))
	{
		return;
	}

	/* Save the maximum position of the scroller and its page size for l8r use */

	m_iMaxRange = a_iMaxRange;
	m_iPageSize = a_iPageSize;

#ifdef __amigaos__

	SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow , NULL,
		SCROLLER_Visible, a_iPageSize, SCROLLER_Total, a_iMaxRange, TAG_DONE);

#elif defined(QT_GUI_LIB)

	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::SetRange() => Slider gadget has not been created");

	/* Qt does not take the page size into account automatically so we have to reduce */
	/* the range to account for this.  We do not want the slider to return values greater */
	/* than (maxrange - pagesize) */

	m_iMaxRange -= a_iPageSize;

	/* Now let Qt know the page size and range to be used */

	((QScrollBar *) m_poGadget)->setPageStep(a_iPageSize);
	((QScrollBar *) m_poGadget)->setRange(0, m_iMaxRange);

#else /* ! QT_GUI_LIB */

	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::SetRange() => Slider gadget has not been created");

	SCROLLINFO ScrollInfo;

	ScrollInfo.cbSize = sizeof(ScrollInfo);
	ScrollInfo.fMask = (SIF_PAGE | SIF_RANGE);
	ScrollInfo.nPage = a_iPageSize;
	ScrollInfo.nMin = 1;
	ScrollInfo.nMax = a_iMaxRange;
	SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE);

#endif /* ! QT_GUI_LIB */

}

#ifdef __amigaos__

/**
 * Shows or hides the gadget.
 * This Amiga OS reimplementation of CStdGadget::SetVisible() will pass the call through to the super method
 * if the gadget is to be hidden, but will recreate the underlying BOOPSI gadget if the gadget is to be shown.
 * This enables slider gadgets to be hidden and unhidden on Amiga OS, even though the underlying Reaction system
 * does not support this functionality.
 *
 * Consideration needs to be given to the gadget's position in the parent layout's list of gadgets.  It will be
 * added to the *end* of the list of gadgets, which is not necessarily the same relative position that the gadget
 * had before it was hidden.
 *
 * @date	Sunday 04-Jul-2021 8:03 am, Code HQ Bergmannstrasse
 * @param	a_bVisible		true to make gadget visible, else false to hide it
 */

void CStdGadgetSlider::SetVisible(bool a_bVisible)
{
	/* If the gadget is being made visible then we want to recreate it */

	if (a_bVisible)
	{
		/* But only if it was previously hidden and thus does not already exist */

		if (m_bHidden)
		{
			if (CreateNative())
			{
				/* Reattach it to the parent window */

				m_poParentLayout->ReAttach(this);
				m_bHidden = false;
			}
		}
	}

	/* Just pass the call through to the super method to hide the gadget */

	else
	{
		CStdGadget::SetVisible(a_bVisible);
	}
}

#endif /* __amigaos__ */
