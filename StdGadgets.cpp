
#include "StdFuncs.h"
#include <string.h>
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

#ifdef __amigaos__

#include <gadgets/layout.h>

#elif defined(QT_GUI_LIB)

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
		// TODO: CAW (multi) - Why are some gadgets now deleting themselves?
		//delete m_poGadget;
	}

#endif /* QT_GUI_LIB */

}

/* Written: Tuesday 13-Jul-2011 06:40 am, Code HQ-by-Thames */

TInt CStdGadget::X()
{

#ifdef __amigaos4__

	GetAttr(GA_Left, m_poGadget, (ULONG *) &m_iX);

#elif defined(__amigaos__)

	m_iX = ((struct Gadget *) m_poGadget)->LeftEdge;

#endif /* __amigaos__ */

	return(m_iX);
}

/* Written: Tuesday 13-Jul-2011 06:42 am, Code HQ-by-Thames */

TInt CStdGadget::Y()
{

#ifdef __amigaos4__

	GetAttr(GA_Top, m_poGadget, (ULONG *) &m_iY);

#elif defined(__amigaos__)

	m_iY = ((struct Gadget *) m_poGadget)->TopEdge;

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

#ifdef __amigaos4__

		GetAttr(GA_Width, m_poGadget, (ULONG *) &m_iWidth);

#else /* ! __amigaos4__ */

		m_iWidth = ((struct Gadget *) m_poGadget)->Width;

#endif /* ! __amigaos4__ */

		RetVal = m_iWidth;
	}
	else
	{
		RetVal = 0;
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW (multi) - Decide how this should work overall
	RetVal = (m_bHidden) ? 0 : (m_poGadget) ? m_poGadget->width() : m_iWidth;

#else /* ! QT_GUI_LIB */

	RetVal = (m_bHidden) ? 0 : m_iWidth;

#endif /* ! QT_GUI_LIB */

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

#ifdef __amigaos4__

		GetAttr(GA_Height, m_poGadget, (ULONG *) &m_iHeight);

#else /* ! __amigaos4__ */

		m_iHeight = ((struct Gadget *) m_poGadget)->Height;

#endif /* ! __amigaos4__ */

		RetVal = m_iHeight;
	}
	else
	{
		RetVal = 0;
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW (multi) - Decide how this should work overall
	RetVal = (m_bHidden) ? 0 : (m_poGadget) ? m_poGadget->height() : m_iHeight;

#else /* ! QT_GUI_LIB */

	RetVal = (m_bHidden) ? 0 : m_iHeight;

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/**
 * Returns the minimum height of the gadget in pixels.
 * Returns the minimum height of the gadget, taking into account whether or not the
 * gadget is visible.  Hidden gadgets return a height of zero.
 *
 * @date	Wednesday 23-Nov-2011 6:37 am, Code HQ Soeflingen
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
 * @date	Sunday 19-May-2011 6:34 am, Sankt Josef Hotel & Restaurant, Wuerzburg
 * @param	a_bVisible		true to make gadget visible, else false to hide it
 */

void CStdGadget::SetVisible(bool a_bVisible)
{
	/* Keep track of the state of the gadget ourselves as some systems make */
	/* querying this information difficult (I'm lookin' at you Qt!) */

	m_bHidden = !a_bVisible;

	/* And hide the underlying OS specific gadget */

#ifdef __amigaos__

	/* Unlike the Qt and Windows versions, once we get rid of this on Amiga OS we can't get it back as it */
	/* is destroyed.  Gadgets that need to make themselves visible again will need to override this method */
	/* and recreate the native gadget when true is passed in */

	if (!a_bVisible && (m_poGadget != NULL))
	{
		SetGadgetAttrs((struct Gadget *) m_poParentLayout->m_poGadget, m_poParentWindow->m_poWindow, NULL,
			LAYOUT_RemoveChild, (ULONG) m_poGadget, TAG_DONE);

		/* The native gadget is destroyed when it is removed, so indicate this by setting its pointer to NULL */

		m_poGadget = NULL;
	}

#elif defined(QT_GUI_LIB)

	if (m_poGadget)
	{
		m_poGadget->setVisible(a_bVisible);
	}

#else /* ! QT_GUI_LIB */

	ShowWindow(m_poGadget, (a_bVisible) ? SW_SHOW : SW_HIDE);

#endif /* ! QT_GUI_LIB */

}
