
#ifndef AMIMENUS_H
#define AMIMENUS_H

class CAmiMenus
{
private:

	TBool					m_bMenuStripSet;	/**< ETrue if menus have been added to the main window */
	TInt					m_iNumMenus;		/**< Number of entries in m_poMenuMappings */
	struct Menu				*m_poMenus;			/**< Ptr to Intuition menus managed by the class */
	struct NewMenu			*m_poNewMenus;		/**< Array of NewMenu structures used for creating menus */
	struct SStdMenuMapping	*m_poMenuMappings;	/**< Array of menu ID -> FULLMENUNUM mappings */
	struct Window			*m_poWindow;		/**< Ptr to underlying Intuition window */

private:

	TInt AddItem(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poDropdownMenu);

	void CreateCommKey(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poMenu);

	void CreateLabel(const struct SStdMenuItem *a_pcoMenuItem, struct NewMenu *a_poMenu);

	struct Menu *CreateIntuitionMenus(struct NewMenu *a_poNewMenus);

	struct NewMenu *FindItem(TInt a_iItemID);

	ULONG FindMapping(TInt a_iItemID);

	struct NewMenu *FindMenu(TInt a_iOrdinalID);

	void FreeLabel(struct NewMenu *a_poNewMenu);

public:

	CAmiMenus(struct Window *a_poWindow)
	{
		m_poWindow = a_poWindow;
	}

	TInt Create(const struct SStdMenuItem *a_pcoMenuItems);

	~CAmiMenus();

	void Close();

	TInt AddItem(const char *a_pccLabel, const char *a_pccHotKey, TInt a_iCommand, TInt a_iOrdinal);

	void CheckItem(TInt a_iItemID, TBool a_bEnable);

	void EnableItem(TInt a_iItemID, TBool a_bEnable);

	TBool ItemChecked(TInt a_iItemID);

	struct Menu *Menus()
	{
		return(m_poMenus);
	}

	void RemoveItem(TInt a_iCommand);

	void UpdateItem(const char *a_pccLabel, const char *a_pccHotKey, TInt a_iCommand);
};

#endif /* ! AMIMENUS_H */
