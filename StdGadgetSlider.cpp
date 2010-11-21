
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

	return((m_poGadget) ? KErrNone : KErrNoMemory);
}

/* Written: Sunday 21-Nov-2010 11:01 am */

void CStdGadgetSlider::Updated()
{
	ULONG Result;

	/* If there is a client interested in getting updates, determine the current value of the */
	/* proportional gadget and let the client know it */

	if (m_poClient)
	{
		IIntuition->GetAttr(PGA_Top, m_poGadget, &Result);
		m_poClient->SliderUpdated(this, Result);
	}
}

