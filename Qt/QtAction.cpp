
#include "../StdFuncs.h"
#include "../StdWindow.h"
#include "QtAction.h"

/**
 * Handles an action being triggered.
 * This function is called by Qt when a menu item is selected and will pass the message onto
 * The Framework and its underlying client.
 *
 * @date	Monday 28-Jan-2013 6:25 am
 */

void CQtAction::actionTriggered()
{
	m_poWindow->Window()->HandleCommand(m_iCommand);
}
