
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdWindow.h"

#ifdef __amigaos__

#include <string.h>
#include "OS4Support.h"
#include "StdReaction.h"

#elif defined(QT_GUI_LIB)

#include "Qt/QtGadgetTree.h"
#include "Qt/QtWindow.h"
#include <QtWidgets/QHeaderView>

#endif /* QT_GUI_LIB */

/**
 * Second phase constructor.
 * This is called by the New() wrapper to perform initialisation steps that can fail and need to be cleaned
 * up upon failure.
 *
 * @date	Sunday 18-Jul-2021 12:32 pm, Am Theater Stralsund FeWo
 * @param	a_numColumns	The number of columns to display in the tree
 * @return	KErrNone if successful, else KErrNoMemory
 */

int CStdGadgetTree::construct(int a_numColumns)
{
	int retVal = KErrNone;

	ASSERTM((m_poParentWindow != nullptr), "CStdGadgetTree::construct() => Parent window is NULL");
	ASSERTM((m_poParentWindow->m_poWindow != nullptr), "CStdGadgetTree::construct() => Parent native window is NULL");

	m_numColumns = a_numColumns;

#ifdef __amigaos__

	if (!createNative())
	{
		retVal = KErrNoMemory;
	}

#elif defined(QT_GUI_LIB)

	/* Initialise the Qt specific helper object */
	m_poGadget = &m_tree;
	m_tree.construct(m_numColumns);

	// TODO: CAW - Why aren't these fetched automatically in CStdGadget?
	m_iWidth = m_poGadget->width();
	m_iHeight = m_poGadget->height();

#else /* ! QT_GUI_LIB */

	m_poParentLayout->Attach(this);

#endif /* ! QT_GUI_LIB */

	/* Attach the gadget to the parent window, if it was created successfully */

	if (m_poGadget)
	{
		m_poParentLayout->Attach(this);
	}

	return retVal;
}

/**
 * CStdGadgetTree Destructor.
 * Frees all lists of items in the map of available lists.
 *
 * @date	Wednesday 13-Mar-2024 6:45 am, Code HQ Tokyo Tsukuda
 */

CStdGadgetTree::~CStdGadgetTree()
{

#ifdef __amigaos__

	delete [] m_columnInfo;

	/* Iterate through the map of available lists and delete the nodes from each one */
	for (auto &item : m_itemsMap)
	{
		FreeListBrowserList(&item.second);
	}

#endif /* __amigaos__ */

}

/**
 * Activates a previously created content ID.
 * This method accepts an integer identifier that represents a list of items that have been previously added to the
 * tree.  It will remove the current set of items and replace them will the ones represented by the identifier.  This
 * enables switching between multiple different sets of items.
 *
 * The content ID must have a value of >= 1. If the special value of 0 is passed in, the contents of the tree gadget
 * will be set to empty.
 *
 * @date	Saturday 24-Feb-2024 6:25 am, Code HQ Tokyo Tsukuda
 * @param	a_contentID		A content ID identifier, returned from newContentID()
 */

void CStdGadgetTree::activateContent(int a_contentID)
{
	ASSERTM((a_contentID >= 0), "CStdGadgetTree::activateContent() => Invalid content ID passed in");

	m_contentID = a_contentID;

#ifdef __amigaos__

	SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) ~0, TAG_DONE);

	if (a_contentID != 0)
	{
		ULONG items = (ULONG) &m_itemsMap[a_contentID];

		RefreshSetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentLayout->WindowOrNULL(), NULL, LISTBROWSER_Labels, items, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	while ((m_tree.takeTopLevelItem(0)) != nullptr) { }

	if (a_contentID != 0)
	{
		m_tree.addTopLevelItems( m_itemsMap[a_contentID]);
	}

#else /* ! QT_GUI_LIB */

	(void) a_contentID;

#endif /* ! QT_GUI_LIB */

}

