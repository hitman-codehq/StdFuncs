
#include "../StdFuncs.h"
#include "../StdWindow.h"
#include "QtGadgetTree.h"
#include <QtWidgets/QHeaderView>

/**
 * Second phase constructor.
 * This is called by the generic tree class in order to complete Qt specific initialisation of the tree widget.
 *
 * @date	Friday 13-Aug-2021 5:33 pm, Code HQ @ Thomas's House
 * @param	a_numColumns	The number of columns to display in the tree
 */

void CQtGadgetTree::construct(int a_numColumns)
{
	/* Set the number of columns required and some sensible default settings */
	setColumnCount(a_numColumns);
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

	/* Hide the header by default, or Qt will add numbers to each column if no titles are set */
	header()->hide();

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

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 07-Mar-2026 9:16 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CQtGadgetTree::setTitle(const std::string &a_title)
{
	header()->show();
	setHeaderLabel(a_title.c_str());
}
