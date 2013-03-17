
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdGadgets.h"
#include "StdReaction.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/gadtools.h>
#include <proto/utility.h>
#include <intuition/gui.h>
#include <intuition/imageclass.h>

#elif defined(QT_GUI_LIB)

#include <QtGui/QKeyEvent>
#include <QtGui/QMenuBar>
#include "Qt/StdWindow.h"

/* Array of key mappings for mapping Windows keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, Qt::Key_Shift }, { STD_KEY_CONTROL, Qt::Key_Shift }, { STD_KEY_BACKSPACE, Qt::Key_Backspace },
	{ STD_KEY_ENTER, Qt::Key_Return }, { STD_KEY_UP, Qt::Key_Up }, { STD_KEY_DOWN, Qt::Key_Down },
	{ STD_KEY_LEFT, Qt::Key_Left }, { STD_KEY_RIGHT, Qt::Key_Right }, { STD_KEY_HOME, Qt::Key_Home },
	{ STD_KEY_END, Qt::Key_End }, { STD_KEY_PGUP, Qt::Key_PageUp }, { STD_KEY_PGDN, Qt::Key_PageDown },
	{ STD_KEY_DELETE, Qt::Key_Delete }, { STD_KEY_TAB, Qt::Key_Tab }, { STD_KEY_F3, Qt::Key_F3 },
	{ STD_KEY_F6, Qt::Key_F6 }, { STD_KEY_F12, Qt::Key_F12 }
};

#define NUM_KEYMAPPINGS 17

#elif defined(WIN32)

/* Array of key mappings for mapping Windows keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, VK_SHIFT }, { STD_KEY_CONTROL, VK_CONTROL }, { STD_KEY_BACKSPACE, VK_BACK },
	{ STD_KEY_ENTER, VK_RETURN }, { STD_KEY_UP, VK_UP }, { STD_KEY_DOWN, VK_DOWN },
	{ STD_KEY_LEFT, VK_LEFT }, { STD_KEY_RIGHT, VK_RIGHT }, { STD_KEY_HOME, VK_HOME },
	{ STD_KEY_END, VK_END }, { STD_KEY_PGUP, VK_PRIOR }, { STD_KEY_PGDN, VK_NEXT },
	{ STD_KEY_DELETE, VK_DELETE }, { STD_KEY_TAB, VK_TAB }, { STD_KEY_F3, VK_F3 },
	{ STD_KEY_F6, VK_F6 }, { STD_KEY_F12, VK_F12 }
};

#define NUM_KEYMAPPINGS 17

TBool CWindow::m_bAltPressed;		/* ETrue if alt is currently pressed */
CWindow *CWindow::m_poActiveDialog;	/* Ptr to currently active dialog, if any */

#endif /* WIN32 */

TBool CWindow::m_bCtrlPressed;      /* ETrue if ctrl is currently pressed */

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

	/* If this is a mouse wheel event then convert the delta to a Windows style reading */
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

/* Written: Saturday 26-Jan-2013 11:49 am, Code HQ Ehinger Tor */
/* @param	a_poKeyEvent	Ptr to a structure containing information about the event */
/*			a_bKeyDown		true if a key press is being handled, else false for key a release */
/* This is the internal function which handles both key presses and key releases under Qt */

void CQtWindow::HandleKeyEvent(QKeyEvent *a_poKeyEvent, bool a_bKeyDown)
{
	TInt NativeKey, Index;

	/* Scan through the key mappings and find the one that has just been pressed or released */

	NativeKey = a_poKeyEvent->key();

	for (Index = 0; Index < NUM_KEYMAPPINGS; ++Index)
	{
		if (g_aoKeyMap[Index].m_iNativeKey == NativeKey)
		{
			break;
		}
	}

	/* If the key has a key mapping then send the standard key to the client */

	if (Index < NUM_KEYMAPPINGS)
	{
		m_poWindow->OfferKeyEvent(g_aoKeyMap[Index].m_iStdKey, a_bKeyDown);
	}

	/* Otherwise it is an ASCII key event so call the CWindow::OfferKeyEvent() function, passing in only */
	/* valid ASCII characters */

	else
	{
		QString String = a_poKeyEvent->text();

		if (String.length() >= 1)
		{
			m_poWindow->OfferKeyEvent(String[0].toAscii(), a_bKeyDown);
		}
	}
}

