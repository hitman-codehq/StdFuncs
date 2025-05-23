
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdGadgets.h"
#include "StdWindow.h"
#include <ctype.h>
#include <string.h>

#ifdef __amigaos__

#include "StdReaction.h"
#include "Amiga/AmiMenus.h"
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <classes/window.h>
#include <intuition/imageclass.h>

#ifdef __amigaos4__

#include <intuition/gui.h>

#endif /* __amigaos4__ */

#elif defined(QT_GUI_LIB)

#include "Qt/QtAction.h"
#include "Qt/QtWindow.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QVBoxLayout>

#elif defined(WIN32)

#include "StdRendezvous.h"
#include <VersionHelpers.h>

#endif /* WIN32 */

#if defined(WIN32) && !defined(QT_GUI_LIB)

/* Array of key mappings for mapping Windows keys onto standard keys */

static const SKeyMapping g_acoKeyMap[] =
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

#define NUM_KEYMAPPINGS (sizeof(g_acoKeyMap) / sizeof(struct SKeyMapping))

CWindow *CWindow::m_poActiveDialog;	/* Ptr to currently active dialog, if any */

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

TBool CWindow::m_bAltPressed;		/* ETrue if alt is currently pressed */
TBool CWindow::m_bCtrlPressed;		/* ETrue if ctrl is currently pressed */
TBool CWindow::m_bMetaPressed;		/* ETrue if meta is currently pressed */
TBool CWindow::m_bShiftPressed;		/* ETrue if shift is currently pressed */
TBool CWindow::m_bIsActive;			/* ETrue if the window is currently active */

#ifdef __amigaos__

/* Written: Saturday 20-Nov-2010 11:05 am */

ULONG CWindow::IDCMPFunction(struct Hook *a_poHook, Object * /*a_poObject*/, struct IntuiMessage *a_poIntuiMessage)
{
	struct TagItem *TagItem;
	CStdGadgetLayout *LayoutGadget;
	CWindow *Window;

	/* Get a ptr to the Window associated with the hook */

	Window = (CWindow *) a_poHook->h_Data;

#ifdef __amigaos4__

	/* If this is a mouse wheel event then convert the delta to a Qt/Windows style reading */
	/* (+/- 120 per notch) and notify the window */

	if (a_poIntuiMessage->Class == IDCMP_EXTENDEDMOUSE)
	{
		struct IntuiWheelData *IntuiWheelData = (struct IntuiWheelData *) a_poIntuiMessage->IAddress;
		Window->HandleWheelEvent(-IntuiWheelData->WheelY * 120);
	}
	else

#endif /* __amigaos4__ */

	/* If this is a message from a BOOPSI object saying that it has been updated, find the object and */
	/* map it onto its matching gadget and call the gadget's Updated() function */

	if (a_poIntuiMessage->Class == IDCMP_IDCMPUPDATE)
	{
		/* Get the gadget's unique ID */

		if ((TagItem = FindTagItem(GA_ID, (struct TagItem *) a_poIntuiMessage->IAddress)) != NULL)
		{
			/* Recursively search through the window's tree of layout gadgets and search each one to see */
			/* if it contains a gadget that represents the Reaction slider that was just moved */

			if ((LayoutGadget = Window->m_poRootLayout) != NULL)
			{
				LayoutGadget->SendUpdate((void *) TagItem->ti_Data, 0);
			}
		}

		/* It could be that the gadget that was updated has requested a redraw, which will be */
		/* indicated by one or more dirty regions being present.  Check for this and perform a */
		/* redraw so that the redraw happens immediately */

		if (Window->m_oDirtyRegions.size() > 0)
		{
			Window->InternalRedraw();
		}
	}

	return(DOSTRUE);
}

#elif defined(QT_GUI_LIB)

/**
 * CQtCentralWidget constructor.
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
 * the event along to the generic CWindow::draw() function, so that client code can
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

	m_poWindow->draw(Rect.top(), (Rect.top() + Rect.height()));
}

#elif defined(WIN32)

/* Written: Saturday 08-May-2010 4:43 pm */