/**
 * Add an item to a list.
 * This method accepts a CTreeNode item, containing a text strings that will be used to add a node to the list
 * represented by the given content ID.
 *
 * The item passed in will be converted to a format suitable for the underlying native tree gadget, and the item that
 * is passed in is no longer required and can be disposed of. If the content ID passed in is 0, this method will
 * create a new list, add the item to the new list, and return the content ID of the new list.
 *
 * Note that this method does not activate the content that is updated; this can be done with activateContent() using
 * the content ID. However, if the content is already active, the display will be updated immediately.
 *
 * @pre		The number of strings passed in in a_item.m_columnText must match the number of columns in the tree
 *
 * @date	Monday 13-Sep-2021 8:53 am, Code HQ Bergmannstrasse
 * @param	a_item			The item to be added to the list
 * @param	a_contentID		0 to create a new content list or an already existing content ID
 * @return	An identifier that represents the content that was just added or updated
 */

int CStdGadgetTree::addItem(CTreeNode &a_item, int a_contentID)
{
	ASSERTM((a_contentID >= 0), "CStdGadgetTree::addItem() => Content index must not be negative");
	ASSERTM((a_contentID < m_nextContentID), "CStdGadgetTree::addItem() => Content index is out of range");

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	int numColumns = static_cast<int>(a_item.m_columnText.size());

	ASSERTM((numColumns == m_numColumns), "CStdGadgetTree::addItem() => Number of columns passed in does not match number in gadget tree");

#endif /* defined(__amigaos__) || defined(QT_GUI_LIB) */

	/* If no content ID is passed in, create a new list with a new unique content ID. Otherwise, we want to */
	/* update an existing list */
	if (a_contentID == 0)
	{
		a_contentID = m_nextContentID++;
	}

#ifdef __amigaos__

	struct List *currentList = nullptr, *itemList;
	struct Node *node;

	/* If the content ID does not already exist in the map, create a new list for it. In this case, accessing the */
	/* items map will return a new exec List, but it must be initialised by calling NewList(). If it already exists, */
	/* it is valid but we want to clear its contents with FreeListBrowserList() */
	if (m_itemsMap.find(a_contentID) == m_itemsMap.end())
	{
		itemList = &m_itemsMap[a_contentID];
		NewList(itemList);
	}
	else
	{
		itemList = &m_itemsMap[a_contentID];
	}

	/* Get a pointer to the currently active list. If it is the one we are about to update, we need to remove */
	/* it from the list browser before we update it */
	GetAttr(LISTBROWSER_Labels, m_poGadget, (ULONG *) &currentList);

	if (itemList == currentList)
	{
		SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) ~0, TAG_DONE);
	}

	/* Allocate a new node, initialise its columns and add it to the list */
	if ((node = AllocListBrowserNode(numColumns, LBNA_UserData, (ULONG) a_item.m_userData, TAG_DONE)) != nullptr)
	{
		for (int index = 0; index < numColumns; ++index)
		{
			SetListBrowserNodeAttrs(node, LBNA_Column, index, LBNCA_CopyText, TRUE, LBNCA_Text,
				(ULONG) a_item.m_columnText[index].c_str(), TAG_DONE);
		}

		AddTail(itemList, node);
	}

	/* If this list is the currently active list, re-add it to the list browser */
	if (itemList == currentList && m_bActive)
	{
		SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentLayout->WindowOrNULL(), NULL, LISTBROWSER_Labels, (ULONG) itemList, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	/* Get a pointer to the content ID's item list, create and append a new QTreeWidgetItem instance to it, and */
	/* assign the list to the tree */
	QList<QTreeWidgetItem *> &items = m_itemsMap[a_contentID];
	QStringList strings;

	for (int index = 0; index < numColumns; ++index)
	{
		strings.append(a_item.m_columnText[index].c_str());
	}

	QTreeWidgetItem *item = new QTreeWidgetItem(strings);

	/* Store the user data passed in and add the item to the tree */
	item->setData(0, Qt::UserRole, a_item.m_userData);
	items.append(item);
	m_tree.addTopLevelItems(items);

#else /* ! QT_GUI_LIB */

	(void) a_item;
	(void) a_contentID;

#endif /* ! QT_GUI_LIB */

	return a_contentID;
}

