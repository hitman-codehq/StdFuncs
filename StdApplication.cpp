
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdWindow.h"

#ifdef __amigaos4__

#include <proto/gadtools.h>
#include <proto/intuition.h>
#include <proto/keymap.h>

/* Array of key mappings for mapping Amiga keys onto standard keys */

static const SKeyMapping g_aoKeyMap[] =
{
	{ STD_KEY_SHIFT, 0x60 }, { STD_KEY_CONTROL, 0x63 }, { STD_KEY_BACKSPACE, 0x41 },
	{ STD_KEY_ENTER, 0x44 }, { STD_KEY_UP, 0x4c }, { STD_KEY_DOWN, 0x4d },
	{ STD_KEY_LEFT, 0x4f }, { STD_KEY_RIGHT, 0x4e }, { STD_KEY_HOME, 0x70 },
	{ STD_KEY_END, 0x71 }, { STD_KEY_PGUP, 0x48 }, { STD_KEY_PGDN, 0x49 },
	{ STD_KEY_DELETE, 0x46 }, { STD_KEY_TAB, 0x42 }
};

#define NUM_KEYMAPPINGS 14

#endif /* __amigaos4__ */

/* Written: Saturday 26-Jun-2010 11:53 am */

RApplication::RApplication()
{

#ifdef __amigaos4__

	m_bDone = m_bMenuStripSet = EFalse;
	m_poMenus = NULL;

#else /* ! __amigaos4__ */

	m_poAccelerators = NULL;

#endif /* ! __amigaos4__ */

	m_pcoMenuItems = NULL;
}

/* Written: Wednesday 30-Jun-2010 6:53 am */
/* @param a_pcoMenuItems Ptr to array of menu items to be created.  Must be persisted for the lifetime */
/*                       of the RApplication class that uses them */

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
	bool KeyHandled;
	int Code, Index, Menu, MenuItem, NumChars;
	ULONG Signal, WindowSignal;
	const struct SStdMenuItem *MenuItems;
	struct InputEvent InputEvent;
	struct IntuiMessage *IntuiMessage;

	ASSERTM(m_poWindow, "RApplication::Main() => Window handle must not be NULL");

	WindowSignal = m_poWindow->GetSignal();

	do
	{
		Signal = IExec->Wait(WindowSignal);

		if (Signal & WindowSignal)
		{
			while ((IntuiMessage = (struct IntuiMessage *) IExec->GetMsg(m_poWindow->GetWindow()->UserPort)) != NULL)
			{
				switch (IntuiMessage->Class)
				{
					case IDCMP_CLOSEWINDOW :
					{
						m_bDone = ETrue;

						break;
					}

					case IDCMP_MENUPICK :
					{
						/* Obtain the code of the first menu item that was selected */

						Code = IntuiMessage->Code;

						/* Scan through the messages, processing each one.  There may be multiple */
						/* messages waiting due to the user selecting more than one */

						while (Code != MENUNULL)
						{
							/* Map the menu item that was selected onto its generic command ID.  The */
							/* idea here it to iterate through the Amiga OS style menus, figuring */
							/* out the menu and menu item number of each one, until we find one that */
							/* matches the menu and menu item number passed in with the message */

							Menu = -1;
							MenuItem = 0;
							MenuItems = m_pcoMenuItems;

							while (MenuItems->m_eType != EStdMenuEnd)
							{
								/* Each time we find a menu title, the menu number increases by 1 and */
								/* the menu item number resets.  The first title is 0, hence starting */
								/* with -1 above */

								if (MenuItems->m_eType == EStdMenuTitle)
								{
									++Menu;
									MenuItem = 0;
								}
								else
								{
									/* If this menu item the one we are looking for then handle the */
									/* command that represents the menu item */

									if ((MENUNUM(Code) == Menu) && (ITEMNUM(Code) == MenuItem))
									{
										m_poWindow->HandleCommand(MenuItems->m_iCommand);

										break;
									}

									/* Otherwise just increment the menu item # and keep searching */

									else
									{
										++MenuItem;
									}
								}

								++MenuItems;
							}

							/* And get the code of the next menu item selected */

							Code = IIntuition->ItemAddress(m_poMenus, Code)->NextSelect;
						}

						break;
					}

					case IDCMP_RAWKEY :
					{
						/* Mising IDCMP_RAWKEY and IDCMP_VANILLAKEY doesn't really work on Amiga OS so */
						/* we will handle cooking the keys ourselves.  Assume to start with that the */
						/* cooked key will not be handled */

						KeyHandled = false;

						/* If this is a key down event then cook the key and if the resulting key is a */
						/* printable ASCII key then send it to the client code and we're done */

						if (!(IntuiMessage->Code & IECODE_UP_PREFIX))
						{
							InputEvent.ie_Class = IECLASS_RAWKEY;
							InputEvent.ie_SubClass = 0;
							InputEvent.ie_Code = IntuiMessage->Code;
							InputEvent.ie_Qualifier = IntuiMessage->Qualifier;
							InputEvent.ie_EventAddress= (APTR *) *((ULONG *) IntuiMessage->IAddress);

							if ((NumChars = IKeymap->MapRawKey(&InputEvent, KeyBuffer, sizeof(KeyBuffer), NULL)) > 0)
							{
								if ((KeyBuffer[0] >= ' ') && (KeyBuffer[0] <= '~'))
								{
									m_poWindow->OfferKeyEvent(KeyBuffer[0], ETrue);
									KeyHandled = true;
								}
							}
						}

						/* If the key wasn't handled then it is probably a non printable ASCII key so */
						/* map it onto the standard keycode and send it to the client code */

						if (!(KeyHandled))
						{
							Code = (IntuiMessage->Code & ~IECODE_UP_PREFIX);

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
								m_poWindow->OfferKeyEvent(g_aoKeyMap[Index].m_iStdKey, (!(IntuiMessage->Code & IECODE_UP_PREFIX)));
							}
						}

						break;
					}

					case IDCMP_REFRESHWINDOW :
					{
						m_poWindow->Draw();

						break;
					}
				}

				/* And reply to the processed message */

				IExec->ReplyMsg((struct Message *) IntuiMessage);
			}
		}
	}
	while (!(m_bDone));

	// TODO: CAW - Move to destructor
	/* If the menus have been created then destroy them */

	if (m_poMenus)
	{
		/* Remove the menus from the main window, if they were added */

		if (m_bMenuStripSet)
		{
			IIntuition->ClearMenuStrip(m_poWindow->m_poWindow);
		}

		IGadTools->FreeMenus(m_poMenus);
	}

