
#include "StdFuncs.h"
#include "StdGadgets.h"

#ifdef QT_GUI_LIB

#include <QtWidgets/QTabWidget>

#endif /* QT_GUI_LIB */

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 29-Dec-2025 3:45 pm, London Town Hotel
 * @param	Parameter		Description
 * @return	Return value
 */

CStdGadgetTabPane *CStdGadgetTabPane::New(CStdGadgetLayout *a_parentLayout)
{
	CStdGadgetTabPane *retVal = new CStdGadgetTabPane(a_parentLayout);

	if (retVal != nullptr)
	{
		retVal->construct();
	}

	return retVal;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 29-Dec-2025 3:45 pm, London Town Hotel
 * @param	Parameter		Description
 * @return	Return value
 */

CStdGadgetTabPane *CStdGadgetTabPane::construct()
{

#ifdef QT_GUI_LIB

	m_tabWidget = new QTabWidget;
	m_poGadget = m_tabWidget;

#endif /* QT_GUI_LIB */

	m_poParentLayout->Attach(this);

	return this;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 19-Jan-2026 6:37 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdGadgetTabPane::addTab(CStdGadget *a_gadget, const char *a_title)
{

#ifdef QT_GUI_LIB

	m_tabWidget->addTab(a_gadget->GetGadget(), a_title);

#endif /* QT_GUI_LIB */

}
