
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdRendezvous.h"
#include "StdWindow.h"
#include <ctype.h>
#include <string.h>

#ifdef __amigaos4__

#include "Amiga/AmiMenus.h"
#include <proto/gadtools.h>
#include <proto/utility.h>
#include <intuition/gui.h>
#include <intuition/imageclass.h>

#elif defined(QT_GUI_LIB)

#include "Qt/StdWindow.h"
#include <QtCore/QLocale>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenuBar>

#endif /* QT_GUI_LIB */

#ifdef __amigaos4__

#elif defined(QT_GUI_LIB)

/* Array of key mappings for mapping Qt keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, Qt::Key_Shift }, { STD_KEY_ALT, Qt::Key_Alt }, { STD_KEY_CONTROL, Qt::Key_Control }, { STD_KEY_BACKSPACE, Qt::Key_Backspace },
	{ STD_KEY_ENTER, Qt::Key_Return }, { STD_KEY_UP, Qt::Key_Up }, { STD_KEY_DOWN, Qt::Key_Down },
	{ STD_KEY_LEFT, Qt::Key_Left }, { STD_KEY_RIGHT, Qt::Key_Right }, { STD_KEY_HOME, Qt::Key_Home },
	{ STD_KEY_END, Qt::Key_End }, { STD_KEY_PGUP, Qt::Key_PageUp }, { STD_KEY_PGDN, Qt::Key_PageDown },  { STD_KEY_ESC, Qt::Key_Escape },
	{ STD_KEY_DELETE, Qt::Key_Delete }, { STD_KEY_TAB, Qt::Key_Tab }, { STD_KEY_TAB, Qt::Key_Backtab }, { STD_KEY_F1, Qt::Key_F1 },
	{ STD_KEY_F2, Qt::Key_F2 }, { STD_KEY_F3, Qt::Key_F3 }, { STD_KEY_F4, Qt::Key_F4 },
	{ STD_KEY_F5, Qt::Key_F5 }, { STD_KEY_F6, Qt::Key_F6 }, { STD_KEY_F7, Qt::Key_F7 },
	{ STD_KEY_F8, Qt::Key_F8 }, { STD_KEY_F9, Qt::Key_F9 }, { STD_KEY_F10, Qt::Key_F10 },
	{ STD_KEY_F11, Qt::Key_F11 },	{ STD_KEY_F12, Qt::Key_F12 }
};

#define NUM_KEYMAPPINGS (sizeof(g_aoKeyMap) / sizeof(struct SKeyMapping))

#elif defined(WIN32)

/* Array of key mappings for mapping Windows keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, VK_SHIFT }, { STD_KEY_CONTROL, VK_CONTROL }, { STD_KEY_BACKSPACE, VK_BACK },
	{ STD_KEY_ENTER, VK_RETURN }, { STD_KEY_UP, VK_UP }, { STD_KEY_DOWN, VK_DOWN },
	{ STD_KEY_LEFT, VK_LEFT }, { STD_KEY_RIGHT, VK_RIGHT }, { STD_KEY_HOME, VK_HOME },
	{ STD_KEY_END, VK_END }, { STD_KEY_PGUP, VK_PRIOR }, { STD_KEY_PGDN, VK_NEXT }, { STD_KEY_ESC, VK_ESCAPE },
	{ STD_KEY_DELETE, VK_DELETE }, { STD_KEY_TAB, VK_TAB }, { STD_KEY_F1, VK_F1 }, { STD_KEY_F2, VK_F2 },
	{ STD_KEY_F3, VK_F3 }, { STD_KEY_F4, VK_F4 }, { STD_KEY_F5, VK_F5 }, { STD_KEY_F6, VK_F6 },
	{ STD_KEY_F7, VK_F7 }, { STD_KEY_F8, VK_F8 }, { STD_KEY_F9, VK_F9 }, { STD_KEY_F10, VK_F10 },
	{ STD_KEY_F11, VK_F11 }, { STD_KEY_F12, VK_F12 }
};

#define NUM_KEYMAPPINGS (sizeof(g_aoKeyMap) / sizeof(struct SKeyMapping))

CWindow *CWindow::m_poActiveDialog;	/* Ptr to currently active dialog, if any */

#endif /* WIN32 */

TBool CWindow::m_bAltPressed;		/* ETrue if alt is currently pressed */
TBool CWindow::m_bCtrlPressed;		/* ETrue if ctrl is currently pressed */
TBool CWindow::m_bShiftPressed;		/* ETrue if shift is currently pressed */
TBool CWindow::m_bIsActive;			/* ETrue if the window is currently active */

#ifdef __amigaos4__

/* Written: Saturday 20-Nov-2010 11:05 am */

void CWindow::IDCMPFunction(struct Hook *a_poHook, Object * /*a_poObject*/, struct IntuiMessage *a_poIntuiMessage)
{
	struct IntuiWheelData *IntuiWheelData;
	struct TagItem *TagItem;
	CStdGadget *Gadget;
	CStdGadgetLayout *LayoutGadget;
	CWindow *Window;

	/* Get a ptr to the Window associated with the hook */

	Window = (CWindow *) a_poHook->h_Data;

	/* If this is a mouse wheel event then convert the delta to a Qt/Windows style reading */
	/* (+/- 120 per notch) and notify the window */

	if (a_poIntuiMessage->Class == IDCMP_EXTENDEDMOUSE)
	{
		IntuiWheelData = (struct IntuiWheelData *) a_poIntuiMessage->IAddress;
		Window->HandleWheelEvent(-IntuiWheelData->WheelY * 120);
	}

	/* If this is a message from a BOOPSI object saying that it has been updated, find the object and */
	/* map it onto its matching gadget and call the gadget's Updated() function */

	else if (a_poIntuiMessage->Class == IDCMP_IDCMPUPDATE)
	{
		/* Get the gadget's unique ID */

		if ((TagItem = IUtility->FindTagItem(GA_ID, (struct TagItem *) a_poIntuiMessage->IAddress)) != NULL)
		{
			/* Iterate through the window's list of layout gadgets and search each one to see */
			/* if it contains a gadget that represents the Reaction slider that was just moved */

			if ((LayoutGadget = Window->m_oGadgets.GetHead()) != NULL)
			{
				do
				{
					if ((Gadget = LayoutGadget->FindNativeGadget((void *) TagItem->ti_Data)) != NULL)
					{
						/* Got it!  Call the gadget's Updated() routine so that it can notify the */
						/* client of the update */

						Gadget->Updated();

						break;
					}
				}
				while ((LayoutGadget = Window->m_oGadgets.GetSucc(LayoutGadget)) != NULL);
			}
		}
	}
}

#elif defined(QT_GUI_LIB)

/* Written: Monday 28-Jan-2013 6:25 am */
/* This function is called by Qt when a menu item is selected and will pass the message */
/* onto The Framework and its underlying client */

void CQtAction::actionTriggered()
{
	m_poWindow->Window()->HandleCommand(m_iCommand);
}

/**
 * Constructor for CQtCentralWidget class.
 * Simply initialisaes the central widget with a ptr to the generic framework
 * window that owns it.
 *
 * @date	Friday 08-Mar-2013 9:13, Leo's house in Vienna
 * @param	a_poWindow	Ptr to parent framework window ptr to be saved
 */

CQtCentralWidget::CQtCentralWidget(CWindow *a_poWindow) : QWidget(a_poWindow->m_poWindow)
{
	m_poWindow = a_poWindow;
}

/**
 * Qt helper function to receive paint events.
 * This function is called whenever Qt performs a repaint of the window and will pass
 * the event along to the generic CWindow::Draw() function, so that client code can
 * perform its custom drawing.
 *
 * @date	Thursday 06-Sep-2012 1:35 pm
 * @param	a_poPaintEvent	Ptr to a structure containing information about the event
 */

