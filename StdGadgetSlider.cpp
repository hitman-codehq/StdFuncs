
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <intuition/icclass.h>

#elif defined(QT_GUI_LIB)

#include "StdApplication.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyle>
#include "Qt/QtWindow.h"

#endif /* QT_GUI_LIB */

/* Written: Sunday 01-May-2011 7:41 am */
/* @param	a_poParentWindow	Ptr to the window to which the gadget should be attached */
/*			a_poParentLayout	Ptr to the gadget layout that will position this gadget */
/*			a_poClient			Ptr to the client to be notified when the slider's value changes */
/*			a_bVertical			ETrue if this is a vertical slider, else EFalse */
/*			a_iGadgetID			Unique identifier of the slider gadget */
/* @return	Ptr to the newly created slider gadget if successful, else NULL */
/* Creates an intance of the slider gadget and attaches it to the parent window specified. */

CStdGadgetSlider *CStdGadgetSlider::New(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, MStdGadgetSliderObserver *a_poClient, TBool a_bVertical, TInt a_iGadgetID)
{
	CStdGadgetSlider *RetVal;

	if ((RetVal = new CStdGadgetSlider(a_bVertical, a_poParentWindow, a_poParentLayout, a_poClient, a_iGadgetID)) != NULL)
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

#ifdef __amigaos4__

	TInt Orientation;

	/* Decide whether to create a vertical or horizontal scroller and setup its dimensions accordingly */

	if (m_iGadgetType == EStdGadgetVerticalSlider)
	{
		Orientation = SORIENT_VERT;
	}
	else
	{
		Orientation = SORIENT_HORIZ;
	}

	/* Create the underlying BOOPSI gadget */

	m_poGadget = (Object *) NewObject(NULL, "scroller.gadget", GA_ID, m_iGadgetID,
		ICA_TARGET, ICTARGET_IDCMP, SCROLLER_Orientation, Orientation, TAG_DONE);

#elif defined(QT_GUI_LIB)

	QScrollBar *ScrollBar;
	Qt::Orientation Orientation;

	/* Decide whether to create a vertical or horizontal scrollbar */

	Orientation = (m_iGadgetType == EStdGadgetVerticalSlider) ? Qt::Vertical : Qt::Horizontal;

	/* Create the underlying Qt widget */

	if ((m_poGadget = ScrollBar = new QScrollBar(Orientation, m_poParentWindow->m_poCentralWidget)) != NULL)
	{
		/* And connect the valueChanged() signal to it so that we know when the slider is moved */

		QObject::connect(ScrollBar, SIGNAL(valueChanged(int)), &m_oSlider, SLOT(valueChanged(int)));

		/* Save the scrollbar's height for l8r use.  Its height will depend on whether it is */
		/* a horizontal or vertical scrollbar */

		if (m_iGadgetType == EStdGadgetVerticalSlider)
		{
			m_iWidth = m_poParentWindow->Application()->Application()->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
			m_iHeight = m_poParentLayout->Height();
		}
		else
		{
			m_iWidth = m_poParentLayout->Width();
			m_iHeight = m_poParentWindow->Application()->Application()->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
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

	if (m_poGadget)
	{
		/* And attach it to the parent window */

		m_poParentLayout->Attach(this);
	}
	else
	{
		Utils::info("CStdGadgetSlider::Construct() => Unable to create scrollbar");
	}

	return((m_poGadget) ? KErrNone : KErrNoMemory);
}

/* Written: Monday 05-Nov-2011 9:20 am, Code HQ Soeflingen */

CStdGadgetSlider::~CStdGadgetSlider()
{

#if defined(WIN32) && !defined(QT_GUI_LIB)

	if (m_poGadget)
	{
		DEBUGCHECK((DestroyWindow(m_poGadget) != FALSE), "CStdGadgetSlider::~CStdGadgetSlider() => Cannot destroy native slider gadget");
	}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

}

/* Written: Sunday 21-Nov-2010 11:01 am */
/* @param	a_ulData	For Amiga OS this is not used.  For Windows it is the type of update */
/*						this is, such as SB_THUMBTRACK etc.  For Qt, it is the position of */
/*						the scrollbar */
/* Determines the current position of the slider gadget and notifies client code about it */

void CStdGadgetSlider::Updated(ULONG a_ulData)
{
	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::Updated() => Slider gadget has not been created");

#ifdef __amigaos4__

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

/* Written: Tuesday 30-Nov-2010 7:38 am */
/* @param	The position, between 1 and (MaxRange - PageSize), to which to set the slider */
/* Manually sets the position of the slider.  The caller must be careful to take into account */
/* that the range is correctly calculated.  PageSize will have been previously set with a call */
/* to SetRange() and this represents the number of lines that can be displayed on screen at once. */
/* Therefore, the highest number that can be passed to this function as a_iPosition is the */
/* number that will represent the line number of the top line of the screen, that is to say, */
/* (MaxRange - PageSize) */

void CStdGadgetSlider::SetPosition(TInt a_iPosition)
{
	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::SetPosition() => Slider gadget has not been created");
	ASSERTM((a_iPosition >= 1), "CStdGadgetSlider::SetPosition() => a_iPosition is too small");

#ifdef __amigaos4__

	SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
		SCROLLER_Top, (a_iPosition - 1), TAG_DONE);

#elif defined(QT_GUI_LIB)

	/* Qt sliders start from 0, whereas the CStdGadgetSlider gadget starts from 1 so take this into */
	/* account.  For compatibility with the other versions, don't notify the client during this operation */

	m_bSettingValue = true;
	((QScrollBar *) m_poGadget)->setValue(a_iPosition - 1);
	m_bSettingValue = false;

#else /* ! QT_GUI_LIB */

	SCROLLINFO ScrollInfo;

	ScrollInfo.cbSize = sizeof(ScrollInfo);
	ScrollInfo.fMask = SIF_POS;
	ScrollInfo.nPos = a_iPosition;
	SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE);

#endif /* ! QT_GUI_LIB */

}

/* Written: Tuesday 23-Nov-2010 7:56 am */
/* @param	a_iPageSize	The number of lines per page;  that is, the number of lines scrolled up or */
/*						down when the page up and page down section of the slider is clicked */
/*			a_iMaxRange	The maximum number of lines that can be displayed */
/* Sets the range of the slider so that when it moves up and down it matches the top and bottom of the */
/* data being displayed on screen.  The range passed in is from 1 - a_iMaxRange so some adjustment */
/* may be required for particular GUI targets */

void CStdGadgetSlider::SetRange(TInt a_iPageSize, TInt a_iMaxRange)
{
	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::SetRange() => Slider gadget has not been created");
	ASSERTM((a_iPageSize <= a_iMaxRange), "CStdGadgetSlider::SetRange() => a_iPageSize is too large");

	/* Save the maximum position of the scroller and its page size for l8r use */

	m_iMaxRange = a_iMaxRange;
	m_iPageSize = a_iPageSize;

#ifdef __amigaos4__

	SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
		SCROLLER_Visible, a_iPageSize, SCROLLER_Total, a_iMaxRange, TAG_DONE);

#elif defined(QT_GUI_LIB)

	/* Qt does not take the page size into account automatically so we have to reduce */
	/* the range to account for this.  We do not want the slider to return values greater */
	/* than (maxrange - pagesize) */

	m_iMaxRange -= a_iPageSize;

	/* Now let Qt know the page size and range to be used */

	((QScrollBar *) m_poGadget)->setPageStep(a_iPageSize);
	((QScrollBar *) m_poGadget)->setRange(0, m_iMaxRange);

#else /* ! QT_GUI_LIB */

	SCROLLINFO ScrollInfo;

	ScrollInfo.cbSize = sizeof(ScrollInfo);
	ScrollInfo.fMask = (SIF_PAGE | SIF_RANGE);
	ScrollInfo.nPage = a_iPageSize;
	ScrollInfo.nMin = 1;
	ScrollInfo.nMax = a_iMaxRange;
	SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE);

#endif /* ! QT_GUI_LIB */

}
