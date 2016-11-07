
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdReaction.h"
#include "StdRendezvous.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include "Amiga/AmiMenus.h"
#include <proto/gadtools.h>
#include <proto/keymap.h>
#include <ctype.h>

/* Array of key mappings for mapping Amiga keys onto standard keys.  Amiga OS differentiates between left alt and */
/* right alt.  When handling keyboard input, we will treat the right alt key differently depending on whether the */
/* keyboard layout has an alt or an alt-gr key, so keep track of the index of this entry in this array with a #define */

#define RALT_INDEX 4

static SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, 0x60 }, { STD_KEY_SHIFT, 0x61 }, { STD_KEY_CONTROL, 0x63 }, { STD_KEY_ALT, 0x64 },
	{ STD_KEY_ALT, 0x65 }, { STD_KEY_BACKSPACE, 0x41 }, { STD_KEY_ENTER, 0x44 }, { STD_KEY_UP, 0x4c },
	{ STD_KEY_DOWN, 0x4d }, { STD_KEY_LEFT, 0x4f }, { STD_KEY_RIGHT, 0x4e },
	{ STD_KEY_HOME, 0x70 }, { STD_KEY_END, 0x71 }, { STD_KEY_PGUP, 0x48 },
	{ STD_KEY_PGDN, 0x49 }, { STD_KEY_DELETE, 0x46 }, { STD_KEY_TAB, 0x42 },
	{ STD_KEY_ESC, 0x45 }, { STD_KEY_F1, 0x50 }, { STD_KEY_F2, 0x51 }, { STD_KEY_F3, 0x52 },
	{ STD_KEY_F4, 0x53 }, { STD_KEY_F5, 0x54 }, { STD_KEY_F6, 0x55 }, { STD_KEY_F7, 0x56 },
	{ STD_KEY_F8, 0x57 }, { STD_KEY_F9, 0x58 }, { STD_KEY_F10, 0x59 }, { STD_KEY_F11, 0x4b },
	{ STD_KEY_F12, 0x6f }
};

#define NUM_KEYMAPPINGS (sizeof(g_aoKeyMap) / sizeof(struct SKeyMapping))

#elif defined(QT_GUI_LIB)

#include <QtCore/QTextCodec>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>

/* Fake arguments for the QApplication constructor, which must have them passed in.  We use our */
/* own argument system so we don't use Qt's.  This makes our argument handling incompatible with */
/* Qt's as programs written using The Framework won't respond to the standard Qt arguments passed */
/* in, but that is the price to pay for using a framework within a framework!  The C++ standard */
/* will (annoyingly) put string literals in the .text section, so we work around this with the */
/* g_acBuffer variable, which is a binary representation of a string, but in the .bss section */

static char g_acBuffer[2] = { '\0' };		/* Empty string in the .bss section */
static char *g_acArgV[1] = { g_acBuffer };	/* Fake argv, which would usually be used for the main window's title */
static TInt g_iArgC = 1;					/* Fake argc, which must be > 0 */

#endif /* QT_GUI_LIB */

/* Written: Saturday 26-Jun-2010 11:53 am */

RApplication::RApplication()
{

#ifdef __amigaos4__

	m_bDone = EFalse;
	m_ulMainSeconds = m_ulMainMicros = 0;
	m_iLastX = m_iLastY = 0;

#elif defined(WIN32) && !defined(QT_GUI_LIB)

	m_poCurrentDialog = NULL;

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	m_poWindows = NULL;
	m_pcoMenuItems = NULL;
}

/**
 * Prepares the application for use.
 * Applications that derive from this class should call this method to initialise the class
 * and to ensure that the RApplication is prepared for handling its incoming messages.  The
 * menu items passed into this function must be in persistent memory and persist for the
 * lifetime of the instance.
 *
 * @date	Thursday 01-Jul-2010 6:46 am
 * @param	a_pcoMenuItems	Array of SStdMenuItem structures containing the dropdown menus and
 *							menu items to be created
 * @return	KErrNone if successful
 * @return  KErrNoMemory if not enough memory was available
 */