#ifdef __amigaos__

/**
 * Creates the native underlying gadget.
 * This is a convenience method, currently only for Amiga OS, used for creating the native BOOPSI list browser
 * gadget.  This is useful as it must be created from more than one place on Amiga OS.
 *
 * @date	Saturday 03-Feb-2024 7:14 am, Code HQ Tokyo Tsukuda
 * @return	true if the gadget was created successfully, else false
 */

bool CStdGadgetTree::createNative()
{
	/* We perform the creation in two steps, to take into account that the column info structure can be reused, */
	/* even if the gadget itself has been destroyed by Reaction. Start by allocating the column info structure, */
	/* if it is not yet allocated */
	if (m_columnInfo == nullptr)
	{
		m_columnInfo = new struct ColumnInfo[m_numColumns + 1];

		if (m_columnInfo != nullptr)
		{
			/* Initialise each column such that they take up an equal amount of space */
			for (int index = 0; index < m_numColumns; ++index)
			{
				m_columnInfo[index].ci_Width = 100 / m_numColumns;
				m_columnInfo[index].ci_Title = "";
				m_columnInfo[index].ci_Flags = CIF_SORTABLE;
			}

			/* Terminate the column list */
			m_columnInfo[m_numColumns].ci_Width = -1;
			m_columnInfo[m_numColumns].ci_Title = (STRPTR) ~0;
			m_columnInfo[m_numColumns].ci_Flags = (ULONG) -1;
		}
	}

	/* If the column info structure was successfully allocated, create the gadget */
	if (m_columnInfo != nullptr)
	{
		/* This may be the first time this is called, or it may have been called before, in which case we need to */
		/* use the already existing item list */
		ULONG items = (ULONG) (m_contentID != 0) ? (ULONG) &m_itemsMap[m_contentID] : (ULONG) &m_itemsList;

		/* And create the native list browser gadget */
		m_poGadget = (Object *) NewObject(LISTBROWSER_GetClass(), NULL, GA_ID, (ULONG) m_iGadgetID, GA_RelVerify, TRUE,
			LISTBROWSER_ColumnTitles, TRUE, LISTBROWSER_ColumnInfo, (ULONG) m_columnInfo, LISTBROWSER_Labels, items,
			TAG_DONE);

		if (m_poGadget == nullptr)
		{
			delete [] m_columnInfo;
			m_columnInfo = nullptr;
		}
	}

	return (m_poGadget != nullptr);
}

#endif /* __amigaos__ */

/**
 * Returns the currently selected item.
 * Determines the current item in the tree gadget and optionally returns its string contents and/or user data. This
 * method assumes that an item is currently selected, and no effort is made to account for the case when one is not.
 *
 * @pre		An item is currently selected
 *
 * @date	Thursday 05-Aug-2021 12:42 pm, Code HQ @ Thomas's House
 * @param	a_column		The number of the column for which to get the item
 * @param	a_text			Optional pointer to a variable into which to place the node's text contents
 * @param	a_userData		Optional pointer to a variable into which to place the node's user data
 */