LRESULT CALLBACK CWindow::WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	bool Result;
	TBool Checked;
	TInt Command, Index, Key;
	TBool Handled;
	HKL KeyboardLayout;
	LRESULT RetVal;
	const struct SStdMenuItem *MenuItem;
	CWindow *Window;
	COPYDATASTRUCT *CopyData;

	/* Return 0 by default for processed messages */

	RetVal = 0;

	/* Get the ptr to the C++ class associated with this window from the window word */

	Window = (CWindow *) GetWindowLongPtr(a_poWindow, GWLP_USERDATA);

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
				/* Convert the key to lower case */

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

					if (LOBYTE((DWORD_PTR) KeyboardLayout) == LANG_ENGLISH)
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
							/* Use the 8859-15 encoding for 'ü', as this file is encoded as UTF-8 and using 'ü' */
							/* in a string will generate an incorrect value */

							a_oWParam = 0xfc;
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
				Handled = Window->OfferKeyEvent((TInt) a_oWParam, ETrue);

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

			Window->OfferRawKeyEvent((TInt) a_oWParam, (a_uiMessage == WM_SYSKEYDOWN));

			/* Have some special processing for the alt key, passing it onto the client code.  Even */
			/* though the the alt key is not handled through the usual g_acoKeyMap array, to client */
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
				if (g_acoKeyMap[Index].m_iNativeKey == (int) a_oWParam)
				{
					break;
				}
			}

			/* If it was a known key then convert it to the standard value and pass it to the */
			/* CWindow::OfferKeyEvent() function */

			if (Index < (TInt) NUM_KEYMAPPINGS)
			{
				Key = g_acoKeyMap[Index].m_iStdKey;

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

			/* Now take the horribleness to the next level.  Certain keys (including the mysterious */
			/* VK_OEM_MINUS key) are simply not passed to WM_CHAR if the control key is pressed.  But */
			/* we need these keys and they work without special treatment on other operating systems. */
			/* So we have to look out for them individually and simulate a WM_CHAR event if found */

			TBool OfferKey;
			UINT VirtualKey;

			if ((m_bCtrlPressed) && (!m_bAltPressed))
			{
				if ((a_oWParam == VK_SUBTRACT) || (a_oWParam == VK_ADD) || (a_oWParam == VK_OEM_MINUS) || (a_oWParam == VK_OEM_PLUS))
				{
					VirtualKey = MapVirtualKey((UINT) a_oWParam, MAPVK_VK_TO_CHAR);

					/* Determine the current input locale */

					KeyboardLayout = GetKeyboardLayout(0);

					/* Do some deep magic based on some empirical investigation into what special key get sent */
					/* by Windows multiple times in which keyboard mappings.  If there is a logic to this, I */
					/* I don't get it */

					if (LOBYTE((DWORD_PTR) KeyboardLayout) == LANG_ENGLISH)
					{
						if ((ShiftPressed()) && (a_oWParam == VK_OEM_MINUS))
						{
							OfferKey = EFalse;
						}
						else
						{
							OfferKey = ETrue;
						}
					}
					else
					{
						if ((ShiftPressed()) && (a_oWParam == VK_OEM_MINUS))
						{
							OfferKey = EFalse;
						}
						else if ((!ShiftPressed()) && (a_oWParam == VK_OEM_PLUS))
						{
							OfferKey = EFalse;
						}
						else
						{
							OfferKey = ETrue;
						}
					}

					/* Horribleness++.  It turns out that three of our "mysterious" keys are always mapped to their */
					/* unshifted state, even if the shift key is pressed.  There is no function I could find that */
					/* will these map correctly, so for these two keys we will perform a manual shift to get the */
					/* required keys */

					if (ShiftPressed())
					{
						if (VirtualKey == '-')
						{
							VirtualKey = '_';
						}
						else if (VirtualKey == '=')
						{
							VirtualKey = '+';
						}

						/* The final nightmare inconsistency is only valid in German keyboard layouts */

						else if ((LOBYTE((DWORD_PTR) KeyboardLayout) == LANG_GERMAN) && (VirtualKey == '+'))
						{
							VirtualKey = '*';
						}
					}

					/* Offer the key to the client only if it has been determined that it is not one of the */
					/* keys that will also be sent by WM_CHAR */

					if (OfferKey)
					{
						Window->OfferKeyEvent(VirtualKey, (a_uiMessage == WM_KEYDOWN));
					}
				}

				/* Number keys also do not generate a WM_CHAR if the control key is pressed, so again we */
				/* must simulate it.  Numbers do not need to be mapped as their virtual keys are the same as */
				/* their character values */

				else if ((a_oWParam >= 0x30) && (a_oWParam <= 0x39))
				{
					Window->OfferKeyEvent((TInt) a_oWParam, (a_uiMessage == WM_KEYDOWN));
				}
			}

			/* Pass the raw key onto the CWindow::OfferRawKeyEvent() function, without any kind */
			/* of preprocessing */

			Window->OfferRawKeyEvent((TInt) a_oWParam, (a_uiMessage == WM_KEYDOWN));

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
			/* Prepare the device context for painting and call the CWindow::draw() routine. */
			/* If this fails then there isn't much we can do besides ignore the error */

			if ((Window->m_poDC = BeginPaint(a_poWindow, &Window->m_oPaintStruct)) != NULL)
			{
				Window->draw(Window->m_oPaintStruct.rcPaint.top, Window->m_oPaintStruct.rcPaint.bottom);
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
			/* Recursively search through the window's tree of layout gadgets and search each one to see */
			/* if it contains a gadget that represents the Windows slider that was just moved */

			Result = false;

			if (Window->m_poRootLayout)
			{
				Result = Window->m_poRootLayout->SendUpdate((void *) a_oLParam, LOWORD(a_oWParam));
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
	close();
}

/* Written: Wednesday 13-Oct-2010 7:29 am */

void CWindow::Activate()
{
	ASSERTM(m_poWindow, "CWindow::Activate() => Window must already be open");

#ifdef __amigaos__

	SetAttrs(m_poWindowObj, WINDOW_FrontBack, WT_FRONT, TAG_DONE);
	ActivateWindow(m_poWindow);

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

#ifndef __amigaos__

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
		Utils::info("CWindow::AddMenuItem() => Unable to create menu item");

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
			Utils::info("CWindow::AddMenuItem() => Unable to allocate memory for menu item label");

			RetVal = KErrNoMemory;
		}
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

#endif /* ! __amigaos__ */

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

#ifndef __amigaos__

	struct SStdMenuItem MenuItem = { a_eMenuItemType, a_pccLabel, a_pccHotKey, STD_KEY_ALT, a_iCommand };

#endif /* ! __amigaos__ */

#ifdef __amigaos__

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

/**
 * Attach a layout to the window.
 * This function will attach the layout passed in to the window.  This will take care of adding the
 * layout to the window's internal list of gadgets, as well as attaching it to the underlying OS
 * specific layout system if required.
 *
 * This is not a general purpose method and should only be used by The Framework when creating the
 * initial window and root layout.  User code must not call this method.
 *
 * @date	Monday 11-Jul-2011 6:16 am
 * @param	a_poGadget		Pointer to the layout gadget to attach to the layout
 */

void CWindow::Attach(CStdGadgetLayout *a_poLayoutGadget)
{
	ASSERTM((a_poLayoutGadget != NULL), "CWindow::Attach() => No gadget to be attached passed in");
	ASSERTM((m_poWindow != NULL), "CWindow::Attach() => Window not yet open");
	ASSERTM((m_poRootLayout == NULL), "CWindow::Attach() => Root layout already attached");

#ifdef __amigaos__

	/* Add the new BOOPSI gadget to the window's root layout */

	Object LayoutObj = 0;

	GetAttr(WINDOW_Layout, m_poWindowObj, (ULONG *) &LayoutObj);

	if (LayoutObj)
	{
		if (SetGadgetAttrs((struct Gadget *) LayoutObj, m_poWindow, NULL,
			LAYOUT_AddChild, (ULONG) a_poLayoutGadget->m_poGadget, TAG_DONE))
		{
			rethinkLayout();
		}
	}

#elif defined(QT_GUI_LIB)

	(void) a_poLayoutGadget;

#else /* ! QT_GUI_LIB */

	if (!m_poRootLayout)
	{
		m_poRootLayout = a_poLayoutGadget;
	}

	rethinkLayout();

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

#ifdef __amigaos__

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

TBool CWindow::createMenus()
{
	TBool RetVal;
	const struct SStdMenuItem *MenuItem;

	ASSERTM((m_poWindow != NULL), "CWindow::createMenus() => Window must be created before menus");

	/* Assume success */

	RetVal = ETrue;

	/* Iterate through the list of menu structures passed in and create a menu item for each */
	/* one as appropriate */

	MenuItem = m_poApplication->MenuItems();

#ifdef __amigaos__

	ASSERTM((m_poAmiMenus == NULL), "CWindow::createMenus() => Menus can only be created once");

	if ((m_poAmiMenus = CAmiMenus::New(this, MenuItem)) == NULL)
	{
		RetVal = EFalse;
	}

#elif defined(QT_GUI_LIB)

	QMenu *DropdownMenu, *PopupMenu, *TopLevelMenu;
	QString Shortcut;

#ifdef _DEBUG

	QList<QAction *> Menus = m_poWindow->menuBar()->actions();

	ASSERTM((Menus.count() == 0), "CWindow::createMenus() => Menus can only be created once");

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
				Utils::info("CWindow::createMenus() => Unable to create drop down menu");

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
				Utils::info("CWindow::createMenus() => Unable to create submenu");

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

	ASSERTM((m_poMenu == NULL), "CWindow::createMenus() => Menus can only be created once");

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
						"CWindow::createMenus() => Unable to append new menu");
				}
				else
				{
					Utils::info("CWindow::createMenus() => Unable to create drop down menu");

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
						"CWindow::createMenus() => Unable to append new submenu");

					/* Any further menu items from here to the next dropdown menu will be added to this submenu */

					DropdownMenu = PopupMenu;
				}
				else
				{
					Utils::info("CWindow::createMenus() => Unable to create submenu");

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

		DEBUGCHECK((SetMenu(m_poWindow, m_poMenu) != FALSE), "CWindow::createMenus() => Unable to assign menu to window");

		/* And free the temporary buffer used for the menu item labels */

		Utils::FreeTempBuffer(Label);
	}
	else
	{
		RetVal = EFalse;

		Utils::info("CWindow::createMenus() => Unable to create top level menubar");
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

#ifdef __amigaos__

	/* First bring the screen to the front */

	ScreenToFront(m_poWindow->WScreen);

	/* And then activate the window */

	Activate();

#elif defined(QT_GUI_LIB)

	/* The Qt version of this method is a little strange as it depends on the underlying X11 server.  Calling */
	/* QWidget::activateWindow() should normally be enough, but this does not guarantee that the X11 server will */
	/* bring the window to the front.  So we have to also call QWidget::raise() to make sure */

	m_poWindow->raise();
	m_poWindow->activateWindow();

	/* As when closing a dialog, Qt calls the QMainWindow::focusInEvent() callback *after* */
	/* it performs a redraw of the main window, resulting in an invisible cursor, so we have */
	/* to manually re-enable display of the cursor when closing */

	m_bIsActive = ETrue;

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((SetForegroundWindow(m_poWindow) != FALSE), "CWindow::BringToFront() => Unable to set foreground window");

#endif /* ! QT_GUI_LIB */

}

/* Written: Wednesday 14-Jul-2011 6:14 am, Code HQ-by-Thames */

void CWindow::ClearBackground(TInt a_iY, TInt a_iHeight, TInt a_iX, TInt a_iWidth)
{

#ifdef __amigaos__

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow)
	{
		/* All window drawing operations are relative to the client area, so adjust the X and Y */
		/* positions such that they point past the left and top borders */

		a_iX += m_poWindow->BorderLeft;
		a_iY += m_poWindow->BorderTop;

#ifdef __amigaos4__

		ShadeRect(m_poWindow->RPort, a_iX, a_iY, (a_iX + a_iWidth - 1), (a_iY + a_iHeight - 1),
			LEVEL_NORMAL, BT_BACKGROUND, IDS_NORMAL, GetScreenDrawInfo(m_poWindow->WScreen), TAG_DONE);

#else /* ! __amigaos4__ */

		SetAPen(m_poWindow->RPort, 0);
		RectFill(m_poWindow->RPort, a_iX, a_iY, (a_iX + a_iWidth - 1), (a_iY + a_iHeight - 1));

#endif /* ! __amigaos4__ */

	}

#else /* ! __amigaos__ */

	// TODO: CAW - Implement this + comment this function
	(void) a_iY;
	(void) a_iHeight;
	(void) a_iX;
	(void) a_iWidth;

#endif /* ! __amigaos__ */

}

