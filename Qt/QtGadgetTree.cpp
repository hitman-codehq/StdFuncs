
#include "../StdFuncs.h"
#include "../StdWindow.h"
#include "QtGadgetTree.h"
#include <QtWidgets/QHeaderView>

/**
 * Second phase constructor.
 * This is called by the generic tree class in order to complete Qt specific initialisation of the tree widget.
 *
 * @date	Friday 13-Aug-2021 5:33 pm, Code HQ @ Thomas's House
 * @param	a_title			Title to be displayed at the top of the tree's column
 */

void CQtGadgetTree::construct(const std::string &a_title, int a_numColumns)
{
	/* Set some sensible default settings, such as the number of columns to display */
	setColumnCount(a_numColumns);

	if (!a_title.empty())
	{
		setHeaderLabel(a_title.c_str());
	}
	else
	{
		header()->hide();
	}

	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

	/* And connect the itemClicked() slot so that we are notified when an item is clicked */
	connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(itemClicked()));
}

/**
 * Slot for when an item is clicked.
 * Called by Qt when an item is clicked.  This method will simply pass the notification onto the
 * client that owns the tree widget.
 *
 * @date	Friday 13-Aug-2021 5:33 pm, Code HQ @ Thomas's House
 */

void CQtGadgetTree::itemClicked()
{
	m_parentTree->GetParentWindow()->HandleCommand(m_parentTree->GetGadgetID());
}