TInt RApplication::Open(const struct SStdMenuItem *a_pcoMenuItems)
{
	TInt RetVal;

	/* Save the ptr to the original menu item structures so they can be used for creating window */
	/* specific menus l8r */

	m_pcoMenuItems = a_pcoMenuItems;

#ifdef __amigaos4__

	/* Assume success */

	RetVal = KErrNone;

	m_bUseAltGr = ETrue;

	/* If the keyboard layout has an alt-gr key then filter out the mapping that would convert it */
	/* to a normal alt key on keybords without an alt-gr key */

	if (m_bUseAltGr)
	{
		g_aoKeyMap[RALT_INDEX].m_iNativeKey = 0x64;
	}

#elif defined(QT_GUI_LIB)

	/* Create an instance of the Qt application class, which is used to control the program */
	/* under Qt.  We have our own argument handling so pass in fake argc and argv arguments */

	m_poApplication = new QApplication(g_iArgC, g_acArgV);
	RetVal = (m_poApplication != NULL) ? KErrNone : KErrNoMemory;

#else /* ! QT_GUI_LIB */

	/* Assume success */

	RetVal = KErrNone;

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/* Written: Monday 08-Feb-2010 6:54 am */

TInt RApplication::Main()
{

#ifdef __amigaos4__

	char KeyBuffer[5];
	const unsigned char *Data;
	TInt DataSize, InnerWidth, InnerHeight, Index, ItemID, NumChars, X, Y;
	TBool DoubleClicked, ExecutedShortcut, KeyDown;
	ULONG Result, Signal, SecondSeconds, SecondMicros;
	UWORD Code, QualifierMask;
	struct InputEvent *InputEvent, ShortcutEvent;
	struct MenuItem *MenuItem;
	struct Message *Message;
	CWindow *Window;
	TStdMouseEvent MouseEvent;

	ASSERTM(m_poWindows, "RApplication::Main() => Application must have at least one window");

	do
	{
		Signal = IExec->Wait(m_ulWindowSignals | g_oRendezvous.GetSignal());

		/* Check to see if a message was received by the rendezvous port */

		if (Signal & g_oRendezvous.GetSignal())
		{
			Message = IExec->GetMsg(g_oRendezvous.GetMessagePort());

			/* Extract the payload from the message and let the RRendezvous class know that a message was received */

			DataSize = (Message->mn_Length - sizeof(struct Message));
			Data = (DataSize > 0) ? (unsigned char *) (Message + 1) : NULL;
			g_oRendezvous.MessageReceived(Data, DataSize);

			/* And reply to the client, to let it know that the message has been processed */

			IExec->ReplyMsg(Message);
		}

		Window = m_poWindows;

		while (Window)
		{
			if (Signal & Window->GetSignal())
			{
				break;
			}

			Window = Window->m_poNext;
		}

		if (Window)
		{
			while ((Result = RA_HandleInput(Window->m_poWindowObj, &Code)) != WMHI_LASTMSG)
			{
				switch (Result & WMHI_CLASSMASK)
				{
					case WMHI_ACTIVE :
					{
						/* Window focus is changing so let the client know */

						Window->m_bIsActive = ETrue;
						Window->Activated(ETrue);

						break;
					}

					case WMHI_CLOSEWINDOW :
					{
						Window->HandleCommand(IDCANCEL);

						break;
					}

					case WMHI_GADGETUP :
					{
						Window->HandleCommand(Result & WMHI_GADGETMASK);

						break;
					}

					case WMHI_INACTIVE :
					{
						/* Window focus is changing so let the client know */

						Window->m_bIsActive = EFalse;
						Window->Activated(EFalse);

						/* Forget about the modifier keypresses as we won't get a key up event for them due */
						/* to the window no longer being active */

						CWindow::m_bAltPressed = CWindow::m_bCtrlPressed = CWindow::m_bShiftPressed = EFalse;

						break;
					}

					case WMHI_MENUPICK :
					{
						/* Scan through the messages, processing each one.  There may be multiple */
						/* messages waiting due to the user selecting more than one menu item.  We */
						/* have to check for Code being 0 in case the menu strip is updated while we */
						/* are processing the menus, in which case Code will never get set to MENUNULL */

						while ((Code != MENUNULL) && (Code != 0))
						{
							/* Get the address of the underlying Intuition MenuItem and from that */
							/* we can extract the command ID of the menu item from the user data */
							/* field and pass it to CWindow::HandleCommand() */

							if ((MenuItem = IIntuition->ItemAddress(Window->Menus(), Code)) != NULL)
							{
								ItemID = (TInt) GTMENUITEM_USERDATA(MenuItem);

								/* If the menu item is checkable then we need to update the internal state of */
								/* the checkmark that is kept by the CAmiMenus class so that the checkmark is */
								/* recreated in the correct state if the menus are updated */

								if (MenuItem->Flags & CHECKIT)
								{
									Window->m_poAmiMenus->UpdateCheckStatus(ItemID, (MenuItem->Flags & CHECKED));
								}

								/* Now send the command to the client */

								Window->HandleCommand((TInt) GTMENUITEM_USERDATA(MenuItem));
							}

							/* And get the code of the next menu item selected */

							Code = IIntuition->ItemAddress(Window->Menus(), Code)->NextSelect;
						}

						break;
					}

					case WMHI_MOUSEBUTTONS :
					{
						/* If this event was a mouse button event then get the position of the mouse, convert */
						/* it to client area relative dimensions and call the windows' HandlePointerEvent() */
						/* function.  Start by determining the event type and mouse position in the window */

						DoubleClicked = EFalse;
						X = (Window->m_poWindow->MouseX - Window->m_poWindow->BorderLeft);
						Y = (Window->m_poWindow->MouseY - Window->m_poWindow->BorderTop);

						/* Only handle the mouse event if it is within the bounds of the client area */

						if ((X >= 0) && (Y >= 0))
						{
							/* See if this is a double click.  If it is then we need to send the */
							/* EStdMouseDoubleClick message to the client window and not do anything else */

							if (Code == SELECTDOWN)
							{
								IIntuition->CurrentTime(&SecondSeconds, &SecondMicros);

								/* Only handle this as a double click if the second click is at the same */
								/* X and Y position as the first */

								if ((X == m_iLastX) && (Y == m_iLastY))
								{
									/* Double click? */

									if (IIntuition->DoubleClick(m_ulMainSeconds, m_ulMainMicros, SecondSeconds, SecondMicros))
									{
										/* Yep!  Signal this and reset the double click time to avoid a third */
										/* click getting treated as a double click */

										DoubleClicked = ETrue;
										m_ulMainSeconds = m_ulMainMicros = 0;

										/* And send the appropriate message to the client window */

										Window->HandlePointerEvent(X, Y, EStdMouseDoubleClick);
									}
								}

								/* If not a double click, save the time amd mouse position for checking next time */

								if (!(DoubleClicked))
								{
									m_ulMainSeconds = SecondSeconds;
									m_ulMainMicros = SecondMicros;
									m_iLastX = X;
									m_iLastY = Y;
								}
							}

							/* If a double click wasn't handled then send the appropriate message to the */
							/* client window */

							if (((Code == SELECTDOWN) && (!(DoubleClicked))) || (Code == SELECTUP))
							{
								/* Send the message */

								MouseEvent = (Code == SELECTDOWN) ? EStdMouseDown : EStdMouseUp;
								Window->HandlePointerEvent(X, Y, MouseEvent);

								/* And save the mouse position for next time */

								m_iLastX = X;
								m_iLastY = Y;
							}
						}

						break;
					}

					case WMHI_MOUSEMOVE :
					{
						X = (Window->m_poWindow->MouseX - Window->m_poWindow->BorderLeft);
						Y = (Window->m_poWindow->MouseY - Window->m_poWindow->BorderTop);

						/* Only handle the mouse event if it is within the bounds of the client area */

						if ((X >= 0) && (Y >= 0))
						{
							Window->HandlePointerEvent(X, Y, EStdMouseMove);
						}

						break;
					}

					case WMHI_RAWKEY :
					{
						/* Mixing IDCMP_RAWKEY and IDCMP_VANILLAKEY doesn't really work on Amiga OS so */
						/* we will handle cooking the keys ourselves.  Assume to start with that the */
						/* cooked key will not be handled */

						/* Get a ptr to the InputEvent from Reaction */

						if (IIntuition->GetAttr(WINDOW_InputEvent, Window->m_poWindowObj, (ULONG *) &InputEvent) > 0)
						{
							KeyDown = (!(Code & IECODE_UP_PREFIX));
							Code = (Code & ~IECODE_UP_PREFIX);

							/* Scan through the key mappings and find the one that has just been pressed */

							for (Index = 0; Index < (TInt) NUM_KEYMAPPINGS; ++Index)
							{
								if (g_aoKeyMap[Index].m_iNativeKey == Code)
								{
									break;
								}
							}

							/* If it was a known key then convert it to the standard value and pass it to the */
							/* CWindow::OfferKeyEvent() function */

							if (Index < (TInt) NUM_KEYMAPPINGS)
							{
								/* Save the current alt, control and shift key states */

								if (g_aoKeyMap[Index].m_iStdKey == STD_KEY_ALT)
								{
									CWindow::m_bAltPressed = (KeyDown) ? ETrue : EFalse;
								}

								if (g_aoKeyMap[Index].m_iStdKey == STD_KEY_CONTROL)
								{
									CWindow::m_bCtrlPressed = (KeyDown) ? ETrue : EFalse;
								}

								if (g_aoKeyMap[Index].m_iStdKey == STD_KEY_SHIFT)
								{
									CWindow::m_bShiftPressed = (KeyDown) ? ETrue : EFalse;
								}

								/* If this was a key down event then execute any menu item shortcuts that match */
								/* the key that was just pressed */

								ExecutedShortcut = EFalse;

								if (KeyDown)
								{
									/* It is possible for the window to not have any menus attached so check this */

									if (Window->m_poAmiMenus)
									{
										ExecutedShortcut = Window->m_poAmiMenus->ExecuteShortcut(g_aoKeyMap[Index].m_iStdKey, CWindow::m_bCtrlPressed);
									}
								}

								/* Pass the key event onto the client, but only if no menu shortcut was executed */

								if (!(ExecutedShortcut))
								{
									Window->OfferKeyEvent(g_aoKeyMap[Index].m_iStdKey, KeyDown);
								}
							}

							/* The raw key was not handled so if this is a key down event then cook the key */
							/* and if the resulting key is a printable ASCII key then send it to the client */
							/* code and we're done */

							else
							{
								if (KeyDown)
								{
									/* Copy the InputEvent and remove any control/alt qualifiers associated with it.  This */
									/* makes it easier to find out what key was pressed without having to concern ourselves */
									/* with changes made by the control or alt keys being pressed.  However, we do want to */
									/* keep the shift qualifiers as filtering out these prevents control key combinations */
									/* that involve shifted characters */

									ShortcutEvent = *InputEvent;
									ShortcutEvent.ie_Qualifier &= (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT);

									/* Cook the raw key press and pass it to the CAmiMenus class to execute any */
									/* menu item shortcuts that match the key that was just pressed */

									ExecutedShortcut = EFalse;

									if ((NumChars = IKeymap->MapRawKey(&ShortcutEvent, KeyBuffer, sizeof(KeyBuffer), NULL)) > 0)
									{
										/* It is possible for the window to not have any menus attached so check this */

										if (Window->m_poAmiMenus)
										{
											ExecutedShortcut = Window->m_poAmiMenus->ExecuteShortcut(toupper(KeyBuffer[0]), CWindow::m_bCtrlPressed);
										}
									}

									/* Now perform the normal keyboard handling, but only if no menu shortcut was executed */

									if (!(ExecutedShortcut))
									{
										/* We want all keys to be processed along with their qualifiers, except the control */
										/* key and the left alt key.  These alter the cooked key values in unwanted ways so */
										/* we filter them out */

										QualifierMask = (IEQUALIFIER_CONTROL | IEQUALIFIER_LALT);

										/* If we are using a keyboard that does not have an alt-gr key then we also filter */
										/* the right alt key out from processing so that it acts like the left alt key.  If */
										/* the keyboard has an alt-gr key then we want it to be cooked so that we receive */
										/* the special characters that it generates */

										if (!m_bUseAltGr)
										{
											QualifierMask |= IEQUALIFIER_RALT;
										}

										if (!(InputEvent->ie_Qualifier & QualifierMask))
										{
											ShortcutEvent.ie_Qualifier = InputEvent->ie_Qualifier;
										}

										if ((NumChars = IKeymap->MapRawKey(&ShortcutEvent, KeyBuffer, sizeof(KeyBuffer), NULL)) > 0)
										{
											/* If the ctrl key is currently pressed then convert the keycode back to lower case, */
											/* but NOT if alt is also pressed or it will break German keymappings that use altgr! */

											if ((CWindow::m_bCtrlPressed) && (!(CWindow::m_bAltPressed)))
											{
												/* Convert the key to lower case, if it is an upper case letter */

												if ((KeyBuffer[0] >= 65) && (KeyBuffer[0] <= 90))
												{
													KeyBuffer[0] |= 0x20;
												}
											}

											/* Call the CWindow::OfferKeyEvent() function, passing in only valid ASCII characters */

											if ((KeyBuffer[0] >= 32) && (KeyBuffer[0] <= 254))
											{
												Window->OfferKeyEvent(KeyBuffer[0], ETrue);
											}
										}
									}
								}
							}

							/* Pass the raw key onto the CWindow::OfferRawKeyEvent() function, without any kind */
							/* of preprocessing */

							Window->OfferRawKeyEvent(Code, KeyDown);
						}
						else
						{
							Utils::Info("RApplication::Main() => Unable to get InputEvent for WMHI_RAWKEY");
						}

						break;
					}

					// TODO: CAW - Needs to be implemented
					/*case WHMI_REFRESHWINDOW :
					{
						IDOS->Printf("Calling Draw()\n");

						Window->Draw();

						break;
					}*/

					case WMHI_NEWSIZE :
					{
						/* Determine the new size of the window */

						InnerWidth = (Window->m_poWindow->Width - (Window->m_poWindow->BorderRight + Window->m_poWindow->BorderLeft));
						InnerHeight = (Window->m_poWindow->Height - (Window->m_poWindow->BorderBottom + Window->m_poWindow->BorderTop));

						/* And notify the window if this new size */

						Window->InternalResize(InnerWidth, InnerHeight);

						break;
					}
				}

				/* If any redraws were requested then they will be waiting in the dirty region list, so perform the redraw now */

				if (Window->m_voDirtyRegions.size() > 0)
				{
					Window->InternalRedraw();
				}
			}
		}
	}
	while (!(m_bDone));

#elif defined(QT_GUI_LIB)

	m_poApplication->exec();

#elif defined(WIN32)

	MSG Msg;

	/* Standard Windows message loop with accelerator handling */

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		/* If a modeless dialog is currently active then try to handle any keyboard messages */
		/* bound for it using the world's stupidest API.  All messages have to be passed to */
		/* IsDialogMessage() even if their MSG::hwnd member is not the same as that of the */
		/* dialog's! */

		if ((!(m_poCurrentDialog)) || (!(IsDialogMessage(m_poCurrentDialog, &Msg))))
		{
			/* No accelerator or dialog message was found so try to translate the accelerator; */
			/* Windows will handle m_poAccelerators being NULL */

			if (!(TranslateAccelerator(m_poWindows->m_poWindow, m_poWindows->m_poAccelerators, &Msg)))
			{
				/* Otherwise just perform the standard translation and despatch processing */

				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}

#endif /* WIN32 */

	return(KErrNone);
}

/**
 * Closes the class and frees any allocated resources.
 * This function is required by the so-called "Symbian idioms" by which The Framework is written,
 * although in the case of the RApplication class it does not do anything.
 *
 * @date	Tuesday 29-Jun-2010 7:59 pm, London Hackspace
 */

void RApplication::Close()
{
}

/* Written: Monday 08-Feb-2010 7:25 am */

void RApplication::AddWindow(CWindow *a_poWindow)
{
	CWindow *Window;

	ASSERTM((a_poWindow != NULL), "RApplication::AddWindow() => Window ptr must be passed in");

	/* If the window list is empty then make this window the first in the list */

	if (!(m_poWindows))
	{
		m_poWindows = a_poWindow;
	}

	/* Otherwise iterate through the window list and append this window to the end of it */

	else
	{
		Window = m_poWindows;
		ASSERTM((Window != a_poWindow), "RApplication::AddWindow() => Window already on window list");

		while (Window->m_poNext)
		{
			Window = Window->m_poNext;
			ASSERTM((Window != a_poWindow), "RApplication::AddWindow() => Window already on window list");
		}

		Window->m_poNext = a_poWindow;
	}

#ifdef __amigaos4__

	/* And add the new window's signal bit to the list of signals that the application will wait on */

	m_ulWindowSignals |= a_poWindow->GetSignal();

#endif /* __amigaos4__ */

	/* Schedule a redraw to ensure the newly added window is refreshed */

	a_poWindow->DrawNow();
}

/* Written: Saturday 25-Sep-2010 3:18 pm */

void RApplication::RemoveWindow(CWindow *a_poWindow)
{
	CWindow *Window, *PrevWindow;

	ASSERTM((a_poWindow != NULL), "RApplication::RemoveWindow() => Window ptr must be passed in");

	/* Iterate through the list of attached windows and find the window to be removed.  We */
	/* keep track of the window before the one to be removed as well, as this is a single */
	/* linked list of windows only */

	Window = m_poWindows;
	PrevWindow = NULL;

	while ((Window) && (Window != a_poWindow))
	{
		PrevWindow = Window;
		Window = Window->m_poNext;
	}

	ASSERTM((Window != NULL), "RApplication::RemoveWindow() => Cannot find window to be removed");

	/* Remove the window from the list, taking into account that it may or may not be the first */
	/* in the list */

	if (PrevWindow)
	{
		PrevWindow->m_poNext = Window->m_poNext;
	}
	else
	{
		m_poWindows = a_poWindow->m_poNext;
	}

	/* And clear the window's m_poNext ptr in case it is re-added to the window list later */

	a_poWindow->m_poNext = NULL;

#ifdef __amigaos4__

	/* Recalculate the set of window signals that the application will wait on.	 We do it like this */
	/* rather than anding out the signal bit for the window being removed as the underlying Intuition */
	/* window has already been closed by the time this function is called and so is thus inaccessible */

	m_ulWindowSignals = 0;
	Window = m_poWindows;

	while (Window)
	{
		m_ulWindowSignals |= Window->GetSignal();
		Window = Window->m_poNext;
	}

#endif /*  __amigaos4__ */

}

/* Written: Wednesday 27-Oct-2010 8:21 am */

#if defined(WIN32) && !defined(QT_GUI_LIB)

void RApplication::SetCurrentDialog(HWND a_poDialog)
{
	m_poCurrentDialog = a_poDialog;
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

/* Written: Saturday 26-Jun-2010 2:18 pm */

void RApplication::Exit()
{

#ifdef __amigaos4__

	m_bDone = ETrue;

#elif defined(__linux__)

	m_poApplication->quit();

#else /* ! __linux__ */

	PostQuitMessage(0);

#endif /* ! __linux__ */

}