/* Written: Monday 08-Feb-2010 7:18 am */

void CWindow::close()
{
	/* Delete the window's single layout gadget.  This will in turn delete all child layout gadgets */
	/* and all gadgets that are attached to them.  Some GUIs will automatically destruct attached */
	/* layouts and gadgets when the window is closed, and some don't.  So to cater for both systems, */
	/* this will manually dispose of all class instances and the associated native widgets, so */
	/* avoiding problems when closing the window */

	delete m_poRootLayout;
	m_poRootLayout = NULL;

#ifdef __amigaos__

	/* Destroy the window's menus and associated resources */

	delete m_poAmiMenus;
	m_poAmiMenus = NULL;

	/* And close the window itself */

	if (m_poWindowObj)
	{
		DisposeObject(m_poWindowObj);
		m_poWindowObj = NULL;
	}

#ifndef __amigaos4__

	/* And the window's screen, if it is open */

	if (m_poScreen)
	{
		CloseScreen(m_poScreen);
	}

#endif /* __amigaos4__ */

#elif defined(QT_GUI_LIB)

	if (m_poWindow)
	{
		/* Let the window know that it is in the process if closing so that it */
		/* accepts the close event in closeEvent() */

		m_poWindow->setClosing(true);

		/* Ensure that the QMainWindow instance is actually freed when it is closed */

		m_poWindow->setAttribute(Qt::WA_DeleteOnClose);

		/* And close and delete the window */

		DEBUGCHECK((m_poWindow->close() != false), "CWindow::close() => Unable to close window");
		delete m_poWindow;
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
		DEBUGCHECK((DestroyWindow(m_poWindow) != 0), "CWindow::close() => Unable to destroy window");
		m_poWindow = NULL;
	}

	/* If the window's class is registered, unregister it and indicate that it is no longer registered */

	if (m_poWindowClass)
	{
		DEBUGCHECK((UnregisterClass((LPCTSTR) (DWORD_PTR) m_poWindowClass, GetModuleHandle(NULL)) != FALSE), "CWindow::close() => Unable to unregister window class");
		m_poWindowClass = 0;
	}

#endif /* ! QT_GUI_LIB */

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

#ifdef __amigaos__

	ASSERTM(m_poWindowObj, "CWindow::CompleteOpen() => Reaction window must already be open");

	/* Get the window's signal so that we can wait until an event occurs */

	GetAttr(WINDOW_Window, m_poWindowObj, (ULONG *) &m_poWindow);

#endif /* __amigaos__ */

	/* Add this dialog to the application so that messages can be routed appropriately */

	m_poApplication->AddWindow(this);
	m_bOpen = ETrue;
}

