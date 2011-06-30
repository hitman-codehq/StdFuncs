
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <proto/gadtools.h>
#include <proto/intuition.h>
#include <reaction/reaction.h>
#include <proto/keymap.h>

/* Array of key mappings for mapping Amiga keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, 0x60 }, { STD_KEY_CONTROL, 0x63 }, { STD_KEY_BACKSPACE, 0x41 },
	{ STD_KEY_ENTER, 0x44 }, { STD_KEY_UP, 0x4c }, { STD_KEY_DOWN, 0x4d },
	{ STD_KEY_LEFT, 0x4f }, { STD_KEY_RIGHT, 0x4e }, { STD_KEY_HOME, 0x70 },
	{ STD_KEY_END, 0x71 }, { STD_KEY_PGUP, 0x48 }, { STD_KEY_PGDN, 0x49 },
	{ STD_KEY_DELETE, 0x46 }, { STD_KEY_TAB, 0x42 }, { STD_KEY_ESC, 0x45 },
	{ STD_KEY_F3, 0x52 }, { STD_KEY_F12, 0x6f }
};

#define NUM_KEYMAPPINGS 17

TBool RApplication::m_bCtrlPressed;

#endif /* __amigaos4__ */

/* Written: Saturday 26-Jun-2010 11:53 am */

RApplication::RApplication()
{

#ifdef __amigaos4__

	m_bDone = m_bMenuStripSet = EFalse;
	m_poMenus = NULL;
	m_ulMainSeconds = m_ulMainMicros = 0;
	m_iLastX = m_iLastY = 0;

#else /* ! __amigaos4__ */

	m_poAccelerators = NULL;
	m_poCurrentDialog = NULL;

#endif /* ! __amigaos4__ */

	m_poWindows = NULL;
	m_pcoMenuItems = NULL;
}

/* Written: Wednesday 30-Jun-2010 6:53 am */
/* @param 	a_pcoMenuItems Ptr to array of menu items to be created.  Must be persisted for the lifetime */
/*			of the RApplication class that uses them */

TInt RApplication::CreateMenus(const struct SStdMenuItem *a_pcoMenuItems)
{

#ifdef __amigaos4__

	TInt Index, NumMenuItems, RetVal;
	const struct SStdMenuItem *MenuItem;
	struct NewMenu *NewMenus;
	struct Screen *Screen;
	APTR VisualInfo;

	/* Assume failure */

	RetVal = KErrNoMemory; // TODO:CAW - Proper error + what about Utils::Info() handling?

	/* Iterate through the menu item structures passed in and count how many need to be created */

	NumMenuItems = 1;
	MenuItem = a_pcoMenuItems;

	do
	{
		++NumMenuItems;
		++MenuItem;
	}
	while (MenuItem->m_eType != EStdMenuEnd);

	/* Save the ptr to the original menu item structures as it can be used for menu item */
	/* command translation later */

	m_pcoMenuItems = a_pcoMenuItems;

	/* Allocate a buffer large enough to hold all of the GadTools NewMenu structures and populate */
	/* them with data taken from the generic SStdMenuItem structures passed in */

	if ((NewMenus = new NewMenu[NumMenuItems]) != NULL)
	{
		for (Index = 0; Index < NumMenuItems; ++Index)
		{
			NewMenus[Index].nm_Type = a_pcoMenuItems[Index].m_eType;
			NewMenus[Index].nm_Label = a_pcoMenuItems[Index].m_pccLabel;
			NewMenus[Index].nm_CommKey = a_pcoMenuItems[Index].m_pccHotKey;
			NewMenus[Index].nm_UserData = (APTR) a_pcoMenuItems[Index].m_iCommand;
		}

		/* Lock the default public screen and obtain a VisualInfo structure, in preparation for laying */
		/* the menus out */

		if ((Screen = IIntuition->LockPubScreen(NULL)) != NULL)
		{
			if ((VisualInfo = IGadTools->GetVisualInfo(Screen, TAG_DONE)) != NULL)
			{
				/* Create the menus and lay them out in preparation for display */

				if ((m_poMenus = IGadTools->CreateMenus(NewMenus, GTMN_FrontPen, 1, TAG_DONE)) != NULL)
				{
					if (IGadTools->LayoutMenus(m_poMenus, VisualInfo, GTMN_NewLookMenus, 1, TAG_DONE))
					{
						RetVal = KErrNone;
					}
				}
			}

			/* And unlock the default public screen */

			IIntuition->UnlockPubScreen(NULL, Screen);
		}

		/* The NewMenu structures are no longer required */

		delete [] NewMenus;
	}

	return(RetVal);

#else /* ! __amigaos4__ */

	(void) a_pcoMenuItems;

	return(KErrNone);

#endif /* ! __amigaos4__ */

}

