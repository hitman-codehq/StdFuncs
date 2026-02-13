
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdWindow.h"

#ifdef __amigaos__

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
	m_tree.construct("", m_numColumns); // TODO: CAW - Title?
	//m_tree.setColumnCount(m_numColumns);

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
 * All content IDs have a value of >= 1.  If the special value of 0 is passed in, the contents of the tree gadget are
 * set to empty.
 *
 * @date	Saturday 24-Feb-2024 6:25 am, Code HQ Tokyo Tsukuda
 * @param	a_contentID		A content ID identifier, returned from setContent(StdList<CTreeNode> &)
 */

void CStdGadgetTree::activateContent(int a_contentID)
{
	ASSERTM((a_contentID >= 0), "CStdGadgetTree::activateContent() => Invalid content ID passed in");

	m_contentID = a_contentID;

#ifdef __amigaos__

	SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) NULL, TAG_DONE);

	if (a_contentID != 0)
	{
		ULONG items = (ULONG) &m_itemsMap[a_contentID];

		Utils::info("CStdGadgetTree::activateContent() => Activating content ID %d", a_contentID);
		SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL, LISTBROWSER_Labels, items, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	while (static_cast<QTreeWidget *>(m_poGadget)->takeTopLevelItem(0)) { }

	if (a_contentID != 0)
	{
		static_cast<QTreeWidget *>(m_poGadget)->insertTopLevelItems(0, m_itemsMap[a_contentID]);
	}

#else /* ! QT_GUI_LIB */

	(void) a_contentID;

#endif /* ! QT_GUI_LIB */

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
	m_columnInfo = new struct ColumnInfo[m_numColumns + 1];

	if (m_columnInfo != nullptr)
	{
		// TODO: CAW - This needs to be dynamically allocated, and when adding a new node, we need to ensure that the number
		//             of strings passed in matches this number
		for (int index = 0; index < m_numColumns; ++index)
		{
			m_columnInfo[index].ci_Width = 100 / m_numColumns;
			m_columnInfo[index].ci_Title = "";
			m_columnInfo[index].ci_Flags = CIF_SORTABLE | CIF_WEIGHTED;
		}

		m_columnInfo[m_numColumns].ci_Width = -1;
		m_columnInfo[m_numColumns].ci_Title = (STRPTR) ~0;
		m_columnInfo[m_numColumns].ci_Flags = (ULONG) -1;

		//g_columnInfo[0].ci_Title = (STRPTR) m_title.c_str();

		m_poGadget = (Object *) NewObject(LISTBROWSER_GetClass(), NULL, GA_ID, (ULONG) m_iGadgetID, GA_RelVerify, TRUE,
			LISTBROWSER_ColumnTitles, TRUE, LISTBROWSER_ColumnInfo, (ULONG) m_columnInfo, LISTBROWSER_Labels, (ULONG) &m_fileList,
			TAG_DONE);

		if (m_poGadget == nullptr)
		{
			delete [] m_columnInfo;
		}
	}

	return (m_poGadget != nullptr);
}

#endif /* __amigaos__ */

/**
 * Returns the currently selected item.
 * Determines the current item in the tree gadget and returns its string contents.  This method assumes that
 * an item is currently selected, and no effort is made to account for the case when one is not.
 *
 * @pre		An item is currently selected
 *
 * @date	Thursday 05-Aug-2021 12:42 pm, Code HQ @ Thomas's House
 * @return	The item's string contents
 */

