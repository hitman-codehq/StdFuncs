
#include "../StdFuncs.h"
#include "../StdApplication.h"
#include "../StdWindow.h"
#include <proto/gadtools.h>
#include <proto/intuition.h>
#include <ctype.h>
#include <string.h>
#include "AmiMenus.h"

static const char *g_pccEmptyString = "";	/* Empty string used for creating menu items */

/**
 * Creates an instance of the CAmiMenus class.
 * Standard Symbian style factory function that will create and initialise an instance of the
 * class.
 *
 * @date	Wednesday 30-Oct-2013 12:19 pm, Henry's Kaffe Welt (Hirschstraße)
 * @param	a_poWindow		Ptr to the parent window to which the menus will belong
 * @param	a_pcoMenuItems	Array of SStdMenuItem structures containing the dropdown menus and
 *							menu items to be created
 * @return	Ptr to the newly created class object if successful, else NULL
 */

CAmiMenus *CAmiMenus::New(CWindow *a_poWindow, const struct SStdMenuItem *a_pcoMenuItems)
{
	CAmiMenus *RetVal;

	/* Create and initialise an instance of the class */

	if ((RetVal = new CAmiMenus(a_poWindow, a_pcoMenuItems)) != NULL)
	{
		if (RetVal->Construct() != KErrNone)
		{
			delete RetVal;
			RetVal = NULL;
		}
	}

	return(RetVal);
}

/**
 * Creates a set of menus and attaches them to the underlying Intuition window.
 * This is the main function of the class.  It will take an array of SStdMenuItem structures and
 * will use them to create both an array of NewItem structures and a set of Intuition specific
 * menus.  The NewMenu structures can be used later on for updating the menus (ie. Adding or
 * removing items etc.) and the Intuition specific menus will be attached to the window that was
 * passed in when the class was created.
 *
 * @date	Sunday 20-Oct-2013 9:20 am, on board RB 19320 to Stuttgart
 * @return	KErrNone if the menus were created successfully
 * @return	KErrNoMemory if not enough memory was available to allocate the menus
 */

TInt CAmiMenus::Construct()
{
	TInt Index, Menu, Item, NumMenuItems, RetVal;
	const struct SStdMenuItem *MenuItem;
	struct NewMenu *NewMenus;

	/* Assume failure */

	RetVal = KErrNoMemory;

	/* Iterate through the menu item structures passed in and count how many need to be created */

	MenuItem = m_pcoMenuItems;
	NumMenuItems = 1;

	do
	{
		++NumMenuItems;
		++MenuItem;
	}
	while (MenuItem->m_eType != EStdMenuEnd);

	/* Get a ptr to the first menu structure again, now that we have already parsed the menu structures */

	MenuItem = m_pcoMenuItems;

	/* Allocate a buffer large enough to hold all of the GadTools NewMenu structures, and */
	/* another large enough to hold all of the menu ID mappings, and populate them with */
	/* data taken from the generic SStdMenuItem structures passed in */

	if ((m_poMenuMappings = new SStdMenuMapping[NumMenuItems]) != NULL)
	{
		m_iNumMenus = NumMenuItems;

		if ((m_poNewMenus = NewMenus = new NewMenu[NumMenuItems]) != NULL)
		{
			Menu = -1;
			Item = 0;

			for (Index = 0; Index < NumMenuItems; ++Index)
			{
				/* Every time we find a menu title, increase the menu number and reset the */
				/* item for that menu to zero.  This way we build up the mappings for */
				/* each menu ID -> FULLMENUNUM required by Intuition for accessing menus */

				if (MenuItem[Index].m_eType == EStdMenuTitle)
				{
					++Menu;
					Item = -1;
				}
				else
				{
					++Item;
				}

				/* Now populate the SStdMenuMapping structure */

				m_poMenuMappings[Index].m_iID = MenuItem[Index].m_iCommand;
				m_poMenuMappings[Index].m_ulFullMenuNum = FULLMENUNUM(Menu, Item, 0);

				/* Checkable menus are handled slightly differently */

				if (MenuItem[Index].m_eType == EStdMenuCheck)
				{
					NewMenus[Index].nm_Type = EStdMenuItem;
					NewMenus[Index].nm_Flags = (CHECKIT | MENUTOGGLE);
				}
				else
				{
					NewMenus[Index].nm_Type = MenuItem[Index].m_eType;
					NewMenus[Index].nm_Flags = 0;
				}

				/* Create labels and hotkey strings that match the values passed in */

				CreateLabel(&MenuItem[Index], &NewMenus[Index]);
				CreateCommKey(&MenuItem[Index], &NewMenus[Index]);

				/* And store the command ID associated with the menu item */

				NewMenus[Index].nm_UserData = (APTR) MenuItem[Index].m_iCommand;
			}

			/* The NewMenu structures have been initialised so create the Intuition specific menus */
			/* and assign them to the Intuition window that was passed into the class's constructor */

			if ((m_poMenus = CreateIntuitionMenus(NewMenus)) != NULL)
			{
				if (IIntuition->SetMenuStrip(m_poWindow->m_poWindow, m_poMenus))
				{
					RetVal = KErrNone;
					m_bMenuStripSet = ETrue;
				}
			}
		}
	}

	return(RetVal);
}

