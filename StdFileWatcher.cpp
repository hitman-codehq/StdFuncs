
#include <StdFuncs.h>
#include "StdFileWatcher.h"

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 23-Aug-2025 7:57 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdFileWatcher::changed(const std::string &a_directoryName)
{
	ASSERTM((m_callback != nullptr), "CStdFileWatcher::changed() => File watcher callback must not be NULL");

	m_callback(a_directoryName);
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 11-Aug-2025 1:25 pm, Sarutahiko Coffee Marunouchi
 * @param	Parameter		Description
 * @return	Return value
 */

void CStdFileWatcher::watch(const std::string &a_directoryName, Callback callback)
{
	m_callback = callback;

#ifdef QT_GUI_LIB

	m_watcher.watch(a_directoryName);

#endif /* QT_GUI_LIB */

};
