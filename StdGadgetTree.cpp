
#include "StdFuncs.h"
#include "StdGadgets.h"
#include "StdWindow.h"

#ifdef __amigaos__

#include "StdReaction.h"

#elif defined(QT_GUI_LIB)

#include "Qt/QtGadgetTree.h"
#include "Qt/QtWindow.h"

#endif /* QT_GUI_LIB */

#ifdef __amigaos__

static struct ColumnInfo g_columnInfo[] =
{
	{ 100, "", CIF_SORTABLE } ,
	{ -1, (STRPTR) ~0, (ULONG) -1 }
};

#endif /* __amigaos__ */

/**
 * Second phase constructor.
 * This is called by the New() wrapper to perform initialisation steps that can fail and need to be cleaned
 * up upon failure.
 *
 * @date	Sunday 18-Jul-2021 12:32 pm, Am Theater Stralsund FeWo
 * @return	KErrNone if successful, else KErrNoMemory
 */

int CStdGadgetTree::construct()
{
	int retVal = KErrNone;

	ASSERTM((m_poParentWindow != nullptr), "CStdGadgetTree::construct() => Parent window is NULL");
	ASSERTM((m_poParentWindow->m_poWindow != nullptr), "CStdGadgetTree::construct() => Parent native window is NULL");

#ifdef __amigaos__

	if (!createNative())
	{
		retVal = KErrNoMemory;
	}

#elif defined(QT_GUI_LIB)

	/* Initialise the Qt specific helper object */
	m_poGadget = &m_tree;
	m_tree.construct("");

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

	/* Iterate through the map of available lists and delete the nodes from each one */
	for (auto &item : m_itemsMap)
	{
		List &list = item.second;
		CTreeNode *node;

		while ((node = reinterpret_cast<CTreeNode *>(RemHead(&list))) != NULL)
		{
			FreeListBrowserNode(reinterpret_cast<struct Node *>(node));
		}
	}

#endif /* __amigaos__ */

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
    g_columnInfo[0].ci_Title = (STRPTR) m_title.c_str();

    m_poGadget = (Object *) NewObject(LISTBROWSER_GetClass(), NULL, GA_ID, (ULONG) m_iGadgetID, GA_RelVerify, TRUE,
        LISTBROWSER_ColumnTitles, TRUE, LISTBROWSER_ColumnInfo, (ULONG) &g_columnInfo, LISTBROWSER_Labels, (ULONG) &m_fileList,
        TAG_DONE);

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
 * Sets the content of the tree gadget.
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

void CStdGadgetTree::setContent(int a_contentID)
{
	ASSERTM((a_contentID >= 0), "CStdGadgetTree::setContent() => Invalid content ID passed in");

	m_contentID = a_contentID;

#ifdef __amigaos__

	SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) NULL, TAG_DONE);

	if (a_contentID != 0)
	{
		ULONG items = (ULONG) &m_itemsMap[a_contentID];

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

/**
 * Sets the content of the tree gadget.
 * This method accepts a list of CTreeNode items, with each item containing a text string that will be used to
 * add a node to the tree.
 *
 * The items passed in will be converted to a format suitable for the underlying native tree gadget, and the original
 * list is no longer required and can be disposed of.  This method can be called multiple times and a new internal
 * list will be created each time.  An integer identifier representing that list will be returned, which can be used
 * to switch between lists using setContent(int).
 *
 * @date	Monday 13-Sep-2021 8:53 am, Code HQ Bergmannstrasse
 * @param	a_items			The list of items to be added to the tree
 * @return	An identifier that represents the content that was just added
 */

int CStdGadgetTree::setContent(StdList<CTreeNode> &a_items)
{

#ifdef __amigaos__

	/* Any previously added item list must be detached from the list browser gadget before being updated */
	SetGadgetAttrs((struct Gadget *) m_poGadget, NULL, NULL, LISTBROWSER_Labels, (ULONG) NULL, TAG_DONE);

	struct List &fileList = m_itemsMap[m_nextContentID];

	NewList(&fileList);

	/* Iterate through the list passed in and, for each item on the list, create a list browser node to represent */
	/* it, and add it to the list browser's list of nodes */
	CTreeNode *treeNode = a_items.getHead();

	while (treeNode)
	{
		struct Node *node;

		if ((node = AllocListBrowserNode(1, LBNCA_CopyText, TRUE, LBNCA_Text, (ULONG) treeNode->m_text.c_str(),
			TAG_DONE)) != nullptr)
		{
			AddTail(&fileList, node);
		}

		treeNode = a_items.getSucc(treeNode);
	}

	/* Re-add the updated list of items to the list browser gadget */
	SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL, LISTBROWSER_Labels, (ULONG) &fileList, TAG_DONE);

#elif defined(QT_GUI_LIB)

	QList<QTreeWidgetItem *> items;

	while (static_cast<QTreeWidget *>(m_poGadget)->takeTopLevelItem(0)) { }

	/* Iterate through the list passed in and, for each item on the list, create a widget item to represent it, */
	/* and add it to the tree widget's list of nodes */
	CTreeNode *treeNode = a_items.getHead();

	while (treeNode)
	{
		items.append(new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr), QStringList(QString(treeNode->m_text.c_str()))));
		treeNode = a_items.getSucc(treeNode);
	}

	/* Add the new list of items to the tree widget */
	static_cast<QTreeWidget *>(m_poGadget)->insertTopLevelItems(0, items);

	m_itemsMap.emplace(m_nextContentID, items);

#else /* ! QT_GUI_LIB */

	(void) a_items;

#endif /* ! QT_GUI_LIB */

	m_contentID = m_nextContentID++;

	return m_contentID;
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
		g_columnInfo[0].ci_Title = (STRPTR) m_title.c_str();

		SetGadgetAttrs((struct Gadget *) m_poGadget, m_poParentWindow->m_poWindow, NULL, LISTBROWSER_ColumnInfo, (ULONG) g_columnInfo, TAG_DONE);
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