/**
 * Destructor for the class.
 * The destructor will remove the Intuition specific menus from the window and will free all
 * structures associated with them.  This inludes the Intuition menus themselves, the class's
 * internal menu array and all strings associated with the labels and hotkey shortcuts.
 *
 * @date	Sunday 20-Oct-2013 9:20 am, on board RB 19320 to Stuttgart
 */

CAmiMenus::~CAmiMenus()
{
	TInt Index;

	/* If the menus have been created then destroy them */

	if (m_poMenus)
	{
		/* Remove the menus from the window, if they were added */

		if (m_bMenuStripSet)
		{
			IIntuition->ClearMenuStrip(m_poWindow->m_poWindow);
		}

		IGadTools->FreeMenus(m_poMenus);
	}

	/* The label strings used by the menus are no longer required so destroy them */

	if (m_poNewMenus)
	{
		/* First delete the temporary menu labels previously allocated */

		for (Index = 0; Index < m_iNumMenus; ++Index)
		{
			FreeLabel(&m_poNewMenus[Index]);
		}

		/* And now delete the NewMenu structures themselves */

		delete [] m_poNewMenus;
		m_poNewMenus = NULL;
	}

	/* Free the menu mapping structures */

	delete [] m_poMenuMappings;
	m_poMenuMappings = NULL;
	m_iNumMenus = 0;
}

/**
 * Adds a menu item to an already existing dropdown menu.
 * This is an internal function.  Client code should use the public version of CAmiMenus::AddItem(),
 * which is a wrapper around this version.  This version accepts a structure containing information about
 * the menu item to add and an Amiga specific ptr to the dropdown menu to which to add the new item.
 *
 * @date	Sunday 20-Oct-2013 10:05 am, on board RB 19320 to Stuttgart
 * @param	a_pcoMenuItem		Ptr to structure containing information such as the label, item ID etc.
 * @param	a_poDropdownMenu	Amiga specific ptr to the dropdown menu to which to add the menu item
 */

void CAmiMenus::AddItem(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poDropdownMenu)
{
	/* Checkable menus are handled slightly differently */

	if (a_pcoMenuItem->m_eType == EStdMenuCheck)
	{
		a_poDropdownMenu->nm_Type = EStdMenuItem;
		a_poDropdownMenu->nm_Flags = (CHECKIT | MENUTOGGLE);
	}
	else
	{
		a_poDropdownMenu->nm_Type = a_pcoMenuItem->m_eType;
		a_poDropdownMenu->nm_Flags = 0;
	}

	/* Create labels and hotkey strings that match the values passed in */

	CreateLabel(a_pcoMenuItem, a_poDropdownMenu);
	CreateCommKey(a_pcoMenuItem, a_poDropdownMenu);

	/* And store the command ID associated with the menu item */

	a_poDropdownMenu->nm_UserData = (APTR) a_pcoMenuItem->m_iCommand;
}

/**
 * Adds a menu item to an already existing dropdown menu.
 * This function appends a new menu item to a dropdown menu.  The label passed in can contain
 * a character preceded by a '&' character, in which case that character will be stripped from
 * the label.  This is to facilitate cross platform portability, as many platforms use this
 * character to denote the shortcut for the menu item.  If no label is passed in then the item
 * will be added as a separator.
 *
 * The dropdown menu to which to add the menu item is specified as an ordinal starting from
 * zero, where zero is the leftmost dropdown menu and (NumMenus - 1) is the rightmost.
 *
 * @date	Sunday 20-Oct-2013 10:04 am, on board RB 19320 to Stuttgart
 * @param	a_pccLabel	Label to be used for the new menu item, or NULL for a separator
 * @param	a_pccHotKey	Shortcut key to be displayed, or NULL for no shortcut.  Ignored for separators
 * @param	a_iOrdinal	Ordinal offset of the dropdown menu to which to add the menu item
 * @param	a_iItemID	Item ID that will be passed to the window's HandleCommand() function
 * @return	KErrNone if successful
 * @return	KErrNotFound if the dropdown menu represented by a_iOrdinal was not found
 * @return	KErrNoMemory if not enough memory was available to allocate the menu item
 */