/* Written: Thursday 01-Jul-2010 6:46 am */
/* Applications that derive from this class should call this method to initialise their menus */

TInt RApplication::Open(const struct SStdMenuItem *a_pcoMenuItems)
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;

	/* Create the application's menu, if requested */

	if (a_pcoMenuItems)
	{
		RetVal = CreateMenus(a_pcoMenuItems);
	}

	return(RetVal);
}

/* Written: Monday 08-Feb-2010 6:54 am */

int RApplication::Main()
{

#ifdef __amigaos4__

	char KeyBuffer[5];
	int Index, NumChars, X, Y;
	TBool DoubleClicked, KeyDown, KeyHandled;
	ULONG Result, Signal, SecondSeconds, SecondMicros;
	UWORD Code;
	struct InputEvent *InputEvent;
	struct MenuItem *MenuItem;
	CWindow *Window;
	TStdMouseEvent MouseEvent;

	ASSERTM(m_poWindows, "RApplication::Main() => Application must have at least one window");

	do
	{
		Signal = IExec->Wait(m_ulWindowSignals);

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
						/* If window focus is changing then notify the client window */

						// TODO: CAW - Should really use an Active() function
						Window->OfferKeyEvent(STD_KEY_CONTROL, EFalse);

						break;
					}

					case WMHI_MENUPICK :
					{
						/* Scan through the messages, processing each one.  There may be multiple */
						/* messages waiting due to the user selecting more than one menu item */

						while (Code != MENUNULL)
						{
							/* Get the address of the underlying Intuition MenuItem and from that */
							/* we can extract the command ID of the menu item from the user data */
							/* field and pass it to CWindow::HandleCommand() */

							if ((MenuItem = IIntuition->ItemAddress(m_poMenus, Code)) != NULL)
							{
								Window->HandleCommand((TInt) GTMENUITEM_USERDATA(MenuItem));
							}

							/* And get the code of the next menu item selected */

							Code = IIntuition->ItemAddress(m_poMenus, Code)->NextSelect;
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

						break;
					}

					case WMHI_MOUSEMOVE :
					{
						Window->HandlePointerEvent((Window->m_poWindow->MouseX - Window->m_poWindow->BorderLeft),
							(Window->m_poWindow->MouseY - Window->m_poWindow->BorderTop), EStdMouseMove);

						break;
					}

					case WMHI_RAWKEY :
					{
						/* Mixing IDCMP_RAWKEY and IDCMP_VANILLAKEY doesn't really work on Amiga OS so */
						/* we will handle cooking the keys ourselves.  Assume to start with that the */
						/* cooked key will not be handled */

						KeyHandled = EFalse;

						/* Get a ptr to the InputEvent from Reaction */

						if (IIntuition->GetAttr(WINDOW_InputEvent, Window->m_poWindowObj, (ULONG *) &InputEvent) > 0)
						{
							KeyDown = (!(Code & IECODE_UP_PREFIX));
							Code = (Code & ~IECODE_UP_PREFIX);

							/* Scan through the key mappings and find the one that has just been pressed */

							for (Index = 0; Index < NUM_KEYMAPPINGS; ++Index)
							{
								if (g_aoKeyMap[Index].m_iNativeKey == Code)
								{
									break;
								}
							}

							/* If it was a known key then convert it to the standard value and pass it to the */
							/* CWindow::OfferKeyEvent() function */

							if (Index < NUM_KEYMAPPINGS)
							{
								Window->OfferKeyEvent(g_aoKeyMap[Index].m_iStdKey, KeyDown);

								/* Link on Windows, when ctrl is pressed the ASCII characters sent to */
								/* WMHI_RAWKEY messages are different so we need to adjust these back */
								/* to standard ASCII so keeping track of the state of the ctrl key is */
								/* the only way to achieve this */

								if (g_aoKeyMap[Index].m_iStdKey == STD_KEY_CONTROL)
								{
									m_bCtrlPressed = (KeyDown) ? ETrue : EFalse;
								}
							}

							/* The raw key was not handled so if this is a key down event then cook the key */
							/* and if the resulting key is a printable ASCII key then send it to the client */
							/* code and we're done */

							else
							{
								KeyDown = (!(Code & IECODE_UP_PREFIX));

								if (KeyDown)
								{
									if ((NumChars = IKeymap->MapRawKey(InputEvent, KeyBuffer, sizeof(KeyBuffer), NULL)) > 0)
									{
										/* If the ctrl key is currently pressed then convert the keycode back to standard ASCII */

										if (m_bCtrlPressed)
										{
											KeyBuffer[0] |= 0x60;
										}

										/* Call the CWindow::OfferKeyEvent() function, passing in only valid ASCII characters */

										if ((KeyBuffer[0] >= 32) && (KeyBuffer[0] <= 254))
										{
											Window->OfferKeyEvent(KeyBuffer[0], ETrue);
											KeyHandled = ETrue;
										}
									}
								}
							}
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
				}
			}
		}
	}
	while (!(m_bDone));

