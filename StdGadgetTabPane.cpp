
#include "StdFuncs.h"
#include "StdGadgetTabPane.h"

#ifdef __amigaos__

#include "StdReaction.h"
#include "StdWindow.h"

#elif defined(QT_GUI_LIB)

#include <QtWidgets/QTabWidget>

#endif /* QT_GUI_LIB */

/**
 * Creates an instance of the tab pane class.
 * Allocates and initialises the class, providing two phase construction.
 *
 * @date	Monday 29-Dec-2025 3:45 pm, London Town Hotel
 * @param	a_poParentLayout	Pointer to the layout in which to create the gadget
 * @param	a_gadgetID			The unique ID to assign to the gadget
 * @param	a_gadget1			Pointer to the first child gadget to be added
 * @param	a_gadget2			Pointer to the second child gadget to be added
 * @return	A pointer to an initialised class instance if successful, else nullptr
 */

CStdGadgetTabPane *CStdGadgetTabPane::New(CStdGadgetLayout *a_parentLayout, int a_gadgetID, CStdGadget *a_gadget1,
	CStdGadget *a_gadget2)
{
	CStdGadgetTabPane *retVal;

	ASSERTM((a_gadgetID > 0), "CStdGadgetTabPane::New() => Gadget ID must be > 0");

	if ((retVal = new CStdGadgetTabPane(a_gadgetID, a_parentLayout)) != nullptr)
	{
		if (retVal->construct(a_gadget1, a_gadget2) != KErrNone)
		{
			delete retVal;
			retVal = nullptr;
		}
	}

	return retVal;
}

/**
 * Second phase constructor.
 * This second phase constructor will create the underlying OS specific gadgets required to show the tab
 * pane gadget.
 *
 * @date	Monday 29-Dec-2025 3:45 pm, London Town Hotel
 * @param	a_gadget1		Pointer to the first child gadget to be added
 * @param	a_gadget2		Pointer to the second child gadget to be added
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available
 */

int CStdGadgetTabPane::construct(CStdGadget *a_child1, CStdGadget *a_child2)
{

#ifdef __amigaos__

	int retVal = KErrNone;
	Node *node;

	/* For Amiga OS's Reaction, it is very difficult to dynamically add child gadgets to a tab pane gadget. To */
	/* begin with, we are going to have to hard code the addition of these gadgets. Start by pushing them onto */
	/* the internal gadget tracking list */
	m_gadgets.push_back(a_child1);
	m_gadgets.push_back(a_child2);

	/* Initialise the Exec node list, and add the two tabs to it */
	NewList(&m_nodeList);
	node = AllocClickTabNode(TNA_Text, "Problems", TNA_Number, 0, TAG_DONE);
	AddTail(&m_nodeList, node);
	node = AllocClickTabNode(TNA_Text, "Build", TNA_Number, 1, TAG_DONE);
	AddTail(&m_nodeList, node);

	if (!createNative())
	{
		retVal = KErrNoMemory;
	}

#elif defined(QT_GUI_LIB)

	(void) a_child1;
	(void) a_child2;

	int retVal = KErrNone;

	m_tabWidget = new QTabWidget;
	m_poGadget = m_tabWidget;

#else /* ! QT_GUI_LIB */

	(void) a_child1;
	(void) a_child2;

	int retVal = KErrNone;

#endif /* ! QT_GUI_LIB */

	if (retVal == KErrNone)
	{
		m_poParentLayout->Attach(this);
	}

	return retVal;
}

#ifdef __amigaos__

/**
 * Creates the native underlying gadget.
 * This is a convenience method, currently only for Amiga OS, used for creating the native BOOPSI tab pane
 * gadget. This is useful as it must be created from more than one place on Amiga OS.
 *
 * @date	Tuesday 10-Mar-2026 6:40 am, Code HQ Tokyo Tsukuda
 * @return	true if the gadget was created successfully, else false
 */

bool CStdGadgetTabPane::createNative()
{
	/* Create two Page gadgets, one for each tab, and a ClickTab gadget with those pages */
	if ((m_gadgetsLayout = NewObject(PAGE_GetClass(), NULL, LAYOUT_Orientation, LAYOUT_VERTICAL,
		PAGE_Add, (ULONG) m_gadgets[0]->GetGadget(), PAGE_Add, (ULONG) m_gadgets[1]->GetGadget(), TAG_DONE)) != NULL)
	{
		if ((m_poGadget = (Object *) NewObject(CLICKTAB_GetClass(), NULL, GA_ID, (ULONG) m_iGadgetID,
			GA_RelVerify, TRUE, CLICKTAB_Labels, (ULONG) &m_nodeList, CLICKTAB_PageGroup, (ULONG) m_gadgetsLayout,
			TAG_DONE)) == nullptr)
		{
			DisposeObject(m_gadgetsLayout);
		}
	}

	return (m_poGadget != nullptr);
}

#endif /* __amigaos__ */

