
#include "../StdFuncs.h"
#include "../StdWindow.h"
#include "QtGadgetTextPane.h"

/**
 * Second phase constructor.
 * This is called by the generic tree class in order to complete Qt specific initialisation of the tree widget.
 *
 * @date	Friday 13-Aug-2021 5:33 pm, Code HQ @ Thomas's House
 * @param	a_title			Title to be displayed at the top of the tree's column
 */

/**
 * Short description.
 * Long multi line description.
 *
 *
 *
 * @date	Saturday 06-Jan-2024 3:21 pm, on board Jetstar flight JQ 8 from Brisbane to Tokyo
 * @param	Parameter		Description
 * @return	Return value
 */

void CQtGadgetTextPane::construct() // const std::string &a_title)
{
	/* Set some sensible default settings, such as the number of columns to display */
	/*setColumnCount(1);
	setHeaderLabels(QStringList(a_title.c_str()));*/
	//setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

	/* And connect the itemClicked() slot so that we are notified when an item is clicked */
	//connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(itemClicked()));
}