#else /* ! __amigaos4__ */

	MSG Msg;

	/* Try to load the default accelerator table.  If this is not found then this is not an error; */
	/* just continue without accelerators */

	// TODO: CAW - Hard coded.  What about elsewhere?
	m_poAccelerators = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(103));

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

			if (!(TranslateAccelerator(m_poWindows->m_poWindow, m_poAccelerators, &Msg)))
			{
				/* Otherwise just perform the standard translation and despatch processing */

				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
	}

	/* Destroy the accelerators if they have been loaded */

	if (m_poAccelerators)
	{
		DestroyAcceleratorTable(m_poAccelerators);
	}

#endif /* ! __amigaos4__ */

	return(KErrNone);
}

/* Written: Tuesday 29-Jun-2010 7:59 pm, London Hackspace */

void RApplication::Close()
{

#ifdef __amigaos4__

	/* If the menus have been created then destroy them */

	if (m_poMenus)
	{
		/* Remove the menus from the main window, if they were added */

		if ((m_bMenuStripSet) && (m_poWindows))
		{
			IIntuition->ClearMenuStrip(m_poWindows->m_poWindow);
		}

		IGadTools->FreeMenus(m_poMenus);
	}

#else /* ! __amigaos4__ */

	// TODO: CAW - Do the same thing for Win32 here

#endif /* ! __amigaos4__ */

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

	/* Add the global application menus to the window, if they exist */

	if (m_poMenus)
	{
		if (IIntuition->SetMenuStrip(a_poWindow->m_poWindow, m_poMenus))
		{
			m_bMenuStripSet = ETrue;
		}
	}

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

#ifdef WIN32

void RApplication::SetCurrentDialog(HWND a_poDialog)
{
	m_poCurrentDialog = a_poDialog;
}

#endif /* WIN32 */

/* Written: Saturday 26-Jun-2010 2:18 pm */

void RApplication::Exit()
{

#ifdef __amigaos4__

	m_bDone = ETrue;

#else /* ! __amigaos4__ */

	PostQuitMessage(0);

#endif /* ! __amigaos4__ */

}
