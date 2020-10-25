
#include "StdFuncs.h"
#include <string.h>
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

#ifdef QT_GUI_LIB

#include <QtWidgets/QWidget>

#endif /* QT_GUI_LIB */

/**
 * CStdGadget destructor.
 * Deletes the underlying OS specific gadget.
 *
 * @date	Wednesday 21-Mar-2011 6:25 am, Hilton Košice
 */

CStdGadget::~CStdGadget()
{

#ifdef QT_GUI_LIB

	/* Delete the Qt widget, if it exists */

	if (m_poGadget)
	{
		delete m_poGadget;
	}

#endif /* QT_GUI_LIB */

}

/* Written: Tuesday 13-Jul-2011 06:40 am, Code HQ-by-Thames */

TInt CStdGadget::X()
{

#ifdef __amigaos__

	IIntuition->GetAttr(GA_Left, m_poGadget, (ULONG *) &m_iX);

#endif /* __amigaos__ */

	return(m_iX);
}

/* Written: Tuesday 13-Jul-2011 06:42 am, Code HQ-by-Thames */

TInt CStdGadget::Y()
{

#ifdef __amigaos__

	IIntuition->GetAttr(GA_Top, m_poGadget, (ULONG *) &m_iY);

#endif /* __amigaos__ */

	return(m_iY);
}

/**
 * Returns the width of the gadget in pixels.
 * Returns the width of the gadget, taking into account whether or not the gadget
 * is visible.  Hidden gadgets return a width of zero.
 *
 * @date	Wednesday 11-May-2011 07:00 am, Code HQ-by-Thames
 * @return	The width of the gadget in pixels
 */

TInt CStdGadget::Width()
{
	TInt RetVal;

#ifdef __amigaos__

	// TODO: CAW - Assert on gadget being added to the screen, here and below
	/* If the gadget is not hidden then query it for its width, which is set */
	/* dynamically and can only be queried after the gadget has been layed out */
	/* onto the screen */

	if (!(m_bHidden))
	{
		IIntuition->GetAttr(GA_Width, m_poGadget, (ULONG *) &m_iWidth);
		RetVal = m_iWidth;
	}
	else
	{
		RetVal = 0;
	}

#else /* ! __amigaos__ */

	RetVal = (m_bHidden) ? 0 : m_iWidth;

#endif /* ! __amigaos__ */

	return(RetVal);
}

/**
 * Returns the height of the gadget in pixels.
 * Returns the height of the gadget, taking into account whether or not the gadget
 * is visible.  Hidden gadgets return a height of zero.
 *
 * @date	Monday 09-May-2011 8:11 am, Code HQ-by-Thames
 * @return	The height of the gadget in pixels
 */

TInt CStdGadget::Height()
{
	TInt RetVal;

#ifdef __amigaos__

	/* If the gadget is not hidden then query it for its height, which is set */
	/* dynamically and can only be queried after the gadget has been layed out */
	/* onto the screen */

	if (!(m_bHidden))
	{
		IIntuition->GetAttr(GA_Height, m_poGadget, (ULONG *) &m_iHeight);
		RetVal = m_iHeight;
	}
	else
	{
		RetVal = 0;
	}

#else /* ! __amigaos__ */

	RetVal = (m_bHidden) ? 0 : m_iHeight;

#endif /* ! __amigaos__ */

	return(RetVal);
}

/**
 * Returns the minimum height of the gadget in pixels.
 * Returns the minimum height of the gadget, taking into account whether or not the
 * gadget is visible.  Hidden gadgets return a height of zero.
 *
 * @date	Wednesday 23-Nov-2011 6:37 am, Code HQ Söflingen
 * @return	The minimum height of the gadget in pixels
 */

TInt CStdGadget::MinHeight()
{
	return((m_bHidden) ? 0 : m_iMinHeight);
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

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	(void) a_iX;
	(void) a_iY;

	DEBUGFAILURE("CStdGadget::SetPosition() => Must not be called on this platform");

#else /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

	if (a_iX == -1)
	{
		a_iX = m_iX;
	}

	if (a_iY == -1)
	{
		a_iY = m_iY;
	}

	DEBUGCHECK((SetWindowPos(m_poGadget, 0, a_iX, a_iY, 0, 0, (SWP_NOSIZE | SWP_NOZORDER)) != FALSE), "CStdGadget::SetPosition() => Unable to set gadget position");

#endif /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

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

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	(void) a_iWidth;
	(void) a_iHeight;

	DEBUGFAILURE("CStdGadget::SetSize() => Must not be called on this platform");

#else /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

	if (a_iWidth == -1)
	{
		a_iWidth = m_iWidth;
	}

	if (a_iHeight == -1)
	{
		a_iHeight = m_iHeight;
	}

	DEBUGCHECK((SetWindowPos(m_poGadget, 0, 0, 0, a_iWidth, a_iHeight, (SWP_NOMOVE | SWP_NOZORDER)) != FALSE), "CStdGadget::SetSize() => Unable to set gadget size");

#endif /* ! defined(__amigaos__) || defined(QT_GUI_LIB) */

}

/**
* Shows or hides the gadget.
* This function will set the associated gadget to hidden and will redraw the
* display to reflect this new state.  It also keeps track of the hidden state
* internally so that if the gadget's size is queried it will return that it is
* zero pixels high and zero pixels wide.  The gadget remains on the window's
* list of gadgets but is now invisible to the eye.
*
* @date	Sunday 19-May-2011 6:34 am, Sankt Josef Hotel & Restaurant, Würzburg
* @param	a_bVisible		true to make gadget visible, else false to hide it
*/

void CStdGadget::SetVisible(bool a_bVisible)
{
	/* Keep track of the state of the gadget ourselves as some systems make */
	/* querying this information difficult (I'm lookin' at you Qt!) */

	m_bHidden = !a_bVisible;

	/* And hide the underlying OS specific gadget */

#ifdef __amigaos__

	/* Unlike the Qt and Windows versions, once we get rid of this on Amiga OS we can't get it back, */
	/* and emulating the behaviour of the other versions proved to be surprisingly difficult.  So */
	/* until a CStdGadget::UnHide() function is required we won't bother */

	IIntuition->SetGadgetAttrs((struct Gadget *) m_poParentLayout->m_poGadget, m_poParentWindow->m_poWindow, NULL,
		LAYOUT_RemoveChild, m_poGadget, TAG_DONE);

#elif defined(QT_GUI_LIB)

	if (m_poGadget)
	{
		m_poGadget->setVisible(a_bVisible);
	}

#else /* ! QT_GUI_LIB */

	ShowWindow(m_poGadget, (a_bVisible) ? SW_SHOW : SW_HIDE);

#endif /* ! QT_GUI_LIB */

}
