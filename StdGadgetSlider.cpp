
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

TInt CStdGadgetSlider::Create(CWindow *a_poParentWindow)
{
	m_poParentWindow = a_poParentWindow;

#if 0
	m_poGadget = (Object *) IIntuition->NewObject(NULL, "scroller.gadget", /*GA_Left, (ScreenWidth - m_poWindow->BorderRight - 20),*/
		GA_ID, 27, /*GA_Top, m_poWindow->BorderTop, GA_Width, 20, GA_Height, (ScreenHeight - m_poWindow->BorderTop - m_poWindow->BorderBottom),*/
		SCROLLER_Top, 0, SCROLLER_Total, 419, SCROLLER_Visible, 20, GA_RelVerify, TRUE, SCROLLER_Orientation, SORIENT_VERT, TAG_DONE);

#else
	// TODO: CAW - Check InnerWidth/Height() + no GA_RelVerify
	m_poGadget = (Object *) IIntuition->NewObject(NULL, "propgclass", GA_Left, (a_poParentWindow->m_poWindow->BorderLeft + a_poParentWindow->InnerWidth() - 20),
		GA_ID, 27, GA_Top, a_poParentWindow->m_poWindow->BorderTop, GA_Width, 20, GA_Height, a_poParentWindow->InnerHeight(),
		PGA_Total, 419, PGA_Visible, 20, PGA_NewLook, TRUE, ICA_TARGET, ICTARGET_IDCMP, TAG_DONE);
#endif

	if (m_poGadget)
	{
		a_poParentWindow->Attach(this);
	}

	return((m_poGadget) ? KErrNone : KErrNoMemory);
}

