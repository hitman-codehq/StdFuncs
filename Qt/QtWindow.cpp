
#include "../StdFuncs.h"
#include "../StdWindow.h"
#include "QtAction.h"
#include "QtWindow.h"
#include <QLocale>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>

/* Array of key mappings for mapping Qt keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, Qt::Key_Shift }, { STD_KEY_ALT, Qt::Key_Alt }, { STD_KEY_CONTROL, Qt::Key_Control },
	{ STD_KEY_META, Qt::Key_Meta }, { STD_KEY_BACKSPACE, Qt::Key_Backspace }, { STD_KEY_ENTER, Qt::Key_Return },
	{ STD_KEY_UP, Qt::Key_Up }, { STD_KEY_DOWN, Qt::Key_Down },	{ STD_KEY_LEFT, Qt::Key_Left },
	{ STD_KEY_RIGHT, Qt::Key_Right }, { STD_KEY_HOME, Qt::Key_Home }, { STD_KEY_END, Qt::Key_End },
	{ STD_KEY_PGUP, Qt::Key_PageUp }, { STD_KEY_PGDN, Qt::Key_PageDown }, { STD_KEY_ESC, Qt::Key_Escape },
	{ STD_KEY_DELETE, Qt::Key_Delete }, { STD_KEY_TAB, Qt::Key_Tab }, { STD_KEY_TAB, Qt::Key_Backtab },
	{ STD_KEY_F1, Qt::Key_F1 }, { STD_KEY_F2, Qt::Key_F2 }, { STD_KEY_F3, Qt::Key_F3 },
	{ STD_KEY_F4, Qt::Key_F4 }, { STD_KEY_F5, Qt::Key_F5 }, { STD_KEY_F6, Qt::Key_F6 },
	{ STD_KEY_F7, Qt::Key_F7 }, { STD_KEY_F8, Qt::Key_F8 }, { STD_KEY_F9, Qt::Key_F9 },
	{ STD_KEY_F10, Qt::Key_F10 }, { STD_KEY_F11, Qt::Key_F11 },	{ STD_KEY_F12, Qt::Key_F12 }
};

#define NUM_KEYMAPPINGS (sizeof(g_aoKeyMap) / sizeof(struct SKeyMapping))

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 27-Sep-2019 3:02 pm, HERE Invalidenstrasse
 * @param	Parameter		Description
 * @return	Return value
 */

CQtWindow::CQtWindow(CWindow *a_poWindow, QSize &a_roSize)
{
	m_poWindow = a_poWindow;
	m_oSize = a_roSize;

	/* Allow the window to accept keyboard input by default */

	setFocusPolicy(Qt::StrongFocus);
}

/**
 * Qt helper function to receive key events.
 * This is the internal function which handles both key presses and key releases under Qt.
 *
 * @date	Saturday 26-Jan-2013 11:49 am, Code HQ Ehinger Tor
 * @param	a_poKeyEvent	Ptr to a structure containing information about the event
 * @param	a_bKeyDown		true if a key press is being handled, else false for key a release
 */

