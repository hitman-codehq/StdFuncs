
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/intuition.h>
#include <intuition/icclass.h>
#include <reaction/reaction.h>

#endif /* __amigaos4__ */

/* Written: Sunday 21-Nov-2010 8:08 am */
/* @param	a_poParentWindow	Ptr to the window to which the gadget should be attached */
/*			a_poClient			Ptr to the client to be notified when the slider's value changes */
/*			a_iGadgetID			Unique identifier of the slider gadget */
/* Creates an intance of the slider gadget and attaches it to the parent window specified. */

TInt CStdGadgetSlider::Create(CWindow *a_poParentWindow, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID)
{
	m_poParentWindow = a_poParentWindow;
	m_poClient = a_poClient;
	m_iGadgetID = a_iGadgetID;

#ifdef __amigaos4__

	/* Create the underlying BOOPSI gadget */

	m_iWidth = 20;

#if 0
	m_poGadget = (Object *) IIntuition->NewObject(NULL, "scroller.gadget", /*GA_Left, (ScreenWidth - m_poWindow->BorderRight - 20),*/
		GA_ID, a_iGadgetID, /*GA_Top, m_poWindow->BorderTop, GA_Width, 20, GA_Height, (ScreenHeight - m_poWindow->BorderTop - m_poWindow->BorderBottom),*/
		SCROLLER_Top, 0, SCROLLER_Total, 419, SCROLLER_Visible, 20, GA_RelVerify, TRUE, SCROLLER_Orientation, SORIENT_VERT, TAG_DONE);

#else
	// TODO: CAW - Check InnerWidth/Height() + no GA_RelVerify
	m_poGadget = (Object *) IIntuition->NewObject(NULL, "propgclass", GA_Left, (a_poParentWindow->m_poWindow->BorderLeft + a_poParentWindow->InnerWidth() - 20),
		GA_ID, a_iGadgetID, GA_Top, a_poParentWindow->m_poWindow->BorderTop, GA_Width, 20, GA_Height, a_poParentWindow->InnerHeight(),
		PGA_Total, 419, PGA_Visible, 20, PGA_NewLook, TRUE, ICA_TARGET, ICTARGET_IDCMP, TAG_DONE);
#endif

#else /* ! __amigaos4__ */

	/* Find out the standard width of a scrollbar control */

	m_iWidth = GetSystemMetrics(SM_CXVSCROLL);

	/* Create the underlying Windows control */

	m_poGadget = CreateWindow("SCROLLBAR", NULL, (SBS_VERT | WS_CHILD | WS_VISIBLE),
		(m_poParentWindow->InnerWidth() - m_iWidth), 0, m_iWidth, m_poParentWindow->InnerHeight(),
		m_poParentWindow->m_poWindow, NULL, NULL, NULL);

#endif /* ! __amigaos4__ */

	if (m_poGadget)
	{
		/* And attach it to the parent window */

		a_poParentWindow->Attach(this);
	}
	else
	{
		Utils::Info("CStdGadgetSlider::Create() => Unable to create scrollbar");
	}

	return((m_poGadget) ? KErrNone : KErrNoMemory);
}

/* Written: Sunday 21-Nov-2010 11:01 am */
/* @param	a_ulData	For Amiga OS this is not used.  For Win32 it is the type of update */
/*						this is, such as SB_THUMBTRACK etc. */
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
		IIntuition->GetAttr(PGA_Top, m_poGadget, &Result);
		m_poClient->SliderUpdated(this, (Result + 1));
	}

#else /* ! __amigaos4__ */

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
				ScrollInfo.nPos = Position = (ScrollInfo.nPos - ScrollInfo.nPage);
			}
			else if (a_ulData == SB_PAGEDOWN)
			{
				ScrollInfo.nPos = Position = (ScrollInfo.nPos + ScrollInfo.nPage);
			}
		}

		/* If we obtained the scroll box position successfully, update the client with */
		/* the new position */

		if (Position != -1)
		{
			SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, FALSE);
			m_poClient->SliderUpdated(this, Position);
		}
	}

#endif /* ! __amigaos4__ */

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

	IIntuition->SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
		PGA_Top, (a_iPosition - 1), TAG_DONE);

#else /* ! __amigaos4__ */

	SCROLLINFO ScrollInfo;

	ScrollInfo.cbSize = sizeof(ScrollInfo);
	ScrollInfo.fMask = SIF_POS;
	ScrollInfo.nPos = a_iPosition;
	SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE); // TODO: CAW - Look into refreshing display

#endif /* ! __amigaos4__ */

}

/* Written: Tuesday 23-Nov-2010 7:56 am */
/* @param	a_iPageSize	The number of lines per page;  that is, the number of lines scrolled up or */
/*						down when the page up and page down section of the slider is clicked */
/*			a_iMaxRange	The maximum number of lines that can be displayed */
/* Sets the range of the slider so that when it moves up and down it matches the top and bottom of the */
/* data being displayed on screen. */

void CStdGadgetSlider::SetRange(TInt a_iPageSize, TInt a_iMaxRange)
{
	ASSERTM((m_poGadget != NULL), "CStdGadgetSlider::SetRange() => Slider gadget has not been created");
	ASSERTM((a_iPageSize <= a_iMaxRange), "CStdGadgetSlider::SetRange() => a_iPageSize is too large");

#ifdef __amigaos4__

	IIntuition->SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL,
		PGA_Visible, a_iPageSize, PGA_Total, a_iMaxRange, TAG_DONE);

#else /* ! __amigaos4__ */

	SCROLLINFO ScrollInfo;

	ScrollInfo.cbSize = sizeof(ScrollInfo);
	ScrollInfo.fMask = (SIF_PAGE | SIF_RANGE);
	ScrollInfo.nPage = a_iPageSize;
	ScrollInfo.nMin = 1;
	ScrollInfo.nMax = a_iMaxRange;
	SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE);

#endif /* ! __amigaos4__ */

}