// TODO: CAW - Change names of variables and parameters and remove some unneeded preconditions
TInt CAmiMenus::AddItem(const char *a_pccLabel, const char *a_pccHotKey, TInt a_iOrdinal, TInt a_iItemID)
{
	TInt Item, NumMenus, RetVal, Size;
	struct Menu *Menus;
	struct NewMenu *DropdownMenu, *NewMenus, *NewMenuItem, *Menu, *Next;
	struct SStdMenuMapping *NewMenuMappings, *Mapping;

	struct SStdMenuItem MenuItem = { EStdMenuItem, a_pccLabel, a_pccHotKey, STD_KEY_MENU, a_iItemID };

	/* Assume failure */

	RetVal = KErrNotFound;

	/* Get a ptr to the dropdown menu to which to add the new menu item */

	if ((DropdownMenu = FindMenu(a_iOrdinal)) != NULL)
	{
		/* If there is no label then this is a separator */

		if (!(a_pccLabel))
		{
			MenuItem.m_eType = EStdMenuSeparator;
		}

		/* Try to get a ptr to the dropdown menu after the one to which we will add the new menu */
		/* item.  This will only be found (and used) if we are not adding an item to the last */
		/* dropdown menu in the list */

		Next = FindMenu(a_iOrdinal + 1);

		/* Allocate new arrays for the menu mappings and NewMenu structures */

		RetVal = KErrNoMemory;
		NewMenuItem = NULL;

		if ((NewMenuMappings = new SStdMenuMapping[m_iNumMenus + 1]) != NULL)
		{
			if ((NewMenus = new NewMenu[m_iNumMenus + 1]) != NULL)
			{
				/* If the target dropdown menu is not the last one then we must copy all menu items */
				/* between the first one and the dropdown menu coming after the one to which we are */
				/* appending.  Also calculate the number of the new item, with 0 meaning the ID of */
				/* the first item already existing in the target dropdown menu */

				if (Next)
				{
					NumMenus = (Next - m_poNewMenus);
					Item = ((Next - DropdownMenu) - 1);
				}

				/* Otherwise copy all menus up to the "NULL terminator" menu.  We will insert the */
				/* new menu item just before this.  Again calculate the number of the new item, */
				/* starting with 0 */

				else
				{
					NumMenus = (m_iNumMenus - 1);
					Item = ((NumMenus - (DropdownMenu - m_poNewMenus)) - 1);
					Next = &m_poNewMenus[NumMenus];
				}

				/* Copy the NewMenu structures and menu mappings up to the point at which the new */
				/* menu item is to be inserted */

				Size = (NumMenus * sizeof(struct NewMenu));
				memcpy(NewMenus, m_poNewMenus, Size);

				memcpy(NewMenuMappings, m_poMenuMappings, (NumMenus * sizeof(struct SStdMenuMapping)));

				/* Populate the NewMenu structure for the menu item we are inserting */

				Menu = NewMenuItem = &NewMenus[NumMenus];
				AddItem(&MenuItem, Menu);

				/* Now populate the new SStdMenuMapping structure */

				Mapping = &NewMenuMappings[NumMenus];
				Mapping->m_iID = MenuItem.m_iCommand;
				Mapping->m_ulFullMenuNum = FULLMENUNUM(a_iOrdinal, Item, 0);

				/* Copy whatever NewMenu items come after the newly inserted one, up to and including the */
				/* "NULL terminator" NewMenu item.  This means that we will copy at least one NewMenu structure */

				++Menu;
				Size = ((m_iNumMenus - NumMenus) * sizeof(struct NewMenu));
				memcpy(Menu, Next, Size);

				/* And copy the remaining menu mappings */

				++Mapping;
				memcpy(Mapping, &m_poMenuMappings[NumMenus], ((m_iNumMenus - NumMenus) * sizeof(struct SStdMenuMapping)));

				/* The NewMenu structures have been initialised so create the Intuition specific menus */
				/* and assign them to the Intuition window that was passed into the class's constructor */

				if ((Menus = CreateIntuitionMenus(NewMenus)) != NULL)
				{
					IIntuition->ClearMenuStrip(m_poWindow->m_poWindow);

					if (IIntuition->SetMenuStrip(m_poWindow->m_poWindow, Menus))
					{
						RetVal = KErrNone;
						m_bMenuStripSet = ETrue;

						/* Success!  Save the new menu related metadata and free the old */

						++m_iNumMenus;

						delete [] m_poMenuMappings;
						m_poMenuMappings = NewMenuMappings;

						delete [] m_poNewMenus;
						m_poNewMenus = NewMenus;

						IGadTools->FreeMenus(m_poMenus);
						m_poMenus = Menus;
					}
				}
			}

			/* If anything failed then free whatever menu related metadata was allocated */

			if (RetVal != KErrNone)
			{
				if (NewMenuItem)
				{
					FreeLabel(NewMenuItem);
				}

				delete [] NewMenus;
				delete [] NewMenuMappings;
			}
		}
	}

	return(RetVal);
}

/**
 * Sets or clears the checkmark on a checkable menu item.
 * Finds a menu item that has an item ID matching that passed in and sets or clears
 * its checkmark.  For a menu item to be checked it must have been created with the
 * type EStdMenuCheck.  It is safe to call this function on a menu item that is not
 * of this type;  in thise case it will simply do nothing.
 *
 * @date	Sunday 20-Oct-2013 6:37 am, Code HQ Ehinger Tor
 * @param	a_iItemID	ID of the menu item to be checked or unchecked
 * @param	a_bEnable	ETrue to check the menu item, else EFalse to uncheck it
 */