void CStdGadgetTree::getSelectedItem(int a_column, std::string *a_text, uint32_t *a_userData)
{
	ASSERTM((m_poGadget != nullptr), "CStdGadgetTree::getSelectedItem() => Tree gadget has not been created");
	ASSERTM((a_column < m_numColumns), "CStdGadgetTree::getSelectedItem() => Column number is out of range");

#ifdef __amigaos__

	char *text;
	struct Node *node;

	GetAttr(LISTBROWSER_SelectedNode, m_poGadget, (ULONG *) &node);
	ASSERTM((node != nullptr), "CStdGadgetTree::getSelectedItem() => No item is currently selected");

	if (a_text != nullptr)
	{
		GetListBrowserNodeAttrs(node, LBNA_Column, a_column, LBNCA_Text, (ULONG) &text, TAG_DONE);
		*a_text = std::string(text);
	}

	if (a_userData != nullptr)
	{
		GetListBrowserNodeAttrs(node, LBNA_UserData, (ULONG) a_userData, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	QTreeWidgetItem *currentItem = m_tree.currentItem();

	if (a_text != nullptr)
	{
		*a_text = std::string(currentItem->text(a_column).toStdString());
	}

	if (a_userData != nullptr)
	{
		*a_userData = currentItem->data(0, Qt::UserRole).toInt();
	}

#else /* ! QT_GUI_LIB */

	(void) a_column;
	(void) a_text;

	if (a_userData != nullptr)
	{
		*a_userData = 0;
	}

#endif /* ! QT_GUI_LIB */

}

/**
 * Create a new content ID.
 * Creates a new content ID and thus a new empty list that can be used for inserting items. The list will be stored
 * internally in the CStdGadgetTree class and can be accessed via the content ID returned from this method.
 *
 * @date	Thursday 12-Feb-2026 8:35 am, Code HQ Tokyo Tsukuda
 * @return	The content ID representing the new list
 */

int CStdGadgetTree::newContentID()
{
	int retVal = m_nextContentID++;

#ifdef __amigaos__

	struct List *itemList = &m_itemsMap[retVal];
	NewList(itemList);

#elif defined(QT_GUI_LIB)

	QList<QTreeWidgetItem *> items;

	m_itemsMap.emplace(retVal, items);

#endif /* QT_GUI_LIB */

	return retVal;
}

/**
 * Remove an item from a list.
 * This method accepts a string that represents the contents of the first column of the item to be removed from the
 * list represented by the given content ID. It will search for the string and if it is found, it will be removed from
 * the list.
 *
 * @date	Sunday 08-Mar-2026 6:47 am, Code HQ Tokyo Tsukuda
 * @param	a_text			The string for which to search
 */

void CStdGadgetTree::removeItem(std::string &a_text, int a_contentID)
{
	ASSERTM((a_contentID >= 0), "CStdGadgetTree::removeItem() => Content index must not be negative");
	ASSERTM((a_contentID < m_nextContentID), "CStdGadgetTree::removeItem() => Content index is out of range");

#ifdef __amigaos__

	const char *text;
	struct List *currentList = nullptr, *itemList;
	struct Node *node;

	// TODO: CAW - Should we protect accesses to the map?
	//if (m_itemsMap.find(a_contentID) == m_itemsMap.end())
	itemList = &m_itemsMap[a_contentID];

	/* Get a pointer to the currently active list. If it is the one we are about to update, we need to remove */
	/* it from the list browser before we update it */
	GetAttr(LISTBROWSER_Labels, m_poGadget, (ULONG *) &currentList);

	if (itemList == currentList)
	{
		SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) ~0, TAG_DONE);
	}

	node = itemList->lh_Head;

	/* Iterate through the list and try to find the item to remove */
	while (node != nullptr)
	{
		text = nullptr;
		GetListBrowserNodeAttrs(node, LBNA_Column, 0, LBNCA_Text, (ULONG) &text, TAG_DONE);

		if (text != nullptr && strcmp(text, a_text.c_str()) == 0)
		{
			/* Found it! Remove and free the node and break out of the loop */
			Remove(node);
			FreeListBrowserNode(node);

			break;
		}

		node = node->ln_Succ;
	}

	/* If this list is the currently active list, re-add it to the list browser */
	if (itemList == currentList && m_bActive)
	{
		SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) itemList, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	QList<QTreeWidgetItem *> &items = m_itemsMap[a_contentID];

	auto it = items.begin();

	/* Iterate through the list and try to find the item to remove */
	while (it != items.end())
	{
		if ((*it)->text(0) == QString::fromStdString(a_text))
		{
			/* Found it! Remove and free the node and break out of the loop */
			int index = m_tree.indexOfTopLevelItem(*it);

			if (index != -1)
			{
				QTreeWidgetItem *oldItem = m_tree.takeTopLevelItem(index);
				delete oldItem;
			}

			items.erase(it);

			break;
		}

		++it;
	}

#else /* ! QT_GUI_LIB */

	(void) a_text;
	(void) a_contentID;

#endif /* ! QT_GUI_LIB */

}