/* Written: Saturday 23-Feb-2013 1:38 pm */
/* @param	a_poCloseEvent	Ptr to structure containing information for handling the event */
/* This function is called whenever the main window is about to be closed due to the close */
/* button or <alt-f4> being pressed.  It cancels that event, thus preventing the window */
/* from closing, and sends a message to the client so that it can handle the close event as */
/* it wishes (for example by prompting the user whether they wish to quit) */

void CQtWindow::closeEvent(QCloseEvent *a_poCloseEvent)
{
	/* Cancel the close event */

	a_poCloseEvent->ignore();

	/* Allow the client window itself to handle the close */

	m_poWindow->HandleCommand(IDCANCEL);
}

/* Written: Friday 31-Aug-2012 3:02 pm */
/* @param	a_poKeyEvent	Ptr to a structure containing information about the event */
/* This function is called whenever a key down event occurs and will pass the event along */
/* to the underlying framework window in the expected format, filtering out any key events */
/* in which the framework is not interested */

void CQtWindow::keyPressEvent(QKeyEvent *a_poKeyEvent)
{
	/* If this is a control key then indicate this in case the client queries about it */

	if (a_poKeyEvent->key() == Qt::Key_Control)
	{
		m_poWindow->m_bCtrlPressed = ETrue;
	}

	/* Perform standard keyboard handling for the key down event */

	HandleKeyEvent(a_poKeyEvent, ETrue);
}

/* Written: Saturday 26-Jan-2013 11:42 am, Code HQ Ehinger Tor */
/* @param	a_poKeyEvent	Ptr to a structure containing information about the event */
/* This function is called whenever a key up event occurs and will pass the event along */
/* to the underlying framework window in the expected format, filtering out any key events */
/* in which the framework is not interested */

void CQtWindow::keyReleaseEvent(QKeyEvent *a_poKeyEvent)
{
	/* If this is a control key then indicate it is no longer pressed */

	if (a_poKeyEvent->key() == Qt::Key_Control)
	{
		m_poWindow->m_bCtrlPressed = EFalse;
	}

	/* Perform standard keyboard handling for the key release event */

	HandleKeyEvent(a_poKeyEvent, EFalse);
}

/* Written: Thursday 06-Sep-2012 1:35 pm */
/* @param	a_poPaintEvent	Ptr to a structure containing information about the event */
/* This function is called whenever Qt performs a repaint of the window and will pass */
/* the event along to the generic CWindow::Draw() function, so that client code can */
/* perform its custom drawing */

void CQtCentralWidget::paintEvent(QPaintEvent *a_poPaintEvent)
{
	QRect Rect = a_poPaintEvent->rect();

	/* Pass the request along to the underlying framework window.  Note that QRect::bottom() */
	/* will return the inclusive bottom pixel but our framework works with exclusive pixel */
	/* positions.  We therefore need to calculate that position using QRect::height() */

	m_poWindow->Draw(Rect.top(), (Rect.top() + Rect.height()));
}

/* Written: Saturday 25-Aug-2012 1:36 pm */
/* @param	a_poResizeEvent	Ptr to a structure containing information about the event */
/* This function is called whenever Qt performs a resize of the window and will pass */
/* the event along to the generic CWindow::Resize() function, to notify client code */

void CQtWindow::resizeEvent(QResizeEvent * /*a_poResizeEvent*/)
{
	/* The size passed in the QResizeEvent is the size of the window itself.  We need to */
	/* use the size of the central widget as we want to exclude the height of the menu bar */

	QSize Size = centralWidget()->size();
	m_poWindow->InternalResize(Size.width(), Size.height());
}

