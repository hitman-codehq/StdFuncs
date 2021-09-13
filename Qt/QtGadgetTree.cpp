
#include "../StdFuncs.h"
#include "../StdWindow.h"
#include "QtGadgetTree.h"

/**
 * Second phase constructor.
 * This is called by the generic tree class in order to complete Qt specific initialisation of the tree widget.
 *
 * @date	Friday 13-Aug-2021 5:33 pm, Code HQ @ Thomas's House
 * @param	Parameter		Description
 */

void CQtTreeWidget::construct()
{
	/* Set some sensible default settings, such as the number of columns to display */
	setColumnCount(1);
	setHeaderLabels(QStringList(QString("Files")));
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

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

void CQtTreeWidget::itemClicked()
{
	m_parentTree->GetParentWindow()->HandleCommand(m_parentTree->GetGadgetID());
}
