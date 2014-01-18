
#ifndef AMIMENUS_H
#define AMIMENUS_H

/**
 * A class enabling the easy creation and modification of Amiga native menus.
 * This class is an elaborate wrapper around the not-so-easy-to-use Intuition menus as
 * used on Amiga OS.  While the creation of Intuition menus is quite simple, modifying
 * already created menus and - especially - adding or removing menu items dynamically, is
 * quite difficult as Intuition offers no such functionality.  Instead, one must rebuild
 * all of the existing menu items along with the required modifications, which can be quite
 * tedious.  This class simplifies this process by hiding the complexity and exporting
 * very simple functions such as CAmiMenus::AddItem() and CAmiMenus::RemoveItem(), which
 * behave in an analogous manner to similar functions found on other operating systems.
 *
 * To use the class, start by creating an array of SStdMenuItem structures describing the
 * desired menu layout.  Create an instance of the CAmiMenus class and call CAmiMenus::Create(),
 * passing in the array of structures.  The Amiga specific menus will then be created and
 * attached to the window assigned to the class during the creation of the class object.
 * After this, the menus can be modified at any time by calling the appropriate functions,
 * and the modifications will be immediately reflected in the window's Intuition menus.
 */

class CAmiMenus
{
private:

	TBool					m_bMenuStripSet;	/**< ETrue if menus have been added to the main window */
	TInt					m_iNumMenus;		/**< Number of entries in m_poMenuMappings */
	const struct SStdMenuItem *m_pcoMenuItems;	/**< Ptr to a list of structures describing the menus */
	struct Menu				*m_poMenus;			/**< Ptr to Intuition menus managed by the class */
	struct NewMenu			*m_poNewMenus;		/**< Array of NewMenu structures used for creating menus */
	struct SStdMenuMapping	*m_poMenuMappings;	/**< Array of menu ID -> FULLMENUNUM mappings */
	CWindow					*m_poWindow;		/**< Ptr to the parent window to which the menus belong */

private:

	/**
	 * Constructor for the class.
	 * This function exists only to prevent the user from creating the class without using the
	 * CAmiMenus::New() factory function.
	 */

	CAmiMenus() { }

	/**
	 * Constructor for the class.
	 * This is an internal function used only by the CAmiMenus::New() factory function.
	 *
	 * @param	a_poWindow		Ptr to the parent window to which the menus will belong
	 * @param	a_pcoMenuItems  Ptr to a list of structures describing the menus
	 */

	CAmiMenus(CWindow *a_poWindow, const struct SStdMenuItem *a_pcoMenuItems)
	{
		m_poWindow = a_poWindow;
		m_pcoMenuItems = a_pcoMenuItems;
	}

	TInt Construct();

	void AddItem(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poDropdownMenu, TBool a_bSubMenu);

	void CreateCommKey(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poNewMenu);

	void CreateLabel(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poNewMenu);

	struct Menu *CreateIntuitionMenus(struct NewMenu *a_poNewMenus);

	struct NewMenu *FindItem(TInt a_iItemID);

	ULONG FindMapping(TInt a_iItemID);

	struct NewMenu *FindMenu(TInt a_iOrdinalID);

	void FreeLabel(struct NewMenu *a_poNewMenu);

	void UpdateCheckStatus(TInt a_iItemID, TBool a_bEnabled);

public:

	static CAmiMenus *New(CWindow *a_poWindow, const struct SStdMenuItem *a_pcoMenuItems);

	~CAmiMenus();

	TInt AddItem(TStdMenuItemType a_eMenuItemType, const char *a_pccLabel, const char *a_pccHotKey, TInt a_iOrdinal, TInt a_iSubOrdinal, TInt a_iItemID);

	void CheckItem(TInt a_iItemID, TBool a_bEnable);

	void EnableItem(TInt a_iItemID, TBool a_bEnable);

	TBool ExecuteShortcut(TInt a_iKey, TBool a_bCtrlPressed);

	TBool ItemChecked(TInt a_iItemID);

	/**
	 * Returns a ptr to the underlying Intuition menus.
	 * This is an internal function used only by The Framework for managing the internal Intuition
	 * menus under Amiga OS and should not be used by public code.
	 *
	 * @return	A ptr to the underlying Intuition menus, or NULL if they have not yet been created
	 */

	struct Menu *Menus()
	{
		return(m_poMenus);
	}

	void RemoveItem(TInt a_iItemID);

	void UpdateItem(const char *a_pccLabel, const char *a_pccHotKey, TInt a_iItemID);

	friend class RApplication;
};

#endif /* ! AMIMENUS_H */
