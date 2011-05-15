
#include "StdFuncs.h"
#include "StdGadgets.h"

#ifdef __amigaos4__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/intuition.h>
#include <reaction/reaction.h>

#endif /* __amigaos4__ */

/* Written: Wednesday 11-May-2011 07:00 am, CodeHQ-by-Thames */

TInt CStdGadget::Width()
{

#ifdef __amigaos4__

	// TODO: CAW - Assert on gadget being added to the screen, here and below
	/* Query the gadget for its width, which is set dynamically and can only */
	/* be queried after the gadget has been layed out onto the screen */

	IIntuition->GetAttr(GA_Width, m_poGadget, (ULONG *) &m_iWidth);

#endif /* __amigaos4__ */

	return(m_iWidth);
}

/* Written: Monday 09-May-2011 8:11 am, CodeHQ-by-Thames */

TInt CStdGadget::Height()
{

#ifdef __amigaos4__

	/* Query the gadget for its height, which is set dynamically and can only */
	/* be queried after the gadget has been layed out onto the screen */

	IIntuition->GetAttr(GA_Height, m_poGadget, (ULONG *) &m_iHeight);

#endif /* __amigaos4__ */

	return(m_iHeight);
}