/**
 * Set the width of a column.
 * Sets the width of the specified column, as a percentage of the width of the tree gadget.
 *
 * @date	Sunday 15-Feb-2026 6:33 am, Code HQ Tokyo Tsukuda
 * @param	a_column		The number of the column for which to set the width
 * @param	a_width			The width of the column, as a percentage
 */

void CStdGadgetTree::setColumnWidth(int a_column, int a_width)
{
	ASSERTM((a_column >= 0), "CStdGadgetTree::setWidth() => Column index must not be negative");
	ASSERTM((a_column < m_numColumns), "CStdGadgetTree::setWidth() => Column index is out of range");
	ASSERTM((a_width >= 0), "CStdGadgetTree::setWidth() => Column width must not be negative");

#ifdef __amigaos__

	m_columnInfo[a_column].ci_Width = a_width;

#elif defined(QT_GUI_LIB)

	/* Convert the percentage to a pixel width before setting it */
	a_width = a_width * m_tree.width() / 100;
	m_tree.header()->resizeSection(a_column, a_width);

# else /* ! QT_GUI_LIB */

	(void) a_column;
	(void) a_width;

#endif /* ! QT_GUI_LIB */

}

/**
 * Sets the title of the gadget tree.
 * Sets or updates the title currently displayed in the gadget tree.
 *
 * @date	Friday 29-Dec-2023 11:00 am, Code HQ @ Ashley's house
 * @param	a_title			Title to be displayed at the top of the tree's column
 */

void CStdGadgetTree::setTitle(const std::string &a_title)
{

#ifdef __amigaos__

	/* Only set the title if it is actually different */
	if (m_title != a_title)
	{
		/* For Amiga OS the title string is not copied by the native gadget, so we copy it into our own persistent */
		/* memory before assigning it to the tree */
		m_title = a_title;
		m_columnInfo[0].ci_Title = (STRPTR) m_title.c_str();

		SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_ColumnInfo, (ULONG) m_columnInfo, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	m_tree.setTitle(a_title);

#else /* ! QT_GUI_LIB */

	(void) a_title;

#endif /* ! QT_GUI_LIB */

}

#ifdef __amigaos__

/**
 * Shows or hides the gadget.
 * This Amiga OS reimplementation of CStdGadget::SetVisible() will pass the call through to the super method if the
 * gadget is to be hidden, but will recreate the underlying BOOPSI gadget if the gadget is to be shown.  This enables
 * list browser gadgets to be hidden and unhidden on Amiga OS, even though the underlying Reaction system does not
 * support this functionality.
 *
 * @date	Friday 29-Dec-2023 4:34 pm, Code HQ @ Ashley's house
 * @param	a_bVisible		true to make gadget visible, else false to hide it
 * @return	True if the visibility of the gadget changed, else false
 */

bool CStdGadgetTree::SetVisible(bool a_bVisible)
{
	bool retVal = false;

	/* If the gadget is being made visible then we want to recreate it */
	if (a_bVisible)
	{
		/* But only if it was previously hidden and thus does not already exist */
		if (m_bHidden)
		{
			m_poParentLayout->m_bEnableRefresh = EFalse;

			if (createNative())
			{
				retVal = true;

				/* Reattach it to the parent window */
				m_poParentLayout->ReAttach(this);
				m_bHidden = false;
			}

			m_poParentLayout->m_bEnableRefresh = ETrue;
		}
	}
	/* Otherwise just pass the call through to the super method to hide the gadget */
	else
	{
		retVal = CStdGadget::SetVisible(a_bVisible);
	}

	return retVal;
}

#endif /* __amigaos__ */
