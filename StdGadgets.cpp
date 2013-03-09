
#include "StdFuncs.h"
#include <string.h>
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

/* Written: Wednesday 21-Mar-2011 6:25 am, Hilton Košice */

CStdGadget::~CStdGadget()
{
	delete [] m_iText;
}

/* Written: Tuesday 13-Jul-2011 06:40 am, Code HQ-by-Thames */

TInt CStdGadget::X()
{

#ifdef __amigaos4__

	IIntuition->GetAttr(GA_Left, m_poGadget, (ULONG *) &m_iX);

#endif /* __amigaos4__ */

	return(m_iX);
}

/* Written: Tuesday 13-Jul-2011 06:42 am, Code HQ-by-Thames */

TInt CStdGadget::Y()
{

#ifdef __amigaos4__

	IIntuition->GetAttr(GA_Top, m_poGadget, (ULONG *) &m_iY);

#endif /* __amigaos4__ */

	return(m_iY);
}

/* Written: Wednesday 11-May-2011 07:00 am, Code HQ-by-Thames */

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

/* Written: Monday 09-May-2011 8:11 am, Code HQ-by-Thames */

TInt CStdGadget::Height()
{

#ifdef __amigaos4__

	/* Query the gadget for its height, which is set dynamically and can only */
	/* be queried after the gadget has been layed out onto the screen */

	IIntuition->GetAttr(GA_Height, m_poGadget, (ULONG *) &m_iHeight);

#endif /* __amigaos4__ */

	return(m_iHeight);
}

/* Written: Wednesday 23-Nov-2011 6:37 am, Code HQ Söflingen */

TInt CStdGadget::MinHeight()
{
	return(m_iMinHeight);
}

/* Written: Wednesday 21-Mar-2011 6:27 am, Hilton Košice */

void CStdGadget::SaveText(const char *a_pccText)
{
	// TODO: CAW - Use function written for Linux for resizable buffer
	delete [] m_iText;

	if ((m_iText = new char[strlen(a_pccText) + 1]) != NULL)
	{
		strcpy(m_iText, a_pccText);
	}
}

/* Written: Tuesday 06-Jul-2011 6:58 am, Code HQ-by-Thames */
/* @param	a_iX	X position to which to move gadget */
/*			a_iY	Y position to which to move gadget */
/* Sets the gadget to the X and Y positions specified, relative to the top left */
/* of the window's client area.  If -1 is passed in for either or both of these */
/* positions then the current position will be retained, thus allowing this */
/* function to adjust only the X or Y position without affecting the other. */
/* This is an internal function and should not be used by client code. */

void CStdGadget::SetPosition(TInt a_iX, TInt a_iY)
{

#ifdef __amigaos4__

	(void) a_iX;
	(void) a_iY;

	DEBUGFAILURE("CStdGadget::SetPosition() => Must not be called on Amiga OS");

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	if (a_iX == -1)
	{
		a_iX = m_iX;
	}

	if (a_iY == -1)
	{
		a_iY = m_iY;
	}

	DEBUGCHECK((SetWindowPos(m_poGadget, 0, a_iX, a_iY, 0, 0, (SWP_NOSIZE | SWP_NOZORDER)) != FALSE), "CStdGadget::SetPosition() => Unable to set gadget position");

#endif /* ! QT_GUI_LIB */

}

/* Written: Tuesday 06-Jul-2011 7:07 am, Code HQ-by-Thames */
/* @param	a_iWidth	Width of the gadget in pixels */
/*			a_iHeight	Height of the gadget in pixels */
/* Sets the width and height of the gadget in pixels.  If -1 is passed in for */
/* either or both of these values then the current position will be retained, */
/* thus allowing this function to adjust only the with or height without */
/* affecting the other.  This is an internal function and should not be used */
/* by client code. */

void CStdGadget::SetSize(TInt a_iWidth, TInt a_iHeight)
{

#ifdef __amigaos4__

	(void) a_iWidth;
	(void) a_iHeight;

	DEBUGFAILURE("CStdGadget::SetSize() => Must not be called on Amiga OS");

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	if (a_iWidth == -1)
	{
		a_iWidth = m_iWidth;
	}

	if (a_iHeight == -1)
	{
		a_iHeight = m_iHeight;
	}

	DEBUGCHECK((SetWindowPos(m_poGadget, 0, 0, 0, a_iWidth, a_iHeight, (SWP_NOMOVE | SWP_NOZORDER)) != FALSE), "CStdGadget::SetSize() => Unable to set gadget size");

#endif /* ! QT_GUI_LIB */

}