void CQtWindow::HandleKeyEvent(QKeyEvent *a_poKeyEvent, bool a_bKeyDown)
{
	TInt Key, NativeKey, Index;

	/* Scan through the key mappings and find the one that has just been pressed or released */

	NativeKey = a_poKeyEvent->key();

	for (Index = 0; Index < (int) NUM_KEYMAPPINGS; ++Index)
	{
		if (g_aoKeyMap[Index].m_iNativeKey == NativeKey)
		{
			break;
		}
	}

	/* If the key has a key mapping then send the standard key to the client */

	if (Index < (int) NUM_KEYMAPPINGS)
	{
		m_poWindow->OfferKeyEvent(g_aoKeyMap[Index].m_iStdKey, a_bKeyDown);
	}

	/* Otherwise it is an ASCII key event so call the CWindow::OfferKeyEvent() function, passing in only */
	/* valid ASCII characters */

	else
	{
		QString String = a_poKeyEvent->text();

#ifdef __APPLE__

		/* If a meta key combination is pressed, no character is passed in in the key event's */
		/* text() string, so we have to simulate one in order for the client to know what was */
		/* pressed */

		if (CWindow::AltPressed() || CWindow::CtrlPressed() || CWindow::MetaPressed())
		{
			if (String.length() == 0)
			{
				/* On Mac OS, handling of the '~' key requires special handling, so do it here and */
				/* send it to the client as a "normal" key so the client doesn't need to worry */
				/* about it */

				if (CWindow::AltPressed() && NativeKey == 'N')
				{
					String.append("~");
				}
				else
				{
					String.append((char) (NativeKey | 0x20));
				}
			}
		}

#endif /* __APPLE__ */

		/* See if we have a key available */

		if (String.length() >= 1)
		{
			Key = (unsigned char) String[0].toLatin1();

			/* It seems that Qt keyboard handling is just as complex as Windows.  :-( */
			/* We have to take care of a complex combination of key presses that come in, */
			/* including keys that are available from both the text() and key() functions */
			/* but which have different values.  The code below was arrived at by trial */
			/* and error and manages to pass all plain ASCII values to the client, as well */
			/* as keys prefixed with ctrl, alt and the ever-easy-to-lose alt-gr key! */
			/* Almost all ASCII keys are handled by the first OfferKeyEvent() while those */
			/* that are pressed while ctrl is held down are processed by the second one. */
			/* In this case text() returns an incorrect keycode so we use key() instead */

			if ((Key >= 32) && (Key <= 254))
			{
				m_poWindow->OfferKeyEvent(Key, a_bKeyDown);
			}
			else if ((a_poKeyEvent->key() != 0) && (a_poKeyEvent->key() != Qt::Key_unknown))
			{
				/* Like the Windows API, the Qt API also maps crazy keycodes onto the '[' and ']' keys */
				/* (or the keys in their positions on keymaps other than UK/US) so we map them back to */
				/* something useful.  However, unlike Windows, if ctrl is pressed then Qt maps these keys */
				/* back to '[' and ']' even on non UK/US keymaps!  So we have to map them back to their */
				/* original keys */

				Key = a_poKeyEvent->key();

				/* If the ctrl key is currently pressed then convert the keycode back to standard ASCII, */
				/* but NOT if alt is also pressed or it will break German keymappings that use altgr! */

				if ((CWindow::m_bCtrlPressed) && (!(CWindow::m_bAltPressed)))
				{
					/* Convert the key to lower case */

					Key |= 0x60;

					/* Determine the current input locale */

					QLocale KeyboardLayout = QGuiApplication::inputMethod()->locale();

					if (KeyboardLayout.language() == QLocale::German)
					{
						/* Adjust the keycodes returned, just like in the Windows version to return the */
						/* real keys rather than these fudged ones.  Note that the "real" keys returned */
						/* are still not in accordance with the standard, but at least they are consistent! */

						if (Key == 123)
						{
							Key = (unsigned char) 252;
						}
						else if (Key == 125)
						{
							Key = '+';
						}
					}
				}

				m_poWindow->OfferKeyEvent(Key, a_bKeyDown);
			}
		}
	}

	/* Pass the raw key onto the CWindow::OfferRawKeyEvent() function, without any kind */
	/* of preprocessing */

	if ((NativeKey = a_poKeyEvent->nativeVirtualKey()) != 0)
	{
		m_poWindow->OfferRawKeyEvent(NativeKey, a_bKeyDown);
	}
}

/**
 * Qt helper function to receive mouse events.
 * This is an internal helper function that will extract information from a QMouseEvent
 * structure and will pass it onto The Framework's client code for processing.
 *
 * @date	Thursday 30-Jan-2014 7:59 am
 * @param	a_poMouseEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::HandlePointerEvent(QMouseEvent *a_poMouseEvent)
{
	TInt X, Y;
	TStdMouseEvent MouseEvent;

	/* Convert the Qt QEvent::Type enum into a TStdMouseEvent enum */

	if (a_poMouseEvent->type() == QEvent::MouseButtonPress)
	{
		MouseEvent = EStdMouseDown;
	}
	else if (a_poMouseEvent->type() == QEvent::MouseButtonRelease)
	{
		MouseEvent = EStdMouseUp;
	}
	else if (a_poMouseEvent->type() == QEvent::MouseButtonDblClick)
	{
		MouseEvent = EStdMouseDoubleClick;
	}
	else
	{
		MouseEvent = EStdMouseMove;
	}

	/* Extract the X and Y positions from the QMouseEvent structure.  The Y position is relative to */
	/* the top of the menu bar so take that into account, so that we have X and Y positions that */
	/* are relative to the client area */

	X = a_poMouseEvent->x();
	Y = (a_poMouseEvent->y() - m_poWindow->m_poWindow->menuBar()->height());

	/* And pass the event onto the client's CWindow::HandlePointerEvent() function.  Qt has a bug whereby */
	/* if you click on the menu bar to drop down a menu and then you click on the window border above that */
	/* menu bar, it sends through an unwanted pointer event!  Checking that the mouse position is >= 0 will */
	/* filter out events generated by this bug */

	if ((X >= 0) && (Y >= 0))
	{
		m_poWindow->HandlePointerEvent(X, Y, MouseEvent);
	}
}