void CAmiMenus::CheckItem(TInt a_iItemID, TBool a_bEnable)
{
	ULONG FullMenuNum;
	struct MenuItem *MenuItem;
	struct NewMenu *NewMenu;

	/* Map the menu item's ID onto a value the can be used by Intuition's menu system */

	if ((FullMenuNum = FindMapping(a_iItemID)) != 0)
	{
		/* Get a ptr to the menu item for which to set or clear the checkmark */

		if ((NewMenu = FindItem(a_iItemID)) != NULL)
		{
			/* Now use the result to find the actual menu in the menu strip */

			if ((MenuItem = IIntuition->ItemAddress(m_poMenus, FullMenuNum)) != NULL)
			{
				/* Enable or disable the menu item's check mark as appropriate */

				IIntuition->ClearMenuStrip(m_poWindow->m_poWindow);

				if (a_bEnable)
				{
					NewMenu->nm_Flags |= CHECKED;
					MenuItem->Flags |= CHECKED;
				}
				else
				{
					NewMenu->nm_Flags &= ~CHECKED;
					MenuItem->Flags &= ~CHECKED;
				}

				IIntuition->ResetMenuStrip(m_poWindow->m_poWindow, m_poMenus);
			}
			else
			{
				Utils::Info("CWindow::CheckMenuItem() => Intuition menu item not found");
			}
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
}

/**
 * Creates a shortcut key label for display in a menu item.
 * This is an internal helper function that takes a string representing the shortcut key of a
 * a menu item, allocates memory for that string, copies the string and stores a ptr to it in
 * the target NewMenu structure.  This allows the user to pass in shortcut key strings to the
 * class that are not in persistent memory.
 *
 * @date  Friday 25-Oct-2013 6:57 am, Code HQ Ehinger Tor
 * @param	a_pcoMenuItem	Ptr to the SStdMenuItem structure containing the new shortcut string
 * @param	a_poNewMenu		Ptr to the NewMenu structure into which to place the shortcut string ptr
 */

void CAmiMenus::CreateCommKey(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poNewMenu)
{
	char *Label;
	TInt Length;

	/* Amiga OS is a little limited in terms of the modifiers and strings that can be */
	/* used for the menu items' shortcut keys, so only use the hotkey if the STD_KEY_MENU */
	/* or STD_KEY_ALT (which both map to ramiga) modifier is used, and the length of the */
	/* shortcut string is only one character long */

	a_poNewMenu->nm_CommKey = NULL;

	if ((a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_MENU) || (a_pcoMenuItem->m_iHotKeyModifier == STD_KEY_ALT))
	{
		if ((a_pcoMenuItem->m_pccHotKey) && (strlen(a_pcoMenuItem->m_pccHotKey) == 1))
		{
			Length = strlen(a_pcoMenuItem->m_pccHotKey);

			if ((Label = new char[Length + 1]) != NULL)
			{
				strcpy(Label, a_pcoMenuItem->m_pccHotKey);
				a_poNewMenu->nm_CommKey = Label;
			}
		}
	}
}

/**
 * Creates a label for display in a dropdown menu or a menu item.
 * This is an internal helper function that takes a string representing the label of a dropdown
 * menu or a menu item, allocates memory for that label, copies the string and stores a ptr to
 * it in the target NewMenu structure.  This allows the user to pass in label strings to the class
 * that are not in persistent memory.  It also strips out Windows style '&' characters from the
 * label passed in, to assist with cross platform compatible client source for The Framework.
 *
 * @date	Friday 25-Oct-2013 7:22 am, Code HQ Ehinger Tor
 * @param	a_pcoMenuItem	Ptr to the SStdMenuItem structure containing the new label
 * @param	a_poNewMenu		Ptr to the NewMenu structure into which to place the label ptr
 */

void CAmiMenus::CreateLabel(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poNewMenu)
{
	char *NewLabel;
	const char *Label;
	TInt DestIndex, SourceIndex, Length;

	/* Separators get a special label that causes Intuition to treat them as such so check */
	/* for this first */

	if (a_pcoMenuItem->m_eType == EStdMenuSeparator)
	{
		a_poNewMenu->nm_Type = EStdMenuItem;
		a_poNewMenu->nm_Label = NM_BARLABEL;
	}

	/* Otherwise if there is a label for the menu item present then it will have the '&' shortcut */
	/* key modifier embedded in it for Windows & Qt.  We need to make a copy of the label and */
	/* remove this modifier, which doesn't have any effect on Amiga OS */

	else if ((Label = a_pcoMenuItem->m_pccLabel) != NULL)
	{
		/* Find out the length of the label, including the NULL terminator, and allocate */
		/* a buffer for it */

		Length = (strlen(Label) + 1);
		a_poNewMenu->nm_Label = NewLabel = new char[Length];

		if (NewLabel)
		{
			/* Copy the label into the newly allocated buffer, removing the '&' as we go */

			DestIndex = 0;

			for (SourceIndex = 0; SourceIndex < Length; ++SourceIndex)
			{
				if (Label[SourceIndex] != '&')
				{
					NewLabel[DestIndex++] = Label[SourceIndex];
				}
			}
		}

		/* If we cannot allocate a new label then we need to use *something* and cannot just */
		/* use a NULL string.  So use a globally allocated empty string and the destructor will */
		/* skip freeing this */

		else
		{
			a_poNewMenu->nm_Label = g_pccEmptyString;
		}
	}
}

/**
 * Create Amiga OS specific Intuition menus for attaching to a Window.
 * This is an internal helper function that will create a set of Intuition specific menu structures
 * based on the content of an array of NewMenu structures.  All of the usual rules that would usually
 * apply to creating menus using NewMenu structures apply to this function, as this function is only
 * a wrapper around the appropriate Intuition functions.
 *
 * @date	Thursday 24-Oct-2013 7:32 am, Code HQ Ehinger Tor
 * @param	a_poNewMenus	Ptr to the array of NewMenu structures
 * @return	A ptr to an array of Menu structures if successful, else NULL
 */

struct Menu *CAmiMenus::CreateIntuitionMenus(struct NewMenu *a_poNewMenus)
{
	APTR VisualInfo;
	struct Menu *RetVal;
	struct Screen *Screen;

	/* Assume failure */

	RetVal = NULL;

	/* Lock the default public screen and obtain a VisualInfo structure, in preparation for laying */
	/* the menus out */

	if ((Screen = IIntuition->LockPubScreen(NULL)) != NULL)
	{
		if ((VisualInfo = IGadTools->GetVisualInfo(Screen, TAG_DONE)) != NULL)
		{
			/* Create the menus and lay them out in preparation for display */

			if ((RetVal = IGadTools->CreateMenus(a_poNewMenus, GTMN_FrontPen, 1, TAG_DONE)) != NULL)
			{
				if (!(IGadTools->LayoutMenus(RetVal, VisualInfo, GTMN_NewLookMenus, 1, TAG_DONE)))
				{
					/* Layout failed so free the menu structure and return failure */

					IGadTools->FreeMenus(RetVal);
					RetVal = NULL;
				}
			}
		}

		/* And unlock the default public screen */

		IIntuition->UnlockPubScreen(NULL, Screen);
	}

	return(RetVal);
}

/**
 * Enables or disables a menu item.
 * Searches for the menu item identified by a_iItemID and enables or disables it.
 *
 * @date	Wednesday 23-Oct-2013 6:44 am, Code HQ Ehinger Tor
 * @param	a_iItemID	ID of the menu item to be enabled or disabled
 * @param	a_bEnable	ETrue to enable the menu item, else EFalse to disable it
 */

void CAmiMenus::EnableItem(TInt a_iItemID, TBool a_bEnable)
{
	ULONG FullMenuNum;

	/* Map the menu item's ID onto a value the can be used by Intuition's menu system */

	if ((FullMenuNum = FindMapping(a_iItemID)) != 0)
	{
		/* Enable or disable the menu item as appropriate */

		if (a_bEnable)
		{
			IIntuition->OnMenu(m_poWindow->m_poWindow, FullMenuNum);
		}
		else
		{
			IIntuition->OffMenu(m_poWindow->m_poWindow, FullMenuNum);
		}
	}
	else
	{
		Utils::Info("CWindow::EnableItem() => Menu mapping not found");
	}
}

/**
 * Simulates the selection of a menu item.
 * This function will search through the class's menu array for a given key value and, if found,
 * will send a command to the associated CWindow that was passed in when the CAmiMenus instance was
 * created.  This is to support key combinations that may be used on Windows and Qt but which
 * are not supported by default on Amiga OS.  For instance, <f3> or ctrl+<key> combinations.  This
 * enables client code to add these keyboard shortcuts to menu items and, even though they will
 * not be displayed in the Intuition specific menu, they will still generate commands as they would
 * on the other platforms.
 *
 * If used in response to keyboard events, this function should only be called on a key down (or
 * key up) event, to prevent the commands being sent to the underlying window multiple times.
 *
 * @date    Thursday 31-Oct-2013 7:06 am, Code HQ Ehinger Tor
 * @param	a_iKey			Value of the keyboard key pressed
 * @param	a_bCtrlPressed	ETrue if the control key is currently pressed, else EFalse
 * @return	ETrue if a shortcut was executed, else EFalse
 */

TBool CAmiMenus::ExecuteShortcut(TInt a_iKey, TBool a_bCtrlPressed)
{
	TBool RetVal;
	TInt FunctionKey, Command;
	const struct SStdMenuItem *MenuItem;

	/* Assume no shortcut is executed */

	RetVal = EFalse;

	/* Iterate through the menu items and see if any match the key combination passed in */

	Command	= -1;
	MenuItem = m_pcoMenuItems;

	do
	{
		/* Only do anything for menu items that have an associated shotcut key */

		if ((MenuItem->m_eType == EStdMenuItem) && (MenuItem->m_pccHotKey))
		{
			/* If the shortcut key is long then it is a function key string so it must be converted */
			/* to a value the can be compared against the Amiga's function keys */

			if (strlen(MenuItem->m_pccHotKey) >= 2)
			{
				/* Is it a function key with a numeric value following it? */

				if ((toupper(MenuItem->m_pccHotKey[0]) == 'F') && (Utils::StringToInt(&MenuItem->m_pccHotKey[1], &FunctionKey) == KErrNone))
				{
					/* Does the numeric value represent a valid function key? */

					if ((FunctionKey >= 1) && (FunctionKey <= 12))
					{
						/* Yes.  Convert the key to a virtual keycode */

						FunctionKey += (STD_KEY_F1 - 1);

						/* Do we have a match? */

						if (FunctionKey == a_iKey)
						{
							Command = MenuItem->m_iCommand;
						}
					}
				}
			}

			/* Otherwise it is a "normal" key so check to see if it matches the key passed in */

			else if (*MenuItem->m_pccHotKey == a_iKey)
			{
				Command	= MenuItem->m_iCommand;
			}

			/* If a matching key was found then also check the shortcut's modifier (if it has one) */
			/* to see if that was also pressed.  If so then call CWindow::HandleCommand() to */
			/* simulate a menu item selection */

			if (Command != -1)
			{
				if ((a_bCtrlPressed) && (MenuItem->m_iHotKeyModifier == STD_KEY_CONTROL))
				{
					RetVal = ETrue;
					m_poWindow->HandleCommand(MenuItem->m_iCommand);

					break;
				}
				else if ((!(a_bCtrlPressed)) && (MenuItem->m_iHotKeyModifier == 0))
				{
					RetVal = ETrue;
					m_poWindow->HandleCommand(MenuItem->m_iCommand);

					break;
				}

				/* The modifier did not match that of the shortcut so keep searching */

				else
				{
					Command = -1;
				}
			}
		}

		++MenuItem;
	}
	while (MenuItem->m_eType != EStdMenuEnd);

	return(RetVal);
}

/**
 * Searches through the array of menus for a particular menu item.
 * This function searches through the array of menus, searching for a menu item
 * of a particular ID.  It is a helper function to allow accessing menus in the
 * Windows manner of addressing them via ID, rather than the Amiga OS style of using
 * the menu item's position.  The menu item for which to search can be either a
 * dropdown menu or an item within a dropdown menu.
 *
 * @date	Sunday 13-Oct-2013 8:32 am, Code HQ Ehinger Tor
 * @param	a_iItemID	 ID of menu item for which to search
 * @return	Ptr to the NewMenu structure of the menu matching the ID passed in, else NULL
 *			if the ID was not found
 */

struct NewMenu *CAmiMenus::FindItem(TInt a_iItemID)
{
	TInt Index;
	struct NewMenu *RetVal;
	struct SStdMenuMapping *MenuMapping;

	/* Assume failure */

	RetVal = NULL;

	/* Iterate through the list of menu mappings and find the one we are looking for */

	MenuMapping = m_poMenuMappings;

	for (Index = 0; Index < m_iNumMenus; ++Index)
	{
		if (MenuMapping->m_iID == a_iItemID)
		{
			RetVal = &m_poNewMenus[Index];

			break;
		}
		else
		{
			++MenuMapping;
		}
	}

	return(RetVal);
}

/**
 * Searches through the array of menu mappings for a particular menu item's mapping.
 * This function searches through the array of menu mappings, searching for an item of a
 * particular ID.  It is a helper function to allow accessing menu items in the Windows
 * manner of addressing them via ID, rather than the Amiga OS style of using the menu
 * item's position.
 *
 * @date	Saturday 14-Apr-2012 8:02 am, Code HQ Ehinger Tor
 * @param	a_iItemID	Item ID to be searched for
 * @return	The Amiga stype menu mapping of the menu matching the ID passed in, else 0
 *			if the ID was not found
 */

ULONG CAmiMenus::FindMapping(TInt a_iItemID)
{
	TInt Index;
	ULONG RetVal;
	struct SStdMenuMapping *MenuMappings;

	/* Assume failure */

	RetVal = 0;

	/* Iterate through the list of menu mappings and find the one we are looking for */

	MenuMappings = m_poMenuMappings;

	for (Index = 0; Index < m_iNumMenus; ++Index)
	{
		if (MenuMappings->m_iID == a_iItemID)
		{
			RetVal = MenuMappings->m_ulFullMenuNum;

			break;
		}

		++MenuMappings;
	}

	return(RetVal);
}

/**
 * Searches through the array of menus for a particular dropdown menu.
 * This function searches through the array of menus, searching for a dropdown
 * menu of a particular ID.  It is a helper function to allow accessing menus in the
 * Windows manner of addressing them via ID, rather than the Amiga OS style of using
 * the menu item's position.
 *
 * @date	Saturday 12-Oct-2013 1:26 pm, Code HQ Ehinger Tor
 * @param	a_iOrdinalID	ID of dropdown menu for which to search
 * @return	Ptr to the NewMenu structure of the menu matching the ID passed in, else NULL
 *			if the ID was not found
 */

struct NewMenu *CAmiMenus::FindMenu(TInt a_iOrdinalID)
{
	TInt Index, MenuIndex;
	struct NewMenu *NewMenu, *RetVal;

	/* Assume failure */

	RetVal = NULL;

	/* Iterate through the list of dropdown menus and find the one we are looking for */

	MenuIndex = 0;
	NewMenu = m_poNewMenus;

	for (Index = 0; Index < m_iNumMenus; ++Index)
	{
		if (NewMenu->nm_Type == EStdMenuTitle)
		{
			if (MenuIndex == a_iOrdinalID)
			{
				RetVal = NewMenu;

				break;
			}
			else
			{
				++MenuIndex;
			}
		}

		++NewMenu;
	}

	return(RetVal);
}

/**
 * Frees the label and hotkey shortcut strings belonging to a menu item.
 * This is an internal helper function that will free the memory allocated to hold the
 * or menu item's label and shortcut hotkey strings.  These are stored within the menu
 * item's NewMenu structure when the menu item is created and are used by Intuition when
 * the menus are displayed.
 *
 * After the strings are freed, their respective members in the NewMenu structure are
 * set to NULL and must be reallocated before the NewMenu structure can be reused.
 *
 * @date	Saturday 26-Oct-2013 7:13 am, Code HQ Ehinger Tor
 */

void CAmiMenus::FreeLabel(struct NewMenu *a_poNewMenu)
{
	const char *Label;

	Label = a_poNewMenu->nm_Label;

	/* If the label was allocated and is not a predefined separator constant or the */
	/* global empty string then free it */

	if ((Label) && (Label != NM_BARLABEL) && (Label != g_pccEmptyString))
	{
		delete [] Label;
		a_poNewMenu->nm_Label = NULL;
	}

	delete [] a_poNewMenu->nm_CommKey;
	a_poNewMenu->nm_CommKey = NULL;
}

/**
 * Determines whether a menu item is checked.
 * Finds a menu item that has an item ID matching that passed in and returns
 * whether or not it is checked.  For a menu item to be able to be checked it
 * must have been created with the type EStdMenuCheck.  It is safe to call this
 * function on a menu item that is not of this type;  in thise case it will
 * always return EFalse.
 *
 * @date	Wednesday 23-Oct-2013 6:47 am, Code HQ Ehinger Tor
 * @param	a_iItemID	ID of the menu item to be queried
 * @return	ETrue if the item is checked, else EFalse
 */

TBool CAmiMenus::ItemChecked(TInt a_iItemID)
{
	TBool RetVal;
	ULONG FullMenuNum;
	struct MenuItem *MenuItem;

	/* Assume the menu item is not checked */

	RetVal = EFalse;

	/* Map the menu item's ID onto a value the can be used by Intuition's menu system */

	if ((FullMenuNum = FindMapping(a_iItemID)) != 0)
	{
		/* Now use the result to find the actual menu in the menu strip */

		if ((MenuItem = IIntuition->ItemAddress(m_poMenus, FullMenuNum)) != NULL)
		{
			/* And determine whether the menu item is checked */

			if (MenuItem->Flags & CHECKED)
			{
				RetVal = ETrue;
			}
		}
	}

	return(RetVal);
}

/**
 * Removes a single menu item from a dropdown menu.
 * This function will search through the array of menus for an item of the given ID and,
 * when found, will remove it from the array and free the memory used for holding its label
 * and shortcut key.  It will then rebuild the underlying Intuition menu structures, remove
 * the current Intuition menu strip and attach the newly created one, thus updating not only
 * the menus held by this class but their representation on screen as well.
 *
 * The item referenced by the item ID passed in must be a menu item, not a dropdown menu.
 * If it represents a dropdown menu then this function will do nothing.
 *
 * @date	Sunday 20-Oct-2013 9:47 am, on board RB 19320 to Stuttgart
 * @param	a_iItemID	 ID of the menu item to be removed
 */

void CAmiMenus::RemoveItem(TInt a_iItemID)
{
	TInt NumMenus, Offset, Size;
	struct Menu *Menus;
	struct NewMenu *NewMenu;

	/* Get a ptr to the menu item to be removed */

	if ((NewMenu = FindItem(a_iItemID)) != NULL)
	{
		/* Only do anything if this is a menu item.  If the user is trying to delete a dropdown */
		/* menu then simply return */

		if (NewMenu->nm_Type != EStdMenuTitle)
		{
			/* We won't free the NewMenu structure itself as it is a part of an array but we must */
			/* free its associated label and hotkey string so as not to leak memory */

			FreeLabel(NewMenu);

			/* Determine the offset in the array of the menu to be removed and the number of menus */
			/* which come after this menu.  This is the number of menu items that must be moved */
			/* down to fill the gap made by the removed menu item */

			Offset = (NewMenu - m_poNewMenus);
			NumMenus = (m_iNumMenus - Offset - 1);

			/* Move NewMenu structures and the menu mapping structures down to fill the gap */

			Size = (NumMenus * sizeof(struct NewMenu));
			memmove(&m_poNewMenus[Offset], &m_poNewMenus[Offset + 1], Size);

			Size = (NumMenus * sizeof(struct SStdMenuMapping));
			memmove(&m_poMenuMappings[Offset], &m_poMenuMappings[Offset + 1], Size);

			/* We now have one menu item less in the array */

			--m_iNumMenus;

			/* Rebuild the underlying Intuition menu structures and attach them to the menu strip */

			if ((Menus = CreateIntuitionMenus(m_poNewMenus)) != NULL)
			{
				/* Remove the old Intuition menu structures from the menu strip and free them */

				IIntuition->ClearMenuStrip(m_poWindow->m_poWindow);
				m_bMenuStripSet = EFalse;

				IGadTools->FreeMenus(m_poMenus);
				m_poMenus = Menus;

				/* And now attach the new Intuition menu structures to the menu strip */

				if (IIntuition->SetMenuStrip(m_poWindow->m_poWindow, m_poMenus))
				{
					m_bMenuStripSet = ETrue;
				}
			}
		}
	}
}

/**
 * Updates the internal checkmark state of a menu item.
 * This is an internal function that will update the state of a menu item's checkmark in the
 * internal NewMenu structures kept by the CAmiMenus class.  When menu items are added, removed
 * or updated, the underlying Intuition menus are destroyed and recreated, thus losing the state
 * of the checkmarks for any checkable menus.  This function should thus be called by The Framework
 * when the state of a checkmark is changed by Intuition, so that CAmiMenus can keep track of
 * these checkmark state changes.
 *
 * @date	Friday 08-Nov-2013 7:15 am, Code HQ Ehinger Tor
 * @param	a_iItemID	ID of the menu item that was checked or unchecked
 * @param	a_bEnable	ETrue if the menu item was checked, else EFalse if it was cleared
 */

void CAmiMenus::UpdateCheckStatus(TInt a_iItemID, TBool a_bEnabled)
{
	struct NewMenu *NewMenu;

	/* Get a ptr to the menu item for which to update the checkmark status */

	if ((NewMenu = FindItem(a_iItemID)) != NULL)
	{
		/* Update the internal flags in the NewMenu structure so that if the Intuition menu structures */
		/* are recreated then they will have the checkmark correctly set or cleared */

		if (a_bEnabled)
		{
			NewMenu->nm_Flags |= CHECKED;
		}
		else
		{
			NewMenu->nm_Flags &= ~CHECKED;
		}
	}
}

/**
 * Updates a menu or menu item's label and hotkey shortcut.
 * This function will update a menu or menu item's label and hotkey shortcut.  The strings passed
 * in for this purpose do not need to be in persistent memory as this function will allocate
 * buffers for them and will store the buffers in the class's menu array.  It will also take
 * care of removing any Windows style '&' characters in the menu label, as this routine is
 * designed to work with cross platform code.
 *
 * @date	Sunday 20-Oct-2013 9:54 am, on board RB 19320 to Stuttgart
 * @param	a_pccLabel	Label to be used for the new menu item, or NULL for a separator
 * @param	a_pccHotKey	Shortcut key to be displayed, or NULL for no shortcut.  Ignored for separators
 * @param	a_iItemID	Item ID that will be passed to the window's HandleCommand() function
 */

void CAmiMenus::UpdateItem(const char *a_pccLabel, const char *a_pccHotKey, TInt a_iItemID)
{
	struct Menu *Menus;
	struct NewMenu *NewMenu;

	struct SStdMenuItem MenuItem = { EStdMenuItem, a_pccLabel, a_pccHotKey, STD_KEY_MENU, a_iItemID };

	/* Get a ptr to the menu item to be updated */

	if ((NewMenu = FindItem(a_iItemID)) != NULL)
	{
		/* The label and hotkey strings are going to be replaced with new ones, so free the old strings */

		FreeLabel(NewMenu);

		/* Create new ones that match the values passed in */

		CreateLabel(&MenuItem, NewMenu);
		CreateCommKey(&MenuItem, NewMenu);

		/* Rebuild the underlying Intuition menu structures and attach them to the menu strip */

		if ((Menus = CreateIntuitionMenus(m_poNewMenus)) != NULL)
		{
			/* Remove the old Intuition menu structures from the menu strip and free them */

			IIntuition->ClearMenuStrip(m_poWindow->m_poWindow);
			m_bMenuStripSet = EFalse;

			IGadTools->FreeMenus(m_poMenus);
			m_poMenus = Menus;

			/* And now attach the new Intuition menu structures to the menu strip */

			if (IIntuition->SetMenuStrip(m_poWindow->m_poWindow, m_poMenus))
			{
				m_bMenuStripSet = ETrue;
			}
		}
	}
}
