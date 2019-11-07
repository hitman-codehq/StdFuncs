
#include "../StdFuncs.h"
#include "../StdWindow.h"
#include "QtGadgetSlider.h"

/**
 * Notifies the generic framework slider of slider events.

 * Called by the Qt framework whenever a Qt slider's value is changed.  This
 * function simply calls the generic framework slider that owns this instance.
 *
 * @date	Saturday 09-Mar-2013 9:13, Leo's house in Vienna
 * @param	a_iValue	The position of the slider
 */

void CQtGadgetSlider::valueChanged(TInt a_iValue)
{
	/* The CStdGadgetSlider class works with a range from 1 - maxrange but Qt works */
	/* from 0 - maxrange, so take this into account when calling the client code */

	m_poParentSlider->Updated(a_iValue + 1);
}
