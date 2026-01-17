
#ifndef STDGADGETTABPANE_H
#define STDGADGETTABPANE_H

#include <vector>
#include "StdGadgets.h"

/**
 * A tab pane that can display multiple tabs and switch between them.
 * Any sort of gadget supported by The Framework can be added to this gadget. The gadget will automatically take care
 * of ensuring that the correct gadget is displayed when a tab is selected.
 */

class CStdGadgetTabPane : public CStdGadget
{
	int							m_activeTab;		/**< The index of the currently active tab */
	std::vector<CStdGadget *>	m_gadgets;			/**< List of child gadgets attached to the tab gadget */

#ifdef __amigaos__

	List						m_nodeList;			/**< List of nodes in the Amiga OS native gadget */
	Object						*m_gadgetsLayout;	/**< Pointer to the layout used to hold child gadgets */

#elif defined(QT_GUI_LIB)

	QTabWidget					*m_tabWidget;		/**< Pointer to the Qt tab widget */

#endif /* QT_GUI_LIB */

public:

	static CStdGadgetTabPane *New(CStdGadgetLayout *a_parentLayout, int a_gadgetID, CStdGadget *a_child1,
		CStdGadget *a_child2);

	CStdGadgetTabPane(int a_gadgetID, CStdGadgetLayout *a_parentLayout)
	{
		m_iGadgetID = a_gadgetID;
		m_poParentLayout = a_parentLayout;
	}

	int construct(CStdGadget *a_child1, CStdGadget *a_child2);

	void addTab(CStdGadget *a_gadget, const char *a_title);

	void clicked();

	void setActiveTab(int a_index);
};

#endif // STDGADGETTABPANE_H