/**
 * Qt helper function to capture window close events.
 * This function is called whenever the main window is about to be closed due to the close
 * button or <alt-f4> being pressed.  It cancels that event, thus preventing the window
 * from closing, and sends a message to the client so that it can handle the close event as
 * it wishes (for example by prompting the user whether they wish to quit).
 *
 * @date	Saturday 23-Feb-2013 1:38 pm
 * @param	a_poCloseEvent	Ptr to structure containing information for handling the event
 */

void CQtWindow::closeEvent(QCloseEvent *a_poCloseEvent)
{
	/* If we are in the process of closing then we want to accept the close event. */
	/* Otherwise ignore it and send it onto the client window for processing */

	if (!(m_bClosing))
	{
		/* Cancel the close event */

		a_poCloseEvent->ignore();

		/* Allow the client window itself to handle the close */

		m_poWindow->HandleCommand(IDCANCEL);
	}
}

/**
 * Qt helper function to detect when a window gains focus.
 * This function is called when the Qt framework detect that the window has gained focus
 * and will simply pass the event onto The Framework.
 *
 * @date	Tuesday 05-Oct-2013 7:44 am
 * @param	a_poFocusEvent	Ptr to a class containing information about the event
 */

void CQtWindow::focusInEvent(QFocusEvent * /*a_poFocusEvent*/)
{
	/* Window focus is changing so let the client know that the window is activating */

	m_poWindow->m_bIsActive = ETrue;
	m_poWindow->Activated(ETrue);
}

/**
 * Qt helper function to detect when a window gains focus.
 * This function is called when the Qt framework detect that the window has gained focus
 * and will simply pass the event onto The Framework.
 *
 * @date	Tuesday 05-Oct-2013 7:42 am
 * @param	a_poFocusEvent	Ptr to a class containing information about the event
 */

void CQtWindow::focusOutEvent(QFocusEvent * /*a_poFocusEvent*/)
{
	/* Window focus is changing so let the client know that the window is deactivating */

	m_poWindow->m_bIsActive = EFalse;
	m_poWindow->Activated(EFalse);

	/* Forget about the modifier keypresses as we won't get a key up event for them due */
	/* to the window no longer being active */

	CWindow::m_bAltPressed = CWindow::m_bCtrlPressed = CWindow::m_bMetaPressed = CWindow::m_bShiftPressed = EFalse;
}

/**
 * Qt helper function to receive key press events.
 * This function is called whenever a key down event occurs and will pass the event along
 * to the underlying framework window in the expected format, filtering out any key events
 * in which the framework is not interested.
 *
 * @date	Friday 31-Aug-2012 3:02 pm
 * @param	a_poKeyEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::keyPressEvent(QKeyEvent *a_poKeyEvent)
{
	/* If this is an alt, control or shift key then indicate this in case the client queries about it */

	if (a_poKeyEvent->key() == Qt::Key_Alt)
	{
		CWindow::m_bAltPressed = ETrue;
	}
	else if (a_poKeyEvent->key() == Qt::Key_Control)
	{
		CWindow::m_bCtrlPressed = ETrue;
	}
	else if (a_poKeyEvent->key() == Qt::Key_Meta)
	{
		CWindow::m_bMetaPressed = ETrue;
	}
	else if (a_poKeyEvent->key() == Qt::Key_Shift)
	{
		CWindow::m_bShiftPressed = ETrue;
	}

	/* Perform standard keyboard handling for the key down event */

	HandleKeyEvent(a_poKeyEvent, ETrue);
}

/**
 * Qt helper function to receive key release events.
 * This function is called whenever a key up event occurs and will pass the event along
 * to the underlying framework window in the expected format, filtering out any key events
 * in which the framework is not interested.
 *
 * @date	Saturday 26-Jan-2013 11:42 am, Code HQ Ehinger Tor
 * @param	a_poKeyEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::keyReleaseEvent(QKeyEvent *a_poKeyEvent)
{
	/* If this is an alt, control or shift key then indicate it is no longer pressed */

	if (a_poKeyEvent->key() == Qt::Key_Alt)
	{
		CWindow::m_bAltPressed = EFalse;
	}
	else if (a_poKeyEvent->key() == Qt::Key_Control)
	{
		CWindow::m_bCtrlPressed = EFalse;
	}
	else if (a_poKeyEvent->key() == Qt::Key_Meta)
	{
		CWindow::m_bMetaPressed = EFalse;
	}
	else if (a_poKeyEvent->key() == Qt::Key_Shift)
	{
		CWindow::m_bShiftPressed = EFalse;
	}

	/* Perform standard keyboard handling for the key release event */

	HandleKeyEvent(a_poKeyEvent, EFalse);
}

