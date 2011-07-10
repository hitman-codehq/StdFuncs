
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

/* Written: Tuesday 06-Jul-2011 6:58 am, CodeHQ-by-Thames */
/* @param	a_iX	X position to which to move gadget */
/*			a_iY	Y position to which to move gadget */
/* Sets the gadget to the X and Y positions specified, relative to the top left */
/* of the window's client area */

void CStdGadget::SetGadgetPosition(TInt a_iX, TInt a_iY)
{

#ifdef __amigaos4__

	// TODO: CAW - Implement this

#else /* ! __amigaos4__ */

	DEBUGCHECK((SetWindowPos(m_poGadget, 0, a_iX, a_iY, 0, 0, (SWP_NOSIZE | SWP_NOZORDER)) != FALSE), "CStdGadget::SetGadgetPosition() => Unable to set gadget position");

#endif /* ! __amigaos4__ */

}

/* Written: Tuesday 06-Jul-2011 7:07 am, CodeHQ-by-Thames */
/* @param	a_iWidth	Width of the gadget in pixels */
/*			a_iHeight	Height of the gadget in pixels */
/* Sets the width and height of the gadget in pixels */

void CStdGadget::SetGadgetSize(TInt a_iWidth, TInt a_iHeight)
{

#ifdef __amigaos4__

	// TODO: CAW - Implement this

#else /* ! __amigaos4__ */

	if (a_iWidth == -1)
	{
		a_iWidth = m_iWidth;
	}

	if (a_iHeight == -1)
	{
		a_iHeight = m_iHeight;
	}

	DEBUGCHECK((SetWindowPos(m_poGadget, 0, 0, 0, a_iWidth, a_iHeight, (SWP_NOMOVE | SWP_NOZORDER)) != FALSE), "CStdGadget::SetGadgetSize() => Unable to set gadget size");

#endif /* ! __amigaos4__ */

}