#elif defined(WIN32)

/* Written: Saturday 08-May-2010 4:43 pm */

LRESULT CALLBACK CWindow::WindowProc(HWND a_poWindow, UINT a_uiMessage, WPARAM a_oWParam, LPARAM a_oLParam)
{
	int Index;
	LRESULT RetVal;
	CStdGadget *Gadget;
	CStdGadgetLayout *LayoutGadget;
	CWindow *Window;

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

			Window->Activated(!(a_oWParam == WA_INACTIVE));

			/* And forget about the alt and ctrl keypresses as we won't get a WM_KEYUP for them now */
			/* that we no longer have focus */

			m_bAltPressed = m_bCtrlPressed = EFalse;

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
			/* Call the CWindow::HandleCommand() function so the client can process the message */

			Window->HandleCommand(LOWORD(a_oWParam));

			break;
		}

		case WM_CHAR :
		{
			/* If the ctrl key is currently pressed then convert the keycode back to standard ASCII, */
			/* but NOT if alt is also pressed or it will break German keymappings that use altgr! */

			if ((m_bCtrlPressed) && (!(m_bAltPressed)))
			{
				a_oWParam |= 0x60;
			}

			/* Call the CWindow::OfferKeyEvent() function, passing in only valid ASCII characters */

			if ((a_oWParam >= 32) && (a_oWParam <= 254) && (a_oWParam != 127))
			{
				Window->OfferKeyEvent(a_oWParam, ETrue);
			}

			break;
		}

		case WM_KEYDOWN :
		case WM_KEYUP :
		{
			/* Scan through the key mappings and find the one that has just been pressed */

			for (Index = 0; Index < NUM_KEYMAPPINGS; ++Index)
			{
				if (g_aoKeyMap[Index].m_iNativeKey == (int) a_oWParam)
				{
					break;
				}
			}

			/* If it was a known key then convert it to the standard value and pass it to the */
			/* CWindow::OfferKeyEvent() function */

			if (Index < NUM_KEYMAPPINGS)
			{
				Window->OfferKeyEvent(g_aoKeyMap[Index].m_iStdKey, (a_uiMessage == WM_KEYDOWN));
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
			RetVal = DefWindowProc(a_poWindow, a_uiMessage, a_oWParam, a_oLParam);

			break;
		}
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
	ASSERTM(m_poWindow, "CWindow::Activate() => Window must be already open");

#ifdef __amigaos4__

	IIntuition->SetAttrs(m_poWindowObj, WINDOW_FrontBack, WT_FRONT, TAG_DONE);
	IIntuition->SetWindowAttrs(m_poWindow, WA_Activate, TRUE, TAG_DONE);

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	DEBUGCHECK((SetActiveWindow(m_poWindow) != NULL), "CWindow::Activate() => Unable to activate window");

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

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

/* Written: Tuesday 10-Apr-2012 7:07 am, Code HQ Ehinger Tor */
/* @param	a_iItemID	ID of the menu item to be checked or unchecked */
/* @param	a_bEnable	ETrue to check the menu item, else EFalse to uncheck it */
/* This function will enable or disable the checkmark in a checkable menu item */

void CWindow::CheckMenuItem(TInt a_iItemID, TBool a_bEnable)
{

#ifdef __amigaos4__

	ULONG FullMenuNum;
	struct MenuItem *MenuItem;

	/* Map the menu item's ID onto a value the can be used by Intuition's menu system */

	if ((FullMenuNum = FindMenuMapping(m_poApplication->MenuMappings(), m_poApplication->NumMenuMappings(), a_iItemID)) != 0)
	{
		/* Now use the result to find the actual menu in the menu strip */

		if ((MenuItem = IIntuition->ItemAddress(m_poApplication->Menus(), FullMenuNum)) != NULL)
		{
			/* Enable or disable the menu item's check mark as appropriate */

			IIntuition->ClearMenuStrip(m_poWindow);

			if (a_bEnable)
			{
				MenuItem->Flags |= CHECKED;
			}
			else
			{
				MenuItem->Flags &= ~CHECKED;
			}

			IIntuition->ResetMenuStrip(m_poWindow, m_poApplication->Menus());
		}
		else
		{
			Utils::Info("CWindow::CheckMenuItem() => Menu item not found");
		}
	}
	else
	{
		Utils::Info("CWindow::CheckMenuItem() => Menu mapping not found");
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((::CheckMenuItem(GetMenu(m_poWindow), a_iItemID, (a_bEnable) ? MF_CHECKED : MF_UNCHECKED) != -1),
		"CWindow::CheckMenuItem() => Unable to set menu checkmark state");

#endif /* ! QT_GUI_LIB */

}

#ifdef QT_GUI_LIB

/* Written: Sunday 05-Jan-2013 7:53 am, Code HQ Ehinger Tor */
/* @returns	ETrue if all menus were created successfully, else EFalse */
/* Creates a set of menus specific to this window, using the list of SStdMenuItem structures */
/* available from RApplication via RApplication::MenuItems().  The menus are created in here */
/* rather than in RApplication as some platforms require a separate copy of the menus to be */
/* created for each window opened, or they cannot create the menus at application creation time */

TBool CWindow::CreateMenus()
{
	TBool RetVal;
	const struct SStdMenuItem *MenuItem;
	QAction *Action;
	QMenu *Menu;
	QString Shortcut;

	/* Assume success */

	RetVal = ETrue;

	/* Iterate through the list of menu structures passed in and create a menu item for each */
	/* one as appropriate */

	MenuItem = m_poApplication->MenuItems();
	Menu = NULL;

	do
	{
		/* If this is a title then create a new drop down menu to which to add menu items */

		if (MenuItem->m_eType == EStdMenuTitle)
		{
			/* And add the new drop down menu to the window's menu bar */

			if ((Menu = m_poWindow->menuBar()->addMenu(MenuItem->m_pccLabel)) == NULL)
			{
				Utils::Info("CWindow::CreateMenus() => Unable to create drop down menu");

				RetVal = EFalse;

				break;
			}
		}
		else
		{
			ASSERTM((Menu != NULL), "CWindow::CreateMenus() => Menu bar must be created before a menu item can be added");

			/* Otherwise create a new menu item and add it to the previously created drop down menu */

			// TODO: CAW - Why isn't this causing leaks in /var/log/syslog?
			if ((Action = new CQtAction(MenuItem->m_iCommand, MenuItem->m_pccLabel, m_poWindow)) != NULL)
			{
				/* If this is a checkable menu option or a separator then adjust the style of the newly */
				/* added menu item to reflect this */

				if (MenuItem->m_eType == EStdMenuCheck)
				{
					Action->setCheckable(true);
				}
				else if (MenuItem->m_eType == EStdMenuSeparator)
				{
					Action->setSeparator(true);
				}

				/* If the menu item has a hotkey then generate a key sequence and assign it to the action */

				if (MenuItem->m_pccHotKey)
				{
					/* The most convenient type of QKeySequence to use for our purposes is */
					/* a fully text based one, so start by converting the modifier keys to */
					/* a textual prefix */

					if (MenuItem->m_iHotKeyModifier == STD_KEY_CONTROL)
					{
						Shortcut = "Ctrl+";
					}
					else if (MenuItem->m_iHotKeyModifier == STD_KEY_ALT)
					{
						Shortcut = "Alt+";
					}
					else if (MenuItem->m_iHotKeyModifier == STD_KEY_SHIFT)
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

					Shortcut.append(MenuItem->m_pccHotKey);
					Action->setShortcut(Shortcut);
				}

				/* And add the new menu item to the drop down menu */

				Menu->addAction(Action);
			}
			else
			{
				Utils::Info("CWindow::CreateMenus() => Unable to create menu item");

				RetVal = EFalse;

				break;
			}
		}

		++MenuItem;
	}
	while (MenuItem->m_eType != EStdMenuEnd);

	return(RetVal);
}

#endif /* QT_GUI_LIB */

/* Written: Saturday 05-Jan-2013 1:12 pm, Code HQ Ehinger Tor */
/* @returns	ETrue if the control key is pressed, else EFalse */
/* Returns whether or not the control key is currently pressed */

TBool CWindow::CtrlPressed()
{

#if defined(WIN32) && !defined(QT_GUI_LIB)

	/* Due to Windows using the control key to simulate usage of the ALT GR */
	/* key, rather than using a dedicated keycode for ALT GR, we have to have */
	/* a workaround to differentiate between control being pressed and ALT GR */
	/* being pressed */

	return((m_bCtrlPressed) && (!(m_bAltPressed)));

#else /* ! defined(WIN32) && !defined(QT_GUI_LIB) */

	return(m_bCtrlPressed);

#endif /* ! defined(WIN32) && !defined(QT_GUI_LIB) */

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

	if (m_poWindowObj)
	{
		IIntuition->DisposeObject(m_poWindowObj);
		m_poWindowObj = NULL;
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	/* If the window is open, close it and indicate that it is no longer open */

	if (m_poWindow)
	{
		DEBUGCHECK((DestroyWindow(m_poWindow) != 0), "CWindow::Close() => Unable to destroy window");
		m_poWindow = NULL;
	}

	/* If the window's class is registered, unregister it and indicate that it is no longer registered */

	if (m_poWindowClass)
	{
		DEBUGCHECK((UnregisterClass((LPCTSTR) m_poWindowClass, GetModuleHandle(NULL)) != FALSE), "CWindow::Close() => Unable to unregister window class");
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

	ASSERTM(m_poWindowObj, "CWindow::CompleteOpen() => Reaction window must be already open");

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

	if (a_iWidth != -1)
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

/* Written: Sunday 08-Apr-2011 8:50 am, Code HQ Ehinger Tor */
/* @param	a_iItemID	ID of the menu item to be enabled or disabled */
/* @param	a_bEnable	ETrue to enable the menu item, else EFalse to disable it */
/* This function will enable or disable an item in the window's menu list */

void CWindow::EnableMenuItem(TInt a_iItemID, TBool a_bEnable)
{

#ifdef __amigaos4__

	ULONG FullMenuNum;

	/* Map the menu item's ID onto a value the can be used by Intuition's menu system */

	if ((FullMenuNum = FindMenuMapping(m_poApplication->MenuMappings(), m_poApplication->NumMenuMappings(), a_iItemID)) != 0)
	{
		/* Enable or disable the menu item as appropriate */

		if (a_bEnable)
		{
			IIntuition->OnMenu(m_poWindow, FullMenuNum);
		}
		else
		{
			IIntuition->OffMenu(m_poWindow, FullMenuNum);
		}
	}
	else
	{
		Utils::Info("CWindow::EnableMenuItem() => Menu mapping not found");
	}

#elif defined(QT_GUI_LIB)

	// TODO: CAW - Implement

#else /* ! QT_GUI_LIB */

	DEBUGCHECK((::EnableMenuItem(GetMenu(m_poWindow), a_iItemID, (a_bEnable) ? MF_ENABLED : MF_DISABLED) != -1),
		"CWindow::EnableMenuItem() => Unable to set menu item state");

#endif /* ! QT_GUI_LIB */

}

#ifdef __amigaos4__

/* Written: Saturday 14-Apr-2012 8:02 am, Code HQ Ehinger Tor */
/* @param	a_poMenuMappings	Ptr to array of menu mappings to search through */
/*			a_poNumMenuMappings	Number of items in the menu mappings array */
/*			a_iItemID			Item ID to be searched for */
/* This function searches through an array of menu mappings, searching for an item of a */
/* particular ID.  It is a helper function to allow accessing menu items in the Windows */
/* manner of addressing them via the ID, rather than the Amiga OS style of using the */
/* menu item's position */

ULONG CWindow::FindMenuMapping(struct SStdMenuMapping *a_poMenuMappings, TInt a_iNumMenuMappings, TInt a_iItemID)
{
	TInt Index;
	ULONG RetVal;

	/* Assume failure */

	RetVal = 0;

	/* Iterate through the list of menu mappings and find the one we are looking for */

	for (Index = 0; Index < a_iNumMenuMappings; ++Index)
	{
		if (a_poMenuMappings->m_iID == a_iItemID)
		{
			RetVal = a_poMenuMappings->m_ulFullMenuNum;

			break;
		}

		++a_poMenuMappings;
	}

	return(RetVal);
}

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

#endif /* __amigaos4__ */

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

/* Written: Monday 08-Feb-2010 7:13 am */

TInt CWindow::Open(const char *a_pccTitle, const char *a_pccScreenName, TBool a_bResizeable)
{
	TInt RetVal, ScreenWidth, ScreenHeight;

	/* Assume failure */

	RetVal = KErrGeneral;

	/* Get the size of the screen so we can open the window filling its full size */

	Utils::GetScreenSize(&ScreenWidth, &ScreenHeight);

#ifdef __amigaos4__

	/* Setup an IDCMP hook that can be used for monitoring gadgets for extra information not */
	/* provided by Reaction, such as the movement of proportional gadgets */

	m_oIDCMPHook.h_Entry = (ULONG (*)()) IDCMPFunction;
	m_oIDCMPHook.h_Data = this;

	/* Create a Reaction Window and open it on the requested screen at the maximum size of */
	/* the screen.  If no screen name is specified, fall back to the Workbench */

	m_poWindowObj = (Object *) WindowObject,
		WA_Title, (ULONG) a_pccTitle, WINDOW_Position, WPOS_CENTERSCREEN,
		WA_PubScreenName, a_pccScreenName, WA_PubScreenFallBack, TRUE,
		WA_Width, ScreenWidth, WA_Height, ScreenHeight, WA_Activate, TRUE,
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

			/* Indicate success */

			RetVal = KErrNone;

			/* Calculate the inner width and height of the window, for l8r use */

			m_iInnerWidth = (m_poWindow->Width - (m_poWindow->BorderRight + m_poWindow->BorderLeft));
			m_iInnerHeight = (m_poWindow->Height - (m_poWindow->BorderBottom + m_poWindow->BorderTop));
		}
		else
		{
			IIntuition->DisposeObject(m_poWindowObj);
			m_poWindowObj = NULL;

			Utils::Info("CWindow::Open() => Unable to open window");
		}
	}
	else
	{
		Utils::Info("CWindow::Open() => Unable to create window");
	}

#elif defined(QT_GUI_LIB)

	(void) a_pccScreenName;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Allocate a window based on the QMainWindow class */

	if ((m_poWindow = new CQtWindow(this)) != NULL)
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
				/* Set the window to the size of the desktop and display it */

				m_poWindow->showMaximized();

				/* And save the size of the client area */

				QSize Size = m_poCentralWidget->size();
				m_iInnerWidth = Size.width();
				m_iInnerHeight = Size.height();
			}
			else
			{
				Utils::Info("CWindow::Open() => Unable to create menus");
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
				Rect.right, Rect.bottom, 0, 0, Instance, 0)) != NULL)
			{
				/* Save a ptr to the window handle for use in the WindowProc() routine */

				SetWindowLong(m_poWindow, GWL_USERDATA, (long) this);

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

/* Written: Saturday 05-Nov-2011 9:03 am, Code HQ S�flingen */

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

/* Written: Saturday 05-Nov-2011 12:04 pm, Code HQ S�flingen */

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