#else /* ! __amigaos4__ */

	MSG Msg;

	/* Try to load the default accelerator table.  If this is not found then this is not an error; */
	/* just continue without accelerators */

	m_poAccelerators = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(103));

	/* Standard Windows message loop with accelerator handling */

	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		/* Try to translate the message;  Windows will handle m_poAccelerators being NULL */

		if (TranslateAccelerator(m_poWindow->m_poWindow, m_poAccelerators, &Msg) == 0)
		{
			/* No accelerator found so do the standard message translation and despatch */

			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
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

/* Written: Monday 08-Feb-2010 7:25 am */

void RApplication::AddWindow(CWindow *a_poWindow)
{
	ASSERTM((a_poWindow != NULL), "RApplication::AddWindow() => Window ptr must be passed in");

	m_poWindow = a_poWindow;
	m_poWindow->SetApplication(this);

#ifdef __amigaos4__

	// TODO: CAW - Is this safe to do multiple times?  Return value handling?  Matching RemoveWindow()?
	/* Add the global application menus to the window, if they exist */

	if (m_poMenus)
	{
		if (IIntuition->SetMenuStrip(m_poWindow->m_poWindow, m_poMenus))
		{
			m_bMenuStripSet = ETrue;
		}
	}

#endif /* __amigaos4__ */

	/* Schedule a redraw to ensure the newly added window is refreshed */

	m_poWindow->DrawNow();
}

/* Written: Saturday 26-Jun-2010 2:18 pm */

void RApplication::Exit()
{

#ifdef __amigaos4__

	m_bDone = ETrue;

#else /* ! __amigaos4__ */

	PostQuitMessage(0);

#endif /* ! __amigaos4__ */

}