std::string CStdGadgetTree::getSelectedItem()
{
	ASSERTM((m_poGadget != nullptr), "CStdGadgetTree::getSelectedItem() => Tree gadget has not been created");

#ifdef __amigaos__

	char *text;
	struct Node *node;

	GetAttr(LISTBROWSER_SelectedNode, m_poGadget, (ULONG *) &node);
	GetListBrowserNodeAttrs(node, LBNCA_Text, (ULONG) &text, TAG_DONE);
	std::string retVal(text);

#elif defined(QT_GUI_LIB)

	QTreeWidgetItem *currentItem = static_cast<QTreeWidget *>(m_poGadget)->currentItem();
	std::string retVal(currentItem->text(0).toStdString());

#else /* ! QT_GUI_LIB */

	std::string retVal;

#endif /* ! QT_GUI_LIB */

	return retVal;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 12-Feb-2026 8:35 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int CStdGadgetTree::newContentID()
{
	int retVal = m_nextContentID++;

#ifdef __amigaos__

	Utils::info("Creating new list for content ID %d", retVal);
	struct List *fileList = &m_itemsMap[retVal];
	NewList(fileList);

#elif defined(QT_GUI_LIB)

	QList<QTreeWidgetItem *> items;

	/*if (!*/ m_itemsMap.emplace(retVal, items); //.second;

#endif /* QT_GUI_LIB */

	return retVal;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Sunday 15-Feb-2026 6:33 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdGadgetTree::setColumnWidth(int a_column, int a_width)
{
	ASSERTM((a_column < m_numColumns), "CStdGadgetTree::setWidth() => Column index is out of range");

#ifdef __amigaos__

	m_columnInfo[a_column].ci_Width = a_width;

#elif defined(QT_GUI_LIB)

	auto header = m_tree.header(); // TODO: CAW - Ensure this is used everywhere

	header->setSectionResizeMode(a_column, QHeaderView::Stretch);
	header->setSectionResizeMode(1, QHeaderView::Stretch);

	header->resizeSection(a_column, a_width);
	header->resizeSection(1, 1);

#endif /* QT_GUI_LIB */

}

/**
 * Sets the content of the tree gadget.
 * This method accepts a list of CTreeNode items, with each item containing a text string that will be used to
 * add a node to the tree.
 *
 * The items passed in will be converted to a format suitable for the underlying native tree gadget, and the list that
 * is passed in is no longer required and can be disposed of.  This method can be called multiple times, with the
 * option of either creating a new internal list, or updating an existing one. The latter use case is useful when a
 * client is holding a content ID and wishes to reuse it. To create a new list, pass in a content ID of 0; to update
 * an existing list, pass in that list's content ID.
 *
 * Note that this method does not activate the content that is updated; this can be done with activateContent() using
 * the Content ID returned from this method. However, if the content is already active, the display will be updated
 * immediately.
 *
 * @date	Monday 13-Sep-2021 8:53 am, Code HQ Bergmannstrasse
 * @param	a_items			The list of items to be added to the tree
 * @param	a_contentID		0 to create a new content list or an ID previously returned from this method
 * @return	An identifier that represents the content that was just added or updated
 */

int CStdGadgetTree::setContent(CTreeNode &a_item, int a_contentID)
{
	/* If no content ID is passed in, create a new list with a new unique content ID. Otherwise, we want to */
	/* update an existing list */
	if (a_contentID == 0)
	{
		Utils::info("CStdGadgetTree::setContent() => Creating new content ID");
		a_contentID = m_nextContentID++;
	}

#if defined(__amigaos__) || defined(QT_GUI_LIB)

	int numColumns = static_cast<int>(a_item.m_columnText.size());

	ASSERTM((numColumns == m_numColumns), "CStdGadgetTree::setContent() => Number of columns passed in does not match number in gadget tree");

#endif /* defined(__amigaos__) || defined(QT_GUI_LIB) */

#ifdef __amigaos__

	struct List *currentList = nullptr, *fileList;
	struct Node *node;

	/* If the content ID does not already exist in the map, create a new list for it. In this case, accessing the */
	/* items map will return a new exec List, but it must be initialised by calling NewList(). If it already exists, */
	/* it is valid but we want to clear its contents with FreeListBrowserList() */
	if (m_itemsMap.find(a_contentID) == m_itemsMap.end())
	{
		Utils::info("Creating new list for content ID %d", a_contentID);
		fileList = &m_itemsMap[a_contentID];
		NewList(fileList);
	}
	else
	{
		Utils::info("Using old list for content ID %d", a_contentID);
		fileList = &m_itemsMap[a_contentID];
	}

	/* Get a pointer to the currently active list. If it is the one we are about to update, we need to remove */
	/* it from the list browser before we update it */
	GetAttr(LISTBROWSER_Labels, m_poGadget, (ULONG *) &currentList);

	Utils::info("%p %p %p", fileList, currentList, &m_fileList);
	if (fileList == currentList)
	{
		Utils::info("*** removing list %p from list browser", fileList);
		SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) ~0, TAG_DONE);
	}

	Utils::info("Allocating %d columns", numColumns);
	if ((node = AllocListBrowserNode(numColumns, LBNA_UserData, (ULONG) a_item.m_userData, TAG_DONE)) != nullptr)
	{
		for (int index = 0; index < numColumns; ++index)
		{
			Utils::info("*** %d %s", index, a_item.m_columnText[index].c_str());
			SetListBrowserNodeAttrs(node, LBNA_Column, index, LBNCA_CopyText, TRUE, LBNCA_Text,
				(ULONG) a_item.m_columnText[index].c_str(), TAG_DONE);
		}

		AddTail(fileList, node);
	}

	/* If this list is the currently active list, re-add it to the list browser */
	if (fileList == currentList && m_bActive)
	{
		Utils::info("*** adding list %p to list browser", fileList);
		SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL, LISTBROWSER_Labels, (ULONG) fileList, TAG_DONE);
	}

