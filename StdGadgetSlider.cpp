
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

	/* And attach it to the parent window */

	if (m_poGadget)
	{
		a_poParentWindow->Attach(this);
	}
	else
	{
		Utils::Info("CStdGadgetSlider::Create() => Unable to create BOOPSI proportional gadget");
	}

#else /* ! __amigaos4__ */

	SCROLLINFO ScrollInfo;

	/* Create the underlying Windows control */ // TODO: CAW - Use SM_CXVSCROLL rather than 20 + try using keyboard + page size (lines per page)

	m_poGadget = CreateWindow("SCROLLBAR", NULL, (SBS_VERT | WS_CHILD | WS_VISIBLE),
		(m_poParentWindow->InnerWidth() - 20), 0, 20, m_poParentWindow->InnerHeight(), m_poParentWindow->m_poWindow,
		NULL, NULL, NULL);

	if (m_poGadget)
	{
		ScrollInfo.cbSize = sizeof(ScrollInfo);
		ScrollInfo.fMask = SIF_RANGE;
		ScrollInfo.nMin = 1;
		ScrollInfo.nMax = 419; // TODO: CAW

		SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, TRUE);
		a_poParentWindow->Attach(this);
	}
	else
	{
		Utils::Info("CStdGadgetSlider::Create() => Unable to create scroll bar");
	}

#endif /* ! __amigaos4__ */

	return((m_poGadget) ? KErrNone : KErrNoMemory);
}

/* Written: Sunday 21-Nov-2010 11:01 am */

void CStdGadgetSlider::Updated()
{

#ifdef __amigaos4__

	ULONG Result;

	/* If there is a client interested in getting updates, determine the current value of the */
	/* proportional gadget and let the client know it */

	if (m_poClient)
	{
		IIntuition->GetAttr(PGA_Top, m_poGadget, &Result);
		m_poClient->SliderUpdated(this, Result);
	}

#else /* ! __amigaos4__ */

	SCROLLINFO ScrollInfo;

	if (m_poClient)
	{
		// TODO: CAW - Assert on m_poGadget
		//IIntuition->GetAttr(PGA_Top, m_poGadget, &Result);
		ScrollInfo.cbSize = sizeof(ScrollInfo);
		ScrollInfo.fMask = SIF_TRACKPOS;

		if (GetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo))
		{
			ScrollInfo.fMask = SIF_POS;
			ScrollInfo.nPos = ScrollInfo.nTrackPos;
			SetScrollInfo(m_poGadget, SB_CTL, &ScrollInfo, FALSE);

			Utils::Info("*** nTrackPos = %d", ScrollInfo.nTrackPos);
			m_poClient->SliderUpdated(this, ScrollInfo.nTrackPos);
		}
		else
		{
			Utils::Info("CStdGadgetSlider::Updated() => Unable to get scrollbar position");
		}
	}

#endif /* ! __amigaos4__ */

}