/**
 * Draws the entire client area.
 * Invalidates the entire client area inside the window and instigates a draw of it.
 *
 * The draw does not happen immediately but is performed the next time the application's
 * message loop is executed.
 *
 * @pre		A draw must not already be in progress when this method is called
 *
 * @date	Saturday 29-May-2010 1:07 pm
 */

void CWindow::DrawNow()
{
	/* Just determine the dimensions of the window and pass the call on, taking */
	/* into account that the bottom pixel offset is exclusive, thus adding 1 on */
	/* the platforms required */

#ifdef __amigaos__

	DrawNow(0, (m_poWindow->BorderTop + m_iInnerHeight + 1));

#elif defined(QT_GUI_LIB)

	DrawNow(0, (m_iInnerHeight + 1));

#else /* ! QT_GUI_LIB */

	RECT Rect;

	/* Get the dimensions of the client area and adjust it to only represent the vertical */
	/* band that we wish to draw, also adjusting the size of the area to be cleared and */
	/* drawn to take into account any attached gadgets */

	if (GetClientRect(m_poWindow, &Rect))
	{
		DrawNow(0, Rect.bottom);
	}
	else
	{
		Utils::info("CWindow::DrawNow() => Unable to obtain client window dimensions");
	}

#endif /* ! QT_GUI_LIB */

}

/**
 * Draws a region of the client area.
 * Invalidates a vertical band of the client area and instigates a draw of that area.
 * The bottom of the area, represented by a_iBottom, is considered exclusive, so the area
 * drawn is between a_iTop and (a_iBottom - 1).
 *
 * The draw does not happen immediately but is performed the next time the application's
 * message loop is executed.
 *
 * @pre		A draw must not already be in progress when this method is called
 *
 * @date	Saturday 30-Nov-2010 9:15 pm
 * @param	a_iTop			Offset from top of client area from which to invalidate
 * @param	a_iBottom		Bottom most part of client area to which to invalidate
 */

void CWindow::DrawNow(TInt a_iTop, TInt a_iBottom)
{
	ASSERTM((a_iTop >= 0), "CWindow::DrawNow() => Y offset to draw from must not be negative");
	ASSERTM((a_iBottom >= 0), "CWindow::DrawNow() => Y offset to draw to must not be negative");
	ASSERTM((a_iTop <= a_iBottom), "CWindow::DrawNow() => Y offset start must be less than Y offset stop");

#ifdef __amigaos__

	std::vector<SRegion>::iterator it;

	ASSERTM(!m_bPerformingRedraw, "CWindow::DrawNow() => New draws must not be requested during a draw");

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow)
	{
		/* Iterate through the dirty regions and try to find one that overlaps with the new */
		/* region that is to be invalidated */

		for (it = m_oDirtyRegions.begin(); it != m_oDirtyRegions.end(); ++it)
		{
			if ((a_iTop <= (*it).m_iBottom) && (a_iBottom >= (*it).m_iTop))
			{
				break;
			}
		}

		/* If the new region overlaps an existing region then extend the existing region to */
		/* include the area represented by the new one */

		if (it != m_oDirtyRegions.end())
		{
			if (a_iTop < (*it).m_iTop)
			{
				(*it).m_iTop = a_iTop;
			}

			if (a_iBottom > (*it).m_iBottom)
			{
				(*it).m_iBottom = a_iBottom;
			}
		}

		/* Otherwise the new region stands alone, so add it to the list of dirty regions */

		else
		{
			SRegion Region = { a_iTop, a_iBottom };

			m_oDirtyRegions.push_back(Region);
		}

		/* And actually perform the redraw */

		InternalRedraw();
	}

