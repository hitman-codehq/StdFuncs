
#include <StdFuncs.h>
#include "StdFileListener.h"

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 11-Aug-2025 1:25 pm, Sarutahiko Coffee Otemachi TODO: CAW
 * @param	Parameter		Description
 * @return	Return value
 */

// TODO: CAW - CStdFileListener or watcher?
void CStdFileListener::listen(Callback callback)
{
	m_callback = callback;
	m_listener.watch(); // TODO: CAW - Listener or watcher?
};