void CQtCentralWidget::paintEvent(QPaintEvent *a_poPaintEvent)
{
	QRect Rect = a_poPaintEvent->rect();

	/* Pass the request along to the underlying framework window.  Note that QRect::bottom() */
	/* will return the inclusive bottom pixel but our framework works with exclusive pixel */
	/* positions.  We therefore need to calculate that position using QRect::height() */

	m_poWindow->Draw(Rect.top(), (Rect.top() + Rect.height()));
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

		/* See if we have a key available */

		if (String.length() >= 1)
		{
			Key = (unsigned char) String[0].toAscii();

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
					/* Determine the current input locale */

					QLocale KeyboardLayout = QApplication::keyboardInputLocale();

					if (KeyboardLayout.language() == QLocale::German)
					{
						/* Adjust the keycodes returned, just like in the Windows version to return the */
						/* real keys rather than these fudged ones.  Note that the "real" keys returned */
						/* are still not in accordance with the standard, but at least they are consistent! */

						if (Key == 91)
						{
							Key = (unsigned char) 'ü';
						}
						else if (Key == 93)
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

	CWindow::m_bAltPressed = CWindow::m_bCtrlPressed = CWindow::m_bShiftPressed = EFalse;
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

	CWindow::m_bAltPressed = CWindow::m_bCtrlPressed = CWindow::m_bShiftPressed = EFalse;
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

#elif defined(WIN32)

/* Written: Saturday 08-May-2010 4:43 pm */

LRESULT CALLBACK CWindow::WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	TBool Checked;
	TInt Command, Index, Key;
	TBool Handled;
	HKL KeyboardLayout;
	LRESULT RetVal;
	const struct SStdMenuItem *MenuItem;
	CStdGadget *Gadget;
	CStdGadgetLayout *LayoutGadget;
	CWindow *Window;
	COPYDATASTRUCT *CopyData;

	/* Return 0 by default for processed messages */

	RetVal = 0;

	/* Get the ptr to the C++ class associated with this window from the window word */

	Window = (CWindow *) GetWindowLong(a_poWindow, GWL_USERDATA);

	switch (a_uiMessage)
	{
		case WM_ACTIVATE :
		{
			/* If there is an active dialog then check to see if the window was just activated */
			/* by any other mechanism than a mouse click.  If so then this means that the */
			/* application was switched away from and then back to so we want to re-activate */
			/* the previously active dialog */

			if (m_poActiveDialog)
			{
				if ((a_oWParam == WA_ACTIVE) && (m_poActiveDialog))
				{
					m_poActiveDialog->Activate();
				}

				/* Otherwise the window was clicked on so indicate that there is no longer an */
				/* active dialog.  This must be done here and not in the dialog's WM_ACTIVATE */
				/* as the dialog has no knowledge of how it is being deactivated */

				else
				{
					m_poActiveDialog = NULL;
				}
			}

			/* Window focus is changing so let the client know that the window is activating */
			/* or deactivating */

			Window->Activated(a_oWParam == WA_ACTIVE);

			/* Forget about the modifier keypresses as we won't get a key up event for them due */
			/* to the window no longer being active */

			m_bAltPressed = m_bCtrlPressed = m_bShiftPressed = EFalse;

			break;
		}

		case WM_SETFOCUS :
		{
			/* Create a native Windows cursor and if successful, show it and position it at its assigned */
			/* X and Y positions */

			if (CreateCaret(Window->m_poWindow, NULL, 0, Window->m_iCursorHeight))
			{
				if (ShowCaret(Window->m_poWindow) == TRUE)
				{
					SetCaretPos(Window->m_iCursorX, Window->m_iCursorY);
				}
			}

			break;
		}

		case WM_CLOSE :
		{
			/* Allow the client window itself to handle the close */

			Window->HandleCommand(IDCANCEL);

			break;
		}

		case WM_DESTROY :
		{
			/* When the WM_DESTROY message is received, the Windows window has already been */
			/* destroyed so set its handle to NULL so that we do not try to destroy it again */
			/* in CWindow's destructor */

			Window->m_poWindow = NULL;
			PostQuitMessage(0);

			break;
		}

		case WM_COMMAND :
		{
			/* Get the ID of the command received */

			Command = LOWORD(a_oWParam);

			/* In Windows, all menu items are checkable, but we only want to toggle the checkmark */
			/* on menu items of type EStdMenuCheck.  So we must search our own menu list for the */
			/* menu item to determine whether it is checkable */

			MenuItem = Window->m_poApplication->MenuItems();

			do
			{
				/* Is this the menu item for which we are searching and is it checkable? */

				if ((MenuItem->m_iCommand == Command) && (MenuItem->m_eType == EStdMenuCheck))
				{
					/* Yes, got it!  Get its current check state and toggle it */

					Checked = Window->MenuItemChecked(Command);
					Window->CheckMenuItem(Command, (!(Checked)));

					break;
				}

				++MenuItem;
			}
			while (MenuItem->m_eType != EStdMenuEnd);

			/* Call the CWindow::HandleCommand() function so the client can process the message */

			Window->HandleCommand(Command);

			break;
		}

		case WM_CHAR :
		case WM_SYSCHAR :
		{
			/* If the ctrl key is currently pressed then convert the keycode back to standard ASCII, */
			/* but NOT if alt is also pressed or it will break German keymappings that use altgr! */

			if ((m_bCtrlPressed) && (!(m_bAltPressed)))
			{
				a_oWParam |= 0x60;

				/* This is awful hackiness taken to the next level.  For reasons unknown, the '[' */
				/* and ']' keys (on the UK/US keyboard layouts) are setup to return the keycodes 27 */
				/* (escape) and 29 (group separator) when pressed in conjunction with the ctrl key. */
				/* The keys in the equivalent position on a German keyboard ('ü' and '+' respectively) */
				/* are also setup to return these key values!  Although this explains the mystery of how */
				/* MSVC magically uses these keys on different keyboard layouts, it makes it impossible */
				/* to write truly generic keyboard handling code.  In a hack of horrific dimensions we */
				/* translate these keys into the values for the real keys at these positions.  This might */
				/* have to be extended to use a table in the future, if more crazy key mappings and */
				/* languages are to be supported */

				if ((a_oWParam == 123) || (a_oWParam == 125))
				{
					/* Determine the current input locale */

					KeyboardLayout = GetKeyboardLayout(0);

					if (LOBYTE(KeyboardLayout) == LANG_ENGLISH)
					{
						if (a_oWParam == 123)
						{
							a_oWParam = '[';
						}
						else if (a_oWParam == 125)
						{
							a_oWParam = ']';
						}
					}
					else
					{
						if (a_oWParam == 123)
						{
							a_oWParam = (unsigned char) 'ü';
						}
						else if (a_oWParam == 125)
						{
							a_oWParam = '+';
						}
					}
				}
			}

			/* Call the CWindow::OfferKeyEvent() function, passing in only valid ASCII characters */

			if ((a_oWParam >= 32) && (a_oWParam <= 254))
			{
				Handled = Window->OfferKeyEvent(a_oWParam, ETrue);

				/* For WM_SYSCHAR we have some special processing to allow client code to override */
				/* alt+x key combinations while still allowing menu shortcuts to work.  If the client */
				/* returns ETrue then it has consumed the keypress - that is it has performed some action */
				/* on it such as a shortcut.  In this case we don't want to pass the keypress onto the */
				/* system as it would activate menu items with the same keycode.  If the keypress was not */
				/* consumed then set RetVal to 1 to pass the kepress onto the system using DefWindowProc() */

				if ((a_uiMessage == WM_SYSCHAR) && (!(Handled)))
				{
					RetVal = 1;
				}
			}

			break;
		}

		case WM_SYSKEYDOWN :
		case WM_SYSKEYUP :
		{
			/* Intercept the menu activation key (F10 or alt+x key combination) so that we can keep track */
			/* of the state of the alt and ctrl keys.  This is to allow activation of menus in the normal */
			/* manner while also allowing the client to override the menu activation key for other tasks */

			if (a_oWParam == VK_MENU)
			{
				if (a_uiMessage == WM_SYSKEYDOWN)
				{
					m_bAltPressed = ETrue;
				}
				else
				{
					m_bAltPressed = m_bCtrlPressed = EFalse;
				}
			}

			/* Pass the raw key onto the CWindow::OfferRawKeyEvent() function, without any kind */
			/* of preprocessing */

			Window->OfferRawKeyEvent(a_oWParam, (a_uiMessage == WM_SYSKEYDOWN));

			/* Have some special processing for the alt key, passing it onto the client code.  Even */
			/* though the the alt key is not handled through the usual g_aoKeyMap array, to client */
			/* code its handling will appear consistent */

			if (a_oWParam == VK_MENU)
			{
				Window->OfferKeyEvent(STD_KEY_ALT, m_bAltPressed);
			}

			/* And indicate that we want to pass the keypress onto the system */

			RetVal = 1;

			break;
		}

		case WM_MENUSELECT :
		{
			/* Forget about the modifier keypresses as we won't get a WM_KEYUP or WM_SYSKEYUP for them */
			/* due to the window no longer being active.  This message is also sent when alt is pressed */
			/* so in this case ignore it */

			if (a_oLParam != 0)
			{
				m_bAltPressed = m_bCtrlPressed = m_bShiftPressed = EFalse;
			}

			break;
		}

		case WM_KEYDOWN :
		case WM_KEYUP :
		{
			/* Scan through the key mappings and find the one that has just been pressed */

			for (Index = 0; Index < (TInt) NUM_KEYMAPPINGS; ++Index)
			{
				if (g_aoKeyMap[Index].m_iNativeKey == (int) a_oWParam)
				{
					break;
				}
			}

			/* If it was a known key then convert it to the standard value and pass it to the */
			/* CWindow::OfferKeyEvent() function */

			if (Index < (TInt) NUM_KEYMAPPINGS)
			{
				Key = g_aoKeyMap[Index].m_iStdKey;

				/* First record the state of the shift key, it it was indeed shift that was pressed */
				/* or released */

				if (Key == STD_KEY_SHIFT)
				{
					m_bShiftPressed = (a_uiMessage == WM_KEYDOWN) ? ETrue : EFalse;
				}

				Window->OfferKeyEvent(Key, (a_uiMessage == WM_KEYDOWN));
			}

			/* This is pretty horrible.  Because Windows mixes its WM_KEY#? and WM_CHAR events, we */
			/* can get duplicate keys that need to get filtered out in order to provide a consistent */
			/* pattern to calling CWindow::OfferKeyEvent().  Also, when ctrl is pressed the ASCII */
			/* characters sent to WM_CHAR messages are different so again we need to adjust these */
			/* back to standard ASCII so keeping track of the state of the ctrl key is the only way */
			/* to achieve this */

			if (a_oWParam == VK_CONTROL)
			{
				m_bCtrlPressed = (a_uiMessage == WM_KEYDOWN) ? ETrue : EFalse;
			}

			/* This is even more horrible.  When the ALT GR key is pressed, rather than using a special */
			/* key, Windows sends through a VK_CONTROL WM_KEYDOWN event, followed by VK_MENU (otherwise */
			/* known as ALT) WM_KEYDOWN and WM_KEYUP events, with NO following VK_CONTROL WM_KEYUP event! */
			/* So we need to put some special magic in here to handle this nonsense or we will think that */
			/* the ctrl key is pressed when it isn't.  We also have to keep track of the VK_MENU key to */
			/* extend our workarounds to work when using altgr to enter characters on German keyboards */

			else if (a_oWParam == VK_MENU)
			{
				if (a_uiMessage == WM_KEYDOWN)
				{
					m_bAltPressed = ETrue;
				}
				else
				{
					m_bAltPressed = m_bCtrlPressed = EFalse;
				}
			}

#ifdef _WIN32_WINNT_WIN2K

			/* Now take the horribleness to the next level.  Certain keys (including the mysterious */
			/* VK_OEM_MINUS key) are simply not passed to WM_CHAR if the control key is pressed.  But */
			/* we need these keys and they work without special treatment on other operating systems. */
			/* So we have to look out for them individually and simulate a WM_CHAR event if found */

			UINT VirtualKey;

			if ((m_bCtrlPressed) && (!m_bAltPressed))
			{
				if ((a_oWParam == VK_SUBTRACT) || (a_oWParam == VK_ADD) || (a_oWParam == VK_OEM_MINUS))
				{
					VirtualKey = MapVirtualKey(a_oWParam, MAPVK_VK_TO_CHAR);

					Window->OfferKeyEvent(VirtualKey, (a_uiMessage == WM_KEYDOWN));
				}

				/* Number keys also do not generate a WM_CHAR if the control key is pressed, so again we */
				/* must simulate it.   Numbers do not need to be mapped as their virtual keys are the same as */
				/* their character values */

				else if ((a_oWParam >= 0x30) && (a_oWParam <= 0x39))
				{
					Window->OfferKeyEvent(a_oWParam, (a_uiMessage == WM_KEYDOWN));
				}
			}

#endif /* _WIN32_WINNT_WIN2K */

			/* Pass the raw key onto the CWindow::OfferRawKeyEvent() function, without any kind */
			/* of preprocessing */

			Window->OfferRawKeyEvent(a_oWParam, (a_uiMessage == WM_KEYDOWN));

			break;
		}

		case WM_KILLFOCUS :
		{
			/* Destroy the native Windows cursor as the window is losing focus */

			DestroyCaret();

			break;
		}

		case WM_LBUTTONDOWN :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseDown);

			break;
		}

		case WM_LBUTTONUP :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseUp);

			break;
		}

		case WM_LBUTTONDBLCLK :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseDoubleClick);

			break;
		}

		case WM_MOUSEMOVE :
		{
			/* Extract the mouse's X and Y positions and send them to the client window */

			Window->HandlePointerEvent(LOWORD(a_oLParam), HIWORD(a_oLParam), EStdMouseMove);

			break;
		}

		case WM_MOUSEWHEEL :
		{
			/* Extract the 16 bit amount the wheel has been scrolled and ensure that it is signed */

			Window->HandleWheelEvent((short) HIWORD(a_oWParam));

			break;
		}

		case WM_PAINT :
		{
			/* Prepare the device context for painting and call the CWindow::Draw() routine. */
			/* If this fails then there isn't much we can do besides ignore the error */

			if ((Window->m_poDC = BeginPaint(a_poWindow, &Window->m_oPaintStruct)) != NULL)
			{
				Window->Draw(Window->m_oPaintStruct.rcPaint.top, Window->m_oPaintStruct.rcPaint.bottom);
				EndPaint(a_poWindow, &Window->m_oPaintStruct);

				/* And indicate that there is no longer a valid DC allocated so that nobody */
				/* tried to use it */

				Window->m_poDC = NULL;
			}

			break;
		}

		case WM_SIZE :
		{
			Window->InternalResize(LOWORD(a_oLParam), HIWORD(a_oLParam));

			break;
		}

		case WM_COPYDATA :
		{
			/* Let the RRendezvous class know that a rendevous has been received */

			CopyData = (COPYDATASTRUCT *) a_oLParam;
			g_oRendezvous.MessageReceived((unsigned char *) CopyData->lpData, CopyData->cbData);

			/* And indicate that the message has been processed */

			RetVal = 1;

			break;
		}

		case WM_HSCROLL :
		case WM_VSCROLL :
		{
			/* Iterate through the window's list of layout gadgets and search each one to see */
			/* if it contains a gadget that represents the Windows slider that was just moved */

			if ((LayoutGadget = Window->m_oGadgets.GetHead()) != NULL)
			{
				do
				{
					if ((Gadget = LayoutGadget->FindNativeGadget((void *) a_oLParam)) != NULL)
					{
						/* Got it!  Call the gadget's Updated() routine so that it can notify the */
						/* client of the update, letting it know what type of update this is */

						Gadget->Updated(LOWORD(a_oWParam));

						break;
					}
				}
				while ((LayoutGadget = Window->m_oGadgets.GetSucc(LayoutGadget)) != NULL);
			}

			break;
		}

		default :
		{
			RetVal = 1;

			break;
		}
	}

	/* If the event was not handled then call the system's default window procedure */

	if (RetVal != 0)
	{
		RetVal = DefWindowProc(a_poWindow, a_uiMessage, a_oWParam, a_oLParam);
	}

	return(RetVal);
}