/**
 * Dynamically add a tab to the tab pane.
 * As well as passing in a list of child gadgets when the tab pane is created, it is possible to add new tabs
 * dynamically using this method.
 *
 * @date	Monday 19-Jan-2026 6:37 am, Code HQ Tokyo Tsukuda
 * @param	a_gadget		Pointer to the gadget to be added to the tab pane
 * @param	a_title			Title to be given to the newly added tab
 */

void CStdGadgetTabPane::addTab(CStdGadget *a_gadget, const char *a_title)
{
	/* Push the gadget onto the internal gadget tracking list */
	m_gadgets.push_back(a_gadget);

#ifdef __amigaos__

	Node *node;

	/* Remove the node list from the ClickTab gadget, add a new node to it and then reattach it */
	SetAttrs(m_poGadget, CLICKTAB_Labels, ~0, TAG_DONE);

	node = AllocClickTabNode(TNA_Text, a_title, TNA_Number, 0, TAG_DONE);
	AddTail(&m_nodeList, node);

	SetGadgetAttrs((Gadget *) m_poGadget, NULL, NULL, CLICKTAB_Labels, (ULONG) &m_nodeList, TAG_DONE);

#elif defined(QT_GUI_LIB)

	m_tabWidget->addTab(a_gadget->GetGadget(), a_title);

#else /* ! QT_GUI_LIB */

	(void) a_gadget;
	(void) a_title;

#endif /* QT_GUI_LIB */

}

/**
 * Handle a tab being clicked.
 * This method should be called when a click on a tab is detected. It will update the currently active tab to
 * reflect the tab that was clicked. This is necessary to be called on Amiga OS, as tabs can only be drawn if they
 * are active. This method will ensure that this drawing is performed if the active tab has changed.
 *
 * @date	Saturday 31-Jan-2026 7:12 am, Code HQ Tokyo Tsukuda
 */

void CStdGadgetTabPane::clicked()
{

#ifdef __amigaos__

	ULONG activeTab = 0;

	/* Get the currently active tab and only do anything if it has changed */
	GetAttr(CLICKTAB_Current, m_poGadget, &activeTab);

	if (m_activeTab != static_cast<int>(activeTab))
	{
		/* Deactivate the old tab and activate the new one. This will cause the old tab to cache updates made to */
		/* its contents, rather than to draw them immediately, and the new tab will be drawn immediately and to */
		/* not cache updates in the future */
		m_gadgets[m_activeTab]->Activated(false);
		m_activeTab = activeTab;
		m_gadgets[m_activeTab]->Activated(true);
	}

#endif /* __amigaos__ */

}

/**
 * Activate a tab by index.
 * Deactivates the currently active tab, before activating the tab specified by the index passed in.
 *
 * @date	Saturday 24-Jan-2026 8:27 am, Code HQ Tokyo Tsukuda
 * @param	a_index			The index of the tab to be activated
 * @param	a_force			True to activate the tab even if it is already active
 */

void CStdGadgetTabPane::setActiveTab(int a_index, bool a_force)
{
	ASSERTM((a_index < static_cast<int>(m_gadgets.size())), "CStdGadgetTabPane::setActiveTab() => Tab index out of range");

	/* Only do anything if the active tab will change or if the user is forcing a change */
	if (m_activeTab != a_index || a_force)
	{
		m_gadgets[m_activeTab]->Activated(false);
		m_activeTab = a_index;

#ifdef __amigaos__

		SetGadgetAttrs((Gadget *) m_poGadget, NULL, NULL, CLICKTAB_Current, a_index, TAG_DONE);

#elif defined(QT_GUI_LIB)

		m_tabWidget->setCurrentIndex(a_index);

#endif /* QT_GUI_LIB */

		m_gadgets[m_activeTab]->Activated(true);
	}
}

#ifdef __amigaos__

/**
 * Shows or hides the gadget.
 * This Amiga OS reimplementation of CStdGadget::SetVisible() will pass the call through to the super method if the
 * gadget is to be hidden, but will recreate the underlying BOOPSI gadget if the gadget is to be shown. This enables
 * tab pane gadgets to be hidden and unhidden on Amiga OS, even though the underlying Reaction system does not
 * support this functionality.
 *
 * As well as recreating the tab pane gadget, any children attached to that tab pane will also be recreated.
 *
 * @date	Tuesday 10-Mar-2026 6:50 am, Code HQ Tokyo Tsukuda
 * @param	a_bVisible		true to make gadget visible, else false to hide it
 * @return	True if the visibility of the gadget changed, else false
 */

bool CStdGadgetTabPane::SetVisible(bool a_bVisible)
{
	bool retVal = false;

	/* If the gadget is being made visible then we want to recreate it */
	if (a_bVisible)
	{
		/* But only if it was previously hidden and thus does not already exist */
		if (m_bHidden)
		{
			m_poParentLayout->m_bEnableRefresh = EFalse;

			m_gadgets[0]->createNative();
			m_gadgets[1]->createNative();

			if (createNative())
			{
				retVal = true;

				/* Reattach it to the parent window */
				m_poParentLayout->ReAttach(this);

				/* Due to rebuilding the gadgets, the active tab will have reset to 0, so force a reset to */
				/* the previously active tab */
				setActiveTab(m_activeTab, true);

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