#elif defined(QT_GUI_LIB)

	QWidget *CentralWidget;

	/* Unit Test support: The Framework must be able to run without a real GUI */

	if (m_poWindow)
	{
		CentralWidget = m_poWindow->centralWidget();
		ASSERTM((CentralWidget != NULL), "CWindow::DrawNow() => Central widget has not been assigned to window");

		/* And invalidate the vertical band.  Like Windows, this will defer the drawing until l8r, */
		/* possibly coalescing multiple draws into one */

		CentralWidget->update(0, a_iTop, m_iInnerWidth, a_iBottom);
	}

#elif defined(WIN32)

	RECT Rect;

	/* Get the dimensions of the client area and adjust it to only represent the vertical */
	/* band that we wish to draw, also adjusting the size of the area to be cleared and */
	/* drawn to take into account any attached gadgets */

	if (GetClientRect(m_poWindow, &Rect))
	{
		Rect.bottom = (Rect.top + a_iBottom);
		Rect.top += a_iTop;
		Rect.right = m_iInnerWidth;

		/* And invalidate the vertical band */

		InvalidateRect(m_poWindow, &Rect, TRUE);
	}
	else
	{
		Utils::info("CWindow::DrawNow() => Unable to obtain client window dimensions");
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

#ifdef __amigaos__

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

#ifdef __amigaos__

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
 * Calls the client code to redraw regions within the dirty region list.
 * This method will iterate through the list of regions in the dirty region list and for each one,
 * will call the client code via the overridden CWindow::draw() method to draw that region.
 *
 * @date	Wednesday 21-Sep-2016 7:02 am, Code HQ Ehinger Tor
 */

void CWindow::InternalRedraw()
{
	std::vector<SRegion>::iterator it;

	/* Set a debug flag to ensure that new redraws are not requested while we are already */
	/* in the middle of a redraw */

	m_bPerformingRedraw = true;

	/* Iterate through the list of dirty regions and, for each one, call the derived rendering method */
	/* to perform the actual drawing */

	for (it = m_oDirtyRegions.begin(); it != m_oDirtyRegions.end(); ++it)
	{
		draw((*it).m_iTop, (*it).m_iBottom);
	}

	/* Indicate that there are no longer any dirty regions to be drawn */

	m_oDirtyRegions.clear();

	/* And clear the protection flag */

	m_bPerformingRedraw = false;
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
	size_t Length;

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

/**
 * Perform a relayout based on the window size changing.
 * This function is called whenever the size of the window changes and will reposition any auto-position
 * gadgets and will notify the derived window class that the size has changed.
 *
 * Note that this is an update method that is triggered in response to the window changing size.
 * It does not itself change the window's size.
 *
 * @date	Sunday 01-May-2011 10:48 am
 * @param	a_iInnerWidth	New width of client area
 * @param	a_iInnerHeight	New height of client area
 */

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

	/* We're going to cheat a bit here.  We have to let the client code know that the layout has changed, but if */
	/* we just call rethinkLayout() then this will generate another native rethink on Amiga OS, even though the */
	/* resize has alrady performed a rethink.  This will cause the gadgets to be drawn twice.  So we'll set the */
	/* rethinker to prevent the layout system from triggering another native rethink */

	CStdGadgetLayout::m_poRethinker = m_poRootLayout;
	rethinkLayout();

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

#ifdef __amigaos__

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
 *							OS only - this is ignored on other platforms.  Pass in NULL to open
 *							on the Workbench screen
 * @param	a_bResizeable	ETrue to enable resizing of the window.  Amiga OS only - on other
 *							platforms windows are always resizeable
 * @return	KErrNone if the window was opened successfully
 * @return	KErrNoMemory if there was not enough memory to open the window
 * @return	KErrGeneral if an operating system specific error occurred
 */

TInt CWindow::open(const char *a_pccTitle, const char *a_pccScreenName, TBool a_bResizeable)
{
	TInt RetVal;

	/* Assume failure */

	RetVal = KErrGeneral;

#ifdef __amigaos__

	struct SRect ScreenSize;

	/* Get the size of the screen so we can open the window filling its full size */

	Utils::GetScreenSize(ScreenSize);

	/* Setup an IDCMP hook that can be used for monitoring gadgets for extra information not */
	/* provided by Reaction, such as the movement of proportional gadgets */

#ifdef __amigaos4__

	m_oIDCMPHook.h_Entry = (uint32 (*)()) (void *) IDCMPFunction;

#else /* ! __amigaos4__ */

	m_oIDCMPHook.h_Entry = (ULONG (*)()) HookEntry;
	m_oIDCMPHook.h_SubEntry = (ULONG (*)()) IDCMPFunction;

#endif /* ! __amigaos4__ */

	m_oIDCMPHook.h_Data = this;

#ifndef __amigaos4__

	/* If a screen name was passed in then open a screen for the window to be opened on.  OS4 will do this */
	/* automatically if the named screen has been configured in preferences, but on OS3 we must do it ourselves */

	if (a_pccScreenName)
	{
		m_poScreen = OpenScreenTags(NULL, SA_LikeWorkbench, TRUE, SA_PubName, (ULONG) a_pccScreenName,
			SA_Title, (ULONG) a_pccScreenName, TAG_DONE);

		if (m_poScreen)
		{
			/* Public screens are private by default (!) so make the newly opened screen public */

			DEBUGCHECK(((PubScreenStatus(m_poScreen, 0) & 0x01) == 1), "CWindow::open() => Unable to make screen public");
		}
		else
		{
			Utils::info("CWindow::open() => Unable to open screen \"%s\"", a_pccScreenName);
		}
	}

#endif /* ! __amigaos4__ */

	/* IDCMP bits to be used for the window.  Assign these to a variabe so they can be tweaked for OS4 */

	ULONG HookBits = IDCMP_IDCMPUPDATE;
	ULONG IDCMP = (IDCMP_CLOSEWINDOW | IDCMP_IDCMPUPDATE | IDCMP_MENUPICK | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE |
				   IDCMP_RAWKEY | IDCMP_REFRESHWINDOW | IDCMP_NEWSIZE);

#ifdef __amigaos4__

	HookBits |= IDCMP_EXTENDEDMOUSE;
	IDCMP |= IDCMP_EXTENDEDMOUSE;

#endif /* __amigaos4__ */

	/* Create a Reaction Window and open it on the requested screen at the maximum size of */
	/* the screen.  If no screen name is specified, fall back to the Workbench */

	m_poWindowObj = (Object *) WindowObject,
		WA_Title, (ULONG) a_pccTitle, WINDOW_Position, WPOS_CENTERSCREEN,
		WA_PubScreenName, a_pccScreenName, WA_PubScreenFallBack, TRUE,
		WA_Width, (ULONG) ScreenSize.m_iWidth, WA_Height, (ULONG) ScreenSize.m_iHeight, WA_Activate, TRUE,
		WA_CloseGadget, TRUE, WA_DepthGadget, TRUE, WA_DragBar, TRUE, WA_ReportMouse, TRUE, WA_SizeGadget, a_bResizeable,
		WINDOW_IDCMPHook, (ULONG) &m_oIDCMPHook, WINDOW_IDCMPHookBits, HookBits, WA_IDCMP, IDCMP,

		WINDOW_Layout, NewObject(LAYOUT_GetClass(), NULL, LAYOUT_Orientation, LAYOUT_VERTICAL,
			/* This is an empty group into which can be placed BOOPSI objects */
		TAG_DONE),
	TAG_DONE);

	if (m_poWindowObj)
	{
		if (RA_OpenWindow(m_poWindowObj))
		{
			/* Get a ptr to the underlying Intuition Window, as it is handy to have for such */
			/* things as obtaining the window's signal bit */

			GetAttr(WINDOW_Window, m_poWindowObj, (ULONG *) &m_poWindow);

			/* Calculate the inner width and height of the window, for l8r use */

			m_iInnerWidth = (m_poWindow->Width - (m_poWindow->BorderRight + m_poWindow->BorderLeft));
			m_iInnerHeight = (m_poWindow->Height - (m_poWindow->BorderBottom + m_poWindow->BorderTop));

			/* Also save the outer width and height of the window */

			m_iWidth = m_poWindow->Width;
			m_iHeight = m_poWindow->Height;

			if ((m_poRootLayout = CStdGadgetLayout::New(NULL, ETrue, NULL, this)) != NULL)
			{
				/* And create the menus specific to this window */

				if (createMenus())
				{
					/* Indicate success */

					RetVal = KErrNone;

					/* And now bring the window's screen to the front, so that if an instance of the client */
					/* application is already running on the screen, it will come to the front.  Without this, */
					/* the screen would only come to the front when the first instance of the application was */
					/* launched */

					ScreenToFront(m_poWindow->WScreen);
				}
				else
				{
					Utils::info("CWindow::open() => Unable to create menus for window");
				}
			}
			else
			{
				Utils::info("CWindow::open() => Unable to create root layout object");
			}
		}
		else
		{
			Utils::info("CWindow::open() => Unable to open window");
		}

		/* If any error has occurred then clean up after ourselves */

		if (RetVal != KErrNone)
		{
			DisposeObject(m_poWindowObj);
			m_poWindowObj = NULL;
		}
	}
	else
	{
		Utils::info("CWindow::open() => Unable to create window");
	}

#elif defined(QT_GUI_LIB)

	(void) a_pccScreenName;
	(void) a_bResizeable;

	/* Assume failure */

	RetVal = KErrNoMemory;

	const QList<QScreen *> Screens = m_poApplication->Application()->screens();
	QPoint MousePosition = QCursor::pos();
	QScreen *TargetScreen = NULL;

	/* Iterate through the list of screens and determine which one the mouse pointer is on */
	for (QScreen *Screen : Screens)
	{
		QRect ScreenRect = Screen->availableGeometry();

		if (ScreenRect.contains(MousePosition))
		{
			TargetScreen = Screen;
			break;
		}
	}

	QSize WindowSize = Screens[0]->availableGeometry().size();
	QPoint WindowPosition;

	/* If we are opening on the main screen, the window will be opened at the same size as the desktop and */
	/* will be opened maximised. Otherwise, if we are opening on a second screen, take into account that the */
	/* screen may be larger than the main screen and open the window non maximised, with the same size as the */
	/* main screen, with the top left of the window at the location of the mouse pointer */
	if (TargetScreen != Screens[0])
	{
		QRect ScreenRect = TargetScreen->availableGeometry();

		/* If the target screen is smaller than the window, resize the window to be the same size */
		if (WindowSize.width() > ScreenRect.width() || WindowSize.height() > ScreenRect.height())
		{
			WindowPosition = QPoint(ScreenRect.x(), ScreenRect.y());
			WindowSize = (QSize(ScreenRect.width(), ScreenRect.height()));
		}
		else
		{
			/* If the target screen and window are exactly the same size, we don't need to set the */
			/* window's size, but we still need to move it to the right place for it to appear on */
			/* the target screen */
			if (WindowSize.width() == ScreenRect.width() || WindowSize.height() == ScreenRect.height())
			{
				WindowPosition = QPoint(ScreenRect.x(), ScreenRect.y());
			}
			else
			{
				int MouseX = (MousePosition.x() - ScreenRect.x());
				int MouseY = (MousePosition.y() - ScreenRect.y());

				WindowPosition = QPoint(ScreenRect.x() + MouseX, ScreenRect.y() + MouseY);
			}
		}
	}

	/* Allocate a window based on the QMainWindow class, passing in the preferred size */
	/* to use for the window when in non maximised state */

	if ((m_poWindow = new CQtWindow(this, WindowPosition, WindowSize)) != NULL)
	{
		/* Install an event filter so that we receive notifications about events such as window resizing */

		m_poWindow->installEventFilter(m_poWindow);

		/* We also need a QWidget based class to use as the so-called "central widget" */

		if ((m_poCentralWidget = new CQtCentralWidget(this)) != NULL)
		{
			/* Assign the widget as the main window's central widget */

			m_poWindow->setCentralWidget(m_poCentralWidget);

			if ((m_poRootLayout = CStdGadgetLayout::New(NULL, ETrue, NULL, this)) != NULL)
			{
				/* Set the window's title to the one passed in */

				QString Title(a_pccTitle);
				m_poWindow->setWindowTitle(Title);

				/* And create the menus specific to this window */

				if (createMenus())
				{
					RetVal = KErrNone;

					/* Display the window maximised only if it is being opened with a size equal to that of the */
					/* target window. This allows use of super-large secondary screens without the window being */
					/* opened with a huge size, such as when running a laptop as the primary screen and a super-large */
					/* widescreen monitor as the secondary screen */

					if (m_poWindow->size() == TargetScreen->availableSize())
					{
						m_poWindow->showMaximized();
					}
					else
					{
						m_poWindow->show();
					}

					/* Showing the window doesn't necessarily give it focus, even if keyboard input to the window is */
					/* working, meaning that the window won't get focus-related events. Explicitly setting the focus */
					/* to the window shouldn't be necessary, but seems to be required to enable these events to be sent */

					m_poWindow->setFocus();

					/* Save the outer width and height of the window */

					m_iWidth = m_poWindow->width();
					m_iHeight = m_poWindow->height();

					/* And save the inner width and height of the window */

					QSize Size = m_poCentralWidget->size();
					m_iInnerWidth = Size.width();
					m_iInnerHeight = Size.height();

					/* And indicate that the window is active, as Qt 5 performs a draw operation */
					/* before calling QMainWindow::focusInEvent(), resulting in the cursor not being */
					/* displayed */

					m_bIsActive = ETrue;
				}
				else
				{
					Utils::info("CWindow::open() => Unable to create menus for window");
				}
			}
			else
			{
				Utils::info("CWindow::open() => Unable to create root layout object");
			}
		}
		else
		{
			Utils::info("CWindow::open() => Not enough memory to create central widget");
		}
	}
	else
	{
		Utils::info("CWindow::open() => Not enough memory to create window");
	}

#elif defined(WIN32)

	int Height;
	HINSTANCE Instance;
	RECT Rect;
	WNDCLASS WndClass;

	(void) a_pccScreenName;
	(void) a_bResizeable;

	/* Populate a WNDCLASS structure in preparation for registering the window class */

	Instance = GetModuleHandle(NULL);
	WndClass.style = CS_DBLCLKS;
	WndClass.lpfnWndProc = WindowProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = Instance;
	WndClass.hIcon = LoadIcon(Instance, MAKEINTRESOURCE(107));
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
			/* This is an awful hack for Windows 10, which seems to include the drop shadows outside the */
			/* window as a part of the window itself, and which only seems to use drop shadows on the left, */
			/* right and bottom borders, resulting in non centred windows when opening non maximised.  So */
			/* for Windows 10, we move the window down a bit to simulate a drop shadow at the top of the */
			/* window, and make it smaller */

			if (IsWindowsVersionOrGreater(0x0a, 0, 0))
			{
				Height = (GetSystemMetrics(SM_CYSIZEFRAME) * 2);
				Rect.top += Height;
				Rect.bottom -= Height;
			}

			if ((m_poWindow = CreateWindow(a_pccTitle, a_pccTitle, WS_OVERLAPPEDWINDOW, Rect.left, Rect.top,
				Rect.right, Rect.bottom, NULL, NULL, Instance, NULL)) != NULL)
			{
				/* Save a ptr to the window handle for use in the WindowProc() routine */

				SetWindowLongPtr(m_poWindow, GWLP_USERDATA, (LONG_PTR) this);

				/* Create the menus specific to this window */

				if (createMenus())
				{
					/* Display the window on the screen, maximised */

					ShowWindow(m_poWindow, SW_MAXIMIZE);

					if (GetClientRect(m_poWindow, &Rect))
					{
						/* Calculate the inner width and height of the window, for l8r use */

						m_iInnerWidth = (Rect.right - Rect.left);
						m_iInnerHeight = (Rect.bottom - Rect.top);

						/* Also save the outer width and height of the window */

						if (GetWindowRect(m_poWindow, &Rect))
						{
							m_iWidth = (Rect.right - Rect.left);
							m_iHeight = (Rect.bottom - Rect.top);

							if ((m_poRootLayout = CStdGadgetLayout::New(NULL, ETrue, NULL, this)) != NULL)
							{
								RetVal = KErrNone;
							}
							else
							{
								Utils::info("CWindow::open() => Unable to create root layout object");
							}
						}
						else
						{
							Utils::info("CWindow::open() => Unable to obtain window dimensions");
						}
					}
					else
					{
						Utils::info("CWindow::open() => Unable to obtain window client dimensions");
					}
				}
				else
				{
					Utils::info("CWindow::open() => Unable to create menus for window");
				}
			}
			else
			{
				Utils::info("CWindow::open() => Unable to open window");
			}
		}
		else
		{
			Utils::info("CWindow::open() => Unable to determine size of desktop");
		}
	}
	else
	{
		Utils::info("CWindow::open() => Unable to register window class");
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
		close();
	}

	return(RetVal);
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
	HACCEL NewAccelerators;

	/* Assume failure */

	RetVal = KErrGeneral;

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
			/* Create a new accelerator table that does not contain the deleted accelerator */

			if ((NewAccelerators = CreateAcceleratorTable(Accelerators, (NumAccelerators - 1))) != NULL)
			{
				RetVal = KErrNone;

				/* The new accelerator table was created successfully so delete the old one and take */
				/* the new one into use */

				DestroyAcceleratorTable(m_poAccelerators);
				m_poAccelerators = NewAccelerators;
			}
		}
		else
		{
			RetVal = KErrNotFound;
		}

		delete [] Accelerators;
	}
	else
	{
		RetVal = KErrNoMemory;
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

#ifdef __amigaos__

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

#ifdef _DEBUG

		TInt Result;

		Result = RemoveAccelerator(a_iCommand);
		DEBUGCHECK(((Result == KErrNone) || (Result == KErrNotFound)), "CWindow::RemoveMenuItem() => Unable to remove accelerator");

#else /* ! _DEBUG */

		RemoveAccelerator(a_iCommand);

#endif /* ! _DEBUG */

	}

