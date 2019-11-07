
#include "../StdFuncs.h"
#include "QtAction.h"
#include "QtWindow.h"

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 27-Sep-2019 3:02 pm, HERE Invalidenstrasse
 * @param	Parameter		Description
 * @return	Return value
 */

CQtWindow::CQtWindow(CWindow *a_poWindow, QSize &a_roSize)
{
	m_poWindow = a_poWindow;
	m_oSize = a_roSize;

	/* Allow the window to accept keyboard input by default */

	setFocusPolicy(Qt::StrongFocus);
}