#endif /* WIN32 */

/* Written: Monday 08-Feb-2010 7:19 am */

CWindow::~CWindow()
{
	Close();
}

/* Written: Wednesday 13-Oct-2010 7:29 am */

void CWindow::Activate()
{
	ASSERTM(m_poWindow, "CWindow::Activate() => Window must already be open");

#ifdef __amigaos4__

	IIntuition->SetAttrs(m_poWindowObj, WINDOW_FrontBack, WT_FRONT, TAG_DONE);
	IIntuition->SetWindowAttrs(m_poWindow, WA_Activate, TRUE, TAG_DONE);

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	DEBUGCHECK((SetActiveWindow(m_poWindow) != NULL), "CWindow::Activate() => Unable to activate window");

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

}

/**
 * Appends an accelerator to the Windows accelerator table.
 * This Windows specific function will add an accelerator to the application's current
 * accelerator table.  There is no such function in the Windows API so this function
 * retrieves a copy of the current accelerator table, appends an accelerator to it and
 * makes that new accelerator table the currently active one.
 *
 * @date	Wednesday 05-Jun-2013 6:36 am Code HQ Ehinger Tor
 * @param	a_pcoMenuItem	Ptr to the SStdMenuItem structure representing the accelerator
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available to allocate the accelerator
 */

#if defined(WIN32) && !defined(QT_GUI_LIB)