#endif /* ! QT_GUI_LIB */

}

/* Written: Saturday 05-Nov-2011 12:04 pm, Code HQ Soeflingen */

void CWindow::rethinkLayout()
{

#ifdef __amigaos__

	/* Rethink the layout gadget sizes, starting from the root gadget that holds them */

	Object LayoutObj = 0;

	GetAttr(WINDOW_Layout, m_poWindowObj, (ULONG *) &LayoutObj);
	ASSERTM((LayoutObj != 0), "CWindow::rethinkLayout() => Unable to obtain window's BOOPSI layout object");

	if (LayoutObj)
	{
		/* Trigger a native Intuition rethink only if one has not already happened */

		if (!CStdGadgetLayout::m_poRethinker)
		{
			RethinkLayout((struct Gadget *) LayoutObj, m_poWindow, NULL, TRUE);

			/* Indicate to the root gadget that an Intuition rethink is already underway, to prevent it from */
			/* triggering another one and thus causing flicker */

			CStdGadgetLayout::m_poRethinker = m_poRootLayout;
		}
	}

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	if (m_poRootLayout)
	{
		m_poRootLayout->m_iWidth = m_iInnerWidth;
		m_poRootLayout->m_iHeight = m_iInnerHeight;
	}

#endif /* ! defined(WIN32) && !defined(QT_GUI_LIB) */

	/* Now recursively iterate through the window's tree of layout gadgets and let them know they */
	/* have been resized */

	if (m_poRootLayout)
	{
		m_poRootLayout->rethinkLayout();
	}
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
 * Enables display of the cursor.
 * This function only has an effect on systems that supply cursor functionality, such as Windows.
 *
 * @pre		The window has been opened
 *
 * @date	Thursday 30-Jun-2022 6:51 am, Oakwood Hotels & Apartments, Tokyo Azabu
 * @param	a_bShow			true to show the cursor, else false to hide it
 */

void CWindow::ShowCursor(bool a_bShow)
{
	ASSERTM(m_poWindow != NULL, "CWindow::ShowCursor() => Trying to set cursor state in unopened window");

#if defined(WIN32) && !defined(QT_GUI_LIB)

	if (a_bShow)
	{
		ShowCaret(m_poWindow);
	}
	else
	{
		HideCaret(m_poWindow);
	}

#else /* ! defined(WIN32) && !defined(QT_GUI_LIB) */

	(void) a_bShow;

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

}

/**
 * Returns whether the meta key is currently pressed.
 * Allows client code to query whether the meta key is currently pressed, in
 * order to use that key in shortcut sequences.  Note that the meta key differs
 * subtly between operating systems.
 *
 * @date	29-May03-2018 8:21 am, Code HQ Bergmannstrasse
 * @return	ETrue if the meta key is pressed, else EFalse
 */

TBool CWindow::MetaPressed()
{

#ifdef __APPLE__

	return(m_bMetaPressed);

#else

	return(CtrlPressed());

#endif

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
 * Triggers an update of the window.
 * This method is for use when clearing the background of the window is desired. Usually, it is not required to
 * manually call this method, as The Framework will take care of such things as redrawing the window when needed.
 * However, in certain circumstances, such as when the client has no draw() method, or when that method does not
 * do anything, it may be necessary to call this method to clear the background of the window.
 *
 * @pre		The window has been created with open()
 *
 * @date	Wednesday 29-Jan-2025 5:59 am, Code HQ Tokyo Tsukuda
 */

void CWindow::Update()
{

#ifdef __amigaos__

#elif defined(QT_GUI_LIB)

	GetRootWindow()->m_poWindow->update();

#else /* ! QT_GUI_LIB */

	ASSERTM(RedrawWindow(m_poWindow, NULL, NULL, RDW_ERASE | RDW_INVALIDATE), "CWindow::Update() => Unable to update window");

#endif /* ! QT_GUI_LIB */

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

#ifdef __amigaos__

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