#elif defined(QT_GUI_LIB)

	QList<QTreeWidgetItem *> *items = &m_itemsMap[a_contentID]; // TODO: CAW - Bounds checking

	//if (updateContent)
	{
		//while (static_cast<QTreeWidget *>(m_poGadget)->takeTopLevelItem(0)) { }
	}

	/* Iterate through the list passed in and, for each item on the list, create a widget item to represent it, */
	/* and add it to the tree widget's list of nodes */
	//CTreeNode *treeNode = a_items.getHead();

	//while (treeNode)
	//{
	//for (int index = 0; index < numColumns; ++index)
	{
		QStringList strings;

		for (int index = 0; index < numColumns; ++index)
		{
			Utils::info("*** %d %s", index, a_item.m_columnText[index].c_str());
			strings.append(a_item.m_columnText[index].c_str());
		}

		items->append(new QTreeWidgetItem(strings));
	}
		//treeNode = a_items.getSucc(treeNode);
	//}

	/* Add the new list of items to the tree widget */
	//if (updateContent)
	{
		static_cast<QTreeWidget *>(m_poGadget)->insertTopLevelItems(0, *items);
	}

	/* Mixing the STL and Qt list is a pain. We have to do it like this to ensure that the data in items is actually */
	/* copies or moved into the target list item, rather than a shared pointer being used */
	//if (!m_itemsMap.emplace(a_contentID, items).second)
	{
		//m_itemsMap[a_contentID] = std::move(items);
	}

#else /* ! QT_GUI_LIB */

	(void) a_items;

#endif /* ! QT_GUI_LIB */

	return a_contentID;
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
		/* For Amiga OS the title string is not copied by the native gadget, so we copy it into our own persistent memory */
		/* before assigning it to the tree */

		m_title = a_title;
		m_columnInfo[0].ci_Title = (STRPTR) m_title.c_str();

		// TODO: CAW - Huh?
		SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL, LISTBROWSER_ColumnInfo, (ULONG) m_columnInfo, TAG_DONE);
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
 */

void CStdGadgetTree::SetVisible(bool a_bVisible)
{
	/* If the gadget is being made visible then we want to recreate it */
	if (a_bVisible)
	{
		/* But only if it was previously hidden and thus does not already exist */
		if (m_bHidden)
		{
			m_poParentLayout->m_bEnableRefresh = EFalse;

			if (createNative())
			{
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
		CStdGadget::SetVisible(a_bVisible);
	}
}

#endif /* __amigaos__ */