TInt CWindow::AddAccelerator(const struct SStdMenuItem *a_pcoMenuItem)
{
	TInt NumAccelerators, RetVal;
	ACCEL *Accelerators;
	HACCEL OldAccelerators;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Determine how many accelerators are in the current accelerator table and allocate */
	/* enough memory to represent those accelerators + one new one */

	NumAccelerators = CopyAcceleratorTable(m_poAccelerators, NULL, 0);

	if ((Accelerators = new ACCEL[NumAccelerators + 1]) != NULL)
	{
		/* Get a copy of the existing accelerator table into the newly allocate memory */

		NumAccelerators = CopyAcceleratorTable(m_poAccelerators, Accelerators, NumAccelerators);

		/* Append a new accelerator to the table, representing the menu item passed in */

		InitialiseAccelerator(&Accelerators[NumAccelerators], a_pcoMenuItem);

		/* Save a ptr to the old accelerator table and create a new one, only deleting the */
		/* old one if creation of the new was successful */

		OldAccelerators = m_poAccelerators;

		if ((m_poAccelerators = CreateAcceleratorTable(Accelerators, (NumAccelerators + 1))) != NULL)
		{
			RetVal = KErrNone;

			/* The new accelerator table was created successfully so delete the old one */

			DestroyAcceleratorTable(OldAccelerators);
		}

		delete [] Accelerators;
	}

	return(RetVal);
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

#ifndef __amigaos4__

/**
 * Adds a menu item to an already existing dropdown menu.
 * This is an internal function.  Client code should use the public version of CWindow::AddMenuItem(),
 * which is a wrapper around this version.  This version accepts a structure containing information about
 * the menu item to add and an OS specific handle to the dropdown menu to which to add the new item.
 *
 * @date	Thursday 23-May-2013 7:40 am Code HQ Ehinger Tor
 * @param	a_pcoMenuItem		Ptr to structure containing information such as the label, command ID etc.
 * @param	a_pvDropdownMenu	OS specific handle to the dropdown menu to which to add the menu item
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available to allocate the menu item
 */

TInt CWindow::AddMenuItem(const struct SStdMenuItem *a_pcoMenuItem, void *a_pvDropdownMenu)
{
	TInt RetVal;

	ASSERTM((a_pcoMenuItem!= NULL), "CWindow::AddMenuItem() => MenuItem structure must be specified");
	ASSERTM((a_pvDropdownMenu != NULL), "CWindow::AddMenuItem() => Menu bar must be specified");

	/* Assume success */

	RetVal = KErrNone;

#ifdef QT_GUI_LIB

	QAction *Action;
	QMenu *DropdownMenu;
	QString Shortcut;

	DropdownMenu = (QMenu *) a_pvDropdownMenu;

	/* Create a new menu item and add it to the previously created drop down menu */

	if ((Action = new CQtAction(a_pcoMenuItem->m_iCommand, a_pcoMenuItem->m_pccLabel, m_poWindow)) != NULL)
	{
		/* If this is a checkable menu option or a separator then adjust the style of the newly */
		/* added menu item to reflect this */

		if (a_pcoMenuItem->m_eType == EStdMenuCheck)
		{
			Action->setCheckable(true);
		}
		else if (a_pcoMenuItem->m_eType == EStdMenuSeparator)
		{
			Action->setSeparator(true);
		}

		/* If the menu item has a hotkey then generate a key sequence and assign it to the action */

		if (a_pcoMenuItem->m_pccHotKey)
		{
			/* The most convenient type of QKeySequence to use for our purposes is */
			/* a fully text based one, so start by converting the modifier keys to */
			/* a textual prefix */

			if ((a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_CONTROL) || (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_MENU))
			{
				Shortcut = "Ctrl+";
			}
			else if (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_ALT)
			{
				Shortcut = "Alt+";
			}
			else if (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_SHIFT)
			{
				Shortcut = "Shift+";
			}

			/* There is no modifier.  However, by default a QString really is empty */
			/* and cannot be appended to, so we must populate it with an empty string */

			else
			{
				Shortcut = "";
			}

			/* Append the hotkey name to the prefix and add the shortcut to the action */

			Shortcut.append(a_pcoMenuItem->m_pccHotKey);
			Action->setShortcut(Shortcut);
		}

		/* And add the new menu item to the drop down menu */

		DropdownMenu->addAction(Action);
	}
	else
	{
		Utils::Info("CWindow::AddMenuItem() => Unable to create menu item");

		RetVal = KErrNoMemory;
	}

#else /* ! QT_GUI_LIB */

	const char *Label;
	HMENU DropdownMenu;

	Label = NULL;
	DropdownMenu = (HMENU) a_pvDropdownMenu;

	/* If this is a separator then create a separator menu item and add it to the previously */
	/* created drop down menu */

	if (a_pcoMenuItem->m_eType == EStdMenuSeparator)
	{
		DEBUGCHECK((AppendMenu(DropdownMenu, MF_SEPARATOR, a_pcoMenuItem->m_iCommand, NULL) != FALSE),
			"CWindow::AddMenuItem() => Unable to append separator menu item");
	}

	/* Otherwise it is a "normal" menu item so create a menu item for it.  This will involve */
	/* also interpreting the requested hotkey and adding a textual string representing said */
	/* hotkey to the menu */

	else
	{
		ASSERTM((DropdownMenu != NULL), "CWindow::AddMenuItem() => Menu bar must be created before a menu item can be added");
		ASSERTM((a_pcoMenuItem->m_pccLabel != NULL), "CWindow::AddMenuItem() => All menu items must have a name");

		/* Build the menu item's label as a composite of the menu's label and its shortcut */

		if ((Label = InitialiseMenuLabel(a_pcoMenuItem)) != NULL)
		{
			/* And create a new menu item using the newly constructed label and add it to the */
			/* previously created drop down menu */

			DEBUGCHECK((AppendMenu(DropdownMenu, 0, a_pcoMenuItem->m_iCommand, Label) != FALSE),
				"CWindow::AddMenuItem() => Unable to append new menu item");

			/* And free the temporary buffer used for the menu item labels */

			delete [] (char *) Label;
		}
		else
		{
			Utils::Info("CWindow::AddMenuItem() => Unable to allocate memory for menu item label");

			RetVal = KErrNoMemory;
		}
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

#endif /* ! __amigaos4__ */

/**
 * Adds a menu item to an already existing dropdown menu.
 * This function appends a new menu item to a dropdown menu.  The label passed in can contain
 * a character preceded by a '&' character, in which case that character will be used as the
 * menu item's shortcut.  If no label is passed in then a separator will be added to the menu.
 *
 * The dropdown menu to which to add the menu item is specified as an ordinal starting from
 * zero, where zero is the leftmost dropdown menu and (NumMenus - 1) is the rightmost.
 *
 * @date	Wednesday 22-May-2013 6:23 pm, Frankfurt am Main Airport, awaiting flight TP 579 to Lisbon
 * @param	a_eMenuItemType	The type of menu item to be added
 * @param	a_pccLabel		Label to be used for the new menu item, or NULL for a separator
 * @param	a_pccHotKey		Shortcut key to be displayed, or NULL for no shortcut.  Ignored for separators
 * @param	a_iOrdinal		Ordinal offset of the dropdown menu to which to add the menu item
 * @param	a_iSubOrdinal	Ordinal offset of the submenu within the dropdown menu to which to add the
 *							menu item.  Set to -1 if no submenu is to be used
 * @param	a_iCommand		Command ID that will be passed to the window's HandleCommand() function
 * @return	KErrNone if successful
 * @return	KErrNotFound if the dropdown menu represented by a_iOrdinal was not found
 * @return	KErrNoMemory if not enough memory was available to allocate the menu item
 */

TInt CWindow::AddMenuItem(TStdMenuItemType a_eMenuItemType, const char *a_pccLabel, const char *a_pccHotKey, TInt a_iOrdinal, TInt a_iSubOrdinal, TInt a_iCommand)
{
	TInt RetVal;

#ifndef __amigaos4__

	struct SStdMenuItem MenuItem = { a_eMenuItemType, a_pccLabel, a_pccHotKey, STD_KEY_ALT, a_iCommand };

#endif /* ! __amigaos4__ */

#ifdef __amigaos4__

	/* Only perform the action if the menus have been created */

	if (m_poAmiMenus)
	{
		RetVal = m_poAmiMenus->AddItem(a_eMenuItemType, a_pccLabel, a_pccHotKey, a_iOrdinal, a_iSubOrdinal, a_iCommand);
	}
	else
	{
		RetVal = KErrNotFound;
	}

#elif defined(QT_GUI_LIB)

	QList<QAction *> Menus = m_poWindow->menuBar()->actions();
	QMenu *DropdownMenu;

	/* Get a handle to the dropdown menu to which to add the new menu item */

	if ((DropdownMenu = Menus.at(a_iOrdinal)->menu()) != NULL)
	{
		/* If the user has requested to add the menu item to a submenu then search for that now */

		if (a_iSubOrdinal != -1)
		{
			DropdownMenu = DropdownMenu->actions().at(a_iSubOrdinal)->menu();
		}

		if (DropdownMenu)
		{
			/* If there is no label then this is a separator */

			if (!(a_pccLabel))
			{
				MenuItem.m_eType = EStdMenuSeparator;
			}

			/* Add the menu item to the dropdown menu.  This will take care of appending a shortcut */
			/* key, if one has been specified */

			RetVal = AddMenuItem(&MenuItem, DropdownMenu);
		}
		else
		{
			RetVal = KErrNotFound;
		}
	}
	else
	{
		RetVal = KErrNotFound;
	}

#else /* ! QT_GUI_LIB */

	HMENU DropdownMenu;

	/* Get a handle to the dropdown menu to which to add the new menu item */

	if ((DropdownMenu = GetSubMenu(m_poMenu, a_iOrdinal)) != NULL)
	{
		/* If the user has requested to add the menu item to a submenu then search for that now */

		if (a_iSubOrdinal != -1)
		{
			DropdownMenu = GetSubMenu(DropdownMenu, a_iSubOrdinal);
		}

		if (DropdownMenu)
		{
			/* If there is no label then this is a separator */

			if (!(a_pccLabel))
			{
				MenuItem.m_eType = EStdMenuSeparator;
			}

			/* Add the menu item to the dropdown menu */

			if ((RetVal = AddMenuItem(&MenuItem, DropdownMenu)) == KErrNone)
			{
				/* If a shortcut key has been specified, then append a new accelerator to the current */
				/* accelerator table, deleting the menu entry if it fails */

				if (MenuItem.m_pccHotKey)
				{
					if ((RetVal = AddAccelerator(&MenuItem)) != KErrNone)
					{
						RemoveMenuItem(a_iOrdinal, a_iCommand);
					}
				}
			}
		}
		else
		{
			RetVal = KErrNotFound;
		}
	}
	else
	{
		RetVal = KErrNotFound;
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/* Written: Monday 11-Jul-2011 6:16 am */

void CWindow::Attach(CStdGadgetLayout *a_poLayoutGadget)
{
	ASSERTM((a_poLayoutGadget != NULL), "CWindow::Attach() => No gadget to be attached passed in");
	ASSERTM((m_poWindow != NULL), "CWindow::Attach() => Window not yet open");

	/* Add the new layout gadget to the window's list of gadgets */

	m_oGadgets.AddTail(a_poLayoutGadget);

#ifdef __amigaos4__

	/* Add the new BOOPSI gadget to the window's root layout */

	if (IIntuition->IDoMethod(m_poRootGadget, LM_ADDCHILD, NULL, a_poLayoutGadget->m_poGadget, NULL))
	{
		/* Let the layout gadget know its new width and then calcuate its new height */

		a_poLayoutGadget->m_iWidth = m_iInnerWidth;
		RethinkLayout();
	}

#elif defined(QT_GUI_LIB)

	/* For Qt there is no need to attach the layout gadget to the window as it is */
	/* done automatically when the layout gadget is created.  So simply let the layout */
	/* gadget know its new width and then calcuate its new height */

	a_poLayoutGadget->m_iWidth = m_iInnerWidth;
	RethinkLayout();

#else /* ! QT_GUI_LIB */

	/* Let the layout gadget know its new width and then calcuate its new height */

	a_poLayoutGadget->m_iWidth = m_iInnerWidth;
	RethinkLayout();

#endif /* ! QT_GUI_LIB */

}

/**
 * Enables or disables the checkmark on a checkable menu item.
 * Finds a menu item that has a command ID matching that passed in and enables
 * or disables its checkmark.  For a menu item to be checked it must have been
 * created with the type EStdMenuCheck.  It is safe to call this function on a
 * menu item that is not of this type;  in thise case it will simply do nothing.
 *
 * @date	Tuesday 10-Apr-2012 7:07 am, Code HQ Ehinger Tor
 * @param	a_iItemID	ID of the menu item to be checked or unchecked
 * @param	a_bEnable	ETrue to check the menu item, else EFalse to uncheck it
 */

void CWindow::CheckMenuItem(TInt a_iItemID, TBool a_bEnable)
{

#ifdef __amigaos4__

	/* Only perform the action if the menus have been created */

	if (m_poAmiMenus)
	{
		m_poAmiMenus->CheckItem(a_iItemID, a_bEnable);
	}

#elif defined(QT_GUI_LIB)

	CQtAction *QtAction;

	/* Map the menu item's ID onto a CQtAction object the can be used by Qt */

	if ((QtAction = FindMenuItem(a_iItemID)) != 0)
	{
		/* And enable or disable the menu item's check mark as appropriate */

		QtAction->setChecked(a_bEnable);
	}

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((::CheckMenuItem(GetMenu(m_poWindow), a_iItemID, (a_bEnable) ? MF_CHECKED : MF_UNCHECKED) != (DWORD) -1),
		"CWindow::CheckMenuItem() => Unable to set menu checkmark state");

#endif /* ! QT_GUI_LIB */

}

/**
 * Creates a set of menus specific to this window
 * Creates a set of menus specific to this window, using the list of SStdMenuItem structures
 * available from RApplication via RApplication::MenuItems().  The menus are created in here
 * rather than in RApplication as some platforms require a separate copy of the menus to be
 * created for each window opened, or they cannot create the menus at application creation time.
 *
 * @pre		The window must already have been created, although not necessarily opened
 * @pre		The menus must not yet have been created
 *
 * @date	Sunday 05-Jan-2013 7:53 am, Code HQ Ehinger Tor
 * @return	ETrue if all menus were created successfully, else EFalse
 */

TBool CWindow::CreateMenus()
{
	TBool RetVal;
	const struct SStdMenuItem *MenuItem;

	ASSERTM((m_poWindow != NULL), "CWindow::CreateMenus() => Window must be created before menus");

	/* Assume success */

	RetVal = ETrue;

	/* Iterate through the list of menu structures passed in and create a menu item for each */
	/* one as appropriate */

	MenuItem = m_poApplication->MenuItems();

#ifdef __amigaos4__

	ASSERTM((m_poAmiMenus == NULL), "CWindow::CreateMenus() => Menus can only be created once");

	if ((m_poAmiMenus = CAmiMenus::New(this, MenuItem)) == NULL)
	{
		RetVal = EFalse;
	}

#elif defined(QT_GUI_LIB)

	QMenu *DropdownMenu, *PopupMenu, *TopLevelMenu;
	QString Shortcut;

#ifdef _DEBUG

	QList<QAction *> Menus = m_poWindow->menuBar()->actions();

	ASSERTM((Menus.count() == 0), "CWindow::CreateMenus() => Menus can only be created once");

#endif /* _DEBUG */

	DropdownMenu = TopLevelMenu = NULL;

	do
	{
		/* If this is a title then create a new drop down menu to which to add menu items */

		if (MenuItem->m_eType == EStdMenuTitle)
		{
			/* And add the new drop down menu to the window's menu bar */

			if ((DropdownMenu = TopLevelMenu = m_poWindow->menuBar()->addMenu(MenuItem->m_pccLabel)) == NULL)
			{
				Utils::Info("CWindow::CreateMenus() => Unable to create drop down menu");

				RetVal = EFalse;

				break;
			}

			/* Connect a signal from the dropdown menu to our Qt window class so that we */
			/* can tell when a menu is about to be displayed */

			QObject::connect(DropdownMenu, SIGNAL(aboutToShow()), m_poWindow, SLOT(aboutToShow()));
		}

		/* If this is a submenu then create a new submenu to which to add menu items */

		else if (MenuItem->m_eType == EStdMenuSubMenu)
		{
			if ((PopupMenu = DropdownMenu->addMenu(MenuItem->m_pccLabel)) != NULL)
			{
				/* Any further menu items from here to the next dropdown menu will be added to this submenu */

				DropdownMenu = PopupMenu;
			}
			else
			{
				Utils::Info("CWindow::CreateMenus() => Unable to create submenu");

				RetVal = EFalse;

				break;
			}
		}

		/* If this is the special value that ends a submenu definition then change back to adding menu items to */
		/* the dropdown menu */

		else if (MenuItem->m_eType == EStdMenuSubMenuEnd)
		{
			DropdownMenu = TopLevelMenu;
		}

		/* Otherwise add a new menu option to an already existing dropdown menu */

		else
		{
			if (AddMenuItem(MenuItem, DropdownMenu) != KErrNone)
			{
				RetVal = EFalse;

				break;
			}
		}

		++MenuItem;
	}
	while (MenuItem->m_eType != EStdMenuEnd);

#else /* ! QT_GUI_LIB */

	char *Label;
	TInt Index, NumAccelerators;
	ACCEL *Accelerators;
	HMENU DropdownMenu, PopupMenu, TopLevelMenu;

	ASSERTM((m_poMenu == NULL), "CWindow::CreateMenus() => Menus can only be created once");

	Label = NULL;
	NumAccelerators = 0;

	/* Create a top level menubar to which drop down menus can be attached */

	if ((m_poMenu = CreateMenu()) != NULL)
	{
		DropdownMenu = TopLevelMenu = NULL;

		do
		{
			/* If this is a title then create a new drop down menu to which to add menu items */

			if (MenuItem->m_eType == EStdMenuTitle)
			{
				if ((DropdownMenu = TopLevelMenu = CreatePopupMenu()) != NULL)
				{
					DEBUGCHECK((AppendMenu(m_poMenu, MF_POPUP, (UINT_PTR) DropdownMenu, MenuItem->m_pccLabel) != FALSE),
						"CWindow::CreateMenus() => Unable to append new menu");
				}
				else
				{
					Utils::Info("CWindow::CreateMenus() => Unable to create drop down menu");

					RetVal = EFalse;

					break;
				}
			}

			/* If this is a submenu then create a new submenu to which to add menu items */

			else if (MenuItem->m_eType == EStdMenuSubMenu)
			{
				if ((PopupMenu = CreatePopupMenu()) != NULL)
				{
					DEBUGCHECK((AppendMenu(DropdownMenu, MF_POPUP, (UINT_PTR) PopupMenu, MenuItem->m_pccLabel) != FALSE),
						"CWindow::CreateMenus() => Unable to append new submenu");

					/* Any further menu items from here to the next dropdown menu will be added to this submenu */

					DropdownMenu = PopupMenu;
				}
				else
				{
					Utils::Info("CWindow::CreateMenus() => Unable to create submenu");

					RetVal = EFalse;

					break;
				}
			}

			/* If this is the special value that ends a submenu definition then change back to adding menu items to */
			/* the dropdown menu */

			else if (MenuItem->m_eType == EStdMenuSubMenuEnd)
			{
				DropdownMenu = TopLevelMenu;
			}

			/* Otherwise add a new menu option to an already existing dropdown menu */

			else
			{
				if (AddMenuItem(MenuItem, DropdownMenu) == KErrNone)
				{
					/* If this menu item contains a hotkey then increment the accelerator count for l8r use */

					if (MenuItem->m_pccHotKey)
					{
						++NumAccelerators;
					}
				}
				else
				{
					RetVal = EFalse;

					break;
				}
			}

			++MenuItem;
		}
		while (MenuItem->m_eType != EStdMenuEnd);

		/* Now add the top level menubar to the window */

		DEBUGCHECK((SetMenu(m_poWindow, m_poMenu) != FALSE), "CWindow::CreateMenus() => Unable to assign menu to window");

		/* And free the temporary buffer used for the menu item labels */

		Utils::FreeTempBuffer(Label);
	}
	else
	{
		RetVal = EFalse;

		Utils::Info("CWindow::CreateMenus() => Unable to create top level menubar");
	}

	/* If the menus were all built successfully then create some accelerators for the */
	/* requested hotkeys */

	if (RetVal)
	{
		/* First allocate an array of ACCEL structures of the number required to hold all */
		/* of the requested hotkeys */

		if ((Accelerators = new ACCEL[NumAccelerators]) != NULL)
		{
			/* Now iterate through the user's menu structures and for each that contains a */
			/* hotkey, initialise a matching ACCEL structure */

			Index = 0;
			MenuItem = m_poApplication->MenuItems();

			do
			{
				/* Does this entry have a hotkey? */

				if (MenuItem->m_pccHotKey)
				{
					/* Yes it does.  Initialise an entry to be a shortcut to the hotkey and */
					/* command specified, using the character codes passed in by the user */

					InitialiseAccelerator(&Accelerators[Index++], MenuItem);
				}

				++MenuItem;
			}
			while (MenuItem->m_eType != EStdMenuEnd);

			/* Create an accelerator table for the window */

			m_poAccelerators = CreateAcceleratorTable(Accelerators, NumAccelerators);
			delete [] Accelerators;
		}
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/**
 * Returns whether the ALT key is currently pressed.
 * Allows client code to query whether the ALT key is currently pressed, in order
 * to use that key in shortcut sequences.  Note that this function specifically
 * refers to the ALT key and not the ALT GR key, which, despite its name, is a
 * completely different key!
 *
 * @date	Wednesday 03-Apr-2013 7:2 am, Code HQ Ehinger Tor
 * @return	ETrue if the ALT key is pressed, else EFalse
 */

TBool CWindow::AltPressed()
{

#ifdef WIN32

	/* Due to Windows using the control key to simulate usage of the ALT GR */
	/* key, rather than using a dedicated keycode for ALT GR, we have to have */
	/* a workaround to differentiate between ALT being pressed and ALT GR */
	/* being pressed */

	return((m_bAltPressed) && (!(m_bCtrlPressed)));

#else /* ! WIN32 */

	return(m_bAltPressed);

#endif /* ! WIN32 */

}

/**
 * Returns whether the control key is currently pressed.
 * Allows client code to query whether the control key is currently pressed, in
 * order to use that key in shortcut sequences.
 *
 * @date	Saturday 05-Jan-2013 1:12 pm, Code HQ Ehinger Tor
 * @return	ETrue if the control key is pressed, else EFalse
 */

TBool CWindow::CtrlPressed()
{

#ifdef WIN32

	/* Due to Windows using the control key to simulate usage of the ALT GR */
	/* key, rather than using a dedicated keycode for ALT GR, we have to have */
	/* a workaround to differentiate between control being pressed and ALT GR */
	/* being pressed */

	return((m_bCtrlPressed) && (!(m_bAltPressed)));

#else /* ! WIN32 */

	return(m_bCtrlPressed);

#endif /* ! WIN32 */

}

/**
 * Brings the window to the top of the Z-Order, so it is in front of all other windows.
 * This method will move the window represented by the current instance of this class to be
 * on top of all other windows in the system.  On Amiga OS, this will also bring the window's
 * screen to the front, if the window is running on its own screen.
 *
 * @pre		The window must already have been created
 *
 * @date	Friday 18-Jul-2014 11:55 am, on board ICE 578 train to Frankfurt Flughafen
 */

void CWindow::BringToFront()
{
	ASSERTM(m_poWindow, "CWindow::BringToFront() => Window must already be open");

#ifdef __amigaos4__

	/* First bring the screen to the front */

	IIntuition->ScreenToFront(m_poWindow->WScreen);

	/* And then activate the window */

	Activate();

#elif defined(QT_GUI_LIB)

	/* The Qt version of this method is a little strange as it depends on the underlying X11 server.  Calling */
	/* QWidget::activateWindow() should normally be enough, but this does not guarantee that the X11 server will */
	/* bring the window to the front.  So we have to also call QWidget::raise() to make sure */

	m_poWindow->raise();
	m_poWindow->activateWindow();

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((SetForegroundWindow(m_poWindow) != FALSE), "CWindow::BringToFront() => Unable to set foreground window");

#endif /* ! QT_GUI_LIB */

}

/* Written: Wednesday 14-Jul-2011 6:14 am, Code HQ-by-Thames */

void CWindow::ClearBackground(TInt a_iY, TInt a_iHeight, TInt a_iX, TInt a_iWidth)
{

#ifdef __amigaos4__

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow)
	{
		/* All window drawing operations are relative to the client area, so adjust the X and Y */
		/* positions such that they point past the left and top borders */

		a_iX += m_poWindow->BorderLeft;
		a_iY += m_poWindow->BorderTop;

		IIntuition->ShadeRect(m_poWindow->RPort, a_iX, a_iY, (a_iX + a_iWidth - 1), (a_iY + a_iHeight - 1),
			LEVEL_NORMAL, BT_BACKGROUND, IDS_NORMAL, IIntuition->GetScreenDrawInfo(m_poWindow->WScreen), TAG_DONE);
	}

#else /* ! __amigaos4__ */

	// TODO: CAW - Implement this + comment this function
	(void) a_iY;
	(void) a_iHeight;
	(void) a_iX;
	(void) a_iWidth;

#endif /* ! __amigaos4__ */

}

/* Written: Monday 08-Feb-2010 7:18 am */

void CWindow::Close()
{
	CStdGadget *LayoutGadget;

#ifdef __amigaos4__

	/* Destroy the window's menus and associated resources */

	delete m_poAmiMenus;
	m_poAmiMenus = NULL;

	/* And close the window itself */

	if (m_poWindowObj)
	{
		IIntuition->DisposeObject(m_poWindowObj);
		m_poWindowObj = NULL;
	}

#elif defined(QT_GUI_LIB)

	if (m_poWindow)
	{
		/* Let the window know that it is in the process if closing so that it */
		/* accepts the close event in closeEvent() */

		m_poWindow->setClosing(true);

		/* Ensure that the QMainWindow instance is actually freed when it is closed */

		m_poWindow->setAttribute(Qt::WA_DeleteOnClose);

		/* And close the window */

		DEBUGCHECK((m_poWindow->close() != false), "CWindow::Close() => Unable to close window");
		m_poWindow = NULL;
	}

#else /* ! QT_GUI_LIB */

	/* Destroy the accelerators, if they have been created */

	if (m_poAccelerators)
	{
		DestroyAcceleratorTable(m_poAccelerators);
		m_poAccelerators = NULL;
	}

	/* If the window is open, close it and indicate that it is no longer open */

	if (m_poWindow)
	{
		DEBUGCHECK((DestroyWindow(m_poWindow) != 0), "CWindow::Close() => Unable to destroy window");
		m_poWindow = NULL;
	}

	/* If the window's class is registered, unregister it and indicate that it is no longer registered */

	if (m_poWindowClass)
	{
		DEBUGCHECK((UnregisterClass((LPCTSTR) (DWORD) m_poWindowClass, GetModuleHandle(NULL)) != FALSE), "CWindow::Close() => Unable to unregister window class");
		m_poWindowClass = 0;
	}

#endif /* ! QT_GUI_LIB */

	/* Iterate through the list of attached gadgets and delete them.  They will remove themselves */
	/* from the gadget list automatically */

	while ((LayoutGadget = m_oGadgets.GetHead()) != NULL)
	{
		delete LayoutGadget;
	}

	/* And remove the window from the application's list of windows, but only */
	/* if it was added */

	if (m_bOpen)
	{
		m_poApplication->RemoveWindow(this);
		m_bOpen = EFalse;
	}
}

/* Written: Wednesday 13-Oct-2010 7:01 am*/

void CWindow::CompleteOpen()
{

#ifdef __amigaos4__

	ASSERTM(m_poWindowObj, "CWindow::CompleteOpen() => Reaction window must already be open");

	/* Get the window's signal so that we can wait until an event occurs */

	IIntuition->GetAttr(WINDOW_Window, m_poWindowObj, (ULONG *) &m_poWindow);

#endif /* __amigaos4__ */

	/* Add this dialog to the application so that messages can be routed appropriately */

	m_poApplication->AddWindow(this);
	m_bOpen = ETrue;
}

/* Written: Saturday 29-May-2010 1:07 pm*/

void CWindow::DrawNow()
{
	/* Just determine the dimensions of the window and pass the call on, taking */
	/* into account that the bottom pixel offset is exclusive, thus adding 1 on */
	/* the platforms required */

#ifdef __amigaos4__

	DrawNow(0, (m_poWindow->BorderTop + m_iInnerHeight + 1));

#elif defined(QT_GUI_LIB)

	DrawNow(0, (m_iInnerHeight + 1));

#else /* ! QT_GUI_LIB */

	RECT Rect;

	/* Get the dimensions of the client area and adjust it to only represent the vertical */
	/* band that we wish to redraw, also adjusting the size of the area to be cleared and */
	/* drawn to take into account any attached gadgets */

	if (GetClientRect(m_poWindow, &Rect))
	{
		DrawNow(0, Rect.bottom);
	}
	else
	{
		Utils::Info("CWindow::DrawNow() => Unable to obtain client window dimensions");
	}

#endif /* ! QT_GUI_LIB */

}

/* Written: Saturday 30-Nov-2010 9:15 pm */
/* @param	a_iTop		Offset from top of client area from which to invalidate */
/*			a_iBottom	Bottom most part of client area to which to invalidate */
/*			a_iWidth	Width of client area to invalidate.  If -1 then the entire width */
/*						is invalidated.  This is useful if you don't want to redraw the */
/*						entire width of the client area for some reason */
/* Invalidates a vertical band of the client area and instigates a redraw of that area. */
/* The bottom of the area, represented by a_iBottom, is considered exclusive, so the area */
/* redrawn is between a_iTop and (a_iBottom - 1) */

void CWindow::DrawNow(TInt a_iTop, TInt a_iBottom, TInt a_iWidth)
{
	ASSERTM((a_iTop >= 0), "CWindow::DrawNow() => Y offset to draw from must not be negative");
	ASSERTM((a_iBottom >= 0), "CWindow::DrawNow() => Y offset to draw to must not be negative");
	ASSERTM((a_iTop <= a_iBottom), "CWindow::DrawNow() => Y offset start must be less than Y offset stop");

#ifdef __amigaos4__

	int Bottom, Top;

	// TODO: CAW - Temporary until we sort out refreshing the screen
	(void) a_iWidth;

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow)
	{
		/* Fill the window background with the standard background colour.  The IIntuition->ShadeRect() */
		/* function is passed the inclusive right and bottom offsets to which to draw, not the size of */
		/* the rect to draw.  We only fill the background if required to as client code can disable */
		/* this functionality */

		if (m_bFillBackground)
		{
			Top = (m_poWindow->BorderTop + a_iTop);
			Bottom = (m_poWindow->BorderTop + a_iBottom);

			/* If the bottom line is off the bottom of the client area then clip it to the bottom line */
			/* of the client area */

			if (Bottom > (m_poWindow->BorderTop + m_iInnerHeight - 1))
			{
				Bottom = (m_poWindow->BorderTop + m_iInnerHeight - 1);
			}

			/* Now fill in the background before drawing */

			//IIntuition->ShadeRect(m_poWindow->RPort, m_poWindow->BorderLeft, Top,
			//	  (m_poWindow->BorderLeft + m_iInnerWidth - 1), Bottom,
			//	  LEVEL_NORMAL, BT_BACKGROUND, IDS_NORMAL, IIntuition->GetScreenDrawInfo(m_poWindow->WScreen), TAG_DONE);
		}
	}

	/* And call the derived rendering function to perform a redraw immediately */

	Draw(a_iTop, a_iBottom);

#elif defined(QT_GUI_LIB)

	QWidget *CentralWidget;

	/* If no width was passed in then we want to redraw the entire width of the client area */

	if (a_iWidth == -1)
	{
		a_iWidth = m_iInnerWidth;
	}

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow)
	{
		CentralWidget = m_poWindow->centralWidget();
		ASSERTM((CentralWidget != NULL), "CWindow::DrawNow() => Central widget has not been assigned to window");

		/* And invalidate the vertical band.  Liken Windows, this will defer the dredrawing until l8r, */
		/* possibly coalescing multiple redraws into one */

		CentralWidget->update(0, a_iTop, a_iWidth, a_iBottom);
	}

#elif defined(WIN32)

	RECT Rect;

	/* Get the dimensions of the client area and adjust it to only represent the vertical */
	/* band that we wish to redraw, also adjusting the size of the area to be cleared and */
	/* drawn to take into account any attached gadgets */

	if (GetClientRect(m_poWindow, &Rect))
	{
		Rect.bottom = (Rect.top + a_iBottom);
		Rect.top += a_iTop;
		Rect.right = (a_iWidth != -1) ? a_iWidth : m_iInnerWidth;

		/* And invalidate the vertical band */

		InvalidateRect(m_poWindow, &Rect, m_bFillBackground);
	}
	else
	{
		Utils::Info("CWindow::DrawNow() => Unable to obtain client window dimensions");
	}

#endif /* WIN32 */

}

/**
 * Enables or disables a menu item.
 * Searches for the menu item identified by a_iItemID and enables or disables it.
 *
 * @date	Sunday 08-Apr-2011 8:50 am, Code HQ Ehinger Tor
 * @param	a_iItemID	ID of the menu item to be enabled or disabled
 * @param	a_bEnable	ETrue to enable the menu item, else EFalse to disable it
 */

void CWindow::EnableMenuItem(TInt a_iItemID, TBool a_bEnable)
{

#ifdef __amigaos4__

	/* Only perform the action if the menus have been created */

	if (m_poAmiMenus)
	{
		m_poAmiMenus->EnableItem(a_iItemID, a_bEnable);
	}

#elif defined(QT_GUI_LIB)

	CQtAction *QtAction;

	/* Map the menu item's ID onto a CQtAction instance the can be used by Qt */

	if ((QtAction = FindMenuItem(a_iItemID)) != 0)
	{
		/* And enable or disable the menu item as appropriate */

		QtAction->setEnabled(a_bEnable);
	}

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((::EnableMenuItem(GetMenu(m_poWindow), a_iItemID, (a_bEnable) ? MF_ENABLED : MF_DISABLED) != -1),
		"CWindow::EnableMenuItem() => Unable to set menu item state");

#endif /* ! QT_GUI_LIB */

}

#ifdef __amigaos4__

/* Written: Saturday 06-Nov-2010 8:27 am */

struct Screen *CWindow::GetRootWindowScreen()
{
	struct Window *Window;

	/* Get a ptr to the root window's underlying Amiga OS window, taking into account that there */
	/* may not be a root window */

	Window = (g_poRootWindow) ? g_poRootWindow->m_poWindow : NULL;

	/* If there was a root window then return that window's Screen */

	return((Window) ? Window->WScreen : NULL);
}

/* Written: Monday 08-Feb-2010 7:33 am */

ULONG CWindow::GetSignal()
{
	return((m_poWindow) ? (1 << m_poWindow->UserPort->mp_SigBit) : 0);
}

/**
 * Returns a ptr to the window's Amiga specific menu structure.
 * This is an internal function that should only be used by The Framework.  It returns a ptr
 * to the window's underlying Intuition menus and it is required for certain internal menu
 * handling by The Framework.
 *
 * @date	Tuesday 22-Oct-2013 7:28 am, Code HQ Ehinger Tor
 * @return	A ptr to the window's Intuition menus, if created, else NULL
 */

struct Menu *CWindow::Menus()
{
	return((m_poAmiMenus) ? m_poAmiMenus->Menus() : NULL);
}

#elif defined(QT_GUI_LIB)

/**
 * Finds a menu item that matches a particular ID.
 * This is an internal function that searches through Qt's list of menu items
 * for one with a particular ID.  If found, the CQtAction instance associated
 * with this menu item is returned.  This function is useful for mapping Windows
 * style command IDs onto their matching menu item, in order to perform actions
 * on the menu item such as enabling or disabling them.
 *
 * @date	Tuesday 19-Mar-2013 6:42 am, Code HQ Ehinger Tor
 * @param	a_iItemID	Command ID of the menu item for which to search
 * @return	Ptr to the menu's CQtAction if successful, else NULL
 */

CQtAction *CWindow::FindMenuItem(TInt a_iItemID)
{
	TInt MenuIndex, MenuItemIndex, SubMenuItemIndex;
	CQtAction *QtAction, *RetVal;
	QAction *Action;
	QList<QAction *> Menus = m_poWindow->menuBar()->actions();
	QMenu *Menu;

	/* Assume failure */

	RetVal = NULL;

	/* Qt has a rather odd way of storing its menu items.  In theory you can call */
	/* QObject::findChildren<QMenu>() on the menu bar and get a list of QMenu objects */
	/* (one for each drop down menu) and you can then call findChildren() again on */
	/* each of these to find the QMenu items attached to those drop down menus.  However */
	/* in practice this doesn't seem to work, and so we have to query the menu bar for */
	/* its actions, convert the actions into QMenu ptrs and then query those for their */
	/* actions, which we can then use to enable or disable the menu items.  This seems */
	/* a rather odd way of doing things but it works.  Start by iterating through the */
	/* list of drop down menus */

	for (MenuIndex = 0; MenuIndex < Menus.count(); ++MenuIndex)
	{
		if ((Menu = Menus.at(MenuIndex)->menu()) != NULL)
		{
			/* Now get the list of actions associated with the drop down menu and */
			/* iterate through those */

			QList<QAction *> MenuItems = Menu->actions();

			for (MenuItemIndex = 0; MenuItemIndex < MenuItems.count(); ++MenuItemIndex)
			{
				/* Get the action and convert it to our derived CQtAction class */

				Action = MenuItems.at(MenuItemIndex);
				QtAction = (CQtAction *) Action;

				/* See if the menu item contains a list of submenu items.  If so then we want */
				/* to iterate through these as well */

				if (QtAction->menu())
				{
					/* Get the list of actions associated with the submenu and iterate through those */

					QList<QAction *> SubMenuItems = Action->menu()->actions();

					for (SubMenuItemIndex = 0; SubMenuItemIndex < SubMenuItems.count(); ++SubMenuItemIndex)
					{
						/* Get the action and convert it to our derived CQtAction class */

						Action = SubMenuItems.at(SubMenuItemIndex);
						QtAction = (CQtAction *) Action;

						/* If it is the menu item we are interested in then return it */

						if (QtAction->Command() == a_iItemID)
						{
							RetVal = QtAction;

							break;
						}
					}
				}

				/* If we found a submenu item in the step above then break out of the loop */

				if (RetVal)
				{
					break;
				}

				/* Otherwise check the current menu item to see if it is the one we are interested in */

				else
				{
					/* If it is the menu item we are interested in then return it */

					if (QtAction->Command() == a_iItemID)
					{
						RetVal = QtAction;

						break;
					}
				}
			}
		}
	}

	return(RetVal);
}

#endif /* QT_GUI_LIB */

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Initialises an ACCEL structure with a shortcut and command ID.
 * This Windows specific function will initialise an ACCEL structure to represent the
 * shortcut key and command ID specified by a generic SStdMenuItem structure.  A function
 * is required as the shortcut key specified in SStdMenuItem can be somewhat complex and
 * this function is called from more than one place internally.
 *
 * @date	Thursday 06-Jun-2013 6:43 am Code HQ Ehinger Tor
 * @param	a_poAccelerator	Ptr to the ACCEL structure to be initialised
 * @param	a_pcoMenuItem	Ptr to the generic SStdMenuItem structure with which to initialise
 *							the accelerator
 */

void CWindow::InitialiseAccelerator(ACCEL *a_poAccelerator, const struct SStdMenuItem *a_pcoMenuItem)
{
	TInt FunctionKey;

	/* Initialise the accelerator with the shortcut key and command to be triggered when */
	/* it is selected.  For the sake of simplicity we work only with virtual keycodes, so */
	/* translate the ASCII value to a virtual key */

	a_poAccelerator->fVirt = FVIRTKEY;
	a_poAccelerator->key = (unsigned char) VkKeyScan(*a_pcoMenuItem->m_pccHotKey);
	a_poAccelerator->cmd = (WORD) a_pcoMenuItem->m_iCommand;

	/* If the hotkey also has a qualifier then add this to the ACCEL structure */

	if ((a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_CONTROL) || (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_MENU))
	{
		a_poAccelerator->fVirt |= FCONTROL;
	}
	else if (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_ALT)
	{
		a_poAccelerator->fVirt |= FALT;
	}
	else if (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_SHIFT)
	{
		a_poAccelerator->fVirt |= FSHIFT;
	}

	/* For function keys such as "F3" we need to convert the textual value passed in to a */
	/* virtual keycode */

	if (strlen(a_pcoMenuItem->m_pccHotKey) >= 2)
	{
		/* Is it a function key with a numeric value following it? */

		if ((toupper(a_pcoMenuItem->m_pccHotKey[0]) == 'F') && (Utils::StringToInt(&a_pcoMenuItem->m_pccHotKey[1], &FunctionKey) == KErrNone))
		{
			/* Does the numeric value represent a valid function key? */

			if ((FunctionKey >= 1) && (FunctionKey <= 12))
			{
				/* Everything is valid.  Convert the key to a virtual keycode and add */
				/* it to the accelerator list */

				FunctionKey += (VK_F1 - 1);
				a_poAccelerator->key = (WORD) FunctionKey;
			}
		}
	}
}

/**
 * Builds a string representing a menu item's label and its associated hotkey.
 * This is a convenience function for use by other functions that are creating or
 * modifying menu items.  It will take a SStdMenuItem structure and will build a
 * disablayable string that represents the contents of that structure.  This string
 * is in a format suitable for use in dropdown menu items.
 *
 * It is the responsibility of the caller to free the returned string's buffer.
 *
 * @date	Friday 21-Jun-2013 8:52 am
 * @param	a_pcoMenuItem	Ptr to the structure representing the label to be created.
 *							Only the m_pccLabel, m_pccHotKey and m_iHotKeyModifier members
 *							are used
 * @return	Ptr to a string containing the label if successful, else NULL if no memory
 */

const char *CWindow::InitialiseMenuLabel(const struct SStdMenuItem *a_pcoMenuItem)
{
	char *RetVal;
	TInt Length;

	/* Determine the length required for the menu item label.  This will be the length of the */
	/* base label plus a tab, the length of the qualifier (maximum 6 for "shift+"), the length */
	/* of the hotkey shortcut as a string and a NULL terminator */

	Length = (strlen(a_pcoMenuItem->m_pccLabel) + 8);

	if (a_pcoMenuItem->m_pccHotKey)
	{
		Length += strlen(a_pcoMenuItem->m_pccHotKey);
	}

	/* Allocate a temporary buffer in which to build the menu item's label */

	if ((RetVal = new char[Length]) != NULL)
	{
		/* Now build the menu item's label as a composite of the label's base name, a */
		/* tab, a modifier key and the textual name of the shortcut key */

		strcpy(RetVal, a_pcoMenuItem->m_pccLabel);

		/* If the menu item has a hotkey then generate a key sequence and append it to the label */

		if (a_pcoMenuItem->m_pccHotKey)
		{
			strcat(RetVal, "\t");

			if ((a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_CONTROL) || (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_MENU))
			{
				strcat(RetVal, "Ctrl+");
			}
			else if (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_ALT)
			{
				strcat(RetVal, "Alt+");
			}
			else if (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_SHIFT)
			{
				strcat(RetVal, "Shift+");
			}

			strcat(RetVal, a_pcoMenuItem->m_pccHotKey);
		}
	}

	return(RetVal);
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

/* Written: Sunday 01-May-2011 10:48 am */
/* @param	a_iInnerWidth	New width of client area */
/*			a_iInnerHeight	New height of client area */
/* This function is called whenever the size of the window changes and will reposition any auto-position */
/* gadgets and will notify the derived window class that the size has changed. */

void CWindow::InternalResize(TInt a_iInnerWidth, TInt a_iInnerHeight)
{
	TInt OldInnerWidth, OldInnerHeight;

	/* Save the old width & height for l8r */

	OldInnerWidth = m_iInnerWidth;
	OldInnerHeight = m_iInnerHeight;

	/* Save the new width & height for l8r */

	m_iInnerWidth = a_iInnerWidth;
	m_iInnerHeight = a_iInnerHeight;

	/* Don't let the client area get negative in size or it will require too much fiddling around */
	/* in the client program.  Just pretend it's 0 x 0 at the smallest */

	if (m_iInnerWidth < 0)
	{
		m_iInnerWidth = 0;
	}

	if (m_iInnerHeight < 0)
	{
		m_iInnerHeight = 0;
	}

	/* Now let the derived window class know that the window's size has changed */

	Resize(OldInnerWidth, OldInnerHeight);
}

/**
 * Returns whether the window is currently active.
 * Allows client code to query whether the window is the currently active window.  ie.  Whether it
 * currently has keyboard and mouse focus.  This is useful for things such as deciding whether to draw
 * the cursor.
 *
 * @date	Saturday 11-Jul-2015 07:32 am, Code HQ Ehinger Tor
 * @return	ETrue if the window is currently active, else EFalse
 */

TBool CWindow::IsActive()
{
	return(m_bIsActive);
}

/**
 * Determines whether a menu item is checked.
 * Finds a menu item that has a command ID matching that passed in and returns
 * whether or not it is checked.  For a menu item to be able to be checked it
 * must have been created with the type EStdMenuCheck.  It is safe to call this
 * function on a menu item that is not of this type;  in thise case it will
 * always return EFalse.
 *
 * @date	Sunday 05-May-2013 8:25 am, Code HQ Ehinger Tor
 * @param	a_iItemID	ID of the menu item to be queried
 * @return	ETrue if the item is checked, else EFalse
 */

TBool CWindow::MenuItemChecked(TInt a_iItemID)
{
	TBool RetVal;

	/* Assume the menu item is not checked */

	RetVal = EFalse;

#ifdef __amigaos4__

	/* Only perform the action if the menus have been created */

	if (m_poAmiMenus)
	{
		RetVal = m_poAmiMenus->ItemChecked(a_iItemID);
	}

#elif defined(QT_GUI_LIB)

	CQtAction *QtAction;

	/* Map the menu item's ID onto a CQtAction object the can be used by Qt */

	if ((QtAction = FindMenuItem(a_iItemID)) != NULL)
	{
		/* And determine whether the menu item is checked */

		RetVal = QtAction->isChecked();
	}

#else /* ! QT_GUI_LIB */

	MENUITEMINFO MenuItemInfo;

	/* Query the checkmark state of the menu item */

	memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));
	MenuItemInfo.cbSize = sizeof(MenuItemInfo);
	MenuItemInfo.fMask = (MIIM_CHECKMARKS | MIIM_STATE);

	if (GetMenuItemInfo(m_poMenu, a_iItemID, FALSE, &MenuItemInfo))
	{
		/* And determine whether the menu item is checked */

		if (MenuItemInfo.fState & MFS_CHECKED)
		{
			RetVal = ETrue;
		}
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/**
 * Opens a window suitable for use as the application's main window.
 * Creates and opens a window that is suitable for use as the primary window of an
 * application.  This function takes into account the differences between supported
 * platforms and attempts to choose a best set of attributes for the window opened.
 * Of course these attributes reflect the opinion of the author of what constitutes a
 * good set of window attributes and so you may not agree with them.  ;-)  For example,
 * all windows are opened full screen by default.
 *
 * @date	Monday 08-Feb-2010 7:13 am
 * @param	a_pccTitle		Ptr to a string to be placed into the title bar of the window
 * @param	a_pccScreenName	Ptr to the name of the screen on which to open the window.  Amiga
 *							OS only - this is ignored on other platforms
 * @param	a_bResizeable	ETrue to enable resizing of the window.  Amiga OS only - on other
 *							platforms windows are always resizeable
 * @return	KErrNone if the window was opened successfully
 * @return	KErrNoMemory if there was not enough memory to open the window
 * @return	KErrGeneral if an operating system specific error occurred
 */

TInt CWindow::Open(const char *a_pccTitle, const char *a_pccScreenName, TBool a_bResizeable)
{
	TInt RetVal;

	/* Assume failure */

	RetVal = KErrGeneral;

#ifdef __amigaos4__

	struct SRect ScreenSize;

	/* Get the size of the screen so we can open the window filling its full size */

	Utils::GetScreenSize(ScreenSize);

	ASSERTM((a_pccScreenName != NULL), "CWindow::Open() => Screen name must be specified");

	/* Setup an IDCMP hook that can be used for monitoring gadgets for extra information not */
	/* provided by Reaction, such as the movement of proportional gadgets */

	m_oIDCMPHook.h_Entry = (ULONG (*)()) IDCMPFunction;
	m_oIDCMPHook.h_Data = this;

	/* Create a Reaction Window and open it on the requested screen at the maximum size of */
	/* the screen.  If no screen name is specified, fall back to the Workbench */

	m_poWindowObj = (Object *) WindowObject,
		WA_Title, (ULONG) a_pccTitle, WINDOW_Position, WPOS_CENTERSCREEN,
		WA_PubScreenName, a_pccScreenName, WA_PubScreenFallBack, TRUE,
		WA_Width, ScreenSize.m_iWidth, WA_Height, ScreenSize.m_iHeight, WA_Activate, TRUE,
		WA_CloseGadget, TRUE, WA_DepthGadget, TRUE, WA_DragBar, TRUE, WA_ReportMouse, TRUE, WA_SizeGadget, a_bResizeable,
		WINDOW_IDCMPHook, &m_oIDCMPHook, WINDOW_IDCMPHookBits, (IDCMP_EXTENDEDMOUSE | IDCMP_IDCMPUPDATE),
		WA_IDCMP, (IDCMP_CLOSEWINDOW | IDCMP_EXTENDEDMOUSE | IDCMP_IDCMPUPDATE | IDCMP_MENUPICK | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY | IDCMP_REFRESHWINDOW | IDCMP_NEWSIZE),

		WINDOW_Layout, m_poRootGadget = (Object *) VGroupObject,
			/* This is an empty group into which can be placed BOOPSI objects */
		EndGroup,
	EndWindow;

	if (m_poWindowObj)
	{
		if (RA_OpenWindow(m_poWindowObj))
		{
			/* Get a ptr to the underlying Intuition Window, as it is handy to have for such */
			/* things as obtaining the window's signal bit */

			IIntuition->GetAttr(WINDOW_Window, m_poWindowObj, (ULONG *) &m_poWindow);

			/* And create the menus specific to this window */

			if (CreateMenus())
			{
				/* Indicate success */

				RetVal = KErrNone;

				/* Calculate the inner width and height of the window, for l8r use */

				m_iInnerWidth = (m_poWindow->Width - (m_poWindow->BorderRight + m_poWindow->BorderLeft));
				m_iInnerHeight = (m_poWindow->Height - (m_poWindow->BorderBottom + m_poWindow->BorderTop));

				/* And now bring the window's screen to the front, so that if an instance of the client */
				/* application is already running on the screen, it will come to the front.  Without this, */
				/* the screen would only come to the front when the first instance of the application was */
				/* launched */

				IIntuition->ScreenToFront(m_poWindow->WScreen);
			}
			else
			{
				Utils::Info("CWindow::Open() => Unable to create menus for window");
			}
		}
		else
		{
			Utils::Info("CWindow::Open() => Unable to open window");
		}

		/* If any error has occurred then clean up after ourselves */

		if (RetVal != KErrNone)
		{
			IIntuition->DisposeObject(m_poWindowObj);
			m_poWindowObj = NULL;
		}
	}
	else
	{
		Utils::Info("CWindow::Open() => Unable to create window");
	}

#elif defined(QT_GUI_LIB)

	(void) a_pccScreenName;
	(void) a_bResizeable;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Allocate a window based on the QMainWindow class, passing in the preferred size */
	/* to use for the window when in non maximised state */

	QSize DesktopSize = m_poApplication->Application()->desktop()->screenGeometry(-1).size();

	if ((m_poWindow = new CQtWindow(this, DesktopSize)) != NULL)
	{
		/* We also need a QWidget based class to use as the so-called "central widget" */

		if ((m_poCentralWidget = new CQtCentralWidget(this)) != NULL)
		{
			RetVal = KErrNone;

			/* Set the window's title to the one passed in */

			QString Title(a_pccTitle);
			m_poWindow->setWindowTitle(Title);

			/* Assign the widget as the main window's central widget */

			m_poWindow->setCentralWidget(m_poCentralWidget);

			/* And create the menus specific to this window */

			if (CreateMenus())
			{
				/* Set the position of the window to the top left of the screen so that */
				/* it does not appear at a random position when set to non maximised */

				m_poWindow->move(QPoint(0, 0));

				/* Display the window maximised */

				m_poWindow->showMaximized();

				/* And save the size of the client area */

				QSize Size = m_poCentralWidget->size();
				m_iInnerWidth = Size.width();
				m_iInnerHeight = Size.height();
			}
			else
			{
				Utils::Info("CWindow::Open() => Unable to create menus for window");
			}
		}
		else
		{
			Utils::Info("CWindow::Open() => Not enough memory to create central widget");
		}
	}
	else
	{
		Utils::Info("CWindow::Open() => Not enough memory to create window");
	}

#elif defined(WIN32)

	HINSTANCE Instance;
	RECT Rect;
	WNDCLASS WndClass;

	(void) a_pccScreenName;
	(void) a_bResizeable;

	/* Populate a WNDCLASS structure in preparation for registering the window class */

	Instance = GetModuleHandle(NULL);
	WndClass.style = CS_DBLCLKS;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.cbClsExtra = 10;
	WndClass.cbWndExtra = 20;
	WndClass.hInstance = Instance;
	WndClass.hIcon = 0;
	WndClass.hCursor = LoadCursor (0, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	WndClass.lpszMenuName = MAKEINTRESOURCE(101);
	WndClass.lpszClassName = a_pccTitle;

	/* Register the window class and open the window */

	if ((m_poWindowClass = RegisterClass(&WndClass)) != 0)
	{
		/* Determine the size of the desktop window so that we can open the window taking up the */
		/* entire size of the screen, but minus that used by the system taskbar.  This is why we */
		/* use SystemParametersInfo() instead of GetClientRect() */

		if (SystemParametersInfo(SPI_GETWORKAREA, 0, &Rect, 0))
		{
			if ((m_poWindow = CreateWindow(a_pccTitle, a_pccTitle, WS_OVERLAPPEDWINDOW, Rect.left, Rect.top,
				Rect.right, Rect.bottom, NULL, NULL, Instance, NULL)) != NULL)
			{
				/* Save a ptr to the window handle for use in the WindowProc() routine */

				SetWindowLong(m_poWindow, GWL_USERDATA, (long) this);

				/* And create the menus specific to this window */

				if (CreateMenus())
				{
					/* And display the window on the screen, maximised */

					ShowWindow(m_poWindow, SW_MAXIMIZE);

					if (GetClientRect(m_poWindow, &Rect))
					{
						/* Indicate success */

						RetVal = KErrNone;

						/* And save the size of the client area */

						m_iInnerWidth = (Rect.right - Rect.left);
						m_iInnerHeight = (Rect.bottom - Rect.top);
					}
					else
					{
						Utils::Info("CWindow::Open() => Unable to obtain window client dimensions");
					}
				}
				else
				{
					Utils::Info("CWindow::Open() => Unable to create menus for window");
				}
			}
			else
			{
				Utils::Info("CWindow::Open() => Unable to open window");
			}
		}
		else
		{
			Utils::Info("CWindow::Open() => Unable to determine size of desktop");
		}
	}
	else
	{
		Utils::Info("CWindow::Open() => Unable to register window class");
	}

#endif /* WIN32 */

	/* If everything went well, perform general postamble window opening work */

	if (RetVal == KErrNone)
	{
		CompleteOpen();
	}

	/* Otherwise clean up whatever resources were allocated */

	else
	{
		Close();
	}

	return(RetVal);
}

/* Written: Saturday 05-Nov-2011 9:03 am, Code HQ Söflingen */

void CWindow::Remove(CStdGadgetLayout *a_poLayoutGadget)
{
	ASSERTM((a_poLayoutGadget != NULL), "CWindow::Remove() => No gadget to be removed passed in");

	/* Remove the layout gadget from this window's private list of layout gadgets */

	m_oGadgets.Remove(a_poLayoutGadget);

#ifdef __amigaos4__

	/* Remove it from the Reaction layout */

	DEBUGCHECK((IIntuition->IDoMethod(m_poRootGadget, LM_REMOVECHILD, NULL, a_poLayoutGadget->m_poGadget, NULL) != 0),
		"CWindow::Remove() => Unable to remove layout gadget from window");

#endif /* __amigaos4__ */

	/* And rethink the layout to reflect the change */

	RethinkLayout();
}

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Removes an accelerator from an accelerator table.
 * This function removes an accelerator from an accelerator table.  The accelerator to be removed
 * is found by iterating through the existing accelerator list and looking for an accelerator
 * with the specified command ID.
 *
 * @date	Thursday 13-Jun-2013 7:41 am, Code HQ Ehinger Tor
 * @param	a_iCommand	Command ID of the accelerator to be removed
 * @param	KErrNone if successful
 * @param	KErrNotFound if the specified accelerator was not found
 */

TInt CWindow::RemoveAccelerator(TInt a_iCommand)
{
	TInt Index, NumAccelerators, RemainingAccelerators, RetVal;
	ACCEL *Accelerators;
	HACCEL OldAccelerators;

	/* Assume failure */

	RetVal = KErrNotFound;

	/* Determine how many accelerators are in the current accelerator table and allocate */
	/* enough memory to represent those accelerators */

	NumAccelerators = CopyAcceleratorTable(m_poAccelerators, NULL, 0);

	if ((Accelerators = new ACCEL[NumAccelerators]) != NULL)
	{
		/* Get a copy of the existing accelerator table into the newly allocate memory */

		NumAccelerators = CopyAcceleratorTable(m_poAccelerators, Accelerators, NumAccelerators);

		/* Iterate through the accelerator table until we find the one we are looking for */
		/* and when found, remove it */

		for (Index = 0; Index < NumAccelerators; ++Index)
		{
			if (Accelerators[Index].cmd == a_iCommand)
			{
				RemainingAccelerators = (NumAccelerators - Index - 1);
				memcpy(&Accelerators[Index], &Accelerators[Index + 1], (RemainingAccelerators * sizeof(ACCEL)));

				break;
			}
		}

		/* Only recreate the accelerator table if the accelerator was found */

		if (Index < NumAccelerators)
		{
			/* Save a ptr to the old accelerator table and create a new one, only deleting the */
			/* old one if creation of the new was successful */

			OldAccelerators = m_poAccelerators;

			if ((m_poAccelerators = CreateAcceleratorTable(Accelerators, (NumAccelerators - 1))) != NULL)
			{
				RetVal = KErrNone;

				/* The new accelerator table was created successfully so delete the old one */

				DestroyAcceleratorTable(OldAccelerators);
			}
		}

		delete [] Accelerators;
	}

	return(RetVal);
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

/**
 * Removes a menu item from a dropdown menu.
 * This function removes a menu item from a dropdown menu.  The dropdown menu from which to remove the
 * menu item is specified as an ordinal starting from zero, where zero is the leftmost dropdown menu
 * and (NumMenus - 1) is the rightmost.
 *
 * @date	Wednesday 29-May-2013 6:50 am Code Ehinger Tor
 * @param	a_iOrdinal	Ordinal offset of the dropdown menu from which to remove the menu item
 * @param	a_iCommand	Command ID of the menu item to be removed
 */

void CWindow::RemoveMenuItem(TInt a_iOrdinal, TInt a_iCommand)
{

#ifdef __amigaos4__

	(void) a_iOrdinal;

	/* Only perform the action if the menus have been created */

	if (m_poAmiMenus)
	{
		m_poAmiMenus->RemoveItem(a_iCommand);
	}

#elif defined(QT_GUI_LIB)

	TInt Index;
	CQtAction *Action;
	QList<QAction *> Menus = m_poWindow->menuBar()->actions();
	QMenu *DropdownMenu;

	/* Get a ptr to the dropdown menu from which to remove the menu item */

	if ((DropdownMenu = Menus.at(a_iOrdinal)->menu()) != NULL)
	{
		QList<QAction *> Actions = DropdownMenu->actions();

		/* Iterate through the list of actions attached to the dropdown menu and */
		/* find the specified command ID */

		for (Index = 0; Index < Actions.count(); ++Index)
		{
			if ((Action = (CQtAction *) Actions.at(Index)) != NULL)
			{
				/* If this is the action then remove the menu item and delete it */

				if (Action->Command() == a_iCommand)
				{
					DropdownMenu->removeAction(Action);
					delete Action;

					break;
				}
			}
		}
	}

#else /* ! QT_GUI_LIB */

	HMENU DropdownMenu;

	/* Get a handle to the dropdown menu from which to remove the menu item */

	if ((DropdownMenu = GetSubMenu(m_poMenu, a_iOrdinal)) != NULL)
	{
		DEBUGCHECK((RemoveMenu(DropdownMenu, a_iCommand, MF_BYCOMMAND) != FALSE),
			"CWindow::RemoveMenuItem() => Unable to remove menu item");

		DEBUGCHECK((RemoveAccelerator(a_iCommand) == KErrNone), "CWindow::RemoveMenuItem() => Unable to remove accelerator");
	}

#endif /* ! QT_GUI_LIB */

}

/* Written: Saturday 05-Nov-2011 12:04 pm, Code HQ Söflingen */

void CWindow::RethinkLayout()
{
	CStdGadgetLayout *LayoutGadget;

#ifdef __amigaos4__

	/* Rethink the layout gadget sizes, starting from the root gadget that holds them */

	ILayout->RethinkLayout((struct Gadget *) m_poRootGadget, m_poWindow, NULL, TRUE);

	/* Now iterate through the framework's gadgets and let them know they have been resized */

	LayoutGadget = m_oGadgets.GetHead();

	while (LayoutGadget)
	{
		LayoutGadget->RethinkLayout();
		LayoutGadget = m_oGadgets.GetSucc(LayoutGadget);
	}

#else /* ! __amigaos4__ */

	TInt InnerHeight, Height, MinHeight, Y;

	Y = 0;
	InnerHeight = m_iInnerHeight;
	LayoutGadget = m_oGadgets.GetHead();

	if (LayoutGadget)
	{
		while (LayoutGadget)
		{
			if (LayoutGadget->Weight() == 1)
			{
				/* CStdGadgetLayout::MinHeight() is expensive so cache the result */

				MinHeight = LayoutGadget->MinHeight();

				LayoutGadget->m_iHeight = MinHeight;
				InnerHeight -= MinHeight;
			}
			else
			{
				LayoutGadget->m_iHeight = -1;
			}

			LayoutGadget = m_oGadgets.GetSucc(LayoutGadget);
		}

		Height = (m_iInnerHeight / m_oGadgets.Count()); // TODO: CAW - What about remainder for last layout gadget?

		LayoutGadget = m_oGadgets.GetHead();

		while (LayoutGadget)
		{
			LayoutGadget->m_iY = Y;
			LayoutGadget->m_iWidth = m_iInnerWidth;

			if (LayoutGadget->Weight() == 1)
			{
				Y += LayoutGadget->MinHeight();
			}
			else if (LayoutGadget->Weight() == 50)
			{
				LayoutGadget->m_iHeight = Height;
				Y += Height;
			}
			else
			{
				LayoutGadget->m_iHeight = InnerHeight;
				Y += InnerHeight;
			}

			LayoutGadget = m_oGadgets.GetSucc(LayoutGadget);
		}

		LayoutGadget = m_oGadgets.GetHead();

		while (LayoutGadget)
		{
			LayoutGadget->RethinkLayout();
			LayoutGadget = m_oGadgets.GetSucc(LayoutGadget);
		}
	}

#endif /* ! __amigaos4__ */

}

/**
 * Sets the position and size of the cursor to be displayed.
 * On systems that use a native cursor, this function will set the position and size of
 * the cursor.  It will then move the native cursor to the position specified.  On systems
 * that do not have a native cursor, this function does nothing.
 *
 * @date	Monday 05-Jan-2015 6:44 am, Leoneck Hotel Zurich
 * @param	a_iX		X position at which to display the cursor
 * @param	a_iY		Y position at which to display the cursor
 * @param	a_iHeight	Height of the cursor in pixels
 */

void CWindow::SetCursorInfo(TInt a_iX, TInt a_iY, TInt a_iHeight)
{
	/* Save the the position of the cursor */

	m_iCursorX = a_iX;
	m_iCursorY = a_iY;

#if defined(WIN32) && !defined(QT_GUI_LIB)

	/* If the size of the cursor has changed, create a new native Windows cursor in the requested size */

	if (m_iCursorHeight != a_iHeight)
	{
		if (CreateCaret(m_poWindow, NULL, 0, a_iHeight))
		{
			ShowCaret(m_poWindow);
		}
	}

	/* And show the cursor at its requested position */

	SetCaretPos(a_iX, a_iY);

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	m_iCursorHeight = a_iHeight;
}

/**
 * Returns whether the shift key is currently pressed.
 * Allows client code to query whether the shift key is currently pressed, in
 * order to use that key in shortcut sequences.
 *
 * @date	Tuesday 11-Feb-2014 6:15 am, Code HQ Ehinger Tor
 * @return	ETrue if the shift key is pressed, else EFalse
 */

TBool CWindow::ShiftPressed()
{
	return(m_bShiftPressed);
}

/**
 * Modifies a menu item in a dropdown menu.
 * This function updates an already existing menu item in a dropdown menu.  The dropdown menu
 * in which to update the menu item is specified as an ordinal starting from zero, where zero
 * is the leftmost dropdown menu and (NumMenus - 1) is the rightmost.
 *
 * @date	Wednesday 19-Jun-2013 8:16 am, Henry's Kaffee Welt Ulm
 * @param	a_pccLabel	Ptr to a string containing the new label to assign to the menu item
 * @param	a_pccHotKey	Ptr to a string containing the new hotkey to assign to the menu item
 * @param	a_iOrdinal	Ordinal offset of the dropdown menu in which to update the menu item
 * @param	a_iCommand	Command ID of the menu item to be updated
 */

void CWindow::UpdateMenuItem(const char *a_pccLabel, const char *a_pccHotKey, TInt a_iOrdinal, TInt a_iCommand)
{

#ifdef __amigaos4__

	(void) a_iOrdinal;

	/* Only perform the action if the menus have been created */

	if (m_poAmiMenus)
	{
		m_poAmiMenus->UpdateItem(a_pccLabel, a_pccHotKey, a_iCommand);
	}

#elif defined(QT_GUI_LIB)

	(void) a_pccHotKey;

	TInt Index;
	CQtAction *Action;
	QList<QAction *> Menus = m_poWindow->menuBar()->actions();
	QMenu *DropdownMenu;

	/* Get a handle to the dropdown menu in which to modify the menu item */

	if ((DropdownMenu = Menus.at(a_iOrdinal)->menu()) != NULL)
	{
		QList<QAction *> Actions = DropdownMenu->actions();

		/* Iterate through the list of actions attached to the dropdown menu and */
		/* find the specified command ID */

		for (Index = 0; Index < Actions.count(); ++Index)
		{
			Action = (CQtAction *) Actions.at(Index);

			/* If this is the action then update its label */

			if (Action->Command() == a_iCommand)
			{
				Action->setText(a_pccLabel);

				break;
			}
		}
	}

#else /* ! QT_GUI_LIB */

	const char *Label;
	HMENU DropdownMenu;
	struct SStdMenuItem MenuItem;

	/* Get a handle to the dropdown menu in which to modify the menu item */

	if ((DropdownMenu = GetSubMenu(m_poMenu, a_iOrdinal)) != NULL)
	{
		/* Build the menu item's label as a composite of the menu's label and its shortcut */

		MenuItem.m_pccLabel = a_pccLabel;
		MenuItem.m_pccHotKey = a_pccHotKey;
		MenuItem.m_iHotKeyModifier = STD_KEY_ALT;

		/* Build a menu label to be displayed, including its shortcut */

		if ((Label = InitialiseMenuLabel(&MenuItem)) != NULL)
		{
			/* And update the menu item using the newly constructed label */

			ModifyMenu(DropdownMenu, a_iCommand, MF_BYCOMMAND, a_iCommand, Label);

			/* And free the temporary buffer used for the menu item label */

			delete [] (char *) Label;
		}
	}

#endif /* ! QT_GUI_LIB */

}