/**
 * Qt helper function to receive mouse button double click events.
 * Captures Qt QEvent::MouseButtonDblClick events and passes them onto client code.
 *
 * @date	Friday 31-Jan-2014 6:23 am
 * @param	a_poMouseEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::mouseDoubleClickEvent(QMouseEvent *a_poMouseEvent)
{
	HandlePointerEvent(a_poMouseEvent);
}

/**
 * Qt helper function to receive mouse move events.
 * Captures Qt QEvent::MouseMove events and passes them onto client code.
 *
 * @date	Thursday 30-Jan-2014 7:34 am
 * @param	a_poMouseEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::mouseMoveEvent(QMouseEvent *a_poMouseEvent)
{
	HandlePointerEvent(a_poMouseEvent);
}

/**
 * Qt helper function to receive mouse button press events.
 * Captures Qt QEvent::MouseButtonPress events and passes them onto client code.
 *
 * @date	Thursday 30-Jan-2014 7:33 am
 * @param	a_poMouseEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::mousePressEvent(QMouseEvent *a_poMouseEvent)
{
	HandlePointerEvent(a_poMouseEvent);
}

/**
 * Qt helper function to receive mouse button release events.
 * Captures Qt QEvent::MouseButtonRelease events and passes them onto client code.
 *
 * @date	Thursday 30-Jan-2014 7:34 am
 * @param	a_poMouseEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::mouseReleaseEvent(QMouseEvent *a_poMouseEvent)
{
	HandlePointerEvent(a_poMouseEvent);
}

/**
 * Qt helper function to receive window resize events.
 * This function is called whenever Qt performs a resize of the window and will pass
 * the event along to the generic CWindow::Resize() function, to notify client code.
 *
 * @date	Saturday 25-Aug-2012 1:36 pm
 * @param	a_poResizeEvent	Ptr to a structure containing information about the event
 */

void CQtWindow::resizeEvent(QResizeEvent * /*a_poResizeEvent*/)
{
	/* The size passed in the QResizeEvent is the size of the window itself.  We need to */
	/* use the size of the central widget as we want to exclude the height of the menu bar */

	QSize Size = centralWidget()->size();
	m_poWindow->InternalResize(Size.width(), Size.height());
}

/**
 * Qt helper function to receive mouse wheel events.
 * This function will receive mouse wheel events and simply passes them onto the client
 * unchanged.  The client is expecting the delta events in steps of +/- 120 per notch and
 * this is the measurement that Qt uses, making this a very simple function.
 *
 * @date	Saturday 15-Mar-2014 7:29 am, Code HQ Ehinger Tor
 * @param	a_poWheelEvent	Structure describing the mouse wheel event received
 */

void CQtWindow::wheelEvent(QWheelEvent *a_poWheelEvent)
{
	/* Pass the event directly onto the client and set it to ignored so that Qt performs */
	/* no further processing of it */

	m_poWindow->HandleWheelEvent(a_poWheelEvent->delta());
	a_poWheelEvent->ignore();
}

/**
 * Returns the preferred size of the window to the Qt framework.
 * This function returns the preferred size of our window to Qt, which Qt then
 * bizarrely changes.  We want the Qt version of The Framework to work like the
 * Windows version, so that a normal (ie. non maximised) window is the same size
 * as the desktop by default.  But Qt has other ideas and refuses to resize the
 * application to more than 2/3 the size of the desktop.  Oh well, that is better
 * than nothing but still a compromise.  :-(
 *
 * @date	Friday 20-Sep-2013 7:30 am
 * @return	The preferred size of the window in pixels, which Qt then somewhat ignores
 */

QSize CQtWindow::sizeHint() const
{
	return(m_oSize);
}

/**
 * Qt helper function called when a dropdown menu is about to be shown.
 * This function is called when a dropdown menu is about to be shown and will clear the
 * flags for the alt and control keys.  If these are pressed at the time the dropdown
 * menu is displayed then no key up events will be received and so when the user releases
 * the keys, the system will think they are still pressed.  Clearing the flags avoids that
 * situation.
 *
 * @date	Thursday 19-Sep-2013 7:18 am
 */

void CQtWindow::aboutToShow()
{
	/* Forget about the modifier keypresses as we won't get a key up event for them due */
	/* to the window no longer being active */

	CWindow::m_bAltPressed = CWindow::m_bCtrlPressed = CWindow::m_bMetaPressed = CWindow::m_bShiftPressed = EFalse;
}
